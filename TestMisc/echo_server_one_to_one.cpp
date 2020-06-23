#include <iostream>
#include "asio.hpp"

using namespace std;
using asio::ip::tcp;

//class AsioThreadContext {
//public:
//	AsioThreadContext() :work_guard_(asio::make_work_guard(io_context_)){
//		thread_ = std::make_unique<std::thread>([&] {
//			io_context_.run();
//			}
//		);
//	}
//	~AsioThreadContext() {
//		io_context_.stop();
//		thread_->join();
//	}
//	template<class F>
//	void Enqueue(F&& f) {
//		asio::post(io_context_, f);
//	}
//	asio::io_context& GetContext() {
//		return io_context_;
//	}
//public:
//	asio::io_context io_context_;
//	std::unique_ptr<std::thread> thread_;
//
//	typedef asio::io_context::executor_type ExecutorType;
//	asio::executor_work_guard<ExecutorType> work_guard_;
//};
//class AsioThreadPool {
//public:
//	AsioThreadPool(size_t thread_num) {
//		for (size_t i = 0; i < thread_num; i++) {
//			//thread_pool_.emplace_back(std::make_unique<AsioThreadContext>());
//		}
//	}
//	template<class F>
//	void Enqueue(F&& f, int index) {
//		//thread_pool_[index]->Enqueue(f);
//	}
//	asio::io_context& GetContext(int index) {
//		asio::io_context xxx;
//		return xxx;
//		//return thread_pool_[index]->GetContext();
//	}
//private:
//	//std::vector<std::unique_ptr<AsioThreadContext>> thread_pool_;
//};


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

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
};

asio::io_context  ctx1;
asio::io_context  ctx2;
class server
{
public:
	server(asio::io_context& io_context, short port)
		: acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
	{
		do_accept();
	}

private:
	void do_accept()
	{
		std::cout << "do_accept:thread id:" << std::this_thread::get_id() << endl;
		acceptor_.async_accept(
			[this](std::error_code ec, tcp::socket socket)
			{
				std::cout << "async_accept:thread id:" << std::this_thread::get_id() << endl;
				if (!ec)
				{
					auto sk = new session(std::move(socket));
					asio::post(ctx2, [=]{

						std::cout << "start session:thread id:" << std::this_thread::get_id() << endl;
						std::shared_ptr<session>(sk)->start();
						});
				}

				do_accept();
			});
	}

	tcp::acceptor acceptor_;
};
void TestThreadPoolOne2One(int argc, char* argv[]) {
	typedef asio::io_context::executor_type ExecutorType;
	asio::executor_work_guard<ExecutorType> work_guard_1 = asio::make_work_guard(ctx1);
	asio::executor_work_guard<ExecutorType> work_guard_2 = asio::make_work_guard(ctx2);
	std::thread t1([] {
		std::cout << "t1:thread id:" << std::this_thread::get_id() << endl;
		ctx1.run();
		std::cout << "t1 end\n";
		});
	std::thread t2([] {
		std::cout << "t2:thread id:" << std::this_thread::get_id() << endl;
		ctx2.run();
		std::cout << "t1 end\n";
		});
	server s(ctx1, std::atoi(argv[1]));

	std::this_thread::sleep_for(std::chrono::seconds(100));
	ctx1.stop();
	ctx2.stop();
	t1.join();
	t2.join();
}
