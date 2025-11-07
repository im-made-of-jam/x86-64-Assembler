// g++ ./testing.cpp -o ./testing -nostdlib -nostartfiles -Wno-pointer-arith -fPIC -e_start

#define lodsq rax = *(reinterpret_cast<uint64_t*>(rsi)); rsi += 8
#define xchgRaxRsi temp = rax; rax = rsi; rsi = temp

#define uint64_t unsigned long long
#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t  unsigned char
#define HANDLE void*

void* kernel32DLLBase = nullptr;

__stdcall __attribute__((noreturn)) void(*ExitProcess2)(uint32_t);

void* getKernel32Base(){
	uint64_t rsi;
	uint64_t rbx;
	uint64_t temp;

	uint64_t rax;
	asm("movq %%gs:[0x60],%0" : "=r"(rax));

	rax = *(uint64_t*)(((void*)rax) + 0x18); // mov rax, [rax + 0x18]   ; PEB->Ldr
    rsi = *(uint64_t*)(((void*)rax) + 0x20); // mov rsi, [rax + 0x20]   ; Ldr->InMemoryOrderModuleList (second)
    lodsq;
    xchgRaxRsi;
    lodsq;
    return (void*)(*(uint64_t*)(((void*)rax) + 0x20)); // mov rbx, [rax + 0x20]   ; DllBase (base address of kernel32.dll)
}

inline uint16_t getPEOffset(void* dllBase){
	return *reinterpret_cast<uint16_t*>(dllBase + 0x3C);
}

struct __attribute__((packed)) ImageDataDirectory{
	uint32_t virtualAddress;
	uint32_t size;
};

struct __attribute__((packed)) PE64Info{
	// COFF header
	uint32_t PEIdentifier; // {'P', 'E', '\0', '\0'}
	uint16_t machineType;
	uint16_t numberOfSections;
	uint32_t dateTimeStamp;
	uint32_t pointerToSymbolTable;
	uint32_t numberOfSymbols;
	uint16_t sizeOfOptionalHeader;
	uint16_t characteristics;

	// optional header
	uint16_t magic;
	uint8_t  majorLinkerVersion;
	uint8_t  minorLinkerVersion;
	uint32_t sizeOfCode;
	uint32_t sizeOfInitializedData;
	uint32_t sizeOfUninitializedData;
	uint32_t addressOfEntryPoint;
	uint32_t baseOfCode;

	// windows specific
	uint64_t imageBase;
	uint32_t sectionAlignment;
	uint32_t fileAlignment;
	uint16_t majorOperatingSystemVersion;
	uint16_t minorOperatingSystemVersion;
	uint16_t majorImageVersion;
	uint16_t minorImageVersion;
	uint16_t majorSubsystemVersion;
	uint16_t minorSubsystemVersion;
	uint32_t win32VersionValue;
	uint32_t sizeOfImage;
	uint32_t sizeOfHeaders;
	uint32_t checksum;
	uint16_t subsystem;
	uint16_t dllCharacteristics;
	uint64_t sizeOfStackReserve;
	uint64_t sizeOfStackCommit;
	uint64_t sizeOfHeapReserve;
	uint64_t sizeOfHeapCommit;
	uint32_t loaderFlags;
	uint32_t numberOfRvaAndSizes;

