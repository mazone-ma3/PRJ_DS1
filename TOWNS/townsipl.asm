; Minimal FM TOWNS FD IPL for OpenWatcom WASM
; 参考 Free TOWNSOS / iosys / FD_IPL.NSM


.186					; 80186命令許可（TOWNSは386だけどリアルモードなのでOK）

NAME	FD_IPL

DATASEG SEGMENT PUBLIC BYTE 'DATA'
	db "IPL4"		   ; offset 0000h: IPL4 magic

	org 0004h
	jmp start		   ; short jmpは自動計算
	nop

	org	000bh		; BPBh
	dw	1024
	db	1
	dw	1
	db	2
	dw	0c0h
	dw	04d0h
	db	0feh
	dw	2
	dw	8
	dw	2
	dd	0
	dd	0

;	org 001Eh		   ; BPB領域スキップ
	; Fake BPB (必要なら埋める)

	org 0020h
io_sys_lba  dd 000000001h   ; IO.SYS開始セクタ (例: sector 1)
io_sys_secs dd 000000040h   ; セクタ数 (64 sectors)

DATASEG ENDS

CODESEG SEGMENT PUBLIC BYTE USE16 'CODE'
	ASSUME CS:CODESEG, DS:DATASEG, ES:NOTHING, SS:NOTHING

	org	005eh

start:
	cmp	bl,2 ; BOOTDEV_FD
	je	boot
	retf

boot:
	cli
;	xor	 ax, ax
	mov	 ax, cs
	mov	 ds, ax
	mov	 es, ax
	mov	 ss, ax
	mov	 sp, 0B000h	  ; ロード領域の下にスタック
	sti

	; 画面FILL (VRAM C0000h - を青で塗りつぶし)
	mov	al,000h	; VRAM
	mov dx,0404h
	out	dx,al

	mov	al,67h; 0l100111b	; all plane display
	mov	dx,0ff82h
	out	dx,al

	mov	al,00000001b	; C0 plane write
	mov	dx,0ff81h
	out	dx,al

	mov	al,00000000b	; vram page 0 write
	mov	dx,0ff83h
	out	dx,al

loop2:
	mov	 ax, 0C000h
	mov	 es, ax
	xor	 di, di
	mov	 cx, 4000h	   ; 16K words = 32KB
	mov	 ax, 0fFfFh
	rep	 stosw

;	jmp	loop2

	; FAR CALLでディスク読み込み (0xFFFB:0x0014)
	mov	 ax, 01000h	   ; 本体ロード先セグメント
	mov	 ds, ax
	xor	 di, di		  ; オフセット0

	mov	 al, 020h		 ; 読み込み関数（リバース準拠の仮値）

	mov	 cx, 0 ; Cylinder ;word ptr io_sys_lba
	mov	 dh, 0 ; Head ;word ptr io_sys_lba+2
	mov	 dl, 2	;Sector

	mov	si,60

read_loop:
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di

	mov	 ah,5	; READ SECTOR
	mov	 bx,1	; NUMNER OF SECTORS

;	call	far ptr 0FFFBh:0014h
	db	 9ah
	dw	 0014h
	dw	 0fffbh

	and	ah,ah
	jne	load_error
;	jmp	$

	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax

	add	di,1024

	inc	dl
	cmp	dl,8		; Sector 8
	jbe	no_next_track

	mov	dl,1
	inc	dh		; Head Change
	cmp	dh,2
	jb	no_next_track

	mov	dh,0
	inc	cx			; Cylinder

no_next_track:
	DEC	 SI			  ; 残りセクタ数 -1
	JNZ	 read_loop	   ; 0になるまでループ

	; 本体へジャンプ（通常は0040:0000）
	db	0eah
	dw	0000h
	dw	1000h

load_error:
	; エラー時FILL
	push	cx
	push	dx

	mov	al,00000010b	; C1 plane write
	mov	dx,0ff81h
	out	dx,al

	mov	 ax, 0C000h
	mov	 es, ax
	xor	 di, di
	mov	 cx, 4000h
	mov	 ax, 0ffFfh	   ;
	rep	 stosw

	pop	dx
	pop	cx

	mov	ax,0e000h
	mov	es,ax
	cli
	hlt

CODESEG ENDS

END start