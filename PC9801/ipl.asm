	.8086				   ; 8086/V30向け（PC-9801は8086互換）
	NAME	ipl98

_TEXT   SEGMENT BYTE PUBLIC 'CODE'
		ASSUME  CS:_TEXT, DS:NOTHING, SS:NOTHING

	ORG	 0			   ; ブートセクタはアドレス0から
	jmp short entry
;	nop

	; ブートセクタのBPB（BIOS Parameter Block） - PC-98 1.23MB FD用簡易版
	ORG	 2				; JMPの後からBPB
	DB	  90h			; NOP（互換性）
	DB	  "HELLO98 "	; OEM名（8バイト）
	DW	  1024			; セクタサイズ（バイト）
	DB	  1				; クラスタサイズ（セクタ数）
	DW	  4				; 予約セクタ数（IPLサイズ分）
	DB	  2				; FAT数
	DW	  512			; ルートディレクトリエントリ数
	DW	  1232			; 総セクタ数（1.23MBの場合）
	DB	  0FEh			; メディア記述子
	DW	  4				; FATセクタ数
	DW	  16			; トラックあたりセクタ数
	DW	  2				; ヘッド数
	DD	  0				; 隠しセクタ数
	DD	  0				; 大きいディスク用総セクタ数

entry:  
	CLI					 ; 割り込み禁止

;	mov ah, 17h	; buzzer
;	int 18h

	mov ah, 0ah	; crt mode set
	mov al,00000000b
	int 18h

	mov ah, 0ch	; text on
	int 18h

;	mov ah, 11h	; cursor on
;	int 18h

	mov ah, 16h	; text fill
	mov dh,0e1h
	mov dl,20h
	int 18h

; テキストVRAM（文字面）のセグメント A000h を設定
	mov ax, 0a000h
	mov es, ax
	
	; 文字 '!' を書き込む (オフセット 0 = 画面左上)
	mov byte ptr es:[0], 21h   ; '!'

	; 属性面 A200h を設定（これがないと黒文字で見えない）
;	mov ax, 0a200h
;	mov es, ax
;	mov byte ptr es:[0], e1h   ; 07h = 白 (通常表示)

	; ここで止めて生存確認
;	jmp $


	XOR	 AX, AX
	MOV	 SS, AX			; スタックセグメントを0に
	MOV	 SP, 7C00h		; スタックポインタをブート位置に（安全のため）
	STI					 ; 割り込み許可

	; データセグメントはブートセグメントと同じ（ORG 0なのでDS=07C0h相当）
	MOV	 AX, CS
	MOV	 DS, AX


	MOV	 SI, 60			; 読み込むセクタ数（調整して。20で約20KB）
	MOV	 BX, 1024		; バッファサイズ

	MOV	 AX, 1000h		;
	MOV	 ES, AX
	MOV	 BP, 0000h		; バッファ開始 1000:0000h（安全領域）

	MOV	 CL, 0			; 開始シリンダ 0
	MOV	 DH, 0			; 開始ヘッド 0
	MOV	 DL, 2			; 開始セクタ 2（IPLの次から）

	MOV	 AH, 076h		; 固定
	MOV	 AL, 090h		; DA_UAモード

	MOV	 CH, 03h		 ; セクタ長固定 1024バイト (2^(3+7)=1024)

read_loop:
	push	si
	push	ax
	push	bx
	push	cx
	push	dx
	INT	 1Bh

	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	si

	JC	  read_error	  ; Carry立ったらエラー

;	jp no_next_track

	; 次バッファ準備
	ADD	 BP, BX;0400h	   ; +1024バイト（次セクタ位置）

	; 次セクタ計算
	INC	 DL			  ; セクタ +1
	CMP	 DL, 8 ;16		  ; トラックあたりセクタ数（15 or 16？ 1.23MBは通常15+1=16まで）
	JBE	 no_next_track

	MOV	 DL, 1		   ; セクタ1に戻す
	INC	 DH			  ; 次ヘッド
	CMP	 DH, 2
	JB	  no_next_track

	MOV	 DH, 0		   ; ヘッド0に戻す
	INC	 CL			  ; 次シリンダ

no_next_track:

	DEC	 SI			  ; 残りセクタ数 -1
	JNZ	 read_loop	   ; 0になるまでループ


	; メインプログラム（OpenWatcomで作成）へジャンプ

	mov ax, cs
	mov ds, ax
	mov es, ax
;	mov ss, ax
;	mov sp, 1fc0h   ; スタックポインタ
	sti


	db 0EAh
	dw 0000h		; オフセット
	dw 1000h		; セグメント

read_error:
	mov ah, 17h	; buzzer
	int 18h
	jmp	$

	ORG	 510			 ; 512バイト目（PC/AT互換のシグネチャ位置）
	DW	  0AA55h		  ; ブートシグネチャ（PC-98でも認識される）

	; PC-98特有の拡張BPB（必要に応じて調整）
	DB	  29h			 ; 拡張ブートシグネチャ
	DD	  0FFFFFFFFh	   ; ボリュームシリアル
	DB	  "NO NAME	"   ; ボリュームラベル（11バイト）
	DB	  "FAT12   "	  ; ファイルシステム（8バイト）

_TEXT	ENDS

		END	 entry
