when writing assembly that will get turned into an elf, all offsets must have 0x28000 added onto them because this is the base address of the text segment, according to the header that gets copy / pasted in

.include
    copy pastes, C preprocessor style
.insert
    puts bytes with the given value straight into that position
    uses hexadecimal with no prefix
.message
    puts a message on the console when it gets properly processed

callr
    callr will never generate with a rex byte, but may have a shortening prefix byte
    for example it will never be "48 ff d0", but can be either "ff d0" or "66 ff d0"
    this is dependant on which register is used in the source line
    for example, "callr rax" and "callr eax" will both generate "ff d0", but "callr ax" will generate "66 ff d0"
