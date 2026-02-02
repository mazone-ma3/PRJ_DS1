/* ds1.c for GCC6809 FM77AV  m@3 */
/* キャラを出す SUB CPUコマンド版 */

#include "mode.h"

#include "inkey.h"
#include "ds1_grp.h"
#include "font.h"

#define A_KEY "Z"
#define B_KEY "X"

void abort(void)
{
	for(;;);
}

/* BITデータ算出 */
#define BITDATA(n) (1 << (n))

/* BITセット */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BITクリア */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}
#define PAGE 0xa

#define SIZE 80

#define X_SIZE 18
#define Y_SIZE 18

#define PARTS_X 2
#define PARTS_Y 8


#define MAXCOLOR 8

/* BRG */

unsigned char org_pal[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 0, 0, 0 },//	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};



unsigned char *vram_adr;

unsigned char active_page;

unsigned char i, j;

#define OPNCOM 0xFD15
#define OPNDAT 0xFD16

#define VRAM_DATA_ADR (PAGE * 0x1000)

#define sub_vram ((volatile unsigned char *)VRAM_DATA_ADR)
#define mmr ((volatile unsigned char *)0xFD80)
#define mem ((volatile unsigned char *)0x6AFF)
#define mem2 ((volatile unsigned char *)0xfd12)
#define mem3 ((volatile unsigned char *)0xfd37)
#define msr ((volatile unsigned char *)0xFD93)
#define keyport ((volatile unsigned char *)0xFD01)
#define opncom ((volatile unsigned char *)OPNCOM)
#define opndat ((volatile unsigned char *)OPNDAT)

#define subinter ((volatile unsigned char *)0xFD04)

unsigned char msr_sv;
unsigned char key, st3 = 0;

void keyscan_on(void)
{
asm volatile(
	"jmp	_SCAN\n"
"_SUBOUT:\n"
	".byte	16\n"
	".blkb	1\n"
	".word	_KEYCON\n"
	".word	6\n"
	".byte	2\n"
"_KEYCON:\n"
	".blkb	2\n"
	".byte	0x45\n"
	".byte	0\n"
	".byte	2\n"
	".blkb	1\n"
"_SCAN:\n"
	"ldx	#_SUBOUT\n"
	"jsr [0xFBFA]\n"
);
}

void keyscan_off(void)
{
asm volatile(
	"ldx	#_SUBOUT2\n"
	"jsr [0xFBFA]\n"
	"lda #0\n"
	"sta _KEYMODE\n"
	"bra	_ENDOFF\n"

"_SUBOUT2:\n"
	".byte	16\n"
	".blkb	1\n"
	".word	_KEYCON2\n"
	".word	6\n"
	".blkb	2\n"

"_KEYCON2:\n"
	".blkb	2\n"
	".byte	0x45\n"
	".byte	0x00\n"
"_KEYMODE:\n"
	".byte	0x00\n"
	".blkb	1\n"

"_ENDOFF:\n"
);
}

void keyrepeat_on(void)
{
asm volatile(
	"jmp	_repeat\n"
"_SUBOUT3:\n"
	".byte	16\n"
	".blkb	1\n"
	".word	_KEYREP\n"
	".word	6\n"
	".byte	2\n"
"_KEYREP:\n"
	".blkb	2\n"
	".byte	0x45\n"
	".byte	4\n"
	".byte	0\n"
	".blkb	1\n"
"_repeat:\n"
	"ldx	#_SUBOUT3\n"
	"jsr [0xFBFA]\n"
);
}

void keyrepeat_off(void)
{
asm volatile(
	"jmp	_repeat2\n"
"_SUBOUT4:\n"
	".byte	16\n"
	".blkb	1\n"
	".word	_KEYREP2\n"
	".word	6\n"
	".byte	2\n"
"_KEYREP2:\n"
	".blkb	2\n"
	".byte	0x45\n"
	".byte	4\n"
	".byte	1\n"
	".blkb	1\n"
"_repeat2:\n"
	"ldx	#_SUBOUT4\n"
	"jsr [0xFBFA]\n"
);
}


unsigned char keycode2, keyflag;

