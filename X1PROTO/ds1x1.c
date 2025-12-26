/* ds1x1.c for z88dk X1/turbo/turboZ ZSDCC版 By m@3 */
/* .com版 スタンダードモードで起動して下さい 
/* キャラを出す  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ds1_pcg.h"
#include "inkey.h"

//#define MAP_ADR 0x8000
//#define PARTS_DATA (MAP_ADR+0x3c00)
//#define CHRPAT_ADR 0xC000

#define PARTS_SIZE 0x1800
#define CHR_SIZE 0xf00

unsigned short y_table[200];

unsigned char vram_data[CHR_SIZE];
//#define VRAM_DATA_ADR vram_data

#define PARTS_HEAD 0x3c00 /*組み合わせキャラデータの先頭番地*/
//#define BUFFSIZE 16384
//unsigned char mapdata[BUFFSIZE];

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0


#define SIZE 80

#define X_SIZE 18
#define Y_SIZE 18

#define PARTS_X 2
#define PARTS_Y 8

#define MAP_SIZE_X 128
#define MAP_SIZE_Y 128

#define OFS_X 2
#define OFS_Y 2

#define CHR_X 8
#define CHR_Y 8

unsigned char turbo = 0;

unsigned short vram_ofs;
//unsigned char map_data[(X_SIZE+2) * 32];
//unsigned char *WK1FD0 = (unsigned char *)0xf8d6;

#define MAXCOLOR 8

/* BRG */

unsigned char org_pal[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 0, 0, 0 },//	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};

unsigned char org_pal2[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },};

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

/* (24倍速)PCG設定 試験に出るX1ほぼまんま */
/* データを8バイトx256パターンx3プレーン=6144バイト設定 */
void set_pcg(unsigned char *mainram)
{
__asm
BLUE	EQU	#0x15+1
RED		EQU	#0x16+1
GREEN	EQU	#0x17+1

MULTI	EQU	#8 ;1だと3倍速

	ld	hl, 2
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)	; bc=mainram
;	push	bc
	ld	l,c
	ld	h,b	; hl = mainram
	ld	(HLWORK),hl

	ld	bc,#0x1800
	ld	a,6
	out	(c),a
	inc	bc
	ld	a,18
	out(c),a

START:
	ld	bc,#0x1FD0
	xor	a
	out	(c),a

	ld	bc,#0x3800+#0x5a0
	ld	hl,#0x260
	ld	d,0
	call	SETIO

	ld	bc,#0x2000+#0x5a0
	ld	hl,#0x260
	ld	d,#0x20
	call	SETIO

;	ld	hl,#0xe200
;	pop	hl
;	ld	(HLWORK),hl
	xor	A

LOOP:
	push	af
	call	SET8
	ld	hl,(HLWORK)
	call	SETPCG
	ld	hl,(HLWORK)
	ld	bc,24*MULTI ;8
	add	hl,bc
	ld	(HLWORK),hl
	pop	af
	add	a,MULTI;8
	jp	nz,LOOP

	ld	bc,#0x1800
	ld	a,6
	out	(c),a
	inc	bc
	ld	a,25
	out	(c),a

;	ld	bc,#0x1FD0
;	ld	a,0x03
;	out	(c),a
	jmp	END

SET8:
	ld	bc,#0x3000+#0x5a0
	ld	d,a
	ld	e,MULTI ;8
SET80:
	push	bc
	ld	hl,48
	call	SETIO
	pop	bc
	ld	hl,80
	add	hl,bc
	ld	b,h
	ld	c,l
	inc	d
	dec	e
	jp	nz,SET80

	ret

SETIO:
	out	(c),d
	inc	bc
	dec	hl
	ld	a,h
	OR	l
	jp	nz,SETIO
	ret

SETPCG:
	ld	b,#0x15+1 ;BLUE
	ld	c,0
	ld	d,#0x16+1 ;RED
	ld	e,#0x17+1 ;GREEN
	ld	a,#0x08*MULTI ;8
	ex	af,af
	exx

	di
	ld	bc,#0x1A01
VDSP0:
	in	a,(c)
	jp	p,VDSP0
