/* Bitmap(8 colors) to PC-88 .grp Converter By m@3 */
/* (デジタル8色) */
/* gcc、clang等でコンパイルして下さい */

#include <stdio.h>
#include <stdlib.h>

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

#define ERROR 1
#define NOERROR 0

#define PC88_WIDTH 256
#define PC88_HEIGHT 80

#define RAM_ADR 0x2000


#define PARTS_X 2
#define PARTS_Y 8


/* 黒をシアンで出力して、PC88でシアンを黒に設定する(マスクの代わり) */
/* 1色減る代わりにORを取る必要が無い */
unsigned char cvPC88_tbl[16] = { 3, 1, 2, 0, 4, 5, 6, 7, 0, 0, 0, 0 ,0 ,0 ,0 ,0 };

//unsigned char conv_tbl[16] = { 0, 0, 4, 4, 1, 1, 2, 5, 2, 2, 6, 6 ,4 ,3 ,7 ,7 };
unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0 ,0 ,0 ,0 ,0 };

//unsigned char mask_tbl[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7 ,7 ,7 ,7 ,7 };
unsigned char mask_tbl[16] = { 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0 ,0 ,0 ,0 ,0 };

FILE *stream[2];

long i, j, k, l, m, write_size, header;
long width, height, filesize, headersize, datasize;

unsigned char read_pattern[PC88_WIDTH * PC88_HEIGHT + 2];
unsigned char write_pattern[PC88_WIDTH * PC88_HEIGHT + 2];
unsigned char write_pattern2[PC88_WIDTH * PC88_HEIGHT + 2];
unsigned char pattern[10+1];

unsigned char grp_buffer[PC88_WIDTH][PC88_HEIGHT];
unsigned char PC88_buffer[PC88_WIDTH / 8][PC88_HEIGHT][3];

void  set_mapchr(unsigned char *src, unsigned char *dst)
{
	int i;
	for(i = 0; i < 8; ++i){
		*dst = *src;	// b1
		src++;
		dst += 2;
		*dst = *src;	// r1
		src++;
		dst += 2;
		*dst = *src;	// g1
		src++;
		dst -= 3;

		*dst = *src;	// b2
		src++;
		dst += 2;
		*dst = *src;	// r2
		src++;
		dst += 2;
		*dst = *src;	// g2
		dst++;

		src += 91;		// ((256 / 8 - 2) * 3 + 1)
	}
}



