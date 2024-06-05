#pragma once

#include <optional>

#include <Net.h>

namespace chat {

/// @brief Chat client class. Call connectionForm() and if it succeeds, call chatForm()
class Client {
public:

	/// @brief Defaulted constructor
	Client() = default;

	/// @brief Displays and handles connection form
	/// @return true if connection succeeded, false if not
	bool connectionForm();

	/// @brief Display and handles chat form post successfull connection
	void chatForm();

private:

	// receiving loop function
	static void recvLoop(
		std::stop_token stopToken,
		Client* _this,
		int* topLine,
		int maxX,
		int maxY,
		std::vector<std::string>* lines,
		std::mutex* linesMutex,
		std::function<void()> refreshWindow
	);

	// response handler for TCPSocket condConnect
	static bool acceptResponseHandler(const void* response, int, void* retval);
	int connectionTypeForm(const std::string& redoMsg);
	int manualConnectionForm(std::string& redoMsg);
	// handles server searching and selecting in list connection mode
	int listSelectionConnectionForm(std::string& redoMsg);
	// handles credential input in list connection mode
	int listCredentialsConnectionForm(std::string& redoMsg);

	arch::net::async::TCPSocket _socket;
	arch::net::IPv4 _ip;
	arch::net::Socket::Port _port;
	std::string _password;
	std::string _nick;
};
} // namespace chat

