/* ��ʏ������ƃX�v���C�g�֌W�ƃp�b�h */

#define TOWNS

#include "sp.h"
#include "subfunc.h"

#include <egb.h>
#include <snd.h>
#include <spr.h>
#include <dos.h>
#include <conio.h>

char egb_work[1536];

/* ��ʐݒ� */
void grp_set(void){

	char para[64];

	EGB_init(egb_work, 1536);

/* 31kHz�o�͗p */
	EGB_resolution(egb_work, 0, 3);			/* �y�|�W0��640x480/16 */
	EGB_resolution(egb_work, 1, 5);			/* �y�|�W1��256x512/32768 */
	EGB_displayPage(egb_work, 1, 3);		/* ��ɂ���y�|�W��1�ŗ����Ƃ��\�� */

/* ����15kHz�o�͂������Ȃ炱������ */
/* 	EGB_resolution(egb_work, 0, 8);			/* �y�|�W0��512x256/32768 */
/* 	EGB_resolution(egb_work, 1, 11);		/* �y�|�W1��256x512/32768 */

	EGB_writePage(egb_work, 0);				/* �y�|�W0(BG)�̐ݒ� */
	EGB_displayStart(egb_work, 2, 2, 2);		/* �\���g�嗦(�c��2�{) */
	EGB_displayStart(egb_work, 3, 640, 480);	/* EGB��ʂ̑傫��(640x480) */
	EGB_displayStart(egb_work, 0, 0, 0);		/* �\���J�n�ʒu */
	EGB_displayStart(egb_work, 1, 0, 0);		/* ���z��ʒ��̈ړ� */

	EGB_writePage(egb_work, 1);				/* �y�|�W1(�X�v���C�g)�̐ݒ� */
	EGB_displayStart(egb_work, 2, 2, 2);		/* */
	EGB_displayStart(egb_work, 3, 256, 240);	/* */
	EGB_displayStart(egb_work, 0, 32, 0);		/* */
	EGB_displayStart(egb_work, 1, 0, 2);		/* ����2�h�b�g���炷(�d�l) */

	EGB_color(egb_work, 0, 0x8000);				/* �y�|�W1���N���A�X�N���|�� */
	EGB_color(egb_work, 2, 0x8000);				/* �����F�Ŗ��߂� */
	EGB_writePage(egb_work, 1);
	EGB_clearScreen(egb_work);
}

/* �F�h��(�e�X�g�p) */
void grp_fill(int backcolor){
	char para[64];

	EGB_writePage(egb_work, 0);
	EGB_paintMode(egb_work, 0x22);
	EGB_color(egb_work, 0, backcolor);
	EGB_color(egb_work, 2, backcolor);
	WORD(para + 0) = 32;
	WORD(para + 2) = 0;
	WORD(para + 4) = 256 - 32;
	WORD(para + 6) = 239;
	EGB_rectangle(egb_work, para);
}

/* ��ʂ�߂�(�R���\�|������̋N���΍�) */
void grp_term(void){
	EGB_color(egb_work, 0, 0x8000);				/* �y�|�W1���N���A�X�N���|�� */
	EGB_color(egb_work, 2, 0x8000);				/* �����F�Ŗ��߂� */
	EGB_writePage(egb_work, 1);
	EGB_clearScreen(egb_work);

	EGB_resolution(egb_work, 0, 4);		/* �y�|�W0��640x400/16 */
	EGB_resolution(egb_work, 1, 4);		/* �y�|�W1��640x400/16 */
	EGB_displayPage(egb_work, 0, 3);

	EGB_writePage(egb_work, 0);			/* �y�|�W0���N���A�X�N���|�� */
	EGB_clearScreen(egb_work);
	EGB_displayStart(egb_work,0,0,0);
	EGB_displayStart(egb_work, 1, 0, 0);
	EGB_displayStart(egb_work,2,1,1);
	EGB_displayStart(egb_work, 3, 640, 400);
	EGB_writePage(egb_work, 1);			/* �y�|�W1���N���A�X�N���|�� */
	EGB_clearScreen(egb_work);
	EGB_displayStart(egb_work,0,0,0);
	EGB_displayStart(egb_work, 1, 0, 0);
	EGB_displayStart(egb_work,2,1,1);
	EGB_displayStart(egb_work, 3, 640, 400);
}

