
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "sp.h"
//#include "subfunc.h"
#include "sp_main.h"


#include "list.h"

/*#include "sp_ene.h"*/
#include "sp_sht.h"
#include "sp_ene.h"

/* �v���g�^�C�v�w�� */

void game_init(void);		/* �Q�[���̏����� */
short game_loop(void);		/* �Q�[���{�� */

void wait_vsync(void);
void set_sprite(void);

#define FALSE 0
#define TRUE 1


#define INIT_HSC 5000

int scrlspd = 0;
long score = 0, hiscore = INIT_HSC;

#define SCREEN2 0

#ifdef SPROW_ON
unsigned char hit_check = FALSE;
#endif

typedef void (*SUBFUNC)(int i); 


#define DMGTIME 3
#define SHOTCOUNT 32
#define URAMAX 4

#define SCORE_MAX 99999999L
#define STAGEMAX 99

/* �X�v���C�g�\���f�|�^��VRAM�ɏ������ރ}�N�� */
/* ���W���V�t�g���ď������݃f�|�^�͐����C���N�������g */
/*#define PUT_SP( X, Y, X_SIZE, Y_SIZE, ADR) {\
	*(spram++) = Y % 256;\
	*(spram++) = Y_SIZE | ((Y / 256) & 0x01);\
	*(spram++) = X % 256;\
	*(spram++) = X_SIZE | ((X / 256) & 0x03);\
	*(spram++) = (ADR) % 256;\
	*(spram++) = (ADR) / 256;\
	*(spram++) = 0;\
	*(spram++) = 0;\
}*/

#define PUT_SP( X, Y, X_SIZE, Y_SIZE, ADR) {\
	*(spram++) = (Y_SIZE) | (Y & 0x01ff) | 0x0200;\
	*(spram++) = (X_SIZE) | (X & 0x03ff);\
	*(spram++) = (ADR);\
	*(spram++) = 0;\
}

CHR_PARA teki_data[MAX_TEKI];		/* �G�f�|�^ */


/*CHR_PARA shot_data[30];*/		/* ���@�̒e�f�|�^ */

/*
1���� ����
2���� �ł��Ԃ�
3���� �ʏ�e����
4���� �ł��Ԃ�3����
5���ڈȍ~ �C���^�[�o���Ȃ�
*/

#define HP_MAX 10
#define SELENMAX 75

int command_num;
int waitcount = 0;
int schedule_ptr;
int *command;

unsigned char noshotdmg_flag;
unsigned char uramode;

unsigned char my_hp_flag;

unsigned char seflag = 0;


/* ���@��� */
CHR_PARA my_data[2];		/* ���@�f�|�^ */

/* ���@�e��� */
/*CHR_OBJ*/
CHR_PARA myshot[MAX_MYSHOT + 2];				/* ���@�e���W�Ǘ� */
unsigned char myshot_next[MAX_MYSHOT + 2];
unsigned char myshot_free[MAX_MYSHOT + 2];

int mypal;
int mypal_dmgtime;
int my_movecount;
int my_hp;
int max_my_hp;

/* �G��� */
SPR teki[MAX_TEKI];					/* �G���W�Ǘ� */
unsigned int tkcount[MAX_TEKI];		/* �G�ړ��p�J�E���^ */
unsigned char teki_pat[MAX_TEKI];			/* �G�p�^�[���ԍ� */
int teki_dir[MAX_TEKI];				/* �G�ړ������ۑ��p�ϐ� */

unsigned char teki_next[MAX_TEKI + 2];			/* �G�\���t���O */
unsigned char teki_free[MAX_TEKI + 2];			/* �G�\���t���O */

unsigned int tkshotcount[MAX_TEKI];	/* �G�e���˃J�E���^ */

unsigned char teki_hp[MAX_TEKI];
int teki_pal[MAX_TEKI];
int teki_dmgtime[MAX_TEKI];

int teki_move[MAX_TEKI];
int teki_shotinfo[MAX_TEKI];

int teki_num = 0;
int stage;
int tmp;
int timeup;
unsigned char trgcount = 0;	/* �V���b�g�Ԋu���~�b�^ */
unsigned char trgcount2 = 0;	/* �A�˃��~�b�^ */
int trgnum = 0;
int total_count = 0;
int renshaflag = TRUE; /*FALSE;*/

