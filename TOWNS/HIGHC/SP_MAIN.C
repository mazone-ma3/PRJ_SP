/* ゲ－ム本体 for FM TOWNS High C */

#define MAIN
#define TOWNS

#define _I386

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <spr.h>
#include <FMCFRB.H>
#include <snd.h>
#include <msvdrv.h>
#include <memory.h>

//#define DEBUG

#ifndef DEBUG
#include <his.h>
#endif

#include "sp_init.h"

enum {
	BGMMAX = 2,
	SEMAX = 4
};

void put_strings(int scr, int x, int y,  char *str, char pal);
void put_numd(long j, char digit);

//pragma Data(Common);
volatile int vsync_flag;
//pragma Data();

#ifdef DEBUG
/* HISを使わない場合(未検証) */
#pragma Calling_convention(_INTERRUPT|_CALLING_CONVENTION);
_Far void handler2(void)
{
	/* DSが設定されていないのが問題になるのでポインタで、ただしスタック使う */
	_Far int *vsync_f = &vsync_flag;
	_FP_SEG(vsync_f) = 0x14;
//	_FP_OFF(vsync_f) = _FP_OFF(&vsync_flag);
	(*vsync_f)++;

//	++score;
//	++vsync_flag;

	_outb(0x05ca, 0);

	_outb(06ch, 0); // 1μ秒ウェイト
	_outb(0x10, 0x20);

	_outb(06ch, 0); // 1μ秒ウェイト
	_outb(0x0, 0x20);
}
#pragma Calling_convention();
#else
char HIS_stack[1000];

void handler(void)
{
	++vsync_flag;
	_outb(0x05ca, 0);
}
#endif

char str_temp[9];
unsigned int encode;

/******************************************************************************/
#include "sp_com.h"
/******************************************************************************/

short test_h_f = TRUE;
short soundflag = FALSE;

unsigned char message_count = 0;

void put_strings(int scr, int y, int x,  char *str, char pal)
{
	char chr;
	unsigned short i = 0;

	while((chr = *(str++)) != '\0'){
		if(spr_count >= MAX_SPRITE)
			break;
//		if((chr < 0x30) || (chr > 0x5f))
//			chr = 0x40;
//		DEF_SP_SINGLE(spr_count, (x + (i++)) * 8 + 16, y * 8 + 16, (chr - '0' + CHR_TOP + 256), CHRPAL_NO, 0);
		if((chr >= 0x30)){ // && (chr <= 0x5f)){
			chr_data[spr_count].x = ((x+2) + (i)) * 8 + SPR_OFS_X; \
			chr_data[spr_count].y = (30-y) * 8 + SPR_OFS_Y + 2; \
			chr_data[spr_count].pat_num = (chr - '0' + CHR_TOP); \
			chr_data[spr_count].atr = (CHRPAL_NO + 256) | 0x8000; \
			spr_count++; \
		}
		i++;
	}
	message_count = spr_count;
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
	}
//	else
//		put_strings(SCREEN2, 28, 0, "  ", CHRPAL_NO);
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

	put_strings(SCREEN2, 13, 10, "HIGH", CHRPAL_NO);
	put_strings(SCREEN2, 13, 10 + 5, str_temp, CHRPAL_NO);
}

#include "inkey.h"

