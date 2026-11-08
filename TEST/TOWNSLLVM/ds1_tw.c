/* ds1_tw.c FM TOWNS OpenWatcom */

//#include <i86.h>

#include "mode.h"

#ifndef DEBUG2
#include <stdio.h>
//#include <conio.h>
#include <stdlib.h>
#include <stdint.h>
#endif

#include "inkey.h"
#include "ds1_98.h"
#include "font98.h"

#define A_KEY "Z"
#define B_KEY "X"

int main(void);

void _start(void)
{
	main();
}

static inline  void poke(uint16_t selector, uint32_t offset, uint8_t val) {
	__asm__ volatile (
		"push %%es\n"
		"movw %1, %%es\n"		// ES レジスタにセレクタを設定
		"movb %2, %%es:(%0)\n"	// ES セグメントのオフセットへストア
		"pop %%es\n"
		: : "r"(offset), "r"(selector), "q"(val)
		: "memory"
	);
}

static inline void pokew(short selector, int offset, short val) {
	__asm__ volatile (
		"push %%es\n"
		"movw %1, %%es\n"		// ES レジスタにセレクタを設定
		"movw %2, %%es:(%0)\n"	// ES セグメントのオフセットへストア
		"pop %%es\n"
		: : "r"(offset), "r"(selector), "q"(val)
		: "memory"
	);
}

static inline void pokel(uint16_t selector, uint32_t offset, uint32_t val) {
    __asm__ volatile (
        "push %%es\n"
        "movw %1, %%es\n\t"
        "movl %2, %%es:(%0)\n"
        "pop %%es\n"
        : : "r"(offset), "r"(selector), "r"(val)
        : "memory"
    );
}

//#ifdef DEBUG2
//extern void set_int41(void);
//extern char key_matrix[128];

