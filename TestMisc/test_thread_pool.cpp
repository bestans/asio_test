#include "test_misc.h"
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include "asio.hpp"

using namespace std;
//#define ASIO_NO_DEPRECATED
class ThreadPool {
public:
	explicit ThreadPool(std::size_t size)
		: work_guard_(asio::make_work_guard(io_context_)) {
		workers_.reserve(size);
		for (std::size_t i = 0; i < size; ++i) {
			workers_.emplace_back([&] { io_context_.run(); } );
		}
	}

	~ThreadPool() {
		io_context_.stop();

		for (auto& w : workers_) {
			w.join();
		}
	}

	// Add new work item to the pool.
	template<class F>
	void Enqueue(F&& f) {
		asio::post(io_context_, f);
	}
	asio::io_context& GetContext() {
		return io_context_;
	}
private:
	std::vector<std::thread> workers_;
	asio::io_context io_context_;

	typedef asio::io_context::executor_type ExecutorType;
	asio::executor_work_guard<ExecutorType> work_guard_;
};
std::mutex g_io_mutex;
void TestThreadPool() {
	// Create a thread pool of 4 worker threads.
	ThreadPool pool(4);

	// Queue a bunch of work items.
	for (int i = 0; i < 8; ++i) {
		auto f = [i] {
			{
				std::lock_guard<std::mutex> lock(g_io_mutex);
				std::cout << "Hello" << "(" << i << ") " << std::endl;
			}

			std::this_thread::sleep_for(std::chrono::seconds(10));

			{
				std::lock_guard<std::mutex> lock(g_io_mutex);
				std::cout << "World" << "(" << i << ")" << std::endl;
			}
		};
		pool.Enqueue(f);
	}
}
