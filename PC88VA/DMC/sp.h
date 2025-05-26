/* ���ʃ}�N�� for PC88VA */

#ifndef SP_H_INCLUDE
#define SP_H_INCLUDE

#define ABS(X) ((X < 0) ? -X : X)

//#define NOERROR		0			/* �����܂�̂��� *
#define ERROR		-1
enum { NOERROR = 0, SYSERR, SYSEXIT, ERRLV1, ERRLV2, ERRLV3};

#define SCREEN_MAX_X 320

#define SCREEN_MAX_Y 200
#define CHR_TOP 32

#define SCREEN_MIN_X 0
#define SCREEN_MIN_Y 0
#define WAIT1S 60

#define MAX_SPRITE 32			/* �ő勖�e�X�v���C�g�\���� */

#define BACKCOLOR	0x00		/* �w�i��h��Ԃ��F */

#define SPBACKCOLOR	0x00	/* �X�v���C�g�̖��\��������h��Ԃ��F */

//#define TEKI_NUM_MAX 16

#define STAR_NUM	24						/* �X�^�|�̐� */

#define SHIFT_NUM	3		/* ���W�n���V�t�g�����(�Œ菬���_���Z�p) */

#define SCRL_SFT 4			/* �X�N���|���̃e�X�g�p */
#define SCRL_MIN 16
#define SCRL_MAX (SCRL_MIN*SCRL_MIN) / 2

/* �֐������C�����ł̂�extern�����ɂ���}�N�� */
#ifndef MAIN
#define EXTERN extern
#else
#define EXTERN
#endif

/* ���ʕϐ� */
EXTERN unsigned char __far *vram;		/* VRAM����p�ϐ� */
EXTERN unsigned short __far *spram;		/* �X�v���C�gRAM����p�ϐ� */

typedef struct{
	short x;
	short y;
} SPR;

/* �e�L�����N�^�̍\����(CHR_PARA) */
typedef struct chr_para{
	struct chr_para *next,*prev;
	int x,y,xx,yy,pat_num,atr,count,hp;
	unsigned short x_size,y_size;
	unsigned short adr;
} CHR_PARA;

EXTERN CHR_PARA start[10],fin[10];		/* ���X�g�\��(���g�p) */
EXTERN CHR_PARA chr_data[MAX_SPRITE];

EXTERN int spr_num;						/* �X�v���C�g�ő�\���� */

EXTERN short scrl, scrl_spd;				/* �X�N���|���Ǘ��p */
EXTERN short int star[5][STAR_NUM];		/* �X�^�|�Ǘ��p */


/**********************************************************************/

#define CONST
// const

enum {
	SPR_OFS_X = -16,
	SPR_OFS_Y = -16,
/*	SPR_DIV = 8, */

/*	MAX_SPRITE = 180,*/
};


/* �^ */
/*typedef struct{
	int x;
	int y;
} SPR;
*/
typedef struct{
	int x;
	int y;
	int xx;
	int yy;
	int patno;
	int count;
} CHR_OBJ;


typedef struct {
	short patno;
	short x;
	short y;
	unsigned short atr;
	unsigned short x_size;
	unsigned short y_size;
	unsigned short adr;
} SPR_COMB;

typedef struct {
	CONST int patmax;
	CONST SPR_COMB *data;
	unsigned short x_size;
	unsigned short y_size;
	unsigned short adr;
} SPR_INFO;

/* �}�N�� */
//#define SYS_WAIT sys_wait

#define SPR_SET(NO, X, Y) {\
	spr[NO].x = X;\
	spr[NO].y = Y;\
}

/* �X�v���C�g�ʒu���`����}�N�� */
/*#define DEF_SP_SINGLE( NO, X, Y, PAT, PAL, ATR, X_SIZE, Y_SIZE, ADR) {\
	chr_data[NO].x = (((X >> SHIFT_NUM) + SPR_OFS_X) * 2 + 1024L) % 1024 ; \
	chr_data[NO].y = (((Y >> SHIFT_NUM) + SPR_OFS_Y) + 512L) % 512; \
	chr_data[NO].x_size = X_SIZE; \
	chr_data[NO].y_size = Y_SIZE; \
	chr_data[NO].adr = ADR; \
	if(!PAL){\
		chr_data[NO].adr += (rev_adr / 2); \
	}\
*/

EXTERN CHR_PARA *pchr_data;

#define DEF_SP_SINGLE( NO, X, Y, PAT, PAL, ATR, X_SIZE, Y_SIZE, ADR) {\
	pchr_data = &chr_data[NO];\
	pchr_data->x = (((X >> SHIFT_NUM) + SPR_OFS_X) * 2 + 1024L) % 1024 ; \
	pchr_data->y = (((Y >> SHIFT_NUM) + SPR_OFS_Y) + 512L) % 512; \
	pchr_data->x_size = X_SIZE; \
	pchr_data->y_size = Y_SIZE; \
	pchr_data->adr = ADR; \
	if(PAL){\
		pchr_data->adr += (rev_adr / 2); \
	}\
	if(pchr_data->y > 204L){\
		if((512L - pchr_data->y) < ((Y_SIZE / 256 / 4 + 1) * 4)){\
			pchr_data->adr += ( ((X_SIZE / 256 / 8 + 1) * 2) * (512L - pchr_data->y)); \
		}else{\
			pchr_data->y = 204L;\
		}\
	}\
	NO++; \
}

/*	chr_data[NO].pat_num = PAT | (ATR << 10); \
	chr_data[NO].atr = (PAL + 256) | 0x8000; \
*/
/*	chrpal = MK_CHRNO(0, pal_no, 0);\*/
/*		chrno++;\*/

/* �}�N���� �����X�v���C�g���� */
#define DEF_SP(chrno, X, Y, pat, pal_no) {\
\
	chrnum = spr_info[pat].patmax;\
	sprcomb = spr_info[pat].data;\
	tmp_xx = X;\
	tmp_yy = Y;\
	while(chrnum--){\
		DEF_SP_SINGLE(chrno, tmp_xx + (sprcomb->x << SHIFT_NUM),\
					 tmp_yy + (sprcomb->y << SHIFT_NUM),\
					sprcomb->patno,\
					pal_no,\
					sprcomb->atr,\
					spr_info[pat].x_size,\
					spr_info[pat].y_size,\
					spr_info[pat].adr\
					);\
		sprcomb++;\
	}\
}
/*
void DEF_SP(int chrno, int X, int Y,int  pat, int pal_no) {

	chrnum = spr_info[pat].patmax;
	sprcomb = spr_info[pat].data;
	tmp_xx = X;
	tmp_yy = Y;
	while(chrnum--){
		DEF_SP_SINGLE(chrno, tmp_xx + (sprcomb->x << SHIFT_NUM),
					 tmp_yy + (sprcomb->y << SHIFT_NUM),
					sprcomb->patno,
					pal_no,
					sprcomb->atr,
					spr_info[pat].x_size,
					spr_info[pat].y_size,
					spr_info[pat].adr
					);
		sprcomb++;
	}
}
*/
//	printf("<%d %x>" , pat, spr_info[pat].adr);\

#endif
