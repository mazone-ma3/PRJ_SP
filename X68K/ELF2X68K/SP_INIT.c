/* 初期化と終了処理 */

#include <stdint.h>
#include <stdio.h>
#include <x68k/iocs.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <x68k/dos.h>

//#define DEBUG

#include "sp_ld.h"
#include "play.h"
#include "subfunc.h"
#include "sp_main.h"
#include "font68.h"

#include "sp_init.h"

#include "sp.h"		/* 共通ヘッダ */

char *mcd_filename, *pcm_filename;
long mcd_status = 0;

#define MAX_MCD_SIZE 30000

unsigned char playbuffer[MAX_MCD_SIZE];

#define MAX_PCM_SIZE 400000

unsigned char pcmbuffer[MAX_PCM_SIZE];

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

/*パレット・セット*/
void pal_set(int pal_no, unsigned short color, unsigned short red, unsigned short green,
	unsigned short blue)
{
	unsigned short *pal_port;
	unsigned char mode = 0;
	if(color)
		mode = 1;

	green = ((green + 1)*2-1)*(green!=0);
	blue = ((blue + 1)*2-1)*(blue!=0);
	red = ((red + 1)*2-1)*(red!=0);

	switch(pal_no){
		case CHRPAL_NO:
			pal_port = (unsigned short *)(0xe82200); // + color * 2);
			*(pal_port+color) = (green * 32 * 32 + red * 32 + blue) * 2 + mode;
//			pal_port = (unsigned short *)(0xe82220); // + color * 2);
//			*(pal_port+color) = (green * 32 * 32 + red * 32 + blue) * 2 + mode;
			break;
		case BGPAL_NO:
			pal_port = (unsigned short *)(0xe82000); // + color * 2);
			*(pal_port+color) = (green * 32 * 32 + red * 32 + blue) * 2 + mode;
			break;
	}
}

void pal_all(int pal_no, unsigned char pal[16][3])
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(pal_no, i, pal[i][0], pal[i][1], pal[i][2]);
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}

//パレットデータを反転し、指定したパレット番号にセットする。
void set_pal_reverse(int pal_no, unsigned char pal[16][3]) //WORD far p[16])
{
	int i, j, k;
	unsigned char  temp_pal[3];

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
	unsigned char pal[3];

	for(j = 0; j < 16; j++){
		if(!((pal_no == CHRPAL_NO) && (j == 0))){
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
		set_constrast(j, org_pal, BGPAL_NO);
		set_constrast(j, org_pal, CHRPAL_NO);
	}
}

//パレットを暗転する。
void pal_allblack(int pal_no)
{
	char j;
	for(j = 0; j < 16; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

void paint_text(unsigned short color)
{
	unsigned short i, j;
	unsigned char *vram_adr = (unsigned char *)0xe00000; // * 2;

	for (i = 0; i < 512; ++i){
		for (j = 0; j < 0x80; ++j){
			*(vram_adr + j + i * 0x80 + 0x20000 * 0) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 1) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 2) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 3) = color; /* bit */;
		}
	}
}

void paint(unsigned short color)
{
	unsigned short i, j;

	for (i = 0; i < 256*2; ++i){
		for (j = 0; j < 256; ++j){
			*(vram + j + i * 512) = color; /* bit */;
		}
	}
}

void paint2(unsigned short color)
{
	unsigned short i, j;
	unsigned short *bgram = (unsigned short *)0xebe000; /* BG1 */
	for(j = 0; j < (256 / 8); j++){
		for(i = 0; i < 32; i++){
			*(bgram + (i * 2 + j * 0x80) / 2) = color;
		}
	}
}