VDSP1:
	in	a,(c)
	jp	m,VDSP1

	exx
	ex	af,af

SETP:
	outi
	ld	b,d
	outi
	ld	b,e
	outi

	ld	b,#0x15+1	;BLUE

	ex	af,af
	ld	a,#0x0b
DLY:
	dec	a
	jp	nz,DLY
	ex	af,af

	inc	c
	dec	a
	jp	nz,SETP

	ei
	ret
HLWORK:
	ds	2

;	END
END:
__endasm;
}

void set_key(void)
{
__asm
	ld	d,0xe4
	call	SEND1
	call	CANW
	ld	d,0
	call	SEND1
	call	CANW
	ld	d,0
	call	SEND1
	call	CANW

__endasm;
}

/* 試験に出るX1より引用 */
void get_key(unsigned char *data, unsigned short num)
{
__asm
	ld	hl, #2
	add	hl, sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)	; de = data

	inc	hl
	ld	c, (hl)
	inc	hl
	ld	b, (hl)	; bc = num


FM49:	ei
	ex	de,hl
	ld	d,(hl)
	inc	hl
	ld	e,c
	call	SEND1
	call	CANW
	di
	dec	e

FM49LP:	call	GET1
	ld	(hl),d
	inc	hl
	dec	e
	jr	nz,FM49LP
	ei
	ret

SEND1:	call	CANW
	ld	bc,#0x1900
	out	(c),d
	ret

GET1:	call	CANR
	ld	bc,#0x1900
	in	d,(c)
	ret

CANW:	ld	bc,#0x1a01
CANWLP:	in	a,(c)
	and	#0x40
	jr	nz,CANWLP
	ret

CANR:	ld	bc,#0x1a01
CANRLP:	in	a,(c)
	and	#0x20
	jr	nz,CANRLP
	ret
__endasm;
}

short pat_tmp;
unsigned char no;

unsigned char chr_tbl[8][4] = {
		{0, 1, 0 + 16, 1 + 16},
		{2, 3, 2 + 16, 3 + 16},
		{4, 5, 4 + 16, 5 + 16},
		{6, 7, 6 + 16, 7 + 16},
		{8, 9, 8 + 16, 9 + 16},
		{10, 11, 10 + 16, 11 + 16},
		{12, 13, 12 + 16, 13 + 16},
		{14, 15, 14 + 16, 15 + 16},
};
unsigned char dir = 2, dir2 = 0, i, j;

//#define VRAM_MACRO(X,Y) (X + (Y / 8) * 80 + (Y & 7) * 0x800)
//#define VRAM_MACRO(X,Y) (X + y_table[Y])
//unsigned char data2[3*2*8]; //[3][2][8];

/* 3プレーン転送 */
/* 0x4000,0x8000,0xc000 */
unsigned char adr_tmp2_x, adr_tmp2_y;

