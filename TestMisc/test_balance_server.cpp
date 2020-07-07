#include <iostream>
#include "asio.hpp"
#include "stat_balance.h"
#include "thread_pool.h"
#include <map>
#include <unordered_map>
#include "sbuffer.h"
#include <shared_mutex>

using namespace std;
using asio::ip::tcp;

namespace test_balance {
	class session
		: public std::enable_shared_from_this<session>
	{
	public:
		session(tcp::socket socket)
			: socket_(std::move(socket))
		{
		}

		void start()
		{
			do_read();
		}

	private:
		void do_read()
		{
			std::cout << "do_read:thread id:" << std::this_thread::get_id() << endl;
			auto self(shared_from_this());
			socket_.async_read_some(asio::buffer(data_, max_length),
				[this, self](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						do_write(length);
					}
				});
		}

		void do_write(std::size_t length)
		{
			std::cout << "do_write:thread id:" << std::this_thread::get_id() << endl;
			auto self(shared_from_this());
			asio::async_write(socket_, asio::buffer(data_, length),
				[this, self](std::error_code ec, std::size_t /*length*/)
				{
					if (!ec)
					{
						do_read();
					}
				});
		}

		void do_write(SBuffer& buffer)
		{
			std::cout << "do_write:thread id:" << std::this_thread::get_id() << endl;
			auto self(shared_from_this());
			asio::async_write(socket_, asio::buffer(buffer_.Data(), buffer_.Size()),
				[this, self](std::error_code ec, std::size_t /*length*/)
				{
					if (!ec)
					{
						do_read();
					}
				});
		}

		tcp::socket socket_;
		enum { max_length = 1024 };
		char data_[max_length];
		SBuffer buffer_;
		std::mutex mutex_;
		SBufferMap bufer_map_;
	};

	asio::io_context  ctx1;
	asio::io_context  ctx2;
	class server
	{
	public:
		server(BalanceContextPool<btd::AsioThreadContext>& pool, std::pair<std::shared_ptr<btd::AsioThreadContext>, int>&& alloc_ctx_, short port)
			: pool_(pool), acceptor_(alloc_ctx_.first->GetContext(), tcp::endpoint(tcp::v4(), port))
		{
			do_accept();
		}

	private:
		void do_accept()
		{
			std::cout << "do_accept:thread id:" << std::this_thread::get_id() << endl;
			auto next_ctx = pool_.AllocContext(10);
			auto& ctx = next_ctx.first->GetContext();
			acceptor_.async_accept(ctx, [this, &ctx, ctx_pair=std::move(next_ctx)](std::error_code ec, tcp::socket socket)
				{
					std::cout << "async_accept:thread id:" << std::this_thread::get_id() << endl;
					if (!ec)
					{
						auto sk = new session(std::move(socket));
						asio::post(ctx, [=] {
							std::cout << "start session:thread id:" << std::this_thread::get_id() << endl;
							std::shared_ptr<session>(sk)->start();
							});
					}
					do_accept();
				});
		}

		tcp::acceptor acceptor_;
		BalanceContextPool<btd::AsioThreadContext>& pool_;
		std::pair<std::shared_ptr<btd::AsioThreadContext>, int> alloc_ctx_;
	};
	void TestThreadPoolOne2One(int argc, char* argv[]) {
		BalanceContextPool<btd::AsioThreadContext> pool(3);
		server s(pool, pool.AllocContext(10), std::atoi(argv[1]));

		std::this_thread::sleep_for(std::chrono::seconds(100));
		for (auto& it : pool.GetContextList()) {
			it->Stop();
		}
	}
}