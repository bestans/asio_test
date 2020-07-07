#pragma once
#include <vector>
#include <thread>
#include "stat_balance.h"

namespace asio {
	class io_context;
}

namespace btd {
	class AsioThreadContext
	{
	public:
		AsioThreadContext();
		asio::io_context& GetContext() {
			return *ctx_;
		}
		void Stop();
	private:
		void run();

	private:
		std::thread thread_;
		std::shared_ptr<asio::io_context> ctx_;
	};
}
