#include "test_misc.h"
#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;

template <class T>
class DataQueue {
	static const T defaultT;
public:
	bool Push(T t) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (stop_) {
			return false;
		}
		out_.push_back(std::move(t));
		condition_.notify_one();
		return true;
	}
	void Stop() {
		std::unique_lock<std::mutex> lock(mutex_);
		if (stop_) {
			return;
		}
		stop_ = true;
		condition_.notify_one();
	}
	bool Pop(std::deque<T>& receive) {
		std::unique_lock<std::mutex> lock(mutex_);
		while (out_.empty() && !stop_)
			condition_.wait(lock);
		receive.clear();
		std::swap(receive, out_);
		return !stop_ || !receive.empty();
	}

private:
	std::mutex mutex_;
	std::condition_variable condition_;
	std::deque<T> out_;
	bool stop_ = false;
};

void test_dataqueue() {
	auto beginTime = std::chrono::high_resolution_clock::now();
	DataQueue<int> dataq;
	int maxC = 1000000;
	std::thread t1([&]() {
		int i = 0;
		while (++i <= maxC) {
			dataq.Push(int(1));
		}
		dataq.Stop();
		std::cout << i << std::endl;
	});
	int receiveC = 0;
	int multiC = 0;
	size_t multiTimes = 0;
	std::thread t2([&]() {
		std::deque<int> in;
		while (dataq.Pop(in)) {
			while (!in.empty()) {
				receiveC++;
				in.pop_front();
			}
		}
	});
	t1.join();
	t2.join();

	auto endTime = std::chrono::high_resolution_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
	std::cout << "elapsed time is " << elapsedTime.count() << " second" << "," << multiC << "," << multiTimes << "," << receiveC << std::endl;
}

void test_dataqueue2() {
	auto beginTime = std::chrono::high_resolution_clock::now();
	DataQueue<int> dataq;
	int maxC = 1000000;
	std::thread t1([&]() {
		int i = 0;
		while (++i <= maxC) {
			dataq.Push(int(1));
		}
		dataq.Stop();
		std::cout << i << std::endl;
		});
	int receiveC = 0;
	int multiC = 0;
	size_t multiTimes = 0;
	std::thread t2([&]() {
		std::deque<int> in;
		while (dataq.Pop(in)) {
			while (!in.empty()) {
				receiveC++;
				in.pop_front();
			}
		}
		});
	t1.join();
	t2.join();

	auto endTime = std::chrono::high_resolution_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
	std::cout << "elapsed time is " << elapsedTime.count() << " second" << "," << multiC << "," << multiTimes << "," << receiveC << std::endl;
}