/* CMOC BIN->FM-7 (PROG_ADR-) CONV. for GCC */
/* 先頭のスタック確保用コードをバイパスする */
/* CMOCで無ければデータ(DATA_ADR-)として変換 */
#include <stdio.h>

#define PROG_ADR 0x6000
#define DATA_ADR 0x0

FILE *stream[2];


int conv(char *loadfil, char*savefil)
{
	long i, size, header, header2;
	unsigned char pattern[65536];
	unsigned char pattern2[65536];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	if ((stream[1] = fopen( savefil, "wb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", savefil);

		fclose(stream[1]);
		return 1;
	}


	size = fread(pattern, 1, 65536, stream[0]);
	if(!(size < 1)){
		header = DATA_ADR;
		header2 = 0;	/* プログラムで無い場合 */
		if((pattern[6] == 0x17)){
			header2 = pattern[6+1]*256+pattern[6+2]+3;	//何も無ければ0x605c;
			if(header2 < (0x8000-PROG_ADR)){
				header = PROG_ADR;		/* プログラム(多分) */
				header2 += (PROG_ADR+6);
			}else{
				header2 = 0;
			}
		}
		printf("Start=%X Run=%X Size=%X",header, header2, size);

		pattern2[0] = 0;
		pattern2[1] = size / 256;
		pattern2[2] = size % 256;
		fwrite(pattern2, 1, 3, stream[1]);	/* サイズヘッダをつける */
		pattern2[0] = header / 256;
		pattern2[1] = header % 256;
		fwrite(pattern2, 1, 2, stream[1]);	/* ロードヘッダをつける */

		i = fwrite(pattern, 1, size, stream[1]);

		pattern2[0] = 0xff;
		fwrite(pattern2, 1, 1, stream[1]);
		pattern2[0] = 0x0;
		fwrite(pattern2, 1, 1, stream[1]);
		pattern2[0] = 0x0;
		fwrite(pattern2, 1, 1, stream[1]);

		pattern2[0] = header2 / 256;
		pattern2[1] = header2 % 256;
		fwrite(pattern2, 1, 2, stream[1]);	/* ロードヘッダをつける */
		pattern2[0] = 0x1A;
		fwrite(pattern2, 1, 1, stream[1]);	/* 終了マークをつける */

	}else{
		printf("Size Error.");
	}


	fclose(stream[0]);
	fclose(stream[1]);

	return 0;
}


int	main(int argc,char **argv){

	if (argv[1] == NULL)
		return 1;
	if (argv[2] == NULL)
		return 1;

	conv(argv[1], argv[2]);

	return 0;
}
