	.globl main

;	.bank	prog

;	.area	.text
	.globl _start

;	org 0x6000

	.text

_start:

;	pshs	d,x,y,u

	LDS  #0x7FFF    ; ハードウェアスタックを$7FFFに設定\n
	LDU  #0x7F00

	jsr	main

;	puls	d,x,y,u
	rts

	.end
; __start