	// table data
	uint32_t exportTableAddr;
	uint32_t exportTableSize;
	uint32_t importTableAddr;
	uint32_t importTableSize;
	uint32_t resourceTableAddr;
	uint32_t resourceTableSize;
	uint32_t exceptionTableAddr;
	uint32_t exceptionTableSize;
	uint32_t certificateTableAddr;
	uint32_t certificateTableSize;
	uint32_t baseRelocationAddr;
	uint32_t baseRelocationSize;
	uint32_t debugDataAddr;
	uint32_t debugDataSize;
	uint32_t architectureDataAddr; // reserved, must be 0
	uint32_t architectureDataSize; // reserved, must be 0
	uint32_t globalPtrAddr;
	uint32_t globalPtrSize;        // must be 0
	uint32_t TLSTableAddr;
	uint32_t TLSTableSize;
	uint32_t loadConfigTableAddr;
	uint32_t loadConfigTableSize;
	uint32_t boundImportTableAddr;
	uint32_t boundImportTableSize;
	uint32_t IATAddr;
	uint32_t IATSize;
	uint32_t delayImportDescriptorAddr;
	uint32_t delayImportDescriptorSize;
	uint32_t CLRRuntimeHeaderAddr;
	uint32_t CLRRuntimeHeaderSize;
	uint64_t reservedMustBeZero;
};

PE64Info* Kernel32;

bool strEquals(char* first, char* second){
	int i = 0;
	while(first[i]){
		if(!(second[i])){
			return false;
		}
		if(first[i] != second[i]){
			return false;
		}
	}

	return true;
}

void* getFunctionFromDLL(void* dllBase, char* name){
	uint32_t offset = getPEOffset(dllBase);
	void* peBase = dllBase + offset;

	PE64Info* dllImage = reinterpret_cast<PE64Info*>(peBase);

	void* exportTableStart = (dllBase + dllImage->exportTableAddr);

	// export directory table
	uint32_t exportFlags           = *reinterpret_cast<uint32_t*>(exportTableStart + 0); // reserved, should be 0
	uint32_t dateTimeStamp         = *reinterpret_cast<uint32_t*>(exportTableStart + 4);
	uint16_t majorVersion          = *reinterpret_cast<uint16_t*>(exportTableStart + 8);
	uint16_t minorVersion          = *reinterpret_cast<uint16_t*>(exportTableStart + 10);
	char*    nameRVA               =  reinterpret_cast<char*>(*reinterpret_cast<uint32_t*>(exportTableStart + 12));
	uint32_t ordinalBase           = *reinterpret_cast<uint32_t*>(exportTableStart + 16);
	uint32_t addressTableEntries   = *reinterpret_cast<uint32_t*>(exportTableStart + 20);
	uint32_t numberOfNamePointers  = *reinterpret_cast<uint32_t*>(exportTableStart + 24);
	uint32_t exportAddressTableRVA = *reinterpret_cast<uint32_t*>(exportTableStart + 28);
	uint32_t namePointerRVA        = *reinterpret_cast<uint32_t*>(exportTableStart + 32);
	uint32_t ordinalTableRVA       = *reinterpret_cast<uint32_t*>(exportTableStart + 36);

	for(uint64_t i = 0; i < numberOfNamePointers; ++i){
		bool isForwader = (*reinterpret_cast<uint32_t*>(exportTableStart + i*4)) < reinterpret_cast<uint64_t>(exportTableStart);

		if(isForwader){
			//TODO: this at some point

			// return nullptr;
		}

		void* symbolAddr    = reinterpret_cast<void*>(exportAddressTableRVA + (i*4));
		char* symbolNamePtr = reinterpret_cast<char*>(namePointerRVA + (i*4));

		if(strEquals(name, symbolNamePtr)){
			return symbolAddr;
		}
	}

	return nullptr;

	ExitProcess2(numberOfNamePointers);
	// stops a compiiler warning for now
	__builtin_unreachable();
}

extern "C" void _start(){
	kernel32DLLBase = getKernel32Base();
	Kernel32 = reinterpret_cast<PE64Info*>(kernel32DLLBase + getPEOffset(kernel32DLLBase));

	ExitProcess2 = reinterpret_cast<void(*)(uint32_t)>(kernel32DLLBase + 268448);

	void* function = getFunctionFromDLL(kernel32DLLBase, "GetStdHandle");

	if(function == nullptr){
		ExitProcess2(420);
	}

	ExitProcess2(*((unsigned int*(*)(int))function)(-10));

	ExitProcess2(getPEOffset(kernel32DLLBase));
}