/* �G�e��� */
SPR tkshot[MAX_TKSHOT];				/* �G�e���W�Ǘ� */

unsigned char tkshot_next[MAX_TKSHOT + 1];
unsigned char tkshot_free[MAX_TKSHOT + 1];

int tkshot_xx[MAX_TKSHOT];			/* �G�e�ړ�����X */
int tkshot_yy[MAX_TKSHOT];			/* �G�e�ړ�����Y */

unsigned char tkshot_pat[MAX_TKSHOT];

int tkshotnum;						/* �G�e ���݂̕\���� */
int tkshot_c;						/* �G�e���ˊԊu �����l */

int scrdspflag;						/* �X�R�A�����X�V�t���O */


/*SPR_COMB spr[MAX_SPRITE * 3];*/
/*const */

/* �e���|�����p�O���[�o���ϐ� */
unsigned char chrnum;
int chrpal;
const SPR_COMB *sprcomb;

int tmp_x, tmp_y;
int tmp_xx, tmp_yy;

int spr_count,old_count;
int *px, *py;

COLIGION *p_coli;

/******************************************************************************/

/* �G�e�����}�N�� */

#define del_tkshot(NO) {\
	tkshot[NO].y = tmp_y = SPR_DEL_Y;\
	tkshotnum--;\
}

#define put_my_hp() my_hp_flag = TRUE;

void put_my_hp_dmg(void)
{
	int i, j = 0;
	char str_temp[11];

	for(i = 0; i < my_hp; i++)
		str_temp[j++] = '`';
	for(i = my_hp; i < HP_MAX; i++)
		str_temp[j++] = ' ';
	str_temp[j] = '\0';

	put_strings(SCREEN2, 4, 7, str_temp, CHRPAL_NO);

	my_hp_flag = TRUE;
}

void my_dmg(void)
{
	if(my_hp)
	{
		if(!mypal_dmgtime){
			--my_hp;
			put_my_hp();
			seflag = 1;
		}
	}
	mypal = REVPAL_NO;
	mypal_dmgtime = DMGTIME * 2;

	noshotdmg_flag = TRUE;
}


/*****************************/
/* �G�@�e�ړ���������(32way) */
/*****************************/

int tekishot_dir(void)
{
	int l = 0;

	tmp_yy = my_data[0].x - tmp_x;
	tmp_xx = my_data[0].y - tmp_y + scrlspd;

	if(tmp_xx < 0){
		tmp_xx = -tmp_xx;
		if(tmp_yy < 0){
			l = 16;
			tmp_yy = -tmp_yy;
		}else{
			l = 24;
		}
	}else if(tmp_yy < 0){
		l = 8;
		tmp_yy = -tmp_yy;
	}

	if((l == 24) || (l == 8)){
		if(tmp_yy < tmp_xx){
			tmp_yy *= 8;
			if((tmp_yy) < tmp_xx)
				l += 0;
			else if((tmp_yy) < tmp_xx * 3)
				l += 1;
			else if((tmp_yy) < (tmp_xx * 5))
				l += 2;
			else if((tmp_yy) < (tmp_xx * 7))
				l += 3;
			else
				l += 4;
		}else{
			tmp_xx *= 8;
			if((tmp_yy * 7) < (tmp_xx))
				l += 4;
			else if((tmp_yy * 5) < (tmp_xx))
				l += 5;
			else if((tmp_yy * 3) < (tmp_xx))
				l += 6;
			else if((tmp_yy) < (tmp_xx))
				l += 7;
			else{
				l += 8;
				l %= 32;
			}
		}
	}else{
		if(tmp_yy < tmp_xx){
			tmp_yy *= 8;
			if((tmp_yy) < tmp_xx){
				l += 8;
				l %= 32;
			}
			else if((tmp_yy) < (tmp_xx * 3)){
				l += 7;
			}
			else if((tmp_yy) < (tmp_xx * 5))
				l += 6;
			else if((tmp_yy) < (tmp_xx * 7))
				l += 5;
			else
				l += 4;
		}else{
			tmp_xx *= 8;
			if((tmp_yy * 7) < (tmp_xx))
				l += 4;
			else if((tmp_yy * 5) < (tmp_xx))
				l += 3;
			else if((tmp_yy * 3) < (tmp_xx))
				l += 2;
			else if(tmp_yy < (tmp_xx))
				l += 1;
		}
	}
	return l;
}

