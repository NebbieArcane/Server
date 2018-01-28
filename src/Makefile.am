bin_PROGRAMS = myst

DEBUGFLAGS = -pg -ggdb -finline-functions -fno-strength-reduce -pipe -fpermissive 
MOREFLAGS = -DALIGN_TEST -DBLOCK_WRITE -DENABLE_AUCTION -DACCESSI
EXTRAFLAGS = -DIMPL_SECURITY
MYFLAGS = $(MOREFLAGS) $(EXTRAFLAGS) $(DEBUGFLAGS) -Wno-unused-result -Wno-unused -Wno-write-strings -Wno-endif-labels -DNETBSD -DLINUX 
## -mno-fp-ret-in-387 
AM_CFLAGS = -O0 $(MYFLAGS) -DLOG_DEBUG -DLOG_DEBUG1 -DALAR -I/usr/include/mysql 
#AM_CPPFLAGS =  $(AM_CFLAGS)
#-L/usr/lib/mysql/ -lmysqlclient to be added when the 64 bit switch is completed
LIBS = -lcrypt -lpthread -lresolv  -lz -lrt -ldl 
AM_LDFLAGS = $(DEBUGFLAGS)  -pthread $(LIBS) 

#HEADERFILES	=	config.h structs.h area.h comm.h db.h handler.h hash.h heap.h limits.h poly.h race.h script.h spells.h trap.h vt100c.h wizlist.h parser.h mail.h interpreter.h
## Rimosso da HEADERFILES utils.h	      
##
#OBJS = version.o comm.o act.comm.o act.info.o act.move.o act.obj1.o act.obj2.o act.off.o act.other.o act.social.o act.wizard.o handler.o mail.o db.o interpreter.o utility.o specass2.o shop.o maximums.o mobact.o fight.o modify.o weather.o spells1.o spells2.o spell_parser.o reception.o constants.o spec_procs.o signals.o board.o magic.o magic2.o skills.o opinion.o trap.o magicutils.o multiclass.o hash.o sound.o Heap.o spec_procs2.o magic3.o security.o spec_procs3.o create.o bsd.o parser.o ansi_parser.o mindskills1.o mem_test.o mind_use1.o create.mob.o create.obj.o rhyodin.o gilde.o charlist.o lucertole.o breath.o snew.o doreg.o locks.o speciali.o auction.o events.o queue.o regen.o pedit.o
##version.o 
#SRCS = version.c comm.c act.comm.c act.info.c act.move.c act.obj1.c act.obj2.c act.off.c act.other.c act.social.c act.wizard.c handler.c mail.c db.c interpreter.c utility.c specass2.c shop.c maximums.c mobact.c fight.c modify.c weather.c spells1.c spells2.c spell_parser.c reception.c constants.c spec_procs.c signals.c board.c magic.c magic2.c skills.c opinion.c trap.c magicutils.c multiclass.c hash.c sound.c Heap.c spec_procs2.c magic3.c security.c spec_procs3.c create.c bsd.c parser.c ansi_parser.c mindskills1.c mem_test.c mind_use1.c create.mob.c create.obj.c rhyodin.c gilde.c charlist.c lucertole.c breath.c snew.c doreg.c locks.c speciali.c auction.c events.c queue.c regen.c pedit.c 
##version.c 
include  $(top_srcdir)/sourcelist
myst_SOURCES=$(SRCS)

myst-am:
	cp myst ../mudroot/myst	
