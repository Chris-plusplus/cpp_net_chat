#include <algorithm>
#include <iostream>

#include <Client.h>
#include <FieldLen.h>
#include <form.h>
#include <ncurses.h>

bool chat::Client::acceptResponseHandler(const void* response, int, void* retval) {
	auto&& out = *(std::string*)retval;
	out = std::string((const char*)response);
	return out == "success";
}

bool chat::Client::connectionForm() {
	int connectionType = 0;
	{ // manual or list server selection
		curs_set(0);

		int maxY, maxX;
		getmaxyx(stdscr, maxY, maxX);
		auto formSubwin = derwin(stdscr, maxY, maxX, 0, 0);
		touchwin(stdscr);

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

		selectField(fields[0]);

		set_form_win(form, stdscr);
		set_form_sub(form, formSubwin);
		post_form(form);

		box(formSubwin, 0, 0);
		char formTitle[] = " Select connection type ";
		mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, formTitle);
		wrefresh(formSubwin);

		set_field_buffer(fields[0], 0, manualPanel);
		set_field_buffer(fields[1], 0, listPanel);
		wrefresh(formSubwin);

		int c;
		while (connectionType == 0 and (c = getch()) != '\033') {
			switch (c) {
				case KEY_UP:
					unselectField(current_field(form));
					form_driver(form, REQ_PREV_FIELD);
					form_driver(form, REQ_NEXT_CHAR);
					form_driver(form, REQ_PREV_CHAR);
					form_driver(form, REQ_END_LINE);
					break;
				case KEY_DOWN:
					unselectField(current_field(form));
					form_driver(form, REQ_NEXT_FIELD);
					form_driver(form, REQ_NEXT_CHAR);
					form_driver(form, REQ_PREV_CHAR);
					form_driver(form, REQ_END_LINE);
					break;
				case KEY_ENTER:
				case '\n':
				case '\r':
					if (current_field(form) == fields[0]) {
						connectionType = 1;
					} else {
						connectionType = 2;
					}
					break;
			}
			selectField(current_field(form));
		}

		unpost_form(form);
		for (size_t i = 0; i != 2; ++i) {
			free_field(fields[i]);
		}
		delwin(formSubwin);

		if (not connectionType) {
			return false;
		}
	}
	curs_set(1);
	if (connectionType == 1) {
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

		const int hostnameMax = 253 + 1;
		const int portMax = 5 + 1;
		const int passwordMax = 50 + 1;
		const int nickMax = 50 + 1;

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
		char formTitle[] = " Connect to server ";
		mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, formTitle);
		wrefresh(formSubwin);

		mvwprintw(formSubwin, firstFieldY, 1, hostnamePanel);
		mvwprintw(formSubwin, firstFieldY + 1, 1, portPanel);
		mvwprintw(formSubwin, firstFieldY + 2, 1, passwordPanel);
		mvwprintw(formSubwin, firstFieldY + 3, 1, nickPanel);
		set_field_buffer(fields[4], 0, connectPanel);
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
					form_driver(form, REQ_NEXT_CHAR);
					form_driver(form, REQ_PREV_CHAR);
					form_driver(form, REQ_END_LINE);
					break;
				case KEY_DOWN:
					unselectField(current_field(form));
					form_driver(form, REQ_NEXT_FIELD);
					form_driver(form, REQ_NEXT_CHAR);
					form_driver(form, REQ_PREV_CHAR);
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
						form_driver(form, REQ_NEXT_CHAR);
						form_driver(form, REQ_PREV_CHAR);
						form_driver(form, REQ_END_LINE);
					} else {
						form_driver(form, REQ_VALIDATION);
						resetErrorSpace();
						box(formSubwin, 0, 0);
						char formTitle[] = " Connect to server ";
						mvwprintw(formSubwin, 0, (maxX - (sizeof(formTitle) + 1)) / 2, formTitle);
						wrefresh(formSubwin);

						char* errPtr;

						std::string tempIP = field_buffer(fields[0], 0);
						tempIP.erase(fieldLen(tempIP));
						this->_ip = arch::net::IPv4(tempIP);
						if (this->_ip.str() != tempIP) {
							printError(std::format("IP '{}' is not a valid IP address", tempIP));
							break;
						}

						int portVal = (int)strtoul(field_buffer(fields[1], 0), &errPtr, 10);
						if (errPtr == field_buffer(fields[1], 0)) {
							printError(std::format("Port '{}' is not a valid port", portVal));
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
						this->_password.erase(fieldLen(this->_password));
						this->_nick = field_buffer(fields[3], 0);
						this->_nick.erase(fieldLen(this->_nick));

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
						try {
							if (not connectFuture.get()) {
								printError(std::format("Error connecting to server: '{}'", response));
								break;
							} else {
								unpost_form(form);
								for (size_t i = 0; i != 5; ++i) {
									free_field(fields[i]);
								}
								delwin(formSubwin);
								return true;
							}
						} catch (arch::Exception& e) {
							printError(std::format("Error connecting to server: '{}'", e.what()));
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
	if (connectionType == 2) {}
}
