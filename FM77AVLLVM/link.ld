#/* 1. メモリの定義 (MEMORY) */
MEMORY
{
    ROM (rx)  : ORIGIN = 0x8000, LENGTH = 32K  /* 0x8000 - 0xFFFF をROMとする (Read/Execute) */
    RAM (rwx) : ORIGIN = 0x300, LENGTH = 30K   /* 0x0000 - 0x0FFF をRAMとする (Read/Write/Execute) */
}

#/* 2. セクションの配置 (SECTIONS) */
SECTIONS
{
    /* ROM領域に配置するもの */

    /* RAM領域に配置するもの */
    .text : {
        *(.text)     /* プログラムのコード本体 */
        *(.rodata)   /* 読み取り専用データ（定数文字列など） */
    } > RAM

    .data : {
        *(.data)     /* 初期値ありのグローバル変数 */
    } > RAM

    .bss : {
        *(.bss)      /* 初期値なしのグローバル変数（ゼロクリアされる領域） */
        *(COMMON)
    } > RAM
}

PROVIDE(__dp_base = 0);
PROVIDE(__dp_base_addr = __dp_base << 8);

PROVIDE(__rs0  = __dp_base_addr + 0x08);
PROVIDE(__rs1  = __dp_base_addr + 0x0A);
PROVIDE(__rs2  = __dp_base_addr + 0x0C);
PROVIDE(__rs3  = __dp_base_addr + 0x0E);
PROVIDE(__rs0hi = __rs0); PROVIDE(__rs0lo = __rs0 + 1);
PROVIDE(__rs1hi = __rs1); PROVIDE(__rs1lo = __rs1 + 1);
PROVIDE(__rs2hi = __rs2); PROVIDE(__rs2lo = __rs2 + 1);
PROVIDE(__rs3hi = __rs3); PROVIDE(__rs3lo = __rs3 + 1);


