#include "test_misc.h"
#include "asio.hpp"

using namespace std;

using asio::ip::tcp;

class TCPConnection : public std::enable_shared_from_this<TCPConnection>
{
public:
	TCPConnection(asio::io_context& io_context)
		: socket_(io_context),
		strand_(io_context)
	{ }

	tcp::socket& socket() { return socket_; }
	void start() { doRead(); }

private:
	void doRead()
	{
		auto self = shared_from_this();
		socket_.async_read_some(
			asio::buffer(buffer_, buffer_.size()),
			strand_.wrap([this, self](asio::error_code ec,
				std::size_t bytes_transferred)
				{
					if (!ec) { doWrite(bytes_transferred); }
				}));
	}
	void doWrite(std::size_t length)
	{
		auto self = shared_from_this();
		asio::async_write(
			socket_, asio::buffer(buffer_, length),
			strand_.wrap([this, self](asio::error_code ec,
				std::size_t /* bytes_transferred */)
				{
					if (!ec) { doRead(); }
				}));
	}
private:
	tcp::socket socket_;
	asio::io_context::strand strand_;
	std::array<char, 8192> buffer_;
};
class EchoServer
{
public:
	EchoServer(asio::io_context& io_context, unsigned short port)
		: io_context_(io_context),
		acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
	{
		doAccept();
	}
	void doAccept()
	{
		auto conn = std::make_shared<TCPConnection>(io_context_);
		acceptor_.async_accept(conn->socket(),
			[this, conn](asio::error_code ec)
			{
				if (!ec) { conn->start(); }
				this->doAccept();
			});
	}

private:
	asio::io_context& io_context_;
	tcp::acceptor acceptor_;
};

void TestEchoServerMultiple(int argc, char* argv[]) {

	asio::io_context io_context;
	vector<thread> pool;
	pool.emplace_back([&] {
		io_context.run();
		});
	auto guard = asio::make_work_guard(io_context);
	EchoServer s(io_context, std::atoi(argv[1]));
	for (auto& t : pool) {
		t.join();
	}
}