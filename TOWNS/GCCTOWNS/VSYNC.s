	.text
	.data

.global _int_vsync_ent
.global _init_vsync_ent
.global _reset_vsync_ent
//.extern _int_vsync
.extern _datasegment
.extern _VECTOR_ADRV
.extern _VECTOR_SEGV
.extern _VECTOR_REAV
.extern _vsync_flag

		.ascii	"VSYNC"

_int_vsync_ent:
	pushal
	pushfl
	push	%ds

//	mov	$0x40,%al
//	out %al,$0x22

	mov $0x14,%ax
	mov	%ax,%ds

//	cli

//	call _int_vsync
//	jmp	__skip

	mov	$1,%eax
	mov	%eax,_vsync_flag

	xor	%eax,%eax
	out	%al,$0x05ca

	out	%al,$0x06c // 1μ秒ウェイト
	cmc
	cmc

//	mov	00100000b,%al
	mov	$0x20,%al
	out	%al,$0x0010	;/* EOI(Slave) */

	out	%al,$0x06c // 1μ秒ウェイト
	cmc
	cmc

	out %al,$0	;/* EOI(Master) */

	out	%al,$0x06c // 1μ秒ウェイト
	cmc
	cmc
__skip:

//	sti

	pop	%ds
	popfl
	popal
	iretl

///////////////////////////////////////////////////////////////////////////////

_init_vsync_ent:
	push	%es
	pushal
	pushfl
/*	popfl
	popal
	pop	%es
	ret
*/
//	jmp	__end

	cli

	mov	%ds,%ax
	mov	%ax,_datasegment

	mov	$0x04b,%cl
	mov	$0x2502,%ax
	int	$0x21
	mov	%es,%ax
	mov	%ebx,_VECTOR_ADRV
	mov	%ax,_VECTOR_SEGV

	mov	$0x04b,%cl
	mov	$0x2503,%ax
	int	$0x21
	mov	%ebx,_VECTOR_REAV

	push	%ds

	mov	%cs,%ax
	mov %ax,%ds

	mov	$0x02506,%ax
//	mov	$0x02504,%ax
	mov	$0x4b,%cl
	leal	_int_vsync_ent,%edx
	int	$0x21

	pop	%ds

//	jmp	__noerror1
	jnb	__noerror1

//	outp(0x22,0x40);
	mov	$0x40,%al
	out %al,$0x22
	hlt

__noerror1:
	out	%al,$0x06c // 1μ秒ウェイト
	cmc
	cmc

	in	$0x0010+2,%al

	out	%al,$0x06c // 1μ秒ウェイト
	cmc
	cmc

//	mov	00100000b,%dl
//	mov	00001000b,%dl
	mov	$0x08,%dl
	xor	$0xff,%dl
	and	%dl,%al
	out	%al,$0x0010+2
	sti

__end:
	popfl
	popal
	pop	%es
	xor	%eax,%eax

	ret

///////////////////////////////////////////////////////////////////////////////

_reset_vsync_ent:
	cli

	push	%es
	pushal
	pushfl

	out	%al,$0x06c	// 1μ秒ウェイト
	cmc
	cmc

	in	$0x0010+2,%al

	out	%al,$0x06c	// 1μ秒ウェイト
	cmc
	cmc

//	or	00001000b,%al
	or	$0x80,%al
	out	%al,$0x0010+2

	out	%al,$0x06c	// 1μ秒ウェイト
	cmc
	cmc

	push	%ds
//	mov	%cs,%ax
//	mov %ax,%ds

	mov	$0x4b,%cl
	mov	_VECTOR_ADRV,%edx
	mov	_VECTOR_SEGV,%ax
	mov	_VECTOR_REAV,%ebx

	mov	%ax,%ds
	mov	$0x2507,%ax
	int	$0x21

	pop	%ds
	jnb	__noerror

//	outp(0x22,0x40);
	mov	$0x40,%al
	out %al,$0x22

	hlt

//	mov	al,00100101b
//	out	$0010h,al	/* EOI */

__noerror:
	popfl
	popal
	pop	%es
	xor	%eax,%eax

	sti

	ret
