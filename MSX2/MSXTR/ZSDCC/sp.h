/* ���ʃ}�N�� for MSX2 */

#ifndef SP_H_INCLUDE
#define SP_H_INCLUDE

#define ABS(X) ((X < 0) ? -X : X)

//#define NOERROR		0			/* �����܂�̂��� *
#define ERROR		-1
enum { NOERROR = 0, SYSERR, SYSEXIT, ERRLV1, ERRLV2, ERRLV3};

#define SCREEN_MAX_X 256

#define SCREEN_MAX_Y 212
#define CHR_TOP 0

#define SCREEN_MIN_X 0
#define SCREEN_MIN_Y 0
#define WAIT1S 60

#define MAX_SPRITE 32			/* �ő勖�e�X�v���C�g�\���� */

#define BACKCOLOR	0x00		/* �w�i��h��Ԃ��F */

#define SPBACKCOLOR	0x00	/* �X�v���C�g�̖��\��������h��Ԃ��F */

//#define TEKI_NUM_MAX 8

#define STAR_NUM	16						/* �X�^�|�̐� */

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
//EXTERN  short vram;		/* VRAM����p�ϐ� */
//EXTERN  char spram;		/* �X�v���C�gRAM����p�ϐ� */

typedef struct{
	short x;
	short y;
} SPR;

/* �e�L�����N�^�̍\����(CHR_PARA) */
typedef struct chr_para{
	struct chr_para *next,*prev;
	short x, y;
	char xx,yy;
	unsigned char pat_num,atr,count,hp;
	unsigned char pal;
//	unsigned short x_size,y_size;
//	unsigned short adr;
} CHR_PARA;

typedef struct chr_para2{
	struct chr_para *next,*prev;
	short x, y;
	int xx,yy;
	unsigned char pat_num,atr,count,hp;
	unsigned char pal;
//	unsigned short x_size,y_size;
//	unsigned short adr;
} CHR_PARA2;

typedef struct chr_para3{
//	struct chr_para *next,*prev;
	unsigned char y;
	short x;
//	char xx,yy;
	unsigned char pat_num,atr; //,count,hp;
	unsigned char pal;
//	unsigned short x_size,y_size;
//	unsigned short adr;
} CHR_PARA3;

typedef struct chr_para4{
	unsigned char pat_num,atr;
	unsigned char pal;
} CHR_PARA4;

//EXTERN CHR_PARA start[10],fin[10];		/* ���X�g�\��(���g�p) */
EXTERN CHR_PARA3 chr_data[MAX_SPRITE * 2];
EXTERN CHR_PARA3 chr_data2[MAX_SPRITE];
EXTERN CHR_PARA4 old_data[2][MAX_SPRITE];

//EXTERN unsigned char spr_num;						/* �X�v���C�g�ő�\���� */

EXTERN short scrl, scrl_spd;				/* �X�N���|���Ǘ��p */
EXTERN unsigned char star[5][STAR_NUM];		/* �X�^�|�Ǘ��p */


/**********************************************************************/

#define CONST
// const

enum {
	SPR_OFS_X = -16, //16,
	SPR_OFS_Y = -16, //-16,
/*	SPR_DIV = 8, */

/*	MAX_SPRITE = 180,*/
};


/* �^ */
/*typedef struct{
	int x;
	int y;
} SPR;
*/
/*typedef struct{
	int x;
	int y;
	int xx;
	int yy;
	unsigned char patno;
	unsigned char count;
} CHR_OBJ;
*/

typedef struct {
	unsigned char patno;
	signed char x;
	signed char y;
	unsigned char atr;
//	unsigned short x_size;
//	unsigned short y_size;
//	unsigned short adr;
} SPR_COMB;

typedef struct {
	CONST unsigned char patmax;
	CONST SPR_COMB *data;
//	unsigned short x_size;
//	unsigned short y_size;
//	unsigned short adr;
} SPR_INFO;

/* �}�N�� */
//#define SYS_WAIT sys_wait

#define SPR_SET(NO, X, Y) {\
	spr[NO].x = X;\
	spr[NO].y = Y;\
}

EXTERN CHR_PARA3 *pchr_data;

/* �X�v���C�g�ʒu���`����}�N�� */
#define DEF_SP_SINGLE( NO, X, Y, PAT, PAL, ATR) {\
	pchr_data = &chr_data[NO];\
	pchr_data->x = ((X >> SHIFT_NUM) + SPR_OFS_X); \
	pchr_data->y = ((Y >> SHIFT_NUM) + SPR_OFS_Y - 1); \
	pchr_data->pat_num = PAT; \
	pchr_data->pal = PAL; \
	if((pchr_data->x >= 256) ||(pchr_data->x <= -16)){\
		pchr_data->y = 212;\
		pchr_data->x = 0;\
		pchr_data->pat_num = 255; \
		pchr_data->atr = 0x80;\
	}\
	else if(pchr_data->x < 0){\
		pchr_data->x += 32;\
		pchr_data->atr = 0x80;\
	}else{\
		pchr_data->atr = 0x00;\
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
					sprcomb->atr\
					);\
		sprcomb++;\
	}\
}

//	printf("<%d %x>" , pat, spr_info[pat].adr);\

#endif
