# SP_RUN.X makefile for X680x0 GCC (elf2x68k)

#É}ÉNÉç
CC = m68k-xelf-gcc

#CFLAGS =  -O -Wall -g
CFLAGS = -m68000 -O2

#LINKOPTS = -liocs -ldos
# -lsignal
LINKOPTS = -Ttext=0x6800 -s
# -specs=x68knodos.specs

.SUFFIXES: .o .c
.c.o:
	$(CC) $(CFLAGS) -c $<

#ê∂ê¨éËèá

all:SP_RUN.X

OBJFILES = SP_INIT.o SUBFUNC.o SP_MAIN.o SP_LD.o PLAY.o FONT68.o

SP_RUN.X : $(OBJFILES)
	$(CC) $(LINKOPTS) -o SP_RUN.X $(OBJFILES)
# $(CFLAGS) $(LINKOPTS)

SP_INIT.o : SP_INIT.c SP.h SP_LD.h PLAY.h

SP_LD.o : SP_LD.c SP_LD.h

PLAY.o : PLAY.c PLAY.h

SUBFUNC.o : SUBFUNC.c SP.h

SP_MAIN.o : SP_MAIN.c SP.h SPR_DEF.h SP_COM.h FONT68.h

FONT68.o : FONT68.c FONT68.h
