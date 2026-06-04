#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <chrono>
#include <numeric>
#include <string>

template<typename Fn>
double time_ms(Fn&& fn) {
	auto start = std::chrono::steady_clock::now();
	fn();
	auto end = std::chrono::steady_clock::now();
	return std::chrono::duration<double, std::milli>(end - start).count();
}

template<typename Container>
double benchmark_push(size_t n) {
	return time_ms([&]() {
		Container c;
		for (size_t i = 0; i < n; ++i) {
			c.push_back(int(i));
		}
		});
}

template<typename Container>
double benchmark_sum(const Container& c) {
	volatile long long sink = 0;
    return time_ms([&]() {
        long long sum = 0;
        for (auto v : c) sum += v;
        sink = sum;
        });
}

// Workload 3: erase every other element
// Takes a COPY so the original is unchanged between runs
template <typename Container>
double benchmark_erase(Container c) {
    return time_ms([&] {
        auto it = c.begin();
        while (it != c.end()) {
            it = c.erase(it);   // erase returns iterator to next element
            if (it != c.end())
                ++it;           // skip one
        }
        });
}

// Pretty-print a row
void row(const std::string& label, double vec, double lst, double deq) {
    std::cout << "  " << label
        << "   vector: " << vec
        << " ms   list: " << lst
        << " ms   deque: " << deq << " ms\n";
}

int main() {
    for (size_t N : {10'000UL, 100'000UL, 1'000'000UL}) {
        std::cout << "\n=== N = " << N << " ===\n";

        // --- Push ---
        double pv = benchmark_push<std::vector<int>>(N);
        double pl = benchmark_push<std::list<int>>(N);
        double pd = benchmark_push<std::deque<int>>(N);
        row("push  ", pv, pl, pd);

        // --- Build containers for sum + erase ---
        std::vector<int> v(N); std::iota(v.begin(), v.end(), 0);
        std::list<int>   l(N); std::iota(l.begin(), l.end(), 0);
        std::deque<int>  d(N); std::iota(d.begin(), d.end(), 0);

        // --- Sum ---
        row("sum   ", benchmark_sum(v), benchmark_sum(l), benchmark_sum(d));

        // --- Erase (passes copies, originals untouched) ---
        row("erase ", benchmark_erase(v), benchmark_erase(l), benchmark_erase(d));
    }
}