// ポートへ1バイト出力
static inline void outp(uint16_t port, uint8_t data) {
__asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// ポートから1バイト入力
static inline uint8_t inp(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* --- 2バイト（16ビット）の入出力 --- */

// ポートへ2バイト出力
static inline void outpw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// ポートから2バイト入力
static inline uint16_t inpw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

//#endif

void _disable(void)
{
__asm__ volatile(
	"cli\n"
);
}

void _enable(void)
{
__asm__ volatile(
	"sti\n"
);
}
//#else
//#endif

static inline  void outpm(int offset, int adr, char data)
{
	poke(0x60, offset * 0x10 + adr, data);
}

__attribute((optnone)) void put_str(char x, char y, char *str)
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
const unsigned char org_pal[MAXCOLOR][3] = {
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
void pal_set(unsigned char, unsigned char,unsigned char,unsigned char,unsigned char);
void pal_all(unsigned char, unsigned char[MAXCOLOR][3]);

void screen_switch(unsigned short);
void cursor_switch(unsigned short);
void v_sync(void);

//__attribute((optnone)) 
void put_8(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j;
	unsigned int c;
	unsigned short *p0 = (unsigned short *)ds1_grp;
//	unsigned char __far *w;

	for (j = 0; j < 16; j++){
		c = x + y * 16 * 80 + j * 80 + 0xc0000;
/*		*((unsigned short __far *)(unsigned char __far *)(bvram + c))
			= p0[no + j * 16 + 1024*0];
		*((unsigned short __far *)(unsigned char __far *)(rvram + c))
			= p0[no + j * 16 + 1024*1];
		*((unsigned short __far *)(unsigned char __far *)(gvram + c))
			= p0[no + j * 16 + 1024*2];
		*((unsigned short __far *)(unsigned char __far *)(ivram + c))
			= 0;
*/
//		w = MK_FP(0xc000,c);
		poke(0x60, 0xcff81, 1);
//		*((unsigned short __far *)(unsigned char __far *)(w))
//			 = p0[no + j * 16 + 1024*0];
		pokew(0x60, c, p0[no + j * 16 + 1024*0]);
		poke(0x60, 0xcff81, 2);
//		*((unsigned short __far *)(unsigned char __far *)(w))
//			 = p0[no + j * 16 + 1024*1];
		pokew(0x60, c, p0[no + j * 16 + 1024*1]);
		poke(0x60, 0xcff81, 4);
//		*((unsigned short __far *)(unsigned char __far *)(w))
//			 = p0[no + j * 16 + 1024*2];
		pokew(0x60, c, p0[no + j * 16 + 1024*2]);
		poke(0x60, 0xcff81, 8);
//		*((unsigned short __far *)(unsigned char __far *)(w))
//			 = p0[no + j * 16 + 1024*0] | p0[no + j * 16 + 1024*1] | p0[no + j * 16 + 1024*2];
		pokew(0x60, c, p0[no + j * 16 + 1024*0] | p0[no + j * 16 + 1024*1] | p0[no + j * 16 + 1024*2]);

//		poke(0x120, c*4, p0[no + j * 16 + 1024*0] | p0[no + j * 16 + 1024*1]*2 | p0[no + j * 16 + 1024*2]*4 | (p0[no + j * 16 + 1024*0] | p0[no + j * 16 + 1024*1] | p0[no + j * 16 + 1024*2])*8);
	}
}

void put_8_font(unsigned short x, unsigned short y, unsigned short no)
{
	unsigned short j;
	unsigned int c;
	unsigned short *p0 = (unsigned short *)font_grp;
//	unsigned char __far *w;

	for (j = 0; j < 16; j++){
		c = x + y * 16 * 80 + j * 80 + 0xc0000;
/*		*((unsigned short __far *)(unsigned char __far *)(bvram + c))
			= p0[no + j * 16 + 1024*0];
		*((unsigned short __far *)(unsigned char __far *)(rvram + c))
			= p0[no + j * 16 + 1024*1];
		*((unsigned short __far *)(unsigned char __far *)(gvram + c))
			= p0[no + j * 16 + 1024*2];
		*((unsigned short __far *)(unsigned char __far *)(ivram + c))
			= 0;
*/
//		w = MK_FP(0xc000,c);
		poke(0x60, 0xcff81, 1);
//		*((unsigned short __far *)(unsigned char __far *)(w))
//			 = p0[no + j * 16 + 1024*0];
		pokew(0x60, c, p0[no + j * 16 + 1024*0]);
		poke(0x60, 0xcff81, 2);
//		*((unsigned short __far *)(unsigned char __far *)(w))
//			 = p0[no + j * 16 + 1024*1];
		pokew(0x60, c, p0[no + j * 16 + 1024*1]);
		poke(0x60, 0xcff81, 4);
//		*((unsigned short __far *)(unsigned char __far *)(w))
//			 = p0[no + j * 16 + 1024*2];
		pokew(0x60, c, p0[no + j * 16 + 1024*2]);
		poke(0x60, 0xcff81, 8);
//		*((unsigned short __far *)(unsigned char __far *)(w))
//			 = p0[no + j * 16 + 1024*0] | p0[no + j * 16 + 1024*1] | p0[no + j * 16 + 1024*2];
		pokew(0x60, c, p0[no + j * 16 + 1024*0] | p0[no + j * 16 + 1024*1] | p0[no + j * 16 + 1024*2]);
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
__attribute((noinline)) void clear(unsigned short type)
{
	unsigned short i;
//	unsigned short __far *w;

//	return;

	if(type & 1){
		outpm(0xc000, 0xff81, 0x0f);
/*		for (i = 0; i < 80 * 400/2; i++){
			w = MK_FP(0xc000,i);
			*w = 0;
		}*/
__asm__ volatile(
	"push	%ax\n"
	"push	%ebx\n"
	"push	%es\n"
	"mov	$0x60,%ax\n"
	"mov	%ax,%es\n"
	"mov	$0xc0000,%ebx\n"
"loopclear:\n"
	"movw	$0x0, %es:(%ebx)\n"
//	"mov	word ptr %es:[%bx], 0\n"
	"add	$2,%ebx\n"
	"cmp	$80*400+0xc0000,%ebx\n"
	"jnz	loopclear\n"

	"pop	%es\n"
	"pop	%ebx\n"
	"pop	%ax\n"
);
	}
#ifndef DEBUG2
//	if(type & 2)
//		printf("\x1b*");
#endif
}

/*パレット・セット*/
__attribute((optnone)) void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	int palram;
/*	switch(pal_no){
		case CHRPAL_NO:
//			_Far unsigned short *palram;

//			_FP_SEG(palram) = 0x130;
//			_FP_OFF(palram) = 0x2000;
			palram = 0x2000;
*/
//			green = ((green + 1)*2-1)*(green!=0);
//			blue = ((blue + 1)*2-1)*(blue!=0);
//			red = ((red + 1)*2-1)*(red!=0);

//			palram[color] = green * 32 * 32 + red * 32 + blue;
//			_poke_word(0x130, palram + color * 2,  green * 32 * 32 + red * 32 + blue);
//			break;
/*
		case BGPAL_NO:
*/			green = ((green + 1)*16-1)*(green!=0);
			blue = ((blue + 1)*16-1)*(blue!=0);
			red = ((red + 1)*16-1)*(red!=0);


			outp(0x448,0x01);
			outp(0x44a,0x21);	/* priority register */

			outp( 0x0fd90, color);
			outp( 0x0fd92, blue);
			outp( 0x0fd94, red);
			outp( 0x0fd96, green);


			outp(0x448,0x01);
			outp(0x44a,0x01);	/* priority register */


			outp( 0x0fd90, color);
			outp( 0x0fd92, blue);
			outp( 0x0fd94, red);
			outp( 0x0fd96, green);


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
__asm__ volatile (
	"mov	$06,%ah\n"
	"mov	$0,%al\n"//	;key buffer clear\n"
	"int	$0x90\n"
);

#endif
}

__attribute((optnone)) void sys_wait(unsigned char wait)
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
//	while(!kbhit());
#endif
}


/*キーバッファ・クリア == 残っているキー入力を読み捨てる*/
void key_flash(void)
{
#ifndef DEBUG2
//	while(kbhit())
//		getch();
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
//	unsigned char __far *vram = (unsigned char far *)MK_FP(0xa000,0000L);
	if((x < 0) || (y < 0))
		return;
}

__attribute((optnone)) void put_chr8(int x, int y, int chr, char atr)
{
	if((x < 0) || (y < 0))
		return;;
	x+=(32/8)*2;

	put_8(x, y, chr);
}

__attribute((optnone)) void put_chr16(int x, int y, char chr) {
	put_chr8(x * 4 + 0, y * 2 + 0, chr * 2 + 0, 0x27);
	put_chr8(x * 4 + 2, y * 2 + 0, chr * 2 + 1, 0x27);

	put_chr8(x * 4 + 0, y * 2 + 1, chr * 2 + 16*16, 0x27);
	put_chr8(x * 4 + 2, y * 2 + 1, chr * 2 + 16*16+1, 0x27);
}

static unsigned short chr;

// VRAM直書き
__attribute((optnone)) void print_at(char x, char y, char *str) {
//	unsigned char __far *vram = (unsigned char far *)MK_FP(0xa000,0000L);
	x+=(32/8)*2;

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

__attribute((optnone)) void print_at_2(char x, char y, char *str) {
//	unsigned char __far *vram = (unsigned char far *)MK_FP(0xa000,0000L);
	x+=(32/8)*2;

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

unsigned char matrix[16] = {0x0};

__attribute((optnone)) void KYB_matrix(void)
{
#ifndef DEBUG2
__asm__ volatile(
	"mov	%ds,%ax\n"
	"push	%es\n"
	"mov	%ax,%es\n"

//;	"lea	ebx,matrix\n"
//;	"mov	ecx,8\n"

	"movw	$matrix, %ax\n"
	"mov	%ax,%di\n"

//	"movw	$matrix, %ax\n"
	"movw	$0x14,%ax\n"
	"mov 	%ax,%ds\n"

	"mov	$0x0a,%ah\n"

	"int	$0x90\n"
	"pop	%es\n"

//	"jc	_error\n"
);
//error:
}
#else
	int i;
	_disable();
/*	for(i = 0; i < 8; i++){
		str[0] = key_matrix[i] + 0x30;
		str[1] = '\0';
		print_at(PRINT_MUL * 24, 12 + i, str);
	}*/
	for(i = 0; i < 16; i++){
		matrix[i] = key_matrix[i];
	}
	_enable();
}
#endif

#ifdef DEBUG
void KYB_matrix(void)
{
	PARABLK[PARA_RINT] = 0x90;
	PARABLK[PARA_EAXDWD] = 0x0a * 256;

__asm__ volatile(
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
);
}
}
#endif

__attribute((optnone)) unsigned char keyscan(void)
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


static uint32_t color_table[16] = {
	0x0ffffffffUL,
	0x0eeeeeeeeUL,
	0x0ddddddddUL,
	0x0ccccccccUL,
	0x0bbbbbbbbUL,
	0x0aaaaaaaaUL,
	0x099999999UL,
	0x088888888UL,
	0x077777777UL,
	0x066666666UL,
	0x055555555UL,
	0x044444444UL,
	0x033333333UL,
	0x022222222UL,
	0x011111111UL,
	0x000000000UL,
};

void test(void)
{
	int i, j, a;
	for(j = 0; j < 16; ++j){
		wait_vsync();
		for(i = 0; i < 0x1ffff; i+=4){
			pokel(0x120, i, color_table[j]);
//			a = peekl(0x120, i);
		}
	}
/*
__asm{
	mov	ax,0b000h
	mov	es,ax
	cli
	hlt
}
*/
}

#include "common.h"

/*メインルーチン
　初期設定とメインループ*/
//#ifndef DEBUG2
int main(void)
//#else
//void main_c(void)
//#endif
{

#ifdef DEBUG2
//	set_int41();
#endif

	pal_all(0, (unsigned char (*)[3])org_pal);
//	set_constrast(0, org_pal, 0);

	outp(0x448,0);
	outp(0x44a,0x11);	// layer 0 DISPLAY ON, layer 1 DISPLAY OFF

	outpm(0xc000, 0xff82, 0x67); // All Plane Display

//	cls();

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

	__asm __volatile(
		"mov	$0x4c,%ah\n"
		"int	$0x21\n"
	);

	return 0;
}


