/* ds1_tw.c FM TOWNS OpenWatcom */

#include <i86.h>

#include "mode.h"

#ifndef DEBUG2
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#endif

#include "inkey.h"
#include "ds1_98.h"
#include "font98.h"

#define A_KEY "Z"
#define B_KEY "X"

void poke(int a, int b, char c) {
	*((char __far *)MK_FP(a,b)) = c;
}

char peek(int a, int b) {
	return *((char __far *)MK_FP(a,b));
}
#ifdef DEBUG2
extern void set_int41(void);
extern char key_matrix[128];

void outp(int adr, char data)
{
_asm{
	xchg ax,dx
	out	dx,ax
}
}

void outpw(int adr, int data)
{
_asm{
	xchg ax,dx
	out	dx,ax
}
}

int inp(int adr)
{
_asm{
	mov	dx,ax
	in	ax,dx
}
}

void _disable(void)
{
_asm{
	cli
}
}

void _enable(void)
{
_asm{
	sti
}
}
//#else
#endif

void outpm(int offset, int adr, char data)
{
	unsigned char __far *p = MK_FP(offset, adr);
	*p = data;
}
/*void outpm(int offset, int adr, int data)
{
	outp(adr, data);
}
*/

void put_str(char x, char y, char *str)
{
	int i = 0;
	while(str[i] != '\0'){
		++i;
	}
}

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

unsigned char mapdata[BUFFSIZE];

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
/*unsigned char org_pal[MAXCOLOR][3] = {
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

/*疑似BG処理と基本的な画面操作各種*/

void put_8(unsigned short ,unsigned short ,unsigned short);
void g_init(unsigned short);
void end(void);
void clear(unsigned short);
void pal_set(unsigned char, unsigned short,unsigned char,unsigned char,unsigned char);
void pal_all(unsigned char, unsigned char[MAXCOLOR][3]);

void screen_switch(unsigned short);
void cursor_switch(unsigned short);
void v_sync(void);

void put_8(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j, c;
	unsigned short *p0 = (unsigned short *)ds1_grp;
	unsigned char __far *w;

	for (j = 0; j < 16; j++){
		c = x + y * 16 * 80 + j * 80;
/*		*((unsigned short __far *)(unsigned char __far *)(bvram + c))
			= p0[no + j * 16 + 1024*0];
		*((unsigned short __far *)(unsigned char __far *)(rvram + c))
			= p0[no + j * 16 + 1024*1];
		*((unsigned short __far *)(unsigned char __far *)(gvram + c))
			= p0[no + j * 16 + 1024*2];
		*((unsigned short __far *)(unsigned char __far *)(ivram + c))
			= 0;
*/
		w = MK_FP(0xc000,c);
		outpm(0xc000, 0x0ff81, 1);
		*((unsigned short __far *)(unsigned char __far *)(w))
			 = p0[no + j * 16 + 1024*0];
		outpm(0xc000, 0xff81, 2);
		*((unsigned short __far *)(unsigned char __far *)(w))
			 = p0[no + j * 16 + 1024*1];
		outpm(0xc000, 0xff81, 4);
		*((unsigned short __far *)(unsigned char __far *)(w))
			 = p0[no + j * 16 + 1024*2];
		outpm(0xc000, 0xff81, 8);
		*((unsigned short __far *)(unsigned char __far *)(w))
			 = p0[no + j * 16 + 1024*0] | p0[no + j * 16 + 1024*1] | p0[no + j * 16 + 1024*2];
	}
}

void put_8_font(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j, c;
	unsigned short *p0 = (unsigned short *)font_grp;
	unsigned char __far *w;

	for (j = 0; j < 16; j++){
		c = x + y * 16 * 80 + j * 80;
/*		*((unsigned short __far *)(unsigned char __far *)(bvram + c))
			= p0[no + j * 16 + 1024*0];
		*((unsigned short __far *)(unsigned char __far *)(rvram + c))
			= p0[no + j * 16 + 1024*1];
		*((unsigned short __far *)(unsigned char __far *)(gvram + c))
			= p0[no + j * 16 + 1024*2];
		*((unsigned short __far *)(unsigned char __far *)(ivram + c))
			= 0;
*/
		w = MK_FP(0xc000,c);
		outpm(0xc000, 0xff81, 1);
		*((unsigned short __far *)(unsigned char __far *)(w))
			 = p0[no + j * 16 + 1024*0];
		outpm(0xc000, 0xff81, 2);
		*((unsigned short __far *)(unsigned char __far *)(w))
			 = p0[no + j * 16 + 1024*1];
		outpm(0xc000, 0xff81, 4);
		*((unsigned short __far *)(unsigned char __far *)(w))
			 = p0[no + j * 16 + 1024*2];
		outpm(0xc000, 0xff81, 8);
		*((unsigned short __far *)(unsigned char __far *)(w))
			 = p0[no + j * 16 + 1024*0] | p0[no + j * 16 + 1024*1] | p0[no + j * 16 + 1024*2];
	}
}


