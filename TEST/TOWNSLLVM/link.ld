OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    . = 0x0; /* 例: ブートセクタなどのアドレス */

    .text : {
        *(.text)
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}
