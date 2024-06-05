#include <ThreadIDStr.hpp>
#include <sstream>

namespace chat {

std::string tidStr(std::thread::id id) noexcept {
	std::stringstream sstream;
	sstream << id;
	return sstream.str();
}

} // namespace chat
