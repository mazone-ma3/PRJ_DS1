; 参考 inufuto氏のLoader.asm(まんま)

;include "BinSize.inc"

size = 24000 ;22707
;0x4353
Sector = 256
count = (size+Sector-1)/Sector

top= 0x1600
;top= 0x2000

.bank	prog

.area  .text
; CODE (ABS)
;.org 0xc000

;.area .text

	.globl __start

;cseg
__start:
;	bra	__start

	LDS  #0x7bFF    ; ハードウェアスタックを$7bFFに設定(TWR 0x7C00-)
	LDU  #0x7b00

	lda #02
	sta 0xfd13	;サブモニタROMをBに
	lda #0x40
	sta 0xfd12	;サブモニタROMをBに

;	jmp LP03

;	MMR ON

	LDA 0xfd93
	anda #0x7f		; MMR OFF
	sta 0xfd93
	clra
	ldx	#MMRDT
LP01:
	sta	0xfd90
	ldy	#0xfd80
LP02:
	ldb	,X+
	stb	,Y+
	cmpy #0xfd90
	bne	LP02
	inca
	cmpa #4
	bne	LP01

LP03:
	clr	0xfd90
	lda	0xfd93
	ora	#0x80		; MMR ON
	sta	0xfd93



	LDX #top	  ; 開始アドレス
	LDD #0x0000	  ; Dレジスタ(A+B)を0にする
LOOP:
	STD ,X++		; 16ビット一気に書き込み、Xを+2する
	CMPX #0x8000
	BNE LOOP

	lda #24
	sta RQNO
	ldx #RCB
	jsr [0xfbfa]

	lda #10			; BIOS DREAD
	sta RQNO

	lda #0
	sta RCBTRK		; TRACK
;	clr RCBTRK

	lda #2
	sta RCBSCT		; SECTOR
	clr RCBSID		; SIDE 0
	clr RCBUNT		; DRIVENO 0
	ldy #top
	ldb #count

loop:
	sty RCBDBA		; Data Buffer Top
	ldx #RCB
	pshs b,y
	jsr [0xfbfa]
	puls b,y
	lda RCBSCT
	inca
	cmpa #16+1

	bcs	end1
;	if cc
	lda RCBSID
	inca
	cmpa #2

	bcs	end2
;		if cc
	inc RCBTRK
	clra
;	endif
end2:
	sta RCBSID
	lda #1
;		endif
end1:

	sta RCBSCT
	leay Sector,y
	decb
	bne loop
;	while ne | wend

jmp top
;loop0:
;	jmp	loop0

;.area data
;dseg
RCB:
RQNO: 
	.byte 0
RCBSTA:
	.byte 0
RCBDBA:
	.word 0
RCBTRK:
	.byte 0
RCBSCT:
	.byte 0
RCBSID:
	.byte 0
RCBUNT:
	.byte 0

MMRDT:
	.byte	0x30,0x31,0x32,0x33	; MSR=0
	.byte	0x34,0x35,0x36,0x37
	.byte	0x04,0x39,0x3a,0x3b
	.byte	0x3c,0x3d,0x3e,0x3f
	.byte	0x10,0x11,0x12,0x13	; MSR=1
	.byte	0x34,0x35,0x36,0x37
	.byte	0x38,0x39,0x3a,0x3b
	.byte	0x3c,0x1d,0x3e,0x3f
	.byte	0x20,0x21,0x22,0x23	; MSR=2
	.byte	0x34,0x35,0x36,0x37
	.byte	0x38,0x39,0x3a,0x3b
	.byte	0x3c,0x3d,0x3e,0x3f
	.byte	0x00,0x01,0x02,0x03	; MSR=3
	.byte	0x34,0x35,0x36,0x37
	.byte	0x38,0x39,0x3a,0x3b
	.byte	0x3c,0x3d,0x3e,0x3f


	.end __start
