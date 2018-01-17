##  these are compile time constructs, that affect the running of the game
## See farther down for explaination of these and more! msw
## $Id$
##Questi sono definiti in snew.h
CPPFLAGS =  -D_GNU_SOURCE 
MOREFLAGS = -DALIGN_TEST -DBLOCK_WRITE -DENABLE_AUCTION -DACCESSI
EXTRAFLAGS = -DIMPL_SECURITY
##  these are architecture dependent flags
## TITANFLAGS = -g -43 -vector_c -safe=ptrs -DTITAN
##DEBUGFLAGS = -pg -ggdb3 -Wstrict-prototypes -finline-functions -fno-strength-reduce -pipe -fpermissive
##DEBUGFLAGS = -pg -ggdb3 -finline-functions -fno-strength-reduce -pipe -fpermissive
DEBUGFLAGS = -pg -finline-functions -fno-strength-reduce -pipe -fpermissive 

## nel caso si usi -DCYGWIN per compilare sotto windows usare anche -DGDBM_STATIC
MYFLAGS = $(MOREFLAGS) $(EXTRAFLAGS) $(DEBUGFLAGS) -m32 -w -Wno-unused -DNETBSD -DLINUX #-Wno-write-strings
## -mno-fp-ret-in-387 
CFLAGS = $(MYFLAGS) -DLOG_DEBUG -DLOG_DEBUG1 -DALAR 
LIBS = -L/usr/lib/mysql/ -lcrypt -lgdbm -lpthread -lresolv -lmysqlclient -lz -lrt -ldl
## -fno-strict-prototype
##
##define CHECK_RENT_INACTIVE 1
#define CLEAN_AT_BOOT 1
##define NOREGISTER 1
##define ACCESSI 1
## Questi sono definiti in structs.h
##define NOSCRAP        0
##define LIMITED_ITEMS  1
##define SITELOCK       1
##define NODUPLICATES   1
##define EGO_RARE       0 
##define EGO            1 
##define EGO_BLADE      1 
##define LEVEL_LOSS     1
##define NEWER_EXP      1
##define FAST_TRACK     1
#define NEW_GAIN       1
##define NEW_ROLL       1
##define NEW_CONNECT    1
##define NEW_BASH       1
##define NEW_ROLL       1
##define LOW_GOLD       0
##define DEATH_FIX      1
##define ZONE_COMM_ONLY 1
##define PREVENT_PKILL  1
##define LAG_MOBILES   1
##define DOFLEEFIGHTINGLD 1
##define ALAR_RENT 1
##
## The compiler your gonna use to compile this with...
##
##CC=/tmp/H-i586-cygwin32/bin/g++
##CC=/tmp/bin/g++-2
##CC=c:/cygwyn/bin/g++-2
CC=/usr/bin/g++
##CC=gcc
##CC=g++-2.95
##
LD=$(CC)
##
## **********************************************************
##  COMPILE TIME DEFINES, CPPFLAGS!
##
##  Exp loss switch:
##  OLD_EXP   : massima perdita di xp
##  NEW_EXP   : perdita ridotta ai primi livelli
##  NEWER_EXP : perdita ridotta a tutti i livelli
## !!!Attenzione: gli switch precedenti sono alternativi.
##  Usa EXPFLAG per impstarne UNO e UNO SOLO
##  For level loss for really low exp -DLEVEL_LOSS
##
##  To make some optimizations, you need to add things to the line above.
##  to use hash tables for rooms  (More cpu, less storage), add
##  -DHASH
##
##  To get rid of tracking (lower cpu)
##  -DNOTRACK
##
##  To save people's xp on death
##  -DDEATH_FIX
##
##  To limit items above a specific rent cost
##  -DLIMITED_ITEMS
##
##  To force new players to be authorized
##  -DPLAYER_AUTH
##
##  To add some new debugging stuff (mostly limited to malloc_debug)
##  -DDEBUG
##  To add some commands for locking out specific hosts
##  -DSITELOCK
##
## To lock out oak.grove.iup.edu from boot time.
## -DLOCKGROVE
##
##
##  To modify some commands to make item duplication near impossible:
##  -DNODUPLICATES
##
##  To have each 58+ god restricted to one site, or set of sites
##  -DIMPL_SECURITY
##
##  To use BSD memory allocation/deallocation routines instead of the
##  native memory routines:
## -DKLUDGE_MEM
##
##  If your system doesn't have built in functions of strdup() and
##  strstr(), then:
##  -DKLUDGE_STRING
##
##  If you want to use a faster mana/hit/move regen system.
##  -DNEW_GAIN
##
##  If you want to save every room and mob items use this.
##  -DSAVEWORLD
##
##  If you want to use the Quest to Gain system use this. Not all classes
##  -DQUEST_GAIN
##
## If you want to use a lower gold ration for your game use this
## -DLOW_GOLD
##
## - DUSE_EGOS (check ego on give and gets )
##
##
## -DLOG_MOB
##    LOG_MOB logs all mob command sent to command_interpreter
##
##  -DLOG_DEBUG
##    This will add a bit more info to the logs, I use it to try
##    and track down impossible bugs.
##
## -DCLEAN_AT_BOOT
##    Forces the mud to remove outdated or inactive players and
##    demote inactive gods a boot up. Slows boot process down a
##    bit.
##
## -DSTRANGE_WACK
##  When used in combination with CLEAN_AT_BOOT it will clear out corrupted
##  user files. Do this once or twice a month if needed, otherwise do not
##  use it.
##
##
## -DZONE_COMM_ONLY
##    Enabling this will force all communications to be limited to
##    the zone that person is in. Making gossip/auction/tell NOT
##    GLOBAL. Tell is limited to people in the zone you are currently
##    standing in. This does NOT include SHOUT.
##
## -DLAG_MOBILES
##    This define should make mobiles lag with commands as
##    pc's do. I.e. bashed pause, etc...
##
## -DPREVENT_PKILL
##    This define will add in a few checks in fight.c to keep
##    pc's from killing each other, willing or Un-willing.
##
## -DCHECK_RENT_INACTIVE
##    Check user inactivity against RENT_INACTIVE, if greater, wack
##    the rent file. RENT_INACTIVE located in db.c
##
## -DPERSONAL_PERM_LOCKOUTS
##    My personal annoying people I lock out perminatly. You can add
##    whomever is annoying in there.
##
## -DNEW_RENT
##    Charge 100 coins per day of rent, regardless of item types.
##
## -DNEW_ROLL
##    Abilita i nuovi metodi di rollata.
##
## -DNEW_CONNECT
##    Se il player che si sta collegando e' gia' nel mud, manda ld quello 
##    presente e accetta la nuova connessione. Serve per risolvere eventuali
##    problemi di linkdead non riconosciuti
## -DMEMTEST
##    Abilita il debug della memoria
HEADERFILES	=	config.h structs.h area.h comm.h db.h handler.h hash.h heap.h limits.h poly.h race.h script.h spells.h trap.h vt100c.h wizlist.h parser.h mail.h interpreter.h
## Rimosso da HEADERFILES utils.h	      
##
OBJS = version.o comm.o act.comm.o act.info.o act.move.o act.obj1.o act.obj2.o act.off.o act.other.o act.social.o act.wizard.o handler.o db.o interpreter.o utility.o specass2.o shop.o limits.o mobact.o fight.o modify.o weather.o spells1.o spells2.o spell_parser.o reception.o constants.o spec_procs.o signals.o board.o magic.o magic2.o skills.o opinion.o trap.o magicutils.o multiclass.o hash.o sound.o Heap.o spec_procs2.o magic3.o security.o spec_procs3.o   create.o bsd.o parser.o ansi_parser.o mail.o mindskills1.o   mem_test.o mind_use1.o create.mob.o create.obj.o rhyodin.o gilde.o   charlist.o lucertole.o breath.o snew.o doreg.o locks.o speciali.o   auction.o events.o queue.o regen.o pedit.o
##version.o 
SRCS = version.c comm.c act.comm.c act.info.c act.move.c act.obj1.c act.obj2.c act.off.c act.other.c act.social.c act.wizard.c handler.c db.c interpreter.c utility.c specass2.c shop.c limits.c mobact.c fight.c modify.c weather.c spells1.c spells2.c spell_parser.c reception.c constants.c spec_procs.c signals.c board.c magic.c magic2.c skills.c opinion.c trap.c magicutils.c multiclass.c hash.c sound.c Heap.c spec_procs2.c magic3.c security.c spec_procs3.c create.c bsd.c parser.c ansi_parser.c mail.o mind_use1.c mem_test.c mindskills1.c create.mob.c create.obj.c rhyodin.c gilde.c  charlist.c lucertole.c breath.c snew.c doreg.c locks.c speciali.c auction.c events.c queue.c regen.c pedit.c
##version.c 


