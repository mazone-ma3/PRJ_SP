/* ゲ－ム本体 for PC-88VA */
#define _BORLANDC_SOURCE

#define MAIN
#define PC88VA

#define SPROW_ON 1

#include <dos.h>
#include <conio.h>
//#include <spr.h>
//#include <FMCFRB.H>

#include "sp_init.h"
#include "FONTVA.h"

enum {
	BGMMAX = 2,
	SEMAX = 4
};

void put_strings(int scr, int x, int y,  char *str, char pal);
void put_numd(long j, char digit);
char str_temp[9];

void __interrupt __far (*keepvector)(void);

/******************************************************************************/
#include "sp_com.h"
/******************************************************************************/

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

#define EOIDATA 0x20
#define EOI 0x188

#define WIDTH 32
#define CONV_LINE 32 //128 //200

short test_h_f = TRUE;
short soundflag = FALSE;

volatile unsigned char __far vs_count;
unsigned char keepport;
unsigned short title_index;

#define VECT 0x0a

void init_v_sync(void)
{
	_disable();
	keepport = inp(0x18a);
	keepvector = _dos_getvect(VECT);
	_dos_setvect(VECT, ip_v_sync);
	outp(0x18a, keepport & 0xfb);

	_enable();
}

void term_v_sync(void)
{
	_disable();
	_dos_setvect(VECT, keepvector);
	outp(0x18a, keepport);
	_enable();
}

void __interrupt __far ip_v_sync(void)
{
	++vs_count;
//	printf("%d\n", vs_count);
	outp(EOI, EOIDATA);
}

/*タイマウェイト*/
void wait(unsigned short wait)
{
	while(1){
		_disable();
		if(vs_count >= wait)
			break;
		_enable();
	}
	_disable();
	vs_count = 0;
	_enable();
}

void write_psg(int reg, int tone)
{
	while(inp(0x44) & 0x80);
	outp(0x44,reg);
	while(inp(0x44) & 0x80);
	outp(0x45, tone);
}

FILE *stream[2];


short set_sprite_pattern(char *loadfil, unsigned short adr, unsigned short width, unsigned short line, unsigned short spr_x_size, unsigned short spr_y_size, unsigned char spr_x_max, unsigned char spr_y_max)
{
	long i, j,count, count2;
	unsigned short k=0, l=0, m;
	unsigned char pattern[100];
	unsigned short header;

	unsigned short spr_x = 0, spr_y = 0, spr_no = 0, spr_no2 = 0;
	unsigned short index = 0;
	unsigned char __far *tvram = (unsigned char __far *)
		MK_FP(0xa000, 0);
	unsigned short spr_size = SPR_SIZE(spr_x_size, spr_y_size);

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return ERROR;
	}
	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */

	for(spr_no2 = 0; spr_no2 < spr_y_max; ++spr_no2){
		spr_y = spr_no2 * (spr_y_size) * spr_x_max;
		for(count = 0; count < line; ++count){

			for(count2 = 0; count2 < (width / 4); ++count2){

				i = fread(pattern, 1, 2, stream[0]);	/* 8dot分 */
				if(i < 1)
					break;

				index = ((spr_no) * spr_size) + spr_x + (spr_y * (spr_x_size / 2)) ;

				/* 横を2倍する */
				for(m = 0; m < 1; ++m){
					tvram[adr + index] = 
						(((pattern[m * 2 + 0]) >> 4) & 0x0f) |
						(((pattern[m * 2 + 0]) >> 4) & 0x0f) * 16;
					++index;
					tvram[adr + index] = 
						(((pattern[m * 2 + 0]) & 0x0f)) |
						(((pattern[m * 2 + 0]) & 0x0f) * 16);
					++index;
					tvram[adr + index] = 
						(((pattern[m * 2 + 1]) >> 4) & 0x0f) |
						(((pattern[m * 2 + 1]) >> 4) & 0x0f) * 16;
					++index;
					tvram[adr + index] = 
						(((pattern[m * 2 + 1]) & 0x0f)) |
						(((pattern[m * 2 + 1]) & 0x0f) * 16);
					++index;
				}

				spr_x += 4;
				if(spr_x >= (spr_x_size / 2)){
					spr_x = 0;
					++spr_no;
					if(spr_no >= spr_x_max){
						spr_no = 0;
						++spr_y;
					}
				}
			}
			for(count2 = 0; count2 < ((256 - width) / 4); ++count2){

				i = fread(pattern, 1, 2, stream[0]);	/* 8dot分 */
				if(i < 1)
					break;
			}
		}
	}
	fclose(stream[0]);

	return index;
}

unsigned short rev_sprite_pattern(unsigned short adr, unsigned short index)
{
	unsigned short i;
	unsigned char __far *tvram = MK_FP(0xa000, 0);
	unsigned char color1, color2;
	for(i = 0; i < index; ++i){
		color1 = (tvram[adr+i] >> 4) & 0x0f;
		color2 = tvram[adr+i] & 0x0f;
		if(color1 > 1)
			color1 = 13; //~color1 & 0x0f;
		else
			color1 = 0;
		if(color2 > 1)
			color2 = 13; //~color2 & 0x0f;
		else
			color2 = 0;

		tvram[adr+index+i] = color1 * 16 + color2;
	}

	return index+i;
}

