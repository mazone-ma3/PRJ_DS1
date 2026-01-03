; éQçl inufutoéÅÇÃLoader.asm(Ç‹ÇÒÇ‹)

;include "BinSize.inc"

size = 23000 ;22707
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

	.end __start