/* �X�v���C�g�p�^�|���ݒ� */
void spr_set(int spbackcolor){
/* �X�v���C�g��` */
	SPR_init();
	spr_clear();
 	spr_fill(spbackcolor);
/*	spr_fill2(7); */
/* 	spr_make(); */
}

/* �X�v���C�g�ʂ̓h��Ԃ��ݒ� */
/* �^�C�����O�ɂ���Ƌ^���������ɂ��Ȃ� */
void spr_fill(int spbackcolor){
	int i;

/* �_�u���o�b�t�@�Ȃ̂�2�x�s�� */
/* �o�b�t�@0 */
//	_FP_OFF(vram) = 0x40000;
	vram = 0x40000;
	i = 512;
	while(i--);
		_poke_word(0x120, vram++, spbackcolor);
//		*vram++ = spbackcolor;

/* �o�b�t�@1 */
//	_FP_OFF(vram) = 0x60000;
	i = 512;
	while(i--);
		_poke_word(0x120, vram++, spbackcolor);
//		*vram++ = spbackcolor;
}

/* �^�������� */
/* [100000 00000 00000][0001 11 00 111 0 0111] */
/* 0x642 0xc63 0x1ce7 */
/* [8000 7fff] */
void spr_fill2(int level){
	int i,j;

	j = ((((level << 5) | level) << 5) | level);

/* �o�b�t�@0 */
//	_FP_OFF(vram) = 0x40000;
	vram = 0x40000;
	i = 128;
	while(i--){
		_poke_word(0x120, vram++, 0x8000);
		_poke_word(0x120, vram++, j);
//		*vram++ = 0x8000;
//		*vram++ = j;
	}
	i = 128;
	while(i--){
		_poke_word(0x120, vram++, j);
		_poke_word(0x120, vram++, 0x8000);
//		*vram++ = j;
//		*vram++ = 0x8000;
	}
/* �o�b�t�@1 */
//	_FP_OFF(vram) = 0x60000;
	vram = 0x60000;
	i = 128;
	while(i--){
		_poke_word(0x120, vram++, j);
		_poke_word(0x120, vram++, 0x8000);
//		*vram++ = j;
//		*vram++ = 0x8000;
	}
	i = 128;
	while(i--){
		_poke_word(0x120, vram++, 0x8000);
		_poke_word(0x120, vram++, j);
//		*vram++ = 0x8000;
//		*vram++ = j;
	}
}

/* �X�v���C�g��S�ĉ�ʊO�Ɉڂ� */
void spr_clear(void){
	int i;
//	_FP_OFF(spram) = 0;
	spram = 0;
	for(i = 0; i < 1024; i++){
		_poke_word(0x130, spram, 0);
		spram += 2;
//		*(spram++) = 0;
		_poke_word(0x130, spram, SCREEN_MAX_Y + 2);
		spram += 2;
//		*(spram++) = SCREEN_MAX_Y + 2; ///240;
		_poke_word(0x130, spram, 0);
		spram += 2;
//		*(spram++) = 0x0; //3ff;
		_poke_word(0x130, spram, 0x2000);
		spram += 2;
//		*(spram++) = 0x2000; //2fff;
	}
}
/* �X�v���C�g�z�u */
/*   SPR_setAttribute(P_num, X_sum, Y_sum, Attr, Col_tbl); */
/*   SPR_setPotition(Size, P_num, X_sum, Y_sum, X, Y); */

void spr_on(int num){
//	SPR_display(1, num);
	outp(0x450, 0);
	outp(0x452, (1024 - num) % 256);

	outp(0x450, 1);
	outp(0x452, 0x80 | (((1024 - num) / 256) & 0x03));
}

void spr_off(void){
	outp(0x450, 0);
	outp(0x452, 0xff);
	outp(0x450, 1);
	outp(0x452, 0x7f);
//	SPR_display(0,1);
}

/* �p�b�h�̓Ǎ� */
void pad_read(int port, int *a, int *b, int *pd){
	int data, ab;

/*	SND_joy_in_2(port, &data); */
 	data = inportb(0x4d0 + port * 2); 

/*	*s = ((data >> 6) & 0x03) ^ 0x03;*/
	ab = ((data >> 4) & 0x03) ^ 0x03;
	*a = ab & 0x01;
	*b = (ab >> 1) & 0x01;

	*pd = (data & 0x0f) ^ 0x0f;
}