void  key_sense(void)
{
//		for(;;);
//	unsigned char keycode2 = 0, old_keycode = 0;
//	while((keycode = *keyport) != old_keycode){
		keycode2 = *keyport;
		if((keycode2 == 0x4d) || (keycode2 == 0x3b)){
			BITSET(0, st3);
			BITCLR(1, st3);
		}else if((keycode2 == 0xcd) || (keycode2 == 0xbb)){
			BITCLR(0, st3);
		}else if((keycode2 == 0x51) || (keycode2 == 0x40)){
			BITSET(3, st3);
			BITCLR(2, st3);
		}else if((keycode2 == 0xd1) || (keycode2 == 0xc0)){
			BITCLR(3, st3);
		}else if((keycode2 == 0x50) || (keycode2 == 0x43)){
			BITSET(1, st3);
			BITCLR(0, st3);
		}else if((keycode2 == 0xD0) || (keycode2 == 0xc3)){
			BITCLR(1, st3);
		}else if((keycode2 == 0x4f) || (keycode2 == 0x3e)){
			BITSET(2, st3);
			BITCLR(3, st3);
		}else if((keycode2 == 0xcf) || (keycode2 == 0xbe)){
			BITCLR(2, st3);
		}else if((keycode2 == 0x2a) || (keycode2 == 0x58) || (keycode2 == 0x35)){
			BITSET(4, st3);
		}else if((keycode2 == 0xaa) || (keycode2 == 0xd8) || (keycode2 == 0xb5)){
			BITCLR(4, st3);
		}else if((keycode2 == 0x2b)){
			BITSET(5, st3);
		}else if((keycode2 == 0xab)){
			BITCLR(5, st3);
		}
//		old_keycode = keycode2;
//	}
}


void set_key_irq(void)
{
asm volatile(

"_TINIT:\n"
	"orcc	#0x10\n"
	"ldx		0xFFF8\n"
	";cmpx	#_IRQET\n"
	";beq		_TCLOSE\n"
	"stx		_IRQJP\n"
	"ldx		#_IRQET\n"
	"stx		0x0FFF8\n"

	"lda		0xFD03\n"
	"anda	#0x01\n"
	"ora	#1\n"
	"sta		0xFD02\n"

"_TCLOSE:\n"
	"bra	_ENDIRQ\n"
);
}

void reset_key_irq(void)
{
asm volatile(
	"orcc	#0x10\n"
	"ldx		_IRQJP\n"
	"stx		0xFFF8\n"
	"bra	_ENDIRQ\n"

"_IRQET:\n"
	";rti\n"
	";jmp		[_IRQJP]\n"

	"pshs	D,X\n"
	"lda		0xFD03\n"
	"anda	#1\n"
	";beq		_IRQRT\n"

	"jsr	_key_sense\n"

"_IRQRT:\n"
	"puls	D,X\n"
	"rti\n"
	"jmp		[_IRQJP]\n"

"_IRQJP:	.blkb	2\n"
"_TCOUNT:	.blkb	1\n"
"_ENDIRQ:\n"
	"andcc	#0xEF\n"
);

}


void sub_disable(void)
{
asm volatile(
"_SUBHLT:\n"
	"lda	0xFD05\n"
	"bmi	_SUBHLT\n"
	"orcc	#(0x50)\n"
	"lda	#0x80\n"
	"sta	0xFD05\n"
"_LOOP:\n"
	"lda	0xFD05\n"
	"bpl	_LOOP\n" // *-3\n"
);

//	msr_sv = *msr;
asm volatile(
	"lda	0xfd93\n"
	";sta	_msr_sv\n"
	"ora	#0x80\n"
	"sta	0xfd93\n"
);
//	*msr |= 0x80;
}

void sub_enable(void)
{
//	*msr = msr_sv;

asm volatile(
	"lda	_msr_sv\n"
	";lda	0xfd93\n"
	"anda	#0x7f\n"
	"sta	0xfd93\n"

	"ldb	0xFC80\n"
	"orb	#0x80\n"
	"stb	0xFC80\n"

	"clr	0xFD05\n"
	"andcc	#0xAF\n"
);
}

void bank1_on()
{
asm volatile(
	"lda	#0x1d\n"
	"sta	0xfd8a\n"
	"lda	#0\n"
	"sta	0xa410\n"
	"lda	#0x24\n"
	"sta	0xa430\n"
);
//	mmr[PAGE] = 0x1d;
//	vram[0x410] = 0;
//	vram[0x430] = 0x20; //0x60;

	active_page = 0x24;
}

