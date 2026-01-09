| crt0.s
.text
.global _start

_start:
    | スタックポインタを安全な場所に設定 (例: $002000)
|    move.l  #0x002000, %sp
    | main関数へジャンプ
    jsr     main
    | 戻ってきたら無限ループ
loop:
    bra     loop
