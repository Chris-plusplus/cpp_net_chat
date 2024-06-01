#pragma once

namespace arch::net {

struct Init final {
	/// @brief Initializes networking
	Init();
	/// @brief Networking cleanup
	~Init();
	/// @brief Initializes networking
	static void init();
	/// @brief Networking cleanup
	static void cleanup();
};

static inline Init _init;

} // namespace arch::net