/*終了処理*/
void end()
{
	cursor_switch(ON);
}

/*カーソル及びファンクションキー表示の制御*/
void cursor_switch(unsigned short mode)
{
#ifndef DEBUG2
/*	if(mode)
		printf("\x1b[>1l\x1b[>5l");
	else
		printf("\x1b*\x1b[>1h\x1b[>5h");*/
#endif
}


/*テキスト画面及びグラフィック画面の消去*/
void clear(unsigned short type)
{
	unsigned short i;
	unsigned char __far *w;

	if(type & 1){
		for (i = 0; i < 80 * 400; i++){
			w = MK_FP(0xc000,i);

			outpm(0xc000, 0xff81, 0x0f);
			*((unsigned short __far *)(unsigned char __far *)(w))
				 = 0;
		}
	}
#ifndef DEBUG2
//	if(type & 2)
//		printf("\x1b*");
#endif
}

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	int palram;
/*	switch(pal_no){
		case CHRPAL_NO:
//			_Far unsigned short *palram;

//			_FP_SEG(palram) = 0x130;
//			_FP_OFF(palram) = 0x2000;
			palram = 0x2000;

			green = ((green + 1)*2-1)*(green!=0);
			blue = ((blue + 1)*2-1)*(blue!=0);
			red = ((red + 1)*2-1)*(red!=0);

//			palram[color] = green * 32 * 32 + red * 32 + blue;
			_poke_word(0x130, palram + color * 2,  green * 32 * 32 + red * 32 + blue);
			break;

		case BGPAL_NO:
*/			green = ((green + 1)*16-1)*(green!=0);
			blue = ((blue + 1)*16-1)*(blue!=0);
			red = ((red + 1)*16-1)*(red!=0);

			outp(0x448,0x01);
			outp(0x44a,0x01);	/* priority register */

			outp(0xfd90, color);
			outp(0xfd92, blue);
			outp(0xfd94, red);
			outp(0xfd96, green);
/*			break;

		case REVPAL_NO:
//			_Far unsigned short *palram;

//			_FP_SEG(palram) = 0x130;
//			_FP_OFF(palram) = 0x2000;
			palram = 0x2000+32;

			green = ((green + 1)*2-1)*(green!=0);
			blue = ((blue + 1)*2-1)*(blue!=0);
			red = ((red + 1)*2-1)*(red!=0);

//			palram[color] = green * 32 * 32 + red * 32 + blue;
			_poke_word(0x130, palram + color * 2,  green * 32 * 32 + red * 32 + blue);
			break;
	}*/
}

/*垂直同期待ち*/
void wait_vsync(void)
{
//	while(!vsync_flag);
//	vsync_flag = 0;
	/* VSYNC(=1)待ち */
	do{
		outp(0x440, 30);
	}while((inp(0x0443) & 0x04)); /* 動作中 */
	do{
		outp(0x440, 30);
	}while(!(inp(0x0443) & 0x04)); /* 動作中 */
#ifndef DEBUG2
_asm{
	mov	ah,06h
	mov	al,00h	;key buffer clear
	int	90h
}
#endif
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
			else if(value < 0)
				pal[k] = org_pal[j][k] * (15 + value) / 15;
			else
				pal[k] = org_pal[j][k];
			if(pal[k] < 0)
				pal[k] = 0;
			else if(pal[k] > 15)
				pal[k] = 15;
		}
		pal_set(pal_no, j, pal[0], pal[1], pal[2]);
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

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short i;
	wait_vsync();
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0], color[i][1], color[i][2]);
}



void key_wait(void);
unsigned char key_scan(unsigned short);
void key_flash(void);

//union REGS reg;
//union REGS reg_out;


/*キー待ち*/
void key_wait(void)
{
#ifndef DEBUG2
	while(!kbhit());
#endif
}


/*キーバッファ・クリア == 残っているキー入力を読み捨てる*/
void key_flash(void)
{
#ifndef DEBUG2
	while(kbhit())
		getch();
#endif
}

unsigned char fadeflag = 0;

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
	unsigned char __far *vram = (unsigned char far *)MK_FP(0xa000,0000L);

	if((x < 0) || (y < 0))
		return;
}

