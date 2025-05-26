/* MSX-SC5->PC-88VA FONT CONV. for GCC */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>

#include "FONTVA.h"

unsigned char pal[16][3] = {
	{  0,  0,  0},
	{  0,  0,  0},
	{  3, 13,  3},
	{  7, 15,  7},
	{  3,  3, 15},
	{  5,  7, 15},
	{ 11,  3,  3},
	{  5, 13, 15},
	{ 15,  3,  3},
	{ 15,  7,  7},
	{ 13, 13,  3},
	{ 13, 13,  7},
	{  3,  9,  3},
	{ 13,  5, 11},
	{ 11, 11, 11},
	{ 15, 15, 15},
};


#define MSXWIDTH 256
#define MSXLINE 212

FILE *fontstream[2];

unsigned char pattern[10];
unsigned char msxcolor[MSXWIDTH / 2][MSXLINE];

unsigned short fontdata[FONTSIZEX][FONTSIZEY][FONTPARTS];

unsigned short font_load(char *loadfil)
{
	long i, j,k,y, x, xx, yy, no, max_xx;
//	unsigned short *fontram = fontdata;

	if ((fontstream[0] = fopen( loadfil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", loadfil);

		fclose(fontstream[0]);
		return 1;
	}

	fread(pattern, 1, 1, fontstream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, fontstream[0]);	/* MSXヘッダも読み捨てる */
	fread(pattern, 1, 2, fontstream[0]);	/* MSXヘッダを読み捨てる */

	for(y = 0; y < MSXLINE; ++y){
		for(x = 0; x < MSXWIDTH / 2 ; ++x){
			msxcolor[x][y] = 0;
		}
	}
	for(y = 0; y < MSXLINE; ++y){
		for(x = 0; x < MSXWIDTH / 8; ++x){
			i = fread(pattern, 1, 4, fontstream[0]);	/* 8dot分 */
			if(i < 1)
				break;

			/* 色分解 */
			msxcolor[0 + x * 4][y] = pattern[0]; 
			msxcolor[1 + x * 4][y] = pattern[1]; 
			msxcolor[2 + x * 4][y] = pattern[2];
			msxcolor[3 + x * 4][y] = pattern[3];
		}
	}
	fclose(fontstream[0]);
	max_xx = 64;

	j = 0;
	xx=0;
	yy=0;
	x=0;
	for(no = 0; no < FONTPARTS; ++no){
//		printf("\nno =%d ",no);
		for(y = 0; y < FONTSIZEY; ++y){
			for(x = 0; x < FONTSIZEX; x+=1){

				if((x+xx) >= max_xx) {
					xx=0;
					yy+=FONTSIZEY;
				}

//				*(fontram++) = 
				fontdata[x][y][no]=msxcolor[x + xx][y + yy]; // * 256 + msxcolor[x + xx + 1][y + yy];
			}
		}
		xx+=FONTSIZEX;
	}
	return 0;
}

