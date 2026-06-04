#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <iostream>
#include <numeric>

template <typename T>
class BlockingQueue {
	std::queue<T> q_;
	mutable std::mutex m_;
	std::condition_variable cv_;
public:
	void push(T value)
	{
		std::lock_guard<std::mutex> lk(m_);
		q_.push(std::move(value));
		cv_.notify_one();

	}

	T pop()
	{
		std::unique_lock<std::mutex> lk(m_);
		cv_.wait(lk, [this] { return !q_.empty(); });
		T value = std::move(q_.front());
		q_.pop();
		return value;
	}

	bool trypop(T& out)
	{
		std::lock_guard<std::mutex> lk(m_);
		if (q_.empty()) return false;
		out = std::move(q_.front());
		q_.pop();
		return true;
	}
};

int main() {

	BlockingQueue <int> bq;
	long long total_sum = 0;  // ← PLAIN long long, NO atomicity (will cause race!)
	const int num_producers = 4;
	const int num_consumers = 4;

	const int items_per_producer = 2500;

	std::vector<std::thread> producers;

	for (int i = 0; i < num_producers; ++i) {
		producers.emplace_back([&] {
			for (int j = 0; j < items_per_producer; ++j) bq.push(j);
			});
		};


	std::vector<std::thread> consumers;

	for (int i = 0; i < num_consumers; ++i) {
		consumers.emplace_back([&] {
			for (int j = 0; j < items_per_producer; ++j) {
				total_sum += bq.pop();  // ← DATA RACE: multiple threads write without sync

			}

			});
	}

	for (auto& t : producers) t.join();
	for (auto& t : consumers) t.join();

	long long expected_sum = num_producers * ((long long)items_per_producer * (items_per_producer - 1) / 2);
	std::cout << "expected sum : " << expected_sum << "\n";
	std::cout << "Actual sum :" << total_sum << "\n";


	if (total_sum == expected_sum) std::cout << "Success: No data loss \n";
	else std::cout << "Failure: Data msmatch \n";
	return 0;


}
