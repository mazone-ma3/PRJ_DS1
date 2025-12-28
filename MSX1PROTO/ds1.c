// ds1_full.c - Dragon Sword 1 Complete Prototype (MSX1 / z88dk)
// 2025/12/26 Compiled

#include <msx.h>
#include <msx/gfx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inkey.h"

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
__asm
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
__endasm;
}

void put_chr8(int x, int y, char chr) {
	if((x < 0) || (y < 0))
		return;
	vpoke(BASE_ADDRESS + x + y * 32, chr);
}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 2, y * 2, chr);
	put_chr8(x * 2 + 1, y * 2,  ' ');
	put_chr8(x * 2 , y * 2 + 1,  ' ');
	put_chr8(x * 2 + 1, y * 2 + 1,  ' ');
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


void define_tiles(void) {
	copy_to_vram(tile_wall, (TILE_WALL * 8), 8); 
	copy_to_vram(tile_floor, (TILE_FLOOR * 8), 8);
	copy_to_vram(tile_player, (TILE_PLAYER * 8), 8);
	copy_to_vram(tile_normal, (TILE_NORMAL * 8), 8);	//通常パネル
	copy_to_vram(tile_gravity,(TILE_GRAVITY * 8), 8);	// 重力パネル
	copy_to_vram(tile_goal,	(TILE_GOAL * 8), 8);
	copy_to_vram(tile_slime, (TILE_SLIME * 8), 8);
	copy_to_vram(tile_hollow, (TILE_HOLLOW * 8), 8);	//	:新ブロック
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
__endasm;
}


unsigned char get_trigger1(unsigned char trigno) __sdcccall(1)
{
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
__endasm;
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

int main(void)
{
	msx_color(15, 1, 1);
	set_mode(1);

	for(;;)
		main2();

	return 0;
}
