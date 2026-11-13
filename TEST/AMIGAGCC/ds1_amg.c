/* ds1_amg.c By m@3 with Grok and Gemini 2026. */

#define SCREEN_WIDTH (320)
#define SCREEN_HEIGHT (200)
 // PAL
#define BITPLANE_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 8)

#include <stdio.h>
#include <stdlib.h>

#include <hardware/Custom.h>
#include <hardware/dmabits.h>

#include <hardware/cia.h>

#include <proto/exec.h>
#include <proto/lowlevel.h>

#include <libraries/dos.h>
#include <proto/dos.h>
#include <clib/intuition_protos.h>

#include <clib/graphics_protos.h>


#include "mode.h"
#include "inkey.h"

#define PRINT_MUL 1

#define A_KEY "Z"
#define B_KEY "X"

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

extern int my_mod(int a, int b) __attribute__((regparm(2)));
extern int my_div(int a, int b) __attribute__((regparm(2)));

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
#define BITDATA(n) (1 << (n))

/* BITセット */
#define BITSET2(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BITクリア */
#define BITCLR2(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BITチェック */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT反転 */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}

#define Custom_BASE 0xDFF000
#define Custom (*(volatile struct Custom *)Custom_BASE)

#define bplpt 0x0e0
#define sprpt2 0x120

 //0x4000
#define BITPLANE_NUM 4
#define SPRITE_SIZE 0x100
#define SPRITE_NUM 8

enum {
	BPL1PTH = bplpt + 0x00,
	BPL1PTL = bplpt + 0x02,

	BPL2PTH = bplpt + 0x04,
	BPL2PTL = bplpt + 0x06,

	BPL3PTH = bplpt + 0x08,
	BPL3PTL = bplpt + 0x0a,

	BPL4PTH = bplpt + 0x0c,
	BPL4PTL = bplpt + 0x0e,

	SPR0PT  = sprpt2  + 0x00,
	SPR0PTH = SPR0PT + 0x00,
	SPR0PTL = SPR0PT + 0x02,

	SPR1PT  = sprpt2  + 0x04,
	SPR1PTH = SPR1PT + 0x00,
	SPR1PTL = SPR1PT + 0x02,

	SPR2PT  = sprpt2  + 0x08,
	SPR2PTH = SPR2PT + 0x00,
	SPR2PTL = SPR2PT + 0x02,

	SPR3PT  = sprpt2  + 0x0C,
	SPR3PTH = SPR3PT + 0x00,
	SPR3PTL = SPR3PT + 0x02,

	SPR4PT  = sprpt2  + 0x10,
	SPR4PTH = SPR4PT + 0x00,
	SPR4PTL = SPR4PT + 0x02,

	SPR5PT  = sprpt2  + 0x14,
	SPR5PTH = SPR5PT + 0x00,
	SPR5PTL = SPR5PT + 0x02,

	SPR6PT  = sprpt2  + 0x18,
	SPR6PTH = SPR6PT + 0x00,
	SPR6PTL = SPR6PT + 0x02,

	SPR7PT  = sprpt2  + 0x1C,
	SPR7PTH = SPR7PT + 0x00,
	SPR7PTL = SPR7PT + 0x02,
};

enum {
	BITPLANE1,
	BITPLANE2,
	BITPLANE3,
	BITPLANE4,
	SPR0,
	SPR1,
	SPR2,
	SPR3,
	SPR4,
	SPR5,
	SPR6,
	SPR7
};


unsigned short COPPERL[] = {
	BPL1PTH,0x0002, //;Bitplane 1 pointer = $22000
	BPL1PTL,0x2000,

	BPL2PTH,0x0002, //;Bitplane 2 pointer = $26000
	BPL2PTL,0x6000,

	BPL3PTH,0x0002, //;Bitplane 3 pointer = $2a000
	BPL3PTL,0xa000,

	BPL4PTH,0x0002, //;Bitplane 4 pointer = $2e000
	BPL4PTL,0xe000,


	SPR0PTH,0x0002, //;Sprite 0 pointer = $21000
	SPR0PTL,0x1000,
	SPR1PTH,0x0002, //;Sprite 1 pointer = $21100
	SPR1PTL,0x1100,
	SPR2PTH,0x0002, //;Sprite 2 pointer = $21200
	SPR2PTL,0x1200,
	SPR3PTH,0x0002, //;Sprite 3 pointer = $21300
	SPR3PTL,0x1300,
	SPR4PTH,0x0002, //;Sprite 4 pointer = $21400
	SPR4PTL,0x1400,
	SPR5PTH,0xd002, //;Sprite 5 pointer = $21500
	SPR5PTL,0x1500,
	SPR6PTH,0x0002, //;Sprite 6 pointer = $21600
	SPR6PTL,0x1600,
	SPR7PTH,0x0002, //;Sprite 7 pointer = $21700
	SPR7PTL,0x1700,
	0xFFFF,0xFFFE, //End of Copper list
};
//	;;
//	;;  Sprite data
//	;;
unsigned short SPRITE[] = {
	0x6D60,0x7200, //VSTART, HSTART, VSTOP
	0x07f0, 0x0808,	// RAIN CHR
	0x0808, 0x17f4,
	0x13e4, 0x2c1a,
	0x1ffc, 0x2002,
	0x16ec, 0x2912,
	0x16ec, 0x2912,
	0x1ddc, 0x2ffa,
	0x1ddc, 0x0ff8,
	0x0ff8, 0x03e0,
	0x1ffc, 0x1ffc,
	0x300f, 0x6ff3,
	0x3013, 0x6fef,
	0x1024, 0x1fde,
	0x0ff8, 0x01e0,
	0x0000, 0x0ff8,
	0x0630, 0x19cc,

	0x0000,0x0000, //End of sprite data
};

