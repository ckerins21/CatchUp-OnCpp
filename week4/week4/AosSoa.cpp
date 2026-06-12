#include <iostream>
#include <vector>
#include <chrono>

const int N = 10'000'000; // 10 Million items

// ============================================================================
// 1. ARRAY OF STRUCTURES (AoS) LAYOUT
// ============================================================================
struct AoS_Order {
    long price;      // 8 bytes -> THE HOT FIELD WE CARE ABOUT
    long volume;     // 8 bytes -> Cold
    long orderId;    // 8 bytes -> Cold
    long accountId;  // 8 bytes -> Cold
    long timestamp;  // 8 bytes -> Cold
}; // Total size = 40 bytes

// ============================================================================
// 2. STRUCTURE OF ARRAYS (SoA) LAYOUT
// ============================================================================
struct SoA_Orders {
    std::vector<long> prices;     // Packed tight in memory together!
    std::vector<long> volumes;
    std::vector<long> orderIds;
    std::vector<long> accountIds;
    std::vector<long> timestamps;
};

int main() {
    // ---- Allocate and Fill AoS ----
    std::vector<AoS_Order> aos_vector(N, AoS_Order{ 10, 20, 30, 40, 50 });

    // ---- Allocate and Fill SoA ----
    SoA_Orders soa_vectors;
    soa_vectors.prices.resize(N, 10);
    soa_vectors.volumes.resize(N, 20);
    soa_vectors.orderIds.resize(N, 30);
    soa_vectors.accountIds.resize(N, 40);
    soa_vectors.timestamps.resize(N, 50);

    // ============================================================================
    // BENCHMARK 1: AoS (Slow due to cache pollution)
    // ============================================================================
    auto start_aos = std::chrono::high_resolution_clock::now();
    long aos_sum = 0;
    for (int i = 0; i < N; ++i) {
        aos_sum += aos_vector[i].price; // Has to skip 32 bytes of cold data every step!
    }
    auto end_aos = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> aos_time = end_aos - start_aos;

    // ============================================================================
    // BENCHMARK 2: SoA (Fast because memory is perfectly packed)
    // ============================================================================
    auto start_soa = std::chrono::high_resolution_clock::now();
    long soa_sum = 0;
    for (int i = 0; i < N; ++i) {
        soa_sum += soa_vectors.prices[i]; // Pure sequential streaming of only prices!
    }
    auto end_soa = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> soa_time = end_soa - start_soa;

    // ---- Print Results ----
    std::cout << "AoS Sum: " << aos_sum << " | Time: " << aos_time.count() << " ms\n";
    std::cout << "SoA Sum: " << soa_sum << " | Time: " << soa_time.count() << " ms\n";

    return 0;
}