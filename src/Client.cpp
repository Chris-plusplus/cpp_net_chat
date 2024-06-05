#include <algorithm>
#include <iostream>
#include <set>
#include <tuple>
#include <cstring>
#include <fstream>
#include <Client.h>
#include <FieldLen.h>
#include <form.h>
#include <ncurses.h>
#include <ThreadIDStr.hpp>

bool chat::Client::acceptResponseHandler(const void* response, int, void* retval) {
	auto&& out = *(std::string*)retval;
	out = std::string((const char*)response);
	return out == "success";
}

int chat::Client::connectionTypeForm(const std::string& redoMsg) {
	curs_set(0);

	int maxY, maxX;
	getmaxyx(stdscr, maxY, maxX);
	auto formSubwin = derwin(stdscr, maxY, maxX, 0, 0);
	touchwin(stdscr);

	auto printError = [&](const std::string& str) {
		mvwprintw(formSubwin, maxY - 2, 1, "%s", str.c_str());
		wrefresh(formSubwin);
	};

	char manualPanel[] = "[Manual]";
	char listPanel[] = "[List]";

	FIELD* fields[3]{};
	int firstFieldY = (maxY - 2) / 2;

	const int manualWidth = sizeof(manualPanel) - 1;
	const int listWidth = sizeof(listPanel) - 1;

	fields[0] = new_field(1, manualWidth, firstFieldY, (maxX - 2 - manualWidth) / 2, 0, 0);
	field_opts_off(fields[0], O_EDIT);
	fields[1] = new_field(1, listWidth, firstFieldY + 1, (maxX - 2 - listWidth) / 2, 0, 0);
	field_opts_off(fields[1], O_EDIT);
	fields[2] = nullptr;

	FORM* form = new_form(fields);

	auto selectField = [&](FIELD* field) {
		set_field_back(field, A_REVERSE);
		form_driver(form, REQ_OVL_MODE);
		form_driver(form, REQ_INS_MODE);
		wrefresh(formSubwin);
	};
	auto unselectField = [&](FIELD* field) {
		set_field_back(field, A_NORMAL);
		form_driver(form, REQ_OVL_MODE);
		form_driver(form, REQ_INS_MODE);
		wrefresh(formSubwin);
	};

	selectField(fields[0]);

	set_form_win(form, stdscr);
	set_form_sub(form, formSubwin);
	post_form(form);

	box(formSubwin, 0, 0);
	char formTitle[] = " Select connection type ";
	mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, "%s", formTitle);
	printError(redoMsg);
	wrefresh(formSubwin);

	set_field_buffer(fields[0], 0, manualPanel);
	set_field_buffer(fields[1], 0, listPanel);
	wrefresh(formSubwin);

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
			case KEY_ENTER:
			case '\n':
			case '\r':
				if (current_field(form) == fields[0]) {
					unpost_form(form);
					for (size_t i = 0; i != 2; ++i) {
						free_field(fields[i]);
					}
					delwin(formSubwin);
					refresh();

					// [Manual]
					return 1;
				} else {
					unpost_form(form);
					for (size_t i = 0; i != 2; ++i) {
						free_field(fields[i]);
					}
					delwin(formSubwin);
					refresh();

					// [List]
					return 2;
				}
		}
		selectField(current_field(form));
	}

	// redo
	return -1;
}

