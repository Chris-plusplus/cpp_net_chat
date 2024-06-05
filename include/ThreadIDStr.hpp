#pragma once

#include <thread>
#include <format>

namespace chat {

std::string tidStr(std::thread::id id) noexcept;

}
