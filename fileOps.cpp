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

}; // namespace tasm
