#include "passes.h"
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

std::vector<AssemblerLine> secondPass(const std::vector<AssemblerLine>& inputLines){
    std::vector<AssemblerLine> output;

    for(AssemblerLine line : inputLines){
        // modify the line here
        output.push_back(line);
    }

    return output;
}

}; // namespace tasm
