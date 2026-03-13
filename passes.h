#pragma once

#include <vector>
#include <string>

#include "AssemblerLine.h"

namespace tasm{

std::vector<AssemblerLine> firstPass(const std::vector<std::string>&);

}; // namespace tasm
