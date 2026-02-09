#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "AssemblerLine.h"

namespace tasm{

constexpr uint8_t index_ax = 0;
constexpr uint8_t index_cx = 1;
constexpr uint8_t index_dx = 2;
constexpr uint8_t index_bx = 3;
constexpr uint8_t index_sp = 4;
constexpr uint8_t index_bp = 5;
constexpr uint8_t index_si = 6;
constexpr uint8_t index_di = 7;

constexpr uint8_t index_8  = 0;
constexpr uint8_t index_9  = 1;
constexpr uint8_t index_10 = 2;
constexpr uint8_t index_11 = 3;
constexpr uint8_t index_12 = 4;
constexpr uint8_t index_13 = 5;
constexpr uint8_t index_14 = 6;
constexpr uint8_t index_15 = 7;

// create a REX byte based off of things such as the accessed registers being wide, extended, etc.
static uint8_t getREXByte(bool, bool, bool, bool);

// see getModRMByteNoIndirect
uint8_t getModRMByteIndirect(bool, bool, uint32_t, uint32_t);

// one line of assembler code, that gets turned into binary data
AssemblerLine assembleOneInstruction(std::string);

// one line of assembler code, that gets turned into binary data, including the line number
AssemblerLine assembleOneInstruction(std::string, uint64_t);

// get a ModR/M byte without any indirection
uint8_t getModRMByteNoIndirect(uint32_t, uint32_t);

// gets size, extended, and index information about a register from a string representing its name
uint8_t getRegisterInformation(std::string);

// only 6 exist so the default return value of 7 is an error value
static uint8_t getSegmentRegisterIndex(std::string);

// generate an instsruction for register to register operations
AssemblerLine registerToRegister(AssemblerLine, uint8_t, uint8_t, uint64_t, std::string, std::vector<uint8_t>);

}; // namespace tasm