void put_chrx1_pat(unsigned short patadr) __sdcccall(1)
{
//	unsigned short adr_tmp;
//	unsigned char *adr_tmp, *adr_tmp3 = data2;
//	unsigned short ii,jj;
/*
//	adr_tmp = CHRPAT_ADR + patadr * 2 + patadr;
	adr_tmp = &vram_data[patadr * 2 + patadr];
	for(jj = 0 ; jj < PARTS_Y; ++jj){*/
//		DI();
//		outp(0x1fd0, *WK1FD0 | 0x10); /* BANK1 */
//		outp(0x1fd0, 0x10); /* BANK1 */
/*		for(ii = 0 ; ii < PARTS_X; ++ii){
//			data2[0][ii][jj] = inp(adr_tmp++);
//			data2[1][ii][jj] = inp(adr_tmp++);
//			data2[2][ii][jj] = inp(adr_tmp++);
//			data2[0][ii][jj] = (*adr_tmp++);
//			data2[1][ii][jj] = (*adr_tmp++);
//			data2[2][ii][jj] = (*adr_tmp++);
			(*adr_tmp3++) = (*adr_tmp++);
			(*adr_tmp3++) = (*adr_tmp++);
			(*adr_tmp3++) = (*adr_tmp++);
		}
		adr_tmp += (32 * 3 - PARTS_X * 3);*/
//		outp(0x1fd0, *WK1FD0); /* 元に戻す */
//		outp(0x1fd0, 0); /* 元に戻す */
//		EI();
//	}
__asm
	ld	c,l
	ld	b,h
	add	hl,bc
	add	hl,bc
	ld	bc,_vram_data;
	add	hl,bc
	push	hl
__endasm;
/*
	ld	de,_data2
	ld	c,PARTS_Y
patloop1:
	ld	b,PARTS_X
patloop2:
	push	bc
	ldi
	ldi
	ldi
	pop	bc
	djnz patloop2
	push	de
	ld	de,32*3-PARTS_X*3
	add	hl,de
	pop	de
	dec	c
	jr	nz,patloop1
__endasm;
*/
//	adr_tmp2 = VRAM_MACRO(((OFS_X - 1 + i) * PARTS_X), ((OFS_Y - 1 + j) * PARTS_Y));
	adr_tmp2_x = ((OFS_X - 1 + i) * PARTS_X);
	adr_tmp2_y = ((OFS_Y - 1 + j) * PARTS_Y);

/*	adr_tmp3 = data2;
	for(jj = 0 ; jj < PARTS_Y; ++jj){
		adr_tmp2 = VRAM_MACRO(((OFS_X - 1 + i) * PARTS_X), ((OFS_Y - 1 + j) * PARTS_Y + jj));
		for(ii = 0 ; ii < PARTS_X; ++ii){
//			outp(0x4000 + adr_tmp2, data2[0][ii][jj]);
//			outp(0x8000 + adr_tmp2, data2[1][ii][jj]);
//			outp(0xc000 + adr_tmp2, data2[2][ii][jj]);
			outp(0x4000 + adr_tmp2, (*adr_tmp3++));
			outp(0x8000 + adr_tmp2, (*adr_tmp3++));
			outp(0xc000 + adr_tmp2, (*adr_tmp3++));
			++adr_tmp2;
		}
	}*/
__asm
;	ld	de,_data2
	pop	de
	ld	c,0 ;PARTS_Y
patloop3:
	push	bc
	ld	a,(_adr_tmp2_y)
	add	a,c
	ld	c,a
	ld	b,0
	ld	hl,_y_table
	add	hl,bc
	add	hl,bc
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	a,(_adr_tmp2_x)
	ld	l,a
	ld	h,0
	add	hl,bc
	pop	bc
	ld	b,PARTS_X
patloop4:
	push	bc
	push	hl
	ld	c,l
	ld	b,h
	ld	hl,0x4000
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	inc	de
	out (c),a
	ld	hl,0x4000
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	inc	de
	out (c),a
	ld	hl,0x4000
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a,(de)
	inc	de
	out (c),a
	pop	hl
	inc	hl
	pop	bc
	djnz patloop4

	ld	hl,32*3-PARTS_X*3
	add	hl,de
	ex	de,hl

	inc	c
	ld	a,c
	cp	PARTS_Y
	jr	nz,patloop3
__endasm;
}

void pat_sub(void)
{
//	outp(0x3000 + vram_ofs, no * 2);
//	outp(0x3000 + vram_ofs+1, no * 2 +1);
__asm
	ld	bc,(_vram_ofs)
	ld	a,(_no)
	sla	a
	out (c),a
	inc	bc
	inc	a
	out	(c),a
__endasm;
}

void pat_sub2(void)
{
	outp(0x2000 + vram_ofs, 0x27);
	outp(0x2000 + vram_ofs+1, 0x27);
}

void chr_sub(void)
{
	unsigned char no2 = chr_tbl[dir * 2 + dir2][(i - CHR_X) + (j - CHR_Y) * 2];

	put_chrx1_pat((no2 & 0x0f) * 2 + (no2 & 0xf0) * 16);

	pat_sub();
}

/*パレット・セット*/
void pal_set_text(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outp(0x1fb9 - 1 + color, green / 4 * 16 + red / 4 * 4 + blue / 4);
}

