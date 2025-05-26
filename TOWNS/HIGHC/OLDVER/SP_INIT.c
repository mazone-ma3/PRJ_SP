/* �������ƏI������ */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <snd.h>
//#include <msvdrv.h>
//#include <wpkload.h>
//#include <FMCFRB.H>
#include <TOWNS/segment.h>

#include "sp.h"
/*#include "spmake.h"*/
#include "sp68_ld.h"
#include "sp_main.h"
#include "subfunc.h"
#include "fonttw.h"

#include "sp_init.h"

#define PUT_SP( x, y, no, atr) {\
	_poke_word(0x130, spram, x); \
	spram += 2; \
	_poke_word(0x130, spram, y); \
	spram += 2; \
	_poke_word(0x130, spram, no); \
	spram += 2; \
	_poke_word(0x130, spram, atr); \
	spram += 2; \
}
/*#define PUT_SP( x, y, no, atr) {\
	*(spram++) = x; \
	*(spram++) = y; \
	*(spram++) = no; \
	*(spram++) = atr; \
}*/

/* �T�E���h�ݒ� */

/*#include "play.h"*/
/* char *eup_filename; */
/* char *eup_dat; */

char *msv_filename;
//char *wpk_filename;
char sndwork[16384];
//char MSVwork[MSVWorkSize];

int  se_steptime[4] , datalen[4] , pcmsw[4] , tp[4];
char *msv_mml[4] = {
	"@33v15o4c1",
	"@34v15o4c1",
	"@35v15o4c1&c1&c1",
	"@36v15o4c1&c1",
};

char msv_se[4][256];

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

/*�p���b�g�E�Z�b�g*/
void pal_set(int pal_no, unsigned short color, unsigned short red, unsigned short green,
	unsigned short blue)
{
	int palram;
	switch(pal_no){
		case CHRPAL_NO:
//			_Far unsigned short *palram;

//			_FP_SEG(palram) = 0x130;
//			_FP_OFF(palram) = 0x2000;
			palram = 0x2000;

			green = ((green + 1)*2-1)*(green!=0);
			blue = ((blue + 1)*2-1)*(blue!=0);
			red = ((red + 1)*2-1)*(red!=0);

//			palram[color] = green * 32 * 32 + red * 32 + blue;
			_poke_word(0x130, palram + color * 2,  green * 32 * 32 + red * 32 + blue);
			break;

		case BGPAL_NO:
			green = ((green + 1)*16-1)*(green!=0);
			blue = ((blue + 1)*16-1)*(blue!=0);
			red = ((red + 1)*16-1)*(red!=0);

			outp(0x448,0x01);
			outp(0x44a,0x01);	/* priority register */

			outp(0xfd90, color);
			outp(0xfd92, blue);
			outp(0xfd94, red);
			outp(0xfd96, green);
			break;
	}
}


void pal_all(int pal_no, unsigned char pal[16][3])
{
	unsigned char i;
	for(i = 0; i < 16; i++)
//		pal_set(pal_no, i, ((pal[i][0] + 1)*2-1) * (pal[i][0] != 0), ((pal[i][2]+1)*2-1) * (pal[i][2] != 0), ((pal[i][1]+1)*2-1) * (pal[i][1] != 0));
		pal_set(pal_no, i, pal[i][0], pal[i][1], pal[i][2]);
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync2();
}


//�p���b�g�f�[�^�𔽓]���A�w�肵���p���b�g�ԍ��ɃZ�b�g����B
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

