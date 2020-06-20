//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include "asio.hpp"

using asio::ip::tcp;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 4)
		{
			std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
			return 1;
		}

		asio::io_context io_context;

		tcp::socket s(io_context);
		tcp::resolver resolver(io_context);
		asio::connect(s, resolver.resolve(argv[1], argv[2]));

		std::cout << "Enter message: ";
		char request[max_length];
		std::cin.getline(request, max_length);
		size_t request_length = std::strlen(request);
		
		auto times = atoi(argv[3]);
		auto beginTime = high_resolution_clock::now();
		char reply[max_length];
		for (int i = 0; i < times; i++) {
			asio::write(s, asio::buffer(request, request_length));

			size_t reply_length = asio::read(s,
				asio::buffer(reply, request_length));
		}
		auto timeInterval  = std::chrono::duration_cast<milliseconds>(high_resolution_clock::now() - beginTime);
		std::cout << "cost time " << timeInterval.count() << ",times=" << times << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}