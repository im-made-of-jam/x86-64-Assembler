#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "AssemblerLine.h"

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

AssemblerLine assembleOneInstruction(std::string);
AssemblerLine assembleOneInstruction(std::string, uint64_t);
uint8_t getModRMByteNoIndirect(uint32_t, uint32_t);
uint8_t getRegisterInformation(std::string);