void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	unsigned short adr1, adr2;
	adr1 = (color & 0x04) / 4 * 16 * 8 + (color & 0x02) / 2 * 8;
	adr2 = (color & 0x01) * 16 * 8;
	outp(0x1000 + adr1, adr2 + blue);
	outp(0x1100 + adr1, adr2 + red);
	outp(0x1200 + adr1, adr2 + green);

	if(color)
		pal_set_text(pal_no, color, red, green, blue);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short j;
	unsigned short adr1, adr2;
	for(j = 0; j < 4096; j++){
		adr1 = (j & 0xf00) / 256 * 16 + (j & 0xf0) / 16;
		adr2 = (j & 0x0f) * 16;
		outp(0x1000 + adr1, adr2 + j & 0x0f);
		outp(0x1100 + adr1, adr2 + (j & 0xf0) / 16);
		outp(0x1200 + adr1, adr2 + (j & 0xf00) / 256);
	}
}

void pal_all_text(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	for(j = 1; j < MAXCOLOR; j++)
		pal_set_text(pal_no, j, color[j][0], color[j][1], color[j][2]);
}

void wait_vsync(void)
{
	while(!(inp(0x1a01) & 0x80)); /* WAIT VSYNC */
	while((inp(0x1a01) & 0x80));
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

//		DI();
//		wait_vsync();
//		outp(0x1fb0, 0x90);	/* 多色モード */
//		outp(0x1fc5, 0x80);	/* グラフィックパレットアクセスON */

		pal_set(pal_no, j, pal[0], pal[1], pal[2]);

//		outp(0x1fc5, 0x0);
//		outp(0x1fb0, 0x0);
//		EI();
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
	short j;
	unsigned short adr1, adr2;

	for(j = 0; j < 4096; j++){
		adr1 = (j & 0xf00) / 256 * 16 + (j & 0xf0) / 16;
		adr2 = (j & 0x0f) * 16;
		outp(0x1000 + adr1, adr2 | 0x00);
		outp(0x1100 + adr1, adr2 | 0x00);
		outp(0x1200 + adr1, adr2 | 0x00);
	}

	for(j = 1; j < 8; j++)
		outp(0x1fb9 - 1 + j, 0);
}

unsigned char data_buf[5];

unsigned char k0, k1, k2, k3, st, data_no;
unsigned char k0_old, k1_old, k2_old;
//unsigned short data, data_tmp;
unsigned char *data, *data_tmp;

unsigned char pat_no;
//unsigned short pat_adr;
unsigned char *pat_adr;
unsigned char x = 165, y = 30,xx, yy, old_x = 255, old_y = 255, k;

unsigned short vram_ofs_tmp;

//unsigned char old_map_data[(X_SIZE + 2) * 32];
unsigned char sub_flag;
unsigned char *map_adr;
unsigned char *old_map_adr;

unsigned char fadeflag = 0;


unsigned char check_turbo(void)
{
__asm
	ld	h,#0
	ld	l,#0

	ld	b, #0x1f
	ld	c, #0xa0			; 本体側のCTC($1fa0)があればturbo
	ld	a,7 ;00000111B
	out	(c), a
	out	(c), c
	in	a, (c)
	xor	c
	jr	nz,CHKEND
	ld	l,1
CHKEND:
__endasm;
}

/* BNN X1-Techknowより */

unsigned char hireso[12] = {
	0x6B,0x50,0x59,0x88,0x1B,0x00,0x19,0x1A,0x00,0x0F,0x00,0x00	//80line
//	0x35,0x28,0x2d,0x84,0x1B,0x00,0x19,0x1A,0x00,0x0F,0x00,0x00	//40line
};
unsigned char lowreso[12] = {
	0x6F,0x50,0x59,0x38,0x1F,0x02,0x19,0x1C,0x00,0x07,0x00,0x00	//80line
//	0x37,0x28,0x2d,0x34,0x1F,0x02,0x19,0x1C,0x00,0x07,0x00,0x00	//40line
};

void set_hireso(void)
{
	unsigned char i;
	for(i = 0; i < 12; ++i){
		outp(0x1800,i);
		outp(0x1801,hireso[i]);
	}
}

