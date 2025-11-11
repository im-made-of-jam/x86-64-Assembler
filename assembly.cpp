#include "assembly.h"

#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <format>
#include <iostream>
#include <algorithm>

#include "fileOps.h"
#include "AssemblerLine.h"
#include "ImmediateValue.h"

#include <elf.h>

Elf64_Off a;

// for instructions of the form (r, r/m), quite literally because i cannot be bothered, they only support register to register for now

namespace tasm{

uint8_t getREXByte(bool wide, bool RExtend, bool indexExtend, bool MExtend){
	uint8_t returnByte = 0b01000000;
	if(wide){
		returnByte |= 0b00001000;
	}
	if(RExtend){
		returnByte |= 0b00000100;
	}
	if(indexExtend){
		returnByte |= 0b00000010;
	}
	if(MExtend){
		returnByte |= 0b00000001;
	}

	return returnByte;
}

// left and right refer to the left and right operands in the text form of the instruction
// e.g. mov rax, rdx
// rax is the left register, rdx is the right register
uint8_t getModRMByteNoIndirect(uint32_t rm, uint32_t reg){
	//	/digit — A digit between 0 and 7 indicates that the ModR/M byte of the instruction uses only the r/m (register or memory) operand. The reg field contains the digit that provides an extension to the instruction's opcode.
	//	/r — Indicates that the ModR/M byte of the instruction contains a register operand and an r/m operand

	//	+rb, +rw, +rd, +ro — Indicated the lower 3 bits of the opcode byte is used to encode the register operand
	//		without a modR/M byte. The instruction lists the corresponding hexadecimal value of the opcode byte with low
	//		3 bits as 000b. In non-64-bit mode, a register code, from 0 through 7, is added to the hexadecimal value of the
	//		opcode byte. In 64-bit mode, indicates the four bit field of REX.b and opcode[2:0] field encodes the register
	//		operand of the instruction. “+ro” is applicable only in 64-bit mode. See Table 3-1 for the codes

	uint8_t byte = 0b11000000;

	if(rm > 7){
		rm = 0;
	}
	byte |= static_cast<uint8_t>(rm);

	if(reg > 7){
		reg = 0;
	}

	byte |= static_cast<uint8_t>(reg << 3);

	return byte;
}

// see getModRMByteNoIndirect
uint8_t getModRMByteIndirect(bool leftIndirect, bool rightIndirect, uint32_t leftRegister, uint32_t rightRegister){
	uint8_t byte = 0;

	if(!leftIndirect){
		byte |= 0b10000000;
	}
	if(!rightIndirect){
		byte |= 0b01000000;
	}

	if(leftRegister > 7){
		leftRegister = 0;
	}
	byte |= static_cast<uint8_t>(leftRegister);

	if(rightRegister > 7){
		rightRegister = 0;
	}

	byte |= static_cast<uint8_t>(rightRegister << 3);

	return byte;
}

//	only supports main integer registers at the moment
//	byte returned contains information
//	A means relevant bit, x means irrelevant
//	high byte
//		0bAAxxxxxx is size specifier (byte, word, dword, qword)(0, 1, 2, 3 respectively)
//		0bxxxxAAAA is register index, with extended registers having the first bit set
//		0bxxAAxxxx
//			00 when the register does not exist or the register is extended
//			01 when it is a main register optionally with prefix e.g. ax, eax, rbx
//			10 when it is a low byte e.g. al or dl
//			11 when it is a high byte e.g. ah or dh
//	notably, this means if the register does not exist, the function returns 0
uint8_t getRegisterInformation(std::string registerName){
	constexpr uint8_t byteSize =   0b00000000;
	constexpr uint8_t wordSize =   0b01000000;
	constexpr uint8_t dwordSize =  0b10000000;
	constexpr uint8_t qwordSize =  0b11000000;

	constexpr uint8_t isExtended = 0b00001000;

	// the specific order within these is important please do not change
	static std::vector<std::string> legalWithPrefix   = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
	static std::vector<std::string> legalNoPrefixLow  = {"al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil"};
	static std::vector<std::string> legalNoPrefixHigh = {"ah", "ch", "dh", "bh"};
	static std::vector<std::string> legalWithSuffix   = {"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};

	// for example ax, eax, rax
	for(uint64_t i = 0; i < 8; ++i){
		std::string ePrefix = std::string{'e'}.append(legalWithPrefix[i]);
		std::string rPrefix = std::string{'r'}.append(legalWithPrefix[i]);

		if(registerName == legalWithPrefix[i]){ // 16 bit register
			return (0b00010000 | wordSize | i);
		}

		if(registerName == ePrefix){ // 32 bit register
			return (0b00010000 | dwordSize | i);
		}

		if(registerName == rPrefix){ // 64 bit register
			return (0b00010000 | qwordSize | i);
		}
	}

	// for example al
	for(uint64_t i = 0; i < 8; ++i){ // these are only legal as is
		if(registerName == legalNoPrefixLow[i]){
			if(i > 3){ // these need a rex byte
				return (0b00100000 | byteSize | i | isExtended);
			}
			return (0b00100000 | byteSize | i);
		}
	}

	// for example ah
	for(uint64_t i = 0; i < 4; ++i){ // these are only legal as is
		if(registerName == legalNoPrefixHigh[i]){
			return (0b00110000 | byteSize | i);
		}
	}

	// for example r8, r8d, r8w, r8b
	for(uint64_t i = 0; i < 8; ++i){ // these are legal with a size specifier suffix
		// since append works in place (annoyingly)
		std::string dwordName = legalWithSuffix[i];
		std::string wordName = legalWithSuffix[i];
		std::string byteName = legalWithSuffix[i];

		dwordName.append("d");
		wordName.append("w");
		byteName.append("b");

		if(registerName == legalWithSuffix[i]){ // full register
			return (isExtended | qwordSize | i);
		}
		if(registerName == dwordName){ // 32 bit register
			return (isExtended | dwordSize | i);
		}
		if(registerName == wordName){ // 16 bit register
			return (isExtended | wordSize | i);
		}
		if(registerName == byteName){ // 8 byte register
			return (isExtended | byteSize | i);
		}
	}

	return 0;
}

// only 6 exist so the default return value of 7 is an error value
uint8_t getSegmentRegisterIndex(std::string registerName){
	if(registerName == "es"){
		return 0;
	}
	if(registerName == "cs"){
		return 1;
	}
	if(registerName == "ss"){
		return 2;
	}
	if(registerName == "ds"){
		return 3;
	}
	if(registerName == "fs"){
		return 4;
	}
	if(registerName == "gs"){
		return 5;
	}
	return 7;
}

constexpr uint8_t registerInformationSizeMask     = 0b11000000;
constexpr uint8_t registerInformationTypeMask     = 0b00110000;
constexpr uint8_t registerInformationIndexMask    = 0b00000111;
constexpr uint8_t registerInformationExtendedMask = 0b00001000;


// prefix to override register size to 16 bit
constexpr uint8_t override16Bit = 0x66;

// generate an instsruction for register to register operations
AssemblerLine registerToRegister(AssemblerLine input, uint8_t destinationInformation, uint8_t sourceInformation, uint64_t sourceLine, std::string name, std::vector<uint8_t> opcode){
	// REX.W + opcode /r

	// for the case of two registers, they must match in size

	AssemblerLine output = input;

	std::vector<uint8_t> binary;

	if(output.type != AssemblerLine::type_unprocessed){
		return output;
	}

	if((destinationInformation & registerInformationSizeMask) != (sourceInformation & registerInformationSizeMask)){
		output.type = AssemblerLine::type_invalid;
		std::string errorMessage = "registers in ";
		errorMessage.append(name);
		errorMessage.append(" are not of same size on line ");
		errorMessage.append(std::to_string(sourceLine));

		for(char byte : errorMessage){
			output.data.push_back(static_cast<uint8_t>(byte));
		}

		return output;
	}

	output.type = AssemblerLine::type_instruction;

	uint16_t size = (destinationInformation & registerInformationSizeMask) >> 6;
	bool destIsExtended = ((destinationInformation & registerInformationExtendedMask) >> 3) == 1;
	bool srcIsExtended = ((sourceInformation & registerInformationExtendedMask) >> 3) == 1;

	switch(size){
		case 0:{
			// r8, r/m8
			// this can die in a hole for all i care i cannot make it work
			output.type = AssemblerLine::type_invalid;
			std::string errorMessage = "dont support ";
			errorMessage.append(name);
			errorMessage.append(" r8, r/m8 yet. line ");
			errorMessage.append(std::to_string(sourceLine));

			for(uint8_t byte : binary){
				output.data.push_back(byte);
			}

			return output;
		}
		case 1:{
			// r16, r/m16
			binary.push_back(override16Bit); // override size to 16 bit
			// then fall through to 32 bit because this is literally the only difference
			[[fallthrough]];
		}
		case 2:{
			// r32, r/m32endl;
			if(srcIsExtended || destIsExtended){
				binary.push_back(getREXByte(false, srcIsExtended, false, destIsExtended));
			}

			for(uint8_t byte : opcode){
				binary.push_back(byte);
			}

			binary.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), (sourceInformation & registerInformationIndexMask)));

			break;
		}
		case 3:{
			// r64, r/m64
			if(srcIsExtended || destIsExtended){
				binary.push_back(getREXByte(true, srcIsExtended, false, destIsExtended));
			}
			else{
				binary.push_back(getREXByte(true, false, false, false));
			}

			for(uint8_t byte : opcode){
				binary.push_back(byte);
			}

			binary.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), (sourceInformation & registerInformationIndexMask)));
		}
	}

	for(uint8_t byte : binary){
		output.data.push_back(byte);
	}

	return output;
};

