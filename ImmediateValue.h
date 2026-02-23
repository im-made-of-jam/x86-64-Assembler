#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace tasm{

struct ImmediateValue{
	enum {
		type_valid      = 0,
		type_outOfRange = 1 << 0,
		type_invalid    = 1 << 1
	};

	uint64_t type;

	std::vector<uint8_t> value;
	std::string message;
};

ImmediateValue getImmediate(std::string, bool = true);

}; // namespace tasm
