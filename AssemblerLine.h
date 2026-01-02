#pragma once

#include <string>
#include <cstdint>

namespace tasm{

struct AssemblerLine{
	enum uint64_t{
		type_none        = 0,
		type_instruction = 1 << 0,
		type_label       = 1 << 1,
		type_directive   = 1 << 2,
		type_data        = 1 << 3,
		type_invalid     = 1 << 4,
		type_unprocessed = 1 << 5
	};
	std::string contents;      // the source line that the instruction came from

	uint64_t offset;           // the binary offset of the raw instruction within the file
	uint64_t size;             // the size of the binary instruction, if an instruction

	uint64_t type = type_none; // the type of line from the assembler

	// data relevant to the type e.g. binary for an instruction, a string in bytes for error, offset for a label etc.
	std::vector<uint8_t> data;

	// indicates that the instruction needs patching, for example to write in a memory immediate
	bool needsPatching = false;
};

}; // namespace tasm
