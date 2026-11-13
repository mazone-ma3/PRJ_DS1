/* plustaker64.c By m@3 with Grok */

#include <stdio.h>
#include <stdlib.h>

#include "mode.h"
#include "inkey.h"

#define PRINT_MUL 1

#define A_KEY "Z"
#define B_KEY "X"

enum {
	TILE_WALL = 0x00,   // 壁
	TILE_FLOOR = 0x04,   // 床
	TILE_PLAYER = 0x08,   // プレイヤー
	TILE_NORMAL = 0x0c,   // 通常パネル（S） - シンプルブロック
	TILE_GRAVITY = 0x10,   // 重力パネル（B） - 下向き矢印付き
	TILE_GOAL = 0x14,   // ゴール
	TILE_HOLLOW = 0x18,  // 格子状の壁

	TILE_SLIME = 0x1c,   // スライム
	TILE_MILE = 0x20,
	TILE_OLDMAN = 0x24,
	TILE_BAT = 0x28,
	TILE_STONE = 0x2c,
	TILE_MUMMY = 0x30,
	TILE_ARMOR = 0x34
};

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
#define BITDATA(n) (1 << (n))
//unsigned char bitdata[8] = { 1, 2, 4, 8, 16, 32, 64, 128};
//#define BITDATA(n) bitdata[n]

/* BITセット */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BITクリア */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BITチェック */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT反転 */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}

enum {
	COLOR_BLACK       = 0,
	COLOR_WHITE       = 1,
	COLOR_RED         = 2,
	COLOR_CYAN        = 3,
	COLOR_PURPLE      = 4,
	COLOR_GREEN       = 5,
	COLOR_BLUE        = 6,
	COLOR_YELLOW      = 7,
	COLOR_ORANGE      = 8,
	COLOR_BROWN       = 9,
	COLOR_LIGHT_RED   = 10,
	COLOR_GRAY_ONE    = 11,
	COLOR_DARK_GREY   = 11,
	COLOR_GRAY_TWO    = 12,
	COLOR_MEDIUM_GREY = 12,
	COLOR_LIGHT_GREEN = 13,
	COLOR_LIGHT_BLUE  = 14,
	COLOR_GRAY_THREE  = 15,
	COLOR_LIGHT_GREY  = 15,
};

enum {
	VICOFFSET = 0x8000,
	BGRAM = (0x400 + VICOFFSET),
	CRAM = (0xd800),
	CHRDATA = (0x800 + VICOFFSET)
};

#define SPRITE_POINTERS ((unsigned char *)0x07f8 + VICOFFSET)
#define SPRITE_ENABLE_REGISTER ((unsigned char *)0xd015)
#define SPRITE_0_COLOR_REGISTER ((unsigned char *)0xd027)
#define SPRITE_MULTICOLOR_REGISTERS ((unsigned char *)0xd01c)
#define SPMC0 ((unsigned char *)0xd025)
#define SPMC1 ((unsigned char *)0xd026)

#define MSGIGX ((unsigned char *)0xd010)
#define SP0X ((unsigned char *)0xd000)
#define SP0Y ((unsigned char *)0xd001)
#define SPXX ((unsigned char *)0xd010)

#define RASTER ((volatile unsigned char *)0xd012)

#define spr01ShapeData ((unsigned char *)(0xf * 64 + VICOFFSET))
#define spr02ShapeData ((unsigned char *)(0xe * 64 + VICOFFSET))
#define spr03ShapeData ((unsigned char *)(0xd * 64 + VICOFFSET))
#define spr04ShapeData ((unsigned char *)(0xc * 64 + VICOFFSET))
#define spr05ShapeData ((unsigned char *)(0xb * 64 + VICOFFSET))
#define spr06ShapeData ((unsigned char *)(0xa * 64 + VICOFFSET))
#define spr07ShapeData ((unsigned char *)(0x9 * 64 + VICOFFSET))
#define spr08ShapeData ((unsigned char *)(0x8 * 64 + VICOFFSET))
#define spr09ShapeData ((unsigned char *)(0x9 * 64 + VICOFFSET))