void set_lowreso(void)
{
	unsigned char i;
	for(i = 0; i < 12; ++i){
		outp(0x1800,i);
		outp(0x1801,lowreso[i]);
	}
}


void VPOKE(short vram, char data)
{
	unsigned short adr = (unsigned short)vram;
	outp(0x3000 + adr, data);
}

void fill_vram(int size, char pattern)
{
	short adr = 0; //(char *)BASE_ADDRESS;
	while(size--)
		VPOKE(adr++, pattern);
}

void beep(void)
{
}

void put_chr8(int x, int y, char chr, char atr) {
	if((x < 0) || (y < 0))
		return;
	VPOKE(x + y * 80, chr);
	outp(0x2000 + x + y * 80, atr);
}

void put_chr16(int x, int y, char chr) {
	put_chr8(x * 4 + 0, y * 2 + 0, chr * 4 + 0, 0x27);
	put_chr8(x * 4 + 1, y * 2 + 0, chr * 4 + 1, 0x27);

	put_chr8(x * 4 + 2, y * 2 + 0, chr * 4 + 2, 0x27);
	put_chr8(x * 4 + 3, y * 2 + 0, chr * 4 + 3, 0x27);


	put_chr8(x * 4 + 0, y * 2 + 1, chr * 4 + 32, 0x27);
	put_chr8(x * 4 + 1, y * 2 + 1, chr * 4 + 33, 0x27);

	put_chr8(x * 4 + 2, y * 2 + 1, chr * 4 + 34, 0x27);
	put_chr8(x * 4 + 3, y * 2 + 1, chr * 4 + 35, 0x27);
}

// VRAM直書き
void print_at(int x, int y, char *str) {
	char chr;
	while ((chr = *(str++)) != '\0') {
		if (chr < 0x20) chr = 0x20;
		put_chr8(x++, y, chr, 7);
	}
}

// vsync
void vsync(void) {
	wait_vsync();
}

void wait(int j) {
	for (i = 0; i < j; ++i)
		vsync();
}

void cls(void) {
	int i,j;
	for(j = 0l; j < 24; j++)
		for(i = 0; i < 80; ++i)
			put_chr8(i, j, ' ', 0);
}

void define_tiles(void) {
}

// 定数
#define MAP_W 10
#define MAP_H 10
#define MAX_PANELS 10
#define MAX_STAGES 3

// 複数ステージ
const char *levels[MAX_STAGES] = {
	"##########"
	"#P.......#"
	"#.###S...#"
	"#.S...B..#"
	"#.###.####"
	"#.S......#"
	"#.###.####"
	"#........#"
	"#........#"
	"#####G####",

	"##########"
	"#P..S....#"
	"#.###....#"
	"#..B.S...#"
	"#.###.####"
	"#....S...#"
	"#.###.####"
	"#........#"
	"#....,...#"
	"#####G####",

	"##########"
	"#P.......#"
	"#.###S####"
	"#..B.....#"
	"#.### ####"
	"#........#"
	"#.### ####"
	"#....S...#"
	"#........#"
	"#####G####"
};

// グローバル変数
int player_x = -1, player_y = -1;
int gravity_x = -1, gravity_y = -1;
int panels_x[MAX_PANELS], panels_y[MAX_PANELS];
int panel_count = 0;
int goal_x, goal_y;

int player_hp = 20;
int player_atk = 5;
int exp = 0;
int level = 1;
int game_mode = 0;  // 0:パズル, 1:バトル
char battle_msg[40];
int current_stage = 0;

int enemy_hp, enemy_atk;
int old_player_x = -1, old_player_y = -1;
int old_gravity_x = -1, old_gravity_y = -1;
int old_panels_x[MAX_PANELS], old_panels_y[MAX_PANELS];

// 関数
void parse_map(void);
int can_move(int x, int y);
int try_move(int dx, int dy);
void gravity_fall(void);
void draw_background(void);
void update_objects(void);
void start_battle(void);
void update_battle(void);
void play_sound_effect(void);
void main2(void);

unsigned char keycode = 0;

