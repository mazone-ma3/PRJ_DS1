/* ds1_68e.c X68000 elf2x68k By m@3 */

#include "mode.h"

#ifndef DEBUG2
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <x68k/iocs.h>
#include <x68k/dos.h>
#endif

#include "inkey.h"

#include "font98.h"
#include "ds1_98.h"

#define A_KEY "Z"
#define B_KEY "X"

#ifdef DEBUG2
void _iocs_g_clr_on(void) {
	register long rd0 asm ("d0");
dum0:	rd0 = 0x90;
asm volatile(
	"trap #15\n"
	:			/* 値が返るレジスタ変数 */
	:"d"(rd0)	/* 引数として使われるレジスタ変数 */
);
}

void _iocs_b_curoff(void) {
	register long rd0 asm ("d0");
dum1:	rd0 = 0x1f;
asm volatile(
	"trap #15\n"
	:			/* 値が返るレジスタ変数 */
	:"d"(rd0)	/* 引数として使われるレジスタ変数 */
);
}

void _iocs_b_curon(void) {
	register long rd0 asm ("d0");
dum2:	rd0 = 0x1e;
asm volatile(
	"trap #15\n"
	:			/* 値が返るレジスタ変数 */
	:"d"(rd0)	/* 引数として使われるレジスタ変数 */
);
}

void _iocs_crtmod(int mode) {
	register long rd1 asm ("d1");
	register long rd0 asm ("d0");
dum3:	rd1 = mode;
	rd0 = 0x10;
asm volatile(
	"trap #15\n"
	:			/* 値が返るレジスタ変数 */
	:"d"(rd0),"d"(rd1)	/* 引数として使われるレジスタ変数 */
);
}

void _iocs_b_super(int mode) {
	register long ra1 asm ("a1");
	register long rd0 asm ("d0");
dum4:	ra1 = mode;
	rd0 = 0x81;
asm volatile(
	"trap #15\n"
	:			/* 値が返るレジスタ変数 */
	:"d"(rd0),"d"(ra1)	/* 引数として使われるレジスタ変数 */
);
}

int _iocs_bitsns(int no) {
	register long rd1 asm ("d1");
	register long rd0 asm ("d0");
dum5:	rd1 = no;
	rd0 =0x04;
asm volatile(
	"trap #15\n"
			:"=d"(rd0)	/* 値が返るレジスタ変数 */
	:"d"(rd0),"d"(rd1)	/* 引数として使われるレジスタ変数 */
);
	return rd0;
}

#define  exit(value)
#endif

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

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

/* BITチェック */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT反転 */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}

#define PARTS_HEAD 0x3c00 /*組み合わせキャラデータの先頭番地*/
#define BUFFSIZE 16384

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

//unsigned char mapdata[BUFFSIZE];

#define MAXCOLOR 16

/* R G B */
unsigned char org_pal[MAXCOLOR][3] = {
	{  0,  0,  0},
	{  0,  0,  7},
	{  7,  0,  0},
	{  7,  0,  7},
	{  0,  7,  0},
	{  0,  7,  7},
	{  7,  7,  0},
	{  7,  7,  7},
	{  0,  0,  0},
	{  0,  0, 15},
	{ 15,  0,  0},
	{ 15,  0, 15},
	{  0, 15,  0},
	{  0, 15, 15},
	{ 15, 15,  0},
	{ 15, 15, 15},
};
/*
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
};*/

#define bvram ((unsigned char *)0xe00000)
#define rvram ((unsigned char *)0xe20000)
#define gvram ((unsigned char *)0xe40000)
#define ivram ((unsigned char *)0xe60000)

void put_8(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned int j, c, d;
	unsigned char *p0 = (unsigned char *)ds1_grp;

	for (j = 0; j < 16; j++){
		c = (x + y * 0x80 * 16 + j * 0x80);
		d = (no + j * 16)*2;

		*((unsigned char *)(bvram + c))
			= p0[d + 1024*0];
		*((unsigned char *)(bvram + c + 1))
			= p0[d + 1024*0 + 1];

		*((unsigned char *)(rvram + c))
			= p0[d + 1024 *2];
		*((unsigned char *)(rvram + c + 1))
			= p0[d + 1024 *2 + 1];

		*((unsigned char *)(gvram + c))
			= p0[d + 1024*4];
		*((unsigned char *)(gvram + c + 1))
			= p0[d + 1024*4 + 1];

		*((unsigned char *)(ivram + c))
			= p0[d + 1024*0] | p0[d + 1024*2] | p0[d + 1024*4];
		*((unsigned char *)(ivram + c + 1))
			= p0[d + 1024*0 + 1] | p0[d + 1024*2 + 1] | p0[d + 1024*4 + 1];

	}
}