#define porta ((volatile unsigned char *)0xdc00)
#define portb ((volatile unsigned char *)0xdc01)
#define portadir ((volatile unsigned char *)0xdc02)
#define portbdir ((volatile unsigned char *)0xdc03)

#define BoaderColorRegister ((unsigned char *)0xd020)
#define BackgroundColor ((unsigned char *)0xd021)

#define screenarea0 ((unsigned char *)(BGRAM))
#define screenarea1 ((unsigned char *)(BGRAM + 0x100))
#define screenarea2 ((unsigned char *)(BGRAM + 0x200))
#define screenarea3 ((unsigned char *)(BGRAM + 0x2e8))

#define foregroundarea0 ((unsigned char *)(CRAM))
#define foregroundarea1 ((unsigned char *)(CRAM + 0x100))
#define foregroundarea2 ((unsigned char *)(CRAM + 0x200))
#define foregroundarea3 ((unsigned char *)(CRAM + 0x2e8))

#define interrupt ((unsigned char *)0xdc0e)
#define charset ((unsigned char *)0x001L)
#define charsetram ((unsigned char *)0xd018)
#define vicbank ((unsigned char *)0xdd00)
unsigned char *chrdata, *chrdata2, oldcharsetram, oldvicbank;

#define SCREENCONTROL1 ((unsigned char *)0xd011)
#define SCREENCONTROL2 ((unsigned char *)0xd016)

#define EXTBGCOOLR1 ((volatile unsigned char *)0xd022)
#define EXTBGCOOLR2 ((volatile unsigned char *)0xd023)

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200 // PAL

// max/min マクロ
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))