void bank1_off(void)
{
asm volatile(
	"lda	#0x1d\n"
	"sta	0xfd8a\n"

	";lda	#0\n"
	";sta	0xa410\n"

	"lda	#0x08\n"
	"sta	0xa41b\n"

	"lda	#0x04\n"
	"sta	0xa430\n"
);
//	mmr[PAGE] = 0x1d;
//	vram[0x410] = 0;
//	vram[0x430] = 0x00;
	active_page = 0x04;
}

void vram_b_on(void)
{
asm volatile(
	"lda	#0x10\n"
	"sta	0xfd8a\n"
	"inca	\n"
	"sta	0xfd8b\n"
	"inca	\n"
	"sta	0xfd8c\n"
	"inca	\n"
	"sta	0xfd8d\n"
);
//	mmr[PAGE] = 0x10;
//	mmr[PAGE+1] = 0x11;
//	mmr[PAGE+2] = 0x12;
//	mmr[PAGE+3] = 0x13;
}

void vram_r_on(void)
{
asm volatile(
	"lda	#0x14\n"
	"sta	0xfd8a\n"
	"inca	\n"
	"sta	0xfd8b\n"
	"inca	\n"
	"sta	0xfd8c\n"
	"inca	\n"
	"sta	0xfd8d\n"
);
//	mmr[PAGE] = 0x14;
//	mmr[PAGE+1] = 0x15;
//	mmr[PAGE+2] = 0x16;
//	mmr[PAGE+3] = 0x17;
}

void vram_g_on(void)
{
asm volatile(
	"lda	#0x18\n"
	"sta	0xfd8a\n"
	"inca	\n"
	"sta	0xfd8b\n"
	"inca	\n"
	"sta	0xfd8c\n"
	"inca	\n"
	"sta	0xfd8d\n"
);
//	mmr[PAGE] = 0x18;
//	mmr[PAGE+1] = 0x19;
//	mmr[PAGE+2] = 0x1a;
//	mmr[PAGE+3] = 0x1b;
}

void vram_off(void)
{
asm volatile(
	"lda	#0x3a\n"
	"sta	0xfd8a\n"
	"inca	\n"
	"sta	0xfd8b\n"
	"inca	\n"
	"sta	0xfd8c\n"
	"inca	\n"
	"sta	0xfd8d\n"
);
//	mmr[PAGE] = 0x30+PAGE;
//	mmr[PAGE+1] = 0x31+PAGE;
//	mmr[PAGE+2] = 0x32+PAGE;
//	mmr[PAGE+3] = 0x33+PAGE;
}


void write_opn(unsigned char reg, unsigned char data)
{
	*opndat = reg;
	*opncom = 0x03;
	*opncom = 0x00;
	*opndat = data;
	*opncom = 0x02;
	*opncom = 0x00;
}

unsigned char read_opn(unsigned char reg)
{
	unsigned char data;

	*opndat = reg;
	*opncom = 0x03;
	*opncom = 0x00;
	*opncom = 0x09;
	data = *opndat;
	*opncom = 0x00;

	return data;
}

//unsigned char *adr_tmp, *adr_tmp3;
//unsigned short *adr_tmp2;
//unsigned char ii, jj;

unsigned char subcpu_flag = 0;

