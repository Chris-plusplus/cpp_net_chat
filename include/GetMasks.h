#pragma once

#include <Net.h>

namespace chat {
arch::net::IPv4Mask getMasks(const arch::net::IPv4& ip);
std::vector<arch::net::IPv4Mask> getMasks(const std::vector<arch::net::IPv4>& ips);
} // namespace chat
