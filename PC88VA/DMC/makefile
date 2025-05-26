# SP_RUN.EXE makefile for PC-88VA DigitalMarsC

#マクロ
#CC = /usr/bin/ia16-elf-gcc-6.3.0
CC= dmc

CFLAGS = -msd -o+all
#-march=i8086 -mcmodel=medium
#-Wall -g -O 

LINKOPTS =  
#-li86

# Cソ－スからobj作成

.SUFFIXES: .obj .c
.c.obj:
	$(CC) $< $(CFLAGS) -c

#生成手順

all:SP_RUN.EXE

OBJFILES =  SP_INIT.obj SP_MAIN.obj FONTVA.obj

SP_RUN.EXE : $(OBJFILES)
	$(CC) -o SP_RUN.EXE $(OBJFILES) $(CFLAGS) $(LINKOPTS)

SP_MAIN.obj : SP_MAIN.c sp.h spr_def.h sp_com.h
# FONTVA.h

SP_INIT.obj : SP_INIT.c sp.h
# FONTVA.h

FONTVA.obj :FONTVA.c FONTVA.h
