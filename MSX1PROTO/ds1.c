// ds1_full.c - Dragon Sword 1 Complete Prototype (MSX1 / z88dk)
// 2025/12/26 Compiled
#pragma output CRT_MODEL = 1
#pragma output CRT_ORG_CODE = 0x4000 // 開始アドレス
#pragma output CRT_SIZE_CODE = 32768 // 32KBを指定

#include <msx.h>
#include <msx/gfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inkey.h"

#define A_KEY "X"
#define B_KEY "C"

#define DI() {\
__asm\
	di\
__endasm;\
}

#define EI() {\
__asm\
	ei\
__endasm;\
}

#define BASE_ADDRESS 0x1800
//ｨｰｸﾀｱｴﾐﾂﾆ
enum {
	TILE_FLOOR = 0x00,   // 床
	TILE_PLAYER,   // プレイヤー
	TILE_SLIME,   // スライム
	TILE_WALL = 'ｸ',   // 壁
	TILE_NORMAL = 'ｰ',   // 通常パネル（S） - シンプルブロック
	TILE_GRAVITY = 'ﾀ',   // 重力パネル（B） - 下向き矢印付き
	TILE_GOAL = 'ｨ',//'ｱ',   // ゴール
	TILE_HOLLOW = 'ｨ'  // 格子状の壁
};

#define PRINT_MUL 1

void copy_to_vram(char *src, short dst, int size)
{
	while(size--)
		vpoke(dst++, *(src++));
}

void fill_vram(int size, char pattern)
{
	short adr = BASE_ADDRESS;
	while(size--)
		vpoke(adr++, pattern);
}

void beep(void)
{
/*__asm
	push	af
	push	hl
	push	bc
	push	de
	push	ix
	push	iy
	call 00c0h
	pop	iy
	pop	ix
	pop	de
	pop	bc
	pop	hl
	pop	af
__endasm;*/
}

void put_chr8(int x, int y, char chr) {
	if((x < 0) || (y < 0))
		return;
	vpoke(BASE_ADDRESS + x + y * 32, chr);
}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 2, y * 2, chr);
	if(chr <= 2)
		chr = ' ';
	put_chr8(x * 2 + 1, y * 2,  chr);
	put_chr8(x * 2 , y * 2 + 1,  chr);
	put_chr8(x * 2 + 1, y * 2 + 1,  chr);
}

// VRAM直書き
void print_at(int x, int y, char *str) {
	char chr;
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
			put_chr8(x++, y, chr);
	}
}

// vsync (jiffy使用)
unsigned char *jiffy = (unsigned char *)0xfc9e, old_jiffy;
void vsync(void) {
	old_jiffy = *jiffy;
	while (*jiffy == old_jiffy);
}

void wait(int j) {
	int i;
	for (i = 0; i < j; ++i)
		vsync();
}

void cls(void) {
	int i,j;
	for(j = 0l; j < 24; j++)
		for(i = 0; i < 32; ++i)
			put_chr8(i, j, ' ');
}

// タイルパターン定義 (8x8)
char tile_wall[8]	= {0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF};   // 壁
char tile_floor[8]   = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};   // 床
char tile_player[8]  = {0x18,0x3C,0x7E,0xDB,0xFF,0x66,0x66,0xFF};   // プレイヤー
char tile_normal[8]  = {0xFF,0xFF,0xDB,0xDB,0xDB,0xDB,0xFF,0xFF};   // 通常パネル（S） - シンプルブロック
char tile_gravity[8] = {0xFF,0xFF,0xDB,0xDB,0x7E,0x3C,0x18,0xFF};   // 重力パネル（B） - 下向き矢印付き
char tile_goal[8]	= {0x3C,0x42,0x81,0x81,0x81,0x81,0x42,0x3C};   // ゴール
char tile_hollow[8] = {0xFF,0x81,0x42,0x24,0x18,0x24,0x42,0xFF};  // 格子状の壁
char tile_slime[8]   = {0x18,0x3C,0x7E,0x99,0x7E,0x3C,0x18,0x00};   // スライム

unsigned short chrdata[] = {
	0x3F,0x7F,0xFF,0xFD,0xFF,0xFD,0xFF,0xEF,
	0xFE,0xF7,0xBF,0xFF,0xFB,0xD1,0x7F,0x3B,
	0xFC,0xBE,0xFF,0xFB,0xFF,0xDF,0xFD,0x7F,
	0xFF,0xBF,0xFD,0xEF,0xFF,0xFB,0xBE,0xFC,
	-1,-1,-1,-1,
	0x7F,0xBE,0xDD,0xEB,0xF7,0xEB,0xDD,0xBE,
	0x7F,0xBE,0xDD,0xEB,0xF7,0xEB,0xDD,0xBE,
	-1,-1,
	0x07,0x0E,0x0F,0x0A,0x0A,0x08,0x04,0x3F,
	0x7E,0x6F,0x6E,0x6F,0x0E,0x0F,0x0E,0x1E,
	0xC0,0xE0,0xE0,0xA0,0xA0,0x20,0x40,0xF8,
	0xFC,0xEC,0xEC,0xEC,0xE0,0xE0,0xE0,0xF0,
	0xBF,0xBF,0x00,0xFB,0xFB,0xFB,0x00,0xBF,
	-1,-1,-1,-1,-1,-1,
	-1,
	0x11,0x22,0x44,0x88,0x11,0x22,0x44,0x88,
	-1,
	0xE0,0xF0,0xF8,0x7C,0x3E,0x1F,0x0F,0x07,
	0x03,0x01,0x00,0x00,0x01,0x01,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC4,
	0xEA,0xF4,0xE8,0x44,0xA3,0x51,0x8A,0x04,
	0x0F,0x18,0x13,0x37,0x27,0x2F,0x2F,0x2F,
	0x2F,0x27,0x37,0x13,0x19,0x0C,0x07,0x00,
	0xF0,0x18,0xE8,0xEC,0xF4,0xF4,0xF4,0xF4,
	0xF4,0xE4,0xEC,0xC8,0x98,0x30,0xE0,0x00,
	-1,-1,-1,-1,-1,-1,
	0xFF,0xFE,0xCE,0xDE,0xFE,0xFC,0x81,0xFF,
	-1,-1,-1,-1,-1,-1,-1,
	0x3C,0x42,0x99,0xA1,0xA1,0x99,0x42,0x3C,
	-2
};