/**********************/
void shottome(int dir, int pat)
{
	int j;

	if(tkshot_free[MAX_TKSHOT] != END_LIST){
		ADD_LIST(MAX_TKSHOT, j, tkshot_next, tkshot_free);

		if(uramode > 1){
			tkshot[j].x = tmp_x + (4 << SHIFT_NUM) + (tkshot_xx[j] = direction[dir][0] * 3 / 2);
			tkshot[j].y = tmp_y + (4 << SHIFT_NUM) + (tkshot_yy[j] = direction[dir][1] * 3 / 2);
		}else{
			tkshot[j].x = tmp_x + (4 << SHIFT_NUM) + (tkshot_xx[j] = direction[dir][0]);
			tkshot[j].y = tmp_y + (4 << SHIFT_NUM) + (tkshot_yy[j] = direction[dir][1]);
		}
		tkshot_pat[j] = pat;
		tkshotnum++;
	}
}

void shottome2(int dir, int pat)
{
	int j;

	if(tkshot_free[MAX_TKSHOT] != END_LIST){
		ADD_LIST(MAX_TKSHOT, j, tkshot_next, tkshot_free);
		tkshot[j].x = tmp_x + (4 << SHIFT_NUM) + (tkshot_xx[j] = direction[dir][0] * 3 / 2);
		tkshot[j].y = tmp_y + (4 << SHIFT_NUM) + (tkshot_yy[j] = direction[dir][1] * 3 / 2);

		tkshot_pat[j] = pat;
		tkshotnum++;
	}

}


/**********************/
/* �G�@�e���˃T�u�֐� */
/**********************/
void shotcheck1(int i)	/* NORMAL */
{
	if(tkshotcount[i] != 0){
		tkshotcount[i]--;
		return;
	}

	tkshotcount[i] = tkshot_c;
	shottome(tekishot_dir(), PAT_TKSHOT1);
}

void shotcheck2(int i)	/* 3 WAY */
{
	int l;

	if(tkshotcount[i] != 0){
		tkshotcount[i]--;
		return;
	}

	tkshotcount[i] = tkshot_c;
	l = tekishot_dir();

	shottome(l, PAT_TKSHOT1);
	shottome((l + 2) % 32, PAT_TKSHOT1);
	shottome((l - 2 + 32) % 32, PAT_TKSHOT1);
}

void shotcheck3(int i)	/* RANDOM */
{
	if(tkshotcount[i] != 0){
		tkshotcount[i]--;
		return;
	}

	tkshotcount[i] = tkshot_c;
	shottome(rand() % 32, PAT_TKSHOT1);
}

void shotcheck4(int i)	/* LASER */
{
	if(tkshotcount[i] == 0){
		return;
	}
	if(tkshotcount[i]-- < (SHOTCOUNT - 1)){
		if(tkshotcount[i] & 2)
			shottome(DIR_DOWN, PAT_TKSHOT2);
	}
}

void dummy(int i)
{
}

/************/

#define SHOTCHECKMAX 7

SUBFUNC shotcheck_tbl[SHOTCHECKMAX] ={
	shotcheck1,
	shotcheck2,
	shotcheck3,
	shotcheck4,
	dummy,
	dummy,
	dummy,
};

void do_shotcheck(int i)
{
/*	SUBFUNC p;*/
	int j;

	j = teki_shotinfo[i];
	if(j < SHOTCHECKMAX)
		if(j >= 0)
			shotcheck_tbl[j](i);
}

