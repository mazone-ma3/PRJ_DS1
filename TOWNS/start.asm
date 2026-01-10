.8086

IO_PIC_PRIMARY_ICW1				EQU	0000H
IO_PIC_PRIMARY_ICW2_3_4_OCW		EQU	0002H
IO_PIC_SECONDARY_ICW1			EQU	0010H
IO_PIC_SECONDARY_ICW2_3_4_OCW	EQU	0012H

IO_PIC_PRIMARY_OCW2		EQU		0000h
IO_PIC_PRIMARY_MASK		EQU		0002h
IO_PIC_SECONDARY_OCW2	EQU		0010h
IO_PIC_SECONDARY_MASK	EQU		0012H

DGROUP GROUP _TEXT, _DATA, _BSS

_TEXT SEGMENT BYTE PUBLIC 'CODE'
    assume cs:_TEXT, ds:DGROUP, ss:DGROUP

    public _c_start_:BYTE

	public set_int41_: far

    extern main_c_: far        ; C言語の main 関数

start_:
_c_start_:
    ; 1. セグメントの同期
    ; 全セグメントが地続きでロードされている前提
    mov ax, cs
    mov ds, ax
    mov es, ax

;	mov	ax,0200h
;    mov ss, ax
    ; SPはIPLの設定を引き継ぐか、ここで再設定
;    mov	sp,0ffffh

;	call set_int41_

    ; 2. BSS(未初期化領域)のクリア (必要なら)
    ; 3. C言語のメインを呼び出す
    call main_c_

    ; 4. 戻ってきたら無限ループ
exit_loop:
    hlt
    jmp exit_loop


set_int41_:


INTAEH_INIT:
	; IF=0 because it is called from INT AEH handler.

	MOV		AL,19H
	mov		dx,IO_PIC_PRIMARY_ICW1
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,40H
	mov		dx,IO_PIC_PRIMARY_ICW2_3_4_OCW
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,80H
	mov		dx,IO_PIC_PRIMARY_ICW2_3_4_OCW
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,1DH
	mov		dx,IO_PIC_PRIMARY_ICW2_3_4_OCW
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,0FFH
	mov		dx,IO_PIC_PRIMARY_ICW2_3_4_OCW
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,19H
	OUT		IO_PIC_SECONDARY_ICW1,AL
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,48H
	mov		dx,IO_PIC_SECONDARY_ICW2_3_4_OCW
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,87H
	mov		dx,IO_PIC_SECONDARY_ICW2_3_4_OCW
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,19H
	mov		dx,IO_PIC_SECONDARY_ICW2_3_4_OCW
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	MOV		AL,0FFH
	mov		dx,IO_PIC_SECONDARY_ICW2_3_4_OCW
	out		dx,AL
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc






    ; INT41hベクタ登録
    xor     ax, ax
    mov     es, ax
    mov     ax, offset keyboard_irq_handler
    mov     bx, 41h * 4     ; INT41hのIVTオフセット
;    mov     bx, 21h * 4     ; INT41hのIVTオフセット
    cli
    mov     es:[bx], ax     ; オフセット
    mov     ax, cs
    mov     es:[bx+2], ax   ; セグメント


	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

	mov	dx,000h+2
	in	al,dx ;0000h+2

;	movb	_saveIMR_M,al

	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc

;	mov	00100000b,%dl
;	mov	00001000b,%dl
	mov	dl,02h
	xor	dl,0ffh
	and	al,dl
	mov	dx,0000h+2
	out	dx,al	; 割り込み許可

	mov	dx,604h
	mov	al,1
	out	dx,al		;キーボード割り込み許可

    sti

	ret


keyboard_irq_handler:
    push    ax
    push    bx
    push    cx
    push    dx
    push    ds

    mov     ax, cs          ; DS=CS (key_matrixアクセス用)
    mov     ds, ax

;	hlt

	mov    dx,602h	; スタースタレジスタ
	in	al,dx
	test al,01h		; OBF(データ有無)をチェック
	jz	end_handler

    ; ポート0x600から1バイト目読む
    mov     dx, 600h
    in      al, dx
;    cmp     al, 80h         ; bit7=1か？（有効データ）
;    jnb      end_handler    ; 無効なら終了（念のため）

    test     al, 80h         ; bit7=1か？（有効データ）
    jz      end_handler    ; 無効なら終了（念のため）

    mov     ah, al          ; AH=1バイト目（フラグ保存）
    in      al, dx          ; AL=2バイト目（キーコード）

	mov		bl,ah
	and	bl,0f0h
	cmp	bl,0f0h
	jz	make		; タイパマチックデータ

normal:
    ; Make/Break判定（bit6 of AH: 仮に1=Break。Tsugaruでテストして逆ならjz/je反転）

    test    ah, 10h         ; bit4=1ならBreak
	mov	ah,0
    jnz     break

make:
    mov bl, al
    mov cl,4
    shr bl, cl           ; BL = 0x04 (上位4bit)
    
    ; 2. 下位4ビットを抽出してビット位置にする
    and al, 0Fh         ; AL = 0x0B (下位4bit = 11番目のビット)

    mov cx, ax          ; CLにシフト回数をセット (ALを使用)
    mov ax, 1           ; AX = 0000 0000 0000 0001
    shl ax, cl


    ; 3. 配列のアドレス計算 ;(word単位なので index * 2)
    xor bh, bh
    shl bx, 1           ; インデックスを2倍 (wordサイズ調整)

    mov     cx, offset _key_matrix

	add	bx,cx

    or      word ptr ds:[bx], ax    ; ビットON（押下中）

	jmp	eoi

break:

;    add     bx, ax          ; BX=キーコードのバッファアドレス

    mov bl, al
    mov cl,4
    shr bl, cl           ; BL = 0x04 (上位4bit)
    
    ; 2. 下位4ビットを抽出してビット位置にする
    and al, 0Fh         ; AL = 0x0B (下位4bit = 11番目のビット)

    mov cx, ax          ; CLにシフト回数をセット (ALを使用)
    mov ax, 1           ; AX = 0000 0000 0000 0001
    shl ax, cl

	xor	ax, 0ffffh

    ; 3. 配列のアドレス計算 ;(word単位なので index * 2)
    xor bh, bh
    shl bx, 1           ; インデックスを2倍 (wordサイズ調整)

    mov     cx, offset _key_matrix

	add	bx,cx

    and     word ptr ds:[bx], ax   ; ビットOFF

eoi:
	mov	dx,006ch
	out	dx,al ; 1μ秒ウェイト
	cmc
	cmc


;    mov     al, 20h         ; Non-specific EOI
;    out     20h, al

	mov	al,20h
	mov	dx,0
	out	dx,al	;/* EOI(Master) */

end_handler:

;	mov	al,20h
;	mov	dx,0
;	out	dx,al	;/* EOI(Master) */


    pop     ds
    pop     dx
    pop     cx
    pop     bx
    pop     ax
    iret


_TEXT ENDS

_DATA SEGMENT WORD PUBLIC 'DATA'

public _key_matrix : far

_key_matrix: db 128 dup(0)  ; キーコード0-127のビットマップ（bit0=押下中）

NUM_INTS				EQU		32
INT_DATA_BLOCKS:		DD		NUM_INTS dup (0)

INT_ENABLE_FLAGS:		DD		0

_DATA ENDS
_BSS  SEGMENT WORD PUBLIC 'BSS'
_BSS  ENDS

END start_