//	requires instruction to be of the form
//		operand arg1 arg2
//	whitespace is important and is what separates different parts of the instruction
//	said whitespace must be a simple space character (0x20)
AssemblerLine assembleOneInstruction(std::string input, uint64_t sourceLine){
	/* currently implemented:
		mov r/r
		mov r/i
		mov r/[r]
		mov [r]/r

		lodsq

		xchg r/r
		xor r/r
		add r/r
		adc r/r
		sub r/r
		pop r
		push r
		inc
		dec
		div
		idiv
		mul
		imul
		shl
		shr

		clc
		cld
		cli
		stc
		std
		sti

		ret
		syscall
		sysret
		hlt

		cpuid
		lock
			yes this is actually a prefix but in this assembler it goes the line before the instruction to be locked
	*/
	/* mnemonics that map to real instructions but actually arent real mnemonics
		pebFinder (mov rax, gs:[0x60])
	*/
	AssemblerLine output{.contents = input, .type=AssemblerLine::type_unprocessed};

	// the binary encoding of the instruction
	std::vector<uint8_t> binary;

	// the parts of the text encoding of the instruction
	std::string operation;
	std::string arg1;
	std::string arg2;

	std::vector<std::string> splitLine = split(input, ' ');
	uint8_t destinationInformation;
	uint8_t sourceInformation;

	if(splitLine.size() == 0){
		// empty line can just be returned
		output.type = AssemblerLine::type_none;
		return output;
	}
	operation = splitLine[0];
	if(operation.at(0) == ';'){
		output.type = AssemblerLine::type_none;
		return output;
	}

	if(splitLine.size() > 1){
		arg1 = splitLine[1];
		destinationInformation = getRegisterInformation(splitLine[1]);
	}
	if(splitLine.size() > 2){
		arg2 = splitLine[2];
		sourceInformation = getRegisterInformation(splitLine[2]);
	}
	if(splitLine.size() == 1){
		// one element on the line, check for label
		if(input.at(input.size() - 1) == ':'){
			output.type = AssemblerLine::type_label;
			return output;
		}
	}

	uint64_t destDataSize = (destinationInformation & registerInformationSizeMask) >> 6;

	// error if there are no operands
	auto errorOnLessThanOne = [&](std::string name){
		if(splitLine.size() == 1){
			output.type = AssemblerLine::type_invalid;
			std::string errorMessage = std::format("no operand for {} on line {}", std::string{name}, sourceLine);

			for(char byte : errorMessage){
				output.data.push_back(static_cast<uint8_t>(byte));
			}

			return output;
		}

		return output;
	};

	// error if there are one or no operands
	auto errorOnLessThanTwo = [&](std::string name){
		if(splitLine.size() == 1){
			output.type = AssemblerLine::type_invalid;
			std::string errorMessage = std::format("no destination for {} on line {}", std::string{name}, sourceLine);

			for(char byte : errorMessage){
				output.data.push_back(static_cast<uint8_t>(byte));
			}

			return output;
		}

		if(splitLine.size() == 2){
			output.type = AssemblerLine::type_invalid;
			std::string errorMessage = std::format("no source for {} on line {}", std::string{name}, sourceLine);

			for(char byte : errorMessage){
				output.data.push_back(static_cast<uint8_t>(byte));
			}

			return output;
		}

		return output;
	};

	// generate a single byte instruction
	auto oneByteOnly = [&](uint8_t byte, std::string name, bool needsREX=false){
		errorOnLessThanOne(name);

		if(destDataSize == 1){
			binary.push_back(override16Bit);
		}

		if((destDataSize == 3) && needsREX){
			binary.push_back(getREXByte(true, false, false, (destinationInformation & registerInformationExtendedMask)));
		}
		else if(destinationInformation & registerInformationExtendedMask){
			binary.push_back(getREXByte(false, false, false, true));
		}

		binary.push_back(byte | (destinationInformation & registerInformationIndexMask));

		for(uint8_t byte : binary){
			output.data.push_back(byte);
		}

		return output;
	};

	// check if data has too many bytes for a given instruction size
	auto isDataTooBig = [&](uint64_t dataSize, std::vector<uint8_t> data){
		if((dataSize == 0) and (data.size() > 1)){ // 1 byte
			return true;
		}
		if((dataSize == 1) and (data.size() > 2)){ // 2 bytes
			return true;
		}
		if((dataSize == 2) and (data.size() > 4)){ // 4 bytes
			return true;
		}
		if((dataSize == 3) and (data.size() > 8)){ // 2 bytes
			return true;
		}

		return false;
	};

	// adds as many extra 0x00 as needed to get the right immediate size for the given data size
	auto padDataToSize = [&](uint64_t dataSize, std::vector<uint8_t> data){
		std::vector<uint8_t> dataCopy = data;
		uint64_t numberAdded = 0;

		if(dataSize == 0){ // 1 byte
			while(dataCopy.size() < 1){
				dataCopy.push_back(0x00);
				numberAdded++;
			}
		}
		if(dataSize == 1){ // 2 bytes
			while(dataCopy.size() < 2){
				dataCopy.push_back(0x00);
				numberAdded++;
			}
		}
		if(dataSize == 2){ // 4 bytes
			while(dataCopy.size() < 4){
				dataCopy.push_back(0x00);
				numberAdded++;
			}
		}
		if(dataSize == 3){ // 8 bytes
			while(dataCopy.size() < 8){
				dataCopy.push_back(0x00);
				numberAdded++;
			}
		}

		while(numberAdded--){
			std::rotate(dataCopy.rbegin(), (dataCopy.rbegin() + 1), dataCopy.rend());
		}

		return dataCopy;
	};

	if(operation == "mov"){ // mov r64, r64     OR     mov r64, imm     OR     mov sreg, r64     OR     mov r64, sreg
		output = errorOnLessThanTwo("mov");

		// try for segment registers
		uint8_t segmentSourceIndex = getSegmentRegisterIndex(splitLine[2]);
		uint8_t segmentDestIndex = getSegmentRegisterIndex(splitLine[1]);

		if((segmentSourceIndex != 7) != (segmentDestIndex != 7)){
			// one is a segment register, the other is no
			if(segmentDestIndex == 7){
				// destination is NOT a segment register
				output.data.push_back(getREXByte(true, false, false, ((destinationInformation & registerInformationExtendedMask) != 0)));
				output.data.push_back(0x8C);
				output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), segmentSourceIndex));

				return output;
			}
			else{
				// source is NOT a segment register
				output.data.push_back(getREXByte(true, false, false, ((destinationInformation & registerInformationExtendedMask) != 0)));
				output.data.push_back(0x8E);
				output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), segmentSourceIndex));

				return output;
			}
			//!TODO mov Sreg/reg64
		}
		if((!(segmentDestIndex == 7)) && (!(segmentSourceIndex == 7))){
			// both are segment registers, this is unencodable
			output.type = AssemblerLine::type_invalid;
			std::string errorMessage = "no such instruction exists to move a segment register to a segment register. line ";
			errorMessage.append(std::to_string(sourceLine));

			for(char c : errorMessage){
				output.data.push_back(c);
			}

			return output;
		}
		// neither are segment registers, try for regular move

		if(destinationInformation && sourceInformation){ // mov r64, r64
			return registerToRegister(output, destinationInformation, sourceInformation, sourceLine, operation, {0x89});
		}

		if(destinationInformation && (!sourceInformation)){ // mov r64, i64
			// main bytes plus prefixes
			AssemblerLine instruction = oneByteOnly({0xB8}, operation, (destDataSize == 3));

			// extra bytes added on at the end
			ImmediateValue imm = getImmediate(arg2);

			if(imm.type != ImmediateValue::type_valid){
				instruction.type = AssemblerLine::type_invalid;
				std::string errorMessage = "invalid value for immediate in mov. line ";
				errorMessage.append(std::to_string(sourceLine));

				instruction.data.clear();

				for(char c : errorMessage){
					instruction.data.push_back(c);
				}
			}

			if(isDataTooBig(destDataSize, imm.value)){
				instruction.type = AssemblerLine::type_invalid;
				std::string errorMessage = "value of immediate is too large for destination. line ";
				errorMessage.append(std::to_string(sourceLine));

				instruction.data.clear();

				for(char c : errorMessage){
					instruction.data.push_back(c);
				}

				return instruction;
			}

			imm.value = padDataToSize(destDataSize, imm.value);

			for(uint64_t i = imm.value.size(); i != 0; --i){
				instruction.data.push_back(imm.value[i - 1]);
			}

			return instruction;
		}

		output.type = AssemblerLine::type_invalid;
		std::string errorMessage = "mov only supported where destination is {register} and source is {register or immediate}, or one GPR and one SegR";

		for(char c : errorMessage){
			output.data.push_back(c);
		}

		return output;
	}
	else if(operation == "mov*"){ // mov r64, [r64]
		// this isnt an actual instruction but i cannot be bothered implementing this assembler properly
		// also this will only support 64 bits
		output = errorOnLessThanTwo("mov*");
		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(!((destDataSize == 3) && (((sourceInformation & registerInformationSizeMask) >> 6) == 3))){
			output.type = AssemblerLine::type_invalid;
			std::string errorMessage = "moving to and from memory only supported for 64 bit operands";
			for(char c : errorMessage){
				output.data.push_back(c);
			}

			return output;
		}

		bool destIsExtended = ((destinationInformation & registerInformationExtendedMask) >> 3) == true;
		bool srcIsExtended = ((sourceInformation & registerInformationExtendedMask) >> 3) == true;
		output.data.push_back(getREXByte(true, destIsExtended, false, srcIsExtended));
		output.data.push_back(0x8B);
		output.data.push_back(getModRMByteIndirect(true, true, (sourceInformation & registerInformationIndexMask), (destinationInformation & registerInformationIndexMask)));

		return output;
	}
	else if(operation == "*mov"){ // mov [r64], r64
		output = errorOnLessThanTwo("mov*");
		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(!((destDataSize == 3) && (((sourceInformation & registerInformationSizeMask) >> 6) == 3))){
			output.type = AssemblerLine::type_invalid;
			std::string errorMessage = "moving to and from memory only supported for 64 bit operands";
			for(char c : errorMessage){
				output.data.push_back(c);
			}

			return output;
		}

		bool destIsExtended = ((destinationInformation & registerInformationExtendedMask) >> 3) == true;
		bool srcIsExtended = ((sourceInformation & registerInformationExtendedMask) >> 3) == true;
		output.data.push_back(getREXByte(true, destIsExtended, false, srcIsExtended));
		output.data.push_back(0x89);
		output.data.push_back(getModRMByteIndirect(true, true, (destinationInformation & registerInformationIndexMask), (sourceInformation & registerInformationIndexMask)));

		return output;
	}
	else if(operation == "xchg"){ // xchg r64, r64
		output = errorOnLessThanTwo(operation);

		if(destinationInformation && sourceInformation){
			return registerToRegister(output, destinationInformation, sourceInformation, sourceLine, operation, {0x87});
		}
	}
	else if(operation == "xor"){  // xor r64, r64
		output = errorOnLessThanTwo(operation);

		if(destinationInformation && sourceInformation){
			return registerToRegister(output, destinationInformation, sourceInformation, sourceLine, operation, {0x33});
		}
	}
	else if(operation == "add"){  // add r64, r64
		output = errorOnLessThanTwo(operation);

		if(destinationInformation && sourceInformation){
			return registerToRegister(output, destinationInformation, sourceInformation, sourceLine, operation, {0x01});
		}
	}
	else if(operation == "adc"){  // add r64, r64
		output = errorOnLessThanTwo(operation);

		if(destinationInformation && sourceInformation){
			return registerToRegister(output, destinationInformation, sourceInformation, sourceLine, operation, {0x11});
		}
	}
	else if(operation == "sub"){  // sub r64, r64
		output = errorOnLessThanTwo(operation);

		if(destinationInformation && sourceInformation){
			return registerToRegister(output, destinationInformation, sourceInformation, sourceLine, operation, {0x29});
		}
	}
	else if(operation == "lock"){ //             yes this is a prefix dont mind it being here
		output.data.push_back(0xF0);

		return output;
	}
	else if(operation == "pop"){  // pop r64
		return oneByteOnly({0x58}, operation);
	}
	else if(operation == "push"){ // push r64
		return oneByteOnly({0x50}, operation);
	}
	else if(operation == "cpuid"){
		binary.push_back(0x0F);
		binary.push_back(0xA2);

		for(uint8_t byte : binary){
			output.data.push_back(byte);
		}

		return output;
	}
	else if(operation == "inc"){  // inc r64
		errorOnLessThanOne("inc");

		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(destDataSize == 1){
			output.data.push_back(0x66);
		}

		if((destDataSize == 3) || ((destinationInformation & registerInformationExtendedMask) >> 3)){
			output.data.push_back(getREXByte((destDataSize == 3), false, false, (((destinationInformation & registerInformationExtendedMask) >> 3))));
		}

		output.data.push_back(0xFF);

		output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), 0));

		return output;
	}
	else if(operation == "dec"){  // dec r64
		errorOnLessThanOne("dec");

		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(destDataSize == 1){
			output.data.push_back(0x66);
		}

		if((destDataSize == 3) || ((destinationInformation & registerInformationExtendedMask) >> 3)){
			output.data.push_back(getREXByte((destDataSize == 3), false, false, (((destinationInformation & registerInformationExtendedMask) >> 3))));
		}

		output.data.push_back(0xFF);

		output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), 1));

		return output;
	}
	else if(operation == "div"){  // div r64
		errorOnLessThanOne("div");

		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(destDataSize == 1){
			output.data.push_back(0x66);
		}

		if((destDataSize == 3) || ((destinationInformation & registerInformationExtendedMask) >> 3)){
			output.data.push_back(getREXByte((destDataSize == 3), false, false, ((destinationInformation & registerInformationExtendedMask) >> 3)));
		}

		output.data.push_back(0xF7);
		output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), 6));

		return output;
	}
	else if(operation == "idiv"){ // idiv r64
		errorOnLessThanOne("idiv");

		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(destDataSize == 1){
			output.data.push_back(0x66);
		}

		if((destDataSize == 3) || ((destinationInformation & registerInformationExtendedMask) >> 3)){
			output.data.push_back(getREXByte((destDataSize == 3), false, false, ((destinationInformation & registerInformationExtendedMask) >> 3)));
		}

		output.data.push_back(0xF7);
		output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), 7));

		return output;
	}
	else if(operation == "mul"){  // mul r64
		errorOnLessThanOne("mul");

		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(destDataSize == 1){
			output.data.push_back(0x66);
		}

		if((destDataSize == 3) || ((destinationInformation & registerInformationExtendedMask) >> 3)){
			output.data.push_back(getREXByte((destDataSize == 3), false, false, ((destinationInformation & registerInformationExtendedMask) >> 3)));
		}

		output.data.push_back(0xF7);
		output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), 4));

		return output;
	}
	else if(operation == "imul"){ // imul r64
		errorOnLessThanOne("imul");

		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(destDataSize == 1){
			output.data.push_back(0x66);
		}

		if((destDataSize == 3) || ((destinationInformation & registerInformationExtendedMask) >> 3)){
			output.data.push_back(getREXByte((destDataSize == 3), false, false, ((destinationInformation & registerInformationExtendedMask) >> 3)));
		}

		output.data.push_back(0xF7);
		output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), 5));

		return output;
	}
	else if(operation == "shl"){ // shifts by one bit
		errorOnLessThanOne("shl");

		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(destDataSize == 1){
			output.data.push_back(0x66);
		}

		if((destDataSize == 3) || ((destinationInformation & registerInformationExtendedMask) >> 3)){
			output.data.push_back(getREXByte((destDataSize == 3), false, false, ((destinationInformation & registerInformationExtendedMask) >> 3)));
		}

		output.data.push_back(0xD1);
		output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), 4));

		return output;
	}
	else if(operation == "shr"){ // shifts by one bit
		errorOnLessThanOne("shr");

		if(output.type == AssemblerLine::type_invalid){
			return output;
		}

		if(destDataSize == 1){
			output.data.push_back(0x66);
		}

		if((destDataSize == 3) || ((destinationInformation & registerInformationExtendedMask) >> 3)){
			output.data.push_back(getREXByte((destDataSize == 3), false, false, ((destinationInformation & registerInformationExtendedMask) >> 3)));
		}

		output.data.push_back(0xD1);
		output.data.push_back(getModRMByteNoIndirect((destinationInformation & registerInformationIndexMask), 5));

		return output;
	}
	else if(operation == "clc"){
		output.data.push_back(0xF8);
		return output;
	}
	else if(operation == "stc"){
		output.data.push_back(0xF9);
		return output;
	}
	else if(operation == "cld"){
		output.data.push_back(0xFC);
		return output;
	}
	else if(operation == "std"){
		output.data.push_back(0xFD);
		return output;
	}
	else if(operation == "cli"){
		output.data.push_back(0xFA);
		return output;
	}
	else if(operation == "sti"){
		output.data.push_back(0xFB);
		return output;
	}
	else if(operation == "ret"){
		output.data.push_back(0xC3);
		return output;
	}
	else if(operation == "syscall"){
		output.data.push_back(0x0F);
		output.data.push_back(0x05);
		return output;
	}
	else if(operation == "sysret"){
		output.data.push_back(getREXByte(true, false, false, false));
		output.data.push_back(0x0F);
		output.data.push_back(0x07);
	}
	else if(operation == "hlt"){
		output.data.push_back(0xF4);
		return output;
	}
	else if(operation == "lodsq"){
		output.data.push_back(getREXByte(true, false, false, false));
		output.data.push_back(0xAD);

		return output;
	}
	else if(operation == "tebfinder"){ // mov rax, gs:[0x60]
		output.data.push_back(0x65);
		output.data.push_back(0x48);
		output.data.push_back(0xA1);
		output.data.push_back(0x60);
		output.data.push_back(0x00);
		output.data.push_back(0x00);
		output.data.push_back(0x00);
		output.data.push_back(0x00);
		output.data.push_back(0x00);
		output.data.push_back(0x00);
		output.data.push_back(0x00);

		return output;
	}
	else{ // no such operation
		std::string errorMessage = "no such operation \"";
		errorMessage.append(operation);
		errorMessage.append("\" exists");

		for(char c : errorMessage){
			output.data.push_back(c);
		}

		output.type = AssemblerLine::type_invalid;

		return output;
	}
	return output;
}

AssemblerLine assembleOneInstruction(std::string input){
	return assembleOneInstruction(input, 0);
}

}; // namespace tasm
