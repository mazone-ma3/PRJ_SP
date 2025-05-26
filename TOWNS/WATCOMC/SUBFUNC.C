/* 画面初期化とスプライト関係とパッド */
/* OpenWatcom版 参考:yama氏(github:yamayamaru)の実験結果 */

#define TOWNS

#include "sp.h"
#include "subfunc.h"

//#include <egb.h>
//#include <snd.h>
//#include <spr.h>
//#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <i86.h>

//char egb_work[1536];

#define WORKSIZE 256*1024
int work01[WORKSIZE];

char *workaddr;
int  worksize;

/* 画面設定 */
void grp_set(void){
	workaddr = (char *)work01;
	worksize = WORKSIZE;
	
	__asm {
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

		push	ds
		pop	gs
		mov	edi,workaddr
		mov	 ecx,worksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,00h					; EGB 初期化
		call	pword ptr fs:0020h
		cmp	 ah, 0
		jnz	 gmode_end

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,01h					; 仮想画面1の設定
		mov	 al,0
		mov	 dx, 3;mode01
		call	pword ptr fs:0020h
		cmp	 ah, 0
		jnz	 gmode_end

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,01h					; 仮想画面2の設定
		mov	 al,1
		mov	 dx, 5
		call	pword ptr fs:0020h
		cmp	 ah, 0
		jnz	 gmode_end

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,05h					; 書き込みページ
		mov	 al,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end

;//	EGB_displayStart(egb_work, 2, 2, 2);		/* 表示拡大率(縦横2倍) */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,2
		mov	dx,2
		mov	bx,2
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end

;//	EGB_displayStart(egb_work, 3, 640, 480);	/* EGB画面の大きさ(640x480) */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,3
		mov	dx,640
		mov	bx,480
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end

;//	EGB_displayStart(egb_work, 0, 0, 0);		/* 表示開始位置 */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,0
		mov	dx,0
		mov	bx,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end

;//	EGB_displayStart(egb_work, 1, 0, 0);		/* 仮想画面中の移動 */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,1
		mov	dx,0
		mov	bx,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end



		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,05h					; 書き込みページ
		mov	 al,1
		call	pword ptr fs:0020h
		cmp	 ah, 0
		jnz	 gmode_end

;//	EGB_displayStart(egb_work, 2, 2, 2);		/* */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,2
		mov	dx,2
		mov	bx,2
		call	pword ptr fs:0020h
		cmp	 ah, 0
		jnz	 gmode_end

;//	EGB_displayStart(egb_work, 3, 256, 240);	/* */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,3
		mov	dx,256
		mov	bx,240
		call	pword ptr fs:0020h
		cmp	 ah, 0
		jnz	 gmode_end

;//	EGB_displayStart(egb_work, 0, 32, 0);		/* */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,0
		mov	dx,32
		mov	bx,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
		jnz	 gmode_end

;//	EGB_displayStart(egb_work, 1, 0, 2);		/* 下に2ドットずらす(仕様) */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,1
		mov	dx,0
		mov	bx,2
		call	pword ptr fs:0020h
		cmp	 ah, 0
		jnz	 gmode_end



		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,06h					; 表示ページ
		mov	 al,1
		mov	 edx,0x03
		call	pword ptr fs:0020h
;		cmp	 ah, 0
;		jnz	 gmode_end

gmode_end:
//		mov	 ret01,ah

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
	}
//	return (int)ret01;
//}
//	if(ret01)
//		return;

//	EGB_init(egb_work, 1536);

/* 31kHz出力用 */
//	EGB_resolution(egb_work, 0, 3);			/* ペ－ジ0は640x480/16 */
//	EGB_resolution(egb_work, 1, 5);			/* ペ－ジ1は256x512/32768 */
//	EGB_displayPage(egb_work, 1, 3);		/* 上にくるペ－ジは1で両方とも表示 */

/* もし15kHz出力したいならこうする */
/* 	EGB_resolution(egb_work, 0, 8);			/* ペ－ジ0は512x256/32768 */
/* 	EGB_resolution(egb_work, 1, 11);		/* ペ－ジ1は256x512/32768 */

//	EGB_writePage(egb_work, 0);				/* ペ－ジ0(BG)の設定 */
//	EGB_displayStart(egb_work, 2, 2, 2);		/* 表示拡大率(縦横2倍) */
//	EGB_displayStart(egb_work, 3, 640, 480);	/* EGB画面の大きさ(640x480) */
//	EGB_displayStart(egb_work, 0, 0, 0);		/* 表示開始位置 */
//	EGB_displayStart(egb_work, 1, 0, 0);		/* 仮想画面中の移動 */

//	EGB_writePage(egb_work, 1);				/* ペ－ジ1(スプライト)の設定 */
//	EGB_displayStart(egb_work, 2, 2, 2);		/* */
//	EGB_displayStart(egb_work, 3, 256, 240);	/* */
//	EGB_displayStart(egb_work, 0, 32, 0);		/* */
//	EGB_displayStart(egb_work, 1, 0, 2);		/* 下に2ドットずらす(仕様) */

//	EGB_color(egb_work, 0, 0x8000);				/* ペ－ジ1をクリアスクリ－ン */
//	EGB_color(egb_work, 2, 0x8000);				/* 透明色で埋める */
//	EGB_writePage(egb_work, 1);
//	EGB_clearScreen(egb_work);
}