void paint3(unsigned short color)
{
	unsigned short i, j;
	unsigned short *bgram = (unsigned short *)0xebe000; /* BG1 */
	for(j = 1; j < (256 / 8); j++){
		for(i = 0; i < 32; i++){
			*(bgram + (i * 2 + j * 0x80) / 2) = color;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1)
		paint(0x0);

	if(type & 2)
		printf("\x1b*");
}

char SNDBUFF[4][SND_BUFFSIZE];
long pcmsize[4]; 

long SND_load(char *fn, char*SNDBUFF){
	FILE *fp;
	long size;
	struct stat statBuf;


	if ((fp = fopen(fn,"rb")) == NULL)
		return 0; //NULL;
	
	fread(SNDBUFF, SND_BUFFSIZE, 1, fp);

	fclose(fp);

	if (stat(fn, &statBuf) == 0)
		return statBuf.st_size;

	return -1;
}

//extern long score;
extern void  __attribute__((interrupt)) int_vsync(void);

#ifdef DEBUG
/* 以下参考 xdev68kの割り込みサンプル(elf2x68kで通るように修正) */
/* 割り込み設定の保存用バッファ */
static volatile uint8_t s_mfpBackup[0x18] = {
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
};
static volatile uint32_t s_vector118Backup = 0;
static volatile uint32_t s_uspBackup = 0;
#endif

int init_int(void)
{
	int ret = 0;

	/* 割り込み off */
	asm volatile("	ori.w	#0x0700,%sr\n");

#ifdef DEBUG

	asm volatile (
		"AER		= 0x003\n"
		"IERA		= 0x007\n"
		"IERB		= 0x009\n"
		"ISRA		= 0x00F\n"
		"ISRB		= 0x011\n"
		"IMRA		= 0x013\n"

		"IMRB		= 0x015\n"

		"	lea.l	int_vsync,%a2\n"

		/* MFP のバックアップを取る */
		"	movea.l	#0x0e88000,%a0\n"			/* a0.l = MFPアドレス */
//		"	lea.l	s_mfpBackup(%pc),%a1\n"		/* a1.l = MFP保存先アドレス */
		"	lea.l	s_mfpBackup,%a1\n"		/* a1.l = MFP保存先アドレス */
		"	move.b	AER(%a0),AER(%a1)\n"		/*  AER 保存 */
		"	move.b	IERB(%a0),IERB(%a1)\n"		/* IERB 保存 */
		"	move.b	IMRB(%a0),IMRB(%a1)\n"		/* IMRB 保存 */
		"	move.l	#0x118,s_vector118Backup\n"	/* 変更前の V-disp ベクタ */

		/* V-DISP 割り込み設定 */
		"	move.l	%a2,0x118\n"				/* V-disp ベクタ書換え */
		"	bclr.b	#4,AER(%a0)\n"				/* 帰線期間と同時に割り込む */
		"	bset.b	#6,IMRB(%a0)\n"				/* マスクをはがす */
		"	bset.b	#6,IERB(%a0)\n"				/* 割り込み許可 */

//		:"=d"(rd0)
//		:"d"(rd0),"d"(rd1),"a"(ra0),"a"(ra1),"a"(ra2) //,"r"(rpc)
	);

#else
	ret = _iocs_vdispst (int_vsync, 0, 0*256+1);
#endif
	/* 割り込み on */
	asm volatile("	andi.w	#0xf8ff,%sr\n");

	return ret;
}


void reset_int(void)
{
asm volatile("	ori.w	#0x0700,%sr\n");

#ifdef DEBUG
	asm volatile(
		"AER		= 0x003\n"
		"IERA		= 0x007\n"
		"IERB		= 0x009\n"
		"ISRA		= 0x00F\n"
		"ISRB		= 0x011\n"
		"IMRA		= 0x013\n"
		"IMRB		= 0x015\n"

		/* MFP の設定を復帰 */
		"	movea.l	#0x0e88000,%a0\n"					/* a0.l = MFPアドレス */
//		"	lea.l	s_mfpBackup(%pc),%a1\n"			/* a1.l = MFPを保存しておいたアドレス */
		"	lea.l	s_mfpBackup,%a1\n"			/* a1.l = MFPを保存しておいたアドレス */

		"	move.b	AER(%a1),%d0\n"
//		"	andi.b	#%%0101_0000,%d0\n"
		"	andi.b	#0x50,%d0\n"
//		"	andi.b	#%%1010_1111,AER(%a0)\n"
		"	andi.b	#0xaf,AER(%a0)\n"
		"	or.b	%d0,AER(%a0)\n"					/* AER bit4&6 復帰 */

		"	move.b	IERB(%a1),%d0\n"
//		"	andi.b	#%%0100_0000,%d0\n"
		"	andi.b	#0x40,%d0\n"
//		"	andi.b	#%%1011_1111,IERB(%a0)\n"
		"	andi.b	#0xbf,IERB(%a0)\n"
		"	or.b	%d0,IERB(%a0)\n"					/* IERB bit6 復帰 */

		"	move.b	IMRB(%a1),%d0\n"
//		"	andi.b	#%%0100_0000,%d0\n"
		"	andi.b	#0x40,%d0\n"
//		"	andi.b	#%%1011_1111,IMRB(%a0)\n"
		"	andi.b	#0xbf,IMRB(%a0)\n"
		"	or.b	%d0,IMRB(%a0)\n"					/* IMRB bit6 復帰 */

		/* V-DISP 割り込み復帰 */
//		"	move.l	s_vector118Backup(%pc),0x118\n"
		"	move.l	s_vector118Backup,0x118\n"
//		:"=d"(rd0)
//		:"d"(rd0),"d"(rd1),"a"(ra0),"a"(ra1)
	);
#else
	_iocs_vdispst (0, 0, 0);
#endif

asm volatile("	andi.w	#0xf8ff,%sr\n");
}


int	main(int argc,char **argv){
//	unsigned short *crtcr20 = (unsigned short *)0xe80000;
//	unsigned short *crtc = (unsigned short *)0xe80028;
	long *lp;
	unsigned short *bg_priority = (unsigned short *)0xeb0808;
	unsigned short *grp_priority = (unsigned short *)0xe82500;
	unsigned short *bgram;
	short i, j;
	short errlv;
//	register unsigned short *scroll_grp_x = (unsigned short *)0xe8001c;	/* */
//	register unsigned short *scroll_grp_y = (unsigned short *)0xe8001e;	/* */

//	register unsigned short *scroll_bg0_x = (unsigned short *)0xeb0800;	/* */
//	register unsigned short *scroll_bg0_y = (unsigned short *)0xeb0802;	/* */

	register unsigned short *scroll_bg1_x = (unsigned short *)0xeb0806;	/* */
	register unsigned short *scroll_bg1_y = (unsigned short *)0xeb0804;	/* */

	register unsigned short *scroll_x = (unsigned short *)0xe80014;	/* */
	register unsigned short *scroll_y = (unsigned short *)0xe80016;	/* */

	long part1,part2;

dum:	_iocs_b_super(0);		/* スーパーバイザモード 最適化防止にラベルを付ける */

//	crtc = (short *)0xe80000;
//	crtcr20 = (short *)0xe80028;

/* 実行時引数が設定されているかどうか調べる */
/*	if (argc < 2 ) */
	/*	return 1;*/
	if (argv[1] == NULL)
		mcd_filename = "C_1_68.MDC";	/* 引数がなかった場合 */
	else
		mcd_filename = argv[1];

	mcd_status = check_mcd();
//	mcd_status = -1;
	if(mcd_status >= 0){
		if ((pcmsize[0] = SND_load("se1.pcm", &SNDBUFF[0][0])) < 1)
			exit(1);
		if ((pcmsize[1] = SND_load("se2.pcm", &SNDBUFF[1][0])) < 1)
			exit(1);
		if ((pcmsize[2] = SND_load("se3.pcm", &SNDBUFF[2][0])) < 1)
			exit(1);
		if ((pcmsize[3] = SND_load("se4.pcm", &SNDBUFF[3][0])) < 1)
			exit(1);
	}

//	pcm_play(&SNDBUFF[0][0], pcmsize[0]);

	grp_set();		/* ゲーム用に画面初期化 */
/*	screen(1,1,1,1);*/

/*	crtc[0] = 69;
	crtc[1] = 6;
	crtc[2] = 11; 
	crtc[3] = 59 ;
	crtc[4] = 567;
	crtc[5] = 5 ;
	crtc[6] = 40 ;
	crtc[7] = 552;
*/
/*	printf("%X\n",*crtcr20);
	*crtcr20 = 0x0110;*/
/*	*crtcr20 = 0x0111;*/
/*	printf("%X\n",*crtcr20);*/


	spr_set();		/* スプライトの定義 */
//	bgram = (unsigned short *)0xebc000; /* BG0 */
//	for(j = 0; j < 32; j++){
//		for(i = 0; i < 32; i++){
//			*(bgram + (i * 2 + j * 0x80) / 2) = 0x0a; //(i + j * 32) % 256;
//		}
//	}

	bgram = (unsigned short *)0xebe000; /* BG1 */
	for(j = 0; j < 32; j++){
		for(i = 0; i < 32; i++){
			*(bgram + (i * 2 + j * 0x80) / 2) = 0x0a; //(i + j * 32) % 256;
		}
	}

//	for(j = 0; j < 8; j++){
//		for(i = 0; i < 32; i++){
//			*(bgram + (i * 2 + j * 0x80) / 2) = (i + j * 32) % 256;
//		}
//	}

/*	printf("mode = %d\n",mcd_status);*/

	if(mcd_status >= 0){
		mcd_load(mcd_filename, playbuffer ,MAX_MCD_SIZE);
		lp = (long *)&playbuffer[0x1c];
		pcm_filename = (char *)&playbuffer[*lp];
		pcm_load(pcm_filename, pcmbuffer, MAX_PCM_SIZE); /* "TEST.PDX" */
	}

	font_load("FONTYOKO.SC5", 0, PCGPARTS);
	sp68_load("CORECRA.SP", CHR_TOP, SPRPARTS);
	pal68_load("CORECRA.PAL");

//	paint_text(0);
	pal_allblack(CHRPAL_NO);
	title_load("TITLE.SC5", (128-48-16 + 256) / 8, 48-16, 16 * 4);
//	pal_all(CHRPAL_NO, org_pal);

	*bg_priority = 0x021a;	/*  BG0=OFF BG1=ON */
//	*grp_priority = 0x1ce4;
	*grp_priority = 0x12e1;

	*scroll_bg1_x = 0;
	*scroll_bg1_y = 0;
//	*scroll_bg0_x = 0;
//	*scroll_bg0_y = 0;

/*	if(mcd_status >= 0){
		mcd_stop();
		mcd_play();
	}
*/
	spr_on(MAX_SPRITE);	/* スプライト表示の開始 */
//	game_run();		/* ゲームの実行 */
/* ゲ－ムを実行 */
	scrl_spd = SCRL_MIN;
	scrl = 0;

/* ゲ－ムを実行 */

	spr_clear();

	if(init_int())
		goto end;
	do{
		wait_vsync();
		pal_allblack(CHRPAL_NO);
		pal_allblack(BGPAL_NO);
		*scroll_x = 256; //(-(128-48-16) + 512) % 512;
		*scroll_y = 0; //(-48+16 + 512) % 512;
		vram =  (unsigned short *)0xc00000;
		paint(0x0);
		paint2(PCG_SPACE);
		init_star();
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
			*scroll_x = 512 + 256;
			pal_allblack(BGPAL_NO);
			vram =  (unsigned short *)0xc00000;
			paint(0x0);
			paint2(PCG_SPACE);
			opening_demo();
			paint2(PCG_SPACE);

		}else if(errlv >= ERRLV1){
//			errlv -= ERRLV1;
//			screen_fill_char(SCREEN2, 0, 0, 26, 32, 0);
			paint3(PCG_SPACE);
//			playbgm(errlv, debugmode);
//			playbgm(0, debugmode);
			if(mcd_status >= 0){
				mcd_stop();
				mcd_play();
			}
			for(i = 0; i < 192 + 16; i += 8){
				wait_vsync();
				*scroll_x = (1024 - i + 256) % 1024;
				score_displayall();
				bg_roll();
				wait_vsync();
				*scroll_x = i + 256;
				score_displayall();
				bg_roll();
			}
			spr_clear();
			errlv = game_run(errlv);
			if(mcd_status >= 0){
				mcd_fadeout();
/*				mcd_release();*/
			}
			if(errlv == SYSEXIT)
				break;
			if(errlv != NOERROR){
				for(j = 0; j < 16 * 8; j++){
					set_sprite();
					wait_vsync();
					set_constrast(j / 8, org_pal, BGPAL_NO);
					set_constrast(j / 8, org_pal, CHRPAL_NO);
					bg_roll();
				}
			}
//			fadeoutwhite(org_pal, CHRPAL_NO, 10);
		}
		wait_vsync();
		spr_clear();
		wait_vsync();
	}while( !(_iocs_bitsns(0) & 2));

	reset_int();
end:
//	if(mcd_status >= 0){
//		mcd_fadeout();
/*		mcd_release();*/
//	}

	pal_allblack(CHRPAL_NO);
	pal_allblack(BGPAL_NO);
	spr_off();
	paint_text(0);

	if(mcd_status >= 0){
		while(mcd_setfadelvl(-1));

//		do{
//			mcd_getplayflg(&part1, &part2);
//			printf("%x %x\n", part1, part2);
//		}while(part1 | part2);
	}
	grp_term();		/* 画面再初期化 */

/*dum2:	B_SUPER(1);*/

	exit(0);

	return NOERROR;
}

