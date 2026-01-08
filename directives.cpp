#include "directives.h"

#include "fileOps.h"

#include <iostream>
#include <vector>
#include <cstdint>

namespace tasm{

// returns the filename of the file to import if such a file exists, otherwise empty string signifies no import
std::string hasImport(AssemblerLine directiveLine){
    std::vector<std::string> splitLine = split(directiveLine.contents, ' ');

    if(splitLine.size() != 2){
        return "";
    }

    if(splitLine[0].at(0) != '.'){
        return "";
    }

    if(splitLine[0] != ".include"){
        return "";
    }

    return splitLine[1];
}

// true if such a directive exists, false otherwise
bool isValidDirective(std::string name){
    if(name == ".include"){
        return true;
    }
    else if(name == ".insert"){
        return true;
    }
    else{
        return false;
    }
}


}; // namespace tasm
