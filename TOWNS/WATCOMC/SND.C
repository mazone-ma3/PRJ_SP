#include "SND.H"
#include <conio.h>

/* 参考 */
/* FM TOWNSテクニカルデータブック */
/* Free386のドキュメント(SND.C) */
/* HIS.hのソース */

//extern long score;

extern void int_fm(void);

char *stackAddress = 0;
char stack[1000];

void __interrupt __near SND_int_ent(void)
{
//	outp(0x22,0x40);
//	outp(0x5ca,0);
	__asm {
		push	eax

		push	es
		push	fs
		push	gs
		pushad

		mov	dx,04e9h
		in	al,dx
		push	eax
		test	al,01h
		je	fm_skip

		mov	ax,0110h
		mov	fs,ax
		mov	ah,10h
		call	pword ptr fs:0080h
		test	dl,01h
		je	timer_a_skip

		push	edx
		mov	ax,0110h
		mov	fs,ax
		mov	ah,17h
		call	pword ptr fs:0080h

		pop	edx

timer_a_skip:
		test	dl,02h
		je	fm_skip

		push	edx
		mov	ax,0110h
		mov	fs,ax
		mov	ah,18h
		call	pword ptr fs:0080h

;		push	ds

;		jp	callend

		mov	ax,014h
		mov	ds,ax
;		jp	callstart

; 旧スタック退避
		mov	edx,esp
		xor	ebp,ebp
		mov	bp,ss

; スタック切り換え
		mov	ecx,stackAddress
		cmp	ecx,0
		jne	short UserStack
; DOS|Extenderのスタックを使う
		mov	ecx,esp
		mov	ax,ss
UserStack:
		mov	ss,ax
		mov	esp,ecx
		push	edx ; 旧ESP
		push	ebp ; 旧SS

callstart:
		call	int_fm
;		jp	callend

; スタック復元
		pop	ebp
		pop	edx
		mov	ss,bp
		mov	esp,edx

callend:
;		pop	ds

		pop	edx

fm_skip:
		pop	eax
		test	al,08h
		je	short pcm_skip

		mov	ax,0110h
		mov	fs,ax
		mov	 ah,51h
		call	pword ptr fs:0080h

pcm_skip:
		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

;		mov	al,00100101b
		mov	al,00100000b
		out	0010h,al	;/* EOI(Slave) */

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		out 0,al	;/* EOI(Master) */

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

		popad
		pop	gs
		pop	fs
		pop	es

		pop	eax

;		iretd
	}
}



//char *sndworkaddr;
//int  sndworksize;

//char *work;

int VECTOR_ADR;
short VECTOR_SEG;
int VECTOR_REA;

void SND_int(void)
{

__asm{
	cli
}
	SND_int_timer_a1(0,0);
	SND_int_timer_b1(0,0);
	SND_int_timer_b1(1,199);

	stackAddress = stack+1000;

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

		mov	cl,04dh
		mov	ax,02502h
		int	21h
		mov	ax,es
		mov	VECTOR_ADR,ebx
		mov	VECTOR_SEG,ax

		mov	cl,04dh
		mov	ax,02503h
		int	21h
		mov	VECTOR_REA,ebx

		push	ds
		mov	ax,cs
		mov ds,ax

		mov	ax,02506h
		mov	cl,4dh ;4bh
		lea	edx,SND_int_ent
		int	21h

		pop	ds
		jc	error

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

		mov	dl,00100000b
;		mov	dl,00001000b
		xor	dl,0ffh
		and	al,dl
		out	0010h+2,al

;		mov	al,010000011b
;		out	0010h,al

;		mov	al,000000001b
;		out	0012h,al

		out	06ch,al ; 1μ秒ウェイト
		cmc
		cmc

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

error:
;	outp(0x22,0x40);
;		mov	al,40h
;		out 22h,al
		hlt
end:
		sti
	}
//	outp(0x5ca,0);

//	return ret01;
}

//int sndwork2[SndWorkSize/4];

char SND_init(char *work1)
{
	char ret01;
//	char *work2 = sndwork;

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

		mov	edi, work1;sndwork2; //work2 ;sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,00h
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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
;--------
	}
	return ret01;
}


char SND_end(void)
{
	char ret01 = 0;
	__asm {
		cli
	}
	SND_int_timer_a1(0,0);
	SND_int_timer_b1(0,0);
	__asm {
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
		or	al,00100000b
		out	0010h+2,al


		push	ds
;		mov	ax,cs
;		mov ds,ax

		mov	cl,4dh ;4bh
		mov	edx,VECTOR_ADR
		mov	ax, VECTOR_SEG
		mov	ebx,VECTOR_REA

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
	return ret01;
}

char SND_elevol_init(void)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,44h
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

char mute;

char SND_elevol_mute(char mute1)
{
	char ret01;
//	mute = mute1;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,46h
		mov	bl,mute1
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

char sw;

char SND_elevol_all_mute(char sw1)
{
	char ret01;
//	sw = sw1;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,49h
		mov	dl,sw1
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

//char ch;

char SND_pcm_mode_set(char ch1)
{
	char ret01;
//	ch = ch1;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,21h
		mov	bl,ch1;1;ch
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

//char note, vol;

char SND_pcm_play(char ch1,char note1,char vol1,char *buff)
{
	char ret01;
//	ch = ch1;
//	note = note1;
//	vol = vol1;

//	sndworkaddr
//	work = (char *)buff;
//	sndworksize = SndWorkSize;
	
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
		mov	esi,buff;work ;sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	ah,25h
		mov	bl,ch1; 71 ;ch
		mov	dh,note1; 60 ;note
		mov	dl,vol1; 127; vol

		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

char SND_pcm_play_stop(char ch1)
{
	char ret01;
//	ch = ch1;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,27h
		mov	bl,ch1
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}



char SND_fm_read_status(void)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,10h
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,dl

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

	return ret01;
}

char SND_fm_write_data(char bank,char reg,char data)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,11h
		mov	bh,bank
		mov	dh,reg
		mov	dl,data
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

char SND_fm_write_save_data(char bank,char reg,char data)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,13h
		mov	bh,bank
		mov	dh,reg
		mov	dl,data
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

char SND_fm_read_save_data(char bank,char reg,char *data)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,13h
		mov	bh,bank
		mov	dh,reg
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
		mov	ecx,data
		mov	[ecx],dl
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}


char SND_int_timer_a1(char sw, short counter)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,15h
		mov	bl,sw
		mov	cx,counter
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;		mov	ecx,data
;		mov	[ecx],dl
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

char SND_int_timer_b1(char sw, short counter)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,16h
		mov	bl,sw
		mov	cx,counter
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;		mov	ecx,data
;		mov	[ecx],dl
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

char SND_int_timer_a2(void)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,17h
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;		mov	ecx,data
;		mov	[ecx],dl
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}

char SND_int_timer_b2(void)
{
	char ret01;
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
;		mov	edi,sndworkaddr
;		mov	 ecx,sndworksize
		mov	ax,0110h
		mov	fs,ax
		mov	 ah,18h
		call	pword ptr fs:0080h
;		cmp	 al, 0
;		jnz	 gmode_end
;		mov	ecx,data
;		mov	[ecx],dl
;gmode_end:
		mov	 ret01,al

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

	return ret01;
}