//unsigned char __far *spr_atr;

//	*(spram++) = y % 256;				/* Y */
//	*(spram++) = (spr_y_size / 4 - 1) * 4 | 0x02 | ((y / 256) & 0x01);
//	*(spram++) = x % 256;				/* X */
//	*(spram++) = (spr_x_size / 8 - 1) * 8 | ((x / 256) & 0x03);
	/* データアドレス(TSP) */
//	*(spram++) = ((SPR_PAT_ADR + index + spr_size * i) / 2) % 256;
//	*(spram++) = ((SPR_PAT_ADR + index + spr_size * i) / 2) / 256;
//	*(spram++) = 0;
//	*(spram++) = 0;

void set_sprite_locate(unsigned short pat_no, unsigned short index, unsigned char i,  unsigned short x, unsigned short y, unsigned short spr_x_size, unsigned short spr_y_size){
	unsigned short spr_size = SPR_SIZE(spr_x_size, spr_y_size);
	spram = (unsigned short __far  *)
		MK_FP(0xa000, ((SPR_ATR) + (pat_no * 8)));
//	PUT_SP( x, y, ((spr_x_size / 4 - 1) * 4 | 0x02), ((spr_y_size / 8 - 1) * 8), ((SPR_PAT_ADR + index + spr_size * i) / 2));

	PUT_SP( x, y, (spr_x_size / 8 - 1) * 8 * 256, (spr_y_size / 4 - 1) * 4 * 256,
		(SPR_PAT_ADR + index + spr_size * i) / 2);

//	*(spram++) = y % 256;				/* Y */
//	*(spram++) = (spr_y_size / 4 - 1) * 4 | 0x02 | ((y / 256) & 0x01);
//	*(spram++) = x % 256;				/* X */
//	*(spram++) = (spr_x_size / 8 - 1) * 8 | ((x / 256) & 0x03);
	/* データアドレス(TSP) */
//	*(spram++) = ((SPR_PAT_ADR + index + spr_size * i) / 2) % 256;
//	*(spram++) = ((SPR_PAT_ADR + index + spr_size * i) / 2) / 256;
//	*(spram++) = 0;
//	*(spram++) = 0;
}


void set_spr_info(unsigned short count, unsigned short index, unsigned short i, unsigned char spr_x_size, unsigned char spr_y_size)
{
	unsigned short spr_size = SPR_SIZE(spr_x_size, spr_y_size);

	spr_info[count].x_size = (spr_x_size / 8 - 1) * 8 * 256;
	spr_info[count].y_size = (spr_y_size / 4 - 1) * 4 * 256;
	spr_info[count].adr = (SPR_PAT_ADR + index + spr_size * i) / 2;
//	printf("(%d %x)\n",count, spr_info[count].adr);
}

unsigned short read_sprite(void)
{
	unsigned short index = 0, index2 = 0, index3 = 0, count = 0;
	unsigned char i, j, k;
	unsigned char spr_no = 0;

	TVRAM_ON();

	if((index2 = set_sprite_pattern("COREJIKI.SC5", SPR_PAT_ADR + index, 8 * 3 * 3, 16, 16 * 3, 16, 3, 1)) == ERROR) {
		term();
		exit(1);
	}
	for(i = 0; i < 3; ++i){
//		set_sprite_locate(spr_no++, index, i, i * 16 * 3, 64, 16 * 3, 16);
		set_spr_info(count++, index, i, 48, 16);
	}
	index += index2;

	if((index2 = set_sprite_pattern("COREBOSS.SC5", SPR_PAT_ADR + index, 16*4, 32, 32 * 4, 16 * 2, 1, 1)) == ERROR) {
		term();
		exit(1);
	}
	for(i = 0; i < 1; ++i){
//		set_sprite_locate(spr_no++, index, i, 0, 32, 32 * 4, 16 * 2);
		set_spr_info(count++, index, i, 32 * 4, 16 * 2);
		index += index2;
	}

	if((index2 = set_sprite_pattern("CORESHOT.SC5", SPR_PAT_ADR + index, 16, 8, 16, 8, 2, 1)) == ERROR) {
		term();
		exit(1);
	}
	for(i = 0; i < 2; ++i){
//		set_sprite_locate(spr_no++, index, i, (i % 16) * 32, (i / 16) * 16 + 16, 16, 8);
		set_spr_info(count++, index, i,  16, 8);
	}
	index += index2;

	if((index2 = set_sprite_pattern("CORESHO2.SC5", SPR_PAT_ADR + index, 16, 16, 32, 16, 1, 1)) == ERROR) {
		term();
		exit(1);
	}
	for(i = 0; i < 1; ++i){
//		set_sprite_locate(spr_no++, index, i, (i % 16) * 32, (i / 16) * 16 + 24, 32, 8);
		set_spr_info(count++, index, i,  32, 16);
	}
	index += index2;

	if((index2 = set_sprite_pattern("CORETEKI.SC5", SPR_PAT_ADR + index, 256, LINE, 32, 16, 16, 1)) == ERROR) {
		term();
		exit(1);
	}
	for(i = 0; i < 16; ++i){
//		set_sprite_locate(spr_no++, index, i, (i % 16) * 32, (i / 16) * 16, 32, 16);
		set_spr_info(count++, index, i, 32, 16);
	}
	index += index2;
	index3 = index;
	
	index2 = rev_sprite_pattern(SPR_PAT_ADR, index);
	index += index2;

//	if((index2 = set_sprite_pattern("COREBOSS.SC5", SPR_PAT_ADR + index, 16*4, 32, 32 * 4, 16 * 2, 1, 1)) == ERROR) {

	if((index2 = set_sprite_pattern("TITLE.SC5", SPR_PAT_ADR + index, 128, 64, 256, 64, 1, 1)) == ERROR) {
		term();
		exit(1);
	}
	i = 0;
	title_index = index;
//	set_sprite_locate(spr_no++, title_index, i, (i % 16) * 32, (i / 16) * 16, 256, 64);
//	set_sprite_locate(spr_no++, title_index, 0,8,16, 256, 64);
//	while((inp(0x09) & 0x80));

	return index3;
}

