/* ds1_88.c for z88dk PC-8801/mkIISR ZSDCC版 By m@3 */
/* キャラを出す */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "inkey.h"
#include "ds1_88.h"
#include "font88.h"

#define VRAM_ADR 0xc000
#define PARTS_SIZE 0x1e00 //0x2000

//static unsigned char mainram_data[PARTS_SIZE];
//static unsigned char font_data[PARTS_SIZE];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

#define SIZE 80

//#define X_SIZE 18
//#define Y_SIZE 18

#define PARTS_X 2
#define PARTS_Y 8

//#define OFS_X 2
//#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char tmp_vram[PARTS_X * PARTS_Y * 3];
unsigned char basic_mode = 0;
unsigned char *n88rom;
unsigned char rom_mode = 0;

#define MAXCOLOR 8

unsigned char org_pal[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};

unsigned char romport;

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

void  cls88(unsigned char color)
{
	(void)color;
__asm
	ld	hl, 2
	add	hl, sp
	ld	a, (hl)
	ld	b,a

	DI

;	XOR A
;	LD BC,#0x035C
;	DI

	in	a,(#0x32)
	or	#0x40
	out	(#0x32),a	; USE ALU
	ld	a,#0x80
	out	(#0x35),a	; Access GVRAM

	xor	a
;	ld	a,#0x07
	out	(#0x34),a

	ld	a, b
;	xor	#0xff
	cpl

;loop1:
;	OUT (C),A
	LD HL,#0x0C000
loop2:
;	LD C,#0x5C
;	OUT (C),A
	LD (HL),A
;	LD C,#0x5D
;	OUT (C),A
;	LD (HL),A
;	LD C,#0x5E
;	OUT (C),A
;	LD (HL),A

	INC HL
	CP H
	JR NZ,loop2
;	INC C
;	DJNZ loop1
;	OUT (#0x5F),A

	xor a
	out(#0x35),a	; Access MAINRAM
	in	a,(#0x32);
	and #0xbf;
	out(#0x32),a	; NOUSE ALU


	LD C,#0x32
	IN A,(C)
	AND #0xef
	OUT (C),A
	XOR A
	LD HL,#0x0f3c8
loop3:
	LD (HL),A
	INC HL
	CP H
	JR NZ,loop3
	IN A,(C)
	OR #0x10
	OUT (C),A
	EI
__endasm;
}


void Set_RAM_MODE(void){
__asm
;	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
	ld	a,#0x3b
	out(#0x31),a
	EI
__endasm;
}

void Set_ROM_MODE(void){
__asm
	DI
;	ld	a,(#0xe6c2)
	ld	a,#0x39
	out(#0x31),a
;	EI
__endasm;
}
/*
void  set_mapchr(unsigned char *srcram, unsigned char *dstram) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	c, (hl)
;	inc	hl
;	ld	b, (hl)	; bc=srcram
;	inc	hl
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	; de = dstram
;	ld	l,c
;	ld	h,b	; hl = srcram


	ld	b,8
	push	hl
looppat11:
;	push	bc

	DI
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	ld	a,(hl)	;b1
	ld	(de),a
	inc	hl
	inc	de
	inc	de
	ld	a,(hl)	;r1
	ld	(de),a
	inc	hl
	inc	de
	inc	de
	ld	a,(hl)	;g1
	ld	(de),a
	inc	hl
	dec	de
	dec	de
	dec	de

	ld	a,(hl)	;b2
	ld	(de),a
	inc	hl
	inc	de
	inc	de
	ld	a,(hl)	;r2
	ld	(de),a
	inc	hl
	inc	de
	inc	de
	ld	a,(hl)	;g2
	ld	(de),a
	inc	de

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI

	push	de	;	hl = mainram
	ld	de, 91 ;((256 / 8 - 2) * 3 + 1)
	add	hl,de
	pop	de

;	pop	bc
	djnz looppat11
	pop	hl

__endasm;
}*/

void  put_chr88_pat(unsigned char *vram, unsigned char *mainram) __sdcccall(1)
{
	(void)vram;
	(void)mainram;
__asm
;	ld	hl, 2
;	add	hl, sp
;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; bc = vram
;	inc	hl
;	ld	c,(hl)
;	inc	hl
;	ld	b,(hl)	; de = mainram
;	ld	l,c
;	ld	h,b	; hl = vram

;	ex	de,hl

; Init draw address
	ld	bc, 80 ;SCRWidth
	ld	(_put00 + 1), hl
	ld	(_put01 + 1), hl
	ld	(_put02 + 1), hl
	add	hl, bc

	ld	(_put03 + 1), hl
	ld	(_put04 + 1), hl
	ld	(_put05 + 1), hl
	add	hl, bc

	ld	(_put06 + 1), hl
	ld	(_put07 + 1), hl
	ld	(_put08 + 1), hl
	add	hl, bc

	ld	(_put09 + 1), hl
	ld	(_put0A + 1), hl
	ld	(_put0B + 1), hl
	add	hl, bc

	ld	(_put10 + 1), hl
	ld	(_put11 + 1), hl
	ld	(_put12 + 1), hl
	add	hl, bc

	ld	(_put13 + 1), hl
	ld	(_put14 + 1), hl
	ld	(_put15 + 1), hl
	add	hl, bc

	ld	(_put16 + 1), hl
	ld	(_put17 + 1), hl
	ld	(_put18 + 1), hl
	add	hl, bc

	ld	(_put19 + 1), hl
	ld	(_put1A + 1), hl
	ld	(_put1B + 1), hl

	; draw
	ex	de, hl
	ld	c,l
	ld	b,h

	ld	(_stack + 1), sp
	di
;	ld	a,(#0xe6c2)
;	or	#0x02
;	out	(#0x31),a

	ld	sp, hl

	pop	hl
	out	(#0x5c),a
_put00:	ld	(#0x0000), hl	;1
	pop	hl
	out	(#0x5d),a
_put01:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put02:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put03:	ld	(#0x0000), hl	;2
	pop	hl
	out	(#0x5d),a
_put04:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put05:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put06:	ld	(#0x0000), hl	;3
	pop	hl
	out	(#0x5d),a
_put07:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put08:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put09:	ld	(#0x0000), hl	;4
	pop	hl
	out	(#0x5d),a
_put0A:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put0B:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put10:	ld	(#0x0000), hl	;5
	pop	hl
	out	(#0x5d),a
_put11:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put12:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put13:	ld	(#0x0000), hl	;6
	pop	hl
	out	(#0x5d),a
_put14:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put15:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put16:	ld	(#0x0000), hl	;7
	pop	hl
	out	(#0x5d),a
_put17:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put18:	ld	(#0x0000), hl

	pop	hl
	out	(#0x5c),a
_put19:	ld	(#0x0000), hl	;8
	pop	hl
	out	(#0x5d),a
_put1A:	ld	(#0x0000), hl
	pop	hl
	out	(#0x5e),a
_put1B:	ld	(#0x0000), hl

_stack:	ld	sp, #0x0000

__endasm;

__asm
	out	(#0x5f),a

;	ld	a,(#0xe6c2)
;	out(#0x31),a
	EI
__endasm;
}

unsigned char no;
unsigned char *vram_adr;

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outp(0x54 + color, red * 8 | blue);
	outp(0x54 + color, green | 0x40);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0]/2, color[i][1]/2, color[i][2]/2);
}

void wait_vsync(void)
{
	while((inp(0x40) & 0x20)); /* WAIT VSYNC */
	while(!(inp(0x40) & 0x20));
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
	unsigned char j, k;
	unsigned char pal[3];

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
		if(j == (MAXCOLOR / 2))
			wait_vsync();
		pal_set(pal_no, j, pal[0]/2, pal[1]/2, pal[2]/2);
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
	unsigned char j;
	for(j = 0; j < MAXCOLOR; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

unsigned char k0, k1, k8, ka, st, pd, k5, k9, *data, *data_tmp;
unsigned char pat_no, *pat_adr;
unsigned char *vram_adr_tmp;

unsigned char sub_flag;

unsigned char fadeflag = 0;

unsigned char *sysport = (unsigned char *)0xe6c2;

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

void beep(void)
{
}

void put_chr8(int x, int y, char chr, char atr) {
	if((x < 0) || (y < 0))
		return;

	vram_adr = (char *)(VRAM_ADR + x + y * PARTS_Y * SIZE);
//	put_chr88_pat(vram_adr, &mainram_data[chr * PARTS_X * PARTS_Y * 3]);
	put_chr88_pat(vram_adr, &ds1_grp[chr * PARTS_X * PARTS_Y * 3]);
}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 4 + 0, y * 2 + 0, chr * 2 + 0, 0x27);
	put_chr8(x * 4 + 2, y * 2 + 0, chr * 2 + 1, 0x27);

	put_chr8(x * 4 + 0, y * 2 + 1, chr * 2 + 16, 0x27);
	put_chr8(x * 4 + 2, y * 2 + 1, chr * 2 + 17, 0x27);
}

char chr;

// VRAM直書き
void print_at(char x, char y, char *str) {
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
		if(chr >= 'a')
			chr -= ('a'-'A');
		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		vram_adr = (char *)(VRAM_ADR + (x++) * PARTS_X + y * PARTS_Y * SIZE);
//		put_chr88_pat(vram_adr, &font_data[chr * PARTS_X * PARTS_Y * 3]);
		put_chr88_pat(vram_adr, &font_grp[chr * PARTS_X * PARTS_Y * 3]);
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
	cls88(0);
/*	int i,j;
	for(j = 0l; j < 24; j++)
		for(i = 0; i < 80; ++i)
			put_chr8(i, j, ' ', 0);*/
}

void define_tiles(void) {
}

void play_sound_effect(void) {
	beep();  // シンプルなビープ音
}


unsigned char keycode = 0;

unsigned char keyscan(void)
{
	keycode = 0;

	k0 = inp(0x00);
	k1 = inp(0x01);
	k8 = inp(0x08);
	k9 = inp(0x09);
	ka = inp(0x0a);
//	if(!(rom_mode <= 0x33)){
	if(!basic_mode){
		DI();
		outp(0x44, 0x0e);
		st = inp(0x45);
		outp(0x44, 0x0f);
		pd = inp(0x45);
		EI();
	}
	k5 = inp(0x05);
	if(!(k1 & 0x01) || !(k8 & 0x02) || !(st & 0x01)){ /* 8 */
		keycode |= KEY_UP1;
	}
	if(!(k0 & 0x40) || !(k8 & 0x04) || !(st & 0x08)){ /* 6 */
		keycode |= KEY_RIGHT1;
	}
	if(!(k0 & 0x04) || !(ka & 0x02) || !(st & 0x02)){ /* 2 */
		keycode |= KEY_DOWN1;
	}
	if(!(k0 & 0x10) || !(ka & 0x04) || !(st & 0x04)){ /* 4 */
		keycode |= KEY_LEFT1;
	}
	if(!(k5 & 0x04) || !(k9 & 0x40) || !(pd & 0x01)){ /* Z,SPACE */
		keycode |= KEY_A;
	}
	if(!(k5 & 0x01) || !(pd & 0x02)){ /* X */
		keycode |= KEY_B;
	}

	return keycode;
}

#include "common.h"

int main(void)
{
//	printf("sys=%x mode=%x\n",*sysport, inp(0x32));

	cls88(0);

	Set_RAM_MODE();

//	outp(0x32, inp(0x32) | 0x20);	/* 512 colors */
	basic_mode = inp(0x31) & 0x80;
//	n88rom = (unsigned char *)0x79d7;
//	Set_ROM_MODE();
//	rom_mode = *n88rom;
//	Set_RAM_MODE();

	if(basic_mode)
		outp(0x51, 0);

/*	for(i = 0; i < 128; ++i){
		unsigned short pat_tmp = (i & 0x0f) * 2 + (i & 0xf0) * 16;
		set_mapchr(&ds1_grp[pat_tmp * 3], &mainram_data[i * PARTS_X * PARTS_Y * 3]);
	}
	for(i = 0; i < 128; ++i){
		unsigned short pat_tmp = (i & 0x0f) * 2 + (i & 0xf0) * 16;
		set_mapchr(&font_grp[pat_tmp * 3], &font_data[i * PARTS_X * PARTS_Y * 3]);
	}
*/

	st = pd = 255;
//	while(((k9 = inp(0x09)) & 0x80)){ /* ESC */
//	for(;;){
		main2();

/*		if(fadeflag == 0){
			fadeflag = 1;
			if(!basic_mode)
				fadeinblack(org_pal, 0, 3);
		}*/
//	}
	if(!basic_mode){
		fadeoutblack(org_pal, 0, 3);
		pal_all(0, org_pal);
	}
	if(basic_mode){
		outp(0x51, 0x20);
	}
	cls();
	clearBuffer();
	return NOERROR;
}

