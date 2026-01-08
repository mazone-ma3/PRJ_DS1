.8086
DGROUP GROUP _TEXT, _DATA, _BSS

_TEXT SEGMENT BYTE PUBLIC 'CODE'
    assume cs:_TEXT, ds:DGROUP, ss:DGROUP

    public _c_start_:BYTE

    extern main_c_: far        ; C言語の main 関数

start_:
_c_start_:
    ; 1. セグメントの同期
    ; 全セグメントが地続きでロードされている前提
    mov ax, cs
    mov ds, ax
    mov es, ax
;    mov ss, ax
    ; SPはIPLの設定を引き継ぐか、ここで再設定
;    mov	sp,0ffffh

    ; 2. BSS(未初期化領域)のクリア (必要なら)
    ; 3. C言語のメインを呼び出す
    call main_c_

    ; 4. 戻ってきたら無限ループ
exit_loop:
    hlt
    jmp exit_loop
_TEXT ENDS

_DATA SEGMENT WORD PUBLIC 'DATA'
_DATA ENDS
_BSS  SEGMENT WORD PUBLIC 'BSS'
_BSS  ENDS

END start_
