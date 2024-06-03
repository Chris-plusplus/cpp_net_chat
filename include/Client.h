#pragma once

#include <optional>

#include <Net.h>

namespace chat {
class Client {
public:

	Client() noexcept = default;

	static void recvLoop(
		std::stop_token stopToken,
		Client* _this,
		int* topLine,
		int maxX,
		int maxY,
		std::vector<std::string>* lines,
		std::mutex* linesMutex,
		std::function<void()> refreshWindow
	);

	bool connectionForm();
	void chatForm();
	static bool acceptResponseHandler(const void* response, int, void* retval);

private:

	arch::net::async::TCPSocket _socket;

	arch::net::IPv4 _ip;
	arch::net::Socket::Port _port;
	std::string _password;
	std::string _nick;
};
} // namespace chat

