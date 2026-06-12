#include <benchmark/benchmark.h>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <random>

// Helper to generate deterministic random lookup keys
std::vector<int> generate_keys(int N) {
    std::vector<int> keys(N);
    for (int i = 0; i < N; ++i) keys[i] = i * 2; // Even numbers

    std::mt19937 g(42); // Fixed seed for reproducibility
    std::shuffle(keys.begin(), keys.end(), g);
    return keys;
}

// 1. Sorted Vector (flat_map simulation) Lookup Benchmark
static void BM_SortedVector_Lookup(benchmark::State& state) {
    int N = state.range(0);
    auto keys = generate_keys(N);

    // Setup container
    std::vector<std::pair<int, int>> container;
    container.reserve(N);
    for (int k : keys) container.push_back({ k, 1 });
    // Sort by key to allow binary search
    std::sort(container.begin(), container.end());

    // Generate lookup targets (some hit, some miss)
    auto lookups = generate_keys(N);

    for (auto _ : state) {
        for (int target : lookups) {
            // Binary search lookup
            auto it = std::lower_bound(container.begin(), container.end(), std::make_pair(target, 0),
                [](const auto& a, const auto& b) { return a.first < b.first; });

            if (it != container.end() && it->first == target) {
                benchmark::DoNotOptimize(it->second); // Force the read to be real
            }
        }
    }
}
BENCHMARK(BM_SortedVector_Lookup)->RangeMultiplier(10)->Range(100, 100000);

// 2. Unordered Map Lookup Benchmark
static void BM_UnorderedMap_Lookup(benchmark::State& state) {
    int N = state.range(0);
    auto keys = generate_keys(N);

    std::unordered_map<int, int> container;
    for (int k : keys) container[k] = 1;

    auto lookups = generate_keys(N);

    for (auto _ : state) {
        for (int target : lookups) {
            auto it = container.find(target);
            if (it != container.end()) {
                benchmark::DoNotOptimize(it->second); // Force read execution
            }
        }
    }
}
BENCHMARK(BM_UnorderedMap_Lookup)->RangeMultiplier(10)->Range(100, 100000);

BENCHMARK_MAIN();


//...@LAPTOP - PFM87O2Q: / mnt / c / Users / caola / source / repos / week3_performance_profiling / week3_performance_profiling$ g++ - O3 - std = c++17 container_benchmark.cpp - lbenchmark - lpthread - o container_bench
//...@LAPTOP - PFM87O2Q: / mnt / c / Users / caola / source / repos / week3_performance_profiling / week3_performance_profiling$ taskset - c 0 . / container_bench
//2026 - 06 - 06T07 : 38 : 33 - 07 : 00
//Running . / container_bench
//Run on(4 X 2904 MHz CPU s)
//CPU Caches :
//L1 Data 32 KiB(x2)
//L1 Instruction 32 KiB(x2)
//L2 Unified 256 KiB(x2)
//L3 Unified 4096 KiB(x1)
//Load Average : 0.20, 0.07, 0.02
//* **WARNING * **Library was built as DEBUG.Timings may be affected.
//------------------------------------------------------------------------
//Benchmark                              Time             CPU   Iterations
//------------------------------------------------------------------------
//BM_SortedVector_Lookup / 100           824 ns          823 ns       834114
//BM_SortedVector_Lookup / 1000        60981 ns        60980 ns        13240
//BM_SortedVector_Lookup / 10000     1059637 ns      1059597 ns          651
//BM_SortedVector_Lookup / 100000   15076600 ns     15076617 ns           46
//BM_UnorderedMap_Lookup / 100           805 ns          805 ns       852391
//BM_UnorderedMap_Lookup / 1000        11012 ns        11011 ns        84966
//BM_UnorderedMap_Lookup / 10000      138785 ns       138765 ns         5654
//BM_UnorderedMap_Lookup / 100000    1935699 ns      1935613 ns          307