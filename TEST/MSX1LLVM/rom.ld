/* rom.ld */
MEMORY
{
    ROM (rx)  : ORIGIN = 0x4000, LENGTH = 16K
    RAM (rwx) : ORIGIN = 0xC000, LENGTH = 16K
}

SECTIONS
{
    .text : { *(.text) } > ROM
    
    /* .data セクションの設定 */
    .data : {
        __data_start = .;      /* RAM上の開始アドレス */
        *(.data)
        __data_end = .;        /* RAM上の終了アドレス */
    } > RAM AT > ROM           /* 実行はRAM、配置（ロード）はROM */

    /* ROM上の .data 初期値データの開始位置を取得するシンボル */
    __data_load = LOADADDR(.data);

    /* BSSセクション */
    .bss : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
    } > RAM
}

