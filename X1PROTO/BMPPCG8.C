/* Bitmap(8 colors) to X1 PCG Converter By m@3 */
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

#define X1_WIDTH 256
#define X1_HEIGHT 64

#define PCGSIZEX 1
#define PCGSIZEY 8
#define PLANE 3
#define PCGPARTS 256

FILE *stream[2];

//unsigned char conv_tbl[16] = { 0, 0, 4, 4, 1, 1, 2, 5, 2, 2, 6, 6 ,4 ,3 ,7 ,7 };
unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0 ,0 ,0 ,0 ,0 };

long i, j, k, l, m, write_size, header;
long y, x, xx, yy, no, max_xx;
long width, height, filesize, headersize, datasize;
unsigned char color;

unsigned char read_pattern[X1_WIDTH * X1_HEIGHT + 2];
unsigned char write_pattern[PCGSIZEX*PCGSIZEY*PCGPARTS*PLANE+1];
unsigned char pattern[10+1];

unsigned char grp_buffer[X1_WIDTH][X1_HEIGHT];
unsigned char X1_buffer[X1_WIDTH / 8][X1_HEIGHT][PLANE];

int conv(int arg, char *bitmapfil, char *savefil)
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
	if((width > X1_WIDTH) || (height > (X1_HEIGHT * 2))){
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
//			write_pattern[i + (height - j - 1) * (X1_WIDTH / 2)] = read_pattern[k++];
				grp_buffer[i * 2 + 0][height / 2 - 1 - (j / 2)] = conv_tbl[read_pattern[k] % 16];
				grp_buffer[i * 2 + 1][height / 2 - 1 - (j / 2)] = conv_tbl[read_pattern[k] / 16];
			}
			k++;
		}
	}
/*
	max_xx = X1_HEIGHT;
	for(j = 0; j < X1_HEIGHT; ++j){
		for(i = 0; i < (X1_WIDTH / 8); ++i){
			for(l = 0; l < 8; ++l){
				for(m = 0; m < PLANE; ++m){
					if(BITTST(m, grp_buffer[i * 8 + l][j])){
						BITSET(7 - l, X1_buffer[i][j][m]);
					}else{
						BITCLR(7 - l, X1_buffer[i][j][m]);
					}
				}
			}
		}
	}*/

	max_xx =0 ;
	for(y = 0; y < X1_HEIGHT; ++y){
		xx = 0;
		j = 0;
		for(x = 0; x < X1_WIDTH; ++x){
			color = grp_buffer[x][y];

			for(i = 0; i < PLANE; ++i){
				if(BITTST(i, color)){
					BITSET(7-j, X1_buffer[xx][y][i]);
				}else{
					BITCLR(7-j, X1_buffer[xx][y][i]);
				}
			}

			++j;
			if(j == 8){
				++xx;
				j = 0;
				if(max_xx < x){
					max_xx = xx;
//					printf("max width = %d \n",max_xx);
				}
			}
		}
	}
	printf("max width = %d \n",max_xx);


///////////////////////////////////////////////////////////////////////////////

	if ((stream[1] = fopen( savefil, "wb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", savefil);

		fclose(stream[1]);
		return ERROR;
	}

	if(arg < 4){
		printf("normal PCG mode.\n");
		j = 0;
//		for(i = 0; i < PLANE; ++i){		/* 3 plane */
			xx=0;
			yy=0;
			for(no = 0; no < PCGPARTS; ++no){	/* 256 blocks */
				printf("\nno =%d ",no);


				for(y = 0; y < PCGSIZEY; ++y){			/* 1*8dot*8line pcg */
					for(x = 0; x < PCGSIZEX ; ++x){

						if((x+xx) >= max_xx) { //(X1_WIDTH)){
							xx=0;
							yy+=PCGSIZEY;
						}

						printf("x=%d y=%d ",x+xx, y+yy);

						for(i = 0; i < PLANE; ++i){		/* 3 plane */

						write_pattern[j++] = X1_buffer[x + xx][y + yy][i];
//						printf("%dbytes\n",++j);
					}

				}
			}
			xx+=PCGSIZEX;
		}
//	}
	}else{
		printf("turbo PCG mode.\n");
		j = 0;
		for(i = 0; i < PLANE; ++i){		/* 3 plane */
			xx=0;
			yy=0;
			for(no = 0; no < PCGPARTS; ++no){	/* 256 blocks */
				printf("\nno =%d ",no);


				for(y = 0; y < PCGSIZEY; ++y){			/* 1*8dot*8line pcg */
					for(x = 0; x < PCGSIZEX ; ++x){

						if((x+xx) >= max_xx) { //(X1_WIDTH)){
							xx=0;
							yy+=PCGSIZEY;
						}

						printf("x=%d y=%d ",x+xx, y+yy);

//						for(i = 0; i < PLANE; ++i){		/* 3 plane */

							write_pattern[j++] = X1_buffer[x + xx][y + yy][i];
//							printf("%dbytes\n",++j);
//						}

					}
				}
				xx+=PCGSIZEX;
			}
		}
	}

	printf("%d Bytes.\n",j);
	i = fwrite(write_pattern, 1, j, stream[1]);	/* 8dot分*2 */
/*	if(i < 1){
		break;
	}*/
	fclose(stream[1]);

	return NOERROR;
}


int	main(int argc,char **argv){

	if (argc < 3){
		printf("Bitmap 8colors to X1 PCG file Converter.\n");
		return ERROR;
	}

	if(conv(argc, argv[1], argv[2]))
		return ERROR;

	return NOERROR;
}
