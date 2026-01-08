// THIS WHOLE THING USES A BASTARDIZED VERSION OF INTEL SYNTAX
// COPE
// (its basically just intel syntax without the comma)
// ((plus a couple of things that i could not be bothered implementing properly (see: tebfinder)))
#include <iostream>
#include <fstream>

#include "fileOps.h"
#include "firstPass.h"
#include "assembly.h"
#include "directives.h"

int main(){
    std::vector<uint8_t> combinedData;

	for(std::string line : tasm::getFileLines("./input")){
		tasm::AssemblerLine a = tasm::assembleOneInstruction(line);

		if(a.type == tasm::AssemblerLine::type_invalid){
			std::cout << std::endl << std::string(a.data.begin(), a.data.end()) << std::endl;
		}
		else if(a.type == tasm::AssemblerLine::type_label){
			std::cout << std::endl << "label called " << a.contents << std::endl;
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

        for(uint8_t byte : a.data){
            combinedData.push_back(byte);
        }

        if(a.type == tasm::AssemblerLine::type_directive){
            std::cout << tasm::hasImport(a) << std::endl;
        }
	}

    tasm::writeElf("./z_testelf", combinedData);
}
