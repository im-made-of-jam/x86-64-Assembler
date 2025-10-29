// THIS WHOLE THING USES A BASTARDIZED VERSION OF INTEL SYNTAX
// COPE
// (its basically just intel syntax without the comma)
#include <iostream>

#include "fileOps.h"
#include "firstPass.h"
#include "assembly.h"

int main(){
	for(std::string line : tasm::getFileLines("./input")){
		tasm::AssemblerLine a = tasm::assembleOneInstruction(line);

		if(a.type == tasm::AssemblerLine::type_invalid){
			std::cout << std::endl << std::string(a.data.begin(), a.data.end()) << std::endl;
		}
		else{
			for(uint64_t i = 0; i < a.data.size(); ++i){
				std::string outputString = std::format("{:x}", static_cast<uint64_t>(a.data[i]));
				if(outputString.size() == 1){
					outputString = "0" + outputString;
				}
				std::cout << outputString << " " << std::flush;
			}
		}
	}
}
