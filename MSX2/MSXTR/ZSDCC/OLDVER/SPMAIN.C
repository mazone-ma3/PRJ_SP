/* ゲ－ム本体 for MSX2 */

#define MAIN
#define MSX2

//#include <iocslib.h>
//#include "sp_init.h"
//#include "play.h"

enum {
	BGMMAX = 2,
	SEMAX = 4
};

void wait_vsync(void);
void put_strings(unsigned char scr, unsigned char x, unsigned char y,  char *str, unsigned char pal);
void put_numd(long j, unsigned char digit) __sdcccall(1);
//char str_temp[11];
void refresh_star(void);
unsigned char spr_page = 0;

/******************************************************************************/
#include "sp_com.h"
#include "spr_col.h"
/******************************************************************************/

//#include <stdio.h>
//#include <stdlib.h>

//#ifdef DEBUG

#define HMMM 0xD0
#define LMMM 0x90

enum {
	VDP_READDATA = 0,
	VDP_READSTATUS = 1
};

enum {
	VDP_WRITEDATA = 0,
	VDP_WRITECONTROL = 1,
	VDP_WRITEPAL = 2,
	VDP_WRITEINDEX = 3
};

#define VDP_readport(no) (VDP_readadr + no)
#define VDP_writeport(no) (VDP_writeadr + no)

unsigned char VDP_readadr;
unsigned char VDP_writeadr;

#define MAXCOLOR 16

#define DI() {\
__asm\
	di\
__endasm;\
}

#define EI() {\
__asm\
	ei\
__endasm;\
}

