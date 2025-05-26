/* 初期化と終了処理 */

//#define _BORLANDC_SOURCE


#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <i86.h>
#include <fcntl.h>
#include <dos.h>
#include <unistd.h>

//typedef __far char __far char

#include "sp.h"
//#include "sp_sht.h"
//#include "spr_def.h"
//#include "sp_com.h"
/*#include "spmake.h"*/
//#include "sp68_ld.h"
#include "sp_main.h"
//#include "subfunc.h"

#include "sp_init.h"

#include "FONTVA.h"

/************************************************************************/
/*	\	BIT操作マクロ定義												*/
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

//#define ON 1
//#define OFF 0
//#define ERROR -1
//#define NOERROR 0

#define MAXCOLOR 16
#define ON 1
#define OFF 0

void paint(unsigned char color);
void paint2(unsigned char color);
void paint3(unsigned char color);
void erase_allsprite(void);

unsigned char org_pal[16][3] = {
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

unsigned short rev_adr = 0;


void g_init(void)
{
	union REGS reg;
	union REGS reg_out;

	reg.h.ah = 0x2a;
	int86(0x83, &reg, &reg_out);	/* テキスト初期化 */

	reg.h.ah = 0x00;
	reg.x.bx = 0xe00e;
	reg.h.cl=4;
	reg.h.ch=4;
	int86(0x8f, &reg, &reg_out);	/* グラフィックBIOS初期化 */

//	reg.h.ah = 0x00;
//	reg.h.al=0;
//	reg.x.dx=SPR_ATR;
//	int86(0x84, &reg, &reg_out);	/* スプライトBIOS初期化 */
//	reg.h.ah = 0x16;
//	int86(0x84, &reg, &reg_out);	/* スプライトBIOSスリープ */

//	outpw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
//	outpw(0x100, 0xb020);	/* none-interless Graphic-on 200dot(200line) */
//	outpw(0x100, 0xb062);	/* none-interless Graphic-on 400dot(200line) */

	outpw(0x100, 0xbc62);	/* none-interless Graphic-on 400dot(200line) */
								/* 画面ON single-plane 2画面 */
//	outpw(0x100, 0xb021);

//	outpw(0x102, 0x0101);	/* graphic0/1 Width640 4dot/pixel */
// 	outpw(0x102, 0x1111);	/* graphic0/1 Width320 4dot/pixel */

//	outp(0x153, 0x54);		/* G-VRAM選択 *
//	outp(0x153, 0x51);		/* T-VRAM選択 */

//	outpw(0x106, 0xab89);	/* パレット指定画面割当て指定 */
//	outpw(0x106, 0xa89b);	/* パレット指定画面割当て指定 */
	outpw(0x106, 0xa89b);	/* パレット指定画面割当て指定 */
	outpw(0x108, 0x0000);	/* 直接色指定画面割当て設定 */
	outpw(0x110, 0x008f);	/* 4ビットピクセル */
	outpw(0x10a, 0);
//	outp(0x500, 0);	/* 独立アクセス */
//	outp(0x512, 0);	/* ブロック0 */
//	outp(0x516, 0);	/* 書き込みプレーン選択 */

	outp(0x580, 0x10);

//	outp(0x10d, 0x01);
//	outpw(0x10c, 0x0100);	/* カラーパレットモード (パレット0) */
//	outpw(0x10c, 0x01b0);	/* カラーパレットモード 混在/バレット1:GRP1 */
	outpw(0x10c, 0x01a0);	/* カラーパレットモード 混在/バレット1:GRP0 */

	outpw(0x124,0x00);	/* 透明色 GRP0 */
	outpw(0x126,0x01);	/* 透明色 GRP1 */
	outpw(0x12e,0x01);	/* 透明色 TEXT/SPRITE */
}

/*終了処理*/
void term()
{
/*	unsigned char color[16][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },
	{ 0, 0, 7 },
	{ 7, 0, 0 },
	{ 7, 0, 7 },
	{ 0, 7, 0 },
	{ 0, 7, 7 },
	{ 7, 7, 0 },
	{ 7, 7, 0 },
	{ 7, 7, 7 },};
*/
	union REGS reg;
	union REGS reg_out;

	erase_allsprite();

	paint(0);
	paint2(0);

//	pal_all(CHRPAL_NO, color);
//	pal_all(BGPAL_NO, color);

	reg.h.ah = 0x00;
	reg.x.bx = 0x2000;
	reg.h.cl=4;
	reg.h.ch=4;
	int86(0x8f, &reg, &reg_out);	/* グラフィックBIOS初期化 */

	reg.h.ah = 0x0a;
	int86(0x8f, &reg, &reg_out);	/* パレット初期化 */

// 	outpw(0x100, 0xb020);	/* none-interless Graphic-on 400dot(400line) */
// 	outpw(0x102, 0x0101);	/* graphic0 Width640 4dot/pixel */
//	TVRAM_ON();
	outpw(0x106, 0xab89);	/* パレット指定画面割当て指定 */
	outpw(0x10c, 0x0180);	/* カラーパレットモード */
//	outpw(0x10c, 0x0110);	/* カラーパレットモード */

	reg.h.ah = 0x2a;
	int86(0x83, &reg, &reg_out);	/* テキスト初期化 */

	_disable();
	reg.h.ah = 0x0c;
	int86(0x82, &reg, &reg_out);	/* キーキュー初期化 */
	_enable();
}

unsigned short __far *vram2;

void paint(unsigned char color)
{
	unsigned short i, j;

	GVRAM_ON();
	vram2 = (unsigned short __far *)_MK_FP(0xa000, 0);

	for (i = 0; i < ((80 * 408L)); ++i){
		*(vram2 + i) = color + color *256;
	}
}

void paint2(unsigned char color)
{
	unsigned short i, j;

	GVRAM_ON();
//	unsigned short __far *vram;
	vram2 = (unsigned short __far *)_MK_FP(0xc000, 0);

	for (i = 0; i < ((80 * 408L)); ++i){
		*(vram2 + i) = color + color *256;
	}
}

void paint3(unsigned char color)
{
	unsigned short i, j;

	GVRAM_ON();
//	unsigned short __far *vram;
	vram2 = (unsigned short __far *)
		MK_FP(0xc000, 0);

	for (i = ((80 * (100L))); i < ((80 * (204L))); ++i){
		*(vram2 + i) = color + color *256;
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(unsigned short type)
{
	if(type & 1){
		paint(0x0);
		paint2(0x0);
	}

//	if(type & 2)
//		printf("\x1b*");
}

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	switch(pal_no){
		case CHRPAL_NO:
			outpw(0x300 + color * 2, (unsigned short)green * 4096 + red * 64 + blue * 2);
			break;
		case BGPAL_NO:
			outpw(0x320 + color * 2, (unsigned short)green * 4096 + red * 64 + blue * 2);
			break;
	}
}

void pal_all(unsigned char pal_no, unsigned char color[16][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0], color[i][1], color[i][2]);
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}


//パレットデータを反転し、指定したパレット番号にセットする。
void set_pal_reverse(int pal_no, unsigned char pal[16][3]) //WORD __far p[16])
{
	int i, j, k;
	int temp_pal[3];

	for(i = 0; i < 16; i++){
		temp_pal[0] = ~pal[i][0];
		temp_pal[1] = ~pal[i][1];
		temp_pal[2] = ~pal[i][2];
		pal_set(pal_no, i, temp_pal[0], temp_pal[1], temp_pal[2]);
	}
}

//value < 0 黒に近づける。
//value = 0 設定した色
//value > 0 白に近づける。
void set_constrast(int value, unsigned char org_pal[16][3], int pal_no)
{
	int j, k;
	int pal[3];


	for(j = 0; j < 16; j++){
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
		pal_set(pal_no, j, pal[0], pal[1], pal[2]);
	}
}

//wait値の速度で黒からフェードインする。
void fadeinblack(unsigned char org_pal[16][3], int pal_no, int wait)
{
	int j;

	for(j = -15; j <= 0; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait値の速度で黒にフェードアウトする。
void fadeoutblack(unsigned char org_pal[16][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j != -16; j--){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait値の速度で白にフェードアウトする。
void fadeoutwhite(unsigned char org_pal[16][3], int pal_no, int wait)
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
	char j;
	for(j = 0; j < 16; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

void erase_allsprite(void)
{
	unsigned char i;
	TVRAM_ON();
//	unsigned short __far  *spram;
	spram = (unsigned short __far *)
		MK_FP(0xa000, SPR_ATR);
	for(i = 0; i < MAX_SPRITE; ++i){
		*(spram++) = 0;
		*(spram++) = 0;
		*(spram++) = 0;
		*(spram++) = 0;
//		set_sprite_locate(i, 0, 0, 0,(SCREEN_MAX_Y), 0, 0);
	}
}

//extern void put_strings(int scr, int y, int x,  char *str, char pal);

unsigned char __far *p0, __far *p1, __far *p2, __far *p3;
//unsigned char __far *p0, *p1, *p2, *p3;
unsigned char __far *pp0, __far *pp1, __far *pp2;

int check_bgm(void)
{
//	char str[4];
	p0 = (unsigned char __far *)
//		(unsigned char __far*)
		MK_FP(0, (0x60 * 4));
	p1 = (unsigned char __far *)
//		(unsigned char __far*)
		MK_FP(0, (0x60 * 4 + 1));
	p2 = (unsigned char __far *)
//		(unsigned char *)
		MK_FP(0, (0x60 * 4 + 2));
	p3 = (unsigned char __far *)
//		(unsigned char *)
		MK_FP(0, (0x60 * 4 + 3));
	pp0 = (unsigned char __far *)
//		(unsigned char *)
		MK_FP( *p2 + *p3 * 256, *p0 + *p1 * 256 + 2);
	pp1 = (unsigned char __far *)
//		(unsigned char *)
		MK_FP( *p2 + *p3 * 256, *p0 + *p1 * 256 + 3);
	pp2 = (unsigned char __far *)
//		(unsigned char *)
		MK_FP( *p2 + *p3 * 256, *p0 + *p1 * 256 + 4);

//	printf("\n\n%x%x:%x%x\n", *p2, *p3, *p0, *p1);
//	printf("%x\n", pp0);
//	printf("%c%c%c\n", *(pp0), *(pp1), *(pp2));
//	str[0] = *pp0;
//	str[1] = *pp1;
//	str[2] = *pp2;
//	str[3] = '\0';
//
//	put_strings(0, 5, 2, str, CHRPAL_NO);

	if(*pp0 == 'P')
		if(*pp1 == 'M')
			if(*pp2 == 'D')
				return NOERROR;

	return ERROR;
}

void playbgm(unsigned char mode)
{
	union REGS reg;
	union REGS reg_out;

	if(check_bgm() == ERROR)
		return;

	reg.h.ah = mode;
	if(mode == 2)
		reg.h.al = 16;

	int86(0x60, &reg, &reg_out);	/* 再生 */
}

/*メインルーチン
　初期設定とメインループ*/
void main()
{
	short i, j;
	short errlv;

//	playbgm(2);

	g_init();
	paint(0);
	paint2(0);
//	pal_all(CHRPAL_NO, org_pal);

	/* TSP coommand */
	while(inp(0x142) & 0x05);
	outp(0x142, 0x82);	/* SPRON スプライトON */
	while(inp(0x142) & 0x05);
	outp(0x146, (SPR_ATR) / 256);	/* スプライト制御テーブルアドレス上位 */
	while(inp(0x142) & 0x05);
	outp(0x146, 0x00);
	while(inp(0x142) & 0x05);
	outp(0x146, 32 * 4 | 0x02)	/* 横32枚 縦方向2倍 */;


	while(inp(0x142) & 0x05);
	outp(0x142, 0x15);	/* CURDEF */
	while(inp(0x142) & 0x05);
	outp(0x146, 0x02);	/* カーソルOFF */

	GVRAM_ON();

	font_load("FONTYOKO.SC5");

	rev_adr = read_sprite();


	_disable();
	outp(0x44, 0x07);
	outp(0x45, 0x00);
	_enable();

//	init_v_sync();
	scrl_spd = SCRL_MIN;
	scrl = 0;

/* ゲ－ムを実行 */

	erase_allsprite();
	do{
		wait_vsync();
		pal_allblack(CHRPAL_NO);
		pal_allblack(BGPAL_NO);
		paint(0x0);
		init_star();
		paint2(0x0);
		wait_vsync();
		pal_all(BGPAL_NO, org_pal);
		if(((errlv = title_demo()) == SYSERR) || (errlv == SYSEXIT))
			break;
		else if(errlv == NOERROR){
			for(j = 0; j != -16 * 8; j--){
				wait_vsync();
				set_constrast(j / 8, org_pal, BGPAL_NO);
				set_constrast(j / 8, org_pal, CHRPAL_NO);
				bg_roll();
			}
			wait_vsync();
			pal_allblack(BGPAL_NO);
			TVRAM_ON();
			set_sprite_locate(0, 0, 0, 0,(SCREEN_MAX_Y), 0, 0);
			paint(0x0);
			paint2(0x0);
			opening_demo();
			paint2(0x0);

		}else if(errlv >= ERRLV1){
//			errlv -= ERRLV1;
//			screen_fill_char(SCREEN2, 0, 0, 26, 32, 0);
			paint3(0x0);
//			playbgm(errlv, debugmode);
//			playbgm(0, debugmode);
			playbgm(0);
			for(i = 0; i < 320 - 32; i += 8){
//				sprallmove(0, 16 * 8, (256 + 8 - i) & 0xff, grptable[TITLEFILE].width, grptable[TITLEFILE].height);
				wait_vsync();
				TVRAM_ON();
				set_sprite_locate(0, title_index, 0, 240 - 32 - 16 - i*2, 16, 256, 64);
				bg_roll();
//				sprallmove(0, 16 * 8, 8 + i, grptable[TITLEFILE].width, grptable[TITLEFILE].height);
				wait_vsync();
				TVRAM_ON();
				set_sprite_locate(0, title_index, 0, 240 - 32 - 16 + i*2, 16, 256, 64);
				bg_roll();
			}
			erase_allsprite();
//			if((errlv = game_run(errlv)) == SYSEXIT)
			errlv = game_run(errlv);
//			S_IL_FUNC(bgm_fadeout());
			playbgm(2);
			if(errlv == SYSEXIT)
				break;
			if(errlv != NOERROR){
				for(j = 0; j < 16 * 8; j++){
					wait_vsync();
					set_constrast(j / 8, org_pal, BGPAL_NO);
					set_constrast(j / 8, org_pal, CHRPAL_NO);
					set_sprite();
					bg_roll();
				}
//				fadeoutwhite(org_pal, CHRPAL_NO, 10);
			}
		}
		wait_vsync();
		erase_allsprite();
		wait_vsync();
	}while((inp(0x09) & 0x80));
//	}
//	game_run();

//	term_v_sync();
	playbgm(2);

	pal_allblack(CHRPAL_NO);
	pal_allblack(BGPAL_NO);

	term();
}