/*
void sub_draw(void)
{
asm volatile(
	"lda	#0x3f\n"
	"sta	0xfc82\n"
	"lda	_subcpu_flag\n"
	"bne	_SKIP1\n"
	"lda	#1\n"
	"sta	_subcpu_flag\n"
	"jsr	_MOVCMD\n"
"_SKIP1:\n"
	"jsr	_SUBMOV\n"
	"bra	_ENDCD\n"

"_SUBHLT1:\n"
	"lda	0xfd05\n"
	"bmi	_SUBHLT1\n"
	"orcc	#0x50\n"
	"lda	#0x80\n"
	"sta	0xfd05\n"
"_SUBHLT2:\n"
	"lda	0xfd05\n"
	"bpl	_SUBHLT2\n"
	"rts\n"

"_RDYREQ:\n"
	"lda	0xfc80\n"
	"ora	#0x80\n"
	"sta	0xfc80\n"
	"rts\n"

"_SUBMOV:\n"
	"clra\n"
	"sta	0xfd05\n"
	"andcc	#0xaf\n"
	"rts\n"

"_MOVCMD:\n"
	"ldx	#_TESTCD\n"
	"ldy	#0xfc82\n"
"_LOOP1:\n"
	"lda	,X+\n"
	"sta	,Y+\n"
	"cmpx	#_ENDCD\n"
	"bne	_LOOP1\n"
	"rts\n"

"_TESTCD:\n"
	".byte	0x3f\n"
	".blkb	8\n"
	".byte	0x93\n"
	".word	0xd38f\n"
	".byte	0x90\n"

	"ldu	0xd3f8\n"
	"ldx	0xd3fa\n" //#0xd3c8\n"
//	"lda	#8\n"
//	"sta	0xd3fa\n"
	"ldy	#8\n"

"_TESTLP:\n"
	"lda ,x\n"
	"ldb 3,x\n"
	"std 0x0000,u\n"
	"lda 1,x\n"
	"ldb 4,x\n"
	"std 0x4000,u\n"
	"lda 2,x\n"
	"ldb 5,x\n"
	"std 0x8000,u\n"
	"leax 6,x\n"
	"leau 80,u\n"

//	"dec	0xd3fa\n"
	"leay	-1,y\n"
	"bne	_TESTLP\n"

	"rts\n"

"_ENDCD:\n"
//";	equ	*\n"
);
}*/

void sub_draw(void)
{
asm(
	"lda	#0x3f\n"
	"sta	0xfc82\n"
	"lda	_subcpu_flag\n"
	"bne	_SKIP1\n"
	"lda	#1\n"
	"sta	_subcpu_flag\n"
	"jsr	_MOVCMD\n"
"_SKIP1:\n"
	"jsr	_SUBMOV\n"
	"bra	_ENDCD\n"

"_SUBHLT1:\n"
	"lda	0xfd05\n"
	"bmi	_SUBHLT1\n"
	"orcc	#0x50\n"
	"lda	#0x80\n"
	"sta	0xfd05\n"
"_SUBHLT2:\n"
	"lda	0xfd05\n"
	"bpl	_SUBHLT2\n"
	"rts\n"

"_RDYREQ:\n"
	"lda	0xfc80\n"
	"ora	#0x80\n"
	"sta	0xfc80\n"
	"rts\n"

"_SUBMOV:\n"
	"clra\n"
	"sta	0xfd05\n"
	"andcc	#0xaf\n"
	"rts\n"

"_MOVCMD:\n"
	"ldx	#_TESTCD\n"
	"ldy	#0xfc82\n"
"_LOOP1:\n"
	"lda	,X+\n"
	"sta	,Y+\n"
	"cmpx	#_ENDCD\n"
	"bne	_LOOP1\n"
	"rts\n"

"_TESTCD:\n"
	".byte	0x3f\n"
	".blkb	8\n"
	".byte	0x93\n"
	".word	0xd38f\n"
	".byte	0x90\n"

	"lda	0xd3e4\n" // active_page(fce4)
	"sta	0xd430\n"

	"ldu	0xd3e0\n"
	"ldx	0xd3e2\n"	//	"ldx	#0xd3c0\n"
	"ldy	#8\n"
//	"sta	0xd3e2\n"

"_TESTLP:\n"
	"lda ,x\n"
	"sta 0x0000,u\n"
	"lda 1,x\n"
	"sta 0x4000,u\n"
	"lda 2,x\n"
	"sta 0x8000,u\n"

	"leax 3,x\n"
	"leau 40,u\n"

//	"dec	0xd3e2\n"
	"leay	-1,y\n"
	"bne	_TESTLP\n"

	"rts\n"

"_ENDCD:\n"
//";	equ	*\n"
);
}

