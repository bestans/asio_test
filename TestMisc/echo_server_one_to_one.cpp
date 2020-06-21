#include <iostream>
#include "asio.hpp"

using namespace std;
using asio::ip::tcp;

class AsioThreadContext {
public:
	AsioThreadContext() :work_guard_(asio::make_work_guard(io_context_)){
		thread_ = std::make_unique<std::thread>([&] {
			io_context_.run();
			}
		);
	}
	~AsioThreadContext() {
		io_context_.stop();
		thread_->join();
	}
	template<class F>
	void Enqueue(F&& f) {
		asio::post(io_context_, f);
	}
	asio::io_context& GetContext() {
		return io_context_;
	}
private:
	asio::io_context io_context_;
	std::unique_ptr<std::thread> thread_;

	typedef asio::io_context::executor_type ExecutorType;
	asio::executor_work_guard<ExecutorType> work_guard_;
};
class AsioThreadPool {
public:
	AsioThreadPool(size_t thread_num) {
		for (size_t i = 0; i < thread_num; i++) {
			thread_pool_.emplace_back(AsioThreadContext());
		}
	}
	template<class F>
	void Enqueue(F&& f, int index) {
		thread_pool_[index].Enqueue(f);
	}
	asio::io_context& GetContext(int index) {
		return thread_pool_[index].GetContext();
	}
private:
	std::vector<AsioThreadContext> thread_pool_;
};


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

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
};

class server
{
public:
	server(asio::io_context& io_context, short port, AsioThreadPool& pool)
		: acceptor_(pool.GetContext(0), tcp::endpoint(tcp::v4(), port)), pool_(pool)
	{
		pool_.Enqueue([this] {
			do_accept();
			}, 0);
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(
			[this](std::error_code ec, tcp::socket socket)
			{
				if (!ec)
				{
					pool_.Enqueue([sk = std::move(socket)]{
						std::make_shared<session>(sk)->start();
						}, 1);
				}
				do_accept();
			});
	}

	tcp::acceptor acceptor_;
	AsioThreadPool& pool_;
};
void TestThreadPoolOne2One() {
	AsioThreadPool pool(2);

	server s(io_context, std::atoi(argv[1]));
}
