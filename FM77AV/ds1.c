/* ds1.c for GCC6809 FM77AV  m@3 */
/* キャラを出す SUB CPUコマンド版 */

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
/*
#define VRAM_DATA_ADR (PAGE * 0x1000)
#define MAP_ADR VRAM_DATA_ADR
#define PARTS_DATA (MAP_ADR+0x3c00)
#define MASK_DATA_ADR 0x1600
#define MAPPAT_ADR 0x2000
#define CHRPAT_ADR 0x4000

#define SUBPAT_ADR 0xa500*/
//#define SUBPAT_ADR2 0xd500

#define SIZE 80

#define X_SIZE 18
#define Y_SIZE 18

#define PARTS_X 2
#define PARTS_Y 8
/*
#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char vram_tmp[PARTS_X * PARTS_Y * 3];
unsigned char map_data[(X_SIZE+2) * 32];
*/
unsigned char *subram_table[15];

//unsigned char no;
unsigned char *vram_adr;

unsigned char i, j;

unsigned char *mmr;
//unsigned char *vram;
unsigned char *mem;
unsigned char *msr;
unsigned char msr_sv;
unsigned char *keyport, key, st3;

#define OPNCOM 0xFD15
#define OPNDAT 0xFD16

unsigned char *opncom;
unsigned char *opndat;

