#include <iostream>
#include <map>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

// ==========================================
// 1. DATA STRUCTURES
// ==========================================
enum class Side { BUY, SELL };

struct Order {
    uint64_t id;
    Side side;
    uint32_t price;
    uint32_t qty;
};

// ==========================================
// 2. THE THREAD-SAFE BLOCKING QUEUE
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

        if (queue_.empty() && done_) {
            return false;
        }

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
// 3. THE MATCHING ENGINE (ORDER BOOK)
// ==========================================
class OrderBook {
private:
    std::map<uint32_t, uint32_t, std::greater<uint32_t>> bids_; // Highest buy first
    std::map<uint32_t, uint32_t, std::less<uint32_t>> asks_;    // Lowest sell first
    uint64_t total_volume_matched_ = 0;

public:
    void process_order(const Order& order) {
        uint32_t remaining_qty = order.qty;

        if (order.side == Side::BUY) {
            while (remaining_qty > 0 && !asks_.empty()) {
                auto it = asks_.begin();
                uint32_t ask_price = it->first;
                uint32_t& ask_qty = it->second;

                if (order.price >= ask_price) {
                    uint32_t matched_qty = std::min(remaining_qty, ask_qty);
                    total_volume_matched_ += matched_qty;
                    remaining_qty -= matched_qty;
                    ask_qty -= matched_qty;

                    if (ask_qty == 0) asks_.erase(it);
                }
                else {
                    break;
                }
            }
            if (remaining_qty > 0) bids_[order.price] += remaining_qty;
        }
        else {
            while (remaining_qty > 0 && !bids_.empty()) {
                auto it = bids_.begin();
                uint32_t bid_price = it->first;
                uint32_t& bid_qty = it->second;

                if (order.price <= bid_price) {
                    uint32_t matched_qty = std::min(remaining_qty, bid_qty);
                    total_volume_matched_ += matched_qty;
                    remaining_qty -= matched_qty;
                    bid_qty -= matched_qty;

                    if (bid_qty == 0) bids_.erase(it);
                }
                else {
                    break;
                }
            }
            if (remaining_qty > 0) asks_[order.price] += remaining_qty;
        }
    }

    uint64_t get_total_volume() const { return total_volume_matched_; }
};

// ==========================================
// 4. DETERMINISTIC SYNTHETIC LOAD GENERATOR
// ==========================================
// Generates identical datasets so single-threaded vs multi-threaded results match perfectly
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
        uint32_t price = 95 + (pseudo_rand() % 10); // Prices between 95 and 105
        uint32_t qty = 1 + (pseudo_rand() % 50);    // Quantities between 1 and 50
        orders.push_back({ i, side, price, qty });
    }
    return orders;
}

// ==========================================
// 5. COORDINATION AND TEST BENCH
// ==========================================
int main() {
    const size_t NUM_ORDERS = 1000000;
    std::cout << "[Step 1] Generating " << NUM_ORDERS << " deterministic orders..." << std::endl;
    std::vector<Order> synthetic_orders = generate_synthetic_load(NUM_ORDERS);

    // ------------------------------------------
    // PHASE A: SINGLE-THREADED BASELINE
    // ------------------------------------------
    std::cout << "[Step 2] Running single-threaded baseline engine..." << std::endl;
    auto start_st = std::chrono::high_resolution_clock::now();

    OrderBook baseline_book;
    for (const auto& order : synthetic_orders) {
        baseline_book.process_order(order);
    }

    auto end_st = std::chrono::high_resolution_clock::now();
    uint64_t baseline_volume = baseline_book.get_total_volume();

    // ------------------------------------------
    // PHASE B: MULTI-THREADED CONCURRENT PIPELINE
    // ------------------------------------------
    std::cout << "[Step 3] Running multi-threaded concurrent engine..." << std::endl;
    auto start_mt = std::chrono::high_resolution_clock::now();

    BlockingQueue<Order> bq;
    OrderBook concurrent_book;

    // Launch the Producer Thread
    std::thread producer([&]() {
        for (const auto& order : synthetic_orders) {
            bq.push(order);
        }
        bq.set_done(); // Signal closure when all orders are queued
        });

    // Launch the Consumer Thread
    std::thread consumer([&]() {
        Order order;
        while (bq.pop(order)) {
            concurrent_book.process_order(order);
        }
        });

    // Wait for both execution streams to finish execution safely
    producer.join();
    consumer.join();

    auto end_mt = std::chrono::high_resolution_clock::now();
    uint64_t concurrent_volume = concurrent_book.get_total_volume();

    // ------------------------------------------
    // PHASE C: CORRECTNESS AND PERFORMANCE VERIFICATION
    // ------------------------------------------
    std::chrono::duration<double, std::milli> st_duration = end_st - start_st;
    std::chrono::duration<double, std::milli> mt_duration = end_mt - start_mt;

    std::cout << "\n==================================================\n";
    std::cout << "Baseline Matched Volume:   " << baseline_volume << "\n";
    std::cout << "Concurrent Matched Volume: " << concurrent_volume << "\n";

    if (baseline_volume == concurrent_volume) {
        std::cout << "VERIFICATION RESULT: SUCCESS! Matches perfectly.\n";
    }
    else {
        std::cout << "VERIFICATION RESULT: CRITICAL FAILURE! Volumes mismatch.\n";
    }
    std::cout << "==================================================\n";
    std::cout << "Single-threaded duration: " << start_st.time_since_epoch().count() << " units (" << st_duration.count() << " ms)\n";
    std::cout << "Concurrent pipeline duration:  " << start_mt.time_since_epoch().count() << " units (" << mt_duration.count() << " ms)\n";

    return 0;
}