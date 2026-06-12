#include <iostream>
#include <array>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

// ==========================================
// 1. CONSTANTS AND DATA STRUCTURES
// ==========================================
static constexpr uint32_t MAX_PRICE = 200;

enum class Side { BUY, SELL };

struct Order {
    uint64_t id;
    Side side;
    uint32_t price;
    uint32_t qty;
};

// ==========================================
// 2. THE THREAD-SAFE BLOCKING QUEUE
// (unchanged from before)
// ==========================================
template <typename T>
class BlockingQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool done_ = false;

public:
    BlockingQueue() = default;
    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    void push(const T& item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(item);
        }
        cv_.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() {
            return !queue_.empty() || done_;
            });
        if (queue_.empty() && done_) return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }

    void set_done() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            done_ = true;
        }
        cv_.notify_all();
    }
};

// ==========================================
// 3. THE NEW FLAT ARRAY ORDER BOOK
// ==========================================
class OrderBook {
private:
    std::array<uint32_t, MAX_PRICE> bids_{};  // index = price, value = qty
    std::array<uint32_t, MAX_PRICE> asks_{};  // {} zero-initialises every slot
    uint64_t total_volume_matched_ = 0;

public:
    void process_order(const Order& order) {
        uint32_t remaining_qty = order.qty;

        if (order.side == Side::BUY) {
            // Walk asks from lowest price upward
            for (uint32_t p = 0; p < MAX_PRICE && remaining_qty > 0; ++p) {
                if (asks_[p] == 0) continue;        // nothing resting here
                if (order.price < p) break;          // price too high, stop

                uint32_t matched = std::min(remaining_qty, asks_[p]);
                asks_[p] -= matched;
                remaining_qty -= matched;
                total_volume_matched_ += matched;
            }
            if (remaining_qty > 0) bids_[order.price] += remaining_qty;
        }
        else {
            // Walk bids from highest price downward
            for (int p = MAX_PRICE - 1; p >= 0 && remaining_qty > 0; --p) {
                if (bids_[p] == 0) continue;
                if (order.price > (uint32_t)p) break;

                uint32_t matched = std::min(remaining_qty, bids_[p]);
                bids_[p] -= matched;
                remaining_qty -= matched;
                total_volume_matched_ += matched;
            }
            if (remaining_qty > 0) asks_[order.price] += remaining_qty;
        }
    }

    uint64_t get_total_volume() const { return total_volume_matched_; }
};

// ==========================================
// 4. SYNTHETIC LOAD GENERATOR
// (unchanged from before)
// ==========================================
std::vector<Order> generate_synthetic_load(size_t count) {
    std::vector<Order> orders;
    orders.reserve(count);

    uint64_t seed = 42;
    auto pseudo_rand = [&seed]() {
        seed = (seed * 6364136223846793005ULL + 1442695040888963407ULL);
        return seed;
        };

    for (size_t i = 0; i < count; ++i) {
        Side side = (pseudo_rand() % 2 == 0) ? Side::BUY : Side::SELL;
        uint32_t price = 95 + (pseudo_rand() % 10);
        uint32_t qty = 1 + (pseudo_rand() % 50);
        orders.push_back({ i, side, price, qty });
    }
    return orders;
}

// ==========================================
// 5. MAIN — unchanged
// ==========================================
int main() {
    const size_t NUM_ORDERS = 1000000;
    std::cout << "[Step 1] Generating " << NUM_ORDERS
        << " deterministic orders...\n";
    std::vector<Order> synthetic_orders = generate_synthetic_load(NUM_ORDERS);

    // PHASE A: SINGLE-THREADED
    std::cout << "[Step 2] Running single-threaded baseline engine...\n";
    auto start_st = std::chrono::high_resolution_clock::now();

    OrderBook baseline_book;
    for (const auto& order : synthetic_orders)
        baseline_book.process_order(order);

    auto end_st = std::chrono::high_resolution_clock::now();
    uint64_t baseline_volume = baseline_book.get_total_volume();

    // PHASE B: MULTI-THREADED
    std::cout << "[Step 3] Running multi-threaded concurrent engine...\n";
    auto start_mt = std::chrono::high_resolution_clock::now();

    BlockingQueue<Order> bq;
    OrderBook concurrent_book;

    std::thread producer([&]() {
        for (const auto& order : synthetic_orders)
            bq.push(order);
        bq.set_done();
        });

    std::thread consumer([&]() {
        Order order;
        while (bq.pop(order))
            concurrent_book.process_order(order);
        });

    producer.join();
    consumer.join();

    auto end_mt = std::chrono::high_resolution_clock::now();
    uint64_t concurrent_volume = concurrent_book.get_total_volume();

    // PHASE C: RESULTS
    std::chrono::duration<double, std::milli> st_dur = end_st - start_st;
    std::chrono::duration<double, std::milli> mt_dur = end_mt - start_mt;

    std::cout << "\n==================================================\n";
    std::cout << "Baseline Matched Volume:   " << baseline_volume << "\n";
    std::cout << "Concurrent Matched Volume: " << concurrent_volume << "\n";

    if (baseline_volume == concurrent_volume)
        std::cout << "VERIFICATION: SUCCESS\n";
    else
        std::cout << "VERIFICATION: FAILURE — volumes mismatch\n";

    std::cout << "==================================================\n";
    std::cout << "Single-threaded:  " << st_dur.count() << " ms\n";
    std::cout << "Multi-threaded:   " << mt_dur.count() << " ms\n";

    return 0;
}