void put_8_font(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned int j, c, d;
	unsigned char *p0 = (unsigned char *)font_grp;

	for (j = 0; j < 16; j++){
		c = (x + y * 0x80 *16 + j * 0x80);
		d = (no + j * 16)*2;

		*((unsigned char *)(bvram + c))
			= p0[d + 1024*0];
		*((unsigned char *)(bvram + c + 1))
			= p0[d + 1024*0 + 1];

		*((unsigned char *)(rvram + c))
			= p0[d + 1024 *2];
		*((unsigned char *)(rvram + c + 1))
			= p0[d + 1024 *2 + 1];

		*((unsigned char *)(gvram + c))
			= p0[d + 1024*4];
		*((unsigned char *)(gvram + c + 1))
			= p0[d + 1024*4 + 1];

		*((unsigned char *)(ivram + c))
			= p0[d + 1024*0] | p0[d + 1024*2] | p0[d + 1024*4];
		*((unsigned char *)(ivram + c + 1))
			= p0[d + 1024*0 + 1] | p0[d + 1024*2 + 1] | p0[d + 1024*4 + 1];
	}
}


void g_init(void)
{
/*	CRTMOD(0x0a); */ 	/* 256x256 256colors 2plane 31kHz */
//	CRTMOD(0x06);	/* 256x256 16colors 4plane 31kHz */
//	CRTMOD(0x0e);	/* 256x256 65536colors 1plane 31kHz */
//	CRTMOD(0x04);	/* 512x512 16colors 4plane 31kHz */
	_iocs_crtmod(0x10);	/* 768x512 16colors 1plane 31kHz */

	_iocs_g_clr_on();
	_iocs_b_curoff();
}

/*終了処理*/
void end()
{
	_iocs_b_curon();
	_iocs_crtmod(0x10);	/* 768x512 16colors 1plane 31kHz */
}

void paint(unsigned short color)
{
	unsigned short i, j, k;
	unsigned long *p0 = (unsigned long *)bvram;
	unsigned long *p1 = (unsigned long *)rvram;
	unsigned long *p2 = (unsigned long *)gvram;
	unsigned long *p3 = (unsigned long *)ivram;

	for (i = 0; i < 512; ++i){
		for (j = 0; j < (0x80 / 4); ++j){
			k = j + i * (0x80 / 4);
			*(p0 + k) = color; /* bit */;
			*(p1 + k) = color; /* bit */;
			*(p2 + k) = color; /* bit */;
			*(p3 + k) = color; /* bit */;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1){
		paint(0x0);
	}
//	if(type & 2)
//		printf("\x1b*");
}

/*パレット・セット*/
void pal_set(int pal_no, unsigned short color, unsigned short red, unsigned short green,
	unsigned short blue)
{
	unsigned short *pal_port;
	unsigned char mode = 0;
	if(color)
		mode = 1;

//	green = ((green + 1)*2-1)*(green!=0);
//	blue = ((blue + 1)*2-1)*(blue!=0);
//	red = ((red + 1)*2-1)*(red!=0);

	switch(pal_no){
		case 0:
			pal_port = (unsigned short *)(0xe82200); // + color * 2);
			*(pal_port+color) = (green * 32 * 32 + red * 32 + blue) * 2 + mode;
//			pal_port = (unsigned short *)(0xe82220); // + color * 2);
//			*(pal_port+color) = (green * 32 * 32 + red * 32 + blue) * 2 + mode;
			break;
		case 1:
			pal_port = (unsigned short *)(0xe82000); // + color * 2);
			*(pal_port+color) = (green * 32 * 32 + red * 32 + blue) * 2 + mode;
			break;
	}
}
/*垂直同期待ち*/
void wait_vsync(void)
{
	unsigned char volatile *vsync = (unsigned char *)0xe88001;	/* MFP */
	/* VSYNC待ち */
	while(!((*vsync) & 0x10));	/* 調査中 */
	while((*vsync) & 0x10);		/* 調査中 */
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}

//value < 0 黒に近づける。
//value = 0 設定した色
//value > 0 白に近づける。
void set_constrast(int value, unsigned char org_pal[MAXCOLOR][3], int pal_no)
{
	int j, k;
	int pal[3];


	for(j = 0; j < MAXCOLOR; j++){
		for(k = 0; k < 3; k++){
			if(value > 0)
				pal[k] = org_pal[j][k] + value;
#ifndef DEBUG2
			else if(value < 0)
				pal[k] = org_pal[j][k] * (15 + value) / 15;
#endif
			else
				pal[k] = org_pal[j][k];
			if(pal[k] < 0)
				pal[k] = 0;
			else if(pal[k] > 15)
				pal[k] = 15;
		}
//		pal_set(pal_no, j, pal[0], pal[1], pal[2]);
		pal_set(pal_no, j, ((pal[0] + 1)*2-1) * (pal[0] != 0), ((pal[1]+1)*2-1) * (pal[1] != 0), ((pal[2]+1)*2-1) * (pal[2] != 0));
	}
}

//wait値の速度で黒からフェードインする。
void fadeinblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = -15; j <= 0; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait値の速度で黒にフェードアウトする。
void fadeoutblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j != -16; j--){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait値の速度で白にフェードアウトする。
void fadeoutwhite(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j < 16; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//パレットを暗転する。
void pal_allblack(int pal_no)
{
	char j;
	for(j = 0; j < MAXCOLOR; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

void pal_all(unsigned char pal_no, unsigned char pal[MAXCOLOR][3])
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(pal_no, i, ((pal[i][0] + 1)*2-1) * (pal[i][0] != 0), ((pal[i][1]+1)*2-1) * (pal[i][1] != 0), ((pal[i][2]+1)*2-1) * (pal[i][2] != 0));
}


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

#define PRINT_MUL 2

void beep(void)
{
}

void put_chr8_text(int x, int y, char chr, char atr)
{
	if((x < 0) || (y < 0))
		return;
}

void put_chr8(int x, int y, int chr, char atr)
{
	if((x < 0) || (y < 0))
		return;;
	x+=(128/8);

	put_8(x, y, chr);
}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 4 + 0, y * 2 + 0, chr * 2 + 0, 0x27);
	put_chr8(x * 4 + 2, y * 2 + 0, chr * 2 + 1, 0x27);

	put_chr8(x * 4 + 0, y * 2 + 1, chr * 2 + 16*16, 0x27);
	put_chr8(x * 4 + 2, y * 2 + 1, chr * 2 + 16*16+1, 0x27);
}

static unsigned short chr;

// VRAM直書き
void print_at(char x, char y, char *str) {
	x+=(128/8);
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
		if(chr >= 'a')
			chr -= ('a'-'A');
		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		put_8_font(x, y, (chr % 16) + (chr / 16) * 16 * 16);

		x+=2;
	}
}

