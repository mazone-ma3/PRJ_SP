#ifndef FONTTW_H_INCLUDE
#define FONTTW_H_INCLUDE

#define FONTSIZEX 4
#define FONTSIZEY 8
#define FONTPARTS 64

unsigned short font_load(char *loadfil);

extern unsigned short fontdata[FONTSIZEX][FONTSIZEY][FONTPARTS];

#endif
