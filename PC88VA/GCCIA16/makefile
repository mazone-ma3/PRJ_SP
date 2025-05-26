# SP_RUN.EXE makefile for PC-88VA GCCia-16

#É}ÉNÉç
CC = /usr/bin/ia16-elf-gcc-6.3.0

CFLAGS =  -march=i8086 -mcmodel=medium -O2
#-Wall -g -O 

LINKOPTS =  -li86


#ê∂ê¨éËèá

all:SP_RUN.EXE

OBJFILES =  SP_INIT.o SP_MAIN.o FONTVA.o

SP_RUN.EXE : $(OBJFILES)
	$(CC) -o SP_RUN.EXE $(OBJFILES) $(CFLAGS) $(LINKOPTS)

SP_MAIN.o : SP_MAIN.c sp.h spr_def.h sp_com.h
# FONTVA.h

SP_INIT.o : SP_INIT.c sp.h
# FONTVA.h

FONTVA.o :FONTVA.c FONTVA.h