char str_temp[9];

void put_strings(int scr, int y, int x,  char *str, char pal)
{
	char chr;
	unsigned short i = 0;
	unsigned char __far *vram;
	unsigned char xx, yy;
	y = 28-y;

	vram = (unsigned char __far *)MK_FP(0x0c000, 0);
	vram += (x * FONTSIZEX + y * FONTSIZEY * 160);

	GVRAM_ON();

	while((chr = *(str++)) != '\0'){
		if((chr < 0x30)) // || (chr > 0x5f))
			chr = 0x40;
		for(yy = 0; yy < FONTSIZEY; ++yy){
			for(xx = 0; xx < FONTSIZEX; ++xx){
				*vram++ = fontdata[xx][yy][chr - '0'];
			}
			vram += (160 - FONTSIZEX);
		}
		vram += (FONTSIZEX - 160 * FONTSIZEY);
//		put_8(((x) + (i++)) * 8, (28-y)*8, chr-'0');
	}
}


void put_numd(long j, char digit)
{
	char i = digit;

	while(i--){
		str_temp[i] = j % 10 + 0x30;
		j /= 10;
	}
	str_temp[digit] = '\0';
}

void score_display(void)
{
	put_numd(score, 8);
	put_strings(SCREEN2, 28, 2 + 6, str_temp, CHRPAL_NO);
	if(score >= hiscore){
		if((score % 10) == 0){
			hiscore = score;
			put_strings(SCREEN2, 28, 0, "HI", CHRPAL_NO);
		}
	}else
		put_strings(SCREEN2, 28, 0, "  ", CHRPAL_NO);
}

void score_displayall(void)
{
	put_strings(SCREEN2, 28, 2, "SCORE", CHRPAL_NO);
	score_display();
}

void hiscore_display(void)
{
	if(score > hiscore)
		if((score % 10) == 0)
			hiscore = score;

	put_numd(hiscore, 8);

	put_strings(SCREEN2, 15, 14, "HIGH", CHRPAL_NO);
	put_strings(SCREEN2, 15, 14 + 5, str_temp, CHRPAL_NO);
}

#include "inkey.h"

unsigned char keyscan(void)
{
	unsigned char k0, k1, k8, ka, st, pd, k5, k9;
	unsigned char keycode = 0;

	k0 = inp(0x00);
	k1 = inp(0x01);
	k8 = inp(0x08);
	ka = inp(0x0a);
	k5 = inp(0x05);
	k9 = inp(0x09);
	_disable();
	outp(0x44, 0x0e);
	st = inp(0x45);
	outp(0x44, 0x0f);
	pd = inp(0x45);
	_enable();

	if(!(k5 & 0x04) || !(k9 & 0x40) || !(pd & 0x01)) /* Z,SPACE */
		keycode |= KEY_A;
	if(!(k5 & 0x01) || !(pd & 0x02)) /* X */
		keycode |= KEY_B;
//		keycode |= KEY_START;
	if(!(k1 & 0x01) || !(k8 & 0x02) || !(st & 0x01)) /* 8 */
		keycode |= KEY_UP1;
	if(!(k0 & 0x04) || !(ka & 0x02) || !(st & 0x02)) /* 2 */
		keycode |= KEY_DOWN1;

	if(!(st & 0x0c)){ /* RL */
		keycode |= KEY_START;
	}else{
		if(!(k0 & 0x10) || !(ka & 0x04) || !(st & 0x04)) /* 4 */
			keycode |= KEY_LEFT1;
		if(!(k0 & 0x40) || !(k8 & 0x04) || !(st & 0x08)) /* 6 */
			keycode |= KEY_RIGHT1;
	}


	return keycode;
}


int opening_demo(void)
{
	signed int i, j;

//	display_allclear(CHRPAL_NO);

	put_strings(SCREEN2, 17, 15, "PROJECT CC", CHRPAL_NO);
	put_strings(SCREEN2, 15, 15, "SINCE 199X", CHRPAL_NO);
	fadeinblack(org_pal, CHRPAL_NO, 6);
	j = 4;
	for(i = 0; i < 75 / 2 * 3; i++){
		if(keyscan() || (!(inp(0x09) & 0x80))){
			j = 2;
			break;
		}
		sys_wait(1);
//		wait_vsync();
	}
	fadeoutblack(org_pal, CHRPAL_NO, j);

	return NOERROR;
}

