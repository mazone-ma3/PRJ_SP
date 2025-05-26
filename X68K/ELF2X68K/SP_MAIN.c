/* �Q�|���{�� for X680x0 */

#define MAIN
#define X68K

#include <string.h>
#include <x68k/iocs.h>
#include "sp_init.h"
#include "play.h"

enum {
	BGMMAX = 2,
	SEMAX = 4
};


void wait_vsync(void);
void put_strings(int scr, int x, int y,  char *str, char pal);
void put_numd(long j, char digit);
char str_temp[9];

unsigned char spr_page = 1;
volatile unsigned char spr_flag = 0, spr_next = 0;
unsigned char tmp_spr_count = 0;
int old_count[2];

/******************************************************************************/
#include "sp_com.h"
/******************************************************************************/

short test_h_f = TRUE;
short soundflag = FALSE;

void put_strings(int scr, int y, int x,  char *str, char pal)
{
	char chr;
	unsigned short i = 0;
	unsigned short *bgram;
	bgram = (unsigned short *)0xebe000;
	bgram += (x * 2 + (28-y) * 0x80) / 2; /* BG1 */

	while((chr = *(str++)) != '\0'){
		if((chr < 0x30)) //|| (chr > 0x5f))
			chr = PCG_SPACE;
		else
			chr -= '0';
		*(bgram++) = chr;
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

	put_strings(SCREEN2, 13, 10, "HIGH", CHRPAL_NO);
	put_strings(SCREEN2, 13, 10 + 5, str_temp, CHRPAL_NO);
}

#include "inkey.h"

unsigned char keyscan(void)
{
	unsigned char k5, k6, k7, k8, k9, st, pd;
	unsigned char *reg = (unsigned char *)0xe9a001;
	unsigned short paddata;
	unsigned char keycode = 0;

	k5 = _iocs_bitsns(5);
	k6 = _iocs_bitsns(6);
	k7 = _iocs_bitsns(7);
	k8 = _iocs_bitsns(8);
	k9 = _iocs_bitsns(9);

	paddata = reg[0];
	st = (paddata & 0x0f); // ^ 0x0f;
	pd = ((paddata >> 5) & 0x03); // ^ 0x03;

	if((k5 & 0x04) || (k6 & 0x20) || !(pd & 0x01)) /* Z,SPACE */
		keycode |= KEY_A;
	if((k5 & 0x08) || !(pd & 0x02)) /* X */
		keycode |= KEY_B;
	if((k8 & 0x10) || (k7 & 0x10) || !(st & 0x01)) /* 8 */
		keycode |= KEY_UP1;
	if((k9 & 0x10) || (k7 & 0x40) || !(st & 0x02)) /* 2 */
		keycode |= KEY_DOWN1;

	if(!(st & 0x0c)){ /* RL */
		keycode |= KEY_START;
	}else{
		if((k8 & 0x80) || (k7 & 0x08) || !(st & 0x04)) /* 4 */
			keycode |= KEY_LEFT1;
		if((k9 & 0x02) || (k7 & 0x20) || !(st & 0x08)) /* 6 */
			keycode |= KEY_RIGHT1;
	}

	return keycode;
}

int opening_demo(void)
{
	signed int i, j;

	put_strings(SCREEN2, 16, 11, "PROJECT CC", CHRPAL_NO);
	put_strings(SCREEN2, 14, 11, "SINCE 199X", CHRPAL_NO);
	fadeinblack(org_pal, CHRPAL_NO, 6);
	j = 4;
	for(i = 0; i < 75 / 2 * 3; i++){
		if(keyscan() || ((_iocs_bitsns(0) & 2))){
			j = 2;
			break;
		}
		sys_wait(1);
	}
	fadeoutblack(org_pal, CHRPAL_NO, j);

	return NOERROR;
}

void put_title(void)
{
	put_strings(SCREEN2, 9, 14, "START", CHRPAL_NO);
	put_strings(SCREEN2, 7, 14, "EXIT", CHRPAL_NO);
	put_strings(SCREEN2, 4, 10, "      ij k   ", CHRPAL_NO);
	put_strings(SCREEN2, 3, 10, "a2022 bcdefgh", CHRPAL_NO);
	score_displayall();
	hiscore_display();
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
//	spr_count = old_count = 0;
	do{
		if(j < 0){
			j++;
			spr_count = 0;
			set_constrast(j / 8, org_pal, CHRPAL_NO);
			if(!j){
//				put_strings(SCREEN2, 8, 7, "NORMAL", CHRPAL_NO);
//				put_strings(SCREEN2, 6, 7, "HARD", CHRPAL_NO);

				put_title();

//				while(keyscan());
			}
		}

		if(!j){
			spr_count = 0;
			put_strings(SCREEN2, 7 + x * 2, 11, "?", CHRPAL_NO);
			keycode = keyscan();
			if(keycode)
				loopcounter = 0;
			if((keycode & KEY_DOWN1) && (x != 0)){
				put_strings(SCREEN2, 7 + x * 2, 11, " ", CHRPAL_NO);
				x = 0;
			}
			if((keycode & KEY_UP1) && (x != 1)){
				put_strings(SCREEN2, 7 + x * 2, 11, " ", CHRPAL_NO);
				x = 1;
			}
//			if(keycode & KEY_START)
//				return SYSEXIT;
			if(keycode & KEY_B){
				if(keycode & KEY_A){
					if(soundtest == FALSE){
						put_strings(SCREEN2, 2, 2, "SOUND TEST", CHRPAL_NO);
						put_numd(soundtestno, 3);
						put_strings(SCREEN2, 2, 13, str_temp, CHRPAL_NO);
						soundtest = TRUE;
					}else{
						put_strings(SCREEN2, 2, 2, "              ", CHRPAL_NO);
						soundtest = FALSE;
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
					put_strings(SCREEN2, 2, 13, str_temp, CHRPAL_NO);
				}
			}
		}
		wait_vsync();
		bg_roll();
		if(++loopcounter == WAIT1S * 30){
			return NOERROR;
		}
	}while(!(_iocs_bitsns(0) & 2));
	return SYSEXIT;
}

/* �Q�|���̃��|�v */
short game_loop(void){
	unsigned char a = 0, b = 0;
	int i, j, xx, yy;
	short *p_x,*p_y;
	int pat_no;
	unsigned char keycode;

	spr_count = 0;

/* �p�b�h���� & ���@�ړ� */
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
//				scrlspd = SCRL_MIN;
//			else{
//				put_strings(SCREEN2, 14, 6, "PAUSE", CHRPAL_NO);
//			}
//			do {
				if(scrl_spd)
					put_strings(SCREEN2, 14, 13, "PAUSE", CHRPAL_NO);
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
				if(keycode & KEY_A){
//				if(keycode & KEY_B){
//					bg_roll();
//					set_sprite();
					return SYSEXIT;		/* ��C�ɔ����� */
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
				wait_vsync();
				set_sprite();
			}while(keyscan() & (KEY_START | KEY_B));
			scrl_spd = SCRL_MIN;
			put_strings(SCREEN2, 14, 13, "     ", CHRPAL_NO);

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

/* ���@�ړ�(�΂ߕ����Ή�) */
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

		/* �΂߂̎��̏���(�蔲����) */
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

	/* ���@���ړ��ł���͈͂�ݒ� */
		if(*p_y <= JIKI_MIN_Y)
			*p_y = JIKI_MIN_Y;
			else if(*p_y >= JIKI_MAX_Y)
				*p_y = JIKI_MAX_Y;
			if(*p_x <= JIKI_MIN_X)
				*p_x = JIKI_MIN_X;
			else if(*p_x >= JIKI_MAX_X)
				*p_x = JIKI_MAX_X;

		DEF_SP(spr_count, *p_x, *p_y, my_data[i].pat_num, mypal);
/*	printf("%d %d \n", chr_data[spr_count - 1].pat_num, (mypal << 8));*/

//		mypal = CHRPAL_NO;

		/* ���@�e���� */
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

						myshot2[tmp].xx = 0;
						myshot2[tmp].yy = -(6 << SHIFT_NUM);

						myshot[tmp].pat_num = PAT_MYSHOT1;

						trgnum++;
					}
					if(myshot_free[MAX_MYSHOT] != END_LIST){
						ADD_LIST(MAX_MYSHOT, tmp, myshot_next, myshot_free);
						myshot[tmp].x = my_data[i].x + (12 << SHIFT_NUM);
						myshot[tmp].y = my_data[i].y;

						myshot2[tmp].xx = 0;
						myshot2[tmp].yy = -(6 << SHIFT_NUM);

						myshot[tmp].pat_num = PAT_MYSHOT1;

						trgnum++;
					}
				}
			}
		}else
			trgcount2 = 60 / 3;
	}

	
	/** �X�P�W���[����͂����s **/
	do_schedule();

	/* ���@�e�ړ� */
	SEARCH_LIST2(MAX_MYSHOT, i, j, myshot_next){
		tmp_x = myshot[i].x;
		tmp_y = myshot[i].y;

		tmp_x += myshot2[i].xx;
		tmp_y += myshot2[i].yy;

		/* ���@�e��ʊO���� */
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

/* �ϐ����������� */
void game_init(void){
	int i;

	srand(time(NULL));	/* �����̏����� */

	stage = 0;
	waitcount = 0;
	schedule_ptr = 0;
	command_num = COM_DUMMY;
	command = (int *)stg1_data;
	uramode = 0;
//	renshaflag = FALSE;
	renshaflag = TRUE;

	trgcount = 0;	/* �V���b�g�Ԋu���~�b�^ */
	trgcount2 = 0;	/* �A�˃��~�b�^ */
	trgnum = 0;
	total_count = 0;

//	scrl = 0;
	scrl_spd = SCRL_MIN;

	mypal = CHRPAL_NO;
	mypal_dmgtime = 0;
	my_movecount = 0;

	/* �G�\����񏉊��� */
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

/* ���X�g������(�܂����g�p) */
/*	for(i=0;i<10;i++){
		start[i].next = &fin[i];
		start[i].prev = NULL;
		fin[i].next = NULL;
		fin[i].prev = &start[i];
	}
*/

	/* SPRITE ������ */
/*	for(i = 0; i < 256; i++)
		spr[i].y = SPR_DEL_Y;*/

/* ���@���W�̏����� */
	my_data[0].x = 128 << SHIFT_NUM;
	my_data[0].y = 120 << SHIFT_NUM;
	my_data[0].pat_num = PAT_JIKI1;
	my_data[1].x = 192 << SHIFT_NUM;		/* ��l�v���C��z�� */
	my_data[1].y = 120 << SHIFT_NUM;
	my_data[1].pat_num = PAT_JIKI1;

/* ���e���W�̏����� */
/*	for(i=0; i<10; i++){
		shot_data[i].x = 0 << SHIFT_NUM;
		shot_data[i].xx = 0;
		shot_data[i].y = SPR_DEL_Y;
		shot_data[i].yy = 0;
		shot_data[i].pat_num = PAT_MYSHOT1;
	}*/


/* �G���W�̏����� */
/* �S�X�v���C�g�\�����ő�180�Ƃ����4�������˂ł�������30���x */
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
/* �X�^�|�̍��W�n�������� */
	for(i = 0;i < STAR_NUM; i++){
		star[0][i] = (i + 1) * (256 / STAR_NUM);
		star[1][i] = rand() % 512;
		star[2][i] = (rand() % 2) + 1;

		/* VRAM�̃A�h���X���Z�o */
		vram = (unsigned short *)0xc00000 + (star[0][i] + star[1][i]*512);
		star[3][i] = *vram;	/* ���̐F���L������ */

		star[4][i] = rand() % 14 + 2;
	}
/* �X�^�|�̕\��(�Œ�\��) */
	i = STAR_NUM;
	while(i--){
		vram = (unsigned short *)0xc00000 + (star[0][i] + ((256 / STAR_NUM) / 2) + 
			(star[1][i] * 256)) * 2;
		*vram |= star[4][i];
	}
}

void bg_roll(void)
{
	int i;
	register unsigned short *scroll = (unsigned short *)0xe8001a;	/* */

/* �X�N���|�����W�X�^���� */

		scrl += 512 - (scrl_spd >> SCRL_SFT);
		scrl %= 512;
		*scroll = scrl;

/* �X�N���|������X�^�|�̌v�Z */

		i = STAR_NUM;
		while(i--){
			vram = (unsigned short *)0xc00000 + (star[0][i] + (star[1][i] * 512)) ;
			*vram = star[3][i];
			star[1][i] += (star[2][i] + 512);
			star[1][i] %= 512;
//		}
//		i = STAR_NUM;
//		while(i--){
			vram = (unsigned short *)0xc00000 + (star[0][i] + (star[1][i] * 512)) ;
			star[3][i] = *vram;
			*vram |= star[4][i];
		}
}



/* �Q�|���{�̂̏��� */
short game_run(short mode){
	short i;
	unsigned char keycode;

//	init_star();

	my_hp_flag = TRUE;
	game_init();	/* �e�ϐ��̏����� */

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

	put_strings(SCREEN2, -3, 0, "SHIELD", CHRPAL_NO);

//	spr_count = old_count = 0;
/* �Q�[���̃��C�����[�v */
	do{
		if(mypal_dmgtime){
			--mypal_dmgtime;
			if(!mypal_dmgtime){
				mypal = CHRPAL_NO;
				if(!my_hp){
					timeup = 60 * 10;
					scrlspd = 0;
					put_strings(SCREEN2, 14, 10, "CONTINUE A", CHRPAL_NO);
					while(keyscan() & (KEY_A | KEY_START)){
						wait_vsync();
						set_sprite();
					}
					do{
						put_numd((long)(timeup / 60), 2);
						put_strings(SCREEN2, 10, 14, str_temp, CHRPAL_NO);

						if(!(--timeup)){
							put_strings(SCREEN2, 14, 10, "           ", CHRPAL_NO);
							put_strings(SCREEN2, 14, 10, " GAME OVER ", CHRPAL_NO);
							put_strings(SCREEN2, 10, 14, "  ", CHRPAL_NO);
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

					put_strings(SCREEN2, 14, 10, "           ", CHRPAL_NO);
					put_strings(SCREEN2, 10, 14, "  ", CHRPAL_NO);
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
			my_hp_flag = FALSE;
		}

		switch(game_loop()){
			case SYSEXIT:
				return NOERROR;
			case NOERROR:
				wait_vsync();
				set_sprite();
				bg_roll();
				break;
			default:
				spr_count = 0;
				continue;
		}
	}while((scrl_spd != 0) && !(_iocs_bitsns(0) & 2));

/* �I������ */
/* 	spr_off(); */
	return SYSEXIT;
}

volatile unsigned char vsync_flag = 0;

void wait_vsync(void)
{
	unsigned char volatile *vsync = (unsigned char *)0xe88001;	/* MFP */
/* VSYNC�҂� */
//	while(!((*vsync) & 0x10));	/* ������ */
//	while((*vsync) & 0x10);		/* ������ */
	while(!vsync_flag);
	vsync_flag = 0;
}

void set_sprite(void)
{
	int i, j;

//	wait_vsync();

/* �X�v���C�g�\�� */
	spdata = (unsigned short *)&chr_data2[spr_page]; //(unsigned short *)0xeb0000;

/*	printf("\x1b[0;0H%d \n", spr_count);*/

/* �\�����Ԃ񏑂����� */
	if(spr_count > MAX_SPRITE){
		if(total_count & 1){
/*			for(i = spr_count - MAX_SPRITE, j = 0; j < MAX_SPRITE; i++, j++){
				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
			}*/
			memcpy(spdata, &chr_data[spr_count - MAX_SPRITE], MAX_SPRITE * 4 * 2);
		}else{
/*			for(i = 0; i < MAX_SPRITE; i++){
				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
			}*/
			memcpy(spdata, chr_data, MAX_SPRITE * 4 * 2);
		}
		tmp_spr_count = MAX_SPRITE;
	}else{
/*		for(i = 0; i < spr_count; i++){
			PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
		}*/
		memcpy(spdata, chr_data, spr_count * 4 * 2);
		spdata += (spr_count * 4);

/* �X�v���C�g�̕\�������������ꍇ�������������ʊO�ɏ������� */
/* �����镪�ɂ͖��Ȃ� */
		if (old_count[spr_page] > spr_count){
			for(i = 0;i < (old_count[spr_page] - spr_count); i++){
				PUT_SP(0,SCREEN_MAX_Y,0,0);
			}
		}
		tmp_spr_count = spr_count;
	}
/* ���̃t���|���ŕ\�������X�v���C�g�̐���ۑ� */
	old_count[spr_page] = tmp_spr_count;
	tmp_spr_count = 0;

	spr_flag = 1;
	spr_next = spr_page;
	spr_page ^= 0x01;


	++total_count;

//	wait_vsync();

/*	if(spr_flag){
		spr_flag = 0;
		spram = (unsigned short *)0xeb0000;
		memcpy(spram, chr_data2[spr_next], MAX_SPRITE * 4 * 2);
	}*/

	if(seflag){
//		if(soundflag == TRUE)
//			if(se_check())
//				se_stop();
//		S_IL_FUNC(se_play(sndtable[0], seflag - 1));	/* ���ʉ� */
		if(mcd_status >= 0){
			pcm_play(&SNDBUFF[seflag - 1][0], pcmsize[seflag - 1]);
		}
		seflag = 0;
	}

//	while(spr_flag != 0);
}

//void 
void  __attribute__((interrupt)) int_vsync(void)
{
//	asm volatile(
//		"rte"
//	);
//	++score;
//	scrdspflag =TRUE;

	vsync_flag = 1;

	if(spr_flag){
		spr_flag = 0;
		spram = (unsigned short *)0xeb0000;
		memcpy(spram, chr_data2[spr_next], MAX_SPRITE * 4 * 2);
	}
}