int conv(int arg, char *bitmapfil, char *grpfil, char *maskfil)
{
	if ((stream[0] = fopen( bitmapfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", bitmapfil);
		return ERROR;
	}

	fread(read_pattern, 1, 2, stream[0]);	/* Bitmapヘッダ */
	if((read_pattern[0] != 'B') || (read_pattern[1] != 'M')){
		printf("Not Bitmap file %s.", bitmapfil);
		fclose(stream[0]);
		return ERROR;
	}
	fread(read_pattern, 1, 4, stream[0]);	/* ファイルサイズ */
	filesize = read_pattern[0x0] + 256 * (read_pattern[0x1] + 256 * (read_pattern[0x2]  +  256 * read_pattern[0x3]));

	fread(read_pattern, 1, 4, stream[0]);	/* 予約 */
	fread(read_pattern, 1, 4, stream[0]);	/* 画像データの開始番地 0x76*/
	fread(read_pattern, 1, 4, stream[0]);	/* ヘッダサイズbyte 40(0x28) */
	headersize = read_pattern[0x0] + 256 * (read_pattern[0x1] + 256 * (read_pattern[0x2]  +  256 * read_pattern[0x3]));

	fread(read_pattern, 1, 4, stream[0]);	/* 横幅 */
	width = read_pattern[0x0] + 256 * (read_pattern[0x1] + 256 * (read_pattern[0x2]  +  256 * read_pattern[0x3]));
	fread(read_pattern, 1, 4, stream[0]);	/* 縦幅 */
	height = read_pattern[0x0] + 256 * (read_pattern[0x1] + 256 * (read_pattern[0x2]  +  256 * read_pattern[0x3]));
	if((width > PC88_WIDTH) || (height > (PC88_HEIGHT * 2))){
		fclose(stream[0]);
		printf("Size over file %s.", bitmapfil);
		return ERROR;
	}else{
		printf("X Size %d / Y Size %d", width, height);
	}

	fread(read_pattern, 1, 2, stream[0]);	/* 面の数 1 */
	fread(read_pattern, 1, 2, stream[0]);	/* 1ピクセル当たりのビット数 4 */
	if((read_pattern[0x0] != 0x04) || (read_pattern[0x1] != 0x00)){
		printf("Not 16colors file %s.", bitmapfil);
		fclose(stream[0]);
		return ERROR;
	}

	fread(read_pattern, 1, 4, stream[0]);	/* 圧縮形式 0 */
	fread(read_pattern, 1, 4, stream[0]);	/* 画像のデータサイズ */
//	datasize = read_pattern[0x0] + 256 * (read_pattern[0x1] + 256 * (read_pattern[0x2]  +  256 * read_pattern[0x3]));

	fread(read_pattern, 1, 4, stream[0]);	/* 1mあたりのピクセル数 横 */
	fread(read_pattern, 1, 4, stream[0]);	/* 1mあたりのピクセル数 縦 */

	fread(read_pattern, 1, 4, stream[0]);	/* カラーテーブル */
	fread(read_pattern, 1, 4, stream[0]);	/* カラーインデックス */
	fread(read_pattern, 1, 4*16, stream[0]);	/* カラーバレット x 16 */

	datasize = filesize - headersize - 4 - 4 - 16 * 4;
	width = (datasize / height) * 2;
	printf("\nDataSize %d / Data Width %d", datasize, width);

	fread(read_pattern, 1, width / 2 * height, stream[0]);	/* ピクセルデータ */

	fclose(stream[0]);

///////////////////////////////////////////////////////////////////////////////

	k = 0;
	for(j = 0; j < height; ++j){
		for(i = 0; i < (width / 2); ++i){
			if((j % 2)){
//			write_pattern[i + (height - j - 1) * (PC88_WIDTH / 2)] = read_pattern[k++];
				if(arg < 4){
					grp_buffer[i * 2 + 0][height / 2 - 1 - (j / 2)] = cvPC88_tbl[read_pattern[k] % 16];
					grp_buffer[i * 2 + 1][height / 2 - 1 - (j / 2)] = cvPC88_tbl[read_pattern[k] / 16];
				}else{
					grp_buffer[i * 2 + 0][height / 2 - 1 - (j / 2)] = conv_tbl[read_pattern[k] % 16];
					grp_buffer[i * 2 + 1][height / 2 - 1 - (j / 2)] = conv_tbl[read_pattern[k] / 16];
				}
			}
			k++;
		}
	}
	for(j = 0; j < PC88_HEIGHT; ++j){
		for(i = 0; i < (PC88_WIDTH / 8); ++i){
			for(l = 0; l < 8; ++l){
				for(m = 0; m < 3; ++m){
					if(BITTST(m, grp_buffer[i * 8 + l][j])){
						BITSET(7 - l, PC88_buffer[i][j][m]);
					}else{
						BITCLR(7 - l, PC88_buffer[i][j][m]);
					}
				}
			}
		}
	}
	k = 0;
	for(j = 0; j < PC88_HEIGHT; ++j){
		for(i = 0; i < (PC88_WIDTH / 8); ++i){
			for(m = 0; m < 3; ++m){
				write_pattern[k++] = PC88_buffer[i][j][m];
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////

	for(i = 0; i < 128; ++i){
		unsigned short pat_tmp = (i & 0x0f) * 2 + (i & 0xf0) * 16;
		set_mapchr(&write_pattern[pat_tmp * 3], &write_pattern2[i * PARTS_X * PARTS_Y * 3]);
	}

	if ((stream[1] = fopen( grpfil, "wb")) == NULL) {
		printf("Can\'t open file %s.", grpfil);
		return ERROR;
	}

	write_size = PC88_WIDTH / 8 * PC88_HEIGHT * 3;

	fwrite(write_pattern2, 1, write_size, stream[1]);

	fclose(stream[1]);

	if(arg < 4)
		return NOERROR;

///////////////////////////////////////////////////////////////////////////////

	k = 0;
	for(j = 0; j < height; ++j){
		for(i = 0; i < (width / 2); ++i){
			if((j % 2)){
//			write_pattern[i + (height - j - 1) * (PC88_WIDTH / 2)] = read_pattern[k++];
				grp_buffer[i * 2 + 0][height / 2 - 1 - (j / 2)] = mask_tbl[read_pattern[k] % 16];
				grp_buffer[i * 2 + 1][height / 2 - 1 - (j / 2)] = mask_tbl[read_pattern[k] / 16];
			}
			k++;
		}
	}
	m = 0;
	for(j = 0; j < PC88_HEIGHT; ++j){
		for(i = 0; i < (PC88_WIDTH / 8); ++i){
			for(l = 0; l < 8; ++l){
//				for(m = 0; m < 3; ++m){
					if(BITTST(m, grp_buffer[i * 8 + l][j])){
						BITSET(7 - l, PC88_buffer[i][j][m]);
					}else{
						BITCLR(7 - l, PC88_buffer[i][j][m]);
					}
//				}
			}
		}
	}
	k = 0;
	for(j = 0; j < PC88_HEIGHT; ++j){
		for(i = 0; i < (PC88_WIDTH / 8); ++i){
//			for(m = 0; m < 3; ++m){
				write_pattern[k++] = PC88_buffer[i][j][m];
//			}
		}
	}

///////////////////////////////////////////////////////////////////////////////

	if ((stream[1] = fopen( maskfil, "wb")) == NULL) {
		printf("Can\'t open file %s.", grpfil);
		return ERROR;
	}

	write_size = PC88_WIDTH / 8 * PC88_HEIGHT;

	fwrite(write_pattern, 1, write_size, stream[1]);

	fclose(stream[1]);

	return NOERROR;
}

int	main(int argc,char **argv)
{
	if (argc < 3){
		printf("Bitmap 8colors to PC-88 .grp file Converter.\n");
		return ERROR;
	}

//	if(argc < 4)
//		argv[3] = '\0';

	if(conv(argc, argv[1], argv[2], argv[3]))
		return ERROR;

	return NOERROR;
}
