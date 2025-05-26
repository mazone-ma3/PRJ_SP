# SP_RUN.EXE makefile for PC-88VA OpenWatcom

#マクロ
#CC = /usr/bin/ia16-elf-gcc-6.3.0
CC= wcl

CFLAGS = -ms -bt=dos -ox -0
#-march=i8086 -mcmodel=medium
#-Wall -g -O 

#LINKOPTS = -lr
# -li86


# Cソ－スからobj作成

.SUFFIXES: .obj .c
.c.obj:
	$(CC) $< $(CFLAGS) -c


#生成手順

all:SP_RUN.EXE

OBJFILES =  SP_INIT.obj SP_MAIN.obj FONTVA.obj

SP_RUN.EXE : $(OBJFILES)
	wlink  system Dos option quiet name SP_RUN file { $(OBJFILES) }
# $(CFLAGS) $(LINKOPTS)
#	ren SP_INIT.exe SP_RUN.EXE

SP_MAIN.obj : SP_MAIN.c sp.h spr_def.h sp_com.h
# FONTVA.h

SP_INIT.obj : SP_INIT.c sp.h
# FONTVA.h

FONTVA.obj :FONTVA.c FONTVA.h
