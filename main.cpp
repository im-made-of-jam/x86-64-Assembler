// THIS WHOLE THING USES A BASTARDIZED VERSION OF INTEL SYNTAX
// COPE
// (its basically just intel syntax without the comma)
#include <iostream>

#include "fileOps.h"
#include "firstPass.h"
#include "assembly.h"

int main(){
	for(std::string line : getFileLines("./input")){
		AssemblerLine a = assembleOneInstruction(line);

		if(a.type == AssemblerLine::type_invalid){
			std::cout << std::endl << std::string(a.data.begin(), a.data.end()) << std::endl;
		}
		else{
			for(uint64_t i = 0; i < a.data.size(); ++i){
				std::cout << std::hex << static_cast<uint64_t>(a.data[i]) << std::dec << " " << std::flush;
			}
		}
	}
}
