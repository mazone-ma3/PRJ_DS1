OUTPUT_FORMAT(binary)
ENTRY(_start)
SECTIONS
{
    . = 0x00E80000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