/******************/
/* �G�@�e�ړ��T�u */
/******************/
void move_tekishot(void)
{
	int i, j, k;

	SEARCH_LIST2(MAX_TKSHOT, i, j, tkshot_next){
		k = tkshot_pat[i];
		tmp_x = (tkshot[i].x += (tkshot_xx[i]));

		/* �G�@�e��ʊO���� */
		if((unsigned short)(tmp_x) > ((SCREEN_MAX_X + 16) << SHIFT_NUM)){
			del_tkshot(i);
			tmp_y = tkshot[i].y;
			DEL_LIST(MAX_TKSHOT, i, j, tkshot_next, tkshot_free);
		}else{
			tmp_y = tkshot[i].y += (tkshot_yy[i] - scrlspd);
			if((unsigned short)(tmp_y) > ((SCREEN_MAX_Y + 16) << SHIFT_NUM)){
				del_tkshot(i);
				DEL_LIST(MAX_TKSHOT, i, j, tkshot_next, tkshot_free);

			}/* �G�@�e�Ǝ��@�̓����蔻�聨���@�_���[�W */
			else{
#ifdef SPROW_ON
				if(hit_check == TRUE){
#endif
					if((tmp_xx = my_data[0].x - tmp_x) < ((TKD_OFS_X - MY_OFS_X + TKD_SIZE_X) << SHIFT_NUM)){
						if(tmp_xx > ((TKD_OFS_X - MY_OFS_X - MY_SIZE_X) << SHIFT_NUM)){
							if((tmp_yy = my_data[0].y - tmp_y) < ((TKD_OFS_Y - MY_OFS_Y + TKD_SIZE_Y) << SHIFT_NUM)){
								if(tmp_yy > ((TKD_OFS_Y - MY_OFS_Y - MY_SIZE_Y) << SHIFT_NUM)){
									/* ���@�_���[�W */
									del_tkshot(i);
									DEL_LIST(MAX_TKSHOT, i, j, tkshot_next, tkshot_free);
									my_dmg();
								}
							}
						}
					}
#ifdef SPROW_ON
				}
#endif
				DEF_SP(spr_count, tmp_x, tmp_y, k, CHRPAL_NO);

			}
		}
	}
}

void check_revshot(void)
{
	int l;

	if(uramode > 2){
		l = tekishot_dir();

		shottome2(l, PAT_TKSHOT1);
		shottome2((l + 2) % 32, PAT_TKSHOT1);
		shottome2((l - 2 + 32) % 32, PAT_TKSHOT1);
	}else
		shottome2(tekishot_dir(), PAT_TKSHOT1);
}

/***********************/
/* �G�����p�^�[������1 */
/***********************/
void move_teki_bomb(int i)
{
	switch(tkcount[i]){
		case 0:
			seflag = 1;
			teki_pat[i] = PAT_BAKU1;
			break;

		case 5:
			teki_pat[i] = PAT_BAKU2;
			break;

		case 10:
			teki_pat[i] = PAT_BAKU3;
			if(uramode)
				check_revshot();
			break;

		case 13:
			if(uramode)
				check_revshot();
			break;

		case 15:
			if(uramode)
				check_revshot();
			teki_hp[i] = 0;
			teki[i].y = SPR_DEL_Y;
			break;
	}

	tkcount[i]++;
}

/***********************/
/* �G�ړ��p�^�[������1 */
/* HENTAI SINWAVE      */
/***********************/
void move_teki_pat1(int i)
{
	tmp_x -= sinwave[tkcount[i] / SINRATE] * ((3 << SHIFT_NUM) / 2);

	if(++tkcount[i] >= (SINFREQ * SINRATE))
		tkcount[i] = 0;

	tmp_x += sinwave[tkcount[i] / SINRATE] * ((3 << SHIFT_NUM) / 2);

	/* �G�@�c�����ړ����� */
	if(!(tmp_y > (128 << SHIFT_NUM)))
		tmp_y += ((3 << SHIFT_NUM) / 2);	/* ������� */
	else
		tmp_y += (2 << SHIFT_NUM);	/* ���� */

	/* �G�@��ʊO����(������) */
	if(tmp_y > (SPR_MAX_Y)){
		teki_hp[i] = 0;
	}
}

/***********************/
/* �G�ړ��p�^�[������2 */
/* FIRE2               */
/***********************/
void move_teki_pat2(int i)
{
	int dir;

	if(++tkcount[i] < 800){
	if(tmp_x < (16 << SHIFT_NUM))
		teki_dir[i] = tekishot_dir();

	if(tmp_y < (16 << SHIFT_NUM))
		teki_dir[i] = tekishot_dir();

	if(tmp_x > (SCREEN_MAX_X << SHIFT_NUM))
		teki_dir[i] = tekishot_dir();

	if(tmp_y > (SCREEN_MAX_Y << SHIFT_NUM))
		teki_dir[i] = tekishot_dir();
	}else{
		/* �G�@��ʊO����(������) */
		if(tmp_x < 0)
			teki_hp[i] = 0;

		/* �G�@��ʊO����(�E����) */
		if(tmp_x > (SPR_MAX_X))
			teki_hp[i] = 0;

		/* �G�@��ʊO����(�����) */
		if(tmp_y < 0)
			teki_hp[i] = 0;

		/* �G�@��ʊO����(������) */
		if(tmp_y > (SPR_MAX_Y))
			teki_hp[i] = 0;
	}
	dir = teki_dir[i];
	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);
}

