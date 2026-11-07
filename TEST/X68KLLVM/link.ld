OUTPUT_FORMAT(binary)
SECTIONS
{
    . = 0x00100000;
    .note.GNU-stack : {
         *(.note.GNU-stack)
    }
    .text : {
         *(.text)
    }
    .rodata : {
         *(.rodata)
        *(.rodata.*)
        *(.rodata,*)
    }
    .data : {
        *(.data)
    }
    .bss : {
         *(.bss)
    }
}
