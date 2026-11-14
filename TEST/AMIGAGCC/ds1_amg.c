/* ds1_amg.c By m@3 with Grok and Gemini 2026. */
/* インターリブ版 */

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

#include <graphics/gfxmacros.h>

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
	long i, j, count2;
	int k = 0, l = 0, m = 0;

	unsigned short amgcolor[4];
	unsigned char msxcolor[8];
	unsigned char color;
	ULONG vram_adr;

	// 1行あたりのバイト数（320 / 8 = 40バイト）
	int row_bytes = SCREEN_WIDTH / 8;

	if (!(stream[0] = Open(loadfil, MODE_READWRITE))) {
		Close(stream[0]);
		return 1;
	}
	Read(stream[0], pattern, 1);	/* MSX先頭読み捨てる */
	Read(stream[0], pattern, 4);	/* ヘッダ読み捨てる */
	Read(stream[0], pattern, 2);	/* ヘッダ読み捨てる */

	Read(stream[0], read_pattern, MSXWIDTH * msxline);
	m = 0;
	
	for(count2 = 0; count2 < MSXWIDTH * msxline / 4 / 2; ++count2){
		
		/* 色分解 */
		msxcolor[0] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[1] = read_pattern[m++] & 0x0f;
		msxcolor[2] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[3] = read_pattern[m++] & 0x0f;
		msxcolor[4] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[5] = read_pattern[m++] & 0x0f;
		msxcolor[6] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[7] = read_pattern[m++] & 0x0f;

		for(i = 0; i < 4; ++i){
			amgcolor[i] = 0;
		}
		for(j = 0; j < 8; ++j){
			for(i = 0; i < 4; ++i){
				color = conv_tbl[msxcolor[j]];
				if(BITTST(i, color)){
					BITSET2(7-j, amgcolor[i]);
				}else{
					BITCLR2(7-j, amgcolor[i]);
				}
			}
		}

		// ★ラインインターリーブ用の正確なVRAMアドレス計算
		// l: 縦のラインオフセット（行が進むごとに SCREEN_WIDTH/8 ずつ増える）
		// その行における 4面分の160バイトブロックの先頭位置を算出
		ULONG line_block_base = bitplane_address + (l * BITPLANE_NUM) + ((x + y * row_bytes) * BITPLANE_NUM);
		
		vram_adr = line_block_base + k;

		// 各プレーンを「40バイト（row_bytes）ずつ離れたストリップ」に書き込む！
		*((unsigned char *)(vram_adr + row_bytes * 0)) = amgcolor[0]; // プレーン0 (+0)
		*((unsigned char *)(vram_adr + row_bytes * 1)) = amgcolor[1]; // プレーン1 (+40)
		*((unsigned char *)(vram_adr + row_bytes * 2)) = amgcolor[2]; // プレーン2 (+80)
		*((unsigned char *)(vram_adr + row_bytes * 3)) = amgcolor[3]; // プレーン3 (+120)

		k += 1;
		if(k >= (MSXWIDTH / 8)){
			k = 0;
			// 次の行へ進む（1行分の標準幅を進める）
			l += row_bytes;
		}
	}
	Close(stream[0]);

	return 0;
}