int chat::Client::manualConnectionForm(std::string& redoMsg) {
	int maxY, maxX;
	getmaxyx(stdscr, maxY, maxX);
	auto formSubwin = derwin(stdscr, maxY, maxX, 0, 0);
	touchwin(stdscr);

	char hostnamePanel[] = "      IP: ";
	char portPanel[] = "    Port: ";
	char passwordPanel[] = "Password: ";
	char nickPanel[] = "    Nick: ";
	char connectPanel[] = "[Connect]";

	FIELD* fields[6]{};
	int firstFieldY = (maxY - 4) / 2;

	// + 1 to let cursor be at end of line
	const int hostnameMax = 15 + 1;
	const int portMax = 5 + 1;
	const int passwordMax = 50 + 1;
	const int nickMax = 25 + 1;

	const int hostnameWidth = std::min(maxX - 2 - (int)sizeof(hostnamePanel) + 1, hostnameMax);
	const int portWidth = std::min(maxX - 2 - (int)sizeof(portPanel) + 1, portMax);
	const int passwordWidth = std::min(maxX - 2 - (int)sizeof(passwordPanel) + 1, passwordMax);
	const int nickWidth = std::min(maxX - 2 - (int)sizeof(nickPanel) + 1, nickMax);
	const int connectWidth = sizeof(connectPanel) - 1;

	fields[0] = new_field(1, hostnameWidth, firstFieldY, sizeof(hostnamePanel), hostnameMax - hostnameWidth, 0);
	fields[1] = new_field(1, portWidth, firstFieldY + 1, sizeof(portPanel), portMax - portWidth, 0);
	fields[2] = new_field(1, passwordWidth, firstFieldY + 2, sizeof(passwordPanel), passwordMax - passwordWidth, 0);
	fields[3] = new_field(1, nickWidth, firstFieldY + 3, sizeof(nickPanel), nickMax - nickWidth, 0);
	fields[4] = new_field(1, connectWidth, firstFieldY + 4, (maxX - 2 - connectWidth) / 2, 0, 0);
	field_opts_off(fields[4], O_EDIT);
	fields[5] = nullptr;

	FORM* form = new_form(fields);

	auto selectField = [&](FIELD* field) {
		set_field_back(field, A_REVERSE);
		form_driver(form, REQ_OVL_MODE);
		form_driver(form, REQ_INS_MODE);
		wrefresh(formSubwin);
	};
	auto unselectField = [&](FIELD* field) {
		set_field_back(field, A_NORMAL);
		form_driver(form, REQ_OVL_MODE);
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
	char formTitle[] = " Connect to server ";
	mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, "%s", formTitle);
	wrefresh(formSubwin);

	mvwprintw(formSubwin, firstFieldY, 1, "%s", hostnamePanel);
	mvwprintw(formSubwin, firstFieldY + 1, 1, "%s", portPanel);
	mvwprintw(formSubwin, firstFieldY + 2, 1, "%s", passwordPanel);
	mvwprintw(formSubwin, firstFieldY + 3, 1, "%s", nickPanel);
	set_field_buffer(fields[4], 0, connectPanel);
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
				if (current_field(form) != fields[4]) {
					unselectField(current_field(form));
					form_driver(form, REQ_NEXT_FIELD);
					form_driver(form, REQ_END_LINE);
				} else {
					form_driver(form, REQ_VALIDATION);
					resetErrorSpace();
					box(formSubwin, 0, 0);
					mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, "%s", formTitle);
					wrefresh(formSubwin);

					std::string tempIP = field_buffer(fields[0], 0);
					tempIP.erase(fieldLen(tempIP));
					this->_ip = arch::net::IPv4(tempIP);
					if (this->_ip.str() != tempIP) {
						printError(std::format("IP '{}' is not a valid IP address", tempIP));
						break;
					}

					char* errPtr;
					int portVal = (int)strtoul(field_buffer(fields[1], 0), &errPtr, 10);
					if (errPtr == field_buffer(fields[1], 0)) {
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

					this->_password = field_buffer(fields[2], 0);
					set_field_buffer(fields[2], 0, "");
					form_driver(form, REQ_VALIDATION);
					wrefresh(formSubwin);
					this->_password.erase(fieldLen(this->_password));
					this->_nick = field_buffer(fields[3], 0);
					this->_nick.erase(fieldLen(this->_nick));

					if (this->_nick.empty()) {
						printError(std::format("Error: empty nick"));
						break;
					}

					std::string connectionData;
					connectionData += this->_password;
					connectionData += '\033';
					connectionData += this->_nick;

					std::string response;

					auto connectFuture = this->_socket.condConnect(
						{ this->_ip },
						this->_port,
						connectionData.c_str(),
						connectionData.length(),
						255,
						acceptResponseHandler,
						&response
					);
					for (auto&& c : this->_password) {
						c = '\0';
					}
					this->_password.clear();
					try {
						// unreliable, might be useful in the future
						/*if (connectFuture.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
							this->_socket.reset();
							std::thread([fut = std::move(connectFuture)]() {
								try {
								} catch (...) {}
							}).detach();
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							printError(std::format("Connecting to server timed out"));
						} else */
						if (not connectFuture.get()) {
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							printError(std::format("Error connecting to server: '{}'", response));
						} else {
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							unpost_form(form);
							for (size_t i = 0; i != 5; ++i) {
								free_field(fields[i]);
							}
							delwin(formSubwin);
							refresh();

							// success
							return 1;
						}
						break;
					} catch (arch::Exception& e) {
						if (response.empty()) {
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							printError(std::format(
								"Unexpected {}: {}, at {}:{}",
								__LINE__,
								e.what(),
								e.location().file_name(),
								e.location().line()
							));
						} else {
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							printError(std::format(
								"Unexpected {}: '{}', {} at {}:{}",
								__LINE__,
								response,
								e.what(),
								e.location().file_name(),
								e.location().line()
							));
						}
						break;
					}
					break;
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

	// redo
	return -1;
}

int chat::Client::listSelectionConnectionForm(std::string& redoMsg) {
	curs_set(0);
	int maxY, maxX;
	getmaxyx(stdscr, maxY, maxX);
	auto formSubwin = derwin(stdscr, maxY, maxX, 0, 0);
	touchwin(stdscr);

	box(formSubwin, 0, 0);
	char searchingFormTitle[] = " Searching... ";
	char foundFormTitle[] = " Select server from list ";
	mvwprintw(formSubwin, 0, (maxX - (sizeof(searchingFormTitle) + 1)) / 2, "%s", searchingFormTitle);
	wrefresh(formSubwin);

	auto parseMessage = [](const std::string& message) -> std::tuple<std::string, arch::net::IPv4, uint16_t> {
		try {
			char ip[16]{};
			char msg[64]{};
			uint16_t port = 0;

			size_t ipEnd = message.find('\033');
			if (ipEnd != std::string::npos) {
				message.copy(ip, ipEnd);

				size_t portStart = ipEnd + 1;
				size_t portEnd = message.find('\033', portStart);
				if (portEnd != std::string::npos) {
					port = (uint16_t)std::stoi(message.substr(portStart, portEnd - portStart));
					size_t msgStart = portEnd + 1;
					std::string msgStr = message.substr(msgStart);
					msgStr.copy(msg, msgStr.size());
				} else {
					return { "", { "0.0.0.0" }, 0 };
				}
			} else {
				return { "", { "0.0.0.0" }, 0 };
			}

			return { std::format("{}:{} - {}", ip, port, msg), { std::string(ip) }, port };
		} catch (...) {
			return { "", { "0.0.0.0" }, 0 };
		}
	};

	std::vector<std::tuple<std::string, arch::net::IPv4, uint16_t>> servers;

	{
		auto sock = arch::net::async::UDPSocket(13'370);
		sock.broadcastEnabled(true);
		auto timeout = std::chrono::seconds(3);
		for (auto begin = std::chrono::high_resolution_clock::now();
			 std::chrono::high_resolution_clock::now() - begin < timeout;) {
			char recv[256]{};
			auto recvFuture = sock.recv(
				recv,
				sizeof(recv),
				std::chrono::duration_cast<std::chrono::milliseconds>(
					timeout - (std::chrono::high_resolution_clock::now() - begin)
				)
					.count()
			);
			if (recvFuture.get()) {
				auto&& [printMsg, ip, port] = parseMessage(recv);
				if (port == 0) {
					continue;
				}

				bool found = false;
				for (auto&& [printMsg2, ip2, port2] : servers) {
					if (printMsg == printMsg2) {
						found = true;
						break;
					}
				}

				if (not found) {
					mvwprintw(formSubwin, servers.size() + 1, 1, "%s", printMsg.c_str());
					wrefresh(formSubwin);
					servers.push_back({ printMsg, ip, port });
				}
			}
		}
		sock.close();
	}
	if (servers.empty()) {
		redoMsg = "No servers were found";
		delwin(formSubwin);
		refresh();
		return 0;
	}

	auto fields = std::vector<FIELD*>(servers.size() + 1, nullptr);
	for (size_t i = 0; i != servers.size(); ++i) {
		fields[i] = new_field(1, std::get<0>(servers[i]).length(), i + 1, 1, 0, 0);
		field_opts_off(fields[i], O_AUTOSKIP);
		field_opts_off(fields[i], O_EDIT);
		set_field_buffer(fields[i], 0, std::get<0>(servers[i]).c_str());
	}

	FORM* form = new_form(fields.data());
	set_form_win(form, stdscr);
	set_form_sub(form, formSubwin);

	auto selectField = [&](FIELD* field) {
		set_field_back(field, A_REVERSE);
		form_driver(form, REQ_OVL_MODE);
		form_driver(form, REQ_INS_MODE);
		wrefresh(formSubwin);
	};
	auto unselectField = [&](FIELD* field) {
		set_field_back(field, A_NORMAL);
		form_driver(form, REQ_OVL_MODE);
		form_driver(form, REQ_INS_MODE);
		wrefresh(formSubwin);
	};
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

	post_form(form);
	box(formSubwin, 0, 0);
	mvwprintw(formSubwin, 0, (maxX - (sizeof(foundFormTitle) + 1)) / 2, "%s", foundFormTitle);
	wrefresh(formSubwin);

	selectField(current_field(form));

	int c;
	int field = 0;
	while ((c = getch()) != (('\033'))) {
		switch (c) {
			case KEY_UP:
				unselectField(current_field(form));
				form_driver(form, REQ_PREV_FIELD);
				field = (field != 0 ? field - 1 : fields.size() - 2);
				break;
			case KEY_DOWN:
				unselectField(current_field(form));
				form_driver(form, REQ_NEXT_FIELD);
				field = (field + 1) % (fields.size() - 1);
				break;
			case KEY_ENTER:
			case '\n':
			case '\r':
				resetErrorSpace();

				this->_ip = std::get<1>(servers[field]);
				this->_port = std::get<2>(servers[field]);

				unpost_form(form);
				free_form(form);
				for (size_t i = 0; i != fields.size() - 1; ++i) {
					free_field(fields[i]);
				}
				delwin(formSubwin);
				refresh();

				return listCredentialsConnectionForm(redoMsg);
		}
		selectField(current_field(form));
	}

	unpost_form(form);
	free_form(form);
	for (size_t i = 0; i != fields.size() - 1; ++i) {
		free_field(fields[i]);
	}
	delwin(formSubwin);
	refresh();

	return -1;
}

int chat::Client::listCredentialsConnectionForm(std::string& redoMsg) {
	curs_set(1);
	int maxY, maxX;
	getmaxyx(stdscr, maxY, maxX);
	auto formSubwin = derwin(stdscr, maxY, maxX, 0, 0);
	touchwin(stdscr);

	char passwordPanel[] = "Password: ";
	char nickPanel[] = "    Nick: ";
	char connectPanel[] = "[Connect]";

	FIELD* fields[4]{};
	int firstFieldY = (maxY - 4) / 2;

	const int passwordMax = 50 + 1;
	const int nickMax = 50 + 1;

	const int passwordWidth = std::min(maxX - 2 - (int)sizeof(passwordPanel) + 1, passwordMax);
	const int nickWidth = std::min(maxX - 2 - (int)sizeof(nickPanel) + 1, nickMax);
	const int connectWidth = sizeof(connectPanel) - 1;

	fields[0] = new_field(1, passwordWidth, firstFieldY, sizeof(passwordPanel), passwordMax - passwordWidth, 0);
	fields[1] = new_field(1, nickWidth, firstFieldY + 1, sizeof(nickPanel), nickMax - nickWidth, 0);
	fields[2] = new_field(1, connectWidth, firstFieldY + 2, (maxX - 2 - connectWidth) / 2, 0, 0);
	field_opts_off(fields[2], O_EDIT);
	fields[3] = nullptr;

	FORM* form = new_form(fields);

	auto selectField = [&](FIELD* field) {
		set_field_back(field, A_REVERSE);
		form_driver(form, REQ_OVL_MODE);
		form_driver(form, REQ_INS_MODE);
		wrefresh(formSubwin);
	};
	auto unselectField = [&](FIELD* field) {
		set_field_back(field, A_NORMAL);
		form_driver(form, REQ_OVL_MODE);
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
	char formTitle[] = " Connect to server ";
	mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, "%s", formTitle);
	wrefresh(formSubwin);

	mvwprintw(formSubwin, firstFieldY, 1, "%s", passwordPanel);
	mvwprintw(formSubwin, firstFieldY + 1, 1, "%s", nickPanel);
	set_field_buffer(fields[2], 0, connectPanel);
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
				if (current_field(form) != fields[2]) {
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

					this->_password = field_buffer(fields[0], 0);
					set_field_buffer(fields[0], 0, "");
					form_driver(form, REQ_VALIDATION);
					wrefresh(formSubwin);
					this->_password.erase(fieldLen(this->_password));
					this->_nick = field_buffer(fields[1], 0);
					this->_nick.erase(fieldLen(this->_nick));

					if (this->_nick.empty()) {
						printError(std::format("Error: empty nick"));
						break;
					}

					std::string connectionData;
					connectionData += this->_password;
					connectionData += '\033';
					connectionData += this->_nick;

					std::string response;

					auto connectFuture = this->_socket.condConnect(
						{ this->_ip },
						this->_port,
						connectionData.c_str(),
						connectionData.length(),
						255,
						acceptResponseHandler,
						&response
					);
					for (auto&& c : this->_password) {
						c = '\0';
					}
					this->_password.clear();
					try {
						if (not connectFuture.get()) {
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							printError(std::format("Error connecting to server: '{}'", response));
							break;
						} else {
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							unpost_form(form);
							for (size_t i = 0; i != 3; ++i) {
								free_field(fields[i]);
							}
							delwin(formSubwin);
							refresh();
							return 1;
						}
					} catch (arch::Exception& e) {
						if (response.empty()) {
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							printError(std::format(
								"Unexpected {}: {}, at {}:{}",
								__LINE__,
								e.what(),
								e.location().file_name(),
								e.location().line()
							));
						} else {
							for (auto&& c : connectionData) {
								c = '\0';
							}
							connectionData.clear();
							printError(std::format(
								"Unexpected {}: '{}', {} at {}:{}",
								__LINE__,
								response,
								e.what(),
								e.location().file_name(),
								e.location().line()
							));
						}
						break;
					}
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
	for (size_t i = 0; i != 3; ++i) {
		free_field(fields[i]);
	}
	delwin(formSubwin);
	refresh();

	return -1;
}

bool chat::Client::connectionForm() {
	int connectionType = 0;
	std::string redoMsg;

	int result = 0;
	do {
		connectionType = connectionTypeForm(redoMsg);

		curs_set(1);
		if (connectionType == 1) {
			result = manualConnectionForm(redoMsg);
		} else if (connectionType == 2) {
			result = listSelectionConnectionForm(redoMsg);
		} else if (connectionType == -1) {
			break;
		}
	} while (not result);

	curs_set(1);

	return result == 1;
}

void chat::Client::chatForm() {
	if (not this->_socket.connected()) {
		throw std::runtime_error("client not connected");
	}

	int maxY, maxX;
	getmaxyx(stdscr, maxY, maxX);

	WINDOW* recvWin = derwin(stdscr, maxY - 1, maxX, 0, 0);
	touchwin(stdscr);
	wrefresh(recvWin);

	FIELD* fields[2];
	FORM* form;

	fields[0] = new_field(1, maxX, 0, 0, 0, 0);
	set_field_back(fields[0], A_REVERSE);
	field_opts_off(fields[0], O_STATIC);
	curs_set(1);
	fields[1] = nullptr;

	form = new_form(fields);

	WINDOW* formSubwin = derwin(stdscr, 1, maxX, maxY - 1, 0);
	touchwin(stdscr);
	set_form_win(form, stdscr);
	set_form_sub(form, formSubwin);

	post_form(form);
	box(recvWin, 0, 0);
	std::string winTitle = std::format(" {}:{} ", this->_ip.str(), this->_port);
	mvwprintw(recvWin, 0, (maxX - (winTitle.length() + 2)) / 2, "%s", winTitle.c_str());
	wrefresh(recvWin);
	wrefresh(formSubwin);

	std::mutex linesMutex;
	std::vector<std::string> lines;

	int topLine = 0;
	int ch;

	auto refreshWindow = [&]() {
		werase(recvWin);
		box(recvWin, 0, 0);
		mvwprintw(recvWin, 0, (maxX - (winTitle.length() + 2)) / 2, "%s", winTitle.c_str());
		{
			auto lock = std::scoped_lock(linesMutex);
			for (int i = 0; i < maxY - 3; ++i) {
				if (topLine + i < lines.size()) {
					mvwprintw(recvWin, i + 1, 1, "%s", lines[topLine + i].c_str());
				}
			}
		}
		wrefresh(recvWin);
		wrefresh(formSubwin);
	};

	auto receiverThread = std::jthread(recvLoop, this, &topLine, maxX, maxY, &lines, &linesMutex, refreshWindow);

	refreshWindow();

	while ((ch = getch()) != (('\033'))) {
		switch (ch) {
			case KEY_UP:
				if (topLine > 0) {
					topLine--;
					refreshWindow();
				}
				break;
			case KEY_DOWN:
				if (topLine + maxY - 3 < lines.size()) {
					topLine++;
					refreshWindow();
				}
				break;
			case KEY_LEFT:
				form_driver(form, REQ_PREV_CHAR);
				break;
			case KEY_RIGHT:
				form_driver(form, REQ_NEXT_CHAR);
				break;
			case '\b':
			case KEY_BACKSPACE:
				form_driver(form, REQ_DEL_PREV);
				break;
			case KEY_DC:
				form_driver(form, REQ_DEL_CHAR);
				break;
			case '\n':
			case '\r':
			case KEY_ENTER:
				{
					form_driver(form, REQ_VALIDATION);
					std::string val = field_buffer(fields[0], 0);
					val.erase(fieldLen(val));
					this->_socket.send(val);
					form_driver(form, REQ_CLR_FIELD);
				}
				break;
			default:
				form_driver(form, REQ_VALIDATION);
				if (fieldLen(field_buffer(fields[0], 0)) < 100) {
					form_driver(form, ch);
					wrefresh(formSubwin);
				}
				break;
		}
	}

	refresh();

	receiverThread.request_stop();
	this->_socket.close();
	receiverThread.join();

	unpost_form(form);
	free_form(form);
	free_field(fields[0]);
	delwin(formSubwin);
	delwin(recvWin);
	refresh();
}

void chat::Client::recvLoop(
	std::stop_token stopToken,
	Client* _this,
	int* topLine,
	int maxX,
	int maxY,
	std::vector<std::string>* lines,
	std::mutex* linesMutex,
	std::function<void()> refreshWindow
) {
	auto&& self = *_this;
	char buf[256]{};
	while (not stopToken.stop_requested()) {
		try {
			int recvLen;
			auto recvFuture = self._socket.recv(buf, sizeof(buf) - 1, recvLen);
			if (recvFuture.get()) {
				if (recvLen != 0) {
					auto lock = std::scoped_lock(*linesMutex);
					lines->push_back(buf);
					if (*topLine + maxY - 3 == lines->size() - 1) {
						++topLine;
					}
				}
				refreshWindow();
				std::memset(buf, 0, sizeof(buf));
			} else if (not self._socket.connected()) {
				lines->push_back(std::format("Server {}:{} disconnected", self._ip.str(), self._port));
				if (*topLine + maxY - 3 == lines->size() - 1) {
					++topLine;
				}
				refreshWindow();
				return;
			}
		} catch (...) {
			return;
		}
	}
}