/* メインRAM->VRAM 3プレーン転送(SUB CPU) */
void put_sub2(char *patadr)
{
	register unsigned char *rx asm("x");

asm(
	"jsr	_SUBHLT1\n"
);

	rx = (unsigned char *)(patadr); //MAPPAT_ADR + patadr * 2 + patadr; 
//	adr_tmp2 = (unsigned short *)(vram_adr);

asm(
//	"jsr	_SUBHLT1\n"

//	"ldx	_adr_tmp\n"
	"ldy	#0xfcc0\n"
"_LOOP00:\n"
	"lda	,X\n"
	"sta	,Y+\n"
	"lda	1,X\n"
	"sta	,Y+\n"
	"lda	2,X\n"
	"sta	,Y+\n"

	"leax 48,x\n"

	"cmpy	#0xfcc0+24\n"
	"bne	_LOOP00\n"

	"ldd	_vram_adr\n" //_adr_tmp2\n"
	"std	0xfce0\n"
	"ldd	#0xd3c0\n"
	"std	0xfce2\n"

	"lda	_active_page\n"
	"sta	0xfce4\n"

		:	/* 値が返るレジスタ変数 */
		:"r"(rx)	/* 引数として使われるレジスタ変数 */
		:"y","d"	/* 破壊されるレジスタ */
);
	sub_draw();
}


void cursor_off(void)
{
asm volatile(
	"jsr	_SUBHLTC1\n"

	"lda	#0x3f\n"
	"sta	0xfc82\n"
	"lda	#0x00"
	"sta	_subcpu_flag\n"
	"jsr	_MOVCMDC\n"
	"jsr	_SUBMOVEC\n"
	"bra	_ENDCDC\n"

"_SUBHLTC1:\n"
	"lda	0xfd05\n"
	"bmi	_SUBHLTC1\n"
	"orcc	#0x50\n"
	"lda	#0x80\n"
	"sta	0xfd05\n"
"_SUBHLTC2:\n"
	"lda	0xfd05\n"
	"bpl	_SUBHLTC2\n"
	"rts\n"

"_RDYREQC:\n"
	"lda	0xfc80\n"
	"ora	#0x80\n"
	"sta	0xfc80\n"
	"rts\n"

"_SUBMOVEC:\n"
	"clra\n"
	"sta	0xfd05\n"
	"andcc	#0xaf\n"
	"rts\n"

"_MOVCMDC:\n"
	"ldx	#_TESTCDC\n"
	"ldy	#0xfc82\n"
"_LOOPC1:\n"
	"lda	,X+\n"
	"sta	,Y+\n"
	"cmpx	#_ENDCDC\n"
	"bne	_LOOPC1\n"
	"rts\n"

"_TESTCDC:\n"
	".byte	0x3f\n"
	".blkb	8\n"
	".byte	0x93\n"
	".word	0xd38f\n"
	".byte	0x90\n"

	"lda	#0x00"
	"sta	0xd004\n"
	"rts\n"

"_ENDCDC:\n"
);
}


void cls_sub(void)
{
asm volatile(
	"jsr	_SUBHLTE1\n"

	"lda	_active_page\n"
	"sta	0xfce4\n"

	";ldd	_vram_adr\n"
	";std	0xfcf8\n"

	"lda	#0x3f\n"
	"sta	0xfc82\n"
	"lda	_subcpu_flag\n"
	";cmpa	#2\n"
	";beq	_SKIPE1\n"
	"lda	#0\n"\
	"sta	_subcpu_flag\n"
	"jsr	_MOVCMDE\n"
"_SKIPE1:\n"
	"jsr	_SUBMOVEE\n"
	"bra	_ENDCDE\n"

"_SUBHLTE1:\n"
	"lda	0xfd05\n"
	"bmi	_SUBHLTE1\n"
	"orcc	#0x50\n"
	"lda	#0x80\n"
	"sta	0xfd05\n"
"_SUBHLTE2:\n"
	"lda	0xfd05\n"
	"bpl	_SUBHLTE2\n"
	"rts\n"

"_RDYREQE:\n"
	"lda	0xfc80\n"
	"ora	#0x80\n"
	"sta	0xfc80\n"
	"rts\n"

"_SUBMOVEE:\n"
	"clra\n"
	"sta	0xfd05\n"
	"andcc	#0xaf\n"
	"rts\n"

"_MOVCMDE:\n"
	"ldx	#_TESTCDE\n"
	"ldy	#0xfc82\n"
"_LOOPE1:\n"
	"lda	,X+\n"
	"sta	,Y+\n"
	"cmpx	#_ENDCDE\n"
	"bne	_LOOPE1\n"
	"rts\n"

"_TESTCDE:\n"
	".byte	0x3f\n"
	".blkb	8\n"
	".byte	0x93\n"
	".word	0xd38f\n"
	".byte	0x90\n"

//	"rts\n"

	"lda	0xd3e4\n" // active_page(fce4)
	"sta	0xd430\n"

	"ldu	#0 ;0xd3f8\n"
//	"ldx	0xd3fa\n" //#0xd3c8\n"
//	"lda	#8\n"
//	"sta	0xd3fa\n"
	"ldx	#0x4000\n"
	"ldd #0x0000\n"

"_TESTLPE:\n"
	"std 0x0000,u\n"
	"std 0x4000,u\n"
	"std 0x8000,u\n"
	"leau 2,u\n"

//	"dec	0xd3fa\n"
	"leax	-2,x\n"
	"bne	_TESTLPE\n"

	"rts\n"

"_ENDCDE:\n"
//";	equ	*\n"
);
}

