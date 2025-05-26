/* ゲ－ム本体 for FM TOWNS OpenWatcom */

//#define MAIN
//#define TOWNS

//#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <i86.h>
#include <conio.h>
#include <string.h>
//#include <spr.h>
//#include <FMCFRB.H>
//#include <snd.h>
#include "snd.h"
//#include <msvdrv.h>

#include "sp_init.h"

enum {
	BGMMAX = 2,
	SEMAX = 4
};

void wait_sprite(void);
void set_sprite(void);
void bg_roll(void);

void put_strings(int scr, int x, int y,  char *str, char pal);
void put_numd(long j, char digit);
char str_temp[9];
unsigned int encode;

#include "inkey.h"


#define PARA_RINT 0
#define PARA_DSWD 1
#define PARA_ESWD 2
#define PARA_FSWD 3
#define PARA_GSWD 4
#define PARA_EAXDWD 5
#define PARA_EDXDWD 7
#define PARA_END 9

/* 80386の仕様でリアルモードでは変数がアドレス先頭の1MBにないとこける */
/* 本来なら共有変数を作るが煩雑になるのでとりあえず */
unsigned short PARABLK[PARA_END];

unsigned char matrix[16];

void KYB_matrix(void)
{
	PARABLK[PARA_RINT] = 0x90;
	PARABLK[PARA_EAXDWD] = 0x0a * 256;

__asm{
	mov	ax,ds
	mov	es,ax

	mov	ax,0250fh
	lea	ebx,matrix
	mov	ecx,8

	cli		/* リアルモードは割り込み禁止にしてみている */
	int	21h
	sti

	jc	error

	mov	di,cx
	ror	ecx,16
;	mov	DSWD,cx

	mov	PARABLK+PARA_DSWD*2,cx

;	mov	ah,0ah; 54h
;	mov	al,DEVNO
;	mov	ch,00h
;	mov	cl,00h

;	mov	EAXDWD,eax
;	mov	bx,90h; 93h
;	mov	RINT,bx

	mov	ax,2511h
	lea	edx,PARABLK;RINT
	cli		/* 同上 */
	int	21h
	sti
error:

}
}

//volatile unsigned char 
volatile int vsync_flag = 0;
int VECTOR_ADRV;
short VECTOR_SEGV;
int VECTOR_REAV;
short datasegment;

void int_fm(void)
{
//	vsync_flag = 1;
}

/*
void int_fm(void)
{
//	_disable();
//	++score;
	++vsync_flag;

	_outp(0x05ca, 0);
//	_outp(0x10, 0x23);

//	_outb(0x0, 0x27);
//	_enable();
//	return;
}*/

extern char *stackAddress;
extern char stack[1000];
extern long score;

#ifdef DEBUG
/* スタックを作ろうとしてうまくいかず */
void __interrupt __near int_vsync(void)
{
//	outp(0x22,0x40);
//	outp(0x5ca,0);
	__asm {
		push	fd
		push	eax

		push	es
		push	fs
		push	gs
		pushad

		push	ds

;		mov	ax,ds
;		cmp	ax,014h
;		jne	skipvsync


;		mov	ebx,int_vsync
		xor	eax,eax
;		mov	ax,cs:[ ebx ].dataSegment

		mov	ax,014h
		mov	ds,ax

		mov	ax,datasegment
		mov	ds,ax

		mov	ds:vsync_flag,1
;		inc	score
;		inc ds:vsync_flag
;	}
;//		vsync_flag = 1;
;		++vsync_flag;
;	__asm {
		xor	al,al
		mov	cx,05cah
		out	cx,al

		jp	skipvsync

		push	edx
		push	ds

; 旧スタック退避
		mov	edx,esp
		xor	ebp,ebp
		mov	bp,ss

; スタック切り換え
		mov	ecx,stackAddress
		cmp	ecx,0
		jne	short UserStack1
; DOS|Extenderのスタックを使う
		mov	ecx,esp
		mov	ax,ss
UserStack1:
		mov	ss,ax
		mov	esp,ecx
		push	edx ; 旧ESP
		push	ebp ; 旧SS

;		call	int_vsync
;		inc	vsync_flag

; スタック復元
		pop	ebp
		pop	edx
		mov	ss,bp
		mov	esp,edx

		pop	ds
		pop	edx

skipvsync:
		pop	ds

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		mov	al,00100000b
		out	0010h,al	;/* EOI(Slave) */

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		out 0,al	;/* EOI(Master) */

		popad
		pop	gs
		pop	fs
		pop	es

		pop	eax

		pop	fd
;		iretd
	}
}
#else
void __interrupt __near int_vsync(void)
{
	__asm {
		push	eax
		push	ds
		xor	eax,eax

;		mov	ax,014h		/* Watcomでは一応DSは設定されるみたいだが詳細不明 */
;		mov	ds,ax

;		mov	ax,datasegment
;		mov	ds,ax

		mov	ds:vsync_flag,1

		xor	al,al
		mov	cx,05cah
		out	cx,al

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		mov	al,00100000b
		out	0010h,al	;/* EOI(Slave) */

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		out 0,al	;/* EOI(Master) */

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		pop	ds
		pop	eax
	}
}