unsigned char keyscan(void)
{
	keycode = 0;
	if(turbo){
		k0_old = k0;
		k1_old = k1;
		k2_old = k2;

		data_buf[0]=0xe3;
		get_key(data_buf, 4);

		k0 = data_buf[1];
		k1 = data_buf[2];
		k2 = data_buf[3];
		k3 = 0;

		data_buf[0]=0xe6;
		get_key(data_buf, 3);
		if((k0 == k0_old) && (k1 == k1_old) && (k2 == k2_old)){
			k3 = data_buf[2];
		}
	}else{
		data_buf[0]=0xe6;
		get_key(data_buf, 3);
		k3 = data_buf[2];
	}

	DI();
	outp(0x1c00,14);
	st = inp(0x1b00);
	EI();

	if((k1 & 0x10) || (k3 == 30) || (k3 == 56) || !(st & 0x01)){ /* 8 */
		keycode |= KEY_UP1;
	}
	if((k1 & 0x02) || (k3 == 28) || (k3 == 54) || !(st & 0x08)){ /* 6 */
		keycode |= KEY_RIGHT1;
	}
	if((k1 & 0x08) || (k3 == 31) || (k3 == 50) || !(st & 0x02)){ /* 2 */
		keycode |= KEY_DOWN1;
	}
	if((k1 & 0x40) || (k3 == 29) || (k3 == 52) || !(st & 0x04)){ /* 4 */
		keycode |= KEY_LEFT1;
	}
	if((k0 & 0x04) || (k2 & 0x02) || (k3 == 32)  || (k3 == 122) || !(st & 0x20)){ /* Z,SPACE */
		keycode |= KEY_A;
	}
	if((k0 & 0x02) || (k3 == 120) || !(st & 0x40)){ /* X */
		keycode |= KEY_B;
	}
	return keycode;
}
/*
char get_stick(char no)
{
	return 0;
}

char get_trigger(char no)
{
	return 0;
}
*/
void main2(void) {
	int i;
	for(i = 0; i < MAX_PANELS; ++i){
		old_panels_x[i] = old_panels_y[i] = -1;
	}
	cls();

	define_tiles();
	fill_vram(0x01, 32*24);  // 初回クリア（床で埋める）

	parse_map();
	draw_background();
	update_objects();

	while (1) {
		sprintf(battle_msg, "STAGE:%d", current_stage+1);
		print_at(0, 20, battle_msg);
		sprintf(battle_msg, "LEVEL:%d", level);
		print_at(0, 21, battle_msg);
		sprintf(battle_msg, "HP:%d", player_hp);
		print_at(0, 22, battle_msg);

		if (game_mode == 0) {
			gravity_fall();

			keycode = keyscan();
			if (keycode & KEY_A) {
				print_at(0, 23, "Give Up");
				wait(60);
				cls();
				parse_map();
				draw_background();
				update_objects();
			}
			else if (keycode && ((rand() % 100) < 5)) {
				start_battle();
			}else{
				if ((keycode & KEY_RIGHT1) && try_move(1, 0)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode & KEY_LEFT1) && try_move(-1, 0)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode & KEY_DOWN1) && try_move(0, 1)) { wait(4); update_objects(); play_sound_effect(); }
				if ((keycode & KEY_UP1) && try_move(0, -1)) { wait(4); update_objects(); play_sound_effect(); }
			}
		} else {
			update_battle();
		}
		vsync();
	}
}

void parse_map(void) {
	static int x, y;
	panel_count = 0;
	const char *current_level = levels[current_stage];
	for (y = 0; y < MAP_H; y++) {
		for (x = 0; x < MAP_W; x++) {
			char c = current_level[y * MAP_W + x];
			if (c == 'P') { player_x = x; player_y = y; }
			if (c == 'B') { gravity_x = x; gravity_y = y; }
			if (c == 'S') {
				if (panel_count < MAX_PANELS) {
					panels_x[panel_count] = x;
					panels_y[panel_count] = y;
					panel_count++;
				}
			}
			if (c == 'G') { goal_x = x; goal_y = y; }
		}
	}
}

