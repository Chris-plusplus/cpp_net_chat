#include <iostream>
#include <format>
#include <GetMasks.h>

int main() {
	arch::net::Host localhost = arch::net::Host::localhost(true);
	for (auto&& ip : localhost.ips()) {
		std::cout << ip.str() << '\n';
	}

	std::vector<arch::net::IPv4Mask> masks = chat::getMasks(localhost.ips());
	std::cout << masks.size() << '\n';

	for (size_t i = 0; i != masks.size(); ++i) {
		std::cout << std::format("{}: {}\n", localhost.ips()[i].str(), masks[i].str());
	}

	addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* data = nullptr;
	int result = getaddrinfo("chrislaptok", nullptr, &hints, &data);

	std::vector<arch::net::IPv4> _ips{ { "127.0.0.1" } };

	for (auto i = data; i != nullptr; i = i->ai_next) {
		if (((sockaddr_in*)i->ai_addr)->sin_addr != _ips[0]) {
			_ips.emplace_back(((sockaddr_in*)i->ai_addr)->sin_addr);
		}
	}
	for (auto&& ip : _ips) {
		std::cout << ip.str() << '\n';
	}

	freeaddrinfo(data);

	return 0;
}
