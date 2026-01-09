OUTPUT_FORMAT(binary)
SECTIONS
{
    . = 0x00100000;
    .text : { *(.text) }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
}