all:    myst
	chmod g+rw *.o

fast:   myst
	chmod g+rw *.o

go:	fast
	(cd ..;./startmud &)
infos:	  
	rcp news localhost:lib
	rcp wiznews localhost:lib
	rcp motd localhost:lib

copy:	
	rcp myst_test localhost:svl
	rsh localhost chmod +x svl/myst	
	rsh localhost touch svl/mystnew

dist:	all
	##strip myst
	rcp myst_test localhost:svl
	rcp news localhost:lib
	rcp wiznews localhost:lib
	rcp motd localhost:lib
	rsh localhost touch svl/mystnew
	rsh localhost chmod +x svl/myst	

local:	all
	strip myst
	cp ./myst_test/mud/svl
	cp ./news /mud/lib
	cp ./wiznews /mud/lib
	cp ./motd /mud/lib
	cp ./news /mud/lib
	touch /mud/svl/mystnew

urgent:	dist
	rsh localhost touch lib/REBOOT.NOW


get:	
	rcp localhost:lib/helptbl . 
	rcp localhost:lib/help . 
	rcp localhost:lib/news .
	rcp localhost:lib/motd .
	rcp localhost:lib/wiznews .
	rcp localhost:lib/wizhelptbl .
	rcp localhost:lib/wizmotd .