void get_key(void)
{
asm volatile(
	"bra	key\n"
"keyin:\n"
	".byte	21\n"
	".blkb	1\n"
	".word	keydt\n"
	".blkb	2\n"
	".blkb	2\n"
"keydt:\n"
	".blkb	2\n"
"key:\n"
	"ldx	#keyin\n"
	"jsr	[0xfbfa]\n"
	"bcs	error\n"
	"lda	keydt+1\n"
	"sta	_keyflag\n"
	"lda	keydt\n"
	"sta	_keycode2\n"
	"bra	keyend\n"
"error:\n"
	"lda	#0\n"
	"sta	_keyflag\n"
"keyend:\n"
);
}

void key_clear(void)
{
	while(1){
		get_key();
		if(!keyflag)
			break;
	}
}

unsigned char timer;

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
//	unsigned char *pal_h = (unsigned char *)0xfd30;
//	unsigned char *pal_l = (unsigned char *)0xfd31;

//	unsigned short *pal = (unsigned char *)0xfd30;

/*	unsigned char *pblue = (unsigned char *)0xfd32;
	unsigned char *pred = (unsigned char *)0xfd33;
	unsigned char *pgreen = (unsigned char *)0xfd34;
*/

//	register unsigned char *rx asm("x");

	register unsigned short ry asm("y");
	register unsigned char ra asm("a");
	register unsigned char rb asm("d"); //("b");

	register unsigned short rd asm("d");
	register unsigned short rx asm("x");

	timer = 13;
//	cblue = blue;
//	cred = red;
//	cgreen = green;
//	ccolor = color;

//	*pal_h = color / 256;
//	*pal_l = color % 256;

//	*pal = color;
/*
asm(
//	"	ldy _ccolor\n"
	"	lda _cblue\n"
	"	ldb _cred\n"
);*/
	ra = blue;
	rb = red;
//	rd = (blue * 256) | red;
	rx = rd;
/*
asm(
	"	tfr d,x\n"
);*/
	rb = green;

	ry = color;

asm(
//	"	ldb _cgreen\n"

	"	sty 0xfd30\n"
	"	jsr timing\n"
	"	stx 0xfd32\n"
	"	stb 0xfd34\n"

	"	bra palend\n"

"timing:\n"
	"	lda 0xfd12\n"
	"	bita #0x01\n"
	"	bne tm04\n"
"tm01:\n"
	"	lda 0xfd12\n"
	"	bita #0x02\n"
	"	bne tm01\n"
"tm02:\n"
	"	lda 0xfd12\n"
	"	bita #0x02\n"
	"	beq tm02\n"
	"	lda _timer\n"
"tm03:\n"
	"	deca\n"
	"	bne tm03\n"
"tm04:\n"
	"	rts\n"

"palend:\n"
);
/*
	*pblue = blue;
	*pred = red;
	*pgreen = green;*/
}

void get_pal(unsigned short no, unsigned char *red, unsigned char *green, unsigned char *blue)
{
	*green = *red = *blue = 0;

		// bank0パレット
		if(no & 0x8)
			*blue = 0xf;
		if(no & 0x80)
			*red = 0xf;
		if(no & 0x800)
			*green = 0xf;


	// 優先順位をつける
	if((!*red && !*blue && !*green)){

		// bank1パレット
		if(no & 0x2)
			*blue = 0xf;
		if(no & 0x20)
			*red = 0xf;
		if(no & 0x200)
			*green = 0xf;

	}
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short no;
	unsigned char red,green,blue;

//	for(j  = 0; j < 8; j++){
	for(no = 1; no < 4096; no++){
//		no = (j & 1) * 8 + (j & 2) * 64 + (j & 4) * 512;
//	for(no = 4096-8; no < 4096; no++){
		get_pal(no, &red, &green, &blue);
		pal_set(0, no, red, green, blue);
	}
}