/***********************/
/* �G�ړ��p�^�[������3 */
/* GURUGURU            */
/***********************/
void move_teki_pat3(int i)
{
	int dir, dir2;

	if(tkcount[i] > 800){
		dir = teki_dir[i];
		/* �G�@��ʊO����(������) */
		if(tmp_x < 0)
			teki_hp[i] = 0;

		/* �G�@��ʊO����(�E����) */
		if(tmp_x > (SPR_MAX_X))
			teki_hp[i] = 0;

	}else if((++tkcount[i] % 7) == 1){
		dir2 = tekishot_dir();
		dir = teki_dir[i];
//		if( (DIR_OFF + dir - dir2 + DIR_OFF/2) % DIR_OFF > (DIR_DOWN + DIR_OFF/2) )
		if((32 + dir - dir2) % 32 > 16)
			dir++;
		else
			dir--;
		dir = teki_dir[i] = (dir + DIR_OFF) % DIR_OFF;
	}else
		dir = teki_dir[i];

	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);

//	teki_pat[i] = teki5_pat[dir / 2];

	/* �G�@��ʊO����(�����) */
	if(tmp_y < 0)
		if(direction[dir][1] < 0)
			teki_hp[i] = 0;

	/* �G�@��ʊO����(������) */
	if(tmp_y > (SPR_MAX_Y))
		teki_hp[i] = 0;


}

/***********************/
/* �G�ړ��p�^�[������4 */
/* FIRE1               */
/***********************/
void move_teki_pat4(int i)
{
	int dir;

	if(tkcount[i] > 800){
		dir = teki_dir[i];
		tmp_x +=  (direction[dir][0]);
		tmp_y +=  (direction[dir][1]);

		/* �G�@��ʊO����(������) */
		if(tmp_x < 0)
			teki_hp[i] = 0;

		/* �G�@��ʊO����(�E����) */
		if(tmp_x > (SPR_MAX_X))
			teki_hp[i] = 0;

	}else if(!tkcount[i] || !(tkcount[i] % 192)){
		++tkcount[i];
		dir = teki_dir[i] = tekishot_dir();

		tmp_x +=  (direction[dir][0]);
		tmp_y +=  (direction[dir][1]);
	}else if(((tkcount[i] % 192) < 80)){ 
		++tkcount[i];
		dir = teki_dir[i];

		tmp_x +=  (direction[dir][0]);
		tmp_y +=  (direction[dir][1]);

		if(tmp_x < (16 << SHIFT_NUM))
			teki_dir[i] = tekishot_dir();
		if(tmp_y < (16 << SHIFT_NUM))
			teki_dir[i] = tekishot_dir();
		if(tmp_x > (SCREEN_MAX_X << SHIFT_NUM))
			teki_dir[i] = tekishot_dir();
		if(tmp_y > (SCREEN_MAX_Y << SHIFT_NUM))
			teki_dir[i] = tekishot_dir();

	}else
		++tkcount[i];

	/* �G�@��ʊO����(�����) */
	if(tmp_y < 0)
		if(direction[dir][1] < 0)
			teki_hp[i] = 0;

	/* �G�@��ʊO����(������) */
	if(tmp_y > (SPR_MAX_Y))
		teki_hp[i] = 0;

}

/***********************/
/* �G�ړ��p�^�[������5 */
/* HERIZAKO            */
/***********************/
void move_teki_pat5(int i)
{
	int dir;

	if(!tkcount[i]){
		dir = teki_dir[i] = tekishot_dir();
		++tkcount[i];

	}else{
		dir = teki_dir[i];
		if(dir != DIR_UP){
			if(++tkcount[i] > 102){
				tkcount[i] = 96;
				dir = DIR_UP;
//				if(direction[dir][0] > 0){
//					++dir;	/* ����] */
//					dir %= DIR_OFF;
//				}else{
//					--dir;	/* �E��] */
//				}
				teki_dir[i] = dir;
			}
		}
	}
	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);


	/* �G�@��ʊO����(�����) */
	if(dir == DIR_UP){
		if(tmp_y < 0){
			teki_hp[i] = 0;
		}
	}
	if(tmp_y > (SPR_MAX_Y)){
		teki_hp[i] = 0;
	}
}

