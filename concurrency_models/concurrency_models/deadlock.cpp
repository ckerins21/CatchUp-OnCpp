#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>

std::mutex m1, m2;

void thread1() {
	std::scoped_lock lock(m1,m2);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::cout << "Thread 1 both acquired safely" << std::endl;
}

void thread2() {
	std::scoped_lock lock(m1,m2);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	
	std::cout << "Thread 1 both acquired safely" << std::endl;
}

int main() {
	std::thread t1(thread1);
	std::thread t2(thread2);

	t1.join();
	t2.join();

	return 0;
}