#pragma once

#include <thread>
#include <queue>
#include <Net.h>

namespace chat {

/// @brief Chat server class. Call configurationForm() and if it succeeds, call run()
class Server {
public:

	/// @brief Defaulted constructor
	Server() = default;
	/// @brief Destructor. Cancels all worker threads and releases all resources
	~Server();

	/// @brief Displays configuration form of server
	/// @return true if succeeded to configure server
	bool configurationForm();

	/// @brief Starts server
	/// @throws std::exception - when server was started without or with bad configuration
	void start();

private:

	// accepting function for TCPSocket::condAccept
	static bool acceptCondition(
		const void* acceptanceData,
		int acceptanceDataLen,
		void* _this,
		void* responseBuf,
		int responseBufLen
	);

	// function accepting new clients
	static void acceptingLoop(std::stop_token stopToken, Server* _this);

	// function handling assigned client
	static void clientLoop(std::stop_token stopToken, Server* _this, size_t iClient);

	// function cleaing after client handling threads
	static void clientThreadCleaner(std::stop_token stopToken, Server* _this);

	// function broadcasting server's IP and port
	static void broadcast(
		std::stop_token stopToken,
		arch::net::IPv4 ip,
		arch::net::Socket::Port port,
		const std::string& message
	);

	arch::net::async::TCPSocket _listenSocket;
	std::mutex _clientStorageMutex; // mutex for client sockets and nicks
	std::vector<std::shared_ptr<arch::net::async::TCPSocket>> _clientSockets;
	std::vector<std::shared_ptr<std::string>> _clientNicks;
	std::mutex _workerThreadsMutex; // mutex for worker threads map
	std::unordered_map<std::jthread::id, std::shared_ptr<std::jthread>> _workerThreads;
	std::mutex _finishedQueueMutex; // mutex for finished queue and condition variable
	std::queue<std::jthread::id> _finishedQueue;
	std::condition_variable _finishedQueueCV;
	arch::net::Socket::Port _port = 0;
	std::string _password;
	std::string _message;
};
} // namespace chat
