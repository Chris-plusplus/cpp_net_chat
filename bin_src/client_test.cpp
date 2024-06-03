#include <iostream>

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
	// cbreak();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
	refresh();

	chat::Client client;
	if (client.connectionForm()) {
		client.chatForm();
	}

	endwin();

	// std::string password = "xd";
	// std::string message;
	// std::string nick;
	// std::uint16_t port = 30'420;

	///*std::cout << "Password: ";
	// std::cin >> password;
	// std::cout << "Port: ";
	// std::cin >> port;*/
	// std::cout << "Nick: ";
	// std::cin >> nick;

	// std::string connData = std::format("{}\033{}", password, nick);

	// std::string ip;
	// std::cout << "IP: ";
	// std::cin >> ip;

	// arch::net::TCPSocket clientSocket;
	// try {
	//	std::cout << clientSocket.condConnect(
	//					 arch::net::Host(arch::net::IPv4(ip)),
	//					 port,
	//					 connData.c_str(),
	//					 connData.length(),
	//					 16,
	//					 responseHandler
	//				 )
	//			  << '\n';
	// } catch (arch::Exception& e) {
	//	std::cout << "exception:\n";
	//	std::cout << e.what() << '\n';
	//	return 1;
	// }

	// do {
	//	std::cout << "Message: ";
	//	std::cin >> message;
	//	if (message != "quit") {
	//		clientSocket.send(message);
	//	}
	// } while (message != "quit");
}