int title_demo(void)
{
	int j, keycode;
	unsigned char x = 0;
	int loopcounter = 0;
	unsigned int soundtestno = 0;
	int soundtest = FALSE;

//	init_star();

	/* Set Title-Logo Pattern */
	/* Opening Start */
//	spralldisp(0, 16 * 8, 1 * 8, grptable[TITLEFILE].width, grptable[TITLEFILE].height, TITLETOP, CHRPAL_NO);
	TVRAM_ON();
	set_sprite_locate(0, title_index, 0, 240 - 32 - 16, 16, 256, 64);

//	pal_read(org_pal, grptable[FONTFILE].pal_table);
//	org_pal[15][0] = org_pal[15][1] = org_pal[15][2] = 31;

	j = -16 * 8;
	x = 1;
	do{
		if(j < 0){
			j++;
			set_constrast(j / 8, org_pal, CHRPAL_NO);
			if(!j){
//				org_pal[15][0] = org_pal[15][1] = org_pal[15][2] = 15;
//				put_strings(SCREEN2, 8, 7, "NORMAL", CHRPAL_NO);
//				put_strings(SCREEN2, 6, 7, "HARD", CHRPAL_NO);
				put_strings(SCREEN2, 11, 18, "START", CHRPAL_NO);
				put_strings(SCREEN2, 9, 18, "EXIT", CHRPAL_NO);
				put_strings(SCREEN2, 6, 14, "      ij k   ", CHRPAL_NO);
				put_strings(SCREEN2, 5, 14, "a2022 bcdefgh", CHRPAL_NO);

				score_displayall();
				hiscore_display();

//				KEYFRUSH;
//				while(keyscan());
			}
		}

		if(!j){
			put_strings(SCREEN2, 9 + x * 2, 15, "?", CHRPAL_NO);
			keycode = keyscan();
			if(keycode)
				loopcounter = 0;
			if((keycode & KEY_DOWN1) && (x != 0)){
				put_strings(SCREEN2, 9 + x * 2, 15, " ", CHRPAL_NO);
				x = 0;
			}
			if((keycode & KEY_UP1) && (x != 1)){
				put_strings(SCREEN2, 9 + x * 2, 15, " ", CHRPAL_NO);
				x = 1;
			}
//			if(keycode & KEY_START)
//				return SYSEXIT;
			if(keycode & KEY_B){
				if(keycode & KEY_A){
					if(soundtest == FALSE){
						put_strings(SCREEN2, 4, 2, "SOUND TEST", CHRPAL_NO);
						soundtest = TRUE;

						put_numd(soundtestno, 3);
						put_strings(SCREEN2, 4, 13, str_temp, CHRPAL_NO);
					}else{
						soundtest = FALSE;
						put_strings(SCREEN2, 4, 2, "              ", CHRPAL_NO);
//						S_IL_FUNC(bgm_fadeout());
					}
				}else{
//					if(soundtestno < BGMMAX){
//						playbgm(soundtestno, debugmode);
//					}else{
//						DI;
//						if(soundflag == TRUE)
//							if(se_check())
//								se_stop();
//						S_IL_FUNC(se_play(sndtable[0], soundtestno - BGMMAX));
//						EI;
//					}
				}
			}
			else if(keycode & (KEY_A)){ // | KEY_B)){ //UP1 | KEY_DOWN1 | KEY_RIGHT1 | KEY_LEFT1 | KEY_START)){
				if(!x)
					return SYSEXIT;
				if(keyscan() & KEY_LEFT1)
					return ERRLV2;
				if(keyscan() & KEY_RIGHT1)
					return ERRLV3;
				return ERRLV1;// + (1 - x);
			}
			if(keycode & (KEY_LEFT1 | KEY_RIGHT1)){
				if(soundtest == TRUE){
					if(keycode & KEY_DOWN2){
						++soundtestno;
					}else{
						--soundtestno;
					}
					soundtestno += (BGMMAX + SEMAX);
					soundtestno %= (BGMMAX + SEMAX);

					put_numd(soundtestno, 3);
					put_strings(SCREEN2, 4, 13, str_temp, CHRPAL_NO);
				}
			}
		}
		wait_vsync();
		bg_roll();
		if(++loopcounter == WAIT1S * 30){
//			S_IL_FUNC(bgm_fadeout());
			return NOERROR;
		}
	}while((inp(0x09) & 0x80));
	return SYSEXIT;
}

int a, b;