#endif

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



unsigned char keyscan(void)
{
	unsigned char st, pd;
	unsigned char k5, k6, k7, k8, k9, ka;
	unsigned char paddata;
	unsigned char keycode = 0;

//	KYB_matrix(matrix);
	KYB_matrix();
	k5 = matrix[5];
	k6 = matrix[6];
	k7 = matrix[7];
	k8 = matrix[8];
	k9 = matrix[9];
	ka = matrix[0xa];

	paddata = _inp(0x4d0 + 0 * 2); 
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
//	wait_vsync();

	fadeinblack(org_pal, CHRPAL_NO, 5);

	j = 4;
	for(i = 0; i < 75 / 2 * 3; i++){
		if(keyscan()){
			j = 2;
			break;
		}
		if((KYB_read( 1, &encode ) == CODE_ESC)){
			j = 2;
			break;
		}
//		sys_wait(1);
		wait_sprite();
		set_sprite();
		wait_vsync();
	}
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
			bg_roll();
			set_sprite();
			wait_vsync();
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
//			S_IL_FUNC(bgm_fadeoutp());
			return NOERROR;
		}
	}while(KYB_read( 1, &encode ) != CODE_ESC);
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
				wait_vsync();
			}while((keyscan() & (KEY_START | KEY_B)));
			do {
				wait_sprite();
				set_sprite();
				wait_vsync();
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
				wait_vsync();
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
}



void init_star(void)
{
	int i;
/* スタ－の座標系を初期化 */
	for(i = 0;i < STAR_NUM; i++){
		star[0][i] = ((((i + 1) * 256) / STAR_NUM) + 32);
		star[1][i] = rand() % 512;
		star[2][i] = (rand() % 2) + 1;
//		_FP_OFF(vram) = ((star[0][i] + star[1][i] * 1024) / 2);
		vram = MK_FP(0x120, ((star[0][i] + star[1][i] * 1024) / 2));
		star[3][i] = *vram;
		star[4][i] = rand() % 14 + 2;//% 14 + 2; //fff;
	}
/* スタ－の表示(固定表示) */
	i = STAR_NUM;
	while(i--){
//		_FP_OFF(vram) = (((star[0][i] + (256 / STAR_NUM)) % 256 + 32 + star[1][i] * 1024) / 2);
		vram = MK_FP(0x120, (((star[0][i] + (256 / STAR_NUM)) % 256 + 32 + star[1][i] * 1024) / 2));
		*vram |= star[4][i];
	}

}