/* R G B */
unsigned char org_pal[MAXCOLOR][3] = {
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

void playbgm(void) __sdcccall(1)
{
__asm
	xor	a
	ld	de,#0x0f000
	call	0ffcah
	or	a
	ret	z	;mgsdrv無し
	ld	de,jptab	;ジャンプテーブル転送
	ld	bc,12
	ldir
;	di
;	ld	a,??	;セグメントを切り替えたりする場合
;	call	jptab+12
	push	ix
	ld	ix,#0x6028	;演奏データの検査
	ld	hl,#0x8000
	call	jptab+9
	pop	ix
	ret nz	;エラー発生
	push	ix
	ld	ix,#0x6016	;演奏開始
	ld	b,-1
	ld	hl,-1
	ld	de,#0x8000
	call	jptab+9
	ei
	pop	ix
	ret
jptab:
	db	0,0,0	;id_interv
	db	0,0,0	;id_chgdrv
	db	0,0,0	;id_revdrv
	db	0,0,0	;id_calmgs
	db	0,0,0	;id_chgseg
__endasm;
}

void stopbgm(void) __sdcccall(1)
{
__asm
	xor	a
	ld	de,#0x0f000
	call	0ffcah
	or	a
	ret	z	;mgsdrv無し
	ld	de,jptab	;ジャンプテーブル転送
	ld	bc,12
	ldir
;	di
;	ld	a,??	;セグメントを切り替えたりする場合
;	call	jptab+12
	push	ix
	ld	ix,#0x6028	;演奏データの検査
	ld	hl,#0x8000
	call	jptab+9
	pop	ix
	ret nz	;エラー発生
	push	ix
	ld	ix,#0x6013	;演奏停止
	ld	b,-1
	ld	hl,-1
	ld	de,#0x8000
	call	jptab+9
	ei
	pop	ix
	ret
__endasm;
}

void set_vol(unsigned char vol) __sdcccall(1)
{
__asm
;	ld	hl, #2
;	add	hl, sp
;	ld	a,(hl) ; a = vol
	push af

	xor	a
	ld	de,#0x0f000
	call	0ffcah
	or	a
	jr	z,error	;mgsdrv無し
	ld	de,jptab	;ジャンプテーブル転送
	ld	bc,12
	ldir
;	di
;	ld	a,??	;セグメントを切り替えたりする場合
;	call	jptab+12
	push	ix
	ld	ix,#0x6028	;演奏データの検査
	ld	hl,#0x8000
	call	jptab+9
	pop	ix
	jr nz,error	;エラー発生

	pop	af
	push	ix
	ld	ix,#0x6022	;ボリュームセット
	ld	b,0
	ld	hl,-1
	ld	de,#0x8000


	call	jptab+9
	ei
	pop	ix
	ret
error:
	pop	af
	ret
__endasm;
}


void write_psg(unsigned char reg, unsigned char tone) __sdcccall(1)
{
__asm
;	ld	hl, #2
;	add	hl, sp
	push	ix

	ld	h,a

	ld	a,(#0xfcc1)	; exptbl
	ld	b,a
	ld	c,0
	push	bc
	pop	iy

;	ld	c,(hl)
;	inc	hl
;	ld	b,(hl)	;bc = reg
;	inc	hl
;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; de = tone

;	ld	a,e
;	ld	e,d
	ld	a,h
	ld	e,l

	ld ix,#0x0093	; WRTPSG(MAINROM)
	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

/* screenのBIOS切り替え */
void set_screenmode(unsigned char mode) __sdcccall(1)
{
__asm
;	ld	 hl, 2
;	add	hl, sp

	push	ix
	ld	b,a

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x005f	; CHGMOD(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,b

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void set_screencolor(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x0062	; CHGCLR(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void key_flush(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x0156	; KILBUF(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void cls(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x00c3	; CLS(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

/* mainromの指定番地の値を得る */
unsigned char  read_mainrom(unsigned short adr) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
;	push	ix

;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; de=adr
;	ld	h,d
;	ld	l,e	; hl=adr

	ld	a,(#0xfcc1)	; exptbl
	call	#0x000c	; RDSLT

;	ld	l,a
;	ld	h,#0

;	pop	ix
__endasm;
}

void write_VDP(unsigned char regno, unsigned char data) __sdcccall(1)
{
//	outp(VDP_writeport(VDP_WRITECONTROL), data);
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x80 | regno);
__asm
	ld	h,a
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	ld	a,l
	out	(c),a
	ld	a,h
	set 7,a
	out	(c),a
__endasm;
}

void write_vram_adr(unsigned char highadr, int lowadr) __sdcccall(1)
{
__asm
	push	de
__endasm;
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
__asm
	pop	de
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	out	(c),e
	ld	a,d
	and	a,0x3f
	set	6,a
	out	(c),a
__endasm;
//	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x40 | ((lowadr >> 8) & 0x3f));
}

void write_vram_data(unsigned char data) __sdcccall(1)
{
__asm
//	outp(VDP_writeport(VDP_WRITEDATA), data);
	ld	b,a
	ld	a,(_VDP_writeadr)
	ld	c,a
	out	(c),b
__endasm;
}

void read_vram_adr(unsigned char highadr, int lowadr) __sdcccall(1)
{
__asm
	push	de
__endasm;
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
__asm
	pop	de
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	out	(c),e
	ld	a,d
	and	a,0x3f
	out	(c),a
__endasm;
//	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x00 | ((lowadr >> 8) & 0x3f));
}

unsigned char read_vram_data(void) __sdcccall(1)
{
__asm
	ld	a,(_VDP_readadr)
	ld	c,a
	in	a,(c)
__endasm;
//	return inp(VDP_readport(VDP_READDATA));
}


//#define read_vram_data() inp(VDP_readport(VDP_READDATA))


void set_displaypage(int page) __sdcccall(1)
{
	DI();
	write_VDP(2, (page << 5) & 0x60 | 0x1f);
	EI();
}

unsigned char read_VDPstatus(unsigned char no) __sdcccall(1)
{
	unsigned char data;
	DI();
	write_VDP(15, no);
//	data = inp(VDP_readport(VDP_READSTATUS));
__asm
	ld	a,(_VDP_readadr)
	inc	a
	ld	c,a
	in a,(c)
	push	af
__endasm;
	write_VDP(15, 0);
__asm
	pop	af
	EI
__endasm;
//	return data;
}

unsigned char port,port2;

void wait_VDP(void) {
//	unsigned char data;
	port = VDP_writeport(VDP_WRITECONTROL);
	port2 = VDP_readport(VDP_READSTATUS);

/*	do{
	EI();
	DI();
		outp(port, 2);
		outp(port, 0x80 + 15);

		data = inp(port2);

		outp(port, 0);
		outp(port, 0x80 + 15);
	}while((data & 0x01));
*/
__asm
waitloop:
	ei
	nop
	di
	ld	a,(_port)
	ld	c,a
	ld	a,2
	out	(c),a
	ld	a,#0x80 + 15
	out	(c),a
	ld	b,c

	ld	a,(_port2)
	ld	c,a
	in a,(c)
	ld	c,b
	ld	b,a

	xor	a,a
	out	(c),a
	ld	a,#0x80 + 15
	out	(c),a

	ld	a,b
	and	a,#0x01
	jr	nz,waitloop
__endasm;
}

void boxfill(int dx, int dy, int nx, int ny, unsigned char dix, unsigned char diy, unsigned char data)
{
	unsigned char port = VDP_writeport(VDP_WRITEINDEX);
	unsigned char port2 = VDP_writeport(VDP_WRITECONTROL);

//	write_vdp(17, 36);

	wait_VDP();
	outp(port2, 36);
	outp(port2, 0x80 | 17);

	outp(port, dx & 0xff);
	outp(port, (dx >> 8) & 0x01);
	outp(port, dy & 0xff);
	outp(port, (dy >> 8) & 0x03);
	outp(port, nx & 0xff);
	outp(port, (nx >> 8) & 0x01);
	outp(port, ny & 0xff);
	outp(port, (ny >> 8) & 0x03);
	outp(port, data);
	outp(port, ((diy << 3) & 0x80) | ((diy << 2) & 0x40));
	outp(port, 0xc0);

	wait_VDP();

	EI();
}

unsigned char port3, port4;

unsigned char sx, sy, dx, dy; //, nc, ny, dix, diy, 
unsigned char VDPcommand;
unsigned char APAGE,VPAGE,XSIZE,XSIZA,YSIZE;

void VDPsetAREA2(void)
/*unsigned short sx, unsigned short sy, unsigned short dx, unsigned short dy, unsigned short nx, unsigned short ny, unsigned char dix, unsigned char diy, unsigned char command)*/
{
	port3 = VDP_writeport(VDP_WRITEINDEX);
	port4 = VDP_writeport(VDP_WRITECONTROL);

//	vdpdata[0] = (sx & 0xff);		/* 32 */
//	vdpdata[1] = ((sx >> 8) & 0x01);	/* 33 */
//	vdpdata[2] = (sy & 0xff);		/* 34 */
//	vdpdata[3] = ((sy >> 8) & 0x03);	/* 35 */
//	vdpdata[4] = (dx & 0xff);		/* 36 */
//	vdpdata[5] = ((dx >> 8) & 0x01);	/* 37 */
//	vdpdata[6] = (dy & 0xff);		/* 38 */
//	vdpdata[7] = ((dy >> 8) & 0x03);	/* 39 */
//	vdpdata[8] = (nx & 0xff);		/* 40 */
//	vdpdata[9] = ((nx >> 8) & 0x01);	/* 41 */
//	vdpdata[0xa] = (ny & 0xff);		/* 42 */
//	vdpdata[0xb] = ((ny >> 8) & 0x03);	/* 43 */
//	vdpdata[0xc] = 0;
//	vdpdata[0xd] = ((diy << 3) & 0x08) | ((dix << 2) & 0x04);	/* 45 */
//	vdpdata[0xe] = VDPcommand;
__asm
	ld	a,(_sx)	;SX
	ld	h,a
	ld	a,(_sy)	;SY
	ld	l,a
;	ld	a,(_vdpdata+3)
;	ld	(_APAGE),a
	ld	a,(_dx)	;DX
	ld	d,a
	ld	a,(_dy)	;DY
	ld	e,a
;	ld	a,(_vdpdata+7)
;	ld	(_VPAGE),a
;	ld	a,(_vdpdata+8)
;	ld	(_XSIZE),a
;	ld	a,(_vdpdata+9)
;	ld	(_XSIZA),a
;	ld	a,(_vdpdata+0xa)
;	ld	(_YSIZE),a
;	exx
__endasm;
/*
	outp(port4, 32);
	outp(port4, 0x80 | 17);
*/

	wait_VDP();

//	outp(port3, data0);			/* 32 */
//	outp(port3, data1);			/* 33 */
//	outp(port3, data2);			/* 34 */
//	outp(port3, data3);			/* 35 */
//	outp(port3, data4);			/* 36 */
//	outp(port3, data5);			/* 37 */
//	outp(port3, data6);			/* 38 */
//	outp(port3, data7);			/* 39 */
//	outp(port3, data8);			/* 40 */
//	outp(port3, data9);			/* 41 */
//	outp(port3, dataa);			/* 42 */
//	outp(port3, datab);			/* 43 */
//	outp(port3, 0);				/* 44 */

//	outp(port3, datad);	/* 45 */

//	outp(port3, VDPcommand);
__asm
	ld	a,(_port4)
	ld	c,a
	ld	a,32
	out	(c),a
	ld	a,#0x80 | 17
	out	(c),a

;	EI
;	exx

	ld	b,0x0f
	ld	a,(_port3)
	ld	c,a
;	ld	hl,_vdpdata

	XOR	A
	OUT	(C),H	;SX
	OUT	(C),A	
	LD	A,(_APAGE)
	OUT	(C),L	;SY
	OUT	(C),A	

	XOR	A
	OUT	(C),D	;DX
	OUT	(C),A	
	LD	A,(_VPAGE)
	OUT	(C),E	;DY
	OUT	(C),A
	LD	A,(_XSIZE)
	LD	B,A
	LD	A,(_XSIZA)
	OUT	(C),B
	OUT	(C),A
	LD	A,(_YSIZE)
	LD	B,A
	XOR	A
	OUT	(C),B
	OUT	(C),A
	OUT	(C),A	;DUMMY

	LD	A,H
	SUB	D
	LD	A,0
	JR	C,DQ
DQ:	OR	2

	OUT	(C),A	;DIX and DIY

	ld	a,(_VDPcommand)
	out	(C),a	/* com */
	ei
__endasm;
}

void spr_on(void)
{
	DI();
	write_VDP(8, 0x08);
	EI();
}

void spr_off(void)
{
	DI();
	write_VDP(8, 0x0a);
	EI();
}

void set_spr_atr_adr(unsigned char highadr) __sdcccall(1) //, int lowadr)
{
	DI();
//	write_VDP(5, (lowadr >> (2 + 5)) & 0xf8 | 0x07);
//	write_VDP(11, ((highadr << 1) & 0x02) | ((lowadr >> 15) & 0x01));
//	write_VDP(5, (0xe8));
	write_VDP(11, ((highadr << 1) & 0x02));
	EI();
}

unsigned char get_key(unsigned char matrix) __sdcccall(1)
{
	outp(0xaa, ((inp(0xaa) & 0xf0) | matrix));
	return inp(0xa9);
/*
__asm
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x0141	; SNSMAT(MAINROM)

	ld	 hl, #2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	#0x001c	; CALSLT

	ld	l,a
	ld	h,#0
__endasm;
*/
}

unsigned char get_stick(unsigned char trigno) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d5	; GTSTCK(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0

	pop	ix
__endasm;
}

unsigned char get_pad(unsigned char trigno) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,#0
	push	de
	pop	iy
	ld ix,#0x00d8	; GTTRIG(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
;	ld	l,a
;	ld	h,#0

	pop	ix
__endasm;
}

/* DISK BASIC only. */
/*volatile void Set_RAM_MODE(void){
__asm
	push	ix
	ld	a,(#0xf342)
	ld	hl,#0x4000
	call	#0x0024
	pop	ix
__endasm;
}

volatile void Set_ROM_MODE(void){
__asm
	push	ix
	ld	a,(#0xfcc1)
	ld	hl,#0x4000
	call	#0x0024
	pop	ix
__endasm;
}*/

void wait_vsync(void)
{
	while((read_VDPstatus(2) & 0x40));
	while(!(read_VDPstatus(2) & 0x40)); /* WAIT VSYNC */
}

unsigned char old_jiffy;

void init_sys_wait(void)
{
__asm
	ld	a,(#0xfc9e)
	ld	(_old_jiffy),a
__endasm;
}

void sys_wait(unsigned char wait) __sdcccall(1)
{
__asm
	ld	l,a
	ld	a,(_old_jiffy)
;	add	a,l
	ld	b,a

jiffyloop2:
	ld	a,(#0xfc9e)
	sub	a,b
;	cp	b
	cp	l
	jr	c,jiffyloop2	; a<b
__endasm;

	++total_count;
/*	if(*jiffy >= 60){
		put_numd((long)(total_count), 2);
		put_strings(SCREEN2, 28, 22, str_temp, CHRPAL_NO);
		total_count = 0;
		*jiffy = 0;
	}*/
}

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	unsigned char port = VDP_writeport(VDP_WRITEPAL);
	write_VDP(16, color);
	outp(port, red * 16 | blue);
	outp(port, green);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0]/2, color[i][1]/2, color[i][2]/2);
}

//value < 0 黒に近づける。
//value = 0 設定した色
//value > 0 白に近づける。
void set_constrast(int value, unsigned char org_pal[MAXCOLOR][3], int pal_no)
{
	int j, k;
	int pal[3];


	for(j = 0; j < MAXCOLOR; j++){
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
		pal_set(pal_no, j, pal[0]/2, pal[1]/2, pal[2]/2);
	}
}

//wait値の速度で黒からフェードインする。
void fadeinblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = -15; j <= 0; j++){
		init_sys_wait();
		wait_vsync();
		set_constrast(j, org_pal, pal_no);
		sys_wait(wait);
	}
}

//wait値の速度で黒にフェードアウトする。
void fadeoutblack(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j != -16; j--){
		init_sys_wait();
		wait_vsync();
		set_constrast(j, org_pal, pal_no);
		sys_wait(wait);
	}
}

//wait値の速度で白にフェードアウトする。
void fadeoutwhite(unsigned char org_pal[MAXCOLOR][3], int pal_no, int wait)
{
	int j;

	for(j = 0; j < 16; j++){
		init_sys_wait();
		wait_vsync();
		set_constrast(j, org_pal, pal_no);
		sys_wait(wait);
	}
}

//パレットを暗転する。
void pal_allblack(int pal_no)
{
	unsigned char j;
	for(j = 0; j < MAXCOLOR; j++)
		pal_set(pal_no, j, 0, 0, 0);
}

short test_h_f = TRUE;
short soundflag = FALSE;

char chr;

void put_strings(unsigned char scr, unsigned char y, unsigned char x,  char *str, unsigned char pal)
{
//	unsigned char chr;
//	unsigned short i = 0;
//	unsigned short *bgram;
//	bgram = (unsigned short *)0xebe000;
//	bgram += (x * 2 + (28-y) * 0x80) / 2; /* BG1 */
	y = 28-y;

	XSIZE = 8;
	XSIZA = 0;
	YSIZE = 8;
	APAGE = 3; //map_page;
	VPAGE = 0;
	VDPcommand = HMMM;

	while(1){
		chr = *(str++);
		if(chr == '\0')
			break;
		if((chr < 0x30)) //|| (chr > 0x5f))
			chr = 0x40;
//		*(bgram++) = chr - '0';
		chr -= '0';
//		VDPsetAREA((chr & 0x0f) * 8, (chr / 16) * 8 + 256 * 3, x * 8, y * 8, 8, 8, 0,0, HMMM);
		sx = (chr & 0x0f) * 8;
		sy = (chr / 16) * 8; // + 256 * 3; // + 512;
		dx = x * 8;
		dy = y * 8;
		VDPsetAREA2();
//		wait_VDP();
		++x;
	}
}


void put_numd(long j, unsigned char digit) __sdcccall(1)
{
	unsigned char i = digit;

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

unsigned char st0, st1, pd0, pd1, pd2, k3, k5, k7, k9, k10;
unsigned char keycode = 0;

//unsigned char 
void keyscan1(void)
{
//	while(((
//	k7 = get_key(7);
//) & 0x04)){ /* ESC */
	keycode = 0;

	return;
}

void keyscan(void)
{
	DI();
	keycode = 0;

	k3 = get_key(3);

	k9 = get_key(9);
	k10 = get_key(10);
	k5 = get_key(5);

	st0 = get_stick(0);
	st1 = get_stick(1);

	pd0 = get_pad(0);
	pd1 = get_pad(1);
	pd2 = get_pad(3);
	EI();

	if((pd0) || (pd1) || !(k5 & 0x20)) /* X,SPACE */
		keycode |= KEY_A;
	if((pd2) || !(k3 & 0x01)) /* C */
		keycode |= KEY_B;
	if((st0 >= 1 && st0 <=2) || (st0 == 8) || (st1 >= 1 && st1 <=2) || (st1 ==8) || !(k10 & 0x08)) /* 8 */
		keycode |= KEY_UP1;
	if((st0 >= 4 && st0 <=6) || (st1 >= 4 && st1 <=6) || !(k9 & 0x20)) /* 2 */
		keycode |= KEY_DOWN1;

//	if(!(st & 0x0c)){ /* RL */
//		keycode |= KEY_START;
//	}else{
	if((st0 >= 6 && st0 <=8) || (st1 >= 6 && st1 <=8) || !(k9 & 0x80)) /* 4 */
		keycode |= KEY_LEFT1;
	if((st0 >= 2 && st0 <=4) || (st1 >= 2 && st1 <=4) || !(k10 & 0x02)) /* 6 */
		keycode |= KEY_RIGHT1;
//	}

	return; // keycode;
}

int opening_demo(void)
{
	signed int i, j;

	put_strings(SCREEN2, 16, 11, "PROJECT CC", CHRPAL_NO);
	put_strings(SCREEN2, 14, 11, "SINCE 199X", CHRPAL_NO);
	fadeinblack(org_pal, CHRPAL_NO, 6);
	j = 4;
	for(i = 0; i < 75 / 2 * 3; i++){
		init_sys_wait();
		keyscan();
		if(keycode || (!(get_key(7) & 0x04))){
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

unsigned char x = 0;
int loopcounter = 0;
unsigned int soundtestno = 0;
int soundtest = FALSE;

int title_demo(void)
{
//	int j; //, keycode;

//	init_star();
	loopcounter = 0;
	soundtest = FALSE;
	soundtestno = 0;

	/* Set Title-Logo Pattern */
	/* Opening Start */
	j = -16 * 8;
	x = 1;
//	spr_count = old_count = 0;
	do{
		if(j < 0){
			j++;
//			spr_count = 0;
			set_constrast(j / 8, org_pal, CHRPAL_NO);
			if(!j){
//				put_strings(SCREEN2, 8, 7, "NORMAL", CHRPAL_NO);
//				put_strings(SCREEN2, 6, 7, "HARD", CHRPAL_NO);

				put_title();
				refresh_star();

//				while(keyscan());
			}
		}

		if(!j){
//			spr_count = 0;
			put_strings(SCREEN2, 7 + x * 2, 11, "?", CHRPAL_NO);
//			keycode =
			keyscan();
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
				keyscan();
				if(!x)
					return SYSEXIT;
				if(keycode & KEY_LEFT1)
					return ERRLV2;
				if(keycode & KEY_RIGHT1)
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
	}while((get_key(7) & 0x04));
	return SYSEXIT;
}


/* ゲ－ムのル－プ */
unsigned char a = 0, b = 0;
int i, j, xx, yy,*p_x,*p_y;
//int pat_no;
unsigned char pat_no;
//	unsigned char keycode;
short game_loop(void){
	unsigned char i,j;
	short xx,yy;
	tmp_spr_count = 0;

/* パッド入力 & 自機移動 */
/*	for(i=0;i<1;i++)*/
	i = 0;
	a = 0;
	b = 0;
	{
//		keycode = 
//		keyscan();

//		pad_read(i, &a, &b, &pd);
		if(keycode & KEY_A) /* Z,SPACE */
			a=1;
		if(keycode & KEY_B) /* X */
			b=1;


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

		DEF_SP(tmp_spr_count, *p_x, *p_y, my_data[i].pat_num, mypal);
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

//	spr_count = MYSHOT_OFFSET;
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

//			DEF_SP(spr_count, tmp_x, tmp_y, myshot[i].pat_num,  CHRPAL_NO);
			DEL_LIST(MAX_MYSHOT, i, j, myshot_next, myshot_free);

		}else{
			myshot[i].x = tmp_x;
			myshot[i].y = tmp_y;
			DEF_SP(tmp_spr_count, tmp_x, tmp_y, myshot[i].pat_num,  CHRPAL_NO);
		}

	}

//	spr_count = TEKI_OFFSET;
	move_teki();

//	spr_count = TKSHOT_OFFSET;
	move_tekishot();

/*	if(old_count[spr_page] != tmp_spr_count){
		put_numd((long)(tmp_spr_count), 2);
		put_strings(SCREEN2, 28, 24, str_temp, CHRPAL_NO);
	}*/
//	spr_count[spr_page] = tmp_spr_count;

	return NOERROR;
}

//unsigned char old_chr_no[MAX_SPRITE * 2];
//unsigned char old_atr[MAX_SPRITE * 2];
//unsigned char old_pal[MAX_SPRITE * 2];
unsigned char color_flag[MAX_SPRITE];

/* 変数初期化処理 */
void game_init(void){
	int i;

//	srand(time(NULL));	/* 乱数の初期化 */

	for(i = 0; i< MAX_SPRITE * 2; ++i){
		chr_data[i].x = 0;
		chr_data[i].y = 212;
		chr_data[i].pat_num = 255;
		chr_data[i].atr = 0x80;
	}
	for(i = 0; i< MAX_SPRITE; ++i){
		color_flag[i] = 0;
		for(j = 0; j < 2; ++j){
			old_data[j][i].pat_num = 255;
			old_data[j][i].atr = 0;
			old_data[j][i].pal = 0;
		}
	}

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
//	int i;
	unsigned char i;
/* スタ－の座標系を初期化 */
	for(i = 0;i < STAR_NUM; i++){
		star[0][i] = (i + 1) * (128 / STAR_NUM) + 7;
		star[1][i] = rand() % 212;
		star[2][i] = (rand() % 2) + 1;

		/* VRAMのアドレスを算出 */
		read_vram_adr(0, star[0][i] + (star[1][i] * 128));
		star[3][i] = read_vram_data();	/* 元の色を記憶する */

		star[4][i] = rand() % 14 + 2;
	}
/* スタ－の表示(固定表示) */
//	i = STAR_NUM;
//	while(i--){
//		vram = (unsigned short *)0xc00000 + (star[0][i] + ((256 / STAR_NUM) / 2) + 
//			(star[1][i] * 256)) * 2;
//		*vram |= star[4][i];
//	}
}

void refresh_star(void)
{
	for(i = 0;i < STAR_NUM; i++){
		/* VRAMのアドレスを算出 */
		read_vram_adr(0, star[0][i] + (star[1][i] * 128));
		k = read_vram_data();	/* 元の色を記憶する */
		if(k != star[4][i])
			star[3][i] = k;
	}
}

void bg_roll(void)
{
	unsigned char i;
//	register unsigned short *scroll = (unsigned short *)0xe8001a;	/* */

/* スクロ－ルレジスタ制御 */

//		scrl += 512 - (scrl_spd >> SCRL_SFT);
//		scrl %= 512;
//		*scroll = scrl;

/* スクロ－ルするスタ－の計算 */

		i = STAR_NUM;
		while(i--){
			if(!star[3][i]){
				__asm
					DI
				__endasm;
				write_vram_adr(0, star[0][i] + (star[1][i] * 128));
				write_vram_data(0); //star[3][i]);
				__asm
					EI
				__endasm;
			}
			star[1][i] += (star[2][i]); // + 212);
			if(star[1][i] >= 212)
				star[1][i] = 0;
//			star[1][i] %= 256;
//		}
//		i = STAR_NUM;
//		while(i--){
//			vram = (unsigned short *)0xc00000 + (star[0][i] + (star[1][i] * 512)) ;
			__asm
				DI
			__endasm;
			read_vram_adr(0, star[0][i] + (star[1][i] * 128));
			star[3][i] = read_vram_data();	/* 元の色を記憶する */
			if(!star[3][i]){
				write_vram_adr(0, star[0][i] + (star[1][i] * 128));
//				star[3][i] = *vram;
//				*vram |= star[4][i];
				write_vram_data(star[4][i]);
			}
			__asm
				EI
			__endasm;
		}
}

inline void clr_sp(void)
{
__asm
	push	bc
	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	a,0xd4
	out	(c),a
	xor	a, a
	out	(c),a
	ld	a,0xff
	out	(c),a
	xor	a, a
	out	(c),a
	pop	bc
__endasm;
}

unsigned char pat_num, atr, atr2; //, pal;

inline void set_spr(void)
{
__asm
;	push	af
;	push	bc
;	push	de
;	push	hl

;	ld	hl, (_pchr_data)
	ld	hl,_chr_data
	ld	de,3

	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	a,(_tmp_spr_count)
	or	a,a
	jr	z,sprend
	ld	b,a
sprloop2:
	ld	a,(hl)
	out	(c),a
	inc	hl
	ld	a,(hl)
	out	(c),a
	inc	hl
	inc	hl
	ld	a,(hl)
	out	(c),a
	xor	a,a
	out	(c),a

	add	hl,de

	djnz	sprloop2
sprend:
;	pop	hl
;	pop	de
;	pop	bc
;	pop	af
__endasm;
}

void set_sprite(void)
{
	unsigned char i, j;
	spr_page ^= 0x01;
//	tmp_spr_count = spr_count[spr_page];

//	wait_vsync();

/* スプライト表示 */
//	spram = 0;

/*	printf("\x1b[0;0H%d \n", spr_count);*/

/* 表示数ぶん書き込む */
	if(tmp_spr_count > MAX_SPRITE){
		if(total_count & 1){
			for(i = tmp_spr_count - MAX_SPRITE, j = 0; j < MAX_SPRITE; i++, j++){
//			for(i = spr_count - TKSHOT_OFFSET, j = TKSHOT_OFFSET; j < MAX_SPRITE; i++, j++){
//				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
				chr_data2[j] = chr_data[i];
//				chr_data2[j].x = chr_data[i].x;
//				chr_data2[j].y = chr_data[i].y;
//				chr_data2[j].pat_num = chr_data[i].pat_num;
//				chr_data2[j].atr = chr_data[i].atr;
//				chr_data2[j].pal = chr_data[i].pal;
			}
//			chr_data = chr_data2;
			for(i = 0; i < MAX_SPRITE; i++){
				chr_data[i] = chr_data2[i];
//				chr_data[i].x = chr_data2[i].x;
//				chr_data[i].y = chr_data2[i].y;
//				chr_data[i].pat_num = chr_data2[i].pat_num;
//				chr_data[i].atr = chr_data2[i].atr;
//				chr_data[i].pal = chr_data2[i].pal;
			}
		}
		tmp_spr_count = MAX_SPRITE;
	}
//else{
//	wait_vsync();
//__asm
//	DI
//__endasm;
//		write_vram_adr(0x00, 0x7600);
//		for(i = 0; i < MAX_SPRITE; i++){
/*		for(i = 0; i < TKSHOT_OFFSET; i++){
			PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
		}*/
//			for(i = TKSHOT_OFFSET; i < MAX_SPRITE; i++){
//				PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
//			}
//		}
//		old_count = MAX_SPRITE;
//	}else
//	{
//	wait_vsync();
//__asm
//	DI
//__endasm;
//		write_vram_adr(0x00, 0x7600);
//		for(i = 0; i < spr_count; i++){
//			PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, chr_data[i].atr);
//		}

//		if (old_count > spr_count){
//			for(i = 0;i < (old_count - spr_count); i++){
//			for(i = spr_count; i < MAX_SPRITE; i++){
//				PUT_SP(0, 212, 0, 0);
//			}
//		}
//		old_count = spr_count;
//	}
//__asm
//	EI
//__endasm;

	for(i = 0; i < tmp_spr_count; i++){
		CHR_PARA4 *pold_data = &old_data[spr_page][i];
		pchr_data = &chr_data[i];
		if((pold_data->pat_num != pchr_data->pat_num) || (pold_data->atr != pchr_data->atr) || (pold_data->pal != pchr_data->pal)){
			color_flag[i] = 1;
			pold_data->pat_num = pchr_data->pat_num;
			pold_data->atr = pchr_data->atr;
			pold_data->pal = pchr_data->pal;
		}
	}

	/* 色情報の処理 */
//	wait_vsync();
	DI();
//	write_vram_adr(0x00, 0x7600);
//	for(i = 0; i < MAX_SPRITE; i++){
	for(i = 0; i < tmp_spr_count; i++){
/*	DI();*/
		if(color_flag[i]){
			pchr_data = &chr_data[i];
			color_flag[i] = 0;
			pat_num = pchr_data->pat_num / 4;
			atr = pchr_data->atr;

//			if(!(read_VDPstatus(2) & 0x40))
//				while(!(read_VDPstatus(2) & 0x40)); /* WAIT V&HSYNC */
			write_vram_adr(spr_page, 0x7600 - 512 + i * 16);
			if(!pchr_data->pal){
				for(j = 0; j < 16; ++j){
//					write_vram_data(spr_col[pat_num][j] | atr);
					atr2 = spr_col[pat_num][j]; // | atr;
__asm
	push	bc
	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	a,(_atr2)
	ld	b,a
	ld	a,(_atr)
	or	a,b
	out	(c),a
	pop	bc
__endasm;
				}
			}else{
//				atr |= 0x0f;
//				atr = 0x08;
//				for(j = 0; j < 16; ++j){
//					write_vram_data(13 | atr); //((~spr_col[pat_num][j]) & 0x0f) | atr);
//				}
__asm
	push	bc
	ld	b,16
	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	a,(_atr)
	or	a,13
c_loop:
	out	(c),a
	djnz	c_loop
	pop	bc
__endasm;
			}
//			write_vram_adr(0x00, 0x7600  + (i) * 4);
		}
//		PUT_SP(chr_data[i].x, chr_data[i].y, chr_data[i].pat_num, 0); //chr_data[i].atr);

/*	EI();*/
	}

/*	DI();*/
	write_vram_adr(spr_page, 0x7600);
	set_spr();
/*	for(i = 0; i < tmp_spr_count; i++){
		pchr_data = &chr_data[i];
__asm
	push	bc
	ld	a,(_VDP_writeadr)
	ld	c,a
	ld	hl, (_pchr_data)
	ld	a,(hl)
	out	(c),a
	inc	hl
	ld	a,(hl)
	out	(c),a
	inc	hl
	inc	hl
	ld	a,(hl)
	out	(c),a
	xor	a,a
	out	(c),a
	pop	bc
__endasm;
*/
/*	EI();*/

//	}

	if (old_count[spr_page] > tmp_spr_count){
		for(i = 0;i < (old_count[spr_page] - tmp_spr_count); i++){
//		for(i = spr_count; i < MAX_SPRITE; i++){
/*	DI();*/
			PUT_SP(0, 212, 255, 0);
			clr_sp();
/*	EI();*/
		}
	}
	EI();

	old_count[spr_page] = tmp_spr_count;
	tmp_spr_count = 0;

//	++total_count;
	sys_wait(1);
	DI();
	set_spr_atr_adr(spr_page); //, SPR_ATR_ADR); /* color table : atr-512 (0x7400) */
	EI();
	init_sys_wait();
}

void set_se(void)
{
	if(seflag == 1){
		seflag = 0;
		DI();
		write_psg(6,127);
		write_psg(11,0);
		write_psg(12,15);
		write_psg(7,0x9c);  // 10011100
		write_psg(13,9);
		write_psg(10,0x10);
		EI();
//		if(soundflag == TRUE)
//			if(se_check())
//				se_stop();
//		S_IL_FUNC(se_play(sndtable[0], seflag - 1));	/* 効果音 */
//		if(mcd_status >= 0){
//			pcm_play(&SNDBUFF[seflag - 1][0], pcmsize[seflag - 1]);
//		}
	}
}

/* スプライトを全て画面外に移す */
void spr_clear(void){
	DI();
	write_vram_adr(spr_page, 0x7600);
	for(i = 0; i < MAX_SPRITE; i++){
		clr_sp();
//		PUT_SP(0, 212, 255, 0);
	}
	set_spr_atr_adr(spr_page);
	EI();
}

void term(void)
{
/*__asm
	xor	a
	ld	c,0
	call	0005h
__endasm;*/
}

void game_put(void)
{
	unsigned char i, j;
	tmp_spr_count = 0;
	i = 0;
	DEF_SP(tmp_spr_count, my_data[i].x, my_data[i].y, my_data[i].pat_num, mypal); /* 0x14); */
//	spr_count = MYSHOT_OFFSET;
	SEARCH_LIST2(MAX_MYSHOT, i, j, myshot_next){
		DEF_SP(tmp_spr_count, myshot[i].x, myshot[i].y, myshot[i].pat_num,  CHRPAL_NO);
	}
//	spr_count = TEKI_OFFSET;
	SEARCH_LIST2(MAX_TEKI, i, j, teki_next){
		DEF_SP(tmp_spr_count, teki[i].x, teki[i].y, teki_pat[i], teki_pal[i]);
	}
//	spr_count = TKSHOT_OFFSET;
	SEARCH_LIST2(MAX_TKSHOT, i, j, tkshot_next){
		DEF_SP(tmp_spr_count, tkshot[i].x, tkshot[i].y, tkshot_pat[i], CHRPAL_NO);
	}
//	spr_count[spr_page] = tmp_spr_count;
}


/* ゲ－ム本体の処理 */
//short game_run(short mode){
short errlv = 0;
unsigned char *vdp_value = 0xf3df;
unsigned char *forclr = 0xf3e9;
unsigned char *bakclr = 0xf3ea;
unsigned char *bdrclr = 0xf3eb;
unsigned char *clicksw = 0xf3db;
unsigned char *oldscr = 0xfcb0;

unsigned char forclr_old, bakclr_old, bdrclr_old, clicksw_old;

void main(void)
{
//	set_vol(16);
//	unsigned char ii,jj;

	VDP_readadr = read_mainrom(0x0006);
	VDP_writeadr = read_mainrom(0x0007);

	forclr_old = *forclr;
	bakclr_old = *bakclr;
	bdrclr_old = *bdrclr;

	*forclr = 15;
	*bakclr = 0;
	*bdrclr = 0;
	set_screencolor();

	clicksw_old = *clicksw;
	*clicksw = 0;

	set_screenmode(5);
	set_displaypage(0);
	DI();
	write_VDP(1, vdp_value[1] | 0x02);
	EI();
	spr_on();
	boxfill(0, 256, 256, 212, 0, 0, 0x00);

	spr_page = 0;
	tmp_spr_count = 0;
	old_count[0] = old_count[1] = MAX_SPRITE;


	do{
//		cls();
		wait_vsync();
//		cls();
		DI();
		pal_allblack(CHRPAL_NO);
		EI();
		boxfill(0, 0, 256, 212, 0, 0, 0x00);

//		spr_count[0] = spr_count[1] = 0;
//		set_sprite();

//		pal_allblack(BGPAL_NO);
//		*scroll_x = 0; //(-(128-48-16) + 512) % 512;
//		*scroll_y = 0; //(-48+16 + 512) % 512;
//		vram =  (unsigned short *)0xc00000;
//		paint(0x0);
//		paint2(0x40);
//		cls();
//		VDPsetAREA(0, 512, 64, 32, 128, 64, 0,0, HMMM);
		sx = 0;
		sy = 0; // + 512;
		dx = 64;
		dy = 32;
		XSIZE = 128;
		XSIZA = 0;
		YSIZE = 64;
		APAGE = 2; //map_page;
		VPAGE = 0;
		VDPcommand = HMMM;
		VDPsetAREA2();
		wait_VDP();
	EI();

		init_star();
		wait_vsync();
	EI();
//		pal_all(BGPAL_NO, org_pal);
		errlv = title_demo();
		if((errlv == SYSERR) || (errlv == SYSEXIT))
			break;
		else if(errlv == NOERROR){
			for(j = 0; j != -16 * 8; j--){
				wait_vsync();
//				set_constrast(j / 8, org_pal, BGPAL_NO);
				DI();
				set_constrast(j / 8, org_pal, CHRPAL_NO);
				EI();
				bg_roll();
			}
//			cls();
			boxfill(0, 0, 256, 212, 0, 0, 0x00);
			wait_vsync();
//			*scroll_x = 512;
//			pal_allblack(BGPAL_NO);
//			vram =  (unsigned short *)0xc00000;
//			paint(0x0);
//			paint2(0x40);
			opening_demo();
//			paint2(0x40);
//			cls();

		}else if(errlv >= ERRLV1){
//			errlv -= ERRLV1;
//			screen_fill_char(SCREEN2, 0, 0, 26, 32, 0);
//			paint3(0x40);
//			cls();
			boxfill(0, 0, 256, 212, 0, 0, 0x00);
//			playbgm(errlv, debugmode);
//			play(0, debugmode);
			set_vol(0);
			playbgm();
//			if(mcd_status >= 0){
//				mcd_stop();
//				mcd_play();
//			}
//			VDPsetAREA(0, 512, 64, 32, 128, 64, 0,0, HMMM);
			sx = 0;
			sy = 0; // + 512;
			dx = 64;
			dy = 32;
			XSIZE = 128;
			XSIZA = 0;
			YSIZE = 64;
			APAGE = 2; //map_page;
			VPAGE = 0;
			VDPcommand = HMMM;
			VDPsetAREA2();
//			wait_VDP();
			DI();
			write_VDP(2, 0x3f);
			write_VDP(25, 1);
			EI();
//			cls();
			for(i = 0; i < 192 + 16; i += 8){
				wait_vsync();
//				*scroll_x = i;
				DI();
				write_VDP(26, i / 8);
				EI();
//				score_displayall();
//				bg_roll();
				wait_vsync();
//				*scroll_x = 1024-i;
				DI();
				write_VDP(26, (512-i) / 8);
				EI();
//				score_displayall();
//				bg_roll();
			}
//			cls();

/*			boxfill(0, 0, 256, 32, 0, 0, 0x00);
			boxfill(0, 32, 256, 32, 0, 0, 0x00);
			boxfill(0, 64, 256, 32, 0, 0, 0x00);
			boxfill(0, 96, 256, 32, 0, 0, 0x00);
			boxfill(0, 128, 256, 32, 0, 0, 0x00);
			boxfill(0, 196, 256, 212-196, 0, 0, 0x00);*/
			boxfill(0, 0, 256, 212, 0, 0, 0x00);
			DI();
			write_VDP(2, 0x1f);
			write_VDP(25, 0);
			write_VDP(26, 0);
			EI();
			spr_clear();

//			put_strings(SCREEN2, 3, 0, "SHIELD", CHRPAL_NO);
//			short i;
//			unsigned char keycode;

			my_hp_flag = TRUE;
			game_init();	/* 各変数の初期化 */

			switch(errlv){
//			if(errlv == ERRLV2){
				case ERRLV2:
					stage = 3;
					max_my_hp = 10;
					my_hp = max_my_hp;
					tkshot_c /= 6;
					score = 1;
//					renshaflag = TRUE;
					break;
//			}else if(errlv == ERRLV3){
				case ERRLV3:
					score = 2;
					max_my_hp = 10;
					my_hp = max_my_hp;
//					renshaflag = TRUE;
//					renshaflag = FALSE;
					break;
			}

//			errlv = NOERROR;

/*			do{
				keyscan();
			}while(!(keycode & KEY_A));
*/
			put_strings(SCREEN2, 3, 0, "SHIELD", CHRPAL_NO);
			score_displayall();
			put_my_hp_dmg();

			init_star();

//			spr_count = 0;
//			old_count = MAX_SPRITE;
			init_sys_wait();
			set_sprite();
			set_se();
			DI();
			write_VDP(23, 0);
			EI();

/* ゲームのメインループ */
			do{
				if(mypal_dmgtime){
					--mypal_dmgtime;
					if(!mypal_dmgtime){
						if(!my_hp){
							timeup = 60 * 10;
							scrlspd = 0;
							do{
							put_strings(SCREEN2, 14, 10, "CONTINUE A", CHRPAL_NO);
								keyscan();
								game_put();
//								wait_vsync();
								set_sprite();
								set_se();
							}while(keycode & (KEY_A | KEY_START));
							do{
								put_numd((long)(timeup / 60), 2);
								put_strings(SCREEN2, 10, 14, str_temp, CHRPAL_NO);

								if(!(--timeup)){
									put_strings(SCREEN2, 14, 10, "           ", CHRPAL_NO);
									put_strings(SCREEN2, 14, 10, " GAME OVER ", CHRPAL_NO);
									put_strings(SCREEN2, 10, 14, "  ", CHRPAL_NO);
									scrlspd = 0; //SPR_DIV / 4;
									for(k = 0; k < 60 ; k++){
										game_put();
//										wait_vsync();
										set_sprite();
										bg_roll();
										set_se();
									}
//									return ERRLV1;
									errlv = ERRLV1;
									goto end;
								}
//								keycode =  
								keyscan();
								if(keycode & KEY_B){
									timeup -= 5;
									if((timeup) < 1)
										timeup = 1;
								}
								game_put();
//								wait_vsync();
								set_sprite();
								set_se();
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
						}else{
							mypal = CHRPAL_NO;
						}
					}
				}

				if(scrdspflag == TRUE){
					if(score > SCORE_MAX)
								score = SCORE_MAX;
					score_display();
					scrdspflag = FALSE;
/*					if(score >= hiscore){
						if((score % 10) == 0){
							hiscore = score;
						}
						put_strings(SCREEN2, 28, 0, "HI", CHRPAL_NO);
					}else
						put_strings(SCREEN2, 28, 0, "  ", CHRPAL_NO);
*/				}
				if(my_hp_flag == TRUE){
					put_my_hp_dmg();
					my_hp_flag = FALSE;
				}

//				game_loop();
//				printf("test0\n");

				keyscan();
//				printf("%x\n", keycode);
				if((keycode & KEY_START) || (keycode & KEY_B)){
//					if(keycode & (KEY_LEFT1))
//						scrlspd = SCRL_MIN;
//					else{
//						put_strings(SCREEN2, 14, 6, "PAUSE", CHRPAL_NO);
//					}
//					do {
						if(scrl_spd)
							put_strings(SCREEN2, 14, 13, "PAUSE", CHRPAL_NO);
//						bg_roll();

//						do_putmessage();

					do{
						game_put();
//						wait_vsync();
						set_sprite();
						set_se();
						keyscan();
					}while((keycode & (KEY_START | KEY_B)));
					do {
						game_put();
//						wait_vsync();
						set_sprite();
						set_se();
//						keycode = key_hit_check();
//						keycode = 
						keyscan();
						if(keycode & KEY_A){
//						if(keycode & KEY_B){
//							bg_roll();
//							set_sprite();
//							return SYSEXIT;		/* 一気に抜ける */
//							return NOERROR;		/* 一気に抜ける */
							errlv = NOERROR;
							goto end;
						}
//						if(keycode & KEY_A){
//						if(keycode & KEY_B){
//							scrl_spd = SCRL_MIN;
//							bg_roll();
//							set_sprite();
//							put_strings(SCREEN2, 14, 13, "     ", CHRPAL_NO);
//							return ERRLV2;
//						}
//						keyscan();
					}while((!(keycode & (KEY_START | KEY_B))));
					do{
						game_put();
//						wait_vsync();
						set_sprite();
						set_se();
						keyscan();
					}while(keycode & (KEY_START | KEY_B));
					scrl_spd = SCRL_MIN;
					put_strings(SCREEN2, 14, 13, "     ", CHRPAL_NO);

//					return ERRLV1;
//					spr_count = 0;
					continue;
				}

				switch(game_loop()){
					case SYSEXIT:
//						printf("test1\n");
						errlv = SYSEXIT;
						goto end;
//						return NOERROR;
					case NOERROR:
//						printf("test2\n");
//						wait_vsync();
						set_sprite();
						set_se();
						bg_roll();
						break;
					default:
//						printf("test3\n");
//						spr_count = 0;
						continue;
				}

//				if(read_VDPstatus(0) & 0x20){
					hit_check = TRUE;
//				}else{
//					hit_check = FALSE;
//				}
			}while((scrl_spd != 0) && (get_key(7) & 0x04)); /* ESC */
			errlv = SYSEXIT;

end:
			if(errlv == SYSEXIT)
				break;
			if(errlv != NOERROR){
				for(k = 0; k < 16 * 8; k++){
//					wait_vsync();
					game_put();
					set_sprite();
					set_se();
//					set_constrast(k / 8, org_pal, BGPAL_NO);
					set_constrast(k / 8, org_pal, CHRPAL_NO);
					bg_roll();
					set_vol(k / 8);
				}
			}
			stopbgm();
//			fadeoutwhite(org_pal, CHRPAL_NO, 10);
		}
		wait_vsync();
		spr_clear();
		wait_vsync();
//		cls();
	}while((get_key(7) & 0x04));


/* 終了処理 */
/* 	spr_off(); */
//	return SYSEXIT;
	stopbgm();
	set_vol(0);
	DI();
	pal_all(CHRPAL_NO, org_pal);
	EI();
	*forclr = forclr_old;
	*bakclr = bakclr_old;
	*bdrclr = bdrclr_old;
	set_screencolor();

	set_screenmode(*oldscr);

	*clicksw = clicksw_old;

	key_flush();
	term();
//	exit(0);
}