short sc5_load__(char *loadfil, short x, short y, short msxline, ULONG bitplane_address, ULONG total_size)
{
	long i, j, count2;
	int k = 0, l = 0, m = 0;

	unsigned short amgcolor[4];
	unsigned char msxcolor[8];
	unsigned char color;
	ULONG vram_adr;

	if (!(stream[0] = Open(loadfil, MODE_READWRITE))) {
		Close(stream[0]);
		return 1;
	}
	Read(stream[0], pattern, 1);	/* MSX先頭読み捨てる */
	Read(stream[0], pattern, 4);	/* ヘッダ読み捨てる */
	Read(stream[0], pattern, 2);	/* ヘッダ読み捨てる */

	Read(stream[0], read_pattern, MSXWIDTH * msxline);
	m = 0;
	
	// 1行あたりのプレーン数等を考慮した幅（バイト単位）
	int row_width_bytes = MSXWIDTH / 8; // 32バイト

	for(count2 = 0; count2 < MSXWIDTH * msxline / 4 / 2; ++count2){
		
		/* 色分解（MSXフォーマットから4プレーン分のデータを抽出） */
		msxcolor[0] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[1] = read_pattern[m++] & 0x0f;
		msxcolor[2] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[3] = read_pattern[m++] & 0x0f;
		msxcolor[4] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[5] = read_pattern[m++] & 0x0f;
		msxcolor[6] = (read_pattern[m] >> 4) & 0x0f;
		msxcolor[7] = read_pattern[m++] & 0x0f;

		for(i = 0; i < 4; ++i){
			amgcolor[i] = 0;
		}
		for(j = 0; j < 8; ++j){
			for(i = 0; i < 4; ++i){
				color = conv_tbl[msxcolor[j]];
				if(BITTST(i, color)){
					BITSET2(7-j, amgcolor[i]);
				}else{
					BITCLR2(7-j, amgcolor[i]);
				}
			}
		}

		// ★ここがインターブリーフ化のキモ！
		// プレーンごとに別アドレスに書くのではなく、
		// 「[P0][P1][P2][P3]` の順にメモリ上で連続して配置する
		// アドレス計算: 基底アドレス + 行のオフセット(l) + 横位置(k * 4)
		vram_adr = bitplane_address + l + (k * BITPLANE_NUM);

		// 4プレーン分を連続したバイトとして書き込む
		*((unsigned char *)(vram_adr + 0)) = (unsigned char)amgcolor[0];
		*((unsigned char *)(vram_adr + 1)) = (unsigned char)amgcolor[1];
		*((unsigned char *)(vram_adr + 2)) = (unsigned char)amgcolor[2];
		*((unsigned char *)(vram_adr + 3)) = (unsigned char)amgcolor[3];

		k += 1;
		if(k >= row_width_bytes){
			k = 0;
			// 1行進むごとに、全プレーン分の幅（32バイト × 4 = 128バイト）を進める
			l += (row_width_bytes * BITPLANE_NUM);
		}
	}
	Close(stream[0]);

	return 0;
}


short sc5_load_(char *loadfil, short x, short y, short msxline, ULONG bitplane_address, ULONG bitplane_size)
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
__chip unsigned char font_buffer[BITPLANE_NUM][8*4][SCREEN_WIDTH / 8];
__chip unsigned char bg_buffer[BITPLANE_NUM][16*3][SCREEN_WIDTH / 8];

char chr;
unsigned char str_temp[11];
#define SCREEN2 0
#define CHRPAL_NO 0

APTR base_address;
APTR sprite_address;
APTR bitplane_address;


void print_at(int x, int y, char *str) {
	// 1行あたりの全プレーン分のバイト数 (320 / 8 * 4 = 160バイト)
	int row_bytes = (SCREEN_WIDTH / 8) * BITPLANE_NUM; 

	// 表示位置の調整（4プレーン分のインターブリーフを考慮）
	x = (x + (32 / 8));
	y = y * 8 * row_bytes;

	WaitBlit();

	while (1) {
		char chr = *(str++);
		if (chr == '\0')
			break;

		if (chr >= 'a')
			chr -= ('a' - 'A');

		if (chr < 0x30)
			chr = 0x40;

		chr -= '0';

		int k = chr & 0x0f;
		int l = (chr / 16) * 8 * row_bytes;

		// 8ライン分の文字を描画
		for (int j = 0; j < 8; ++j) {
			// インターブリーフVRAMの行アドレスを計算
			ULONG vram_adr = (ULONG)bitplane_address + y + j * row_bytes + x;

			// 4プレーン分のフォントデータをCPUで順番に書き込む
			for (int i = 0; i < BITPLANE_NUM; ++i) {
				// font_buffer もインターブリーフ化されている、またはプレーン別の場合は `font_buffer[i]` から取得
				*((unsigned char *)(vram_adr + i * (SCREEN_WIDTH / 8) )) =
					*(&font_buffer[0][0][0] + i * (SCREEN_WIDTH / 8) + k + l + j * row_bytes);
			  }
		}

		// 次の文字へ（4プレーン分進む）
		x++;
	}
}