unsigned short SPRITE2[] = {
	0x6D60,0x7200, //VSTART, HSTART, VSTOP

	0x03c0, 0x03c0,
	0x03c0, 0x03c0,
	0x03c0, 0x03c0,
	0x03c0, 0x03c0,

	0x03c0, 0x03c0,
	0x03c0, 0x03c0,
	0x03c0, 0x03c0,
	0x03c0, 0x03c0,

	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,

	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,

	0x0000, 0x0000  // short
};

unsigned short SPRITE3[] = {
	0x8080,0x8D00,		// TOFU
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,

	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,

	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,

	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,
	0xffff,0x0000,

	0x0000, 0x0000
};


// max/min マクロ
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))


// 構造体
typedef struct {
	int x, y;
	int active;
} Entity;

int score = 0, hiscore = 5000;
int combo = 0;

unsigned char org_pal[16][3] = {
	{  0,  0,  0},
	{  0,  0,  0},
	{  3, 13,  3},
	{  7, 15,  7},
	{  3,  3, 15},
	{  5,  7, 15},
	{ 11,  3,  3},
	{  5, 13, 15},
	{ 15,  3,  3},
	{ 15,  7,  7},
	{ 13, 13,  3},
	{ 13, 13,  7},
	{  3,  9,  3},
	{ 13,  5, 11},
	{ 11, 11, 11},
	{ 15, 15, 15},
};

#define MSXWIDTH 256
#define MSXLINE 212

unsigned char pattern[10];
unsigned char msxcolor[MSXWIDTH / 2][MSXLINE];
unsigned short *vram_adr;

unsigned char read_pattern[MSXWIDTH * MSXLINE * 2+ 2];

BPTR stream[1];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};


short sc5_load(char *loadfil, short x, short y, short msxline, ULONG bitplane_address, ULONG bitplane_size)
{
	long i, j, count, count2;
	int k=0, l=0, m=0;

	unsigned short amgcolor[4];
	unsigned char msxcolor[8];
	unsigned char color;
	ULONG vram_adr;

	if (!(stream[0] = Open( loadfil, MODE_READWRITE))) {
//		printf("Can\'t open file %s.", loadfil);

		Close(stream[0]);
		return 1;
	}
	Read(stream[0], pattern, 1);	/* MSX先頭を読み捨てる */
	Read(stream[0], pattern, 4);	/* MSXヘッダも読み捨てる */

	Read(stream[0], pattern, 2);	/* MSXヘッダを読み捨てる */


	i = Read(stream[0], read_pattern, MSXWIDTH * msxline);
	m = 0;
	for(count2 = 0; count2 < MSXWIDTH * msxline / 4 / 2; ++count2){
	
		/* 色分解 */
		msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
		msxcolor[1] = read_pattern[m++] & 0x0f;
		msxcolor[2] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[3] = read_pattern[m++] & 0x0f;
		msxcolor[4] = (read_pattern[m] >>4) & 0x0f;
		msxcolor[5] = read_pattern[m++] & 0x0f;
		msxcolor[6] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[7] = read_pattern[m++] & 0x0f;

		for(i = 0; i < 4; ++i){
			amgcolor[i] = 0;
		}
		for(j = 0; j < 8; ++j){
			for(i = 0; i < 4; ++i){
				color = conv_tbl[msxcolor[j]];	/* 色変換 */
				if(BITTST(i, color)){
					BITSET2(7-j, amgcolor[i]);
				}else{
					BITCLR2(7-j, amgcolor[i]);
				}
			}
		}
		for(i = 0; i < 4; ++i){
			pattern[i] = amgcolor[i];
		}
		vram_adr = k + l + (x + y * (MSXWIDTH / 8)) + bitplane_address; //); // * 2;
		*((unsigned char *)(vram_adr + bitplane_size * 0)) = pattern[0];
		*((unsigned char *)(vram_adr + bitplane_size * 1)) = pattern[1];
		*((unsigned char *)(vram_adr + bitplane_size * 2)) = pattern[2];
		*((unsigned char *)(vram_adr + bitplane_size * 3)) = pattern[3];

		k += 1;
		if(k >= (MSXWIDTH / 8)){
			k = 0;
			l += (SCREEN_WIDTH / 8);
		}
	}
	Close(stream[0]);

	return 0;
}

