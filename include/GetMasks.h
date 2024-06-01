#pragma once

#include <Net.h>

namespace chat {
std::vector<arch::net::IPv4Mask> getMasks(const std::vector<arch::net::IPv4>& ips);
}
