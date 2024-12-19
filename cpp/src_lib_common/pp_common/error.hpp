#pragma once
#include "./base.hpp"

enum struct eErrorType : uint32_t {
	Undefined       = 0x01,
	ConfigError     = 0x02,
	ServiceConflict = 0x03,
};