unsigned char colordata[6] = {
	0x8F,0x2F,0x81,0xE1,0xE1,0xF8,
};

void define_tiles(void) {
	int i,j;
/*	copy_to_vram(tile_wall, (TILE_WALL * 8), 8); 
*/
	copy_to_vram(tile_floor, (TILE_FLOOR * 8), 8);
	copy_to_vram(tile_floor, ((TILE_FLOOR+1) * 8), 8);
	copy_to_vram(tile_floor, ((TILE_FLOOR+2) * 8), 8);
	copy_to_vram(tile_floor, ((TILE_FLOOR+3) * 8), 8);
	copy_to_vram(tile_player, (TILE_PLAYER * 8), 8);
/*	copy_to_vram(tile_normal, (TILE_NORMAL * 8), 8);	//通常パネル
	copy_to_vram(tile_gravity,(TILE_GRAVITY * 8), 8);	// 重力パネル
	copy_to_vram(tile_goal,	(TILE_GOAL * 8), 8);
*/	copy_to_vram(tile_slime, (TILE_SLIME * 8), 8);
//	copy_to_vram(tile_hollow, (TILE_HOLLOW * 8), 8);	//	:新ブロック

//	for(i = 0; i < 25; ++i)
//		for(j = 0; j < 8; ++j)
	i = 0;
	j = 0;
	do{
		if(chrdata[j] == -1)
			i+=8;
		else
			vpoke(1344 + i++, chrdata[j]);
	}while(chrdata[++j] != -2);

	for(i = 0; i < 6; ++i){
		vpoke(8213 + i, colordata[i]);
	}

//	print_at(24,23,"ｨｰｸﾀｱｴﾐﾂﾆ");
}

unsigned char get_key(unsigned char matrix) __sdcccall(1)
{
	outp(0xaa, ((inp(0xaa) & 0xf0) | matrix));
	return inp(0xa9);
/*
__asm
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x0141	; SNSMAT(MAINROM)

	ld	 hl, #2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	#0x001c	; CALSLT

	ld	l,a
	ld	h,#0
__endasm;
*/
}

unsigned char get_stick1(unsigned char trigno) __sdcccall(1)
{
	(void)trigno;
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d5	; GTSTCK(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0

	pop	ix
	ret
__endasm;
	return 0;
}


unsigned char get_trigger1(unsigned char trigno) __sdcccall(1)
{
	(void)trigno;
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d8	; GTTRIG(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0

	pop	ix
	ret
__endasm;
	return 0;
}

unsigned char st0, st1, pd0, pd1, pd2, k3, k5, k7, k9, k10;
unsigned char keycode = 0;

unsigned char keyscan(void)
{
	DI();
	keycode = 0;

	k3 = get_key(3);

	k9 = get_key(9);
	k10 = get_key(10);
	k5 = get_key(5);

	st0 = get_stick1(0);
	st1 = get_stick1(1);

	pd0 = get_trigger1(0);
	pd1 = get_trigger1(1);
	pd2 = get_trigger1(3);

	EI();

	if((pd0) || (pd1) || !(k5 & 0x20)) /* X,SPACE */
		keycode |= KEY_A;
	if((pd2) || !(k3 & 0x01)) /* C */
		keycode |= KEY_B;
	if((st0 >= 1 && st0 <=2) || (st0 == 8) || (st1 >= 1 && st1 <=2) || (st1 ==8) || !(k10 & 0x08)) /* 8 */
		keycode |= KEY_UP1;
	if((st0 >= 4 && st0 <=6) || (st1 >= 4 && st1 <=6) || !(k9 & 0x20)) /* 2 */
		keycode |= KEY_DOWN1;

//	if(!(st & 0x0c)){ /* RL */
//		keycode |= KEY_START;
//	}else{
	if((st0 >= 6 && st0 <=8) || (st1 >= 6 && st1 <=8) || !(k9 & 0x80)) /* 4 */
		keycode |= KEY_LEFT1;
	if((st0 >= 2 && st0 <=4) || (st1 >= 2 && st1 <=4) || !(k10 & 0x02)) /* 6 */
		keycode |= KEY_RIGHT1;
//	}

	return keycode;
}

void play_sound_effect(void) {
	beep();  // シンプルなビープ音
}

#include "common.h"

#define clicksw ((unsigned char *)0xf3db)

int main(void)
{
	*clicksw = 0;
	msx_color(15, 1, 1);
	set_mode(1);

	for(;;)
		main2();

	return 0;
}
