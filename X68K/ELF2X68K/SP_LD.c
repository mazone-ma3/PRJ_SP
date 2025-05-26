/* 簡易スプライトロ－ダ */
/* SP形式をロ－ドする。 */

#include <stdio.h>
#include <stdlib.h>
//#include <sys\dos.h>
//#include <conio.h>

#include "sp.h"
#include "sp_ld.h"

#define FILE_MAX 1 /* 10 */

FILE *sp_stream[FILE_MAX];


int sp68_load(char *fil, short offset, short sprparts)
{
	long i, j, k;
	unsigned short data;
	unsigned char pattern[128];

	unsigned short *spram;
	spram  = (unsigned short *)0xeb8000;
	spram += ((64 * offset));

	if ((sp_stream[0] = fopen( fil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", fil);

		fclose(sp_stream[0]);
		return 1;
	}
/*	printf("Loading file:%s\n" ,fil);*/
/* 512 = 128 * 4 */
	for (i = 0; i < sprparts; i++){
		fread(pattern, 1, 128, sp_stream[0]);

		for (j = 0; j < 64; j++){
			data = pattern[j * 2] * 256 + pattern[j * 2 + 1];
			*(spram++) = data;
		}
	}
	fclose(sp_stream[0]);

	return 0;
}

int pal68_load(char *fil)
{
	long i, j, k;
	unsigned short data;
	unsigned char pattern[2];

	unsigned short *palram_char;
	palram_char  = (unsigned short *)0xe82200;

	if ((sp_stream[0] = fopen( fil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", fil);

		fclose(sp_stream[0]);
		return 1;
	}
/*	printf("Loading file:%s\n" ,fil);*/

	for (i = 0; i < 240; i++){
		k = fread(pattern, 1, 2, sp_stream[0]);
		if(k < 1)
			break;
		data = pattern[0] * 256 + pattern[1];
		*(palram_char++) = data;
	}
	fclose(sp_stream[0]);

	return 0;
}