/* ゲ－ムのル－プ */
short game_loop(void){
//	unsigned char a = 0, b = 0;

	int i, j, xx, yy,*p_x,*p_y;
	int pat_no;
	unsigned char keycode;

	spr_count = 0;

/*  パッド入力 & 自機移動 */
/*	for(i=0;i<1;i++)*/
	i = 0;
	{
		a = 0;
		b = 0;
		keycode = keyscan();
//		pad_read(i, &a, &b, &pd);

		if(keycode & KEY_A){ /* Z,SPACE */
			a = 1;
		}
		if(keycode & KEY_B){ /* X */
			b = 1;
		}

		if((keycode & KEY_START) || (keycode & KEY_B)){
//			if(keycode & (KEY_LEFT1))
//				scrlspd = 0;
//			else{
//				put_strings(SCREEN2, 16, 6, "PAUSE", CHRPAL_NO);
//			}
//			do {
				if(scrl_spd)
					put_strings(SCREEN2, 16, 18, "PAUSE", CHRPAL_NO);
//				bg_roll();

//				do_putmessage();
				DEF_SP(spr_count, my_data[i].x, my_data[i].y, my_data[i].pat_num, mypal); /* 0x14); */
				SEARCH_LIST2(MAX_MYSHOT, i, j, myshot_next){
					DEF_SP(spr_count, myshot[i].x, myshot[i].y, myshot[i].pat_num,  CHRPAL_NO);
				}
				SEARCH_LIST2(MAX_TEKI, i, j, teki_next){
					DEF_SP(spr_count, teki[i].x, teki[i].y, teki_pat[i], teki_pal[i]);
				}
				SEARCH_LIST2(MAX_TKSHOT, i, j, tkshot_next){
					DEF_SP(spr_count, tkshot[i].x, tkshot[i].y, tkshot_pat[i], CHRPAL_NO);
				}
			do{
				wait_vsync();
				set_sprite();
			}while((keyscan() & (KEY_START | KEY_B)));
			do {
				wait_vsync();
				set_sprite();
//				keycode = key_hit_check();
				keycode = keyscan();
//				if(keycode & KEY_B){
				if(keycode & KEY_A){
//					bg_roll();
//					set_sprite();
					put_strings(SCREEN2, 16, 18, "     ", CHRPAL_NO);
					return SYSEXIT;		/* 一気に抜ける */
				}
//				if(keycode & KEY_A){
//				if(keycode & KEY_B){
//					scrl_spd = SCRL_MIN;
//					bg_roll();
//					set_sprite();
//					put_strings(SCREEN2, 16, 18, "     ", CHRPAL_NO);
//					return ERRLV2;
//				}
			}while((!(keyscan() & (KEY_START | KEY_B))));
			do{
				wait_vsync();
				set_sprite();
			}while(keyscan() & (KEY_START | KEY_B));
			scrl_spd = SCRL_MIN;
			put_strings(SCREEN2, 16, 18, "     ", CHRPAL_NO);

			return ERRLV1;
//			continue;
		}

/* 00  0=hit 1=Nohit */
/* AB */

		if (b){
			if (scrl_spd < SCRL_MAX)
				scrl_spd++;
		}else if (scrl_spd > SCRL_MIN)
			scrl_spd--;

		if (a & b)
			scrl_spd = 0;

/* 自機移動(斜め方向対応) */
		xx = yy = 0;
		pat_no = PAT_JIKI1;

		if(keycode & KEY_UP1){ /* 8 */
			yy = -1;
		}
		if(keycode & KEY_RIGHT1){ /* 6 */
			pat_no = PAT_JIKI2;
			xx = 1;
		}
		if(keycode & KEY_DOWN1){ /* 2 */
			yy = 1;
		}
		if(keycode & KEY_LEFT1){ /* 4 */
			pat_no = PAT_JIKI3;
			xx = -1;
		}
		my_data[i].pat_num = pat_no;

		/* 斜めの時の処理(手抜き版) */
		if ((xx == 0) || (yy == 0)){
			xx *= 3; yy *= 3;
		}
		else{
			xx *= 2; yy *= 2;
		}
		xx <<= SHIFT_NUM;
		yy <<= SHIFT_NUM;

		p_x=&my_data[i].x;
		p_y=&my_data[i].y;

		*p_x+=xx;
		*p_y+=yy;

	/* 自機が移動できる範囲を設定 */
		if(*p_y <= JIKI_MIN_Y)
			*p_y = JIKI_MIN_Y;
			else if(*p_y >= JIKI_MAX_Y)
				*p_y = JIKI_MAX_Y;
			if(*p_x <= JIKI_MIN_X)
				*p_x = JIKI_MIN_X;
			else if(*p_x >= JIKI_MAX_X)
				*p_x = JIKI_MAX_X;

		DEF_SP(spr_count, *p_x, *p_y, my_data[i].pat_num, mypal); /* 0x14); */
/*	printf("%d %d \n", chr_data[spr_count - 1].pat_num, (mypal << 8));*/

//		mypal = CHRPAL_NO;

		/* 自機弾発射 */
		if(trgcount)
			trgcount--;
		if(renshaflag == FALSE)
			if(trgcount2)
				trgcount2--;

		if (a && ((MAX_MYSHOT - trgnum) >= 2)){
			noshotdmg_flag = TRUE;
			if(trgcount2){
				if(!trgcount){
					trgcount = 5;

					if(myshot_free[MAX_MYSHOT] != END_LIST){
						ADD_LIST(MAX_MYSHOT, tmp, myshot_next, myshot_free);
						myshot[tmp].x = my_data[i].x + (1 << SHIFT_NUM);
						myshot[tmp].y = my_data[i].y;

						myshot[tmp].xx = 0;
						myshot[tmp].yy = -(6 << SHIFT_NUM);

						myshot[tmp].pat_num = PAT_MYSHOT1;

						trgnum++;
					}
					if(myshot_free[MAX_MYSHOT] != END_LIST){
						ADD_LIST(MAX_MYSHOT, tmp, myshot_next, myshot_free);
						myshot[tmp].x = my_data[i].x + (13 << SHIFT_NUM);
						myshot[tmp].y = my_data[i].y;

						myshot[tmp].xx = 0;
						myshot[tmp].yy = -(6 << SHIFT_NUM);

						myshot[tmp].pat_num = PAT_MYSHOT1;

						trgnum++;
					}
				}
			}
		}else
			trgcount2 = 60 / 3;
	}

	
	/** スケジュール解析を実行 **/
	do_schedule();

	/* 自機弾移動 */
	SEARCH_LIST2(MAX_MYSHOT, i, j, myshot_next){

		tmp_x = myshot[i].x;
		tmp_y = myshot[i].y;

		tmp_x += myshot[i].xx;
		tmp_y += myshot[i].yy;

		/* 自機弾画面外消去 */
		if(tmp_y < ((SCREEN_MIN_Y << SHIFT_NUM) + SPR_OFS_Y + 16)){
			tmp_y = SPR_DEL_Y;
			trgnum--;

			DEL_LIST(MAX_MYSHOT, i, j, myshot_next, myshot_free);
		}else{
			myshot[i].x = tmp_x;
			myshot[i].y = tmp_y;
		}
		DEF_SP(spr_count, tmp_x, tmp_y, myshot[i].pat_num,  CHRPAL_NO);
	}

	move_teki();
	move_tekishot();
	return NOERROR;
}

