/* ���o�͐ݒ�T�u���|�`���Q for X680x0 */

#define X68K

#include "sp.h"
#include "subfunc.h"

#include <x68k/iocs.h>

void grp_set(void);
void spr_set(void);
void spr_fill(int);
void spr_clear(void);
/* void spr_make(void); */


/* ��ʐݒ� */
void grp_set(void){
/*	CRTMOD(10); */
	_iocs_crtmod(6);
	_iocs_ms_init();
	_iocs_skey_mod(0, 0, 0);
	_iocs_ms_curof();
	_iocs_b_curoff();
	_iocs_g_clr_on();
}

/* ��ʂ�߂� */
void grp_term(void){
//	_iocs_ms_curon();
	_iocs_skey_mod(-1, 0, 0);
	_iocs_b_curon();
	_iocs_crtmod(16);
}


/* �X�v���C�g��S�ĉ�ʊO�Ɉڂ� */
void spr_clear(void){
	int i;

	spram = (unsigned short *)0xeb0000;
	for(i = 0;i < 128; i++){
		*(spram++) = 0;
		*(spram++) = SCREEN_MAX_Y; //256;
		*(spram++) = 0;
		*(spram++) = 0; 
	}
}
/* �X�v���C�g�z�u */
/*  SPR_setAttribute(P_num, X_sum, Y_sum, Attr, Col_tbl); */
/*  SPR_setPotition(Size, P_num, X_sum, Y_sum, X, Y); */

void spr_on(int num){
	_iocs_sp_on();
}

void spr_off(void){
	_iocs_sp_off();
}

/* �X�v���C�g�p�^�|���ݒ� */
void spr_set(void){
	spr_off();

/* �X�v���C�g��` */
/*	SPR_init(); */
	spr_clear(); 
/*	spr_fill(SPBACKCOLOR); */
/*	spr_make(); */
}


/* �p�b�h�̓Ǎ� */
void pad_read(int port, int *a, int *b, int *pd){
	unsigned char *reg = (unsigned char *)0xe9a001;
	int data;
	int ab;

/*	data = JOYGET(port);*/
	data = reg[port * 2];

/* ���� */
	*pd = (data & 0x0f) ^ 0x0f;

/* �{�^�� */
	ab = ((data >> 5) & 0x03) ^ 0x03;
	*a = (ab >> 1) & 0x01;
	*b = ab & 0x01;
}

