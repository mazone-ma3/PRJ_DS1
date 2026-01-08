/* ds1_98.c OpenWatcom */
/* VM/UV以降(多分) */
/* キャラを出す */

#include <i86.h>

#define DEBUG2

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
#endif

void put_str(char x, char y, char *str)
{
	unsigned char __far *vram = (unsigned char far *)MK_FP(0xa000,0000L);
	int i = 0;
	while(str[i] != '\0'){
		vram[(i + x + y * 80) * 2] = str[i];
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
	{  0,  0, 15},
	{ 15,  0,  0},
	{ 15,  0, 15},
	{  0, 15,  0},
	{  0, 15, 15},
	{ 15, 15,  0},
	{ 15, 15, 15},
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

void get_8(unsigned short ,unsigned short ,unsigned short);
void put_8(unsigned short ,unsigned short ,unsigned short);
void g_init(unsigned short);
void end(void);
void setpage(unsigned short, unsigned short);
void clear(unsigned short);
void pal_set(unsigned char, unsigned short,unsigned char,unsigned char,unsigned char);
void pal_all(unsigned char, unsigned char[MAXCOLOR][3]);

void screen_switch(unsigned short);
void cursor_switch(unsigned short);
void v_sync(void);

#define X_SIZE 18
#define Y_SIZE 18

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char __far *bvram, // = (unsigned char __far *)MK_FP(0xa800, 0),
	__far *rvram, // = (unsigned char __far *)MK_FP(0xb000, 0),
	__far *gvram, // = (unsigned char __far *)MK_FP(0xb800, 0),
	__far *ivram; // = (unsigned char __far *)MK_FP(0xe000, 0);


#define WIDTH 32
#define LINE 200
#define RAM_ADR 0x2000


unsigned char __far *flame[4]
	 = {MK_FP(0xa800,0)	,MK_FP(0xb000,0),MK_FP(0xb800,0),MK_FP(0xe000,0)};

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};


void put_8(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j, c;
	unsigned short *p0 = (unsigned short *)ds1_grp;

	for (j = 0; j < 16; j++){
		c = x + y * 16 * 80 + j * 80;
		*((unsigned short __far *)(unsigned char __far *)(bvram + c))
			= p0[no + j * 16 + 1024*0];
		*((unsigned short __far *)(unsigned char __far *)(rvram + c))
			= p0[no + j * 16 + 1024*1];
		*((unsigned short __far *)(unsigned char __far *)(gvram + c))
			= p0[no + j * 16 + 1024*2];
		*((unsigned short __far *)(unsigned char __far *)(ivram + c))
			= 0;
	}
}

void put_8_font(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j, c;
	unsigned short *p0 = (unsigned short *)font_grp;

	for (j = 0; j < 16; j++){
		c = x + y * 16 * 80 + j * 80;
		*((unsigned short __far *)(unsigned char __far *)(bvram + c))
			= p0[no + j * 16 + 1024*0];
		*((unsigned short __far *)(unsigned char __far *)(rvram + c))
			= p0[no + j * 16 + 1024*1];
		*((unsigned short __far *)(unsigned char __far *)(gvram + c))
			= p0[no + j * 16 + 1024*2];
		*((unsigned short __far *)(unsigned char __far *)(ivram + c))
			= 0;
	}
}

/*カーソルのＯＦＦ、ＶＲＡＭアドレスの設定*/
void g_init(unsigned short mode_f)
{
/*unsigned short g_driver, g_mode;
	g_driver = DETECT;
	g_mode = 1;
	initgraph(&g_driver, &g_mode ,"");
*/
	_disable();

	outp(0x6a, 1   ); /* 16色モード (0x6a=mode f/fp2)*/

/*	outp(0x6a, 0x07);  拡張モード変更可能
	outp(0x6a, 0x84);  gdc2.5mhz */

	outp(0xa2, 0x4b); /* GDC CSRFORMコマンド */
	outp(0xa0, (mode_f == 1)); /* L/R = 1 (縦方向の拡大係数)*/

	outp(0x68, 8   ); /* モードF/F1 (8で高解像度)*/

	outpw(0x4a0,0xfff0);
	outpw(0x7c, 0);

	screen_switch(ON); /* 表示開始 */

	_enable();

	cursor_switch(OFF);
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
	if(mode)
		printf("\x1b[>1l\x1b[>5l");
	else
		printf("\x1b*\x1b[>1h\x1b[>5h");
#endif
}

void screen_switch(unsigned short mode)
{
	if(mode){
		outp(0xa2, 0x0d); /* 表示開始 */
	}else{
		outp(0xa2, 0x0c);
	}
}

/*ページ切り替え*/
void setpage(unsigned short visual, unsigned short active)
{
	outp(0xa4, visual);
	outp(0xa6, active);
}


/*テキスト画面及びグラフィック画面の消去*/
void clear(unsigned short type)
{
	unsigned short i;

	if(type & 1){
		for (i = 0; i < 80 * 400; i++){
			*(bvram++) = 0;
			*(rvram++) = 0;
			*(gvram++) = 0;
			*(ivram++) = 0;
		}
	}
#ifndef DEBUG2
	if(type & 2)
		printf("\x1b*");
#endif

	bvram = ((unsigned char __far *)MK_FP(0xa800, 0));
	rvram = ((unsigned char __far *)MK_FP(0xb000, 0));
	gvram = ((unsigned char __far *)MK_FP(0xb800, 0));
	ivram = ((unsigned char __far *)MK_FP(0xe000, 0));
}

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outpw(0xa8, color);
	outpw(0xaa, green);
	outpw(0xac, red);
	outpw(0xae, blue);
}

