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

		if(a.type == tasm::AssemblerLine::type_invalid_directive){
			std::cout << "Invalid directive: '" << std::string(a.contents) << "'" << std::endl;
		}
        else if(a.type == tasm::AssemblerLine::type_invalid){
            std::cout << std::string{a.data.begin(), a.data.end()} << std::endl;
        }
        else if(a.type == tasm::AssemblerLine::type_directive){
            switch(a.data[a.data.size() - 1]){
                case tasm::directive_type::invalid:{
                    break;
                }
                case tasm::directive_type::include:{
                    std::string includePath = tasm::hasImport(a);

                    if(includePath.size()){
                        std::cout << "Include: " << includePath << std::endl;
                    }
                    break;
                }
                case tasm::directive_type::insert:{
                    std::cout << "Insert: ";
                    std::vector<uint8_t> bytes = tasm::getInsertBytes(a);
                    for(uint32_t i = 0; i < bytes.size(); ++i){
                        std::cout << std::hex << 0ull + bytes[i] << std::dec << ' ';
                    }
                    std::cout << std::endl;
                    break;
                }
                case tasm::directive_type::message:{
                    std::cout << "Message: " << tasm::getMessage(a) << std::endl;
                    break;
                }
            }
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
	}

    tasm::writeElf("./z_testelf", combinedData);
}
