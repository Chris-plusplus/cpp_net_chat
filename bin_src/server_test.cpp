#include <Server.h>
#include <iostream>

int main() {
	arch::Logger::init(arch::LogLevel::info);

	chat::Server server;

	std::string password = "xd";
	std::string message = "d";
	std::uint16_t port = 30'420;

	/*std::cout << "Password: ";
	std::cin >> password;
	std::cout << "Port: ";
	std::cin >> port;
	std::cout << "Message: ";
	std::cin >> message;*/

	server.setMessage(message);
	server.setPassword(password);
	server.setPort(port);

	server.start();

	std::string cmd;
	do {
		std::cin >> cmd;
	} while (cmd != "quit");
	arch::Logger::info("quitting");

	exit(0);
}
