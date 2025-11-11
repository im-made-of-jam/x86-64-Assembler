#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace tasm{

std::vector<std::string> getFileLines(std::string);

std::string left_stripWhitespace(std::string);
std::string right_stripWhitespace(std::string);
std::string stripAllWhitespace(std::string);

std::vector<std::string> split(std::string, char);

void writeElf(std::string outputFilename, std::vector<uint8_t> textSection);

}; // namespace tasm
