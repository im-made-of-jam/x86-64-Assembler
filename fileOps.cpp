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
	// open both input files
	std::ifstream elfStart("./binaryLib/startOfElf.bin", std::ifstream::binary);
	std::ifstream elfEnd("./binaryLib/endOfElf.bin", std::ifstream::binary);
	// open output file
	std::ofstream outputFile(outputFilename, std::ofstream::binary);

	// find size of both input files
	elfStart.seekg(0, elfStart.end);
	uint32_t elfStartSize = elfStart.tellg();
	elfStart.seekg(0, elfStart.beg);

	elfEnd.seekg(0, elfEnd.end);
	uint32_t elfEndSize = elfEnd.tellg();
	elfEnd.seekg(0, elfEnd.beg);

	// make buffers for readung from both input files
	char* elfStartBuffer = new char[elfStartSize];
	char* elfEndBuffer   = new char[elfEndSize];

	// go and pull out the data from both input files
	elfStart.read(elfStartBuffer, elfStartSize);
	elfEnd.read(elfEndBuffer, elfEndSize);

	// write the start to the file
	outputFile.write(elfStartBuffer, elfStartSize);

	// write the actual code we want
	for(uint8_t byte : textSection){
		outputFile << byte;
	}

	// now write the end of the file
	outputFile.write(elfEndBuffer, elfEndSize);
}

}; // namespace tasm
