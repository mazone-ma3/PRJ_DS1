/* ds1_mz25.c for z88dk MZ-2500 ZSDCC版 By m@3 */
/* キャラを出す  */

#include "mode.h"

//#include <stdio.h>
//#include <stdlib.h>
#include <conio.h>

//#include "arch\mz2500.h"

#include "ds1_pcg.h"
#include "inkey.h"

#define A_KEY "Z"
#define B_KEY "X"

#define SIZE 80
#define VRAM_ADR 0x4000 //0xa000

//#define CHR_SIZE 0xf00

unsigned char *vram_ofs;

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

#define PARTS_X 2
//#define PARTS_Y 8

#define MAXCOLOR 8

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

void set_pcg(void)
{
	unsigned char a, b, *mem, *mem2, *mem3;
	int i,j, k;

	DI();

	outp(0xcf, 0);

	outp(0xb4,2);
	a = inp(0xb5);
	outp(0xb4,3);
	b = inp(0xb5);

/*	outp(0xb4,2);
	outp(0xb5,0x38);

	// Set Text/
	mem = (unsigned char *)0x4000;
	for(i = 0; i < 256; ++i){
		mem[i] = i;			// Text1
		mem[i+0x800] = 0x0f; //f;			// Atr
		mem[i+0x1000] = 0 ;//(i % 2) * 2;	// Text2
	}

	outp(0xb4,2);
	outp(0xb5,a);
*/
	outp(0xb4,3); //6);
	outp(0xb5,0x39);

	/* Set PCG */
	mem3 = (unsigned char *)ds1_pcg; //0x6000;
	for(j = 0; j < 3; ++j){
		mem2 = (unsigned char *)(0x6000 + 0x800 * (j+1));
		for(i = 0; i < 256 * 8 / 8; ++i){
			for(k = 0; k < 8; ++k){
			*(mem2++) = *(mem3++);
			}
		}
	}

	outp(0xb4,3);
	outp(0xb5,b);

	EI();
}

unsigned char a,b;

void vram_off(void){
	outp(0xb4,2);
	outp(0xb5,a);
	outp(0xb4,3);
	outp(0xb5,b);
}

void vram_i_on(void){
	outp(0xb4,2);
	outp(0xb5,0x26);
	outp(0xb4,3);
	outp(0xb5,0x27);
}

void vram_b_on(void){
	outp(0xb4,2);
	outp(0xb5,0x20);
	outp(0xb4,3);
	outp(0xb5,0x21);
}

void vram_r_on(void){
	outp(0xb4,2);
	outp(0xb5,0x22);
	outp(0xb4,3);
	outp(0xb5,0x23);
}

void vram_g_on(void){
	outp(0xb4,2);
	outp(0xb5,0x24);
	outp(0xb4,3);
	outp(0xb5,0x25);
}

void pat_sub(char no)
{
	DI();
	/* TextRam ON */
	outp(0xb4,2);
	outp(0xb5,0x38);

	vram_ofs[0] = no * 2;
	vram_ofs[1] = no * 2 + 1;

	vram_off();
	EI();
}

void pat_sub2(void)
{
	DI();
	/* TextRam ON */
	outp(0xb4,2);
	outp(0xb5,0x38);

	vram_ofs[0] = 0x20+128/2;
	vram_ofs[1] = 0x20+128/2;

	vram_ofs[0x800] = 0x0f;		/* 白 */
	vram_ofs[0x801] = 0x0f;
	vram_ofs[0x1000] = 0x00;	/* PCG */
	vram_ofs[0x1001] = 0x00;

	vram_off();
	EI();
}


void put_chr8(int x, int y, char chr, char atr) {
	if((x < 0) || (y < 0))
		return;
	x+=(32/8)*1;

	vram_ofs = (unsigned char *)(VRAM_ADR +(x) * PARTS_X + (y) * SIZE);
	pat_sub(chr);
}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 2 + 0, y * 2 + 0, chr * 2 + 0, 0x27);
	put_chr8(x * 2 + 1, y * 2 + 0, chr * 2 + 1, 0x27);

	put_chr8(x * 2 + 0, y * 2 + 1, chr * 2 + 16, 0x27);
	put_chr8(x * 2 + 1, y * 2 + 1, chr * 2 + 17, 0x27);
}

// VRAM直書き
void print_at(int x, int y, char *str) {
	char chr;
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;

		if(chr >= 'a')
			chr -= ('a'-'A');
		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		put_chr8(x++, y, chr*1+128/2, 0x27);
	}
}