//value < 0 ���ɋ߂Â���B
//value = 0 �ݒ肵���F
//value > 0 ���ɋ߂Â���B
void set_constrast(int value, unsigned char org_pal[16][3], int pal_no)
{
	int j, k;
	unsigned char pal[3];


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

//wait�l�̑��x�ō�����t�F�[�h�C������B
void fadeinblack(unsigned char org_pal[16][3], int pal_no, int wait)
{
	int j;

	for(j = -15; j <= 0; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait�l�̑��x�ō��Ƀt�F�[�h�A�E�g����B
void fadeoutblack(unsigned char org_pal[16][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j != -16; j--){
		sys_wait(wait);
		set_constrast(j, org_pal, pal_no);
	}
}

//wait�l�̑��x�Ŕ��Ƀt�F�[�h�A�E�g����B
void fadeoutwhite(unsigned char org_pal[16][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j < 16; j++){
		sys_wait(wait);
		set_constrast(j, org_pal, CHRPAL_NO);
		set_constrast(j, org_pal, BGPAL_NO);
	}
}

//�p���b�g���Ó]����B
void pal_allblack(int pal_no)
{
	char j;
	for(j = 0; j < 16; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

void paint(unsigned short color)
{
	unsigned short i, j;

	for (i = 0; i < (512); ++i){
		for (j = 0; j < 256; ++j){
//			_FP_OFF(vram) = (j + i * 1024 + 32) / 2;
//			*vram = color;
			VRAM_putPixelW((j + i * 1024 + 32) / 2, color);
		}
	}
}

/*�e�L�X�g��ʋy�уO���t�B�b�N��ʂ̏���*/
void clear(short type)
{
	if(type & 1){
		paint(0x0);
	}
	if(type & 2)
		printf("\x1b*");
}

char SNDBUFF[4][SND_BUFFSIZE];
//char WPKWK[SND_BUFFSIZE * 4];

char *SND_load(char *fn, char*SNDBUFF){
	FILE *fp;
	
	if ((fp = fopen(fn,"rb")) == NULL)
		return NULL;
	
	fread(SNDBUFF, SND_BUFFSIZE, 1, fp);
	fclose(fp);
	
	return SNDBUFF;
}

int	main(int argc,char **argv){
	short i, j;
	short errlv;

//	_FP_SEG(vram)=0x120;
//	_FP_SEG(spram)=0x130;

/* ���s���������ݒ肳��Ă��邩�ǂ������ׂ� */
/*	if (argc < 2 ) */
	/*	return 1;*/
	if (argv[1] == NULL){
		msv_filename = "C_1_TWN.MSV";	/* �������Ȃ������ꍇ */
//		wpk_filename = "SE.WPK";
	}else{
		msv_filename = argv[1];
//		wpk_filename = NULL;
	}


/* 	eup_filename =argv[1]; */

	if ((SND_load("se1.snd", &SNDBUFF[0][0])) == NULL)
		return ERROR;
	if ((SND_load("se2.snd", &SNDBUFF[1][0])) == NULL)
		return ERROR;
	if ((SND_load("se3.snd", &SNDBUFF[2][0])) == NULL)
		return ERROR;
	if ((SND_load("se4.snd", &SNDBUFF[3][0])) == NULL)
		return ERROR;


/* �T�E���h���C�u�����̏����� */
/* 	if (eup_init()) */
/* 		return ERROR; */
	SND_init(sndwork);
	SND_pcm_mode_set( 1 ); 
//	MSV_init(MSVwork,MSVWorkSize,EXPMODE);
	SND_elevol_all_mute(-1);
	SND_elevol_mute(0x33);

/* �ȃf�|�^�̓ǂ݂��� */
/* �T�E���h���C�u�����ɓn�� */
/*	if ((eup_dat = eup_load(eup_filename)) == NULL)
	{
		eup_term();
		return ERROR;
	}
*/
//	MSV_init(MSVwork,MSVWorkSize,EXPMODE);

/*	if ((MSV_load(msv_filename)) == ERROR)
	{
		MSV_end();
		return ERROR;
	}*/
/*	wpk_filename = MSV_get_wpkname(MSVwork);
	if(wpk_filename != NULL){
		if ((MSV_wpk_load(wpk_filename, MSVwork)) == ERROR)
		{
			MSV_end();
			return ERROR;
		}
	}
*/
//	MSVC_set_playcount(MSVWK->playcount);
	for(i = 0 ; i < 4; ++i){
		pcmsw[i] = 1;	/* PCM�p�̃R���p�C�� */
		tp[i] = 3;		/* MS3�`�����W�� */
//		MSVC_line_compile(msv_mml[i] , msv_se[i] , &se_steptime[i] , &datalen[i] , pcmsw[i] , tp[i]);
	}

//	SND_pcm_play( 71, 60, 127, SNDBUFF );
//	do{}while(SND_pcm_status(71));  /*  �������[�h�Đ��I���҂�*/
//	SND_pcm_play_stop(71) ;


/* ��ʏ����� */
	grp_set();
/*	grp_fill(BACKCOLOR); */
	spr_set(SPBACKCOLOR);

//	_FP_SEG(spram)=0x130;
//	_FP_OFF(spram) = 0x4000; // + 256*SPRSIZEX*SPRSIZEY;
	spram = 0x4000;
	font_load("FONTYOKO.SC5", FONTPARTS); //SPRPARTS);
	title_load("TITLE.SC5", TITLEPARTS, 64);

/* 	spload("STAGE1.PTN", 0x4000); */
/*	spload("STAGE1.COL", 0x2000); */
/* 	spload("CORECRA.SP", 0x4000); */
	sp68_load("CORECRA.SP", SPRPARTS);
	pal68_load("CORECRA.PAL");
/*	spsave("CORECRA.PTN", 0x4000);*/

/* ���y�Đ� */
/* 	eup_play(eup_dat, 1); */
//	MSV_play_start();

/* �Q�|�������s */
	spr_on(MAX_SPRITE);	/* �X�v���C�g����̊J�n */
	scrl_spd = SCRL_MIN;
	scrl = 0;

/* �Q�|�������s */

	spr_clear();
	spr_count = old_count = 0;
	do{
		wait_vsync2();
		pal_allblack(BGPAL_NO);
		pal_allblack(CHRPAL_NO);
		paint(0x0);
		init_star();
		pal_all(BGPAL_NO, org_pal);
		if(((errlv = title_demo()) == SYSERR) || (errlv == SYSEXIT))
			break;
		else if(errlv == NOERROR){
			for(j = 0; j != -16 * 8; j--){
				wait_vsync2();
				set_constrast(j / 8, org_pal, BGPAL_NO);
				set_constrast(j / 8, org_pal, CHRPAL_NO);
				bg_roll();
			}
			wait_vsync2();
			pal_allblack(BGPAL_NO);
			paint(0x0);
			opening_demo();

		}else if(errlv >= ERRLV1){
//			MSV_play_stop();
//			while(MSV_stat_flag());
//			MSV_play_start();

//			init_chr_data();
			old_count = MAX_SPRITE;

			for(i = 0; i < 192 + 8; i += 8){
				spr_count = 0;
				put_titlelogo( 128 - 48 - i, 48 );
				score_displayall();
				wait_sprite();
//				bg_roll();
				set_sprite();
				wait_vsync();
				bg_roll();
				spr_count = 0;
				put_titlelogo( 128 - 48 + i, 48 );
				score_displayall();
				wait_sprite();
//				bg_roll();
				set_sprite();
				wait_vsync();
				bg_roll();
			}
//			spr_clear();
			errlv = game_run(errlv);
			if(errlv == SYSEXIT)
				break;
//			MSV_fader(255,-100,1000/4);
			if(errlv != NOERROR){
				for(j = 0; j < 16 * 8; j++){
					wait_sprite();
					set_sprite();
					wait_vsync();
					set_constrast(j / 8, org_pal, CHRPAL_NO);
					set_constrast(j / 8, org_pal, BGPAL_NO);
					bg_roll();
				}
			}
		}
		wait_sprite();
		spr_clear();
		wait_vsync2();
	}while(KYB_read( 1, &encode ) != 0x1b);

end:
	pal_allblack(BGPAL_NO);
	pal_allblack(CHRPAL_NO);

/* ��ʂ�߂�(�R���\�|���Ή�) */

	spr_off();
	grp_term();

/* ���y���t��~ */
//	if(MSV_stat_flag());
//		MSV_fader(255,-100,1000/4);
//	while(MSV_stat_flag());

/* �T�E���h���C�u�����̊J�� */
/* 	eup_stop(); */
/* 	eup_term(); */
	SND_elevol_mute(0x00);
//	MSV_end();
	SND_end();

	return NOERROR;
}
