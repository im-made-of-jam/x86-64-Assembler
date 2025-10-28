#include "firstPass.h"
#include "fileOps.h"

#include "AssemblerLine.h"

#include <iostream>
#include <algorithm>

namespace tasm{

std::vector<AssemblerLine> firstPass(const std::vector<std::string>& inputLines){
	std::vector<AssemblerLine> output;

	for(std::string line : inputLines){
		line = stripAllWhitespace(line);

		AssemblerLine thisLine{.contents = line, .type = AssemblerLine::type_none};

		output.push_back(thisLine);
	}

	return output;
}

}; // namespace tasm
