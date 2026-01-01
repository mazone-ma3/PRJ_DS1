	.globl _main

	.bank	prog

	.area	.text
	.globl __start

__start:

	jsr	_main
	rts

	.end __start