/* 変数初期化処理 */
void game_init(void){
	int i;

//	srand(time(NULL));	/* 乱数系列初期化 */

	stage = 0;
	waitcount = 0;
	schedule_ptr = 0;
	command_num = COM_DUMMY;
	command = (int *)stg1_data;
	uramode = 0;
//	renshaflag = FALSE;
	renshaflag = TRUE;

	trgcount = 0;	/* ショット間隔リミッタ */
	trgcount2 = 0;	/* 連射リミッタ */
	trgnum = 0;
	total_count = 0;

//	scrl = 0;
	scrl_spd = SCRL_MIN;

	mypal = CHRPAL_NO;
	mypal_dmgtime = 0;
	my_movecount = 0;

	/* 敵表示情報初期化 */
	INIT_LIST(MAX_MYSHOT, i, myshot_next, myshot_free);
	INIT_LIST(MAX_TKSHOT, i, tkshot_next, tkshot_free);
	INIT_LIST(MAX_TEKI, i, teki_next, teki_free);


	score = 0;
	tkshot_c = (6 << SHIFT_NUM);
	max_my_hp = 7;

	my_hp = max_my_hp;

	for(i = 0; i < MAX_TKSHOT; i++){
		tkshot_xx[i] = 0;
		tkshot_yy[i] = 0;
	}
	tkshotnum = 0;
	scrdspflag = TRUE;
	noshotdmg_flag = FALSE;

	seflag = 0;

/* リスト初期化(まだ未使用) */
/*	for(i=0;i<10;i++){
		start[i].next = &fin[i];
		start[i].prev = NULL;
		fin[i].next = NULL;
		fin[i].prev = &start[i];
	}
*/

	/* SPRITE 初期化 */
/*	for(i = 0; i < 256; i++)
		spr[i].y = SPR_DEL_Y;*/

/* 自機座標の初期化 */
	my_data[0].x = 128 << SHIFT_NUM;
	my_data[0].y = 120 << SHIFT_NUM;
	my_data[0].pat_num = PAT_JIKI1;
	my_data[1].x = 192 << SHIFT_NUM;		/* 二人プレイを想定 */
	my_data[1].y = 120 << SHIFT_NUM;
	my_data[1].pat_num = PAT_JIKI1;

/* 自弾座標の初期化 */
/*	for(i=0; i<10; i++){
		shot_data[i].x = 0 << SHIFT_NUM;
		shot_data[i].xx = 0;
		shot_data[i].y = SPR_DEL_Y;
		shot_data[i].yy = 0;
		shot_data[i].pat_num = PAT_MYSHOT1;
	}*/



/* 敵座標の初期化 */
/* 全スプライト表示数最大180とすると4枚かさねでせいぜい30程度 */
/*	for(i=0; i<TEKI_NUM_MAX; i++){
		teki_data[i].x = (rand() % SCREEN_MAX_Y) << SHIFT_NUM;
		teki_data[i].xx = 0; 
		teki_data[i].y = (rand() % SCREEN_MAX_X) << SHIFT_NUM;
		teki_data[i].yy = ((rand() & 7) + 2) << SHIFT_NUM;
		teki_data[i].pat_num = PAT_TEKI1;
	}
*/
//	spram = (unsigned char __far *)MK_FP(0xa000, SPR_ATR);
}



