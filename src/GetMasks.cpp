#include <GetMasks.h>
#include <vector>
#include <map>

#if CHAT_WINDOWS
#include <WinSock2.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#else
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

std::vector<arch::net::IPv4Mask> chat::getMasks(const std::vector<arch::net::IPv4>& ips) {
#if CHAT_WINDOWS
	ULONG retval = 0;

	// Set the flags to pass to GetAdaptersAddresses
	ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

	// default to unspecified address family (both)
	ULONG family = AF_UNSPEC;

	LPVOID buf = nullptr;

	PIP_ADAPTER_ADDRESSES addresses = nullptr;
	ULONG bufLen = 0;
	ULONG iters = 0;

	PIP_ADAPTER_ADDRESSES currAddresses = nullptr;
	PIP_ADAPTER_UNICAST_ADDRESS unicast = nullptr;

	family = AF_INET;

	// Allocate a 15 KB buffer to start with.
	bufLen = 15'000;

	std::map<std::string, size_t> mask_of;
	std::vector<arch::net::IPv4Mask> masks;

	do {
		addresses = (IP_ADAPTER_ADDRESSES*)malloc(bufLen);
		if (addresses == NULL) {
			return {};
		}

		retval = GetAdaptersAddresses(family, flags, NULL, addresses, &bufLen);

		if (retval == ERROR_BUFFER_OVERFLOW) {
			free(addresses);
			addresses = NULL;
		} else {
			break;
		}

		iters++;
	} while ((retval == ERROR_BUFFER_OVERFLOW) && (iters < 3));

	if (retval == NO_ERROR) {
		// If successful, output some information from the data we received
		currAddresses = addresses;
		while (currAddresses) {
			unicast = currAddresses->FirstUnicastAddress;
			if (unicast != nullptr) {
				for (size_t i = 0; unicast != NULL; i++) {
					SOCKET_ADDRESS sockaddr = unicast->Address;
					ULONG prefixLength = unicast->OnLinkPrefixLength;

					// Convert prefix length to network mask
					ULONG mask = 0xFFFF'FFFF << (32 - prefixLength);
					struct in_addr maskAddr;
					maskAddr.S_un.S_addr = htonl(mask);
					// Print network mask
					/*printf("IP: %s\n", inet_ntoa(((sockaddr_in*)sockaddr.lpSockaddr)->sin_addr));
					printf("Network mask: %s\n", inet_ntoa(maskAddr));*/

					mask_of[inet_ntoa(((sockaddr_in*)sockaddr.lpSockaddr)->sin_addr)] = prefixLength;

					unicast = unicast->Next;
				}
			}

			currAddresses = currAddresses->Next;
		}
	} else {
		return {};
	}

	if (addresses) {
		free(addresses);
	}

	for (auto&& ip : ips) {
		masks.emplace_back(mask_of[ip.str()]);
	}

	return masks;
#else
	ifaddrs* ifaddr;
	if (getifaddrs(&ifaddr) == -1) {
		return {};
	}

	std::map<std::string, std::string> mask_of;
	std::vector<arch::net::IPv4Mask> masks;

	for (auto ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == nullptr or ifa->ifa_addr->sa_family != AF_INET or ifa->ifa_netmask == nullptr) {
			continue;
		}

		char addr[INET_ADDRSTRLEN]{};
		char mask[INET_ADDRSTRLEN]{};
		inet_ntop(AF_INET, &(((sockaddr_in*)ifa->ifa_addr)->sin_addr), addr, sizeof(addr));
		inet_ntop(AF_INET, &(((sockaddr_in*)ifa->ifa_netmask)->sin_addr), mask, sizeof(mask));
		mask_of[addr] = mask;
	}

	for (auto&& ip : ips) {
		masks.emplace_back(mask_of[ip.str()]);
	}

	return masks;
#endif
}