void print_at_2(int x, int y, char *str) {
	// 1行あたりの全プレーン分のバイト数 (320 / 8 * 4 = 160バイト)
	int row_bytes = (SCREEN_WIDTH / 8) * BITPLANE_NUM; 

	// 表示位置の調整（4プレーン分のインターブリーフを考慮）
	x = (x + (32 / 8));
	y = y * 8 * row_bytes;

	WaitBlit();

	while (1) {
		char chr = *(str++);
		if (chr == '\0')
			break;

		if (chr < 0x30)
			chr = 0x40;

		chr -= '0';

		int k = chr & 0x0f;
		int l = (chr / 16) * 8 * row_bytes;

		// 8ライン分の文字を描画
		for (int j = 0; j < 8; ++j) {
			// インターブリーフVRAMの行アドレスを計算
			ULONG vram_adr = (ULONG)bitplane_address + y + j * row_bytes + x;

			// 4プレーン分のフォントデータをCPUで順番に書き込む
			for (int i = 0; i < BITPLANE_NUM; ++i) {
				// font_buffer もインターブリーフ化されている、またはプレーン別の場合は `font_buffer[i]` から取得
				*((unsigned char *)(vram_adr + i * (SCREEN_WIDTH / 8) )) =
					*(&font_buffer[0][0][0] + i * (SCREEN_WIDTH / 8) + k + l + j * row_bytes);
			  }
		}

		// 次の文字へ（4プレーン分進む）
		x++;
	}
}


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
	ULONG row_bytes = SCREEN_WIDTH / 8; // 1行あたりのバイト数 (320 / 8 = 40バイト)

	// インターブリーフ用：各プレーンの開始位置は「1行分（40バイト）」ずつズレる
	COPPERL[(BITPLANE1 + 0) * 4 + 1] = ((bitplane_address >> 16) & 0xffff);
	COPPERL[(BITPLANE1 + 0) * 4 + 3] = (bitplane_address & 0xffff);

	COPPERL[(BITPLANE1 + 1) * 4 + 1] = (((bitplane_address + row_bytes) >> 16) & 0xffff);
	COPPERL[(BITPLANE1 + 1) * 4 + 3] = ((bitplane_address + row_bytes) & 0xffff);

	COPPERL[(BITPLANE1 + 2) * 4 + 1] = (((bitplane_address + row_bytes * 2) >> 16) & 0xffff);
	COPPERL[(BITPLANE1 + 2) * 4 + 3] = ((bitplane_address + row_bytes * 2) & 0xffff);

	COPPERL[(BITPLANE1 + 3) * 4 + 1] = (((bitplane_address + row_bytes * 3) >> 16) & 0xffff);
	COPPERL[(BITPLANE1 + 3) * 4 + 3] = ((bitplane_address + row_bytes * 3) & 0xffff);

	// スプライトのポインタ設定（ここは従来通り）
	for(int i = 0 ; i < SPRITE_NUM; ++i){
		COPPERL[(SPR0 + i) * 4 + 1] = ((sprite_address >> 16) & 0xffff);
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

void cls_blitter(void) {
	// 1. 進行中のBlitter処理が完了するのを待つ
	WaitBlit();

	// 2. 塗りつぶし（ゼロクリア）用の制御レジスタ設定
	// BLTCON0: Aソース無効、デスティネーション有効、ミンターム 0 (すべてゼロ)
	Custom.bltcon0 = 0x0100; // デスティネーションのみ、ミンターム0 (ゼロフィル)
	Custom.bltcon1 = 0x0000;

	// すべてのプレーンにわたる連続的なブロック書き込みのためモジュロは0
	Custom.bltamod = 0;
	Custom.bltdmod = 0;

	// 3. デスティネーションポインタをビットプレーンの先頭に設定
	unsigned char *dest_addr = (unsigned char *)bitplane_address;
	Custom.bltdpt = (APTR)(dest_addr);

	// 4. BLTSIZE: 高さ（行数）と幅（1行あたりの16ビットワード数）
	// 320x200画面、4ビットプレーンの場合:
	// 総高さ = 200ライン * 4ビットプレーン = 800行
	// 幅 = 320ピクセル / 16 = 20ワード (1行あたり40バイト)
//	Custom.bltsize = ((SCREEN_HEIGHT * BITPLANE_NUM) & 0x3FF) | ((SCREEN_WIDTH / 16) << 10);
	Custom.bltsize = ((SCREEN_HEIGHT * BITPLANE_NUM) << 6) | ((SCREEN_WIDTH / 16));
}
/*
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
}*/

void cls(void) {
//	int i,j;

	cls_blitter();

//	cls2();
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

void put_chr16_(short x, short y, char chr)
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
					(bg_buffer[i][j][chr*2+k]);// + ( j*SCREEN_WIDTH/8+16*3*0) + (chr*2+k));
			}
		}
	}
}