void keyscan_on(void)
{
asm(
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
asm(
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
asm(
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
asm(
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
asm(

"_TINIT:\n"
	"orcc	#0x10\n"
	"ldx		0xFFF8\n"
	"cmpx	#_IRQET\n"
	"beq		_TCLOSE\n"
	"stx		_IRQJP\n"
	"ldx		#_IRQET\n"
	"stx		0x0FFF8\n"
	"bra	_ENDIRQ\n"
"_TCLOSE:\n"
	"ldx		_IRQJP\n"
	"stx		0xFFF8\n"
	"bra	_ENDIRQ\n"

"_IRQET:\n"
	"pshs	D,X\n"
	"lda		0xFD03\n"
	"anda	#1\n"
	"beq		_IRQRT\n"

	"jsr	_key_sense\n"

"_IRQRT:\n"
	"puls	D,X\n"
	"jmp		[_IRQJP]\n"

"_IRQJP:	.blkb	2\n"
"_TCOUNT:	.blkb	1\n"
"_ENDIRQ:\n"
	"andcc	#0xEF\n"
);

}


void sub_disable(void)
{
asm(
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
asm(
	"lda	0xfd93\n"
	"sta	_msr_sv\n"
	"ora	#0x80\n"
	"sta	0xfd93\n"
);
//	*msr |= 0x80;
}

void sub_enable(void)
{
//	*msr = msr_sv;

asm(
	"lda	_msr_sv\n"
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
asm(
	"lda	#0x1d\n"
	"sta	0xfd8a\n"
	"lda	#0\n"
	"sta	0xa410\n"
	"lda	#0x20\n"
	"sta	0xa430\n"
);
//	mmr[PAGE] = 0x1d;
//	vram[0x410] = 0;
//	vram[0x430] = 0x20; //0x60;
}

void bank1_off(void)
{
asm(
	"lda	#0x1d\n"
	"sta	0xfd8a\n"
	"lda	#0\n"
	"sta	0xa410\n"
	"sta	0xa430\n"
);
//	mmr[PAGE] = 0x1d;
//	vram[0x410] = 0;
//	vram[0x430] = 0x00;
}

void vram_b_on(void)
{
asm(
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
asm(
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
asm(
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
asm(
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
unsigned char ii, jj;

unsigned char subcpu_flag = 0;

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
}

/* メインRAM->VRAM 3プレーン転送(SUB CPU) */
void put_sub2(char *patadr)
{
	register unsigned char *rx asm("x");

asm(
	"jsr	_SUBHLT1\n"
);

//	adr_tmp
///	rx = (unsigned char *)MAPPAT_ADR + patadr * 2 + patadr; 
	rx = (unsigned char *)(patadr); //0 + patadr * 2 + patadr); 
//	adr_tmp2 = (unsigned short *)(vram_adr);


asm(
//	"ldx	_adr_tmp\n"
//	"ldx	_adr_tmp\n"
	"ldy	#0xfcc8\n"
	"lda	#8\n"
"_LOOP00:\n"
	"ldu	,X\n"
	"stu	,Y++\n"
	"ldu	2,X\n"
	"stu	,Y++\n"
	"ldu	4,X\n"
	"stu	,Y++\n"

	"leax 96,x\n"
//	"leay 6,y\n"

//	"cmpy	#0xfcc8+48\n"
	"deca\n"
	"bne	_LOOP00\n"

//	"ldd	_adr_tmp2\n"
	"ldd	_vram_adr\n"
	"std	0xfcf8\n"
	"ldd	#0xd3c8\n"
	"std	0xfcfa\n"

		:	/* 値が返るレジスタ変数 */
		:"r"(rx)	/* 引数として使われるレジスタ変数 */
		:"y","d"	/* 破壊されるレジスタ */
);
	sub_draw();
}


void get_key(void)
{
asm(
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
	"bra	end\n"
"error:\n"
	"lda	#0\n"
	"sta	_keyflag\n"
"end:\n"
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
/*
unsigned char data_no, *data, *data_tmp;

unsigned char *vram_adr_tmp;
unsigned short old_x = 255, old_y = 255;
unsigned short x = 165, y = 30;
unsigned char old_map_data[(X_SIZE + 2) * 32];
*/
unsigned char k0, k1, k2, k3, st, st2;

//unsigned short xx, yy;
//unsigned char k;

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

	vram_adr = (char *)(x + y * 80 * 8);
	put_sub2((char *)(ds1_grp + ((chr & 0x0f) * 2 + (chr & 0xf0) * 16)  * 3));
}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 4 + 0, y * 2 + 0, chr * 2 + 0, 0x27);
	put_chr8(x * 4 + 2, y * 2 + 0, chr * 2 + 1, 0x27);

	put_chr8(x * 4 + 0, y * 2 + 1, chr * 2 + 16, 0x27);
	put_chr8(x * 4 + 2, y * 2 + 1, chr * 2 + 17, 0x27);
}

char chr;

// VRAM直書き
void print_at(char x, char y, char *str) {
	register unsigned char *rx asm("x");
	register unsigned char *ry asm("y");

	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
		if(chr >= 'a')
			chr -= ('a'-'A');
		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		vram_adr = (char *)((x++) * 2 + y * 80 * 8);
		put_sub2((char *)(font_grp + ((chr & 0x0f) * 2 + (chr & 0xf0) * 16)  * 3));
	}
}

void wait_vsync(void)
{
	unsigned char *submode = (unsigned char *)0xfd12;
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

void cls(void) {
/*	int i,j;
	for(j = 0l; j < 24; j++)
		for(i = 0; i < 80; ++i)
			put_chr8(i, j, ' ', 0);*/

	sub_disable();
asm(
	"LDA	#0x02	; CLSコマンド ($02)\n"
	"STA	0xFC82	; コマンド設定エリア\n"
);
	sub_enable();
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
asm(
	"orcc	#0x10\n"
);
	write_opn(15, 0x2f);
	st = ~read_opn(14);
	st2 = st3;
asm(
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
	mmr = (unsigned char *)0xFD80;
	mem = (unsigned char *)0x6AFF;
	msr = (unsigned char *)0xFD93;
	keyport = (unsigned char *)0xFD01;

	opncom = (unsigned char *)OPNCOM;
	opndat = (unsigned char *)OPNDAT;
	subcpu_flag = 0;

	/* ジョイスティック設定 */
asm(
	"orcc	#0x10\n"
);
	write_opn(15, 0x3f);
	write_opn(7, 0xbf);
asm(
	"andcc	#0xef\n"
);

	st3 = 0;

	keyscan_on();
	keyrepeat_off();
	set_key_irq();

	key_clear();

	main2();
	cls();

	vram_off();
	set_key_irq();
	keyrepeat_on();
	keyscan_off();
	key_clear();
	return 0;
}