//パレットを暗転する。
void pal_allblack(int pal_no)
{
	short j, no;
	unsigned short adr1, adr2;

	for(j = 1; j < 4096; j++){
		pal_set(0, j, 0, 0, 0);
	}
}

unsigned char k0, k1, k2, k3, st, st2 = 0;

unsigned char sub_flag;

enum {
	TILE_WALL,   // 壁
	TILE_FLOOR,   // 床
	TILE_PLAYER,   // プレイヤー
	TILE_NORMAL,   // 通常パネル（S） - シンプルブロック
	TILE_GRAVITY,   // 重力パネル（B） - 下向き矢印付き
	TILE_GOAL,   // ゴール
	TILE_HOLLOW,  // 格子状の壁
	TILE_SLIME   // スライム
};

#define PRINT_MUL 1

void beep(void)
{
}

void put_chr8(int x, int y, char chr, char atr) {
	if((x < 0) || (y < 0))
		return;
	x += (32/8); //*2;

	sub_disable();
	bank1_on();
	sub_enable();

	vram_adr = (char *)(x + y * 40 * 8);

/*asm volatile(
	"orcc	#0x10\n"
);*/
	put_sub2((char *)(ds1_grp + ((chr & 0x0f) * 1 + (chr & 0xf0) *  8)  * 3));
/*asm volatile(
	"andcc	#0xef\n"
);*/

	sub_disable();

	bank1_off();
	sub_enable();
}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 2 + 0, y * 2 + 0, chr * 2 + 0, 0x27);
	put_chr8(x * 2 + 1, y * 2 + 0, chr * 2 + 1, 0x27);

	put_chr8(x * 2 + 0, y * 2 + 1, chr * 2 + 16, 0x27);
	put_chr8(x * 2 + 1, y * 2 + 1, chr * 2 + 17, 0x27);
}

char chr;

// VRAM直書き
void print_at(char x, char y, char *str) {
	x += (32/8);

	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
		if(chr >= 'a')
			chr -= ('a'-'A');
		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		vram_adr = (char *)((x++) * 1 + y * 40 * 8);

		put_sub2((char *)(font_grp + ((chr & 0x0f) * 1 + (chr & 0xf0) * 8)  * 3));
	}

}

void print_at_2(char x, char y, char *str) {
	x += (32/8);

	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		vram_adr = (char *)((x++) * 1 + y * 40 * 8);
		put_sub2((char *)(font_grp + ((chr & 0x0f) * 1 + (chr & 0xf0) * 8)  * 3));
	}
}

void put_logo(int x, int y)
{
	print_at_2(x, y, "      i  k   ");
	print_at_2(x, y+1, " 2026 bcdefgh");
}

#define submode (unsigned char *)0xfd12

void wait_vsync(void)
{
	while((*submode & 0x01)); /* WAIT VSYNC */
	while(!(*submode & 0x01));
}

// vsync
void vsync(void) {
	wait_vsync();
}

void wait(int j) {
	for (i = 0; i < j; ++i)
		vsync();
}

unsigned short ii, jj, ll;
int kk;

void cls(void)
{
//	return;
/*	int i,j;
	for(j = 0l; j < 24; j++)
		for(i = 0; i < 80; ++i)
			put_chr8(i, j, ' ', 0);*/

	sub_disable();
	bank1_on();
	sub_enable();
	cls_sub();

	sub_disable();
	bank1_on();
	sub_vram[0x40f] = 0;	// スクロール座標を戻す
	sub_vram[0x40e] = 0;
	sub_enable();


//	cls_sub();


	sub_disable();
	bank1_off();
	sub_enable();
	cls_sub();

	return;
/*
	sub_disable();
asm volatile(
	"LDA	#0x02	; CLSコマンド ($02)\n"
	"STA	0xFC82	; コマンド設定エリア\n"
);
	sub_enable();*/
}