/* 色塗り(テスト用) */
void grp_fill(int backcolor){
//	char para[64];

//	EGB_writePage(egb_work, 0);
//	EGB_paintMode(egb_work, 0x22);
//	EGB_color(egb_work, 0, backcolor);
//	EGB_color(egb_work, 2, backcolor);
//	WORD(para + 0) = 32;
//	WORD(para + 2) = 0;
//	WORD(para + 4) = 256 - 32;
//	WORD(para + 6) = 239;
//	EGB_rectangle(egb_work, para);
}

/* 画面を戻す(コンソ－ルからの起動対策) */
void grp_term(void) {
	workaddr = (char *)work01;
	worksize = WORKSIZE;

	__asm {
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

		push	ds
		pop	gs
		mov	edi,workaddr
		mov	 ecx,worksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,00h					; EGB 初期化
		call	pword ptr fs:0020h
;		cmp	 ah, 0
;		jnz	 gmode_end2


		mov	ax,0110h
		mov	fs,ax
		mov	 ah,07h					; 描画色
		mov	 al,0
		mov	edi,workaddr
		mov	 edx,8000h
		call	pword ptr fs:0020h

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,07h					; 描画色
		mov	 al,2
		mov	edi,workaddr
		mov	 edx,8000h
		call	pword ptr fs:0020h

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,05h					; 書き込みページ
		mov	 al,1
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,21h					; クリアスクリーン
		call	pword ptr fs:0020h



;//	EGB_resolution(egb_work, 1, 4);		/* ペ－ジ1は640x400/16 */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,01h					; 仮想画面の設定
		mov	 al,1
		mov	 dx, 4
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

;//	EGB_resolution(egb_work, 0, 4);		/* ペ－ジ0は640x400/16 */

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,01h					; 仮想画面の設定
		mov	 al,0
		mov	 dx, 4
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2


; ページ1

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,05h					; 書き込みページ
		mov	 al,1
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,21h					; クリアスクリーン
		call	pword ptr fs:0020h


;//	EGB_displayStart(egb_work,0,0,0);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,0
		mov	dx,0
		mov	bx,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

;//	EGB_displayStart(egb_work, 1, 0, 0);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,1
		mov	dx,0
		mov	bx,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

;//	EGB_displayStart(egb_work,2,1,1);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,2
		mov	dx,1
		mov	bx,1
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

;//	EGB_displayStart(egb_work, 3, 640, 400);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,3
		mov	dx,640
		mov	bx,400
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2



; ページ0

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,05h					; 書き込みページ
		mov	 al,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,21h					; クリアスクリーン
		call	pword ptr fs:0020h

;//	EGB_displayPage(egb_work, 0, 3);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,06h					; 表示ページ
		mov	 al,0
		mov	 edx,0x03
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

;//	EGB_writePage(egb_work, 1);			/* ペ－ジ0をクリアスクリ－ン */
;//	EGB_clearScreen(egb_work);


;//	EGB_displayStart(egb_work,0,0,0);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,0
		mov	dx,0
		mov	bx,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

;//	EGB_displayStart(egb_work, 1, 0, 0);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,1
		mov	dx,0
		mov	bx,0
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

;//	EGB_displayStart(egb_work,2,1,1);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,2
		mov	dx,1
		mov	bx,1
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2

;//	EGB_displayStart(egb_work, 3, 640, 400);

		mov	ax,0110h
		mov	 fs,ax
		mov	 ah,02h
		mov	 al,3
		mov	dx,640
		mov	bx,400
		call	pword ptr fs:0020h
		cmp	 ah, 0
;		jnz	 gmode_end2


		jp	gmode_end3
gmode_end2:
//		mov	 ret01,ah

gmode_end3:
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
	}
//	return (int)ret01;
//	EGB_color(egb_work, 0, 0x8000);				/* ペ－ジ1をクリアスクリ－ン */
//	EGB_color(egb_work, 2, 0x8000);				/* 透明色で埋める */
//	EGB_writePage(egb_work, 1);
//	EGB_clearScreen(egb_work);

//	EGB_resolution(egb_work, 0, 4);		/* ペ－ジ0は640x400/16 */
//	EGB_resolution(egb_work, 1, 4);		/* ペ－ジ1は640x400/16 */
//	EGB_displayPage(egb_work, 0, 3);

//	EGB_writePage(egb_work, 0);			/* ペ－ジ0をクリアスクリ－ン */
//	EGB_clearScreen(egb_work);
//	EGB_displayStart(egb_work,0,0,0);
//	EGB_displayStart(egb_work, 1, 0, 0);
//	EGB_displayStart(egb_work,2,1,1);
//	EGB_displayStart(egb_work, 3, 640, 400);
//	EGB_writePage(egb_work, 1);			/* ペ－ジ1をクリアスクリ－ン */
//	EGB_clearScreen(egb_work);
//	EGB_displayStart(egb_work,0,0,0);
//	EGB_displayStart(egb_work, 1, 0, 0);
//	EGB_displayStart(egb_work,2,1,1);
//	EGB_displayStart(egb_work, 3, 640, 400);
}