void put_chr16__(short x, short y, char chr) {
	// 画面上のバイト単位のX座標、Yライン位置
	int dest_x = (x + (32 / 16)) * 2;
	int dest_y = y * 16;

	for (int i = 0; i < BITPLANE_NUM; ++i) {
		WaitBlit();

		// ソース: bg_buffer のプレーンiにおける、0行目の「chr番目のキャラクターのX位置」を指す
		unsigned char *src = &bg_buffer[i][0][chr * 2];

		// デスティネーション: VRAMの該当プレーン位置
		unsigned char *dest = (unsigned char *)bitplane_address 
								+ (BITPLANE_SIZE * i) 
								+ dest_x + dest_y * (SCREEN_WIDTH / 8);

		// マスク設定
		Custom.bltafwm = 0xFFFF; // First Word Mask
		Custom.bltalwm = 0xFFFF; // Last Word Mas

		// レジスタ設定
		Custom.bltcon0 = 0x09f0; //0x0FCA; // A→D コピー (Minterm: ABC)
		Custom.bltcon1 = 0x0000;

		// ★超重要：モジュロ（行末から次の行へジャンプするバイト数）の設定
		// タイルシート全体の1行の幅（40バイト）から、描画する幅（2バイト）を引いた値
		Custom.bltamod = (SCREEN_WIDTH / 8) - 2; // 40 - 2 = 38

		// デスティネーション側の1行の幅からも、描画幅(2バイト)を引く
		Custom.bltdmod = (SCREEN_WIDTH / 8) - 2; // 40 - 2 = 38

		Custom.bltapt = (APTR)src;
		Custom.bltdpt = (APTR)dest;

		// 高さ16ライン、幅1ワード（16ピクセル = 2バイト）
		// (高さ << 6) | 幅ワード数
		Custom.bltsize = (16 << 6) | 1;
	}
}

void put_chr16(short x, short y, char chr) {
	// 1回のBlitで4プレーン分を同時に処理するため、プレーンのforループは消滅します！
	WaitBlit();

	// 画面上のバイト単位のX座標、Yライン位置
	// インターブリーフでは1行あたり全プレーン分のデータ（40バイト × 4 = 160バイト）が並ぶ
	int row_bytes = (SCREEN_WIDTH / 8) * BITPLANE_NUM; // 320/8 * 4 = 160バイト
	
	int dest_x = (x + (32 / 16)) * 2;// * BITPLANE_NUM; // 4プレーン分を考慮したオフセット
	int dest_y = y * 16;// * row_bytes;

	// ソース（タイルバッファもインターブリーフ化されている前提）
	// 1キャラクターの幅は 2バイト × 4プレーン = 8バイト
	unsigned char *src = &bg_buffer[0][0][0] + (chr*2); // * row_bytes); //BITPLANE_NUM);

	// デスティネーション（VRAMの該当位置）
	unsigned char *dest = (unsigned char *)bitplane_address 
						  + dest_x + dest_y * row_bytes;

	// レジスタ設定
	Custom.bltafwm = 0xFFFF;
	Custom.bltalwm = 0xFFFF;
	Custom.bltcon0 = 0x09F0; // A → D コピー
	Custom.bltcon1 = 0x0000;

	// ★超重要：インターブリーフ時のモジュロ設定
	// 画面全体の1行の幅（160バイト）から、描画する幅（8バイト = 4ワード）を引く
	Custom.bltamod = (SCREEN_WIDTH / 8) - (8/8)*2;//(2 * BITPLANE_NUM); // 160 - 8 = 152
	Custom.bltdmod = (SCREEN_WIDTH / 8) - (8/8)*2;//(2 * BITPLANE_NUM); // 160 - 8 = 152

	Custom.bltapt = (APTR)src;
	Custom.bltdpt = (APTR)dest;

	// ★ここが最大の魔法：
	// 高さ 16ライン、幅 4ワード（プレーン0?3の計4ワード分を1行として同時に扱う）
	Custom.bltsize = ((16 * BITPLANE_NUM) << 6) | (8/8);
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
	
	// ★インターブリーフ用モジュロ設定
	// 1行進む間にスキップするバイト数 = (プレーン数 - 1) × 1行のバイト数
	// (4 - 1) * 40 = 120バイト (0x0078)
	Custom.bpl1mod = 0x0078; // 奇数プレーン用モジュロ
	Custom.bpl2mod = 0x0078; // 偶数プレーン用モジュロ

	Custom.bplcon1 = 0x0000;
	Custom.bplcon2 = 0x0024;
	Custom.ddfstrt = 0x0038;
	Custom.ddfstop = 0x00d0;
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

	cls_blitter();

//	sc5_load("ds1_8bit.sc5", 0, 0, 16*3, (ULONG)&bg_buffer[0], (SCREEN_WIDTH / 8 * 16 * 3));

	sc5_load("ds1_8bit.sc5", 0, 0, 16*3, (ULONG)&bg_buffer[0][0][0], (SCREEN_WIDTH / 8 * 16 * 3));
	sc5_load("FONTYOKO.SC5", 0, 0, 8*4, (ULONG)&font_buffer[0][0][0], (SCREEN_WIDTH / 8 * 8 * 4));

//	raw_load("se1.raw", rawbuffer, rawsize);

	keywindowopen();

#ifdef DEBUG2
	for(;;)
#endif
		main2();

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

//#include <stdio.h>

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