int can_move(int x, int y) {
	if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return 0;
	if (levels[current_stage][y * MAP_W + x] == '#') return 0;
	if (gravity_x == x && gravity_y == y) return 0;

	static int i;
	for (i = 0; i < panel_count; i++) {
		if (panels_x[i] == x && panels_y[i] == y) return 0;
	}
	return 1;
}

int try_move(int dx, int dy) {
	static int nx, ny;
	nx = player_x + dx;
	ny = player_y + dy;

	if (gravity_x == nx && gravity_y == ny) {
		int gnx = nx + dx;
		int gny = ny + dy;
		if (can_move(gnx, gny)) {
			gravity_x = gnx;
			gravity_y = gny;
//			player_x = nx; player_y = ny;
			return 1;
		}
		return 0;
	}

	static int i;
	for (i = 0; i < panel_count; i++) {
		if (panels_x[i] == nx && panels_y[i] == ny) {
			int pnx = nx + dx;
			int pny = ny + dy;
			if (can_move(pnx, pny)) {
				panels_x[i] = pnx;
				panels_y[i] = pny;
//				player_x = nx; player_y = ny;
				return 1;
			}
			return 0;
		}
	}

	if (can_move(nx, ny)) {
		player_x = nx; player_y = ny;
		return 1;
	}
	return 0;
}

void gravity_fall(void) {
	if (gravity_x == -1) return;

	if (gravity_y + 1 < MAP_H && can_move(gravity_x, gravity_y + 1)) {
		gravity_y++;
		play_sound_effect();

		if (gravity_x == goal_x && gravity_y == goal_y) {
			update_objects();
			print_at(20, 10, "STAGE CLEAR!!");
			wait(60);
			cls();
			player_hp = 20 + 5 * level;
			current_stage++;
			if (current_stage >= MAX_STAGES) current_stage = 0;  // ループ
			parse_map();
			draw_background();
		}
			update_objects();
	}
}

void draw_background(void) {
	static int x, y;
	for (y = 0; y < MAP_H; y++) {
		for (x = 0; x < MAP_W; x++) {
			char c = levels[current_stage][y * MAP_W + x];
			if (c == '#') put_chr16(x , y, 0);
			else if (c == 'G') put_chr16(x, y, 5);
			else put_chr16(x, y, 1);
		}
	}
}

void update_objects(void) {
	static int i;

	// 前の位置を床に戻す
	if (old_player_x >= 0) put_chr16(old_player_x , old_player_y, 1);
	if ((old_gravity_x != gravity_x) || (old_gravity_y != gravity_y)) put_chr16(old_gravity_x, old_gravity_y, 1);
	for (i = 0; i < panel_count; i++) {
		if ((old_panels_x[i] != panels_x[i]) || old_panels_y[i] != panels_y[i]) put_chr16(old_panels_x[i], old_panels_y[i], 1);
	}

	// 新しい位置に描画
	if (player_x >= 0)
		put_chr16(player_x, player_y, 2);  // プレイヤー

	if (gravity_x >= 0) put_chr16(gravity_x, gravity_y, 4);  // 重力パネル（B）

	for (i = 0; i < panel_count; i++) {
		put_chr16(panels_x[i], panels_y[i], 3);  // 通常パネル（S）
	}

	// 位置保存
	old_player_x = player_x; old_player_y = player_y;
	old_gravity_x = gravity_x; old_gravity_y = gravity_y;
	for (i = 0; i < panel_count; i++) {
		old_panels_x[i] = panels_x[i];
		old_panels_y[i] = panels_y[i];
	}
}

void start_battle(void) {
	game_mode = 1;
	enemy_hp = 10 + (level - 1) * 5;
	enemy_atk = 3 + (level - 1) * 2;
	strcpy(battle_msg, "Slime appeared!");
	cls();
	// スライム表示（中央上部に）
	put_chr16(8, 2, 6);  // スライムキャラコード6
	print_at(10, 8, battle_msg);
	print_at(10, 12, "Press SPACE to attack");
}