void init_star(void)
{
	int i;
/* スタ－の座標系を初期化 */
//	GVRAM_ON();
	for(i = 0;i < STAR_NUM; i++){
		star[0][i] = ((i + 1) * 160) / STAR_NUM;
		star[1][i] = rand() % 200;
		star[2][i] = (rand() % 2) + 1;
		star[3][i] = 0; //rand() % 4;
		star[4][i] = rand() % 14 + 2; //% 16;//0x01; //ffff;
/*		star[0][i+1]=star[0][i];
		star[1][i+1]=star[1][i] + 200;
		star[2][i+1]=star[2][i];
		star[3][i+1]=star[3][i];
		star[4][i+1]=star[4][i];*/
	}
/* スタ－の表示(固定表示) */
	GVRAM_ON();
	i = STAR_NUM;
	while(i--){
		vram = (unsigned char __far *)MK_FP(0xa000, (star[0][i] + ((320 / STAR_NUM) >> 1) + 
			(star[1][i] * 80)) << 1);
		*vram |= star[4][i];
	}

}

void bg_roll(void)
{
	int i;
	unsigned short index;
/* スクロ－ルレジスタ制御 */

//	_outb(0x440,17);
/* 	_outb(0x442,scrl % 256); */
//	_outb(0x443,scrl);

//	outp(0x20c, scrl % 200);
	outpw(0x20c, scrl & 0x03ff);
	outpw(0x20e, (scrl * 160L) & 0xfffc);
	outpw(0x210, ((scrl * 160L) >> 16) & 0x0003);
	scrl -= (scrl_spd >> SCRL_SFT);
	scrl += 200; //(400L * 1);
//	++scrl; //l %= 200;
	scrl %= 200; //(400L * 1);

//	score = scrl;
//	scrdspflag = TRUE;

/* スクロ－ルするスタ－ */

	GVRAM_ON();
	i = STAR_NUM;
	while(i--){
//		_FP_OFF(vram) = (star[0][i] + (star[1][i] * 512)) * 2;
		index = 
		(star[0][i] + (star[1][i] * 80 * 2)) ;
		//(star[0][i] + (star[1][i] * 80)) ;
		vram = (unsigned char __far *)MK_FP(0xa000, index);
/*		switch(star[3][i]){
			case 0:
				vram = MK_FP(0xa000, index);
				break;
			case 1:
				vram = MK_FP(0xb000, index);
				break;
			case 2:
				vram = MK_FP(0xc000, index);
				break;
			case 3:
				vram = MK_FP(0xd000, index);
				break;
		}*/
		*vram = star[3][i]; //&= ~star[4][i];
		star[1][i] += (star[2][i] + 200);
		star[1][i] %= 200;
//	}
//	i = STAR_NUM;
//	while(i--){
//		_FP_OFF(vram) = (star[0][i] + (star[1][i] * 512)) * 2;
		index = 
		(star[0][i] + (star[1][i] * 80 * 2)) ;
		//(star[0][i] + (star[1][i] * 80)) ;
		vram = (unsigned char __far *)MK_FP(0xa000, index);
//		star[3][i] = *vram;
//		*vram |= star[4][i];

/*		switch(star[3][i]){
			case 0:
				vram = MK_FP(0xa000, index);
				break;
			case 1:
				vram = MK_FP(0xb000, index);
				break;
			case 2:
				vram = MK_FP(0xc000, index);
				break;
			case 3:
				vram = MK_FP(0xd000, index);
				break;
		}*/
		star[3][i] = *vram;
		*vram |= star[4][i];
	}
}


/* ゲ－ム本体の処理 */
short game_run(short mode){
	unsigned int encode;
	short i;
	unsigned char keycode;

//	init_star();
//	init_v_sync();

	game_init();	/* 変数(広域)初期化 */
//	spr_on(MAX_SPRITE);	/* スプライト動作の開始 */

	switch(mode){
		case ERRLV2:
			stage = 3;
			my_hp = max_my_hp = 10;
			tkshot_c /= 6;
			score = 1;
//			renshaflag = TRUE;
			break;

		case ERRLV3:
			score = 2;
			my_hp = max_my_hp = 10;
//			renshaflag = TRUE;
//			renshaflag = FALSE;
			break;
	}

	put_strings(SCREEN2, 4, 0, "SHIELD", CHRPAL_NO);

	GVRAM_ON();
	score_displayall();
	scrdspflag = TRUE;
	my_hp_flag = TRUE;

	spr_count = old_count = 0;
	do{
		if(mypal_dmgtime){
			--mypal_dmgtime;
			if(!mypal_dmgtime){
				mypal = CHRPAL_NO;
				if(!my_hp){
					timeup = 60 * 10;
					scrlspd = 0;
					put_strings(SCREEN2, 18, 15, "CONTINUE A", CHRPAL_NO);
					while(keyscan() & (KEY_A | KEY_START)){
						wait_vsync();
						set_sprite();
					}
					do{
						put_numd((long)(timeup / 60), 2);
						put_strings(SCREEN2, 14, 19, str_temp, CHRPAL_NO);

//						bg_roll();
//						set_sprite();
						if(!(--timeup)){
							put_strings(SCREEN2, 18, 15, "           ", CHRPAL_NO);
							put_strings(SCREEN2, 18, 15, " GAME OVER ", CHRPAL_NO);
							put_strings(SCREEN2, 14, 19, "  ", CHRPAL_NO);
							scrlspd = 0; //SPR_DIV / 4;
							for(i = 0; i < 60 ; i++){
								wait_vsync();
								set_sprite();
								bg_roll();
							}
							return ERRLV1;
						}
						keycode =  keyscan();
						if(keycode & KEY_B){
							if((timeup -= 5) < 1)
								timeup = 1;
						}
						wait_vsync();
						set_sprite();
					}while(!(keycode & (KEY_START | KEY_A)));

//					bg_roll();
//					set_sprite();

					put_strings(SCREEN2, 18, 15, "           ", CHRPAL_NO);
					put_strings(SCREEN2, 14, 19, "  ", CHRPAL_NO);
					scrlspd = 0; //SPR_DIV / 4;
					score %= 10;
					if(score != 9){
						++score;
					}
					scrdspflag =TRUE;
					my_hp = max_my_hp;
					put_my_hp();
					mypal = REVPAL_NO;
					mypal_dmgtime = DMGTIME * 4;
					noshotdmg_flag = TRUE;
				}
			}
		}
		if(scrdspflag == TRUE){
			if(score > SCORE_MAX)
				score = SCORE_MAX;
			score_display();
			scrdspflag = FALSE;
		}
		if(my_hp_flag == TRUE){
			put_my_hp_dmg();
			my_hp_flag = FALSE;
		}
		switch(game_loop()){
			case SYSEXIT:
				return NOERROR; //SYSEXIT;
			case NOERROR:
				break;
			default:
				spr_count = 0;
				continue;
		}

		if(!b){
//			wait(1);
			wait_vsync();
		}
		/* TSP coommand */
#ifdef SPROW_ON
		while(inp(0x142) & 0x05);
		outp(0x142, 0x81);	/* SPROV スプライト衝突判定 */
		while(inp(0x142) & 0x05);
		if(inp(0x146) & 0x20){

//		if(inp(0x142) & 0x20){
			hit_check = TRUE;
		}else{
			hit_check = FALSE;
		}
#endif
//		_disable();
		set_sprite();
//		_enable();
		bg_roll();
	}while((scrl_spd != 0) && (inp(0x09) & 0x80));

//	term_v_sync();

/* 終了処理 */
/* 	spr_off(); */
//	return NOERROR;
	return SYSEXIT;
}