/***********************/
/* �G�ړ��p�^�[������6 */
/* ���[�U�[            */
/***********************/
void move_teki_pat6(int i)
{
	int dir;

	dir = teki_dir[i];
	if(tkcount[i] == 120){
		dir = tekishot_dir();
		teki_shotinfo[i] = 0;
	}else if(tkcount[i] == 40){
		dir = DIR_OFF;
		tkshotcount[i] = SHOTCOUNT * 2;
		teki_shotinfo[i] = 3;
	}

	teki_dir[i] = dir;
	++tkcount[i];

	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);


	/* �G�@��ʊO����(������) */
	if(tmp_y > (SPR_MAX_Y)){
		teki_hp[i] = 0;
	}
	/* �G�@��ʊO����(�����) */
	if(tmp_y < 0){
		if(direction[dir][1] < 0){
			teki_hp[i] = 0;
		}
	}
	/* �G�@��ʊO����(������) */
	if(tmp_x < 0)
		teki_hp[i] = 0;

	/* �G�@��ʊO����(�E����) */
	if(tmp_x > (SPR_MAX_X))
		teki_hp[i] = 0;
}

/*************************/

void boss_tkappend(int i, int pat, int move, int dir, int shot)
{
	int j;

	if(teki_free[MAX_TEKI] == END_LIST)
		return;

	ADD_LIST(MAX_TEKI, j, teki_next, teki_free);

	teki_pat[j] = pat;
	tkcount[j] = 0;
	tkshotcount[j] = SHOTCOUNT - 2;

	teki_move[j] = move;
	teki[j].y = teki[i].y;
	teki[j].x = teki[i].x;

	teki_hp[j] = 10;
	teki_pal[j] = CHRPAL_NO;
	teki_shotinfo[j] = shot;
	teki_dir[j] = dir;
	teki_num++;
}

/*************************/
/* �{�X�ړ��p�^�[������1 */
/*************************/
void move_boss_pat1(int i)
{
	int dir;
	dir = teki_dir[i];

	if(++tkcount[i] == 40){
		dir = DIR_RIGHT;
	}else if((tkcount[i] % 600) == 40){
		tkshotcount[i] = SHOTCOUNT * 3 / 2;
		dir = DIR_DOWN;
	}else if(tkcount[i] > 40){
		switch(dir){
			case DIR_UP:
				if(tmp_y < (48 << SHIFT_NUM)){
					dir = DIR_OFF;
					tkshotcount[i] = SHOTCOUNT * 3 / 2;
					boss_tkappend(i, PAT_TEKI4, 6, DIR_DOWN - 4, 0);
					boss_tkappend(i, PAT_TEKI4, 6, DIR_DOWN + 4, 0);
					boss_tkappend(i, PAT_TEKI4, 6, DIR_DOWN, 0);
				}
				break;

			case DIR_RIGHT:
				if(tmp_x > ((320 - 24) << SHIFT_NUM)){
					dir = DIR_LEFT;
				}
				break;

			case DIR_DOWN:
				if(tmp_y > ((192 - 16) << SHIFT_NUM)){
					tkshotcount[i] = SHOTCOUNT * 3 / 2;
					if(tkcount[i] < 3500)
						dir = DIR_UP;
				}
				break;

			case DIR_LEFT:
				if(tmp_x < (40 << SHIFT_NUM)){
					dir = DIR_RIGHT;
				}
				break;

			case DIR_OFF:
				if((tkcount[i] % 200) == 50){
					dir = DIR_LEFT;
				}
				break;
		}
	}
	/* �G�@��ʊO����(������) */
	if(tmp_y > (SPR_MAX_Y))
		teki_hp[i] = 0;

	tmp_x +=  (direction[dir][0]);
	tmp_y +=  (direction[dir][1]);
	teki_dir[i] = dir;

}
/************/

#define MOVETEKIMAX 10

