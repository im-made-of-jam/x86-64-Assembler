#pragma once

#include <vector>
#include <string>

#include "AssemblerLine.h"

namespace tasm{

// turns strings into basic tokens
std::vector<AssemblerLine> firstPass(const std::vector<std::string>&);

// sorts out offsets with labels
std::vector<AssemblerLine> secondPass(const std::vector<AssemblerLine>&);

}; // namespace tasm