void put_chr8(int x, int y, int chr, char atr)
{
	if((x < 0) || (y < 0))
		return;;

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
	unsigned char __far *vram = (unsigned char far *)MK_FP(0xa000,0000L);

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

// vsync
void vsync(void) {
	wait_vsync();
}

int i;

void wait(int j) {
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



unsigned char keycode = 0;

unsigned char count = 0;
char str[3];

#define PARA_RINT 0
#define PARA_DSWD 1
#define PARA_ESWD 2
#define PARA_FSWD 3
#define PARA_GSWD 4
#define PARA_EAXDWD 5
#define PARA_EDXDWD 7
#define PARA_END 9

/* 80386の仕様でリアルモードでは変数がアドレス先頭の1MBにないとこける */
/* 本来なら共有変数を作るが煩雑になるのでとりあえず */
unsigned short PARABLK[PARA_END];

unsigned char matrix[16];

void KYB_matrix(void)
{
#ifndef DEBUG2
__asm{
	mov	ax,ds
	mov	es,ax

;	lea	ebx,matrix
;	mov	ecx,8

	mov	ax,OFFSET matrix
	mov	di,ax

	mov	ax,SEG matrix
	mov ds,ax

	mov	ah,0ah

	int	90h

	jc	error

error:
}
#else
	int i;
	_disable();
	for(i = 0; i < 16; i++){
//		str[0] = key_matrix[i] + 0x30;
//		str[1] = '\0';
//		print_at(PRINT_MUL * 24, 12 + i, str);

		matrix[i] = key_matrix[i];
	}
	_enable();
#endif
}

#ifdef DEBUG
void KYB_matrix(void)
{
	PARABLK[PARA_RINT] = 0x90;
	PARABLK[PARA_EAXDWD] = 0x0a * 256;

__asm{
	mov	ax,ds
	mov	es,ax

	mov	ax,0250fh
	lea	ebx,matrix
	mov	ecx,8

	cli		/* リアルモードは割り込み禁止にしてみている */
	int	21h
	sti

	jc	error

	mov	di,cx

;	xchg	cl,ch
	ror	ecx,16
;	ror	cx

;	mov	DSWD,cx

	mov	PARABLK+PARA_DSWD*2,cx

;	mov	ah,0ah; 54h
;	mov	al,DEVNO
;	mov	ch,00h
;	mov	cl,00h

;	mov	EAXDWD,eax
;	mov	bx,90h; 93h
;	mov	RINT,bx

	mov	ax,2511h
	lea	edx,PARABLK;RINT
	cli		/* 同上 */
	int	21h
	sti
error:

}
}
#endif

unsigned char keyscan(void)
{
	unsigned char st, pd;
	unsigned char k5=0, k6=0, k7=0, k8=0, k9=0, ka=0;
	unsigned char paddata;
//	static char matrix[16];
	unsigned char keycode = 0;

	_disable();
	KYB_matrix();
	_enable();

	k5 = matrix[5];
	k6 = matrix[6];
	k7 = matrix[7];
	k8 = matrix[8];
	k9 = matrix[9];
	ka = matrix[0xa];

	paddata = inp(0x4d0 + 0 * 2); 
	st = (paddata & 0x0f);
	pd = (paddata >> 4) & 0x03;

	if((k5 & 0x04) || (k6 & 0x20) || !(pd & 0x01)) /* Z,SPACE */
		keycode |= KEY_A;
	if((k5 & 0x08) || !(pd & 0x02)) /* X */
		keycode |= KEY_B;
	if((k7 & 0x08) || (k9 & 0x20) || !(st & 0x01)) /* 8 */
		keycode |= KEY_UP1;
	if((k8 & 0x08) || (ka & 0x01) || !(st & 0x02)) /* 2 */
		keycode |= KEY_DOWN1;

	if(!(st & 0x0c)){ /* RL */
		keycode |= KEY_START;
	}else{
		if((k7 & 0x40) || (k9 & 0x80) || !(st & 0x04)) /* 4 */
			keycode |= KEY_LEFT1;
		if((k8 & 0x01) || (ka & 0x02) || !(st & 0x08)) /* 6 */
			keycode |= KEY_RIGHT1;
	}

	return keycode;
}

#include "common.h"

/*メインルーチン
　初期設定とメインループ*/
#ifndef DEBUG2
void main(void)
#else
void main_c(void)
#endif
{
/*
__asm{
	mov	ax,0b000h
	mov	es,ax
	cli
	hlt
}
*/
#ifdef DEBUG2
	set_int41();
#endif

	pal_all(0, org_pal);
//	set_constrast(0, org_pal, 0);

	outp(0x448,0);
	outp(0x44a,0x11);	// layer 0 DISPLAY ON, layer 1 DISPLAY OFF

	cls();

#ifdef DEBUG2
	do{
#endif
		main2();

#ifdef DEBUG2
	}while(1);
#endif
//	fadeoutblack(org_pal, 0, 3);

//	term_v_sync();

	cls();

	outp(0x448,0);
	outp(0x44a,0x15);	// layer 0 DISPLAY ON, layer 1 DISPLAY ON

	key_flash();

	end();
}