SUBFUNC moveteki_tbl[MOVETEKIMAX] = 
{
	move_teki_bomb,
	move_teki_pat1,
	move_teki_pat2,
	move_teki_pat3,
	move_teki_pat4,
	move_teki_pat5,
	move_teki_pat6,
	dummy,
	dummy,
	move_boss_pat1,
};

/************/
/* �G�@�ړ� */
/************/
void move_teki(void)
{
	int i, j, k, l;

	SEARCH_LIST2(MAX_TEKI, i, k, teki_next){

		/* �G�@�ړ����� */
		tmp_x = teki[i].x;
		tmp_y = teki[i].y;

		l = teki_move[i];

		if(l >= 0)
			if(l < MOVETEKIMAX)
				moveteki_tbl[l](i);

		teki[i].x = tmp_x;
		teki[i].y = tmp_y;

		if((teki_hp[i] > 2)){
#ifdef SPROW_ON
			if(hit_check == TRUE){
#endif
				/* �G�@�Ǝ��@�e�̐ڐG���� �� �G�@�A���@�e���� �X�R�A���� */
				SEARCH_LIST(MAX_MYSHOT, j, myshot_next){
//					if((tmp_yy = myshot[j].y - tmp_x) < (16 << SHIFT_NUM)){
//						if(tmp_yy > ((TK_OFS_Y - MY_OFS_Y - 8) << SHIFT_NUM)){
					if((tmp_xx = myshot[j].x - tmp_x) < (16 << SHIFT_NUM)){
//						if(tmp_xx > (-20 << SHIFT_NUM)){
						if(tmp_xx > (-(4 << SHIFT_NUM))){
							if((tmp_yy = myshot[j].y - tmp_y) < (16 << SHIFT_NUM)){
//								if(tmp_yy > (-20 << SHIFT_NUM)){
								if(tmp_yy > (-(4 << SHIFT_NUM))){

									/* �G�_���[�W */
									teki_pal[i] = REVPAL_NO;
									teki_dmgtime[i] = DMGTIME;

									--teki_hp[i];

									if(uramode)
										shottome2(tekishot_dir(), PAT_TKSHOT1);

									/* �m�[�}���e�Ȃ��ʊO�ɏ��� */
									myshot[j].y = 0; /*SPR_DEL_Y;*/
									/*myshot[j].yy = 0;*/

									score += 10;		/* �G�̓_���𑫂� */
									scrdspflag = TRUE;
									break;
								}
							}
						}
					}
				}
#ifdef SPROW_ON
			}
#endif
		}

		if(teki_hp[i] > 2){
#ifdef SPROW_ON
			if (hit_check == TRUE){
#endif
				/* �G�@�Ǝ��@�̐ڐG���� �� ���@�_���[�W */
				p_coli = &hantei[0];
				if((tmp_xx = my_data[0].x - tmp_x) < p_coli->x1){
					if(tmp_xx > p_coli->x2){
						if((tmp_yy = my_data[0].y - tmp_y) < p_coli->y1){
							if(tmp_yy > p_coli->y2){

								/* ���@�_���[�W */
								my_dmg();
							}
						}
					}
				}
#ifdef SPROW_ON
			}
#endif
			do_shotcheck(i);
		}
		DEF_SP(spr_count, teki[i].x, teki[i].y, teki_pat[i], teki_pal[i]);

		/* �G���� �� �����p�^�[���� */
		if(!teki_hp[i]){
			teki[i].y = SPR_DEL_Y;
			DEL_LIST(MAX_TEKI, i, k, teki_next, teki_free);
			--teki_num;
		}
		else if(teki_hp[i] == 2){
			teki_hp[i] = 1;
			teki_move[i] = 0;
			tkcount[i]  = 0;
			teki_dmgtime[i] = 0;
			teki_pal[i] = CHRPAL_NO;

			score += 100;		/* �G�̓_���𑫂� */
			scrdspflag = TRUE;

		}else if(teki_dmgtime[i]){
			teki_dmgtime[i]--;
			if(!teki_dmgtime[i])
				teki_pal[i] = CHRPAL_NO;
		}
	}
}

