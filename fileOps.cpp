#include "fileOps.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

namespace tasm{

std::vector<std::string> getFileLines(std::string pathToFile){
	std::vector<std::string> everyLine;

	std::ifstream inputFile(pathToFile);
	std::string line;

	while(std::getline(inputFile, line)){
		everyLine.push_back(line);
	}

	return everyLine;
}

std::string left_stripWhitespace(std::string input){
	std::string output;

	bool stillTrimming = true;
	for(char c : input){
		if(stillTrimming){
			if(!std::isspace(c)){
				stillTrimming = false;
				output += c;
			}
		}
		else{
			output += c;
		}
	}

	return output;
}

std::string right_stripWhitespace(std::string input){
	std::string copy{input};
	std::reverse(copy.begin(), copy.end());
	copy = left_stripWhitespace(copy);
	std::reverse(copy.begin(), copy.end());

	return copy;
}

std::string stripAllWhitespace(std::string input){
	std::string copy{left_stripWhitespace(input)};
	std::reverse(copy.begin(), copy.end());
	copy = left_stripWhitespace(copy);
	std::reverse(copy.begin(), copy.end());

	return copy;
}

std::vector<std::string> split(std::string splitThis, char onThisChar){
	std::vector<std::string> output;

	std::string working;

	for(char c : splitThis){
		if(c == onThisChar){
			if(!working.empty()){
				output.push_back(working);
			}

			working = "";
		}
		else{
			working += c;
		}
	}

	if(!working.empty()){
		output.push_back(working);
	}

	return output;
}

void writeElf(std::string outputFilename, std::vector<uint8_t> textSection){
	// open output file
	std::ofstream outputFile(outputFilename, std::ofstream::binary);

    uint8_t byteToWrite;

    // macro for writing a byte to the file
    auto write = [&](uint8_t byte){
        byteToWrite = byte;
        outputFile.write(reinterpret_cast<const char*>(&byte), 1);
    };

    auto writeVec = [&](std::vector<uint8_t> bytes){
        for(uint8_t byte : bytes){
            write(byte);
        }
    };

    // main elf header
    writeVec({0x7F, 'E', 'L', 'F'});       // magic number
    write(0x02);                           // bitness
    write(0x01);                           // endianess
    write(0x01);                           // ELF header version
    write(0x00);                           // OS ABI version
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // padding
    writeVec({0x02, 0x00});                // ELF type
    writeVec({0x3E, 0x00});                // instruction set
    writeVec({0x01, 0, 0, 0});             // ELF version
    writeVec({0xC0, 0, 0, 0, 0, 0, 0, 0}); // program entry offset
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // program header table offset
    writeVec({0x80, 0, 0, 0, 0, 0, 0, 0}); // section header table offset
    writeVec({0, 0, 0, 0});                // flags
    writeVec({64,   0});                   // ELF header size
    writeVec({64,   0});                   // size of program header table entry
    writeVec({1,    0});                   // number of program header table entries
    writeVec({64,   0});                   // size of section header table entry
    writeVec({2,    0});                   // number of entries in the section header table
    writeVec({0x00, 0x00});                // index of .shstrtab in the section header table

    // program header
    writeVec({0x01, 0x00, 0x00, 0x00});    // type of section
    writeVec({0x07, 0x00, 0x00, 0x00});    // flags
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // offset of this section in the file image
    writeVec({0x10, 0, 0, 0, 0, 0, 0, 0}); // address of the segment in virtual memory
    writeVec({0x10, 0, 0, 0, 0, 0, 0, 0}); // address of the segment in physical memory, if relevant
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // size in bytes of the segment within the image
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // size in bytes of the segment in memory
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // alignment of the section in memory
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // null bytes for 64 byte alignment

    // section header (.shstrtab)
    writeVec({0x00, 0x00, 0x00, 0x00});    // offset in the .shstrtab table of the name of this section
    writeVec({0x01, 0x00, 0x00, 0x00});    // the type of section
    writeVec({7, 0, 0, 0, 0, 0, 0, 0});    // flags
    writeVec({0x10, 0, 0, 0, 0, 0, 0, 0}); // address of the section in virtual memory
    writeVec({0x10, 0, 0, 0, 0, 0, 0, 0}); // offset of the section in the file image
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // size of the section in bytes
    writeVec({0, 0, 0, 0});                // index of any linked sections, none in this case
    writeVec({0, 0, 0, 0});                // any extra info about the section, nothing in this case
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // required alignment of the section
    writeVec({0, 0, 0, 0, 0, 0, 0, 0});    // size of the section if it has a fixed size

	// write the actual code we want
	for(uint8_t byte : textSection){
        outputFile << byte;
	}
}

}; // namespace tasm
