#pragma once

#include <thread>
#include <queue>

#include <Net.h>

namespace chat {
class Server {
public:

	Server() = default;
	~Server();

	static bool acceptCondition(
		const void* acceptanceData,
		int acceptanceDataLen,
		void* _this,
		void* responseBuf,
		int responseBufLen
	);

	static void acceptingLoop(std::stop_token stopToken, Server* _this);
	static void clientLoop(std::stop_token stopToken, Server* _this, size_t iClient);
	static void clientThreadCleaner(std::stop_token stopToken, Server* _this, std::shared_ptr<std::jthread> thread);

	static void broadcast(
		std::stop_token stopToken,
		arch::net::IPv4 ip,
		arch::net::Socket::Port port,
		const std::string& message
	);

	bool configurationForm();

	void start();

private:
	arch::net::async::TCPSocket _listenSocket;
	arch::net::async::Host _localhost = arch::net::async::Host::localhost();
	std::mutex _clientStorageMutex;
	std::vector<std::shared_ptr<arch::net::async::TCPSocket>> _clientSockets;
	std::vector<std::shared_ptr<std::string>> _clientNicks;
	std::mutex _workerThreadsMutex;
	std::vector<std::shared_ptr<std::jthread>> _workerThreads;
	arch::net::Socket::Port _port;
	std::string _password;
	std::string _message;
};
} // namespace chat