int sprite_pattern_no[8], old_sprite_pattern_no[8], spr_x[8], spr_y[8];
unsigned char font_buffer[BITPLANE_NUM][8*4][SCREEN_WIDTH / 8];
unsigned char bg_buffer[BITPLANE_NUM][16*3][SCREEN_WIDTH / 8];

char chr;
unsigned char str_temp[11];
#define SCREEN2 0
#define CHRPAL_NO 0

APTR base_address;
APTR sprite_address;
APTR bitplane_address;

void print_at(int x, int y,  char *str)
//, unsigned char pal)
{
	ULONG vram_adr;
	int k, l, i;

	x += (32/8);
	y *= 8;

	while(1){
		chr = *(str++);
		if(chr == '\0')
			break;

		if(chr >= 'a')
			chr -= ('a'-'A');

		if((chr < 0x30))
			chr = 0x40;

		chr -= '0';

		k = chr & 0x0f;
		l = chr / 16 * 8;

		for(int j = 0; j < 8; ++j){
			vram_adr = (x) + (y + j) * (SCREEN_WIDTH / 8) + (ULONG)bitplane_address;
			for(i = 0; i < BITPLANE_NUM; ++i){
				*((unsigned char *)(vram_adr + BITPLANE_SIZE * i)) = 
					font_buffer[i][l + j][k];
			}
		}

		++x;
	}
}

void print_at_2(int x, int y,  char *str)
//, unsigned char pal)
{
	ULONG vram_adr;
	int k, l, i;

	x += (32/8);
	y *= 8;

	while(1){
		chr = *(str++);
		if(chr == '\0')
			break;
		if((chr < 0x30))
			chr = 0x40;

		chr -= '0';

		k = chr & 0x0f;
		l = chr / 16 * 8;

		for(int j = 0; j < 8; ++j){
			vram_adr = (x) + (y + j) * (SCREEN_WIDTH / 8) + (ULONG)bitplane_address;
			for(i = 0; i < BITPLANE_NUM; ++i){
				*((unsigned char *)(vram_adr + BITPLANE_SIZE * i)) = 
					font_buffer[i][l + j][k];
			}
		}

		++x;
	}
}

void put_numd(int j, unsigned char digit)
{
	unsigned char i = digit;
	int mod = 0;

	while(i--){
		mod = my_mod(j, 10);
		str_temp[i] = mod + 0x30;
		j = my_div(j, 10);
	}
	str_temp[digit] = '\0';
}

#ifdef DEBUG

void score_display(void)
{
	put_numd(score, 8);
	put_strings(SCREEN2, 19, 22 , str_temp, CHRPAL_NO);
	if(score >= hiscore){
		if((my_mod(score, 10)) == 0){
			hiscore = score;
			put_strings(SCREEN2, 12, 22, "HIGH ", CHRPAL_NO);
		}
	}
	else
		put_strings(SCREEN2, 12, 22, "SCORE", CHRPAL_NO);
}

void combo_display(void)
{
	put_numd(combo, 8);
	put_strings(SCREEN2, 19, 24 , str_temp, CHRPAL_NO);
		put_strings(SCREEN2, 12, 24, "COMBO", CHRPAL_NO);
}

void score_displayall(void)
{
//	put_strings(SCREEN2, 9, 22, "SCORE", CHRPAL_NO);
	score_display();
}

void hiscore_display(void)
{
/*	if(score > hiscore)
		if((score % 10) == 0)
			hiscore = score;
*/
	put_numd(hiscore, 8);

	put_strings(SCREEN2, 12, 24, "HIGH ", CHRPAL_NO);
	put_strings(SCREEN2, 19, 24, str_temp, CHRPAL_NO);
}

void hiscore_display_clear(void)
{
	put_strings(SCREEN2, 12, 24, "	 ", CHRPAL_NO);
	put_strings(SCREEN2, 19, 24, "		", CHRPAL_NO);
}

#endif

// スプライトポインタ設定
void set_sprite(int num, int posx, int posy) {
	spr_x[num] = posx;
	spr_y[num] = posy;
}

