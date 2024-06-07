#include <Server.h>
#include <GetMasks.h>
#include <ncurses.h>
#include <form.h>
#include <FieldLen.h>
#include <cstring>
#include <exception>
#include <ThreadIDStr.hpp>

bool chat::Server::configurationForm() {
	int maxY, maxX;
	getmaxyx(stdscr, maxY, maxX);
	auto formSubwin = derwin(stdscr, maxY, maxX, 0, 0);
	touchwin(stdscr);

	char portPanel[] = "    Port: ";
	char passwordPanel[] = "Password: ";
	char messagePanel[] = " Message: ";
	char startPanel[] = "[Start]";

	FIELD* fields[5]{};
	int firstFieldY = (maxY - 4) / 2;

	const int portMax = 5 + 1;
	const int passwordMax = 50 + 1;
	const int messageMax = 50 + 1;

	const int portWidth = std::min(maxX - 2 - (int)sizeof(portPanel) + 1, portMax);
	const int passwordWidth = std::min(maxX - 2 - (int)sizeof(passwordPanel) + 1, passwordMax);
	const int messageWidth = std::min(maxX - 2 - (int)sizeof(messagePanel) + 1, messageMax);
	const int startWidth = sizeof(startPanel) - 1;

	fields[0] = new_field(1, portWidth, firstFieldY, sizeof(portPanel), portMax - portWidth, 0);
	fields[1] = new_field(1, passwordWidth, firstFieldY + 1, sizeof(passwordPanel), passwordMax - passwordWidth, 0);
	fields[2] = new_field(1, messageWidth, firstFieldY + 2, sizeof(messagePanel), messageMax - messageWidth, 0);
	fields[3] = new_field(1, startWidth, firstFieldY + 3, (maxX - 2 - startWidth) / 2, 0, 0);
	field_opts_off(fields[4], O_EDIT);
	fields[4] = nullptr;

	FORM* form = new_form(fields);

	auto selectField = [&](FIELD* field) {
		set_field_back(field, A_REVERSE);
		form_driver(form, REQ_OVL_MODE); // Ensure the field is updated in display mode
		form_driver(form, REQ_INS_MODE);
		wrefresh(formSubwin);
	};
	auto unselectField = [&](FIELD* field) {
		set_field_back(field, A_NORMAL);
		form_driver(form, REQ_OVL_MODE); // Ensure the field is updated in display mode
		form_driver(form, REQ_INS_MODE);
		wrefresh(formSubwin);
	};

	for (auto&& field : fields) {
		field_opts_off(field, O_AUTOSKIP);
		field_opts_off(field, O_BLANK);
	}
	selectField(fields[0]);

	set_form_win(form, stdscr);
	set_form_sub(form, formSubwin);
	post_form(form);

	box(formSubwin, 0, 0);
	char formTitle[] = " Server configuration ";
	mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, "%s", formTitle);
	wrefresh(formSubwin);

	mvwprintw(formSubwin, firstFieldY, 1, "%s", portPanel);
	mvwprintw(formSubwin, firstFieldY + 1, 1, "%s", passwordPanel);
	mvwprintw(formSubwin, firstFieldY + 2, 1, "%s", messagePanel);
	set_field_buffer(fields[3], 0, startPanel);
	wrefresh(formSubwin);

	auto printError = [&](const std::string& str) {
		mvwprintw(formSubwin, maxY - 2, 1, "%s", str.c_str());
		wrefresh(formSubwin);
	};
	auto resetErrorSpace = [&]() {
		static const auto emptyStr = std::string(maxX - 2, ' ');
		int ch = mvwinch(formSubwin, maxY - 2, 1);
		if ((ch & A_CHARTEXT) != ' ') {
			mvwprintw(formSubwin, maxY - 2, 1, "%s", emptyStr.c_str());
			wrefresh(formSubwin);
		}
	};

	int c;
	while ((c = getch()) != (('\033'))) {
		switch (c) {
			case KEY_UP:
				unselectField(current_field(form));
				form_driver(form, REQ_PREV_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
			case KEY_DOWN:
				unselectField(current_field(form));
				form_driver(form, REQ_NEXT_FIELD);
				form_driver(form, REQ_END_LINE);
				break;
			case KEY_LEFT:
				form_driver(form, REQ_PREV_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(form, REQ_NEXT_CHAR);
				break;
			case '\b':
			case KEY_BACKSPACE:
				if (fieldLen(field_buffer(current_field(form), 0)) != 0) {
					form_driver(form, REQ_DEL_PREV);
					form_driver(form, REQ_VALIDATION);
				}
				break;
			case KEY_DC:
				form_driver(form, REQ_DEL_CHAR);
				break;
			case KEY_ENTER:
			case '\n':
			case '\r':
				if (current_field(form) != fields[3]) {
					unselectField(current_field(form));
					form_driver(form, REQ_NEXT_FIELD);
					form_driver(form, REQ_END_LINE);
				} else {
					form_driver(form, REQ_VALIDATION);
					resetErrorSpace();
					box(formSubwin, 0, 0);
					mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, "%s", formTitle);
					wrefresh(formSubwin);

					char* errPtr;
					int portVal = (int)strtoul(field_buffer(fields[0], 0), &errPtr, 10);
					if (errPtr == field_buffer(fields[0], 0) or portVal == 0) {
						std::string portVal2 = field_buffer(fields[0], 0);
						portVal2.erase(fieldLen(portVal2));
						printError(std::format("Port '{}' is not a valid port", portVal2));
						break;
					} else if (portVal > std::numeric_limits<decltype(this->_port)>::max()) {
						printError(std::format(
							"Port {} > {} (max port number)",
							portVal,
							std::numeric_limits<decltype(this->_port)>::max()
						));
						break;
					}
					this->_port = (decltype(this->_port))portVal;

					std::string password = field_buffer(fields[1], 0);
					password.erase(fieldLen(password));
					this->_password = password;

					this->_message = field_buffer(fields[2], 0);
					this->_message.erase(fieldLen(this->_message));

					unpost_form(form);
					free_form(form);
					for (size_t i = 0; i != 5; ++i) {
						free_field(fields[i]);
					}
					delwin(formSubwin);
					refresh();
					return true;
				}
				break;
			default:
				int fieldSize;
				int fieldWidth;
				int fieldOff;
				field_info(current_field(form), nullptr, &fieldWidth, nullptr, nullptr, &fieldOff, nullptr);
				fieldSize = fieldWidth + fieldOff - 1;
				if (fieldLen(field_buffer(current_field(form), 0)) < fieldSize) {
					form_driver(form, c);
					form_driver(form, REQ_VALIDATION);
				}
				break;
		}
		selectField(current_field(form));
	}

	unpost_form(form);
	free_form(form);
	for (size_t i = 0; i != 5; ++i) {
		free_field(fields[i]);
	}
	delwin(formSubwin);
	refresh();

	return false;
}

