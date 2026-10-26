/* rom.ld */
MEMORY
{
    ROM (rx)  : ORIGIN = 0x4000, LENGTH = 16K
    RAM (rwx) : ORIGIN = 0xC000, LENGTH = 16K
}

SECTIONS
{
    /* 読み取り専用コードや定数は ROM (0x4000?) に配置 */
    .text : {
        *(.text)
        *(.text.*)
    } > ROM

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > ROM

    /* 書き込み可能な変数などは RAM (0xC000?) に配置 */
    .data : {
        *(.data)
        *(.data.*)
    } > RAM

    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    } > RAM
}