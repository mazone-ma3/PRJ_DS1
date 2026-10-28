; MSX ROMカートリッジ IPL (llvm-mc / LLVM-Z80 向け)

        .EXTERN __data_load
        .EXTERN __data_start
        .EXTERN __data_end
        .EXTERN __bss_start
        .EXTERN __bss_end
        .EXTERN main

        .global _start

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

        ; ==========================================
        ; 1. .data セクションの初期値コピー (ROM -> RAM)
        ; ==========================================
        ; ランタイムでサイズを計算: BC = __data_end - __data_start
        ld   hl, __data_end
        ld   de, __data_start
        xor  a                  ; キャリーフラグをクリア (A = 0, CY = 0)
        sbc  hl, de             ; HL = __data_end - __data_start (サイズ)
        ld   b, h
        ld   c, l               ; BC = サイズ

        ; サイズが 0 の場合はコピーをスキップ
        ld   a, b
        or   c
        jr   z, .data_done

        ld   hl, __data_load   ; HL = コピー元 (ROM上の初期値)
        ld   de, __data_start  ; DE = コピー先 (RAM上の変数領域)
        ldir                   ; 転送実行
.data_done:

        ; ==========================================
        ; 2. BSS セクションの 0 クリア (RAM)
        ; ==========================================
        ; ランタイムでサイズを計算: BC = __bss_end - __bss_start
        ld   hl, __bss_end
        ld   de, __bss_start
        xor  a                  ; キャリーフラグをクリア
        sbc  hl, de             ; HL = __bss_end - __bss_start (サイズ)
        ld   b, h
        ld   c, l               ; BC = サイズ

        ; サイズが 0 の場合はスキップ
        ld   a, b
        or   c
        jr   z, .bss_done

        ld   hl, __bss_start
        ld   de, __bss_start + 1
        ld   (hl), 0            ; 先頭を 0 にする

        dec  bc                 ; 最初の1バイト分を減らす
        ld   a, b
        or   c
        jr   z, .bss_done
        ldir                   ; 残りを 0 で埋める
.bss_done:
        jp       _main