void update_battle(void) {
	if (keyscan() & KEY_A) {
		enemy_hp -= player_atk;
		if (enemy_hp <= 0) {
			exp += 10;
			if (exp >= level * 20) {
				level++;
				exp = 0;
				player_hp = 20 + 5 * level;
				player_atk += 2;
				print_at(10, 14, "Level Up!");
				wait(30);
			}
			cls();
			strcpy(battle_msg, "Enemy defeated!");
			print_at(10, 10, battle_msg);
			wait(60);
			cls();
			game_mode = 0;
			draw_background();
			update_objects();
		} else {
			player_hp -= enemy_atk;
			sprintf(battle_msg, "You took %d damage! HP:%d", enemy_atk, player_hp);
			cls();
			// スライム再描画（バトル継続時）
			put_chr16(8, 2, 6);
			print_at(10, 8, "Enemy defeated? No!");
			print_at(10, 10, battle_msg);
			if(player_hp > 0){
				print_at(10, 12, "Press SPACE to attack");
			}else{
				wait(60);
				cls();
				print_at(20, 10, "You dead!");
				player_hp = 20 + 5 * level;
				wait(60);
				cls();
				game_mode = 0;
				parse_map();
				draw_background();
				update_objects();
			}
		}
		play_sound_effect();
		wait(10);
	}
}

void play_sound_effect(void) {
	beep();  // シンプルなビープ音
}

int main(void)
{
	unsigned char zmode = 0;

	for(i = 0; i < 200; ++i){
		y_table[i] = ((i / 8) * 80 + (i & 7) * 0x800);
	}

	outp(0x1a03, 0x0c);	/* 80行 */
	set_lowreso();
	set_pcg(ds1_pcg);
	if(!(inp(0x1ff0) & 0x01)){
		set_hireso();
		outp(0x1fd0, 0x03);
	}

	/* Change Pallet */
	outp(0x1000, 0xa2);
	outp(0x1100, 0xc4);
	outp(0x1200, 0xf0);

	/* Priority */
	outp(0x1300, 0xfe);

	outp(0x1fb0, 0x90);	/* 多色モード */
	if(inp(0x1fb0) == 0x90){
		zmode = 1;
		outp(0x1fc5, 0x80);	/* グラフィックパレットアクセスON */
//		pal_allblack(0);
	}

	for(i = 0; i < X_SIZE - 2; ++i){
		for(j = 0; j < Y_SIZE - 2; ++j){
			vram_ofs = (OFS_X + i) * PARTS_X + (OFS_Y + j) * SIZE;
			pat_sub2();
		}
	}

	if(zmode){
		outp(0x1fc0, 0x01);	/* 多色モードプライオリティ */
//		outp(0x1fe0, 0);
	}

	k0 = 0;
	k1 = 0;
	k2 = 0;

/*	outp(#0x1fa0, 0x07);
	outp(#0x1fa0, 0xa0);
	if((inp(0x1fa0) ^ 0xa0)){
		turbo = 1;
	}*/
	turbo = check_turbo();
//	if(!turbo)
	set_key();

	main2();
	do{
#ifdef DEBUG

#endif
		if(fadeflag == 0){
			fadeflag = 1;
			if(inp(0x1fb0) == 0x90){
				fadeinblack(org_pal, 0, 3);
			}
		}

	}while(!(k2 & 0x80) && (k3 != 27));
//	printf("End.\n");

	if(zmode){
		fadeoutblack(org_pal, 0, 3);
//		outp(0x1fb0, 0x90);	/* 多色モード */
//		outp(0x1fc5, 0x80);	/* グラフィックパレットアクセスON */
//		pal_all(0, org_pal2);
		pal_all_text(0, org_pal2);
//		for(j = 1; j < 8; j++)
//			outp(0x1fb9 - 1 + j, 255);
		set_constrast(0, org_pal2, 0);

		outp(0x1fc5, 0x0);
		outp(0x1fb0, 0x0);
	}

	/* Pallet */
	outp(0x1000, 0xaa);
	outp(0x1100, 0xcc);
	outp(0x1200, 0xf0);

	/* Priority */
	outp(0x1300, 0x00);

	return NOERROR;
}

