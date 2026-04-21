/* ds1mz.c for z88dk MZ-1500 ZSDCC版 By m@3 */
/* キャラを出す  */

#include "mode.h"

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <conio.h>

#include "ds1_pcg.h"
//#include "fontpcg.h"
#include "inkey.h"

#define A_KEY "Z"
#define B_KEY "X"


#define vram_data ((volatile unsigned char *)0xd000)
#define atr ((volatile unsigned char *)0xd800)

#define keystrobe ((volatile unsigned char *)0xe000)
#define keydata ((volatile unsigned char *)0xe001)

#define joydata ((volatile unsigned char *)0xe008)


#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

void DI(void){
__asm
	DI
__endasm;
}

void EI(void){
__asm
	EI
__endasm;
}

void clearBuffer(void) {
	while (kbhit()) {
		getch(); // 押されているキーをすべて読み飛ばす
	}
}

#define pcgvram1 ((volatile unsigned char *)0xd400)
#define pcgvram2 ((volatile unsigned char *)0xdc00)

#define pcgdata ((volatile unsigned char *)0xd000)

void setpcg(unsigned char *data)
{
	int i, j, k = 0;
	for(j = 0; j < 256; ++j){
		for(i = 0; i < 8; ++i){
			outp(0xe5, 0x01);	/* blue */
			pcgdata[k] = *(data++);
			outp(0xe5, 0x02);	/* red */
			pcgdata[k] = *(data++);
			outp(0xe5, 0x03);	/* green */
			pcgdata[k] = *(data++);
			++k;
		}
	}
	outp(0xe6, 0x00);	/* close */
}


void wait_vsync(void)
{
	int i;
	for(i = 0; i < 200; ++i){
		while(!((*joydata) & 0x80)); /* WAIT VSYNC */
		while((*joydata) & 0x80);
	}
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
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

#define PRINT_MUL 1

void VPOKE(short adr, char data)
{
	pcgvram1[adr] = data;
	pcgvram2[adr] = 0x08;
}

void fill_vram(int size, char pattern)
{
	short adr = 0;
	while(size--)
		VPOKE(adr++, pattern);
}

void beep(void)
{
}

void put_chr8(int x, int y, char chr, char atr_data) {
	int i;

	if((x < 0) || (y < 0))
		return;
	x += (32 / 8) * 1;
//	VPOKE(x + y * 80, chr);
//	outp(0x2000 + x + y * 80, atr);

	i = x + y * 40;

	pcgvram1[i] = chr;
	pcgvram2[i] = 0x08;

}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 2 + 0, y * 2 + 0, chr * 2 + 0, 0x27);
	put_chr8(x * 2 + 1, y * 2 + 0, chr * 2 + 1, 0x27);

//	put_chr8(x * 4 + 2, y * 2 + 0, chr * 4 + 2, 0x27);
//	put_chr8(x * 4 + 3, y * 2 + 0, chr * 4 + 3, 0x27);


	put_chr8(x * 2 + 0, y * 2 + 1, chr * 2 + 16, 0x27);
	put_chr8(x * 2 + 1, y * 2 + 1, chr * 2 + 17, 0x27);

//	put_chr8(x * 4 + 2, y * 2 + 1, chr * 4 + 34, 0x27);
//	put_chr8(x * 4 + 3, y * 2 + 1, chr * 4 + 35, 0x27);
}

enum {
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_YELLOW,
	COLOR_WHITE
};

// VRAM直書き
void print_at(int x, int y, char *str) {
	char chr;
	while ((chr = *(str++)) != '\0') {
		if(chr >= 'a')
			chr -= ('a'-'A');

		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

/*		if (chr == 0x20) chr = 0x0;
		else if(chr >= 'A' && chr <= 'Z')
			chr -= 0x40;
		else if(chr >= '0' && chr <= '9')
			chr -= 0x10;

		vram_data[x + y * 40] = chr;
		atr[x + y * 40] = COLOR_BLACK | (COLOR_WHITE * 16);
		x++;
*/
		put_chr8(x++, y, chr + 64, 0x27);
	}
}

void print_at_2(int x, int y, char *str) {
	char chr;
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
//		put_chr8(x++, y, chr, 0x87);
//		put_chr8(x++, y, chr, 0x87);;

		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		put_chr8(x++, y, chr + 64, 0x27);
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
	int i,j,k=0;
	for(j = 0; j < 25; j++){
		for(i = 0; i < 40; ++i){
//			put_chr8(i, j, ' ', 0);
//			k = i + j * 40;
			pcgvram1[k] = 0x20;
			pcgvram2[k] = 0x08;

			vram_data[k] = 0;
			atr[k] = COLOR_BLACK | (COLOR_WHITE * 16);
			++k;
		}
	}
}

unsigned char keycode = 0;
unsigned char st = 0xff, k2, k1, k3, k6, k7;

unsigned char keyscan(void)
{
	keycode = 0;
//	DI();
//	outp(0x1c00,14);
//	st = *joydata;
//	EI();

	*keystrobe = 0x01 | 0x80;
	k1 = ~(*keydata);

	*keystrobe = 0x02 | 0x80;
	k2 = ~(*keydata);

	*keystrobe = 0x03 | 0x80;
	k3 = ~(*keydata);

	*keystrobe = 0x06 | 0x80;
	k6 = ~(*keydata);

	*keystrobe = 0x07 | 0x80;
	k7 = ~(*keydata);

	if((k7 & 0x20) || !(st & 0x01)){ /* ↑ */
		keycode |= KEY_UP1;
	}
	if((k7 & 0x08) || !(st & 0x08)){ /* → */
		keycode |= KEY_RIGHT1;
	}
	if((k7 & 0x10) || !(st & 0x02)){ /* ↓ */
		keycode |= KEY_DOWN1;
	}
	if((k7 & 0x04) || !(st & 0x04)){ /* ← */
		keycode |= KEY_LEFT1;
	}
	if((k6 & 0x10) || (k1 & 0x40)){ /* Z,SPACE */
		keycode |= KEY_A;
	}
	if((k2 & 0x01) || !(st & 0x40)){ /* X */
		keycode |= KEY_B;
	}
	return keycode;
}

void define_tiles(void) {
}

void play_sound_effect(void) {
	beep();  // シンプルなビープ音
}

#include "common.h"

int main(void)
{
	outp(0xf0,0x01);
	setpcg(ds1_pcg);
	cls();

/*	do{*/
#ifdef DEBUG2
	for(;;)
#endif
		main2();

#ifndef DEBUG2
	cls();

#endif
	return NOERROR;
}