void print_at_2(char x, char y, char *str) {
	x+=(128/8);
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		put_8_font(x, y, (chr % 16) + (chr / 16) * 16 * 16);

		x+=2;
	}
}

void put_logo(int x, int y)
{
	print_at_2(x, y, "      i  k   ");
	print_at_2(x, y+1, " 2026 bcdefgh");
}

// vsync
void vsync(void) {
	wait_vsync();
}

void wait(int j) {
	int i;
	for (i = 0; i < j; ++i)
		vsync();
}

void cls(void) {
	int i,j;
	clear(3);
	for(j = 0l; j < 24; j++)
		for(i = 0; i < 80; ++i)
			put_chr8_text(i, j, ' ', 0);
}

void define_tiles(void) {
}

void play_sound_effect(void) {
	beep();  // シンプルなビープ音
}

#define reg ((unsigned char *)0xe9a001)

unsigned char keycode;

unsigned char keyscan(void)
{
	unsigned char k5, k6, k7, k8, k9, st, pd;
	unsigned short paddata;
	keycode = 0;

	k5 = _iocs_bitsns(5);
	k6 = _iocs_bitsns(6);
	k7 = _iocs_bitsns(7);
	k8 = _iocs_bitsns(8);
	k9 = _iocs_bitsns(9);

	paddata = reg[0];
	st = (paddata & 0x0f); // ^ 0x0f;
	pd = ((paddata >> 5) & 0x03); // ^ 0x03;

	if((k5 & 0x04) || (k6 & 0x20) || !(pd & 0x01)) /* Z,SPACE */
		keycode |= KEY_A;
	if((k5 & 0x08) || !(pd & 0x02)) /* X */
		keycode |= KEY_B;
	if((k8 & 0x10) || (k7 & 0x10) || !(st & 0x01)) /* 8 */
		keycode |= KEY_UP1;
	if((k9 & 0x10) || (k7 & 0x40) || !(st & 0x02)) /* 2 */
		keycode |= KEY_DOWN1;

	if(!(st & 0x0c)){ /* RL */
		keycode |= KEY_START;
	}else{
		if((k8 & 0x80) || (k7 & 0x08) || !(st & 0x04)) /* 4 */
			keycode |= KEY_LEFT1;
		if((k9 & 0x02) || (k7 & 0x20) || !(st & 0x08)) /* 6 */
			keycode |= KEY_RIGHT1;
	}

	return keycode;
}


#include "common.h"

int	main(int argc,char **argv){
	unsigned char fadeflag = 0;

dum:	_iocs_b_super(0);		/* スーパーバイザモード 最適化防止にラベルを付ける */

	g_init();

	pal_all(0,org_pal);
	clear(3);
//	pal_allblack(0);

#ifdef DEBUG2
	for(;;)
#endif
		main2();

	fadeoutblack(org_pal, 0, 3);

	clear(3);
	end();

#ifndef DEBUG2
	while(_iocs_b_keysns())
		_iocs_b_keyinp();
#endif

	exit(0);

	return 0;
}