void set_sprite_all(APTR sprite_base_address) {
	int no;
	unsigned long *pointer, *pointer2;
	for(int i = 0; i < 8; ++i){

		if((no = sprite_pattern_no[i]) != old_sprite_pattern_no[i]){
			old_sprite_pattern_no[i] = no;

			pointer = (unsigned long *)((unsigned char *)sprite_base_address + 0x100 * i);
			switch(no){
				case 0:
					pointer2 = (unsigned long *)SPRITE;
					break;

				case 1:
					pointer2 = (unsigned long *)SPRITE2;
					break;

				case 2:
					pointer2 = (unsigned long *)SPRITE3;
					break;
			}

			pointer++;
			pointer2++;
			for(int j = 1; j < 17; ++j){
				*(pointer++) = *(pointer2++);
			}
		}

		int V_START = spr_y[i] + 44 - 16;
		int V_STOP = V_START + 16;
		int H_START = spr_x[i] + 128 - 16;
		unsigned char *pointer = sprite_base_address  + SPRITE_SIZE * i;
		*((unsigned char *)pointer + 0) = V_START & 0xff;		// V_START

		*((unsigned char *)pointer + 1) = (H_START >> 1) & 0xff;	// H_START

		*((unsigned char *)pointer + 2) = V_STOP & 0xff;	// V_STOP

		*((unsigned char *)pointer + 3) = 0x80 * 0 | ((V_STOP & 0x100) << 2) | ((V_START & 0x100) << 1) | (H_START & 0x01);
	}
}


void set_sprite_pattern(int num, int no) {
	sprite_pattern_no[num] = no;
}

void set_copperl(ULONG bitplane_address, ULONG sprite_address)
{
	for(int i = 0 ; i < BITPLANE_NUM; ++i){
		COPPERL[(BITPLANE1 + i) * 4 + 1] = ((bitplane_address >> 16) & 0xffff); //0x0002;
		COPPERL[(BITPLANE1 + i) * 4 + 3] = (bitplane_address & 0xffff) + BITPLANE_SIZE * i;
	}

	for(int i = 0 ; i < SPRITE_NUM; ++i){
		COPPERL[(SPR0 + i) * 4 + 1] = ((sprite_address >> 16) & 0xffff); //0x0002;
		COPPERL[(SPR0 + i) * 4 + 3] = (sprite_address & 0xffff) + SPRITE_SIZE * i;
	}
}

#define rawsize 2000

APTR rawbuffer;

int raw_load(char *loadfil, char *mem, int size)
{
	if (!(stream[0] = Open( loadfil, MODE_READWRITE))) {
//		printf("Can\'t open file %s.", loadfil);
		print_at( 0, 0, "ERROR");

		Close(stream[0]);
		return 1;
	}
	size = Read(stream[0], mem, size);
	Close(stream[0]);
	return size;
}

void se(void)
{
	unsigned int *p;
	unsigned short *pp;

	pp = (unsigned short *)0x0dff096;
	*pp = 0x0001;	/* DMA OFF (ch0) */

	p = (unsigned int *)0x0dff0a0;
	*p = (unsigned int)rawbuffer;	/* ch.0 address */

	pp = (unsigned short *)0x0dff0a4;
	*pp = 1600;	/* length */
	pp = (unsigned short *)0x0dff0a6;
	*pp = 447;	/* pitch */
	pp = (unsigned short *)0x0dff0a8;
	*pp = 64;	/* volume(0-64)  */
	pp = (unsigned short *)0x0dff096;
	*pp = 0x8001;	/* DMA ON (ch0) */

/*
lea.l sample_start, a1	 ; サンプルアドレス
move.l a1, $DFF0A0		 ; ch0 アドレス
move.w #SAMPLE_LENGTH, $DFF0A4  ; 長さ (e.g. 100)
move.w #700, $DFF0A6	   ; ピッチ
move.w #0, $DFF0A8		 ; ボリューム0スタート

move.w #$8001, $DFF096	 ; DMA ON (ch0)

; ボリューム/ピッチ徐々変化 (ループ)
moveq #0, d1			   ; vol=0
move.l #700, d2			; per=700
up_loop:
	move.w d1, $DFF0A8
	move.w d2, $DFF0A6
	addq #1, d1
	subq #8, d2
	dbra #63, up_loop	 ; 64回

move.w #$0001, $DFF096	 ; DMA OFF*/
}

void seoff(void)
{
	unsigned short *pp;

	pp = (unsigned short *)0x0dff096;
	*pp = 0x0001;	/* DMA OFF (ch0) */
}

void vsync(void)
{
	WaitTOF();
}

void cls2(void)
{
	long i;
	// 画面クリア
	i = BITPLANE_SIZE - 4;

	vsync();
	do{
		*((unsigned long *)(unsigned char *)(bitplane_address + i + BITPLANE_SIZE * 0)) = 0; //xffffffffL;
		*((unsigned long *)(unsigned char *)(bitplane_address + i + BITPLANE_SIZE * 1)) = 0; //xffffffffL; //xffffffffL;
		*((unsigned long *)(unsigned char *)(bitplane_address + i + BITPLANE_SIZE * 2)) = 0; //xffffffffL;
		*((unsigned long *)(unsigned char *)(bitplane_address + i + BITPLANE_SIZE * 3)) = 0x0; //ffffffffL;
	}while((i-=4) >= 0);
}

