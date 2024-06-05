#include <iostream>
#include <fstream>
#include <filesystem>

#include <Client.h>
#include <Net.h>
#include <ncurses.h>

bool responseHandler(const void* response, int responseLen, void*) {
	std::cout << ((const char*)response) << '\n';
	return std::string(((const char*)response)) == "success";
}

int main() {
	arch::Logger::init(arch::LogLevel::info);

	setlocale(LC_ALL, "");
	initscr();
	raw();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
	refresh();

	chat::Client client;
	if (client.connectionForm()) {
		client.chatForm();
	}

	refresh();
	endwin();
}