void wait_vsync(void)
{
/* 2回見ないと誤判断する可能性がある */
//	while(!(inp(0x0040) & 0x20)); /* WAIT VSYNC */
//	while(inp(0x0040) & 0x20));
	while((inp(0x0142) & 0x40)); /* WAIT VSYNC */
	while(!(inp(0x0142) & 0x40));
}

void set_sprite(void)
{
	int i, j;

/* スプライト表示 */
/* BIOSは使わずスプライトRAMに直接書き込む */
/* 	SPR_display(2, 0); */
/* SPRAM先頭アドレスをスプライト表示最大数から算出 */
/* 最大数が可変の時はどうなる? */
//	_FP_OFF(spram) = (1024 - MAX_SPRITE) * 8;
	spram = (unsigned short __far  *)
		MK_FP(0xa000, SPR_ATR); // + i * 8);

	TVRAM_ON();

/* 表示数ぶん書き込む */
	if(spr_count > MAX_SPRITE){
//		spr_count = MAX_SPRITE ;
		if(total_count & 1){
			for(i = spr_count - MAX_SPRITE, j = 0; j < MAX_SPRITE; i++, j++){
				pchr_data = &chr_data[i];\
				PUT_SP(pchr_data->x, pchr_data->y, pchr_data->x_size, pchr_data->y_size, pchr_data->adr );
			}
		}else{
			for(i = 0; i < MAX_SPRITE; i++){
				pchr_data = &chr_data[i];\
				PUT_SP(pchr_data->x, pchr_data->y, pchr_data->x_size, pchr_data->y_size, pchr_data->adr );
			}
		}
		old_count = MAX_SPRITE;
	}else{
//		for(i = spr_count - 1; i >= 0; i--){
		for(i = 0; i < spr_count; ++i){
//		spram = (unsigned short __far *)MK_FP(0xa000, SPR_ATR + i * 8);
			pchr_data = &chr_data[i];\
			PUT_SP(pchr_data->x, pchr_data->y, pchr_data->x_size, pchr_data->y_size, pchr_data->adr );
//		printf("(%d:X=%d Y=%d X_SIZE=%d Y_SIZE=%d ADR=%x %x)\n", i, chr_data[i].x, chr_data[i].y, chr_data[i].x_size, chr_data[i].y_size,chr_data[i].adr, spram);
		}
/* スプライトの表示数が減った場合､減った分を画面外に消去する */
/* 増える分には問題ない */
		if (old_count > spr_count){
			for(i = 0;i < (old_count - spr_count); i++){
//				PUT_SP(0,(SCREEN_MAX_Y),0,0,0);
				*(spram++) = 0L;
				*(spram++) = 0;
				*(spram++) = 0;
				*(spram++) = 0;
			}
		}
		old_count = spr_count;
	}

/* このフレ－ムで表示したスプライトの数を保存 */
//	old_count = spr_count;

	++total_count;

	if(seflag == 1){
		_disable();
		write_psg(6,127);
		write_psg(11,0);
		write_psg(12,15);
		write_psg(7,0x1c);  // 00011100
		write_psg(13,9);
		write_psg(10,0x10);
		_enable();
	}
	seflag = 0;
}
