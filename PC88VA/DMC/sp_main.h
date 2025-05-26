#ifndef SP_MAIN_H_INCLUDE
#define SP_MAIN_H_INCLUDE

#define CHRPAL_NO 0
#define REVPAL_NO 1
#define BGPAL_NO 2

unsigned short read_sprite(void);

void bg_roll(void);
short game_run(short);
void set_sprite_locate(unsigned short pat_no, unsigned short index, unsigned char i,  unsigned short x, unsigned short y, unsigned short spr_x_size, unsigned short spr_y_size);


void __interrupt __far ip_v_sync(void);
void __interrupt __far (*keepvector)(void);

void init_v_sync(void);
void term_v_sync(void);

void wait_vsync(void);
int opening_demo(void);
int title_demo(void);
void init_star(void);
void set_sprite(void);

unsigned char keyscan(void);

extern unsigned short title_index;

#endif