unsigned char keyscan(void)
{
	unsigned char st, pd;
	unsigned char k5, k6, k7, k8, k9, ka;
	unsigned char paddata;
	static char matrix[16];
	unsigned char keycode = 0;

	_disable();
	KYB_matrix(matrix);
	_enable();

	k5 = matrix[5];
	k6 = matrix[6];
	k7 = matrix[7];
	k8 = matrix[8];
	k9 = matrix[9];
	ka = matrix[0xa];

	paddata = _inb(0x4d0 + 0 * 2); 
	st = (paddata & 0x0f);
	pd = (paddata >> 4) & 0x03;

	if((k5 & 0x04) || (k6 & 0x20) || !(pd & 0x01)) /* Z,SPACE */
		keycode |= KEY_A;
	if((k5 & 0x08) || !(pd & 0x02)) /* X */
		keycode |= KEY_B;
	if((k7 & 0x08) || (k9 & 0x20) || !(st & 0x01)) /* 8 */
		keycode |= KEY_UP1;
	if((k8 & 0x08) || (ka & 0x01) || !(st & 0x02)) /* 2 */
		keycode |= KEY_DOWN1;

	if(!(st & 0x0c)){ /* RL */
		keycode |= KEY_START;
	}else{
		if((k7 & 0x40) || (k9 & 0x80) || !(st & 0x04)) /* 4 */
			keycode |= KEY_LEFT1;
		if((k8 & 0x01) || (ka & 0x02) || !(st & 0x08)) /* 6 */
			keycode |= KEY_RIGHT1;
	}

	return keycode;
}

int opening_demo(void)
{
	signed int i, j;

	spr_count = 0;
	put_strings(SCREEN2, 17, 11, "PROJECT CC", CHRPAL_NO);
	put_strings(SCREEN2, 15, 11, "SINCE 199X", CHRPAL_NO);
	wait_sprite();
	set_sprite();
	fadeinblack(org_pal, CHRPAL_NO, 6);
	j = 4;
	for(i = 0; i < 75 / 2 * 3; i++){
		if(keyscan()){
			j = 2;
			break;
		}
		_disable();
		if(KYB_read( 1, &encode ) == 0x1b){
			j = 2;
			break;
		}
		_enable();
		sys_wait(1);
		wait_sprite();
		set_sprite();
	}
	_enable();
	fadeoutblack(org_pal, CHRPAL_NO, j);

	return NOERROR;
}

void put_titlelogo(short x, short y)
{
	unsigned char i, j, num = FONTPARTS + CHR_TOP;
	for(j = 0; j < 4; ++j){
		for(i = 0; i < 8; ++i){
			chr_data[spr_count].x = x + i * 16 + SPR_OFS_X; \
			chr_data[spr_count].y = y + j * 16 + SPR_OFS_Y + 2; \
			chr_data[spr_count].pat_num = (num++); \
			chr_data[spr_count].atr = (CHRPAL_NO + 256) | 0x8000; \
			spr_count++; \
		}
	}
}

void put_title(void)
{
	score_displayall();
	hiscore_display();
	put_strings(SCREEN2, 10, 14, "START", CHRPAL_NO);
	put_strings(SCREEN2, 8, 14, "EXIT", CHRPAL_NO);
	put_strings(SCREEN2, 5, 10, "      ij k   ", CHRPAL_NO);
	put_strings(SCREEN2, 4, 10, "a2022 bcdefgh", CHRPAL_NO);
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
	j = -16 * 8;
	x = 1;
	spr_count = 0; //old_count = 0;
	put_titlelogo(128-48, 48);
	wait_sprite();
	set_sprite();
	wait_vsync();
	bg_roll();
	do{
		if(j < 0){
			j++;
//			spr_count = 0;
//			wait_vsync2();
			wait_sprite();
			set_sprite();
			wait_vsync();
			set_constrast(j / 8, org_pal, CHRPAL_NO);
			bg_roll();
//			if(!j){
//				put_title();

//				while(keyscan());
//			}
		}

		if(!j){
			spr_count = 0;
			put_titlelogo(128-48, 48);
			put_title();
			put_strings(SCREEN2, 8 + x * 2, 11, "?", CHRPAL_NO);

			if(soundtest == TRUE){
				put_strings(SCREEN2, 3, 2, "SOUND TEST", CHRPAL_NO);
				put_numd(soundtestno, 3);
				put_strings(SCREEN2, 3, 13, str_temp, CHRPAL_NO);
			}else{
//				put_strings(SCREEN2, 3, 2, "              ", CHRPAL_NO);
			}
			wait_sprite();
			set_sprite();
			wait_vsync();
			bg_roll();
//			bg_roll();

			keycode = keyscan();
			if(keycode)
				loopcounter = 0;
			if((keycode & KEY_DOWN1) && (x != 0)){
//				put_strings(SCREEN2, 8 + x * 2, 11, " ", CHRPAL_NO);
				x = 0;
			}
			if((keycode & KEY_UP1) && (x != 1)){
//				put_strings(SCREEN2, 8 + x * 2, 11, " ", CHRPAL_NO);
				x = 1;
			}
//			if(keycode & KEY_START)
//				return SYSEXIT;
			if(keycode & KEY_B){
				if(keycode & KEY_A){
					if(soundtest == FALSE){
						soundtest = TRUE;
					}else{
						soundtest = FALSE;
					}
				}else{
//					if(!MSV_stat_flag())
//						MSV_play_start();

//					SND_pcm_play_stop(71) ;
//					SND_pcm_play( 71, 60, 127, SNDBUFF );
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
				}
			}
		}
//		wait_vsync2();
		if(++loopcounter == WAIT1S * 30){
//			S_IL_FUNC(bgm_fadeout());
			return NOERROR;
		}
		_disable();
		if(KYB_read( 1, &encode ) == 0x1b)
			break;
		_enable();
	}while(1); //KYB_read( 1, &encode ) != 0x1b);
	_enable();
	return SYSEXIT;
}

