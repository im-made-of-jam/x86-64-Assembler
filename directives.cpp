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

// returns the message from the message directive
std::string getMessage(AssemblerLine directiveLine){
    std::vector<std::string> splitLine = split(directiveLine.contents, ' ');

    if(splitLine.size() < 2){
        return "";
    }

    if(splitLine[0].at(0) != '.'){
        return "";
    }

    if(splitLine[0] != ".message"){
        return "";
    }

    std::string returnString = "";

    // make the rest of the line back into a string
    for(uint32_t i = 1; i < splitLine.size(); ++i){
        returnString += splitLine[i];
        returnString += ' ';
    }

    return returnString;
}

std::vector<uint8_t> getInsertBytes(AssemblerLine directiveLine){
    std::vector<uint8_t> returnVec;

    std::vector<std::string> splitLine = split(directiveLine.contents, ' ');

    for(uint32_t i = 1; i < splitLine.size(); ++i){
        uint8_t byte = getByteFromHex(splitLine[i]);
        returnVec.push_back(byte);
    }

    return returnVec;
}

// returns a value in [0-255] if valid, otherwise some value < 0
int16_t getByteFromHex(std::string thisString){
    int16_t notQuiteByte;

    if(thisString.length() > 2){
        std::cout << "Invalid byte " << thisString << std::endl;
        return -1;
    }

    char char0 = std::tolower(thisString.at(0));
    char char1 = std::tolower(thisString.at(1));

    if(!(('0' <= char0 && char0 <= '9') || ('a' <= char0 && char0 <= 'f'))){
        std::cout << "Invalid byte " << thisString << std::endl;
        return -1;
    }

    if(!(('0' <= char1 && char1 <= '9') || ('a' <= char1 && char1 <= 'f'))){
        std::cout << "Invalid byte " << thisString << std::endl;
        return -1;
    }

    if(char1 > '9'){
        notQuiteByte += (char1 - 'a') + 10;
    }
    else{
        notQuiteByte += (char1 - '0');
    }

    if(char0 > '9'){
        notQuiteByte += ((char0 - 'a') + 10) * 16;
    }
    else{
        notQuiteByte += (char0 - '0') * 16;
    }

    return (notQuiteByte & 0xFF);
}

// returns 0 if no such directive exists, otherwise returns the index of that directive
uint32_t isValidDirective(std::string name){
    if(name == ".include"){
        return directive_type::include;
    }
    else if(name == ".insert"){
        return directive_type::insert;
    }
    else if(name == ".message"){
        return directive_type::message;
    }
    else{
        return directive_type::invalid;
    }
}


}; // namespace tasm