void cls(void) {
	int i,j;

	cls2();
//	for(j = 0l; j < 24; j++)
//		for(i = 0; i < 40; ++i)
//			put_chr8(i, j, ' ');
}

void put_logo(int x, int y)
{
	print_at_2(x, y,   "      i  k   ");
	print_at_2(x, y+1, " 2026 bcdefgh");
}

void define_tiles(void)
{
}

void wait(int j) {
	int i;
	for (i = 0; i < j; ++i)
		vsync();
}

void play_sound_effect(void) {
//	beep();  // シンプルなビープ音
}

void put_chr16(short x, short y, char chr)
{
	ULONG vram_adr;
	x += (32/16);

	x*=2;
	y*=16;

	for(short k = 0; k < 2; ++k){
		for(short j = 0; j < 16; ++j){
			vram_adr =// (short unsigned int *)
				(x + k) + (y + j) * (SCREEN_WIDTH / 8) + (ULONG)bitplane_address;
			for(int i = 0; i < BITPLANE_NUM; ++i){
				*((unsigned char *)(vram_adr + BITPLANE_SIZE * i)) = 
					bg_buffer[i][j][(128 / 8)*0 + chr*2+k];
			}
		}
	}
}

unsigned char keyscan2(unsigned char keycode);
int keywindowopen(void);
int keywindowclose(void);

unsigned char keycode = 0;
unsigned char keycode2 = 0;

unsigned char keyscan(void)
{
	keycode = 0;

	ULONG joy = ReadJoyPort(1);

	// 移動 (仮にキーボードor joy direct)
	if (joy & JPF_JOY_UP){  // adjust bit
		keycode |= KEY_UP1;
	}
	if (joy & JPF_JOY_DOWN) {  // adjust bit
		keycode |= KEY_DOWN1;
	}
	if (joy & JPF_JOY_LEFT) {  // adjust bit
		keycode |= KEY_LEFT1;
	}
	if (joy & JPF_JOY_RIGHT) {  // adjust bit
		keycode |= KEY_RIGHT1;
	}

	// 射撃 (ボタン押したら弾発射)
	if (joy & JPF_BTN1) {  // adjust bit
		keycode |= KEY_A;
	}
	if (joy & JPF_BTN2) {  // adjust bit
		keycode |= KEY_B;
	}


	if(keycode)
		return keycode;

	keycode2 = keyscan2(keycode2);

	return keycode2;
}

#include "common.h"

