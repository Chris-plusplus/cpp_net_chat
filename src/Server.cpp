#include <Server.h>
#include <GetMasks.h>

void chat::Server::setPort(arch::net::Socket::Port port) {
	_port = port;
}

void chat::Server::setMessage(const std::string& message) {
	_message = message;
}

void chat::Server::setPassword(const std::string& password) {
	_password = password;
}

void chat::Server::broadcast(
	std::stop_token stopToken,
	arch::net::IPv4 ip,
	arch::net::Socket::Port port,
	const std::string& message
) {
	arch::net::UDPSocket socket;
	socket.broadcastEnabled(true);

	try {
		socket.bind(ip, port);
	} catch (arch::Exception& e) {
		arch::Logger::error("Broadcast socket for {}:{} failed to bind", ip.str(), port);
		return;
	}

	arch::net::IPv4Mask mask;
	try {
		mask = getMasks({ ip }).at(0);
	} catch (...) {
		arch::Logger::error("Could not get mask for address {}", ip.str());
		return;
	}

	arch::net::IPv4Network net(ip, mask);
	auto broadcastIP = net.broadcast();

	arch::Logger::info("Broadcasting on {}", broadcastIP.str());

	std::string broadcastData = std::format("{}\033{}\033{}", ip.str(), port, message);
	while (not stopToken.stop_requested()) {
		socket.sendTo(arch::net::Host(ip), broadcastData);
		// arch::Logger::info("broadcasting {}:{} ### {}", ip.str(), port, stopToken.stop_requested());
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	arch::Logger::info("stopped {}", __func__);
}

bool chat::Server::acceptCondition(
	const void* acceptanceData,
	int acceptanceDataLen,
	void* _this,
	void* responseBuf,
	int responseBufLen
) {
	Server& self = *(Server*)_this;
	std::string_view acceptDataStr{ (const char*)acceptanceData };

	auto delimPos = acceptDataStr.find('\033');
	if (delimPos == std::string::npos) {
		memcpy(responseBuf, "badFmt", sizeof("badFmt"));
		return false;
	}

	std::string_view password = acceptDataStr.substr(0, delimPos);
	if (password != self._password) {
		memcpy(responseBuf, "invalidPasswd", sizeof("invalidPasswd"));
		return false;
	}

	if (acceptDataStr.length() == delimPos) {
		memcpy(responseBuf, "emptyNick", sizeof("emptyNick"));
		return false;
	}
	std::string_view nick = acceptDataStr.substr(delimPos + 1);
	if (std::find(self._clientNicks.begin(), self._clientNicks.end(), nick) != self._clientNicks.end()) {
		memcpy(responseBuf, "nickInUse", sizeof("nickInUse"));
		return false;
	}

	self._clientNicks.push_back(std::string(nick));
	arch::Logger::info("push {} -> {}", nick, (void*)&self._clientNicks.back());
	memcpy(responseBuf, "success", sizeof("success"));
	return true;
}

void chat::Server::clientLoop(std::stop_token stopToken, Server* _this, size_t iClient) {
	auto&& self = *_this;

	auto&& mySock = *self._clientSockets[iClient];
	auto myNick = self._clientNicks[iClient];

	arch::Logger::info("get {} -> {}", (void*)&myNick, myNick);

	char message[200]{};

	while (not stopToken.stop_requested()) {
		if (mySock.recv(message, sizeof(message), 16, false).get()) {
			arch::Logger::info("{}({}): '{}'", mySock.getPeer().str(), myNick, message);
			for (auto&& sock : self._clientSockets) {
				sock->send(std::format("{}\033{}", myNick, message));
			}
			std::memset(message, 0, sizeof(message));
		}
	}
	arch::Logger::info("stopped {}", __func__);
}

void chat::Server::acceptingLoop(std::stop_token stopToken, Server* _this) {
	try {
		auto&& self = *_this;

		arch::Logger::info("Listener awaits connections...");

		arch::net::async::TCPSocket* newSock = new arch::net::async::TCPSocket;
		std::future<bool> acceptFuture = self._listenSocket.condAccept(*newSock, acceptCondition, 128, 16, &self);
		while (not stopToken.stop_requested()) {
			auto result = acceptFuture.wait_for(std::chrono::milliseconds(100));
			if (result == std::future_status::ready) {
				if (acceptFuture.get()) {
					self._clientSockets.push_back(newSock);
					newSock = nullptr;
					self._workerThreads.emplace_back(std::jthread(clientLoop, _this, self._clientSockets.size() - 1));
					arch::Logger::info(
						"Accepted new client: {}({})",
						self._clientSockets.back()->getPeer().str(),
						self._clientNicks.back()
					);
				} else {
					arch::Logger::info("Rejected connection");
					delete newSock;
					newSock = nullptr;
				}
				newSock = new arch::net::async::TCPSocket;
				acceptFuture = self._listenSocket.condAccept(*newSock, acceptCondition, 128, 16, &self);
			}
		}
		if (newSock) {
			delete newSock;
		}
	} catch (arch::Exception& e) {
		arch::Logger::critical("{}, {};{}", e.what(), e.location().file_name(), e.location().line());
	}
	arch::Logger::info("stopped {}", __func__);
}

void chat::Server::start() {
	this->_localhost.update().wait();

	if (not _listenSocket.bind(_port)) {
		arch::Logger::critical("Could not bind listening socket at port {}", _port);
		return;
	}
	arch::Logger::info("Bound socket on port {}", _port);
	if (not _listenSocket.listen().get()) {
		arch::Logger::critical("Could not set socket to listening mode");
		return;
	}
	arch::Logger::info("Put socket into listening mode");

	// accepting thread
	_workerThreads.emplace_back(std::jthread(acceptingLoop, this));
	for (auto&& ip : this->_localhost.ips()) {
		_workerThreads.emplace_back(std::jthread(broadcast, ip, this->_port, this->_message));
	}
}

chat::Server::~Server() {
	arch::Logger::info("stopping worker threads...");
	for (auto&& thread : this->_workerThreads) {
		thread.request_stop();
		arch::Logger::info("stopped worker thread");
	}
	arch::Logger::info("stopped all worker threads");
	for (auto&& sock : this->_clientSockets) {
		delete sock;
	}
}
