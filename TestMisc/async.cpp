#include <asio/ts/executor.hpp>
#include <asio/thread_pool.hpp>
#include <iostream>
#include <string>
#include <map>
#include <mutex>
#include "test_misc.h"

using asio::bind_executor;
using asio::dispatch;
using asio::make_work_guard;
using asio::post;
using asio::thread_pool;

// A function to asynchronously read a single line from an input stream.
template <class Handler>
void async_getline1(std::istream& is, Handler handler)
{
	// Create executor_work for the handler's associated executor.
	auto work = make_work_guard(handler);

	// Post a function object to do the work asynchronously.
	post([&is, work, handler = std::move(handler)]() mutable
	{
		std::string line;
		std::getline(is, line);
		std::cout << "xxxx111\n";
		//std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "xxxx\n";
		// Pass the result to the handler, via the associated executor.
		dispatch(work.get_executor(),
			[line = std::move(line), handler = std::move(handler)]() mutable
		{
			handler(std::move(line));
		});
	});
}

class AsyncResult {
};

typedef std::function< void(AsyncResult*) > AsyncFunc;

static std::map<int64_t, AsyncFunc> handler_map_;
static std::mutex mutex_;
static std::atomic<int64_t> g_seq_number;

int64_t GetSeqNumber() {
	return g_seq_number.fetch_add(1);
}
static int64_t add_async_op(AsyncFunc func) {
	auto seq = GetSeqNumber();
	std::lock_guard<std::mutex> g(mutex_);
	handler_map_[seq] = std::move(func);
	return seq;
}
static void async_complete(asio::thread_pool::executor_type ctx, int64_t seq_number, AsyncResult* res) {
	mutex_.lock();
	auto it = handler_map_.find(seq_number);
	if (it == handler_map_.end()) {
		mutex_.unlock();
		return;
	}
	auto&& handlerInfo = std::move(it->second);
	handler_map_.erase(it);
	mutex_.unlock();
	
	dispatch(ctx, [=, handler=std::move(handlerInfo)] {
			handler(res);
		});
}

template <class Handler>
void async_getline2(std::istream& is, Handler handler)
{
	// Create executor_work for the handler's associated executor.
	auto work = make_work_guard(handler);

	// Post a function object to do the work asynchronously.
	post([&is, work, handler = std::move(handler)]() mutable
	{
		std::string line;
		std::getline(is, line);
		std::cout << "xxxx111\n";
		//std::this_thread::sleep_for(std::chrono::seconds(5));
		std::cout << "xxxx\n";
		// Pass the result to the handler, via the associated executor.
		dispatch(work.get_executor(),
			[line = std::move(line), handler = std::move(handler)]() mutable
		{
			handler(std::move(line));
		});
	});
}

void test_async()
{
	thread_pool pool(1);

	std::cout << "Enter a line: ";
	
	auto seq = add_async_op([](AsyncResult* res) {
		std::cout << "finish\n";
		});
	
	async_complete(std::move(pool.get_executor()), seq, nullptr);
	std::cout << "finish11\n";
	pool.join();
}
