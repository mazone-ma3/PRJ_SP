/* 入出力設定サブル－チン群 for X680x0 */

#define X68K

#include "sp.h"
#include "subfunc.h"

#include <iocslib.h>

void grp_set(void);
void spr_set(void);
void spr_fill(int);
void spr_clear(void);
/* void spr_make(void); */


/* 画面設定 */
void grp_set(void){
/*	CRTMOD(10); */
	CRTMOD(6);
	B_CUROFF();
	G_CLR_ON();
}

/* 画面を戻す */
void grp_term(void){
	B_CURON();
	CRTMOD(16);
}


/* スプライトを全て画面外に移す */
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
/* スプライト配置 */
/*  SPR_setAttribute(P_num, X_sum, Y_sum, Attr, Col_tbl); */
/*  SPR_setPotition(Size, P_num, X_sum, Y_sum, X, Y); */

void spr_on(int num){
	SP_ON();
}

void spr_off(void){
	SP_OFF();
}

/* スプライトパタ－ン設定 */
void spr_set(void){
	spr_off();

/* スプライト定義 */
/*	SPR_init(); */
	spr_clear(); 
/*	spr_fill(SPBACKCOLOR); */
/*	spr_make(); */
}


/* パッドの読込 */
void pad_read(int port, int *a, int *b, int *pd){
	unsigned char *reg = (unsigned char *)0xe9a001;
	int data;
	int ab;

/*	data = JOYGET(port);*/
	data = reg[port * 2];

/* 方向 */
	*pd = (data & 0x0f) ^ 0x0f;

/* ボタン */
	ab = ((data >> 5) & 0x03) ^ 0x03;
	*a = (ab >> 1) & 0x01;
	*b = ab & 0x01;
}