void cls2(void) {

//	bank1_on();

//	mmr[PAGE] = 0x1d;
//	vram[0x410] = 0;
//	vram[0x430] = 0x20; //0x60;

	jj = 320/8*200;
	ll = 0;
	for(ii = 0; ii < 200*(320/8); ii+=(320/8)){
//		*((unsigned char *)0xd40e) 
//		*((unsigned char *)0xd40f)
//			vram_r_on();

		wait_vsync();
		sub_disable();
		bank1_on();
		sub_vram[0x40f] = ii & 0xff;
		sub_vram[0x40e] = ((ii & 0x3f00) >> 8) & 0x3f;
		sub_enable();

//		jj = 320/8*200;

		for(kk = 0; kk < 40; ++kk){
			sub_disable();
			vram_r_on();
			*(sub_vram + jj) = *(sub_vram + ll);
//			*((unsigned char *)((unsigned short)VRAM_DATA_ADR + jj)) = 0xff;
			vram_g_on();
			*(sub_vram + jj) = *(sub_vram + ll);
			vram_b_on();
			*(sub_vram + jj) = *(sub_vram + ll);
			sub_enable();
			++jj;
			++ll;
//			if(jj>(0x2000))
//				jj = 0;
		}
		jj -= 40;
		ll -= 40;
		if(ll < 0)
			ll = 0x2000;

	}
//	*((unsigned char *)0xd40e) = 0;
//	*((unsigned char *)0xd40f) = 0;

	cls();
}


void define_tiles(void) {
}

void play_sound_effect(void) {
	beep();  // シンプルなビープ音
}

int memcpy(char *dst, char *src, int size)
{
	while(size--)
		*dst++ = *src++;
	return 0;
}

unsigned char keycode = 0;

unsigned char keyscan(void)
{
	keycode = 0;
		/* ジョイスティック読み込み */
asm volatile(
	"orcc	#0x10\n"
);
	write_opn(15, 0x2f);
	st = ~read_opn(14);
	st2 = st3;
asm volatile(
	"andcc	#0xef\n"
);
	st2 = st3;

	if((st & 0x01) || (st2 & 0x01)){ /* U */
		keycode |= KEY_UP1;
	}
	if((st & 0x08) || (st2 & 0x08)){ /* R */
		keycode |= KEY_RIGHT1;
	}
	if((st & 0x02) || (st2 & 0x02)){ /* D */
		keycode |= KEY_DOWN1;
	}
	if((st & 0x04) || (st2 & 0x04)){ /* L */
		keycode |= KEY_LEFT1;
	}
	if((st & 0x10) || (st2 & 0x10)){ /* A */
		keycode |= KEY_A;
	}
	if((st & 0x20) || (st2 & 0x20)){ /* B */
		keycode |= KEY_B;
	}

	return keycode;
}

#include "common.h"

int main(void)
{
/*asm volatile(
	"LDS  #0x7FFF    ; ハードウェアスタックを$7FFFに設定\n"
	"LDU  #0x7F00\n"
	"lda #1\n"
	"sta 0xfd13\n	;サブモニタROMをAに"
);*/
	msr_sv = *msr;
	subcpu_flag = 0;

//	subinter[0] = 0x20*0 | 0x10*0 | 0x08*1 | 0x04;

	mem2[1] = 0x02;
	mem2[0] = 0x40;	/* 多色モードにする */

	mem3[0] = 00; //x77;


	sub_disable();
	bank1_off();	/* 論理演算を切る */
	sub_enable();

	cursor_off();
	pal_all(0, org_pal);

	/* ジョイスティック設定 */
asm volatile(
	"orcc	#0x10\n"
);
	write_opn(15, 0x3f);
	write_opn(7, 0xbf);
asm volatile(
	"andcc	#0xef\n"
);

	st3 = 0;
	st2 = 0;
	keycode = 0;

	keyscan_on();
	keyrepeat_off();
	set_key_irq();

	key_clear();
	cls();

#ifdef DEBUG2
	for(;;)
#endif
		main2();

#ifndef DEBUG2
	cls();

	vram_off();
	reset_key_irq();
	keyrepeat_on();
	keyscan_off();
	key_clear();

	mem2[1] = 0x01;
	mem2[0] = 0x00;	/* 8色2画面モードにする */

	*msr = msr_sv;

	return 0;
#endif
}