/* スプライトパタ－ン設定 */
void spr_set(int spbackcolor){
/* スプライト定義 */
//	SPR_init();
	spr_clear();
 	spr_fill(spbackcolor);
/*	spr_fill2(7); */
/* 	spr_make(); */
}

/* スプライト面の塗りつぶし設定 */
/* タイリングにすると疑似半透明にもなる */
void spr_fill(int spbackcolor){
	int i;

/* ダブルバッファなので2度行う */
/* バッファ0 */
//	_FP_OFF(vram) = 0x40000;
	vram = MK_FP(0x120, 0x40000);
	i = 512;
	while(i--)
		*vram++ = spbackcolor;

/* バッファ1 */
//	_FP_OFF(vram) = 0x60000;
	vram = MK_FP(0x120, 0x60000);
	i = 512;
	while(i--)
		*vram++ = spbackcolor;
}

/* 疑似半透明 */
/* [100000 00000 00000][0001 11 00 111 0 0111] */
/* 0x642 0xc63 0x1ce7 */
/* [8000 7fff] */
void spr_fill2(int level){
	int i,j;

	j = ((((level << 5) | level) << 5) | level);

/* バッファ0 */
//	_FP_OFF(vram) = 0x40000;
	vram = MK_FP(0x120, 0x40000);
	i = 128;
	while(i--){
		*vram++ = 0x8000;
		*vram++ = j;
	}
	i = 128;
	while(i--){
		*vram++ = j;
		*vram++ = 0x8000;
	}
/* バッファ1 */
//	_FP_OFF(vram) = 0x60000;
	vram = MK_FP(0x120, 0x60000);
	i = 128;
	while(i--){
		*vram++ = j;
		*vram++ = 0x8000;
	}
	i = 128;
	while(i--){
		*vram++ = 0x8000;
		*vram++ = j;
	}
}

