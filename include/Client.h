#pragma once

#include <optional>

#include <Net.h>

namespace chat {
class Client {
public:

	Client() noexcept = default;

	bool connectionForm();
	void display();
	static bool acceptResponseHandler(const void* response, int, void* retval);

private:

	arch::net::async::TCPSocket _socket;

	arch::net::IPv4 _ip;
	arch::net::Socket::Port _port;
	std::string _password;
	std::string _nick;
};
} // namespace chat

