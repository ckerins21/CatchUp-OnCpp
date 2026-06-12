#include <iostream>
#include <vector>
#include <chrono>

int main() {
    constexpr int N = 4096;
    std::vector<std::vector<int>> m(N, std::vector<int>(N, 1));

    // --- WARM THE MEMORY FIRST (Forces page faults out of our telemetry) ---
    long long volatile dummy = 0;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            dummy += m[i][j];

    // 1. Row-major (cache-friendly)
    auto t1 = std::chrono::steady_clock::now();
    long long s1 = 0;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) s1 += m[i][j];
    auto t2 = std::chrono::steady_clock::now();

    // 2. Column-major (cache-hostile)
    auto t3 = std::chrono::steady_clock::now();
    long long s2 = 0;
    for (int j = 0; j < N; ++j)
        for (int i = 0; i < N; ++i) s2 += m[i][j];
    auto t4 = std::chrono::steady_clock::now();

    std::cout << "row-major: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
    std::cout << "col-major: " << std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count() << " ms\n";

    return s1 + s2;
}