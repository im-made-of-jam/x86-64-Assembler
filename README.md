when writing assembly that will get turned into an elf, all offsets must have 0x28000 added onto them because this is the base address of the text segment

.include
    copy pastes, C preprocessor style
.insert
    puts bytes with the given value straight into that position
    uses hexadecimal with no prefix
.message
    puts a message on the console when it gets properly processed