/*垂直同期待ち*/
void wait_vsync(void)
{
	while((inp(0x60) & 0x20));
	while(!(inp(0x60) & 0x20));
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


/*キースキャン及びタイマウエイト・ルーチン*/

#define UP     (key_scan(0x7) & 0x04)
#define DOWN   (key_scan(0x7) & 0x20)
#define RIGHT  (key_scan(0x7) & 0x10)
#define LEFT   (key_scan(0x7) & 0x08)
#define C_UP   (key_scan(0x5) & 0x02)
#define C_DOWN (key_scan(0x5) & 0x04)
#define ESC    (key_scan(0x0) & 0x01)
#define SPACE  (key_scan(0x6) & 0x10)
#define TAB    (key_scan(0x1) & 0x80)
#define SHIFT  (key_scan(0xe) & 0x01)
#define F1     (key_scan(0xc) & 0x04)
#define F2     (key_scan(0xc) & 0x08)
#define F3     (key_scan(0xc) & 0x10)
#define F4     (key_scan(0xc) & 0x20)
#define F5     (key_scan(0xc) & 0x40)
#define F6     (key_scan(0xc) & 0x80)
#define F7     (key_scan(0xd) & 0x01)
#define F8     (key_scan(0xd) & 0x02)
#define F9     (key_scan(0xd) & 0x04)
#define F10    (key_scan(0xd) & 0x08)

#define EOIDATA 0x20
#define EOI 0

void __interrupt __far ip_v_sync(void);
void __interrupt __far (*keepvector)(void);
volatile unsigned char __far vs_count;
unsigned short keepport;

void key_wait(void);
unsigned char key_scan(unsigned short);
void key_beep_off(void);
void key_flash(void);

union REGS reg;
union REGS reg_out;

/*キーグループの参照*/
unsigned char key_scan(unsigned short group)
{
#ifndef DEBUG2
	reg.h.ah = 0x04;
	reg.h.al = (unsigned char)group;
	int86(0x18, &reg, &reg);
	return(reg.h.ah);
#else
_asm{
	mov	ah,04h
	int	18h
	mov	al,ah
}
#endif
}
#ifdef DEBUG
/*
void init_v_sync(void)
{
	_disable();
	keepport = inp(2);
	keepvector = _dos_getvect(10);
	_dos_setvect(10, ip_v_sync);

	outp(EOI, EOIDATA);
	outp(2, keepport & 0xfb);

	outp(0x64, 1);
	_enable();
}

void term_v_sync(void)
{
	_disable();
	_dos_setvect(10, keepvector);
	outp(2, keepport);
	vs_count = 0;
	_enable();
}
*/
void __interrupt __far ip_v_sync(void)
{
	++vs_count;
	outp(0x64, 1);	/* VSYNC初期化 */
	outp(EOI, EOIDATA);
}
/*タイマウェイト*/
/*void wait(unsigned short wait)
{
	while(1){
		_disable();
		if(vs_count >= wait)
			break;
		_enable();
	}
	_enable();
}*/
#endif

/*キー待ち*/
void key_wait(void)
{
#ifndef DEBUG2
	while(!kbhit());
#endif
}


/*キーバッファオーバー時のキー音をなくす*/
void key_beep_off(void)
{
	poke(0, 0x0500, (peek(0, 0x0500) | 0x20));
	poke(0, 0x0500, (peek(0, 0x0500) | 0xdf));
}

/*キーバッファ・クリア == 残っているキー入力を読み捨てる*/
void key_flash(void)
{
#ifndef DEBUG2
	while(kbhit())
	getch();
#endif
}

/* ジョイスティック PC-9801プログラマーズBibleより */
#define FM_PORT1	0x0188
#define FM_PORT2	0x018a

#define OPN_IO_A	0x0e
#define OPN_IO_B	0x0f

#define JOY_UP	0x01
#define	JOY_DOWN	0x02
#define JOY_LEFT	0x04
#define JOY_RIGHT	0x08
#define	JOY_TRIG1	0x10
#define	JOYTRIG2	0x20

void	InitJoystick(unsigned short stick);
unsigned char GetJoystick(void);

unsigned char triger(unsigned char joy_status);
unsigned char joy_key(unsigned char joy_status);


void InitJoystick(unsigned short stick)
{
	unsigned char result;
	_disable();
	outp(FM_PORT1, 0x07);
	result = inp(FM_PORT2);
	result &= 0x1f;
	result |= 0x80;
	outp(FM_PORT1, 0x07);
	outp(FM_PORT2, result);

	outp(FM_PORT1,OPN_IO_B);
	if(stick == 2){
		outp(FM_PORT2, 0xcf);
	}else{
		outp(FM_PORT2, 0x8f);
	}
	_enable();
}

unsigned char GetJoystick(void)
{
	unsigned char result;

	_disable();
	outp(FM_PORT1, OPN_IO_A);
	result = inp(FM_PORT2);
	_enable();
	result ^= 0xff;
	result &= 0x3f;
	return result;
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

	vram[(x + y * 80) * 2] = chr;
	vram[(x + y * 80) * 2+1] = 0;
	vram[(x + y * 80) * 2 + 0x2000] = 0xe1;
	vram[(x + y * 80) * 2 + 0x2001] = 0;
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
	unsigned short code = 0, code2 = 0, code3 = 0, code4 = 0;
	unsigned char st;

unsigned char count = 0;
char str[3];

int keyscan(void)
{

	keycode = 0;

	code = key_scan(0x7);
	code2 = key_scan(0x8);
	code3 = key_scan(0x9);
	code4 = key_scan(0x5);
	st = GetJoystick();

/*(	UP     (key_scan(0x7) & 0x04)
	DOWN   (key_scan(0x7) & 0x20)
	RIGHT  (key_scan(0x7) & 0x10)
	LEFT   (key_scan(0x7) & 0x08)	)*/

	if ((code & 0x04) || (code2 & 0x08) || (st & 0x01)){	/* 8 */
		keycode |= KEY_UP1;
	}
	if ((code & 0x10) || (code3 & 0x01) || (st & 0x08)){	/* 6 */
		keycode |= KEY_RIGHT1;
	}
	if ((code & 0x20) || (code3 & 0x08) || (st & 0x02)){	/* 2 */
		keycode |= KEY_DOWN1;
	}
	if ((code & 0x08) || (code2 & 0x40) || (st & 0x04)){	/* 4 */
		keycode |= KEY_LEFT1;
	}
	if (ESC){
		keycode |= KEY_B;
	}
	if ((st & 0x10) || (code4 & 0x02) || (SPACE)){	/* Z */
		keycode |= KEY_A;
	}
	if((st & 0x020) || (code4 & 0x04)){	/* X */
		keycode |= KEY_B;
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
	unsigned short mode = 1;
	unsigned char sub_flag;

	int j;
	bvram = ((unsigned char __far *)MK_FP(0xa800, 0));
	rvram = ((unsigned char __far *)MK_FP(0xb000, 0));
	gvram = ((unsigned char __far *)MK_FP(0xb800, 0));
	ivram = ((unsigned char __far *)MK_FP(0xe000, 0));

	key_beep_off();

	screen_switch(OFF);
	g_init(0);
	pal_all(0, org_pal);
//	set_constrast(0, org_pal, 0);
	setpage(1,1);
//	screen_switch(ON);
	setpage( 0, 0);

	InitJoystick(1);

	outp(0x62,0x0c);	/**Text OFF */
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

	outp(0x62,0x0d);	/**Text ON */

	end();
}