void chat::Server::start() {
	if (this->_port == 0) {
		throw std::runtime_error("server is not configured");
	}

	_listenSocket.reuse(true);

	if (not _listenSocket.bind(_port)) {
		arch::Logger::critical("Could not bind listening socket at port {}, {}", _port, netErrno());
		return;
	}
	arch::Logger::info("Bound listening socket on port {}", _port);
	if (not _listenSocket.listen().get()) {
		arch::Logger::critical("Could not set socket to listening mode, {}", netErrno());
		return;
	}
	arch::Logger::info("Put socket into listening mode at port {}", this->_port);

	{ // creating basic worker threads
		auto lock = std::scoped_lock(this->_workerThreadsMutex);

		// accepting thread
		auto newThread = std::make_shared<std::jthread>(std::jthread(acceptingLoop, this));
		auto newThreadID = newThread->get_id();
		_workerThreads.insert({ newThreadID, std::move(newThread) });

		// broadcasting threads
		auto localhost = arch::net::async::Host::localhost();
		localhost.update().wait();
		for (auto&& ip : localhost.ips()) {
			arch::Logger::trace("Using IP: {}", ip.str());
			newThread = std::make_shared<std::jthread>(std::jthread(broadcast, ip, this->_port, this->_message));
			newThreadID = newThread->get_id();
			_workerThreads.insert({ newThreadID, std::move(newThread) });
		}

		// cleaner of client handling threads
		newThread = std::make_shared<std::jthread>(std::jthread(clientThreadCleaner, this));
		newThreadID = newThread->get_id();
		_workerThreads.insert({ newThreadID, std::move(newThread) });
	}
}