int main(void)
{
	long i = 0;
	unsigned long *pointer, *pointer2;

	bitplane_address = AllocMem(BITPLANE_SIZE * BITPLANE_NUM, MEMF_CHIP | MEMF_CLEAR);
	if (!bitplane_address) {
//		printf("cannot open lowlevel.library\n");
		return 10;
	}
	rawbuffer = AllocMem(rawsize, MEMF_CHIP | MEMF_CLEAR);
	if (!rawbuffer) {
//		printf("cannot open lowlevel.library\n");
		return 10;
	}
	base_address = AllocMem((BITPLANE_NUM + SPRITE_NUM + 1) * 8, MEMF_CHIP | MEMF_CLEAR);
	if (!base_address) {
//		printf("cannot open lowlevel.library\n");
		return 10;
	}
	sprite_address = AllocMem( SPRITE_SIZE * SPRITE_NUM, MEMF_CHIP | MEMF_CLEAR );
	if (!sprite_address) {
//		printf("cannot open lowlevel.library\n");
		return 10;
	}


	struct Library *LowLevelBase = OpenLibrary("lowlevel.library", 39L);
	if (!LowLevelBase) {
//		printf("cannot open lowlevel.library\n");
		return 10;
	}

	/* 一時停止 */
	Custom.dmacon = 0x8120;
	Custom.intena = 0x8000;

	/* 画面設定 */
	Custom.bplcon0 = 0x4200;
	Custom.bpl1mod = 0x0000; //MOVE.W  #$0000,BPL1MOD(a0) ;Modulo = 0
	Custom.bplcon1 = 0x0000; //MOVE.W  #$0000,BPLCON1(a0) ;Horizontal scroll value = 0

	Custom.bplcon2 = 0x0024; //	MOVE.W  #$0024,BPLCON2(a0) ;Sprites have priority over playfields
	Custom.ddfstrt = 0x0038; //	MOVE.W  #$0038,DDFSTRT(a0) ;Set data-fetch start
	Custom.ddfstop = 0x00d0; //	MOVE.W  #$00D0,DDFSTOP(a0) ;Set data-fetch stop

	Custom.diwstrt = 0x2c81;
	Custom.diwstop = 0xf4c1;

	// set vram & sprite
	set_copperl((ULONG)bitplane_address, (ULONG)sprite_address);
	pointer = (unsigned long *)base_address; //0x20000L;
	pointer2 = (unsigned long *)COPPERL;

	do{
		*(pointer++) = *(pointer2++);
	}while(*pointer2 != 0xfffffffe);


	for(i = 0; i < 8; ++i){
		old_sprite_pattern_no[i] = 255;
		pointer = (unsigned long *)((unsigned char *)sprite_address + 0x100 * i);
		switch(i){
			case 0:
				set_sprite_pattern(i, 0);
				break;

			case 1:
			case 2:
			case 3:
				set_sprite_pattern(i, 1);
				break;

			case 4:
			case 5:
			case 6:
			case 7:
				set_sprite_pattern(i, 2);
				break;
		}
	}


	/* COPPERLへのポインタセット */
	Custom.cop1lc = (ULONG)base_address; //0x20000L; //MOVE.L  #$20000,COP1LC(a0)
	Custom.copjmp1 = 0;
	Custom.dmacon = 0x83a0;

	/* pallet */
	for(i = 0; i < 16; i+=4){
		Custom.color[17 + i] = 0xff0;	/* SPR-0 Yellow */
		Custom.color[18 + i] = 0xf00;	/* SPR-1 Red */
		Custom.color[19 + i] = 0xfff;	/* SPR-2 White */
	}
	for(i = 0; i < 16; ++i)
		Custom.color[i] = org_pal[i][2] / 1 | ((org_pal[i][1]/1) << 4) | ((org_pal[i][0]/1) << 8);

	Custom.dmacon = DMAF_SETCLR | DMAF_SPRITE;

	cls2();


	sc5_load("ds1_8bit.sc5", 0, 0, 16*3, (ULONG)&bg_buffer[0][0][0], (SCREEN_WIDTH / 8 * 16 * 3));
	sc5_load("FONTYOKO.SC5", 0, 0, 8*4, (ULONG)&font_buffer[0][0][0], (SCREEN_WIDTH / 8 * 8 * 4));

//	raw_load("se1.raw", rawbuffer, rawsize);


	ULONG vram_adr;
	int x = 0,y = 18 * 8;
	for(int k = 0; k < SCREEN_WIDTH / 8; k+=2){
		for(int j = 0; j < 1; j+=2){
			put_chr16(x + k, y + j, 0);
/*		for(int j = 0; j < 16; ++j){
			vram_adr = (x + k) + (y + j) * (SCREEN_WIDTH / 8) + (ULONG)bitplane_address;
			for(i = 0; i < BITPLANE_NUM; ++i){
				*((unsigned char *)(vram_adr + BITPLANE_SIZE * i)) = 
					bg_buffer[i][j][128 / 8];
			}
*/		}
	}

	keywindowopen();

#ifdef DEBUG2
	for(;;)
#endif
		main2();

#ifdef DEBUG
	for(;;){
		Entity player = {160, 160-16};
		Entity bullets[3] = {{0}};
		Entity enemies[4] = {{0}};
		Entity pluses[8] = {{0}};  // Plus最大8個

		score = 0;
		combo = 0;
		int spawn_timer = 0;
		int combo_timer = 0;
		int wave = 1;
		int enemies_killed_this_wave = 0;
		int game_over = 0;
		int score_disp_flag = 0;
		int combo_display_flag = 0;
		int enemy_speed = 2;

		int count = 0;
		int seflag = 0;
		int secounter = 0;

		score_displayall();
//		combo_display();

		while (!game_over) {
			++count;
			if(combo)
				++combo_timer;
			ULONG joy =ReadJoyPort(1);

			// 移動 (仮にキーボードor joy direct)
			if (joy & JPF_JOY_UP) player.y -= 3;  // up
			if (joy & JPF_JOY_DOWN) player.y += 3;  // down
			if (joy & JPF_JOY_LEFT) player.x -= 3;  // left
			if (joy & JPF_JOY_RIGHT) player.x += 3;  // right

			player.x = MAX(16, MIN(((SCREEN_WIDTH)), player.x));
			player.y = MAX(16, MIN(((SCREEN_HEIGHT)), player.y));

			set_sprite(0, player.x, player.y);

			// 射撃 (ボタン押したら弾発射)
			if (joy & JPF_BTN1) {  // adjust bit
				for (int i = 0; i < 3; i++) {
					if (!bullets[i].active) {
						bullets[i].active = 1;
						bullets[i].x = player.x;
						bullets[i].y = player.y;// - 16;
						break;
					}
				}
			}

			// 弾更新
			for (int i = 0; i < 3; i++) {
				if (bullets[i].active) {
					bullets[i].y -= 8;
					if (bullets[i].y < 0) bullets[i].active = 0;
					set_sprite(i+1, bullets[i].x, bullets[i].y);
				} else {
					set_sprite(i+1, 0, 0);  // hide
				}
			}

		// 敵スポーン&更新
			if (++spawn_timer > 10) {
				spawn_timer = 0;
				for (int i = 0; i < 4; i++) {
					if (!enemies[i].active) {
						enemies[i].active = 1;
						enemies[i].x = 16 + ((rand() & ((SCREEN_WIDTH - 16) * 4 / 5 + 1)) * 5 / 4);
						enemies[i].y = 0;
						break;
					}
				}
			}

			for (int i = 0; i < 4; i++) {
				if (enemies[i].active) {
					if (!pluses[i].active) {
						set_sprite_pattern(i+4, 2);
						enemies[i].y += enemy_speed;
						if (enemies[i].y > (SCREEN_HEIGHT + 16)) enemies[i].active = 0;
						set_sprite(i+4, enemies[i].x, enemies[i].y);
					} else {
						set_sprite(i+4, 0, 0);
					}
				}else{
					set_sprite(i+4, 0, 0);
				}
			}

			// 衝突判定: 弾 vs 敵
			for (int b = 0; b < 3; b++) {
				if (!bullets[b].active) continue;
				for (int e = 0; e < 4; e++) {
					if (pluses[e].active) continue;
					if (!enemies[e].active) continue;
					if (abs(bullets[b].x - enemies[e].x) < 16 && abs(bullets[b].y - enemies[e].y) < 16) {
						bullets[b].active = 0;
//						enemies[e].active = 0;
						set_sprite(b+1, 0, 0);
						set_sprite(e+4, 0, 0);

//						if(!secounter)
//						if(!seflag)
							seoff();
							seflag = 1;
						// Plus生成
//						for (int p = 0; p < 1; p++) {
//							if (!pluses[e].active) {
								pluses[e].active = 1;
								pluses[e].x = enemies[e].x;
								pluses[e].y = enemies[e].y;
//								break;
//							}
//						}

						combo++;
						combo_timer = 0;
						combo_display_flag = 1;
						score += 20 * combo;
						score_disp_flag = 1;
						enemies_killed_this_wave++;

						// WAVEクリア判定
						if (enemies_killed_this_wave >= 5 * wave) {
							wave++;
							enemies_killed_this_wave = 0;
							++enemy_speed;
						}
						break;
					}
				}
			}

			// Plus取得
			for (int p = 0; p < 4; p++) {
				if (pluses[p].active) {
					set_sprite_pattern(p+4, 0);
					pluses[p].y += 1;
					if (abs(pluses[p].x - player.x) < 16 && abs(pluses[p].y - player.y) < 16) {
						pluses[p].active = 0;
						enemies[p].active = 0;
						score += 50 * combo;
						score_disp_flag = 1;
						combo_timer = 0;
						combo_display_flag = 1;
					}else{
						set_sprite(p+4, pluses[p].x, pluses[p].y);
						if (pluses[p].y > (SCREEN_HEIGHT + 16)){
							pluses[p].active = 0;
							enemies[p].active = 0;
						}
					}
				}
			}

			// コンボ切れ
			if (combo_timer > 60) {
				combo = 0;
				combo_display_flag = 1;
				combo_timer = 0;
			}

			// ゲームオーバー (敵接触)
			for (int e = 0; e < 4; e++) {
				if (enemies[e].active && abs(enemies[e].x - player.x) < 16 && abs(enemies[e].y - player.y) < 16) {
					if(!pluses[e].active)
						game_over = 1;
				}
			}
//			while(((Custom.vhposr / 256))); // == 0x20));

void vsync(void)
{
	WaitTOF();
}

			set_sprite_all(sprite_address);
			if(seflag){
				se();
				seflag = 0;
				secounter = 8;
			}
			if(secounter){
				--secounter;
				if(!secounter)
					seoff();
			}

			if(score_disp_flag){
				score_disp_flag = 0;
				score_display();
			}
			else if(combo_display_flag){
				combo_display_flag = 0;
				if(combo){
					combo_display();
				}else{
					hiscore_display_clear();
				}
			}

//			put_strings(SCREEN2, 8, 22, "SCORE", CHRPAL_NO);
//	put_numd(score, 8);

//			*((unsigned char *)0x25001) = player.x;	// H_START
//			*((unsigned char *)0x25000) = player.y;		// V_START
//			*((unsigned char *)0x25002) = player.y + 16;	// V_STOP
		}
		seoff();
		for(;;){
			ULONG joy = ReadJoyPort(1);
			if(!(joy & JPF_BTN1))
				break;
		}
		// ゲームオーバー画面
		/* ここにテキスト描画追加可能 */
		hiscore_display();
		for(;;){
			ULONG joy = ReadJoyPort(1);
			if(joy & JPF_BTN1)
				break;
		}
		hiscore_display_clear();
		// (スプライト全消し)
		for (int i = 0; i < 8; i++){
			set_sprite(i, 0, 0);
			set_sprite_all(sprite_address);
		}
		for(;;){
			ULONG joy = ReadJoyPort(1);
			if(!(joy & JPF_BTN1))
				break;
		}
	}
#endif
	keywindowclose();

	FreeMem(sprite_address, SPRITE_SIZE * SPRITE_NUM);
	FreeMem(base_address, (BITPLANE_NUM + SPRITE_NUM + 1) * 8);
	FreeMem(rawbuffer, rawsize);
	FreeMem(bitplane_address, BITPLANE_SIZE * BITPLANE_NUM);

	return(0);
}