/* スプライト配置 */
/*   SPR_setAttribute(P_num, X_sum, Y_sum, Attr, Col_tbl); */
/*   SPR_setPotition(Size, P_num, X_sum, Y_sum, X, Y); */

void spr_on(int num){
	short num01 = (short)num;
#ifndef DEBUG
	_disable();
	_outp(0x450, 0);
	_outp(0x452, (1024 - num) % 256);

	_outp(0x450, 1);
	_outp(0x452, 0x80 | (((1024 - num) / 256) & 0x03));
	_enable();

	return;
#else
//	SPR_display(1, num);
	__asm {
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

;		push	ds
;		pop	gs

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,01h
		mov	 al,1
		mov	cx,num01

;		mov	edi,workaddr
;		mov	 edx,color01

		call	pword ptr fs:0060h

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
	}
#endif
}

void spr_off(void){
#ifndef DEBUG
/*	spr_clear();*/
	_outp(0x450, 1);
	_outp(0x452, 0x7f);
	_outp(0x450, 0);
	_outp(0x452, 0xff);

	return;
#else
/*__asm{
	cli
	mov	cx,450h
	mov	al,0
	out	cx,al

	mov	al,1
	mov	cx,452h
	out	cx,al

	mov	cx,450h
	mov	al,1
	out	cx,al

	mov	cx,452h
	mov	al,0
	out	cx,al

	sti
}*/

//	return;

//	SPR_display(0,1);
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

;		mov	ax,0110h
;		mov	fs,ax
;		mov	 ah,00h
;		call	pword ptr fs:0060h

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,01h
		mov	 al,2
		mov	 cx,1

		call	pword ptr fs:0060h

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,01h
		mov	 al,0
		mov	 cx,1

		call	pword ptr fs:0060h

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,01h
		mov	 al,2
		mov	 cx,1

		call	pword ptr fs:0060h

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

		sti
	}

/*	_outp(0x450, 0);
	_outp(0x452, 0);
	_outp(0x450, 1);
	_outp(0x452, 0);
*/
//	spr_fill(0x8000);
#endif
}

/* スプライトを全て画面外に移す */
void spr_clear(void){
	int i;

//	spr_off();
	while(1){
		_disable();
		if(!(_inp(0x044c) & 0x02)) /* READY */
			break;
		_enable();
	}
//	_FP_OFF(spram) = 0;
	spram = MK_FP(0x130, 0);
	for(i = 0; i < 1024; i++){
		*(spram++) = 0;
		*(spram++) = SCREEN_MAX_Y + 2; ///240;
		*(spram++) = 0x0; //3ff;
		*(spram++) = 0x2000; //2fff;
	}
	_enable();

//	spr_on(MAX_SPRITE);
}


/* パッドの読込 */
void pad_read(int port, int *a, int *b, int *pd){
	int data, ab;

/*	SND_joy_in_2(port, &data); */
 	data = _inp(0x4d0 + port * 2); 

/*	*s = ((data >> 6) & 0x03) ^ 0x03;*/
	ab = ((data >> 4) & 0x03) ^ 0x03;
	*a = ab & 0x01;
	*b = (ab >> 1) & 0x01;

	*pd = (data & 0x0f) ^ 0x0f;
}


