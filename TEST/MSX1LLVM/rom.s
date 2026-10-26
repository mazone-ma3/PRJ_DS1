; MSX ROMカートリッジ IPL (llvm-mc / LLVM-Z80 向け)

        .section .text,"ax"      ; ROMヘッダー開始アドレス

; ROMヘッダー (ABで識別、INITアドレスで起動)
        .ascii  "AB"            ; カートリッジ識別子
        .short   INIT           ; 起動アドレス (INITラベル)
        .short   0x0000         ; STATEMENT (使わない)
        .short   0x0000         ; DEVICE
        .short   0x0000         ; TEXT (BASICプログラムなし)
        .short   0x0000, 0x0000, 0x0000 ; 予約

_start:
INIT:   di                      ; 割り込み禁止

        jp       _main