void bg_roll(void)
{
	int i;

/* スクロ－ルレジスタ制御 */
	_disable();
	_outp(0x440,17);
 	_outp(0x442,scrl * 128 % 256);
	_outp(0x443,scrl * 128 / 256);
	_enable();
	scrl += 512 - (scrl_spd >> SCRL_SFT);
	scrl %= 512;

/* スクロ－ルするスタ－ */

	i = STAR_NUM;
	while(i--){
//		_FP_OFF(vram) = ((star[0][i] + star[1][i] * 1024) / 2);
		vram = MK_FP(0x120, ((star[0][i] + star[1][i] * 1024) / 2));
		*vram = star[3][i];
		star[1][i] += (star[2][i] + 512);
		star[1][i] %= 512;
//	}
//	i = STAR_NUM;
//	while(i--){
//		_FP_OFF(vram) = ((star[0][i] + star[1][i] * 1024) / 2);
		vram = MK_FP(0x120, ((star[0][i] + star[1][i] * 1024) / 2));
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
							wait_vsync();
							for(i = 0; i < 30 ; i++){
								wait_sprite();
								set_sprite();
								wait_vsync();
								bg_roll();
							}
							for(i = 0; i < 30 ; i++){
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
	}while((scrl_spd != 0) && (KYB_read( 1, &encode ) != CODE_ESC));

/* 終了処理 */
/* 	spr_off(); */
	return SYSEXIT;
}


int init_vsync(void)
{
	int ret;
	vsync_flag = 0;

__asm{
	cli
}
//	SND_int_timer_a1(0,0);
//	SND_int_timer_b1(0,0);
//	SND_int_timer_b1(1,199);

//	stackAddress = stack+1000;

//	outp(0x04ea, 0xff);
//	outp(0x22,0x40);
//	outp(0x5ca,0);
	__asm {
		cli
		push	es
		push	fs
		push	gs

		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi
		push	ebp

		pushad
		pushfd

;		hlt

		mov	ax,ds
		mov	datasegment,ax

		mov	cl,04bh
		mov	ax,02502h
		int	21h
		mov	ax,es
		mov	VECTOR_ADRV,ebx
		mov	VECTOR_SEGV,ax

		mov	cl,04bh
		mov	ax,02503h
		int	21h
		mov	VECTOR_REAV,ebx

		push	ds
		mov	ax,cs
		mov ds,ax

		mov	ax,02506h
;		mov	ax,02504h
		mov	cl,4bh
		lea	edx,int_vsync
		int	21h

		pop	ds
		jc	error1

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		in	al,0010h+2

;		jp	short dummy0
;dummy0:
;		jp	short dummy1
;dummy1:
;		jp	short dummy2
;dummy2:

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

;		mov	dl,00100000b
		mov	dl,00001000b
		xor	dl,0ffh
		and	al,dl
		out	0010h+2,al

;		mov	al,010000011b
;		out	0010h,al

;		mov	al,000000001b
;		out	0012h,al

		popfd
		popad

		pop	 ebp
		pop	 edi
		pop	 esi
		pop	 edx
		pop	 ecx
		pop	 ebx
		pop	 eax

		pop	 gs
		pop	 fs
		pop	 es

		xor	eax,eax

		jp	end

error1:
;	outp(0x22,0x40);
;		mov	al,40h
;		out 22h,al
		hlt
end:
		sti
	}
	return 0;
}

void reset_vsync(void)
{
	__asm {
		cli

		push	es
		pushad
		pushfd

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		in	al,0010h+2

;		jp	short dummy0
;dummy0:
;		jp	short dummy1
;dummy1:
;		jp	short dummy2
;dummy2:
		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

;		mov	dl,00100000b
;		mov	dl,00001000b
;		xor	dl,0ffh
;		or	al,dl
		or	al,00001000b
		out	0010h+2,al

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		push	ds
;		mov	ax,cs
;		mov ds,ax

		mov	cl,4bh
		mov	edx,VECTOR_ADRV
		mov	ax, VECTOR_SEGV
		mov	ebx,VECTOR_REAV

		mov	ds,ax
		mov	ax,02507h
		int	21h

		pop	ds
		jnc	noerror

;	outp(0x22,0x40);
		mov	al,40h
		out 22h,al

		hlt

;		mov	al,00100101b
;		out	0010h,al	/* EOI */

noerror:
		popfd
		popad
		pop	es
		xor	eax,eax

		sti
	}
}

void wait_vsync(void)
{
//	goto dummy;
//dummy:
//	while(!vsync_flag);
	while(1){
		_disable();
		if(vsync_flag)
			break;
		_enable();
	}
//	_enable();
//	_disable();
	vsync_flag = 0;
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
	while(1){
		_disable();
		if(vsync_flag)
			break;
		_enable();
	}
	vsync_flag = 0;
	_enable();
	/* VSYNC(=1)待ち */
//	do{
//		_outp(0x440, 30);
//	}while((_inp(0x0443) & 0x04)); /* 動作中 */
//	do{
//		_outp(0x440, 30);
//	}while(!(_inp(0x0443) & 0x04)); /* 動作中 */
}

void wait_sprite(void)
{
	if(seflag){
//		if(soundflag == TRUE)
//			if(se_check())
//				se_stop();
//		S_IL_FUNC(se_play(sndtable[0], seflag - 1));	/* 効果音 */
		SND_pcm_play_stop(71) ;
		SND_pcm_play( 71, 60, 127, &SNDBUFF[seflag - 1][0] );

//		MSV_partstop(13);
//		MSV_partplay(13 , msv_se[seflag - 1]);
		seflag = 0;
	}

/* スプライト動作チェック(BUSY=1) */
/* VSYNC割り込みしない場合は2回見ないと誤判断する可能性があった */
//	_disable();
//	if((_inp(0x044c) & 0x02)){	/* BUSY */
//		while(1){
//			_disable();
//			if((_inp(0x044c) & 0x02)) /* BUSY */
//				break;
//			_enable();
//		}
//		_enable();
		while(1){
			_disable();
			if(!(_inp(0x044c) & 0x02)) /* READY */
				break;
			_enable();
		}
//	}
/*	_outp(0x450, 1);	// スプライトコントローラーを切るテスト
	_outp(0x452, 0x7f);*/
	_enable();


//	if((_inp(0x044c) & 0x02)){
//		while(!(_inp(0x044c) & 0x02)); /* 動作中 */
//		while((_inp(0x044c) & 0x02)); /* 動作中 */
//	}
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
//	_FP_OFF(spram) = (1024 - MAX_SPRITE) * 8;
//	spram = MK_FP(0x130, (1024 - MAX_SPRITE) * 8);

/* 表示数ぶん書き込む */
	if(spr_count > MAX_SPRITE){
		spram = MK_FP(0x130, (1024 - MAX_SPRITE) * 8);

		_disable();
		if(total_count & 1){
			pchr_data = &chr_data[spr_count - 1];
			for(i = spr_count - 1, j = 0; j < (MAX_SPRITE - message_count); i--, j++){
//				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
				_fmemcpy(spram, pchr_data, 4 * 2);
				pchr_data--;
				spram += 4;
			}
			pchr_data = &chr_data[message_count - 1];
			for(i = message_count - 1; i >= 0; i--){
//				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
				_fmemcpy(spram, pchr_data, 4 * 2);
				pchr_data--;
				spram += 4;
			}

		}else{
			pchr_data = &chr_data[MAX_SPRITE - 1];
			for(i = MAX_SPRITE - 1; i >= 0; i--){
//				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
				_fmemcpy(spram, pchr_data, 4 * 2);
				pchr_data--;
				spram += 4;
			}
		}
		spr_on(MAX_SPRITE);
		_enable();
		old_count = MAX_SPRITE;
	}else{
		spram = MK_FP(0x130, (1024 - spr_count) * 8);
		pchr_data = &chr_data[spr_count - 1];
		_disable();
		for(i = spr_count - 1; i >= 0; i--){
//			PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
			_fmemcpy(spram, pchr_data, 4 * 2);
			pchr_data--;
			spram += 4;
		}
		spr_on(spr_count);
		_enable();
//		_fmemcpy(spram, chr_data, spr_count * 4 * 2);

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