void print_at_2(int x, int y, char *str) {
	char chr;
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;

		if(chr >= 0x30)
			chr -= 0x30;
		else
			chr = 0x10;

		put_chr8(x++, y, chr*1 + 128/2, 0x27);
	}
}

void put_logo(int x, int y)
{
	print_at_2(x, y, "      i  k   ");
	print_at_2(x, y+1, " 2026 bcdefgh");
}

void wait_vsync(void)
{
	while((inp(0xbd) & 0x80)); /* WAIT VSYNC */
	while(!(inp(0xbd) & 0x80));
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

	wait_vsync();
	DI();
	/* TextRam ON */
	outp(0xb4,2);
	outp(0xb5,0x38);
	vram_ofs = (unsigned char *)(VRAM_ADR);
	EI();
	for(j = 0; j < 24; j++){
		for(i = 0; i < 80; i+=2){
			vram_ofs[0] = (0x20 + 128/2);
			vram_ofs[1] = (0x20 + 128/2);
			vram_ofs += 2;
//			put_chr8(i, j, ' ', 0);
		}
	}
	vram_off();
}

unsigned char k1, k2, k3, k7, ka;
unsigned char  st, data_no;

unsigned char keycode = 0;

unsigned char keyscan(void)
{
	keycode = 0;

	outp(0xe8, 0x31);
	k1 =inp(0xea);

	outp(0xe8, 0x32);
	k2 =inp(0xea);

	outp(0xe8, 0x33);
	k3 =inp(0xea);

	outp(0xe8, 0x37);
	k7 =inp(0xea);

	outp(0xe8, 0x3a);
	ka =inp(0xea);

	st = inp(0xef);

	if(!(k1 & 0x04) || !(k3 & 0x08) || !(st & 0x01)){ /* 8 */
		keycode |= KEY_UP1;
	}
	if(!(k2 & 0x40) || !(k3 & 0x40) || !(st & 0x08)){ /* 6 */
		keycode |= KEY_RIGHT1;
	}
	if(!(k2 & 0x04) || !(k3 & 0x10) || !(st & 0x02)){ /* 2 */
		keycode |= KEY_DOWN1;
	}
	if(!(k2 & 0x10) || !(k3 & 0x20) || !(st & 0x04)){ /* 4 */
		keycode |= KEY_LEFT1;
	}
	if(!(k3 & 0x02) || !(k7 & 0x04) || !(st & 0x10)) /* Z,SPACE */
		keycode |= KEY_A;
	if(!(k7 & 0x01) || !(st & 0x20)) /* X */
		keycode |= KEY_B;

	return keycode;
}

void define_tiles(void) {
}

void beep(void)
{
}

void play_sound_effect(void) {
	beep();  // シンプルなビープ音
}

#include "common.h"

unsigned char colortbl[16] = {
	0x00, 0x19, 0x1a, 0x10, 0x1c, 0x1d, 0x1e, 0x1f,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
};

unsigned char i, j;

void main(void)
{
	// Graphic
	outp(0xbc, 0x0e);
	outp(0xbd, 0x86); // 0001 0110

	// Text
	outp(0xf4,0);
	outp(0xf5,0x5); //0000 0101;
//	outp(0xf4,0x00);
//	outp(0xf5,0x00);
/*
	outp(0xf4,0x03);
	outp(0xf5,0x26);
	outp(0xf4,0x05);
	outp(0xf5,0xee);
	outp(0xf4,0x07);
	outp(0xf5,0x0b);
	outp(0xf4,0x08);
	outp(0xf5,0x5b);*/
//	outp(0xf4,0x0f);
//	outp(0xf5,0x00);

	outp(0xf7,1);


//	mz2500_set_mode(0x14);	// 0001 0100
//	printf("%d", mz2500_set_mode(0x4));	// 1001 0000

	outp(0xcf, 0);

	outp(0xb4,2);
	a = inp(0xb5);
	outp(0xb4,3);
	b = inp(0xb5);

	/* Change Pallet & Priority */
	for(i = 0; i < 16; ++i){
		outp(0xf4, 0x80 + i);
		outp(0xf5, colortbl[i]);
	}

	for(i = 0; i < 40; ++i){
		for(j = 0; j < 25; ++j){
			vram_ofs = (unsigned char *)(VRAM_ADR +(i) * PARTS_X + (j) * SIZE);
			pat_sub2();
		}
	}

	set_pcg();

	cls();

#ifdef DEBUG2
	for(;;)
#endif
		main2();

	/* Change Pallet & Priority */
	for(i = 0; i < 16; ++i){
		outp(0xf4, 0x80 + i);
		outp(0xf5,i);
	}
}
