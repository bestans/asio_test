#include "thread_pool.h"
#include "asio.hpp"

namespace btd {
	AsioThreadContext::AsioThreadContext() :
		ctx_(std::make_shared<asio::io_context>()),
		thread_(&AsioThreadContext::run, this)
	{
	}
	void AsioThreadContext::Stop()
	{
		ctx_->stop();
		thread_.join();
	}
	void AsioThreadContext::run()
	{
		typedef asio::io_context::executor_type ExecutorType;
		asio::executor_work_guard<ExecutorType> work_guard_1 = asio::make_work_guard(*ctx_);
		ctx_->run();
	}
}
