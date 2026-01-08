#pragma once

#include "AssemblerLine.h"

#include <string>

namespace tasm{

std::string hasImport(AssemblerLine);

bool isValidDirective(std::string);

}; // namespace tasm
