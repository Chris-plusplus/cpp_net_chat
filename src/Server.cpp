#include <Server.h>
#include <GetMasks.h>
#include <ncurses.h>
#include <form.h>
#include <FieldLen.h>

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

	arch::Logger::info("Broadcasting from {} to {}", ip.str(), broadcastIP.str());

	std::string broadcastData = std::format("{}\033{}\033{}", ip.str(), port, message);
	while (not stopToken.stop_requested()) {
		socket.sendTo(arch::net::Host(ip), 13'370, broadcastData);
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

	{
		auto lock = std::scoped_lock(self._clientStorageMutex);
		std::string_view nick = acceptDataStr.substr(delimPos + 1);
		if (std::find_if(
				self._clientNicks.begin(),
				self._clientNicks.end(),
				[&](const std::shared_ptr<std::string>& ptr) { return *ptr == nick; }
			) != self._clientNicks.end()) {
			memcpy(responseBuf, "nickInUse", sizeof("nickInUse"));
			return false;
		}

		self._clientNicks.push_back(std::make_shared<std::string>(nick));
		arch::Logger::info("push {} -> {}", nick, (void*)self._clientNicks.back().get());
	}

	memcpy(responseBuf, "success", sizeof("success"));
	return true;
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

	arch::Logger::info("get {} -> {}", (void*)&myNick, *myNick);

	char message[256]{};

	while (not stopToken.stop_requested() and mySock->connected()) {
		if (mySock->recv(message, sizeof(message)).get()) {
			arch::Logger::info("{}({}): '{}'", mySock->getPeer().str(), *myNick, message);

			{
				auto lock = std::scoped_lock(self._clientStorageMutex);
				for (auto&& sock : self._clientSockets) {
					sock->send(std::format("'{}': {}", *myNick, message));
				}
			}

			std::memset(message, 0, sizeof(message));
		} else if (not mySock->connected()) {
			arch::Logger::info("{}({}) disconnected", mySock->getPeer().str(), *myNick);

			{
				auto lock = std::scoped_lock(self._clientStorageMutex);
				for (auto&& sock : self._clientSockets) {
					if (sock != mySock) {
						sock->send(std::format("'{}' disconnected", *myNick));
					}
				}
				self._clientNicks.erase(std::find(self._clientNicks.begin(), self._clientNicks.end(), myNick));
				self._clientSockets.erase(std::find(self._clientSockets.begin(), self._clientSockets.end(), mySock));
			}
			break;
		}
	}
	arch::Logger::info("stopped {}", __func__);
}

void chat::Server::clientThreadCleaner(std::stop_token stopToken, Server* _this, std::shared_ptr<std::jthread> thread) {
	auto&& self = *_this;
	auto id = thread->get_id();
	thread->join();
	if (not stopToken.stop_requested()) {
		auto lock = std::scoped_lock(self._workerThreadsMutex);
		std::erase(self._workerThreads, thread);
	}
	std::stringstream ss;
	ss << id;
	arch::Logger::info("freed client handling thread {}", ss.str());
}

void chat::Server::acceptingLoop(std::stop_token stopToken, Server* _this) {
	try {
		auto&& self = *_this;

		arch::Logger::info("Listener awaits connections...");

		auto newSock = std::make_shared<arch::net::async::TCPSocket>();
		std::future<bool> acceptFuture = self._listenSocket.condAccept(*newSock, acceptCondition, 128, 16, &self);
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
					self._workerThreads.emplace_back(
						std::make_shared<std::jthread>(std::jthread(clientLoop, _this, self._clientSockets.size() - 1))
					);
					self._workerThreads.emplace_back(std::make_shared<std::jthread>(
						std::jthread(clientThreadCleaner, _this, self._workerThreads.back())
					));
					arch::Logger::info(
						"Accepted new client: {}({})",
						self._clientSockets.back()->getPeer().str(),
						*self._clientNicks.back()
					);
				} else {
					arch::Logger::info("Rejected connection");
					newSock = nullptr;
				}
				newSock = std::make_shared<arch::net::async::TCPSocket>();
				acceptFuture = self._listenSocket.condAccept(*newSock, acceptCondition, 128, 16, &self);
			}
		}
	} catch (arch::Exception& e) {
		if (not stopToken.stop_requested()) {
			arch::Logger::critical("{}, {};{}", e.what(), e.location().file_name(), e.location().line());
		}
	} catch (std::exception& e) {
		arch::Logger::critical("xd: {}", e.what());
	}
	arch::Logger::info("stopped {}", __func__);
}

void chat::Server::start() {
	this->_localhost.update().wait();

	if (not _listenSocket.bind(_port)) {
		arch::Logger::critical("Could not bind listening socket at port {}", _port);
		return;
	}
	arch::Logger::info("Bound listening socket on port {}", _port);
	if (not _listenSocket.listen().get()) {
		arch::Logger::critical("Could not set socket to listening mode");
		return;
	}
	arch::Logger::info("Put socket into listening mode");

	// accepting thread
	{
		auto lock = std::scoped_lock(this->_workerThreadsMutex);
		_workerThreads.emplace_back(std::make_shared<std::jthread>(std::jthread(acceptingLoop, this)));
		for (auto&& ip : this->_localhost.ips()) {
			_workerThreads.emplace_back(
				std::make_shared<std::jthread>(std::jthread(broadcast, ip, this->_port, this->_message))
			);
		}
	}
}

chat::Server::~Server() {
	arch::Logger::info("requesting stop of worker threads...");
	{
		auto lock = std::scoped_lock(this->_workerThreadsMutex);
		for (auto&& thread : this->_workerThreads) {
			std::stringstream ss;
			ss << thread->get_id();
			thread->request_stop();
			arch::Logger::info("requested stop of worker thread {}", ss.str());
		}
	}
	{
		auto lock = std::scoped_lock(this->_clientStorageMutex);
		_listenSocket.close();
		for (auto&& sock : this->_clientSockets) {
			sock->close();
		}
	}
	{
		auto lock = std::scoped_lock(this->_workerThreadsMutex);
		for (auto&& thread : this->_workerThreads) {
			if (thread->joinable()) {
				auto id = thread->get_id();
				thread->join();
				std::stringstream ss;
				ss << id;
				arch::Logger::info("stopped worker thread {}", ss.str());
			}
		}
	}
	arch::Logger::info("stopped all worker threads");
}

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

	mvwprintw(formSubwin, firstFieldY, 1, portPanel);
	mvwprintw(formSubwin, firstFieldY + 1, 1, passwordPanel);
	mvwprintw(formSubwin, firstFieldY + 2, 1, messagePanel);
	set_field_buffer(fields[3], 0, startPanel);
	wrefresh(formSubwin);

	auto printError = [&](const std::string& str) {
		mvwprintw(formSubwin, maxY - 2, 1, str.c_str());
		wrefresh(formSubwin);
	};
	auto resetErrorSpace = [&]() {
		static const auto emptyStr = std::string(maxX - 2, ' ');
		int ch = mvwinch(formSubwin, maxY - 2, 1);
		if ((ch & A_CHARTEXT) != ' ') {
			mvwprintw(formSubwin, maxY - 2, 1, emptyStr.c_str());
			wrefresh(formSubwin);
		}
	};

	std::vector<std::string> inputs;

	int c;
	while ((c = getch()) != '\033') {
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
	printw("%zu\n", inputs.size());
	for (auto&& o : inputs) {
		printw("'%s'\n", o.c_str());
	}
	refresh();

	return false;
}