void chat::Server::broadcast(
	std::stop_token stopToken,
	arch::net::IPv4 ip,
	arch::net::Socket::Port port,
	const std::string& message
) {
	arch::net::UDPSocket socket;
	socket.broadcastEnabled(true);

	arch::net::IPv4Mask mask;
	try {
		mask = getMasks(ip);
	} catch (...) {
		arch::Logger::error("Could not get mask for address {}, cannot broadcast", ip.str());
		return;
	}

	auto net = arch::net::IPv4Network(ip, mask);
	auto broadcastIP = net.broadcast();

	arch::Logger::info("Broadcasting from {} to {}, at port 13370", ip.str(), broadcastIP.str());

	std::string broadcastData = std::format("{}\033{}\033{}", ip.str(), port, message);
	while (not stopToken.stop_requested()) {
		auto result = socket.sendTo(broadcastIP, 13'370, broadcastData);
		// arch::Logger::info("broadcasting {}:{} ### {}, result: {}", ip.str(), port, stopToken.stop_requested(),
		// result);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	arch::Logger::info("{}:{} ended", __func__, tidStr(std::this_thread::get_id()));
}

bool chat::Server::acceptCondition(
	const void* acceptanceData,
	int acceptanceDataLen,
	void* _this,
	void* responseBuf,
	int responseBufLen
) {
	Server& self = *(Server*)_this;
	// data sent by client
	std::string_view acceptDataStr{ (const char*)acceptanceData };

	// required format "<password>\033<nick>"
	auto delimPos = acceptDataStr.find('\033');
	if (delimPos == std::string::npos) {
		memcpy(responseBuf, "Bad format", sizeof("Bad format"));
		return false;
	}

	// extract password
	std::string_view password = acceptDataStr.substr(0, delimPos);
	if (password != self._password) {
		memcpy(responseBuf, "Invalid password", sizeof("Invalid password"));
		return false;
	}

	// extract nick
	if (acceptDataStr.length() == delimPos) {
		memcpy(responseBuf, "Empty nick", sizeof("Empty nick"));
		return false;
	}

	{ // add client if nick is unique
		auto lock = std::scoped_lock(self._clientStorageMutex);
		std::string_view nick = acceptDataStr.substr(delimPos + 1);
		if (std::find_if(
				self._clientNicks.begin(),
				self._clientNicks.end(),
				[&](const std::shared_ptr<std::string>& ptr) { return *ptr == nick; }
			) != self._clientNicks.end()) {
			memcpy(responseBuf, "Nick already in use", sizeof("Nick already in use"));
			return false;
		}

		self._clientNicks.push_back(std::make_shared<std::string>(nick));
	}

	memcpy(responseBuf, "success", sizeof("success"));
	return true;
}

void chat::Server::acceptingLoop(std::stop_token stopToken, Server* _this) {
	try {
		auto&& self = *_this;

		arch::Logger::info("Listener awaits connections...");

		auto newSock = std::make_shared<arch::net::async::TCPSocket>();
		std::future<bool> acceptFuture = self._listenSocket.condAccept(*newSock, acceptCondition, 128, 32, &self);
		while (not stopToken.stop_requested()) {
			auto result = acceptFuture.wait_for(std::chrono::milliseconds(100));
			if (result == std::future_status::ready) {
				if (acceptFuture.get()) {
					auto lock = std::scoped_lock(self._clientStorageMutex, self._workerThreadsMutex);

					for (auto&& sock : self._clientSockets) {
						sock->send(std::format("'{}' connected", *self._clientNicks.back()));
					}

					self._clientSockets.push_back(newSock);
					newSock = nullptr;
					auto newThread =
						std::make_shared<std::jthread>(std::jthread(clientLoop, _this, self._clientSockets.size() - 1));
					auto newThreadID = newThread->get_id();
					self._workerThreads.insert({ newThreadID, std::move(newThread) });
					arch::Logger::info(
						"Accepted new client: {}({})",
						self._clientSockets.back()->getPeer().str(),
						*self._clientNicks.back()
					);
				} else {
					arch::Logger::info("Rejected connection from {}", newSock->getPeer().str());
					newSock = nullptr;
				}
				newSock = std::make_shared<arch::net::async::TCPSocket>();
				acceptFuture = self._listenSocket.condAccept(*newSock, acceptCondition, 128, 32, &self);
			}
		}
	} catch (arch::Exception& e) {
		if (not stopToken.stop_requested()) {
			arch::Logger::critical(
				"Unexpected error: {}, from {}:{}",
				e.what(),
				e.location().file_name(),
				e.location().line()
			);
		}
	} catch (std::exception& e) {
		arch::Logger::critical("Unexpected error: {}", e.what());
	}
	arch::Logger::info("{}:{} ended", __func__, tidStr(std::this_thread::get_id()));
}

void chat::Server::clientLoop(std::stop_token stopToken, Server* _this, size_t iClient) {
	auto&& self = *_this;

	std::shared_ptr<arch::net::async::TCPSocket> mySock;
	std::shared_ptr<std::string> myNick;
	{
		auto lock = std::scoped_lock(self._clientStorageMutex);
		mySock = self._clientSockets[iClient];
		myNick = self._clientNicks[iClient];
	}

	arch::Logger::info(
		"Started client handling thread {} for client {}({})",
		tidStr(std::this_thread::get_id()),
		mySock->getPeer().str(),
		*myNick
	);

	char message[256]{};

	while (not stopToken.stop_requested() and mySock->connected()) {
		if (mySock->recv(message, sizeof(message), 1'000, false).get()) {
			arch::Logger::info("{}({}): '{}'", mySock->getPeer().str(), *myNick, message);

			{ // send message to clients
				auto lock = std::scoped_lock(self._clientStorageMutex);
				for (auto&& sock : self._clientSockets) {
					sock->send(std::format("'{}': {}", *myNick, message));
				}
			}

			// reset buffer
			std::memset(message, 0, sizeof(message));
		} else if (not mySock->connected()) {
			arch::Logger::info("{}({}) disconnected", mySock->getPeer().str(), *myNick);

			{ // send that client has disconnected
				auto lock = std::scoped_lock(self._clientStorageMutex);
				for (auto&& sock : self._clientSockets) {
					if (sock != mySock) {
						sock->send(std::format("'{}' disconnected", *myNick));
					}
				}

				// cleanup
				self._clientNicks.erase(std::find(self._clientNicks.begin(), self._clientNicks.end(), myNick));
				self._clientSockets.erase(std::find(self._clientSockets.begin(), self._clientSockets.end(), mySock));
			}
			break;
		}
	}
	{ // add this thread to finished queue
		auto lock = std::scoped_lock(self._finishedQueueMutex);
		self._finishedQueue.push(std::this_thread::get_id());
		self._finishedQueueCV.notify_one();
	}
	arch::Logger::info("{}:{} ended", __func__, tidStr(std::this_thread::get_id()));
}

void chat::Server::clientThreadCleaner(std::stop_token stopToken, Server* _this) {
	auto&& self = *_this;

	arch::Logger::info("Started client cleaner thread {}", tidStr(std::this_thread::get_id()));

	auto lock = std::unique_lock(self._finishedQueueMutex);
	while (not stopToken.stop_requested() or not self._finishedQueue.empty()) {
		// wait for some thread to finish
		self._finishedQueueCV.wait(lock, [&]() {
			return stopToken.stop_requested() or not self._finishedQueue.empty();
		});
		while (not stopToken.stop_requested() and not self._finishedQueue.empty()) {
			auto id = self._finishedQueue.front();
			self._finishedQueue.pop();

			auto threadsLock = std::scoped_lock(self._workerThreadsMutex);
			self._workerThreads.erase(id);

			arch::Logger::info("Cleaned worker thread {}", tidStr(id));
		}
	}
	arch::Logger::info("{}:{} ended", __func__, tidStr(std::this_thread::get_id()));
}

chat::Server::~Server() {
	arch::Logger::info("Requesting stop of worker threads");
	{
		auto lock = std::scoped_lock(this->_workerThreadsMutex);
		for (auto&& [threadID, thread] : this->_workerThreads) {
			thread->request_stop();
			arch::Logger::info("Requested stop of worker thread {}", tidStr(threadID));
		}
		this->_finishedQueueCV.notify_one();
	}
	{
		auto lock = std::scoped_lock(this->_clientStorageMutex);
		_listenSocket.close();
		for (auto&& sock : this->_clientSockets) {
			sock->close();
		}
		arch::Logger::info("Closed all sockets");
	}
	{
		auto lock = std::scoped_lock(this->_workerThreadsMutex);
		for (auto&& [threadID, thread] : this->_workerThreads) {
			if (thread->joinable()) {
				thread->join();
				arch::Logger::info("Stopped worker thread {}", tidStr(threadID));
			}
		}
	}
	arch::Logger::info("Stopped all worker threads");
}
