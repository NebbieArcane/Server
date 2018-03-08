/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
* $Id: specass2.c,v 1.2 2002/02/13 12:30:59 root Exp $
 * */
/***************************  System  include ************************************/
#include <sys/errno.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
/***************************  General include ************************************/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "constants.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "specass2.hpp"
#include "breath.hpp"
#include "db.hpp"
#include "interpreter.hpp"
#include "lucertole.hpp"
#include "rhyodin.hpp"
#include "shop.hpp"
#include "snew.hpp"
#include "specialproc_other.hpp"
#include "specialproc_room.hpp"
#include "utility.hpp"

namespace Alarmud {
/**
#ifdef CYGWIN
	#define errno (*__errno())
#else
#endif
**/




#define SPECFILE "myst.spe"
FILE* fd;
FILE* fp;
long ifp=0;


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* murder is disabled for now */
#define MAX_MUTYPE 16
int is_murdervict(struct char_data* ch) {
	int i;
	int mutype[] = {
		3060,  /* killing these mobs will make the PC a murderer */
		3067,
		3069,
		3072,
		3141,
		3661,
		3662,
		3663,
		3682,
		16101,
		17809,
		18215,
		18222,
		18223,
		22601,
		27011,
		-1

	};

	if( ch->nr >= 0 ) {
		for( i = 0; mutype[i] >= 0; i++ ) {
			if (mob_index[ch->nr].iVNum == mutype[i])
			{ return TRUE; }
		}
	}

	return FALSE;
}
int FileToArray(char* fname,char* p[]) {
	FILE* fp;
	char buf[1024];
	int ifp=0;
	int csize=0;
	fp=fopen(fname,"r");
	if (!fp) {
		return(ifp);
	}

	while (!feof(fp)) {
		if (fgets(buf,1023,fp)) {
			if (buf[0]) {
				if (csize==ifp) {
					csize+=10;
					realloc(p,(csize)* sizeof(char*));
					p[ifp++]=strdup(buf);
				}

			}
		}
	}
	realloc(p,ifp * (sizeof(char*)));
	fclose(fp);
	return(ifp);
}

int xcompare(const void* p1, const void* p2) {
	struct special_proc_entry* s1,*s2;
	s1 = (struct special_proc_entry*)p1;
	s2 = (struct special_proc_entry*)p2;
	return strcasecmp(s1->nome,s2->nome);
}
int nomecompare(const void* p1, const void* p2) {
	char* s1;
	struct special_proc_entry* s2;
	s1 = (char*)p1;
	s2 = (struct special_proc_entry*)p2;
	return strcasecmp(s1,s2->nome);
}

/* assign special procedures to mobiles */

char* Aggiungi(char* vecchia,char* nuova) {
	int l;
	l=vecchia?0:strlen(vecchia);
	realloc(vecchia,l+strlen(nuova));
	strcat(vecchia,nuova);
	return(vecchia);

}
void assign_speciales() {
	int lastroomproc=0;
	int lastotherproc=0;
	struct special_proc_entry* op;
	struct RoomSpecialProcEntry* _or;
	struct room_data* rp;
	int i, rnum;
	char buf[256];
	char* p;
	char procedura[256];
	char parms[256];
	char svnum[256];
	int vnum;
	char cmd[256];

	for (i=0; strcmp(otherproc[i].nome,"zFineprocedure"); i++) {
		lastotherproc++;
		if (IsTest())
		{ mudlog(LOG_CHECK,"Generic special: [%3d] %s",lastotherproc,otherproc[i].nome); }
	}
	mudlog(LOG_CHECK,"Generic procedure: %d (%d)",lastotherproc,i);

	for (i=0; strcmp(roomproc[i].nome,"zFineprocedure"); i++) {
		lastroomproc++;
		if (IsTest())
		{ mudlog(LOG_CHECK,"Room special: [%3d] %s",lastroomproc,roomproc[i].nome); }
	}
	mudlog(LOG_CHECK,"Room procedure: %d (%d)",lastroomproc,i);

	/* trovato l'ultimo elemento, eseguo un sort per sicurezza */

	mudlog(LOG_CHECK,"Room sort...[%3d]",lastroomproc);
	qsort(roomproc,lastroomproc,
		  sizeof(struct RoomSpecialProcEntry),xcompare);

	mudlog(LOG_CHECK,"Generic sort...[%3d]",lastotherproc);
	qsort(otherproc,lastotherproc,
		  sizeof(struct special_proc_entry),xcompare);

	mudlog(LOG_CHECK,"Done!");
	fd=fopen(SPECFILE,"r");
	if (!fd) {
		mudlog(LOG_ERROR,"Error opening %s: %s",SPECFILE,strerror(errno));
		return;
	}
	while (!feof(fd)) {
		fgets(buf,255,fd);
		if (*buf&& buf[strlen(buf)-1]=='\n')
		{ buf[strlen(buf)-1]='\0'; }
		vnum=-1;
		procedura[0]='\0';
		cmd[0]='\0';
		parms[0]='\0';
		p=buf;
		p=one_argument(p,cmd);
		p=one_argument(p,svnum);
		p=one_argument(p,procedura);
		only_argument(p,parms);
		vnum=atoi(svnum);
		switch (cmd[0]) {
		case '*':
			break;
		case 'm':
			rnum = real_mobile(vnum);
			if ((rnum<0) ||
					!(op=(struct special_proc_entry*)
						 bsearch(&procedura,
								 otherproc,
								 lastotherproc,
								 sizeof(struct special_proc_entry),
								 nomecompare))
			   ) {
				mudlog( LOG_ERROR,
						"mobile_assign: Mobile %d not found in database.",vnum);
			}
			else {
				mob_index[rnum].func = op->proc;
				mob_index[rnum].specname= op->nome;
				mob_index[rnum].specparms=strdup(parms);
			}
			break;
		case 'o':
			rnum = real_object(vnum);
			if ((rnum<0) ||
					!(op=(struct special_proc_entry*)
						 bsearch(&procedura,
								 otherproc,
								 lastotherproc,
								 sizeof(struct special_proc_entry),
								 nomecompare))
			   ) {
				mudlog( LOG_ERROR,
						"obj_assign: Object %d not found in database.",vnum);
			}
			else {
				obj_index[rnum].func = op->proc;
				obj_index[rnum].specname= op->nome;
				obj_index[rnum].specparms=strdup(parms);
			}
			break;
		case 'r':
			rp = real_roomp(vnum);
			if ((!rp) ||
					!(_or=(struct RoomSpecialProcEntry*)
						  bsearch(&procedura,
								  roomproc,
								  lastroomproc,
								  sizeof(struct RoomSpecialProcEntry),
								  nomecompare))
			   ) {
				mudlog( LOG_ERROR, "assign_rooms: Room %d not found in database.",
						vnum);
			}
			else {
				rp->funct=_or->proc;
				rp->specname=_or->nome;
				rp->specparms=strdup(parms);

			}
			break;
			mudlog(LOG_ERROR,"Unknown command %c %d %s %s",
				   cmd[0],vnum,procedura,parms);
			break;
		}
	}
	mudlog(LOG_CHECK,"Special procedures done");
	boot_the_shops();
	assign_the_shopkeepers();
}


} // namespace Alarmud

