# SP_RUN.EXP makefile for FM TOWNS OpenWatcom

# Cコンパイラとオプション
CC = wcc386
#CC = run386 hcd386p.exp
#CFLAGS = -lpharlap -3r -zq -zk0 -s
CFLAGS1 = -3r -zq -zk0  -ox -fo=$*.OBJ

# リンカ(オプションはPRJ_SP.LNKによる)
#LINK = EXP TLINKP

# Cソ－スからobj作成

.SUFFIXES: .OBJ .C
.C.OBJ:
	$(CC)  $< $(CFLAGS1)

# 使用するobj

OBJFILES = SND.OBJ SP_MAIN.OBJ SP_INIT.OBJ SUBFUNC.OBJ  SP68_LD.OBJ FONTTW.OBJ

#SPMAKE.OBJ 

# リンクしてEXP作成

SP_RUN.EXP : $(OBJFILES)
#	wlink system pharlap runtime CALLBUFS=65536 option quiet name SP_RUN file{ $(OBJFILES) }
	wlink system pharlap option quiet,STACK=160000 name SP_RUN file{ $(OBJFILES) }
#	wlink system pharlap option quiet,STACK=262144 name SP_RUN file{ $(OBJFILES) }
#	wlink system pharlap runtime CALLBUFS=65536 option quiet,STACK=262144 name SP_RUN file{ $(OBJFILES) }

#	$(CC) $(OBJFILES)  $(CFLAGS)
#..\SP_RUN.EXP

SP_INIT.OBJ : SP_INIT.C SP.H SND.H

#SPMAKE.OBJ : SPMAKE.C SPMAKE.H

#PLAY.OBJ : PLAY.C PLAY.H

SND.OBJ : SND.C SND.H

SUBFUNC.OBJ : SUBFUNC.C SUBFUNC.H SP.H

SP68_LD.OBJ : SP68_LD.C SP68_LD.H SP.H

SP_MAIN.OBJ : SP_MAIN.C SP_MAIN.H SP.H SPR_DEF.H SP_COM.H  FONTTW.H SND.H

FONTTW.OBJ : FONTTW.C FONTTW.H