getworld:	
	rcp localhost:lib/myst.* ../lib 
	rm -f ../lib/aree/*
	rcp localhost:lib/aree/* ../lib/aree
	rcp localhost:lib/objects/* ../lib/objects
	rcp localhost:lib/rooms/* ../lib/rooms

getplayers:	
	rcp localhost:lib/players/* ../lib/players 
	rcp localhost:lib/rent/* ../lib/rent

putworld:	
	rm -f ../lib/aree/*~
	rcp ../lib/aree/* localhost:lib/aree

putextraworld:	
	rm -f ../lib/objects/*~
	rm -f ../lib/rooms/*~
	rcp ../lib/objects/* localhost:lib/objects
	rcp ../lib/rooms/* localhost:lib/rooms


put:	
	rcp helptbl localhost:lib
	rcp help localhost:lib
	rcp wizhelptbl localhost:lib
	rcp ../lib/scripts.dat localhost:lib

tags:	${SRCS}
	ctags -vtrS *.[ch]

clean:	
	rm -f ${OBJS}

myst: 	 ${OBJS}

	$(LD)  $(DEBUGFLAGS) -O0  -o $@ -m32 $(mysql_config --cflags) -pthread ${OBJS} ${LIBS}
	if [ -f myst ] ; then chmod +x myst ; cp -f myst ../myst ; cp -f myst ../mudroot/myst ; rm -f myst ; fi 



depend :
	makedepend ${SRCS}



# DO NOT DELETE THIS LINE -- make depend depends on it.

comm.o:  ${HEADERFILES} status.h fight.h version.h auction.h
act.comm.o:  ${HEADERFILES} auction.h
act.move.o:  ${HEADERFILES} cmdid.h fight.h status.h
act.off.o:  ${HEADERFILES} fight.h
act.obj1.o:  ${HEADERFILES} status.h
act.obj2.o:  ${HEADERFILES}
act.info.o:  ${HEADERFILES} cmdid.h fight.h breath.h version.h
act.other.o:  ${HEADERFILES} fight.h
act.social.o:  ${HEADERFILES}
act.wizard.o:  ${HEADERFILES} cmdid.h fight.h
handler.o:  ${HEADERFILES} fight.h
db.o:  ${HEADERFILES} gilde.h status.h fight.h auction.h
interpreter.o:  ${HEADERFILES} cmdid.h breath.h interpreter.h
utility.o:  ${HEADERFILES} fight.h
specass2.o:  ${HEADERFILES} rhyodin.h carceri.h lucertole.h \
               breath.h otherproc.h roomproc.h
regen.o:   ${HEADERFILES} events.h structs.h utils.h
shop.o:  ${HEADERFILES}
limits.o:  ${HEADERFILES} fight.h
mobact.o:  ${HEADERFILES} cmdid.h status.h fight.h
fight.o:  ${HEADERFILES} fight.h cmdid.h
modify.o:  ${HEADERFILES}
weather.o:  ${HEADERFILES}
spells1.o:  ${HEADERFILES}
spells2.o:  ${HEADERFILES} fight.h
spell_parser.o:  ${HEADERFILES} cmdid.h fight.h breath.h spell_list.h auction.h
reception.o:  ${HEADERFILES} status.h
constants.o:  ${HEADERFILES}
spec_procs.o:  ${HEADERFILES} cmdid.h fight.h breath.h
signals.o:  ${HEADERFILES} status.h
board.o:  ${HEADERFILES}
magic.o:  ${HEADERFILES} fight.h
magic2.o:  ${HEADERFILES} fight.h
magic3.o:  ${HEADERFILES} fight.h
magicutils.o:  ${HEADERFILES} fight.h
multiclass.o:  ${HEADERFILES}
skills.o:  ${HEADERFILES} fight.h status.h
opinion.o:  ${HEADERFILES}
trap.o:  ${HEADERFILES} fight.h
hash.o:  ${HEADERFILES}
sound.o:  ${HEADERFILES}
Heap.o:  ${HEADERFILES}
spec_procs2.o:  ${HEADERFILES} cmdid.h status.h fight.h
spec_procs3.o:  ${HEADERFILES} cmdid.h fight.h
security.o:
create.o:  ${HEADERFILES}
bsd.o:
parser.o:  ${HEADERFILES}
ansi_parser.o:  ${HEADERFILES}
mail.o: ${HEADERFILES}
mind_use1.o:  ${HEADERFILES}
mindskills1.o: ${HEADERFILES} fight.h
create.mob.o: ${HEADERFILES}
create.obj.o: ${HEADERFILES}
rhyodin.o:    ${HEADERFILES} rhyodin.h cmdid.h charlist.h fight.h
gilde.o:      ${HEADERFILES} gilde.h cmdid.h charlist.h
charlist.o:  charlist.h
lucertole.o:  ${HEADERFILES} lucertole.h cmdid.h fight.h
breath.o:     ${HEADERFILES} breath.h fight.h
version.o:   release.h ${SRCS}
queue.c:    ${HEADERFILES} queue.h
events.c:   ${HEADERFILES} events.h
auction.o:   ${HEADERFILES} auction.h
pedit.o:   ${HEADERFILES} structs.h utils.h
