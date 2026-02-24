#include "ImmediateValue.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace tasm{

ImmediateValue getImmediate(std::string numberToGet, bool is64Bit){
	uint64_t rawValue;
	uint64_t unused;

    ImmediateValue returnThis{.type = ImmediateValue::type_valid};

    if(numberToGet.size() >= 1 && numberToGet.at(0) == '%'){
        returnThis.type = ImmediateValue::type_label_replacement;
        returnThis.needsPatching = true;
        return returnThis;
    }

	if((numberToGet.size() >= 2) && ((numberToGet[1] == 'x') or (numberToGet[1] == 'X'))){
		// hex string
		numberToGet = numberToGet.substr(2);
		try{
			rawValue = std::stoull(numberToGet, &unused, 16);
            if(!is64Bit){
                if(rawValue > 0xFFFFFFFFull){
                    throw std::invalid_argument("");
                }
            }
		}
		catch(std::invalid_argument ia){
			returnThis.type = ImmediateValue::type_invalid;
		}
		catch(std::out_of_range oor){
			returnThis.type = ImmediateValue::type_invalid;
		}

		while(rawValue != 0){
			returnThis.value.push_back(rawValue & 0xFF);
			rawValue = rawValue >> 8;
		}
	}
	else{
		// decimal string
		try{
			rawValue = std::stoull(numberToGet, &unused, 10);
		}
		catch(std::invalid_argument ia){
			returnThis.type = ImmediateValue::type_invalid;
		}
		catch(std::out_of_range oor){
			returnThis.type = ImmediateValue::type_invalid;
		}

		while(rawValue != 0){
			returnThis.value.push_back(rawValue & 0xFF);
			rawValue = rawValue >> 8;
		}
	}

	std::reverse(returnThis.value.begin(), returnThis.value.end());

	return returnThis;
};

}; // namespace tasm