/**********/
/* �G�o�� */
/**********/
void com_tkappend(int teki_no)
{
	int i;

	if(teki_free[MAX_TEKI] == END_LIST)
		return;

	ADD_LIST(MAX_TEKI, i, teki_next, teki_free);

	teki_pat[i] = teki_no;
	tkcount[i] = 0;
	tkshotcount[i] = SHOTCOUNT - 2;

	teki_move[i] = command[schedule_ptr + 2];
	teki[i].x = (((command[schedule_ptr + 3] - 3) << SHIFT_NUM) * 320L) / 144;
	teki[i].y = ((command[schedule_ptr + 4] << SHIFT_NUM) * 256) / 200;

	teki_hp[i] = command[schedule_ptr + 5];
	teki_pal[i] = CHRPAL_NO;

	teki_shotinfo[i] = command[schedule_ptr + 6];

	teki_dir[i] = DIR_DOWN;
	teki_num++;
}


/**************************************/

void com_waitcount(int i)
{
	waitcount = i;
}

void com_tkalldel(int i)
{
	waitcount = 1;
}

void com_bgmchange(int i)
{
/*	playbgm(i, 0);*/
}

void com_bgmfadeout(int i)
{
/*	S_IL_FUNC(bgm_fadeout());*/
}

void com_dummy(int i)
{
	waitcount = 1;
}

void com_putmessage(int i)
{
	put_strings(SCREEN2, command[schedule_ptr + 1], command[schedule_ptr + 2], (char *)message[command[schedule_ptr + 3]], CHRPAL_NO);
}

void com_putstage(int i)
{
	char str_x = command[schedule_ptr + 1];
	char str_y = command[schedule_ptr + 2];

	if(stage < STAGEMAX)
		++stage;

	put_strings(SCREEN2, str_x, str_y, "STAGE", CHRPAL_NO);
	if(stage > 0){
		if(stage < 10)
			put_numd((long)stage, 1);
		else
			put_numd((long)stage, 2);
		put_strings(SCREEN2, str_x, str_y + 6, str_temp, CHRPAL_NO);
	}
}

void com_end(int i)
{
	waitcount = 1;
}

void com_se(int i)
{
	seflag = command[schedule_ptr + 1];
}

/**************************************/
/* �X�P�W���[����͂ōs���R�}���h
�E�G�ǉ�
�E�����҂�
�EBGM�ύX(F-Out)
�E���b�Z�[�W�\��
�E�e��G�t�F�N�g
*/
/**************************************/

/* �X�P�W���[���p�W�����v�e�[�u�� enum�̒l�ƑΉ����� */
#define COMMAND_MAX 10

SUBFUNC command_tbl[COMMAND_MAX] = {
	com_dummy,
	com_waitcount,
	com_tkalldel,
	com_tkappend,
	com_bgmchange,
	com_bgmfadeout,
	com_putmessage,
	com_putstage,
	com_end,
	com_se,
};

/********************/
/* �X�P�W���[�����s */
/********************/
void do_schedule(void)
{
/*	SUBFUNC p;*/

	while(1){

	/* �C�x���g�����҂��Ȃ�߂� */
	if(waitcount){
		switch(command_num){
			case COM_WAITCOUNT:
				--waitcount;
				return;
				break;

			case COM_TKALLDEL:
				if(teki_next[MAX_TEKI] == END_LIST)
					waitcount = 0;
				return;
				break;

			case COM_DUMMY:
				return;
				break;

			case COM_END:
				--waitcount;
				schedule_ptr = 0;
				if(tkshot_c > (1 << SHIFT_NUM))
					tkshot_c /= 2;
				else{
					if(uramode != URAMAX){
						uramode++;
						if(uramode != URAMAX)
							tkshot_c = 6 << SHIFT_NUM;
						else
							tkshot_c = 0;
					}
				}
				if(!noshotdmg_flag){	/* PACIFIST */
					score += 10000;
					scrdspflag = TRUE;
					seflag = 4;
				}
				noshotdmg_flag = FALSE;

				if(my_hp < max_my_hp)
					my_hp = max_my_hp;
				else
					my_hp += 1;

				if(my_hp > HP_MAX)
					my_hp = HP_MAX;
				put_my_hp();
				break;
		}
	}

	command_num = command[schedule_ptr];	/* ���Ɏ��s����R�}���h */

	if(command_num >= 0)
		if(command_num < COMMAND_MAX)
/*			if((p = command_tbl[command_num]) != NULL)*/
				command_tbl[command_num](command[schedule_ptr + 1]);

	schedule_ptr += command_length[command_num];	/* �����̐�������ɐi�߂� */

	}
}

