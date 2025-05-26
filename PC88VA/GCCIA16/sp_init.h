#ifndef SP_INIT_H_INCLUDE
#define SP_INIT_H_INCLUDE

#define SPR_ATR 0x7e00
#define SPR_PAT_ADR 0xaa00
#define SPR_SIZE(X, Y) (X / 2 * Y)
#define LINE 16

#define TVRAM_ON() 	outportb(0x153, 0x51)		/* T-VRAM選択 */
#define GVRAM_ON() 	outportb(0x153, 0x54)		/* G-VRAM選択 */

//#define TVRAM_ON() 	outportb(0x153, 0x41)		/* T-VRAM選択 */
//#define GVRAM_ON() 	outportb(0x153, 0x44)		/* G-VRAM選択 */

void g_init(void);
void end(void);
void clear(unsigned short);
void sys_wait(unsigned char wait);
void pal_set(unsigned char, unsigned short,unsigned char,unsigned char,unsigned char);
void pal_all(unsigned char, unsigned char[16][3]);
void set_pal_reverse(int pal_no, unsigned char pal[16][3]); //WORD far p[16])
void set_constrast(int value, unsigned char org_pal[16][3], int pal_no);
void fadeinblack(unsigned char org_pal[16][3], int pal_no, int wait);
void fadeoutblack(unsigned char org_pal[16][3], int pal_no, int wait);
void fadeoutwhite(unsigned char org_pal[16][3], int pal_no, int wait);
void pal_allblack(int pal_no);

extern unsigned short rev_adr;
extern __far unsigned char *flame[4];
extern unsigned char org_pal[16][3];

#endif
