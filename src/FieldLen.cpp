#include <FieldLen.h>

namespace chat {
size_t fieldLen(const std::string& arg) {
	for (size_t i = arg.length() - 1; i != (size_t)-1; --i) {
		if (arg[i] != ' ') {
			return i + 1;
		}
	}
	return 0;
}
} // namespace chat
