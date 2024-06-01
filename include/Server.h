#pragma once

#include <thread>

#include <Net.h>

namespace chat {
class Server {
public:

	Server() = default;
	~Server();

	// void configurationForm();

	static bool acceptCondition(
		const void* acceptanceData,
		int acceptanceDataLen,
		void* _this,
		void* responseBuf,
		int responseBufLen
	);

	static void acceptingLoop(std::stop_token stopToken, Server* _this);
	static void clientLoop(std::stop_token stopToken, Server* _this, size_t iClient);

	static void broadcast(
		std::stop_token stopToken,
		arch::net::IPv4 ip,
		arch::net::Socket::Port port,
		const std::string& message
	);

	void setPort(arch::net::Socket::Port port);
	void setPassword(const std::string& password);
	void setMessage(const std::string& message);

	void start();

private:
	arch::net::async::TCPSocket _listenSocket;
	arch::net::async::Host _localhost = arch::net::async::Host::localhost();
	std::vector<arch::net::async::TCPSocket*> _clientSockets;
	std::vector<std::string> _clientNicks;
	std::vector<std::jthread> _workerThreads;
	arch::net::Socket::Port _port;
	std::string _password;
	std::string _message;
};
} // namespace chat