#include <exec/types.h>
#include <exec/exec.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/keymap.h>

#include <proto/exec.h>
#include <proto/intuition.h>

#include <stdio.h>

/* Amigaの主要なキーコード (Raw Key Code) の定義 */
#define RAWKEY_UP	 0x4C
#define RAWKEY_DOWN   0x4D
#define RAWKEY_RIGHT  0x4E
#define RAWKEY_LEFT   0x4F
#define RAWKEY_Z	  0x31
#define RAWKEY_X	  0x32
//#define RAWKEY_ESC	(0x45 | IECODE_UP_PREFIX) /* 離されたとき等に利用 */

struct Window *window;

int keywindowopen(void) {
	window = NULL;

	keycode2 = 0;
	window = NULL;

	// 1. 操作を受け付けるウィンドウを開く (IDCMP_RAWKEYを指定)
	window = OpenWindowTags(NULL,
		WA_Title,	   (ULONG)"Amiga Key Input Test",
		WA_Width,	   0,
		WA_Height,	  0,
		WA_IDCMP,	   IDCMP_RAWKEY | IDCMP_CLOSEWINDOW,
		WA_Flags,	   WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE,
		TAG_END
	);

	if (!window) {
		print_at(0,1,"Error: OpenWindow failed\n");
		return 20;
	}

//	print_at(0,1,"Window opened. Press ESC or close window to exit.\n");
//	print_at(0,2,"Use Cursor Keys and Z, X keys.\n");
}

