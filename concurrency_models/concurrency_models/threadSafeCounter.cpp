#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

/*// --- Mutex Counter ---
class Counter {
    mutable std::mutex m_;
    long long count_ = 0;
public:
    void increment() {
        std::lock_guard<std::mutex> lk(m_);
        ++count_;
    }
    long long get() const {
        std::lock_guard<std::mutex> lk(m_);
        return count_;
    }
};

// --- Atomic Counter ---
class AtomicCounter {
    std::atomic<long long> count_{ 0 };
public:
    void increment() { count_.fetch_add(1, std::memory_order_relaxed); }
    long long get() const { return count_.load(std::memory_order_relaxed); }
};

// --- Helper to Run Benchmark ---
template <typename T>
void run_benchmark(const std::string& name) {
    T counter;
    const int num_threads = 8;
    const int increments_per_thread = 1000000;
    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < increments_per_thread; ++j) counter.increment();
            });
    }

    for (auto& t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << name << " took: " << diff.count() << " seconds. Final count: " << counter.get() << std::endl;
}*/

// This class is intentionally NOT thread-safe
class RaceCounter {
    long long count_ = 0;
public:
    void increment() {
        // No mutex or atomic here! 
        // Multiple threads will access 'count_' simultaneously.
        ++count_;
    }
    long long get() const { return count_; }
};

RaceCounter increment_func(RaceCounter x, int no_threads, int incrementing_no, std::vector<std::thread> utlize_thread)
{
    for (int i = 0; i < no_threads; ++i) {
        utlize_thread.emplace_back([&] {
            for (int j = 0; j < incrementing_no; j++) x.increment();
            });
    }
    for (auto& t : utlize_thread) t.join();

    

    return x;
}

int main() {
    std::cout << "Starting benchmark..." << std::endl;
    //run_benchmark<Counter>("Mutex Version");
    //run_benchmark<AtomicCounter>("Atomic Version");

    RaceCounter c;
    const int num_threads = 8;
    const int increments = 1000000;

    std::vector<std::thread> threads;

    increment_func(c, num_threads, increments, threads);

    std::cout << "final count should be 8,000,000 but it is : " << c.get() << std::endl;

    return 0;
}