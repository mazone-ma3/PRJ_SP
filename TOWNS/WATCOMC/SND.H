#ifndef SND_H_INCLUDE
#define SND_H_INCLUDE

#define	SndWorkSize	16384

void SND_int(void);

char SND_elevol_init(void);

char SND_elevol_mute(char mute);
char SND_elevol_all_mute(char sw);

char SND_init(char *work);
char SND_end(void);

char SND_pcm_mode_set(char ch);

char SND_pcm_play(char ch,char note,char vol,char *buff);
char SND_pcm_play_stop(char ch);

char SND_fm_read_status(void);
char SND_fm_write_data(char bank,char reg,char data);
char SND_fm_write_save_data(char bank,char reg, char data);
char SND_fm_read_save_data(char bank,char reg,char *data);

char SND_int_timer_a1(char sw, short counter);
char SND_int_timer_b1(char sw, short counter);

char SND_int_timer_a2(void);
char SND_int_timer_b2(void);

#endif