unsigned char keyscan2(unsigned char keycode) {

	// 2. メインループ
//	while (running) {
	BOOL running = TRUE;

		struct IntuiMessage *msg;
		
		// 3. ウィンドウのメッセージポートからイベントを取得
		// WaitPort を使うとCPUを無駄に消費せずイベントを待てます
//		WaitPort(window->UserPort);

		while ((msg = (struct IntuiMessage *)GetMsg(window->UserPort)) != NULL) {
			ULONG class = msg->Class;
			UWORD code  = msg->Code;
			
			// メッセージを返却（ReplyMsg）
			ReplyMsg((struct Message *)msg);

			switch (class) {
				case IDCMP_CLOSEWINDOW:
					running = FALSE;
					break;

				case IDCMP_RAWKEY: {
					// code の最上位ビット (IECODE_UP_PREFIX = 0x80) が立っている場合は「キーを離した」状態
					BOOL is_break = (code & IECODE_UP_PREFIX) != 0;
					UWORD raw_code = code & ~IECODE_UP_PREFIX;

					if (is_break) {
						// --- キーを離したときの処理 ---
						switch (raw_code) {
							case RAWKEY_Z://	 print_at(0,0,"Z released\n");
								keycode &= ~KEY_A;
								break;
							case RAWKEY_X://	 print_at(0,0,"X released\n");
								keycode &= ~KEY_B;
								break;
;
							case RAWKEY_UP://	print_at(0,0,"UP released\n");
								keycode &= ~KEY_UP1;
								break;
							case RAWKEY_DOWN://  print_at(0,0,"DOWN released\n");
								keycode &= ~KEY_DOWN1;
								break;
							case RAWKEY_LEFT://  print_at(0,0,"LEFT released\n");
								keycode &= ~KEY_LEFT1;
								break;
							case RAWKEY_RIGHT:// print_at(0,0,"RIGHT released\n");
								keycode &= ~KEY_RIGHT1;
								break;
						}
					} else {
						// --- キーを押したときの処理 ---
						switch (raw_code) {
							case 0x45: // ESCキー
								running = FALSE;
								break;
							case RAWKEY_Z:
//								print_at(0,0,"Z pressed\n");
								keycode |= KEY_A;
								break;
							case RAWKEY_X:
//								print_at(0,0,"X pressed\n");
								keycode |= KEY_B;
								break;
							case RAWKEY_UP:
//								print_at(0,0,"UP pressed\n");
								keycode |= KEY_UP1;
								break;
							case RAWKEY_DOWN:
//								print_at(0,0,"DOWN pressed\n");
								keycode |= KEY_DOWN1;
								break;
							case RAWKEY_LEFT:
//								print_at(0,0,"LEFT pressed\n");
								keycode |= KEY_LEFT1;
								break;
							case RAWKEY_RIGHT:
//								print_at(0,0,"RIGHT pressed\n");
								keycode |= KEY_RIGHT1;
								break;
						}
					}
					break;
				}
			}
		}
//	}
	return keycode;
}

int keywindowclose(void) {
	// 4. クリーンアップ
	if (window) {
		CloseWindow(window);
	}

	return 0;
}