void set_object(void)
{
	short i=0, j=0;

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
}

/* ゲ－ムのル－プ */
short game_loop(void){
	unsigned char a = 0, b = 0;
	int i, j, xx, yy,*p_x,*p_y;
	int pat_no;
	unsigned char keycode;

/*  パッド入力 & 自機移動 */
/*	for(i=0;i<1;i++)*/
	i = 0;
	{
		keycode = keyscan();
//		pad_read(i, &a, &b, &pd);

		if(keycode & KEY_A) /* Z,SPACE */
			a=1;
		if(keycode & KEY_B) /* X */
			b=1;

		if((keycode & KEY_START) || (keycode & KEY_B)){
//			if(keycode & (KEY_LEFT1))
//				scrlspd = 0;
//			else{
//				put_strings(SCREEN2, 14, 6, "PAUSE", CHRPAL_NO);
//			}
//			do {
				if(scrl_spd)
					put_strings(SCREEN2, 14, 13, "PAUSE", CHRPAL_NO);
//				bg_roll();

				do_putmessage();
				set_object();
			do{
				wait_sprite();
				set_sprite();
			}while((keyscan() & (KEY_START | KEY_B)));
			do {
				wait_sprite();
				set_sprite();
//				keycode = key_hit_check();
				keycode = keyscan();
//				if(keycode & KEY_B){
				if(keycode & KEY_A){
//					bg_roll();
//					set_sprite();
					return SYSEXIT;		/* 一気に抜ける */
				}
//				if(keycode & KEY_A){
//				if(keycode & KEY_B){
//					scrl_spd = SCRL_MIN;
//					bg_roll();
//					set_sprite();
//					put_strings(SCREEN2, 14, 13, "     ", CHRPAL_NO);
//					return ERRLV2;
//				}
			}while((!(keyscan() & (KEY_START | KEY_B))));
			do{
				wait_sprite();
				set_sprite();
			}while(keyscan() & (KEY_START | KEY_B));
			scrl_spd = SCRL_MIN;
//			put_strings(SCREEN2, 14, 13, "     ", CHRPAL_NO);

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

		if(keycode & KEY_UP1) /* 8 */
//		if(pd & 0x01)	/* UP */
			yy = -1;
		if(keycode & KEY_DOWN1) /* 2 */
//		if(pd & 0x02)	/* DOWN */
			yy = 1;
		if(keycode & KEY_LEFT1){ /* 4 */
//		if(pd & 0x04){	/* LEFT */
			pat_no = PAT_JIKI3;
			xx = -1;
		}
		if(keycode & KEY_RIGHT1){ /* 6 */
//		if(pd & 0x08){	/* RIGHT */
			pat_no = PAT_JIKI2;
			xx = 1;
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
						myshot[tmp].x = my_data[i].x;
						myshot[tmp].y = my_data[i].y;

						myshot[tmp].xx = 0;
						myshot[tmp].yy = -(6 << SHIFT_NUM);

						myshot[tmp].pat_num = PAT_MYSHOT1;

						trgnum++;
					}
					if(myshot_free[MAX_MYSHOT] != END_LIST){
						ADD_LIST(MAX_MYSHOT, tmp, myshot_next, myshot_free);
						myshot[tmp].x = my_data[i].x + (12 << SHIFT_NUM);
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

	srand(time(NULL));	/* 乱数系列初期化 */

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
}



void init_star(void)
{
	int i;
/* スタ－の座標系を初期化 */
	for(i = 0;i < STAR_NUM; i++){
		star[0][i] = ((((i + 1) * 256) / STAR_NUM) + 32);
		star[1][i] = rand() % 512;
		star[2][i] = (rand() % 2) + 1;
		_FP_OFF(vram) = ((star[0][i] + star[1][i] * 1024) / 2);
		star[3][i] = *vram;
		star[4][i] = rand() % 14 + 2;//% 14 + 2; //fff;
	}
/* スタ－の表示(固定表示) */
	i = STAR_NUM;
	while(i--){
		_FP_OFF(vram) = (((star[0][i] + (256 / STAR_NUM)) % 256 + 32 + star[1][i] * 1024) / 2);
		*vram |= star[4][i];
	}

}

void bg_roll(void)
{
	int i;

/* スクロ－ルレジスタ制御 */
	_disable();
	_outb(0x440,17);
 	_outb(0x442,scrl * 128 % 256);
	_outb(0x443,scrl * 128 / 256);
	_enable();
	scrl += 512 - (scrl_spd >> SCRL_SFT);
	scrl %= 512;

/* スクロ－ルするスタ－ */

	i = STAR_NUM;
	while(i--){
		_FP_OFF(vram) = ((star[0][i] + star[1][i] * 1024) / 2);
		*vram = star[3][i];
		star[1][i] += (star[2][i] + 512);
		star[1][i] %= 512;
//	}
//	i = STAR_NUM;
//	while(i--){
		_FP_OFF(vram) = ((star[0][i] + star[1][i] * 1024) / 2);
		star[3][i] = *vram;
		*vram |= star[4][i];
	}
}


/* ゲ－ム本体の処理 */
short game_run(short mode){
	short i;
	unsigned char keycode;

//	init_star();

	my_hp_flag == TRUE;
	game_init();	/* 変数(広域)初期化 */

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

//	put_strings(SCREEN2, -3, 2, "SHIELD", CHRPAL_NO);


	spr_count = 0; //old_count = 0;
	do{
		if(mypal_dmgtime){
			--mypal_dmgtime;
			if(!mypal_dmgtime){
				mypal = CHRPAL_NO;
				if(!my_hp){
					mypal = REVPAL_NO;
					timeup = 60 * 10;
					scrlspd = 0;

					do{
						spr_count = 0;
						put_strings(SCREEN2, 14, 10, "CONTINUE A", CHRPAL_NO);
						if(timeup != 60 * 10){
							put_numd((long)(timeup / 60), 2);
							put_strings(SCREEN2, 10, 14, str_temp, CHRPAL_NO);
						}
						if(!(--timeup)){
							spr_count = 0;
//							put_strings(SCREEN2, 14, 10, "           ", CHRPAL_NO);
							put_strings(SCREEN2, 14, 10, " GAME OVER ", CHRPAL_NO);
//							put_strings(SCREEN2, 10, 14, "  ", CHRPAL_NO);
							scrlspd = 0; //SPR_DIV / 4;
							score_displayall();
							put_my_hp_dmg();
							set_object();
							wait_sprite();
							set_sprite();
							for(i = 0; i < 60 ; i++){
								wait_sprite();
								set_sprite();
								wait_vsync();
								bg_roll();
							}
							return ERRLV1;
						}
						keycode =  keyscan();
						if(keycode & KEY_B){
							if((timeup -= 5) < 1)
								timeup = 1;
						}
						score_displayall();
						put_my_hp_dmg();
						set_object();
						wait_sprite();
						set_sprite();
						wait_vsync();

						if(timeup == (60*10-1)){
							while(keyscan() & (KEY_A | KEY_START)){
								wait_sprite();
								set_sprite();
								wait_vsync();
							}
						}
					}while(!(keyscan() & (KEY_START | KEY_A)));

//					put_strings(SCREEN2, 14, 10, "           ", CHRPAL_NO);
//					put_strings(SCREEN2, 10, 14, "  ", CHRPAL_NO);
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
		spr_count = 0;

		if(scrdspflag == TRUE){
			if(score > SCORE_MAX)
				score = SCORE_MAX;
			score_displayall();
//			scrdspflag = FALSE;
/*			if(score >= hiscore){
				if((score % 10) == 0){
					hiscore = score;
				}
				put_strings(SCREEN2, 28, 0, "HI", CHRPAL_NO);
			}else
				put_strings(SCREEN2, 28, 0, "  ", CHRPAL_NO);
*/		}
		if(my_hp_flag == TRUE){
			put_my_hp_dmg();
		}
		do_putmessage();

		switch(game_loop()){
			case SYSEXIT:
				return NOERROR;
			case NOERROR:
				wait_sprite();
				set_sprite();
				wait_vsync();
				bg_roll();
				break;
			default:
				continue;
		}
		if(!scrl_spd)
			break;
		_disable();
		if(KYB_read( 1, &encode ) == 0x1b)
			break;
		_enable();
	}while(1);//while((scrl_spd != 0) && (KYB_read( 1, &encode ) != 0x1b));
	_enable();

/* 終了処理 */
/* 	spr_off(); */
	return SYSEXIT;
}


#ifdef DEBUG
#define VSYNC_VECTOR 0x4b
_real_int_handler_t saver;
_Handler savep = 0;
#else
#define VSYNC_VECTOR 0xb
#endif

int init_vsync(void)
{
	int ret;
	vsync_flag = 0;

#ifdef DEBUG
	saver = _getrvect(VSYNC_VECTOR);
	savep = _getpvect(VSYNC_VECTOR);

//	return 0;

	_disable();

	ret = _setrpvectp(VSYNC_VECTOR, handler2);
//	ret = _setpvect(VSYNC_VECTOR, handler);
	if(!ret){
		_outb(0x12, _inb(0x12) & 0xf7);
	}

	_outb(0x05ca, 0);

	_enable();
	return ret;
#else
	HIS_stackArea(HIS_stack, 1000);
	HIS_setHandler(VSYNC_VECTOR, handler);
	HIS_enableInterrupt(VSYNC_VECTOR);

	return 0;
#endif

}

void reset_vsync(void)
{
#ifdef DEBUG
	_disable();

	_outb(0x12, _inb(0x12) | 0x08);

	_setrvect(VSYNC_VECTOR, saver);
	_setpvect(VSYNC_VECTOR, savep);

	_enable();
#else
	HIS_detachHandler(VSYNC_VECTOR);
#endif
}

void wait_vsync(void)
{
	while(!vsync_flag);
	_disable();
	vsync_flag = 0;
//	_outb(0x10, 0x23);
//	_outb(0x05ca, 0);
	_enable();
	/* VSYNC(=1)待ち */
//	do{
//		_outb(0x440, 30);
//	}while((_inb(0x0443) & 0x04)); /* 動作中 */
//	do{
//		_outb(0x440, 30);
//	}while(!(_inb(0x0443) & 0x04)); /* 動作中 */
}

void wait_vsync2(void)
{
	/* VSYNC(=1)待ち */
	do{
		_outb(0x440, 30);
	}while((_inb(0x0443) & 0x04)); /* 動作中 */
	do{
		_outb(0x440, 30);
	}while(!(_inb(0x0443) & 0x04)); /* 動作中 */
}

void wait_sprite(void)
{
	if(seflag){
//		if(soundflag == TRUE)
//			if(se_check())
//				se_stop();
//		S_IL_FUNC(se_play(sndtable[0], seflag - 1));	/* 効果音 */
/*		SND_pcm_play_stop(71) ;
		SND_pcm_play( 71, 60, 127, &SNDBUFF[seflag - 1][0] );
*/
		MSV_partstop(13);
		MSV_partplay(13 , msv_se[seflag - 1]);
		seflag = 0;
	}

/* スプライト動作チェック(BUSY=1) */
/* VSYNC割り込みしない場合は2回見ないと誤判断する可能性があった */
//	_disable();
//	if((_inb(0x044c) & 0x02)){	/* BUSY */
//		while(1){
//			_disable();
//			if((_inb(0x044c) & 0x02)) /* BUSY */
//				break;
//			_enable();
//		}
//		_enable();
		while(1){
			_disable();
			if(!(_inb(0x044c) & 0x02)) /* READY */
				break;
			_enable();
		}
//	}
/*	_outb(0x450, 1);	// スプライトコントローラーを切るテスト
	_outb(0x452, 0x7f);*/
	_enable();

//	while(!(_inb(0x044c) & 0x02)); /* 動作中 */
}

void set_sprite(void)
{
	int i, j;

/* スプライト表示 */
/* EGB/TBIOSは使わずスプライトRAMに直接書き込む */
/* 	SPR_display(2, 0); */
/* SPRAM先頭アドレスをスプライト表示最大数から算出 */
/* 最大数が可変の時はどうなる? */

/* 表示数ぶん書き込む */
	if(spr_count > MAX_SPRITE){
		_FP_OFF(spram) = (1024 - MAX_SPRITE) * 8;

		_disable();
		if(total_count & 1){
			for(i = spr_count - 1, j = 0; j < (MAX_SPRITE - message_count); i--, j++){
				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
			}
			for(i = message_count - 1; i >= 0; i--){
				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
			}

		}else{
			for(i = MAX_SPRITE - 1; i >= 0; i--){
				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
			}
		}
		spr_on(MAX_SPRITE);
		_enable();

		old_count = MAX_SPRITE;
	}else{
//		spram = MK_FP(0x130, (1024 - spr_count) * 8);
		_FP_OFF(spram) = (1024 - spr_count) * 8;
//		pchr_data = &chr_data[spr_count - 1];
		_disable();
		for(i = spr_count - 1; i >= 0; i--){
			PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
//			memcpy(spram, pchr_data, 4 * 2);
//			pchr_data--;
//			spram += 4;
		}
		spr_on(spr_count);
		_enable();

/* スプライトの表示数が減った場合､減った分を画面外に消去する */
/* 増える分には問題ない */
/*		if (old_count > spr_count){
			for(i = 0;i < (old_count - spr_count); i++){
				PUT_SP(0,(SCREEN_MAX_Y + 2),0,0x2000);
			}
		}*/
/* このフレ－ムで表示したスプライトの数を保存 */
		old_count = spr_count;
	}


	++total_count;

}

/*void init_chr_data(void)
{
	int i;
	spr_count = 0;
	old_count = MAX_SPRITE;

	for(i = 0;i < MAX_SPRITE; i++){
		chr_data[i].x = 0;
		chr_data[i].y = 0;
		chr_data[i].pat_num = 0;
		chr_data[i].atr = 0;
	}
}*/
