#pragma once

#include "AssemblerLine.h"

#include <string>
#include <vector>

namespace tasm{

std::string hasImport(AssemblerLine);

std::string getMessage(AssemblerLine);

uint32_t isValidDirective(std::string);

std::vector<uint8_t> getInsertBytes(AssemblerLine);

static int16_t getByteFromHex(std::string);

namespace directive_type{

constexpr uint64_t invalid = 0;
constexpr uint64_t include = 1;
constexpr uint64_t insert  = 1 << 1;
constexpr uint64_t message = 1 << 2;

}; // namespace directive_type

}; // namespace tasm