// Color Index 5 (Palette B=255, G=255, R=0)
unsigned char mychr_sprite_color_5[63] = {
    0x07, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x0c, 0x00, // Row 2
    0x00, 0x0c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, // Row 5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 7
    0x0f, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x3f, 0x00, // Row 10
    0x00, 0x1f, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x0f, // Row 13
    0x80, 0x00, 0x1f, 0x80, 0x00, 0x1f, 0x80, 0x00, // Row 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

// Color Index 1 (Palette B=255, G=0, R=0)
unsigned char mychr_sprite_color_1[63] = {
    0x00, 0xe0, 0x00, 0x01, 0xf0, 0x00, 0x01, 0xf0, // Row 2
    0x00, 0x01, 0xf0, 0x00, 0x01, 0x30, 0x00, 0x00, // Row 5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 7
    0x00, 0xf0, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, // Row 10
    0x00, 0x00, 0xf0, 0x00, 0x00, 0x70, 0x00, 0x00, // Row 13
    0x70, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, // Row 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

// Color Index 7 (Palette B=255, G=255, R=255)
unsigned char mychr_sprite_color_7[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, // Row 2
    0x00, 0x02, 0x00, 0x00, 0x06, 0x00, 0x00, 0x05, // Row 5
    0x00, 0x00, 0x05, 0x00, 0x00, 0x03, 0x80, 0x00, // Row 7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 10
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 13
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

// Color Index 6 (Palette B=0, G=255, R=255)
unsigned char mychr_sprite_color_6[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 2
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, // Row 5
    0xa0, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x40, 0x00, // Row 7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 10
    0x00, 0x60, 0x06, 0x00, 0x60, 0x06, 0x00, 0x00, // Row 13
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };


// Color Index 5 (Palette B=255, G=255, R=0)
unsigned char block_sprite_color_5[63] = {
    0x7f, 0xfe, 0x00, 0xff, 0xff, 0x00, 0xfe, 0x7f, // Row 2
    0x00, 0xfe, 0x7f, 0x00, 0xfe, 0x7f, 0x00, 0xfe, // Row 5
    0x7f, 0x00, 0xfe, 0x7f, 0x00, 0xfe, 0x7f, 0x00, // Row 7
    0xfe, 0x7f, 0x00, 0xfe, 0x7f, 0x00, 0xfe, 0x7f, // Row 10
    0x00, 0xf2, 0x4f, 0x00, 0xf8, 0x1f, 0x00, 0xfe, // Row 13
    0x7f, 0x00, 0xff, 0xff, 0x00, 0x7f, 0xff, 0x00, // Row 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

// Color Index 7 (Palette B=255, G=255, R=255)
unsigned char block_sprite_color_7[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, // Row 2
    0x00, 0x01, 0x80, 0x00, 0x01, 0x80, 0x00, 0x01, // Row 5
    0x80, 0x00, 0x01, 0x80, 0x00, 0x01, 0x80, 0x00, // Row 7
    0x01, 0x80, 0x00, 0x01, 0x80, 0x00, 0x01, 0x80, // Row 10
    0x00, 0x0d, 0xb0, 0x00, 0x07, 0xe0, 0x00, 0x01, // Row 13
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

// Color Index 5 (Palette B=255, G=255, R=0)
unsigned char slime_sprite_color_5[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 2
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 5
    0x00, 0x00, 0x07, 0xf0, 0x00, 0x07, 0xf0, 0x00, // Row 7
    0x0f, 0xf8, 0x00, 0x0f, 0xf8, 0x00, 0x3d, 0xdc, // Row 10
    0x00, 0x3d, 0xda, 0x00, 0x3f, 0xf4, 0x00, 0x3f, // Row 13
    0xea, 0x00, 0x0f, 0xd0, 0x00, 0x0f, 0xa8, 0x00, // Row 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

// Color Index 1 (Palette B=255, G=0, R=0)
unsigned char slime_sprite_color_1[63] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 2
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 5
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 7
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, // Row 10
    0x00, 0x00, 0x04, 0x00, 0x00, 0x0a, 0x00, 0x00, // Row 13
    0x14, 0x00, 0x00, 0x28, 0x00, 0x00, 0x50, 0x00, // Row 15
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };


unsigned char block1_data[32] = {
    0xef, 0xef, 0xef, 0xaa, 0xfe, 0xfe, 0xfe, 0xaa, // Character 0
    0xef, 0xef, 0xef, 0xaa, 0xfe, 0xfe, 0xfe, 0xaa, // Character 1
    0xef, 0xef, 0xef, 0xaa, 0xfe, 0xfe, 0xfe, 0xaa, // Character 2
    0xef, 0xef, 0xef, 0xaa, 0xfe, 0xfe, 0xfe, 0xaa, // Character 3
};

unsigned char block2_data[32] = {
    0x3f, 0xff, 0xfd, 0xf5, 0xfd, 0xfd, 0xdd, 0xd5, // Character 0
    0xfc, 0xff, 0x7f, 0x5f, 0x7f, 0x7f, 0x77, 0x57, // Character 1
    0xd5, 0xdd, 0xfd, 0xfd, 0xf5, 0xfd, 0xff, 0x3f, // Character 2
    0x57, 0x77, 0x7f, 0x7f, 0x5f, 0x7f, 0xff, 0xfc, // Character 3
};

unsigned char block4_data[32] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Character 0
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Character 1
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Character 2
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Character 3
};

unsigned char goal_data[32] = {
    0xaa, 0xaa, 0xaa, 0xab, 0xae, 0xae, 0xba, 0xba, // Character 0
    0xaa, 0xaa, 0xaa, 0xfa, 0xae, 0xaa, 0xaa, 0xaa, // Character 1
    0xba, 0xba, 0xae, 0xae, 0xab, 0xaa, 0xaa, 0xaa, // Character 2
    0xfe, 0xae, 0xae, 0xbe, 0xee, 0xaa, 0xaa, 0xaa, // Character 3
};

char *bgram;
unsigned char *cram;

// 構造体
typedef struct {
	int x, y;
	int active;
} Entity;

static void print_at(unsigned char x, unsigned char y,  char *str)//, unsigned char pal)
{
	char chr;
	unsigned char pal=COLOR_WHITE;

	x += 4;

	bgram = (char *)BGRAM + (x + y * 40);
	cram = (unsigned char *)CRAM + (x + y * 40);

//	if(cram > (char *)0x0dbe7)
//		return;

	while((chr = *(str++)) != '\0'){
/*		if((chr < 0x30)) //|| (chr > 0x5f))
			chr = 0x20;
		else
			chr -= '0';*/
		if(chr >= 'a')
			chr -= ('a'-'A');

		chr &= 0x3f;
//		if((chr >= 0x40))
//			chr -= 0x40;
		*(bgram++) = chr;
		*(cram++) = pal;
	}
}

static void put_chr_color(unsigned char x, unsigned char y,  char chr, unsigned char pal)
{
	bgram = (char *)BGRAM + (x + y * 40);
	cram = (unsigned char *)CRAM + (x + y * 40);
	chr &= 0x3f;
	*(bgram) = chr;
	*(cram) = pal | 0x08;
}

static void put_chr8(unsigned char x, unsigned char y,  char chr)
{
	bgram = (char *)BGRAM + (x + y * 40);
	cram = (unsigned char *)CRAM + (x + y * 40);
	chr &= 0x3f;
	*(bgram) = chr;
	*(cram) = COLOR_WHITE;
}

char str_temp[9];

static void put_numd(long j, char digit)
{
	char i = digit;

	while(i--){
		str_temp[i] = j % 10 + 0x30;
		j /= 10;
	}
	str_temp[digit] = '\0';
}


#ifdef DEBUG
static void copy(void)
{
	register unsigned char ra asm("a");

	asm volatile(
//		"txa\n"
//		"pha\n"
//		"tya\n"
//		"pha\n"
		"lda #0\n"
		"ldy #$d0\n"
		"sta 95\n"
		"sty 96\n"
		"lda #0\n"
		"ldy #$e0\n"
		"sta 90\n"
		"sty 91\n"
		"lda #0\n"
		"ldy #$40\n"
		"sta 88\n"
		"sty 89\n"
		"jsr $a3bf\n"
//		"pla\n"
//		"tay\n"
//		"pla\n"
//		"tax\n"
		://"=a"(ra)	/* 値が返るレジスタ変数 */
		://"a"(ra)	/* 引数として使われるレジスタ変数 */
		:"%a","%x","%y"		/* 破壊されるレジスタ */
	);
}
#endif

/*static void put_sprite(unsigned char no, short x, unsigned char y, unsigned char color, unsigned char pat_no)
{
	BITSET(no, *SPRITE_ENABLE_REGISTER);
	SP0Y[no * 2] = y;
	SP0X[no * 2] = x & 0xff;
	if(x & 256){
		BITSET(no, *SPXX);
	}else{
		BITCLR(no, *SPXX);
	}
	SPRITE_0_COLOR_REGISTER[no] = color;
	SPRITE_POINTERS[no] = pat_no;
}*/

#define put_sprite(no, x, y, color, pat_no) {\
	BITSET(no, *SPRITE_ENABLE_REGISTER);\
	SP0Y[no * 2] = y;\
	SP0X[no * 2] = x & 0xff;\
	if(x & 256){\
		BITSET(no, *SPXX);\
	}else{\
		BITCLR(no, *SPXX);\
	}\
	SPRITE_0_COLOR_REGISTER[no] = color;\
	SPRITE_POINTERS[no] = pat_no;\
}

char sprite_pattern_no[8], sprite_color_no[8];
int spr_x[8], spr_y[8];

/* sprite position set */
void set_sprite_pos(char num, int posx, int posy)
{
	spr_x[num] = posx + 8;
	spr_y[num] = posy + 16 + 16 + 2;
}

char patterncolor_table[] = {
	COLOR_YELLOW,
	COLOR_WHITE,
	COLOR_BLUE,
	COLOR_LIGHT_BLUE,

	COLOR_WHITE,
	COLOR_LIGHT_BLUE,

	COLOR_BLUE,
	COLOR_LIGHT_BLUE,
};


void set_sprite_attribute(char num, char no) {
	sprite_pattern_no[num] = 0x0f - no;
	sprite_color_no[num] = patterncolor_table[no];
}

void put_sprite_all(void) {
	char i;
	for(i = 0; i < 8; ++i){
		put_sprite(i, spr_x[i], spr_y[i], sprite_color_no[i], sprite_pattern_no[i]);
	}
}


void set_sprite_mychr(short x, unsigned char y)
{
	set_sprite_pos(0, x, y);
	set_sprite_pos(1, x, y);
	set_sprite_pos(2, x, y);
	set_sprite_pos(3, x, y);
}

void set_sprite_blockchr(short x, unsigned char y)
{
	set_sprite_pos(4, x, y);
	set_sprite_pos(5, x, y);
}

void set_sprite_slimechr(short x, unsigned char y)
{
	set_sprite_pos(6, x, y);
	set_sprite_pos(7, x, y);
}

unsigned char keyscan(void)
{
	unsigned char keycode = 0;
			/* PAD */
			*portadir = 224;
			*porta = 0xff;
			if(!BITTST(0, *portb)){	/* UP */
				keycode |= KEY_UP1;
			}
			if(!BITTST(1, *portb)){	/* DOWN */
				keycode |= KEY_DOWN1;
			}
			if(!BITTST(2, *portb)){	/* LEFT */
				keycode |= KEY_LEFT1;
			}
			if(!BITTST(3, *portb)){	/* RIGHT */
				keycode |= KEY_RIGHT1;
			}
			if(!BITTST(4, *portb)){	/* SHOT */
				keycode |= KEY_A;
			}

			if(keycode)
				goto skip;

			/* KEYBOARD */
			*portadir = 255;
			*porta = 0xef;

			if(!BITTST(1, *portb)){	/* I */
				keycode |= KEY_UP1;
			}
			if(!BITTST(4, *portb)){	/* M */
				keycode |= KEY_DOWN1;
			}
			if(!BITTST(2, *portb)){	/* J */
				keycode |= KEY_LEFT1;
			}
			if(!BITTST(5, *portb)){	/* K */
				keycode |= KEY_RIGHT1;
			}
			if(!BITTST(7, *portb)){	/* N */
				keycode |= KEY_A;
			}
			if(!BITTST(3, *portb)){	/* 0 */
				keycode |= KEY_B;
			}

			*porta = 0x7f;
			if(!BITTST(4, *portb)){	/* ' ' */
				keycode |= KEY_A;
			}
//			if(!BITTST(1, *portb)){	/* <- */
//				keycode |= KEY_B;
//			}
			if(!BITTST(7, *portb)){	/* STOP */
				keycode |= KEY_B;
			}

			*porta = 0xfd;
			if(!BITTST(4, *portb)){	/* Z */
				keycode |= KEY_A;
			}
			*porta = 0xfb;
			if(!BITTST(7, *portb)){	/* X */
				keycode |= KEY_B;
			}

skip:
			return keycode;
}

void se(void)
{
	unsigned char *s=(unsigned char *)54272;
	s[4] = 128;			/* stop */
	for(unsigned char l=0; l<25; ++l)
		s[l] = 0;
	s[0] = 24;		/* noise freq. */
	s[1] = 33;
	s[5] = 8;		/* A=0,D=8 */
	s[22] = 112;	/* high cut filter */
	s[23] = 1;		/* voice1 filter ON */
	s[24] = 79;		/* volume=15 + High-Pass */
	s[4] = 129;		/* Noise start */
}

void seoff(void)
{
	unsigned char *s=(unsigned char *)54272;
	s[4] = 128;			/* stop */
	for(unsigned char l=0; l<25; ++l)
		s[l] = 0;
	s[0] = 0;		/* noise freq. */
	s[1] = 0;
	s[5] = 8;		/* A=0,D=8 */
	s[22] = 112;	/* high cut filter */
	s[23] = 1;		/* voice1 filter ON */
	s[24] = 79;		/* volume=15 + High-Pass */
	s[4] = 129;		/* Noise start */
}

short x;
unsigned char y, color;

unsigned char i = 0, keycode, oldcharset, data = 0;
short j;
unsigned char k,l;

#define BASE_CHR 0x1b

void set_chr(unsigned char no, unsigned char *srcdata){
	unsigned char data;
	for(l = 0; l <= 3; ++l){
		for(chrdata = (unsigned char *)((CHRDATA + (BASE_CHR) * 8 + (no * 4 + l) * 8 * 1)), k = l*8; k < 8+l*8; ++chrdata, ++k){
			data = srcdata[k];
			*chrdata = data;
			*(chrdata + 128 * 8) = 255 - data;
		}
	}
}

void set_sprite_pattern(unsigned char *sprShapeData, unsigned char *spr)
{
	for(i = 0; i < 63; i++){
		sprShapeData[i] = spr[i];
	}
}

void put_chr16_color(unsigned char x, unsigned char y, char pat, unsigned char color)
{
	put_chr_color(x*1, y*1, BASE_CHR + pat, color);
	put_chr_color(x*1+1, y*1, BASE_CHR + pat+1, color);
	put_chr_color(x*1, y*1+1, BASE_CHR + pat+2, color);
	put_chr_color(x*1+1, y*1+1, BASE_CHR + pat+3, color);
}

void put_chr16(unsigned char x, unsigned char y, char chr)
{
	x += 2;

	switch(chr){
		case TILE_PLAYER:
			set_sprite_mychr(x*16+16, y*16+16);
			put_sprite_all();
			break;

		case TILE_SLIME:
			set_sprite_slimechr(x*16+16, y*16+16);
			put_sprite_all();
			break;

		case TILE_GRAVITY:
			set_sprite_blockchr(x*16+16, y*16+16);
			put_sprite_all();
			break;

		case TILE_WALL:
			put_chr16_color(x * 2, y * 2, 0x40+2*0, COLOR_YELLOW);
			break;

		case TILE_NORMAL:
			put_chr16_color(x * 2, y * 2, 0x40+2*4, COLOR_LIGHT_GREEN);
			break;

		case TILE_HOLLOW:
			put_chr16_color(x * 2, y * 2, 0x40+3*4, COLOR_LIGHT_BLUE);
			break;

		case TILE_GOAL:
			put_chr16_color(x * 2, y * 2, 0x40+4*4, COLOR_WHITE);
			break;

		default:
			put_chr8(x*2, y*2, ' ');
			put_chr8(x*2+1, y*2, ' ');
			put_chr8(x*2, y*2+1, ' ');
			put_chr8(x*2+1, y*2+1, ' ');
			break;
	}
}

void cls(void) {
	int i,j;

	set_sprite_mychr(0, 0);
	set_sprite_blockchr(0, 0);
	set_sprite_slimechr(0, 0);
	put_sprite_all();

	for(j = 0l; j < 24; j++)
		for(i = 0; i < 40; ++i)
			put_chr8(i, j, ' ');
}

void put_logo(int x, int y)
{
	print_at(x, y+1, " 2026 maZone");
}

void define_tiles(void)
{
}

void vsync(void)
{
	while(*RASTER == 249);
	while(*RASTER != 249);
}

void wait(int j) {
	int i;
	for (i = 0; i < j; ++i)
		vsync();
}

void play_sound_effect(void) {
//	beep();  // シンプルなビープ音
}

#include "common.h"

int main(void)
{
	//Copy font ROM -> RAM
	*interrupt = *interrupt & 0xfe;
	oldcharset = *charset;
	*charset = *charset & 0xfb;
	*charset |= 0x03;	// BASIC ROM

//	copy();
	for(j = 0, chrdata = (unsigned char *)(CHRDATA), chrdata2 = (unsigned char *)0xd000; j < 4096; ++j, ++chrdata, ++chrdata2){
		*chrdata = *chrdata2;
	}
/*	for(chrdata = (unsigned char *)53248; chrdata <= (unsigned char *)57343; ++chrdata){
			*(chrdata - 40960) = *chrdata;
	}*/

//	*charset = *charset | 0x04;
	*charset = oldcharset;
	*interrupt = *interrupt | 0x01;
	oldcharsetram = *charsetram;
	*charsetram = *charsetram & 0xf1 | 0x02;	/* Pointer of CHRDATA */
	oldvicbank = *vicbank;
	*vicbank = *vicbank & 0xfc | 0x01;	/* VICBANK OFFSET */

//	register unsigned char j;

	set_chr(0, block1_data);
	set_chr(2, block2_data);
	set_chr(3, block4_data);
	set_chr(4, goal_data);

/*	asm(
		"txa\n"
		"pha\n"
		"tya\n"
		"pha\n"
		"jsr $e544\n"
		"pla\n"
		"tay\n"
		"pla\n"
		"tax\n"
	);*/

	// Clear Screen Black
	*BoaderColorRegister = 0;
	*BackgroundColor = 0;

	*SCREENCONTROL1 = *SCREENCONTROL1 & 0x9f;
	*SCREENCONTROL2 = *SCREENCONTROL2 | 0x10;

	*EXTBGCOOLR1 = COLOR_WHITE;
	*EXTBGCOOLR2 = COLOR_LIGHT_RED;

	for(;;){
		screenarea0[i] = screenarea1[i] = screenarea2[i] = screenarea3[i] = 0x20;

		foregroundarea0[i] = foregroundarea1[i] = foregroundarea2[i] = foregroundarea3[i] = 0x01;
		++i;
		if(i == 0)
			break;
	}


	// Set SPRITE Data
//	*SPRITE_ENABLE_REGISTER = 0x01;

	BITCLR(0, *SPRITE_MULTICOLOR_REGISTERS);
	*SPMC0 = COLOR_LIGHT_RED; //YELLOW; //WHITE;
	*SPMC1 = COLOR_WHITE; //LIGHT_RED;
//	SPRITE_POINTERS[0] = 0x0d;	/* 13 */

	set_sprite_pattern(spr04ShapeData, mychr_sprite_color_5);
	set_sprite_pattern(spr03ShapeData, mychr_sprite_color_1);
	set_sprite_pattern(spr02ShapeData, mychr_sprite_color_7);
	set_sprite_pattern(spr01ShapeData, mychr_sprite_color_6);

	set_sprite_pattern(spr06ShapeData, block_sprite_color_5);
	set_sprite_pattern(spr05ShapeData, block_sprite_color_7);

	set_sprite_pattern(spr08ShapeData, slime_sprite_color_5);
	set_sprite_pattern(spr07ShapeData, slime_sprite_color_1);

	color = COLOR_YELLOW; //LIGHT_RED; //COLOR_YELLOW;
	x = 176;
//	*MSGIGX = 0;
//	*SP0X = x;
	y = 178;


	for(i = 0; i < 8; ++i){
		char patno;
		BITCLR(i, *SPRITE_MULTICOLOR_REGISTERS);	// Mono color
		set_sprite_attribute(i, i);
	}
	put_sprite_all();

	*EXTBGCOOLR1 = COLOR_WHITE;
	*EXTBGCOOLR2 = COLOR_LIGHT_RED;

#ifdef DEBUG2
	for(;;)
#endif
		main2();

#ifdef DEBUG2
	*SPRITE_ENABLE_REGISTER = 0x0;
	*SCREENCONTROL2 = *SCREENCONTROL2 & 0xef;
//	*interrupt = *interrupt & 0xfe;
//	*charset = *charset | 0x04;
//	*interrupt = *interrupt | 0x01;
	*charsetram = oldcharsetram;
	*vicbank = oldvicbank;
	*((unsigned char *)0x2b) = 1;
	*((unsigned char *)0x2c) = 8;
#endif

	return 0;
}
