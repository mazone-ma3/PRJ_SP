/* 簡易スプライトロ－ダ */
/* X68KのSP形式をロ－ドする。 */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>

#include "sp.h"
#include "subfunc.h"
#include "sp68_ld.h"

#define FILE_MAX 1 /* 10 */

FILE *stream[FILE_MAX];


int sp68_load(char *fil, short sprparts)
{
	long i, j, k;
	unsigned char data;
	unsigned char pattern[128];
	_Far unsigned char *spram_char;

	spram_char = (_Far unsigned char *)spram;

//	_FP_SEG(spram_char) = 0x130;
//	_FP_OFF(spram_char) = 0x4000;

	if ((stream[0] = fopen( fil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", fil);

		fclose(stream[0]);
		return 1;
	}
/*	printf("Loading file:%s\n" ,fil); */

	for (i = 0; i < sprparts; i++){
		fread(pattern, 1, 128, stream[0]);

/* スモ－ルエンディアンに直してブロック再配置 */
		for (j = 0; j < 16; j++){
			for(k=0;k<4;k++){
				data = pattern[j * 4 + k];
				*(spram_char++) = ((data >> 4) % 16) | ((data % 16) << 4);
			}
			for(k=0;k<4;k++){
				data = pattern[j * 4 + k + 16 * 4];
				*(spram_char++) = ((data >> 4) % 16) | ((data % 16) << 4);
			}
		}
	}
	fclose(stream[0]);

	return 0;
}

int pal68_load(char *fil)
{
	long i, j, k;
	unsigned short data;
	unsigned char pattern[2];
	_Far unsigned short *palram_char;

	_FP_SEG(palram_char) = 0x130;
	_FP_OFF(palram_char) = 0x2000;

	if ((stream[0] = fopen( fil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", fil);

		fclose(stream[0]);
		return 1;
	}
/*	printf("Loading file:%s\n" ,fil); */

	for (i = 0; i < 240; i++){
		k = fread(pattern, 1, 2, stream[0]);
		if(k < 1)
			break;
		data = pattern[1] + pattern[0] * 256;
		*(palram_char++) = (data >> 1) & 0x7fff;
	}
	fclose(stream[0]);

	return 0;
}

