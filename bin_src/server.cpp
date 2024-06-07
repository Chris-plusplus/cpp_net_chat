#include <Server.h>
#include <iostream>
#include <ncurses.h>

int main() {
	arch::Logger::init(arch::LogLevel::info);

	setlocale(LC_ALL, "");
	initscr();
	raw();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
	refresh();

	chat::Server server;
	bool doStart = server.configurationForm();

	refresh();
	endwin();

	try {
		if (doStart) {
			server.start();

			std::string cmd;
			do {
				std::cin >> cmd;
			} while (cmd != "quit");
			arch::Logger::info("Quitting");
		}
	} catch (arch::net::NetException& e) {
		arch::Logger::info(
			"{}, errno: {}, at {}:{}",
			e.what(),
			netErrno(),
			e.location().file_name(),
			e.location().line()
		);
	}
}
