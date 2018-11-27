/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*AlarMUD*/
/***************************  System  include ************************************/
#include <cstring>
#include <cctype>
#include <cstdio>
#include <string>
#include <arpa/telnet.h>
#include <unistd.h>
#include <cstdlib>
#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <curl/curl.h>
#include "../contrib/slacking/slacking.hpp"
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
#include "interpreter.hpp"
#include "Registered.hpp"
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.obj1.hpp"
#include "act.obj2.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
#include "breath.hpp"
#include "comm.hpp"
#include "create.hpp"
#include "create.mob.hpp"
#include "create.obj.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "mail.hpp"
#include "modify.hpp"
#include "parser.hpp"
#include "reception.hpp"
#include "regen.hpp"
#include "security.hpp"
#include "skills.hpp"
#include "spec_procs3.hpp"
#include "spell_parser.hpp"
#include "Sql.hpp"

namespace Alarmud {
using std::string;


/* $Id: interpreter.c,v 1.1.1.1 2002/02/13 11:14:53 root Exp $
 * */
#define NOT !
#define AND &&
#define OR ||
#define SEND_TO_Q2(msg,d) write_to_descriptor(d->descriptor,msg);
#define STATE(d) ((d)->connected)

int WizLock;
int Silence = 0;
long SystemFlags=0;         /* used for making teleport/astral/stuff not work */
int plr_tick_count=0;
typedef std::array<nanny_func,E_CONNECTION_TYPES_COUNT> t_nannies;
t_nannies nannyFuncs;//[E_CONNECTION_TYPES_COUNT];


/* this is how we tell which race gets which class ! */
/* to add a new class seletion add the CLASS_NAME above the */
/* message 'NEW CLASS SELETIONS HERE'  */

/* these are the allowable PC races */
const int race_choice[]= {
	RACE_DWARF,
	RACE_ELVEN,
	RACE_GOLD_ELF,
	RACE_WILD_ELF,
	RACE_HUMAN,
	RACE_GNOME,
	RACE_HALFLING,       /*Nascono allo shire*/
	RACE_HALF_ELVEN,
	RACE_HALF_ORC,
	RACE_HALF_OGRE,
	RACE_HALF_GIANT,
	RACE_DARK_ELF,                /* bad guys here */
#if 1 /*Era 0, adesso tutte le razze gia' previste sono abilitate. GGPATCH*/
	RACE_ORC,             /*Nascono allo shire */
	RACE_GOBLIN,          /*Nascono allo shire */
	RACE_TROLL,           /*Nascono allo shire */
	RACE_DARK_DWARF,      /*Nascono allo shire */
	RACE_DEEP_GNOME,      /*Nascono allo shire */
	RACE_SEA_ELF,         /*Nascono vicino al mare */
	RACE_DEMON,
#endif
	-1
};

/* WARNING: do not remove the 0 at the end of the const, will cause your */
/*             mud to die hard! */


const int elf_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_PALADIN,
	CLASS_RANGER,
	CLASS_PSI,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
	CLASS_RANGER+CLASS_CLERIC,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int wild_elf_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_RANGER,
	CLASS_PSI,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_CLERIC+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_DRUID,
	CLASS_WARRIOR+CLASS_MAGIC_USER+CLASS_CLERIC,
	CLASS_WARRIOR+CLASS_CLERIC,
	CLASS_RANGER+CLASS_CLERIC,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int dwarf_class_choice[]= {
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_PALADIN,
	CLASS_PSI,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_CLERIC,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int halfling_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_THIEF+CLASS_WARRIOR,
	CLASS_DRUID+CLASS_WARRIOR,
	CLASS_PSI+CLASS_WARRIOR,
	CLASS_PSI+CLASS_THIEF,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int gnome_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_PSI,
	CLASS_PSI,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int deep_gnome_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_MONK,
	CLASS_CLERIC+CLASS_MONK,
	CLASS_CLERIC+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_THIEF,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int human_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_BARBARIAN,
	CLASS_PALADIN,
	CLASS_RANGER,
	CLASS_PSI,
	CLASS_WARRIOR+CLASS_DRUID,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_CLERIC,
	CLASS_WARRIOR+CLASS_CLERIC+CLASS_MAGIC_USER,
	CLASS_THIEF+CLASS_CLERIC+CLASS_MAGIC_USER,
	/* NEW CLASS SELECTIONS HERE */
	0
};


const int half_orc_class_choice[]= {
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_CLERIC+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_CLERIC+CLASS_THIEF,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int half_ogre_class_choice[]= {
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_CLERIC+CLASS_WARRIOR,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int half_giant_class_choice[]= {
	CLASS_WARRIOR,
	CLASS_BARBARIAN,
	CLASS_PSI,
	/* NEW CLASS SELECTIONS HERE */
	0
};


const int orc_class_choice[]= {
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_CLERIC,
	CLASS_BARBARIAN,
	CLASS_THIEF+CLASS_CLERIC,
	/* new class below here */
	0
};

const int goblin_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_CLERIC,
	CLASS_BARBARIAN,
	CLASS_WARRIOR+CLASS_THIEF,
	/* new class below here */
	0
};

const int demon_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	0
};

const int troll_class_choice[]= {
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_CLERIC,
	CLASS_BARBARIAN,
	CLASS_WARRIOR+CLASS_CLERIC,
	/* new clases below here */
	0
};

const int default_class_choice[]= {
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_CLERIC,
	CLASS_MAGIC_USER,
	CLASS_BARBARIAN,
	CLASS_PALADIN,
	CLASS_RANGER,
	CLASS_PSI,
	/* new clases below here */
	0
};

const int dark_dwarf_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_MONK,
	CLASS_CLERIC+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_WARRIOR+CLASS_CLERIC,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const int dark_elf_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_PALADIN,
	CLASS_RANGER,
	CLASS_PSI,
	CLASS_MAGIC_USER+CLASS_CLERIC,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
	/* NEW CLASS SELECTIONS HERE */
	0
};

const char* fill[]= {
	"in",
	"from",
	"with",
	"and",
	"the",
	"on",
	"at",
	"to",
	"\n"
};
int search_block(const char* arg, const char** list, bool exact) {
	size_t l=std::max(strlen(arg),1UL);
	for(int i=0;** (list+i) != '\n'; i++) {
		if(exact) {
			if(!strcasecmp(arg, *(list+i))) {
				return(i);
			}
		}
		else {
			if(!strncasecmp(arg, *(list+i),l)) {
				return(i);
			}
		}
	}
	return -1;
}
int old_search_block(const char* argument,int begin,int length,const char** list,int mode) {
	std::string key(argument);
	int rc=search_block(key.substr(begin,length).c_str(),list,mode);
	return rc>=0?rc+1:rc;
}
#if 0
int search_block(char* arg, const char** list, bool exact) {
	register int i,l;

	/* Make into lower case, and get length of string */
	for(l=0; *(arg+l); l++) {
		*(arg+l)=LOWER(*(arg+l));
	}

	if(exact) {
		for(i=0;** (list+i) != '\n'; i++)
			if(!strcmp(arg, *(list+i))) {
				return(i);
			}
	}
	else {
		if(!l) {
			l=1;    /* Avoid "" to match the first available string */
		}
		for(i=0;** (list+i) != '\n'; i++)
			if(!strncmp(arg, *(list+i), l)) {
				return(i);
			}
	}

	return(-1);
}


int old_search_block(const char* argument,int begin,int length,const char** list,int mode) {
	int guess, found, search;


	/* If the word contain 0 letters, then a match is already found */
	found = (length < 1);

	guess = 0;

	/* Search for a match */

	if(mode)
		while(NOT found AND *(list[guess]) != '\n') {
			found=(length==(int)strlen(list[guess]));
			for(search=0; (search < length AND found); search++) {
				found=(*(argument+begin+search)== *(list[guess]+search));
			}
			guess++;
		}
	else {
		while(NOT found AND *(list[guess]) != '\n') {
			found=1;
			for(search=0; (search < length AND found); search++) {
				found=(*(argument+begin+search)== *(list[guess]+search));
			}
			guess++;
		}
	}

	return (found ? guess : -1);
}
#endif
void command_interpreter(struct char_data* ch, const char* argument) {
	char buf[254];
	NODE* n;
	char buf1[255], buf2[255];
	struct char_data* temp;

	REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

	/* PARRY is removed as HIDE GAIA 2001 */

	if(IS_AFFECTED2(ch,AFF2_PARRY) && !ch->specials.fighting) {
		act("$c0006$n smette di ripararsi con lo scudo.", TRUE, ch, 0, 0, TO_ROOM);
		act("$c0006Smetti di proteggerti con lo scudo.", TRUE, ch, 0, 0, TO_CHAR);
		REMOVE_BIT(ch->specials.affected_by2, AFF2_PARRY);
	}

	if(ch->listening_to) {
		REMOVE_FROM_LIST(ch, real_roomp(ch->listening_to)->listeners, next_listener);
		ch->listening_to = 0;
		act("Smetti di origliare la stanza.", TRUE, ch, 0, 0, TO_CHAR);
	}

	if(IS_AFFECTED2(ch,AFF2_AFK)) {
		act("$c0006$n ritorna alla sua tastiera", TRUE, ch, 0, 0, TO_ROOM);
		act("$c0006Ritorni alla tua tastiera.", TRUE, ch, 0, 0, TO_CHAR);
		REMOVE_BIT(ch->specials.affected_by2, AFF2_AFK);
	}

	if(MOUNTED(ch)) {
		if(ch->in_room != MOUNTED(ch)->in_room) {
			Dismount(ch, MOUNTED(ch), POSITION_STANDING);
		}
	}

	/*
	 *  a bug check.
	 */
	if(!IS_NPC(ch)) {
		int i, found=FALSE;
		if((!ch->player.name[0]) || (ch->player.name[0]<' ')) {
			mudlog(LOG_SYSERR, "Error in character name.  Changed to 'Error'");
			free(ch->player.name);
			ch->player.name = (char*)malloc(10);
			strncpy(ch->player.name, "Error\0",6);
			/*    return; Vediamo se stracrasha o no.... */
		}
		strcpy(buf, ch->player.name);
		for(i = 0; i< (int)strlen(buf) && !found; i++) {
			if(buf[i]<65) {
				found = TRUE;
			}
		}
		if(found) {
			mudlog(LOG_SYSERR, "Error in character name.  Changed to 'Error'");
			free(ch->player.name);
			ch->player.name = (char*)malloc(6);
			strncpy(ch->player.name, "Error\0",6);
			/*    return; Vediamo se stracrasha o no.... */
		}
	} /* Fine if IS_NPC */

	/* Toglie gli spazi prima del comando. */
	for(; isspace(*argument); argument++) ;

	if(*argument && *argument != '\n') {
		if(!isalpha(*argument)) {
			buf1[0] = *argument;
			buf1[1] = '\0';
			if(*(argument + 1)) {
				strcpy(buf2, argument + 1);
			}
			else {
				buf2[0] = '\0';
			}
		}
		else {
			int i=0;
			half_chop(argument, buf1, buf2,sizeof buf1 -1,sizeof buf2 -1);
			while(buf1[i] != '\0') {
				buf1[i] = LOWER(buf1[i]);
				i++;
			}
		}

		/* New parser by DM */
		if(*buf1) {
			n = FindValidCommand(buf1);
		}
		else {
			n = NULL;
		}

		/* GGNOTA test sui livelli dei comandi*/

		if(n != NULL && GetMaxLevel(ch) >= n->min_level) {
			if(n->func != 0) {
				if(GET_POS(ch) < n->min_pos) {
					switch(GET_POS(ch)) {
					case POSITION_DEAD:
						send_to_char("Non puoi essere ancora qui. Sei MORTO!!!\n\r",
									 ch);
						break;
					case POSITION_INCAP:
					case POSITION_MORTALLYW:
						send_to_char("Sei in una brutta posizione per poter fare "
									 "qualunque cosa!\n\r", ch);
						break;

					case POSITION_STUNNED:
						send_to_char("Tutto quello che puoi fare ora, e` respirare "
									 "e sperare che passi!\n\r", ch);
						break;
					case POSITION_SLEEPING:
						send_to_char("Nei tuoi sogni o cosa?\n\r", ch);
						break;
					case POSITION_RESTING:
						send_to_char("Nah... Sei troppo rilassato...\n\r", ch);
						break;
					case POSITION_SITTING:
						send_to_char("Prima forse dovresti alzarti, non credi?\n\r",ch);
						break;
					case POSITION_FIGHTING:
						send_to_char("Non puoi! Stai combattendo per la tua vita!\n\r",
									 ch);
						break;
					case POSITION_STANDING:
						send_to_char("Ho paura che tu non lo possa fare.\n\r", ch);
						break;
					} /* switch */
				}
				else if(!IS_AFFECTED(ch, AFF_PARALYSIS)) {
					/* They can't move, must have pissed off an immo!         */
					/* make sure polies can move, some mobs have this bit set */
					if(IS_SET(ch->specials.act, PLR_FREEZE) &&
							(IS_SET(ch->specials.act, ACT_POLYSELF) || IS_PC(ch))) {
						send_to_char("Sei stato immobilizzato e non puoi fare nulla!\n\r", ch);
					}
					else {
						if((IS_SET(SystemFlags, SYS_LOGALL) &&
								(IS_PC(ch) ||
								 IS_SET(ch->specials.act, ACT_POLYSELF))) ||
								(IS_SET(SystemFlags, SYS_LOGMOB) &&
								 (IS_NPC(ch) &&
								  !IS_SET(ch->specials.act, ACT_POLYSELF))))
						{
							mudlog(LOG_CHECK,
								   "[%5ld]ACMD %s:%s", ch->in_room, ch->player.name,
								   argument);
						}
						else if(n->log) {
							mudlog(LOG_CHECK,
								   "[%5ld]CCMD %s:%s", ch->in_room, ch->player.name,
								   argument);
						}
						else if(IS_AFFECTED2(ch, AFF2_LOG_ME)) {
							mudlog(LOG_CHECK,
								   "[%5ld]PCMD %s:%s", ch->in_room, ch->player.name,
								   argument);
						}
						else if(GetMaxLevel(ch) >= IMMORTALE &&
								GetMaxLevel(ch) < 60) {
							mudlog(LOG_CHECK,
								   "[%5ld]ICMD %s:%s", ch->in_room, ch->player.name,
								   argument);
						}
						else if(GET_GOLD(ch) > 2000000) {
							mudlog(LOG_CHECK,
								   "[%5ld]GCMD %s:%s", ch->in_room, ch->player.name,
								   argument);
						}

						/* special() restituisce TRUE se il comando e` stato
						 * interpretato da una procedura speciale.
						 */
						if(no_specials || !special(ch, n->number, buf2)) {
							/* Finalmente viene esequito il comando */
							((*n->func)(ch, buf2, n->number));
						}
					}
				}
				else {
					send_to_char("Sei paralizzato, non puoi fare molto!\n\r",ch);
				}
			}
			else /* n->func == 0 */
				send_to_char("Mi spiace, ma il comando non e` stato ancora "
							 "implementato.\n\r",ch);
		}
		else { /* n == NULL || GetMaxLevel( ch ) < n->min_level */
			send_to_char("Pardon?\n\r", ch);
		}
	}
}

void argument_interpreter(const char* argument,char* first_arg,char* second_arg) {
	int look_at, begin;

	begin = 0;

	do {
		/* Find first non blank */
		for(; *(argument + begin) == ' ' ; begin++) ;

		/* Find length of first word */
		for(look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)
			/* Make all letters lower case, AND copy them to first_arg */
		{
			*(first_arg + look_at) = LOWER(*(argument + begin + look_at));
		}

		*(first_arg + look_at) = '\0';
		begin += look_at;
	}
	while(fill_word(first_arg));

	do {
		/* Find first non blank */
		for(; *(argument + begin) == ' ' ; begin++) ;

		/* Find length of first word */
		for(look_at = 0; *(argument + begin + look_at) > ' ' ; look_at++)
			/* Make all letters lower case, AND copy them to second_arg */
		{
			*(second_arg + look_at) = LOWER(*(argument + begin + look_at));
		}

		*(second_arg + look_at) = '\0';
		begin += look_at;

	}
	while(fill_word(second_arg));
}

void ThreeArgumentInterpreter(char* pchArgument, char* pchFirstArg,
							  char* pchSecondArg, char* pchThirdArg) {
	int nLookAt, nBegin;

	nBegin = 0;

	do {
		/* Find first non blank */
		for(; *(pchArgument + nBegin) == ' ' ; nBegin++) ;

		/* Find length of first word */
		for(nLookAt = 0; *(pchArgument + nBegin + nLookAt) > ' ' ; nLookAt++)
			/* Make all letters lower case, AND copy them to first_arg */
		{
			*(pchFirstArg + nLookAt) = LOWER(*(pchArgument + nBegin + nLookAt));
		}

		*(pchFirstArg + nLookAt) = '\0';
		nBegin += nLookAt;
	}
	while(fill_word(pchFirstArg));

	do {
		/* Find first non blank */
		for(; *(pchArgument + nBegin) == ' ' ; nBegin++) ;

		/* Find length of first word */
		for(nLookAt = 0; *(pchArgument + nBegin + nLookAt) > ' ' ; nLookAt++)
			/* Make all letters lower case, AND copy them to second_arg */
			*(pchSecondArg + nLookAt) =
				LOWER(*(pchArgument + nBegin + nLookAt));

		*(pchSecondArg + nLookAt) = '\0';
		nBegin += nLookAt;

	}
	while(fill_word(pchSecondArg));

	do {
		/* Find first non blank */
		for(; *(pchArgument + nBegin) == ' ' ; nBegin++) ;

		/* Find length of first word */
		for(nLookAt = 0; *(pchArgument + nBegin + nLookAt) > ' ' ; nLookAt++)
			/* Make all letters lower case, AND copy them to second_arg */
		{
			*(pchThirdArg + nLookAt) = LOWER(*(pchArgument + nBegin + nLookAt));
		}

		*(pchThirdArg + nLookAt) = '\0';
		nBegin += nLookAt;

	}
	while(fill_word(pchThirdArg));
}

int is_number(char* str) {
	int look_at;

	if(*str =='-' || *str =='+') {
		look_at =1;
	}
	else {
		look_at =0;
	}
	for(; str[ look_at ] != '\0'; look_at++)
		if(str[ look_at ] < '0' || str[ look_at ] > '9') {
			return FALSE;
		}
	return TRUE;
}

/* find the first sub-argument of a string, return pointer to first char in
 *  primary argument, following the sub-arg
 */
const char* one_argument(const char* argument, char* first_arg) {
	/* Ritorna un argomento, ignorando le parole definite in filler */
	int begin,look_at;

	begin = 0;

	do {
		/* Find first non blank */
		for(; isspace(*(argument + begin)); begin++);

		/* Find length of first word */
		for(look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

			/* Make all letters lower case,
			* AND copy them to first_arg */
		{
			*(first_arg + look_at) = LOWER(*(argument + begin + look_at));
		}

		*(first_arg + look_at)='\0';
		begin += look_at;
	}
	while(fill_word(first_arg));

	return(argument+begin);
}

const char* OneArgumentNoFill(const char* argument, char* first_arg) {
	int begin,look_at;

	begin = 0;

	/* Find first non blank */
	for(; isspace(*(argument + begin)); begin++);

	/* Find length of first word */
	for(look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

		/* Make all letters lower case,
		 * AND copy them to first_arg */
	{
		*(first_arg + look_at) = LOWER(*(argument + begin + look_at));
	}

	*(first_arg + look_at)='\0';
	begin += look_at;

	return(argument+begin);
}


void only_argument(const char* argument, char* dest)
/* Trimma i blanks iniziali e copia argument in dest */
{
	while(*argument && isspace(*argument)) {
		argument++;
	}
	strcpy(dest, argument);
}




int fill_word(const char* argument) {
	return (search_block(argument,fill,TRUE) >= 0);
}





/* determine if a given string is an abbreviation of another */
int is_abbrev(const char* arg1, const char* arg2) {
	return (!strncasecmp(arg1,arg2,strlen(arg1)));
}




/**
 * Split the string in two at the first space
 * Arguments returned are guaranteed to not be longer than requeste.
 * Default length is 99 for both arguments
 */
void half_chop(const char* argument, char* arg1, char* arg2,size_t len1,size_t len2) {
	std::string work(argument);
	try {
		boost::algorithm::trim_left(work);
	}
	catch(exception &e) {
		LOG_ALERT("Chopping " << work << " " << e.what());
	}
	size_t space=work.find_first_of(" ");
	if(space==std::string::npos) {  // No space found, only one argument
		arg2[0]='\0';
		std::strcpy(arg1,work.substr(0,len1).c_str());
	}
	else {
		std::strcpy(arg1,work.substr(0,min<size_t>(space,len1)).c_str());
		work=work.substr(space);
		boost::algorithm::trim_left(work);
		std::strcpy(arg2,work.substr(0,len2).c_str());
	}
	return;
	/*



		mudlog(LOG_ALWAYS,"Passed %s(%d), Sizeof arg1=%d arg2=%d, Len arg1=%d arg2=%d",argument,strlen(argument),sizeof *arg1,sizeof *arg2,sizeof arg1,sizeof arg2);
		char* tmp1;
		char* tmp2;
		char* p1;
		char* p2;
		int i;
		i=0;
		tmp1=(char*)calloc(1,strlen(argument)+1);
		tmp2=(char*)calloc(1,strlen(argument)+1);
		p1=tmp1;
		p2=tmp2;
		for (; isspace(*argument); argument++);

		for (; !isspace(*p1 = *argument) && *argument && i <49; i++,argument++, p1++);
		*p1='\0';
		for (; isspace(*p1) && *p1; p1++);

		for (; isspace(*argument); argument++);

		for (; ( *p2 = *argument ) != 0; argument++, p2++);
		strncpy(arg1,tmp1,50);
		strcpy(arg2,tmp2);
		free(tmp1);
		free(tmp2);
		*/
}



int special(struct char_data* ch, int cmd, const char* arg) {
	register struct obj_data* i;
	register struct char_data* k;
	int j;
	if(ch->in_room == NOWHERE) {
		char_to_room(ch, 3001);
		return FALSE;
	}

	/* special in room? */
	if(real_roomp(ch->in_room)->funct) {
		if((*real_roomp(ch->in_room)->funct)(ch, cmd, arg,
											 real_roomp(ch->in_room),
											 EVENT_COMMAND)) {
			return(TRUE);
		}
	}

	/* special in equipment list? */
#define CUR_OBJ ch->equipment[j]

	for(j = 0; j <= (MAX_WEAR - 1); j++) {
		if(CUR_OBJ && CUR_OBJ->item_number >= 0) {
			if(IS_SET(CUR_OBJ->obj_flags.extra_flags, ITEM_ANTI_SUN)) {
				AntiSunItem(ch, cmd, arg, CUR_OBJ, EVENT_COMMAND);
			}
			if(CUR_OBJ) {
				if(obj_index[ CUR_OBJ->item_number ].func) {
					PushStatus(obj_index[CUR_OBJ->item_number].specname);
					if((*obj_index[ CUR_OBJ->item_number ].func)
							(ch, cmd, arg, CUR_OBJ, EVENT_COMMAND)) {
						PopStatus();
						return(TRUE);
					}
					PopStatus();
				}
			}
		}
	}

	/* special in inventory? */
	for(i = ch->carrying; i; i = i->next_content)
		if(i->item_number >= 0)
			if(obj_index[ i->item_number ].func) {
				PushStatus(obj_index[i->item_number].specname);
				if((*obj_index[ i->item_number ].func)(ch, cmd, arg, i,
													   EVENT_COMMAND)) {
					PopStatus();
					return(TRUE);
				}
				PopStatus();
			}

	/* special in mobile present? */
	for(k = real_roomp(ch->in_room)->people; k; k = k->next_in_room)
		if(IS_MOB(k))
			if(mob_index[ k->nr ].func) {
				PushStatus(mob_index[k->nr].specname);

				if((*mob_index[ k->nr ].func)(ch, cmd, arg, k, EVENT_COMMAND)) {
					PopStatus();
					return(TRUE);
				}
				PopStatus();
			}

	/* special in object present? */
	for(i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
		if(i->item_number >=0)
			if(obj_index[ i->item_number ].func) {
				PushStatus(obj_index[i->item_number].specname);
				if((*obj_index[ i->item_number ].func)(ch, cmd, arg, i,
													   EVENT_COMMAND)) {
					PopStatus();
					return(TRUE);
				}
				PopStatus();
			}

	return(FALSE);
}
void assign_command_pointers() {
	InitRadix();
	AddCommand( "north",                do_move,            CMD_NORTH,                  POSITION_STANDING,  TUTTI                   );  /*    1 */
	AddCommand( "east",                 do_move,            CMD_EAST,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "south",                do_move,            CMD_SOUTH,                  POSITION_STANDING,  TUTTI                   );
	AddCommand( "west",                 do_move,            CMD_WEST,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "up",                   do_move,            CMD_UP,                     POSITION_STANDING,  TUTTI                   );  /*    5 */
	AddCommand( "down",                 do_move,            CMD_DOWN,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "enter",                do_enter,           CMD_ENTER,                  POSITION_STANDING,  TUTTI                   );
	AddCommand( "exits",                do_exits,           CMD_EXITS,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "kiss",                 do_action,          CMD_KISS,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "get",                  do_get,             CMD_GET,                    POSITION_RESTING,   TUTTI                   );  /*   10 */
	AddCommand( "drink",                do_drink,           CMD_DRINK,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "eat",                  do_eat,             CMD_EAT,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "wear",                 do_wear,            CMD_WEAR,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "wield",                do_wield,           CMD_WIELD,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "look",                 do_look,            CMD_LOOK,                   POSITION_RESTING,   TUTTI                   );  /*   15 */
	AddCommand( "score",                do_score,           CMD_SCORE,                  POSITION_DEAD,      TUTTI                   );
	AddCommand( "say",                  do_new_say,         CMD_SAY,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "shout",                do_shout,           CMD_SHOUT,                  POSITION_RESTING,   2                       );
	AddCommand( "tell",                 do_tell,            CMD_TELL,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "inventory",            do_inventory,       CMD_INVENTORY,              POSITION_DEAD,      TUTTI                   );  /*   20 */
	AddCommand( "qui",                  do_qui,             CMD_QUI,                    POSITION_DEAD,      TUTTI                   );
	AddCommand( "bounce",               do_action,          CMD_BOUNCE,                 POSITION_STANDING,  TUTTI                   );
	AddCommand( "smile",                do_action,          CMD_SMILE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "dance",                do_action,          CMD_DANCE,                  POSITION_STANDING,  TUTTI                   );
	AddCommand( "kill",                 do_kill,            CMD_KILL,                   POSITION_FIGHTING,  TUTTI                   );  /*   25 */
	AddCommand( "cackle",               do_action,          CMD_CACKLE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "laugh",                do_action,          CMD_LAUGH,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "giggle",               do_action,          CMD_GIGGLE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "shake",                do_action,          CMD_SHAKE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "puke",                 do_vomita,          CMD_PUKE,                   POSITION_RESTING,   TUTTI                   );  /*   30 */
	AddCommand( "growl",                do_action,          CMD_GROWL,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "scream",               do_action,          CMD_SCREAM,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "insult",               do_insult,          CMD_INSULT,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "comfort",              do_action,          CMD_COMFORT,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "nod",                  do_action,          CMD_NOD,                    POSITION_RESTING,   TUTTI                   );  /*   35 */
	AddCommand( "sigh",                 do_action,          CMD_SIGH,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "sulk",                 do_action,          CMD_SULK,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "help",                 do_help,            CMD_HELP,                   POSITION_DEAD,      TUTTI                   );
	AddCommand( "who",                  do_who,             CMD_WHO,                    POSITION_DEAD,      TUTTI                   );
	AddCommand( "emote",                do_emote,           CMD_EMOTE,                  POSITION_SLEEPING,  TUTTI                   );  /*   40 */
	AddCommand( ":",                    do_emote,           CMD_EMOTE,                  POSITION_SLEEPING,  TUTTI                   );  /*   40 */
	AddCommand( "echo",                 do_echo,            CMD_ECHO,                   POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "stand",                do_stand,           CMD_STAND,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "sit",                  do_sit,             CMD_SIT,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "rest",                 do_rest,            CMD_REST,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "sleep",                do_sleep,           CMD_SLEEP,                  POSITION_SLEEPING,  TUTTI                   );  /*   45 */
	AddCommand( "wake",                 do_wake,            CMD_WAKE,                   POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "force",                do_force,           CMD_FORCE,                  POSITION_SLEEPING,  MAESTRO_DEL_CREATO      );
	AddCommand( "transfer",             do_trans,           CMD_TRANSFER,               POSITION_SLEEPING,  QUESTMASTER             );
	AddCommand( "hug",                  do_action,          CMD_HUG,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "snuggle",              do_action,          CMD_SNUGGLE,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "cuddle",               do_action,          CMD_CUDDLE,                 POSITION_RESTING,   TUTTI                   );  /*   50 */
	AddCommand( "nuzzle",               do_action,          CMD_NUZZLE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "cry",                  do_action,          CMD_CRY,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "news",                 do_news,            CMD_NEWS,                   POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "equipment",            do_equipment,       CMD_EQUIPMENT,              POSITION_SLEEPING,  TUTTI                   );  /*   55 */
	AddCommand( "buy",                  do_not_here,        CMD_BUY,                    POSITION_STANDING,  TUTTI                   );
	AddCommand( "sell",                 do_not_here,        CMD_SELL,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "value",                do_value,           CMD_VALUE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "list",                 do_not_here,        CMD_LIST,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "drop",                 do_drop,            CMD_DROP,                   POSITION_RESTING,   TUTTI                   );  /*   60 */
	AddCommand( "goto",                 do_goto,            CMD_GOTO,                   POSITION_SLEEPING,  IMMORTALE               );
	AddCommand( "weather",              do_weather,         CMD_WEATHER,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "read",                 do_read,            CMD_READ,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "pour",                 do_pour,            CMD_POUR,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "grab",                 do_grab,            CMD_DUMMY,                  POSITION_RESTING,   TUTTI                   );  /*   65 */
	AddCommand( "remove",               do_remove,          CMD_REMOVE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "put",                  do_put,             CMD_PUT,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "shutdow",              do_shutdow,         CMD_SHUTDOW,                POSITION_DEAD,      MAESTRO_DEI_CREATORI    );
	AddCommand( "save",                 do_save,            CMD_SAVE,                   POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "hit",                  do_hit,             CMD_HIT,                    POSITION_FIGHTING,  TUTTI                   );  /*   70 */
	AddCommand( "string",               do_string,          CMD_STRING,                 POSITION_SLEEPING,  DIO                     );
	AddCommand( "give",                 do_give,            CMD_GIVE,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "quit",                 do_quit,            CMD_QUIT,                   POSITION_DEAD,      TUTTI                   );
	AddCommand( "wiznews",              do_wiznews,         CMD_WIZNEWS,                POSITION_DEAD,      IMMORTALE               );
	AddCommand( "guard",                do_guard,           CMD_GUARD,                  POSITION_STANDING,  TUTTI                   );  /*   75 */
	AddCommand( "time",                 do_time,            CMD_TIME,                   POSITION_DEAD,      TUTTI                   );
	AddCommand( "oload",                do_oload,           CMD_OLOAD,                  POSITION_DEAD,      QUESTMASTER             );
	AddCommand( "purge",                do_purge,           CMD_PURGE,                  POSITION_DEAD,      DIO_MINORE              );
	AddCommand( "shutdown",             do_shutdown,        CMD_SHUTDOWN,               POSITION_DEAD,      CREATORE                );
	AddCommand( "idea",                 do_idea_new,        CMD_IDEA,                   POSITION_DEAD,      TUTTI                   );  /*   80 */
	AddCommand( "typo",                 do_typo,            CMD_TYPO,                   POSITION_DEAD,      TUTTI                   );
	AddCommand( "bug",                  do_bug_new,         CMD_BUG,                    POSITION_DEAD,      TUTTI                   );
	AddCommand( "whisper",              do_whisper,         CMD_WHISPER,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "cast",                 do_cast,            CMD_CAST,                   POSITION_SITTING,   TUTTI                   );
	AddCommand( "at",                   do_at,              CMD_AT,                     POSITION_DEAD,      DIO                     );  /*   85 */
	AddCommand( "ask",                  do_ask,             CMD_ASK,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "order",                do_order,           CMD_ORDER,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "sip",                  do_sip,             CMD_SIP,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "taste",                do_taste,           CMD_TASTE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "snoop",                do_snoop,           CMD_SNOOP,                  POSITION_DEAD,      QUESTMASTER             );  /*   90 */
	AddCommand( "follow",               do_follow,          CMD_FOLLOW,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "rent",                 do_not_here,        CMD_RENT,                   POSITION_STANDING,  1                       );
	AddCommand( "offer",                do_not_here,        CMD_OFFER,                  POSITION_STANDING,  1                       );
	AddCommand( "poke",                 do_action,          CMD_POKE,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "advance",              do_advance,         CMD_ADVANCE,                POSITION_DEAD,      MAESTRO_DEI_CREATORI    );  /*   95 */
	AddCommand( "accuse",               do_action,          CMD_ACCUSE,                 POSITION_SITTING,   TUTTI                   );
	AddCommand( "grin",                 do_action,          CMD_GRIN,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "bow",                  do_action,          CMD_BOW,                    POSITION_STANDING,  TUTTI                   );
	AddCommand( "open",                 do_open,            CMD_OPEN,                   POSITION_SITTING,   TUTTI                   );
	AddCommand( "close",                do_close,           CMD_CLOSE,                  POSITION_SITTING,   TUTTI                   );  /*  100 */
	AddCommand( "lock",                 do_lock,            CMD_LOCK,                   POSITION_SITTING,   TUTTI                   );
	AddCommand( "unlock",               do_unlock,          CMD_UNLOCK,                 POSITION_SITTING,   TUTTI                   );
	AddCommand( "leave",                do_leave,           CMD_LEAVE,                  POSITION_STANDING,  TUTTI                   );
	AddCommand( "applaud",              do_action,          CMD_APPLAUD,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "blush",                do_action,          CMD_BLUSH,                  POSITION_RESTING,   TUTTI                   );  /*  105 */
	AddCommand( "burp",                 do_action,          CMD_BURP,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "chuckle",              do_action,          CMD_CHUCKLE,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "nose",                 do_action,          CMD_NOSE,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "cough",                do_action,          CMD_COUGH,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "curtsey",              do_action,          CMD_CURTSEY,                POSITION_STANDING,  TUTTI                   );  /*  110 */
	AddCommand( "rise",                 do_action,          CMD_RISE,                   POSITION_RESTING,   DIO                     );
	AddCommand( "flip",                 do_action,          CMD_FLIP,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "fondle",               do_action,          CMD_FONDLE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "frown",                do_action,          CMD_FROWN,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "gasp",                 do_action,          CMD_GASP,                   POSITION_RESTING,   TUTTI                   );  /*  115 */
	AddCommand( "glare",                do_action,          CMD_GLARE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "groan",                do_action,          CMD_GROAN,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "grope",                do_action,          CMD_GROPE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "hiccup",               do_action,          CMD_HICCUP,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "lick",                 do_action,          CMD_LICK,                   POSITION_RESTING,   TUTTI                   );  /*  120 */
	AddCommand( "love",                 do_action,          CMD_LOVE,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "moan",                 do_action,          CMD_MOAN,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "nibble",               do_action,          CMD_NIBBLE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "pout",                 do_action,          CMD_POUT,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "purr",                 do_action,          CMD_PURR,                   POSITION_RESTING,   TUTTI                   );  /*  125 */
	AddCommand( "ruffle",               do_action,          CMD_RUFFLE,                 POSITION_STANDING,  TUTTI                   );
	AddCommand( "shiver",               do_action,          CMD_SHIVER,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "shrug",                do_action,          CMD_SHRUG,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "sing",                 do_action,          CMD_SING,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "slap",                 do_action,          CMD_SLAP,                   POSITION_RESTING,   TUTTI                   );  /*  130 */
	AddCommand( "smirk",                do_action,          CMD_SMIRK,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "support",              do_support,         CMD_SUPPORT,                POSITION_STANDING,  TUTTI                   );
	AddCommand( "sneeze",               do_action,          CMD_SNEEZE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "snicker",              do_action,          CMD_SNICKER,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "sniff",                do_action,          CMD_SNIFF,                  POSITION_RESTING,   TUTTI                   );  /*  135 */
	AddCommand( "snore",                do_action,          CMD_SNORE,                  POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "spit",                 do_action,          CMD_SPIT,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "squeeze",              do_action,          CMD_SQUEEZE,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "stare",                do_action,          CMD_STARE,                  POSITION_RESTING,   TUTTI                   );
    AddCommand( "associa",              do_associa,         CMD_ASSOCIA,                POSITION_STANDING,  PRINCIPE                );  /*  140 */
    AddCommand( "associate",            do_associa,         CMD_ASSOCIA,                POSITION_STANDING,  PRINCIPE                );  /*  140 */
	AddCommand( "thank",                do_action,          CMD_THANK,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "twiddle",              do_action,          CMD_TWIDDLE,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "wave",                 do_action,          CMD_WAVE,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "whistle",              do_action,          CMD_WHISTLE,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "wiggle",               do_action,          CMD_WIGGLE,                 POSITION_STANDING,  TUTTI                   );  /*  145 */
	AddCommand( "wink",                 do_action,          CMD_WINK,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "yawn",                 do_action,          CMD_YAWN,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "snowball",             do_action,          CMD_SNOWBALL,               POSITION_STANDING,  IMMORTALE               );
	AddCommand( "write",                do_write,           CMD_WRITE,                  POSITION_STANDING,  1                       );
	AddCommand( "hold",                 do_grab,            CMD_HOLD,                   POSITION_RESTING,   TUTTI                   );  /*  150 */
	AddCommand( "flee",                 do_flee,            CMD_FLEE,                   POSITION_SITTING,   1                       );
	AddCommand( "sneak",                do_sneak,           CMD_SNEAK,                  POSITION_STANDING,  1                       );
	AddCommand( "hide",                 do_hide,            CMD_HIDE,                   POSITION_RESTING,   1                       );  /*  153 */
	AddCommand( "camouflage",           do_hide,            CMD_HIDE,                   POSITION_STANDING,  1                       );  /*  153 */
	AddCommand( "backstab",             do_backstab,        CMD_BACKSTAB,               POSITION_STANDING,  1                       );
	AddCommand( "pick",                 do_pick,            CMD_PICK,                   POSITION_STANDING,  1                       );  /*  155 */
	AddCommand( "steal",                do_steal,           CMD_STEAL,                  POSITION_STANDING,  1                       );
	AddCommand( "bash",                 do_bash,            CMD_BASH,                   POSITION_FIGHTING,  1                       );
	AddCommand( "rescue",               do_rescue,          CMD_RESCUE,                 POSITION_FIGHTING,  1                       );
	AddCommand( "kick",                 do_kick,            CMD_KICK,                   POSITION_FIGHTING,  1                       );
	AddCommand( "french",               do_action,          CMD_FRENCHKISS,             POSITION_RESTING,   ALLIEVO                 );  /*  160 */
	AddCommand( "comb",                 do_action,          CMD_COMB,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "massage",              do_action,          CMD_MASSAGE,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "tickle",               do_action,          CMD_TICKLE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "practice",             do_practice,        CMD_PRACTICE,               POSITION_RESTING,   1                       );  /*  164 */
	AddCommand( "practise",             do_practice,        CMD_PRACTICE,               POSITION_RESTING,   1                       );  /*  164 */
	AddCommand( "pat",                  do_action,          CMD_PAT,                    POSITION_RESTING,   TUTTI                   );  /*  165 */
	AddCommand( "examine",              do_examine,         CMD_EXAMINE,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "take",                 do_get,             CMD_TAKE,                   POSITION_RESTING,   1                       );
	AddCommand( "info",                 do_info,            CMD_INFO,                   POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "'",                    do_new_say,         CMD_SAY_APICE,              POSITION_RESTING,   TUTTI                   );
    AddCommand( "?",                    do_commune,         CMD_THINK_SUPERNI,          POSITION_RESTING,   CREATORE                );  /*  170 */
    AddCommand( "curse",                do_action,          CMD_CURSE,                  POSITION_RESTING,   5                       );
	AddCommand( "use",                  do_use,             CMD_USE,                    POSITION_SITTING,   1                       );
	AddCommand( "where",                do_where,           CMD_WHERE,                  POSITION_DEAD,      1                       );
	AddCommand( "levels",               do_levels,          CMD_LEVELS,                 POSITION_DEAD,      TUTTI                   );
	AddCommand( "register",             do_register,        CMD_REGISTER,               POSITION_DEAD,      TUTTI                   );  /*  175 */
    AddCommand( "pray",                 do_pray,            CMD_PRAY,                   POSITION_SITTING,   1                       );
    AddCommand( ",",                    do_emote,           CMD_EMOTE_VIRGOLA,          POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "beg",                  do_action,          CMD_BEG,                    POSITION_RESTING,   TUTTI                   );
    AddCommand( "forge",                do_forge,           CMD_FORGE,                  POSITION_STANDING,  1                       );
	AddCommand( "cringe",               do_action,          CMD_CRINGE,                 POSITION_RESTING,   TUTTI                   );  /*  180 */
	AddCommand( "daydream",             do_action,          CMD_DAYDREAM,               POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "fume",                 do_action,          CMD_FUME,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "grovel",               do_action,          CMD_GROVEL,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "hop",                  do_action,          CMD_HOP,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "nudge",                do_action,          CMD_NUDGE,                  POSITION_RESTING,   TUTTI                   );  /*  185 */
	AddCommand( "peer",                 do_action,          CMD_PEER,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "point",                do_action,          CMD_POINT,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "ponder",               do_action,          CMD_PONDER,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "punch",                do_action,          CMD_PUNCH,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "snarl",                do_action,          CMD_SNARL,                  POSITION_RESTING,   TUTTI                   );  /*  190 */
	AddCommand( "spank",                do_action,          CMD_SPANK,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "steam",                do_action,          CMD_STEAM,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "tackle",               do_action,          CMD_TACKLE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "taunt",                do_action,          CMD_TAUNT,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "think",                do_commune,         CMD_THINK,                  POSITION_RESTING,   IMMORTALE               );  /*  195 */
	AddCommand( "\"",                   do_commune,         CMD_THINK,                  POSITION_RESTING,   IMMORTALE               );  /*  195 */
	AddCommand( "whine",                do_action,          CMD_WHINE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "worship",              do_action,          CMD_WORSHIP,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "yodel",                do_action,          CMD_YODEL,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "brief",                do_brief,           CMD_BRIEF,                  POSITION_DEAD,      TUTTI                   );
	AddCommand( "wizlist",              do_wizlist,         CMD_WIZLIST,                POSITION_DEAD,      TUTTI                   );  /*  200 */
	AddCommand( "consider",             do_consider,        CMD_CONSIDER,               POSITION_RESTING,   TUTTI                   );
	AddCommand( "group",                do_group,           CMD_GROUP,                  POSITION_RESTING,   1                       );
	AddCommand( "restore",              do_restore,         CMD_RESTORE,                POSITION_DEAD,      MAESTRO_DEGLI_DEI       );
	AddCommand( "return",               do_return,          CMD_RETURN,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "switch",               do_switch,          CMD_SWITCH,                 POSITION_DEAD,      CREATORE                );  /*  205 */
	AddCommand( "quaff",                do_quaff,           CMD_QUAFF,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "recite",               do_recite,          CMD_RECITE,                 POSITION_STANDING,  TUTTI                   );
	AddCommand( "users",                do_users,           CMD_USERS,                  POSITION_DEAD,      DIO_MINORE              );
	AddCommand( "pose",                 do_pose,            CMD_POSE,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "noshout",              do_noshout,         CMD_NOSHOUT,                POSITION_SLEEPING,  IMMORTALE               );  /*  210 */
	AddCommand( "wizhelp",              do_wizhelp,         CMD_WIZHELP,                POSITION_SLEEPING,  IMMORTALE               );
	AddCommand( "credits",              do_credits,         CMD_CREDITS,                POSITION_DEAD,      TUTTI                   );
	AddCommand( "compact",              do_compact,         CMD_COMPACT,                POSITION_DEAD,      TUTTI                   );
	AddCommand( "daimoku",              do_daimoku,         CMD_DAIMOKU,                POSITION_SLEEPING,  1                       );
	AddCommand( "deafen",               do_plr_noshout,     CMD_DEAFEN,                 POSITION_SLEEPING,  1                       );  /*  215 */
	AddCommand( "slay",                 do_slay,            CMD_SLAY,                   POSITION_FIGHTING,  DIO_MINORE              );
	AddCommand( "wimpy",                do_wimp,            CMD_WIMPY,                  POSITION_DEAD,      TUTTI                   );
	AddCommand( "junk",                 do_junk,            CMD_JUNK,                   POSITION_RESTING,   1                       );
	AddCommand( "deposit",              do_not_here,        CMD_DEPOSIT,                POSITION_RESTING,   1                       );
	AddCommand( "withdraw",             do_not_here,        CMD_WITHDRAW,               POSITION_RESTING,   1                       );  /*  220 */
	AddCommand( "balance",              do_not_here,        CMD_BALANCE,                POSITION_RESTING,   1                       );
	AddCommand( "nohassle",             do_nohassle,        CMD_NOHASSLE,               POSITION_DEAD,      DIO                     );
	AddCommand( "system",               do_system,          CMD_SYSTEM,                 POSITION_DEAD,      QUESTMASTER             );
	AddCommand( "pull",                 do_open_exit,       CMD_PULL,                   POSITION_STANDING,  1                       );
	AddCommand( "stealth",              do_stealth,         CMD_STEALTH,                POSITION_DEAD,      DIO                     );  /*  225 */
	AddCommand( "edit",                 do_edit,            CMD_EDIT,                   POSITION_DEAD,      MAESTRO_DEGLI_DEI       );
	AddCommand( "@",                    do_set,             CMD_CHIOCCIOLA,             POSITION_DEAD,      MAESTRO_DEI_CREATORI    );
	AddCommand( "rsave",                do_rsave,           CMD_RSAVE,                  POSITION_DEAD,      MAESTRO_DEGLI_DEI       );
	AddCommand( "rload",                do_rload,           CMD_RLOAD,                  POSITION_DEAD,      MAESTRO_DEGLI_DEI       );
	AddCommand( "track",                do_track,           CMD_TRACK,                  POSITION_DEAD,      1                       );  /*  230 */
	AddCommand( "wizlock",              do_wizlock,         CMD_WIZLOCK,                POSITION_DEAD,      MAESTRO_DEI_CREATORI    );
	AddCommand( "highfive",             do_highfive,        CMD_HIGHFIVE,               POSITION_DEAD,      TUTTI                   );
	AddCommand( "title",                do_title,           CMD_TITLE,                  POSITION_DEAD,      INIZIATO-1              );
	AddCommand( "whozone",              do_who,             CMD_WHOZONE,                POSITION_DEAD,      TUTTI                   );
	AddCommand( "assist",               do_assist,          CMD_ASSIST,                 POSITION_FIGHTING,  1                       );  /*  235 */
	AddCommand( "attribute",            do_attribute,       CMD_ATTRIBUTE,              POSITION_DEAD,      5                       );
	AddCommand( "world",                do_world,           CMD_WORLD,                  POSITION_DEAD,      TUTTI                   );
	AddCommand( "allspells",            do_spells,          CMD_ALLSPELLS,              POSITION_DEAD,      TUTTI                   );
	AddCommand( "breath",               do_breath,          CMD_BREATH,                 POSITION_FIGHTING,  1                       );
	AddCommand( "show",                 do_show,            CMD_SHOW,                   POSITION_DEAD,      DIO_MINORE              );  /*  240 */
	AddCommand( "debug",                do_debug,           CMD_DEBUG,                  POSITION_DEAD,      MAESTRO_DEI_CREATORI    );
	AddCommand( "invisible",            do_invis,           CMD_INVISIBLE,              POSITION_DEAD,      IMMORTALE               );
	AddCommand( "gain",                 do_gain,            CMD_GAIN,                   POSITION_DEAD,      1                       );
	AddCommand( "mload",                do_mload,           CMD_MLOAD,                  POSITION_DEAD,      DIO                     );
	AddCommand( "disarm",               do_disarm,          CMD_DISARM,                 POSITION_FIGHTING,  1                       );  /*  245 */
	AddCommand( "bonk",                 do_action,          CMD_BONK,                   POSITION_SITTING,   3                       );
	AddCommand( "chpwd",                do_passwd,          CMD_CHPWD,                  POSITION_SITTING,   MAESTRO_DEI_CREATORI    );  /*  247 */
	AddCommand( "passwd",               do_passwd,          CMD_CHPWD,                  POSITION_SITTING,   MAESTRO_DEI_CREATORI    );  /*  247 */
	AddCommand( "fill",                 do_not_here,        CMD_FILL,                   POSITION_SITTING,   TUTTI                   );
	AddCommand( "mantra",               do_mantra,          CMD_MANTRA,                 POSITION_SITTING,   TUTTI                   );
	AddCommand( "shoot",                do_fire,            CMD_SHOOT,                  POSITION_STANDING,  TUTTI                   );  /*  250 */
	AddCommand( "fire",                 do_fire,            CMD_SHOOT,                  POSITION_DEAD,      TUTTI                   );  /*  250 */
	AddCommand( "silence",              do_silence,         CMD_SILENCE,                POSITION_STANDING,  DIO                     );
	AddCommand( "teams",                do_not_here,        CMD_TEAMS,                  POSITION_STANDING,  MAESTRO_DEL_CREATO      );
	AddCommand( "player",               do_not_here,        CMD_PLAYER,                 POSITION_STANDING,  MAESTRO_DEL_CREATO      );
	AddCommand( "create",               do_create,          CMD_CREATE,                 POSITION_STANDING,  QUESTMASTER             );
	AddCommand( "bamfin",               do_bamfin,          CMD_BAMFIN,                 POSITION_STANDING,  IMMORTALE               );  /*  255 */
	AddCommand( "bamfout",              do_bamfout,         CMD_BAMFOUT,                POSITION_STANDING,  IMMORTALE               );
	AddCommand( "vis",                  do_invis,           CMD_VIS,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "doorbash",             do_doorbash,        CMD_DOORBASH,               POSITION_STANDING,  1                       );
	AddCommand( "mosh",                 do_action,          CMD_MOSH,                   POSITION_FIGHTING,  1                       );
	AddCommand( "alias",                do_alias,           CMD_ALIAS,                  POSITION_SLEEPING,  1                       );  /*  260 */
	AddCommand( "1",                    do_alias,           CMD_1,                      POSITION_DEAD,      1                       );
	AddCommand( "2",                    do_alias,           CMD_2,                      POSITION_DEAD,      1                       );
	AddCommand( "3",                    do_alias,           CMD_3,                      POSITION_DEAD,      1                       );
	AddCommand( "4",                    do_alias,           CMD_4,                      POSITION_DEAD,      1                       );
	AddCommand( "5",                    do_alias,           CMD_5,                      POSITION_DEAD,      1                       );  /*  265 */
	AddCommand( "6",                    do_alias,           CMD_6,                      POSITION_DEAD,      1                       );
	AddCommand( "7",                    do_alias,           CMD_7,                      POSITION_DEAD,      1                       );
	AddCommand( "8",                    do_alias,           CMD_8,                      POSITION_DEAD,      1                       );
	AddCommand( "9",                    do_alias,           CMD_9,                      POSITION_DEAD,      1                       );
	AddCommand( "0",                    do_alias,           CMD_0,                      POSITION_DEAD,      1                       );  /*  270 */
	AddCommand( "swim",                 do_swim,            CMD_SWIM,                   POSITION_STANDING,  1                       );
	AddCommand( "spy",                  do_spy,             CMD_SPY,                    POSITION_STANDING,  1                       );
	AddCommand( "springleap",           do_springleap,      CMD_SPRINGLEAP,             POSITION_RESTING,   1                       );
	AddCommand( "quivering palm",       do_quivering_palm,  CMD_QUIVERING,              POSITION_FIGHTING,  INIZIATO-1              );
	AddCommand( "feign death",          do_feign_death,     CMD_FEIGH_DEATH,            POSITION_FIGHTING,  1                       );  /*  275 */
	AddCommand( "mount",                do_mount,           CMD_MOUNT,                  POSITION_STANDING,  1                       );
	AddCommand( "dismount",             do_mount,           CMD_DISMOUNT,               POSITION_MOUNTED,   1                       );
	AddCommand( "ride",                 do_mount,           CMD_RIDE,                   POSITION_STANDING,  1                       );
	AddCommand( "sign",                 do_sign,            CMD_SIGN,                   POSITION_RESTING,   1                       );
	AddCommand( "set",                  do_set_flags,       CMD_SET,                    POSITION_DEAD,      TUTTI                   );  /*  280 */
	AddCommand( "first aid",            do_first_aid,       CMD_FIRST_AID,              POSITION_RESTING,   1                       );
	AddCommand( "log",                  do_set_log,         CMD_LOG,                    POSITION_DEAD,      MAESTRO_DEL_CREATO      );
	AddCommand( "recall",               do_cast,            CMD_RECALL,                 POSITION_SITTING,   1                       );
	AddCommand( "reload",               reboot_text,        CMD_RELOAD,                 POSITION_DEAD,      MAESTRO_DEI_CREATORI    );
	AddCommand( "event",                do_event,           CMD_EVENT,                  POSITION_DEAD,      MAESTRO_DEI_CREATORI    );  /*  285 */
	AddCommand( "disguise",             do_disguise,        CMD_DISGUISE,               POSITION_STANDING,  1                       );
	AddCommand( "climb",                do_climb,           CMD_CLIMB,                  POSITION_STANDING,  1                       );
	AddCommand( "beep",                 do_beep,            CMD_BEEP,                   POSITION_DEAD,      DIO_MINORE              );
	AddCommand( "bite",                 do_action,          CMD_BITE,                   POSITION_RESTING,   1                       );
	AddCommand( "redit",                do_redit,           CMD_REDIT,                  POSITION_SLEEPING,  MAESTRO_DEGLI_DEI       );  /*  290 */
	AddCommand( "display",              do_display,         CMD_DISPLAY,                POSITION_SLEEPING,  1                       );
	AddCommand( "resize",               do_resize,          CMD_RESIZE,                 POSITION_SLEEPING,  1                       );
	AddCommand( "ripudia",              do_ripudia,         CMD_RIPUDIA,                POSITION_STANDING,  VASSALLO                );
	AddCommand( "#",                    do_cset,            CMD_DIESIS,                 POSITION_DEAD,      QUESTMASTER             );
	AddCommand( "spell",                do_stat,            CMD_SPELL,                  POSITION_DEAD,      DIO_MINORE              );  /*  295 */
	AddCommand( "name",                 do_stat,            CMD_NAME,                   POSITION_DEAD,      DIO_MINORE              );
	AddCommand( "stat",                 do_stat,            CMD_STAT,                   POSITION_DEAD,      MAESTRO_DEGLI_DEI       );
	AddCommand( "immortali",            do_immort,          CMD_IMMORT,                 POSITION_DEAD,      TUTTI                   );
	AddCommand( "auth",                 do_auth,            CMD_AUTH,                   POSITION_SLEEPING,  MAESTRO_DEGLI_DEI       );
	AddCommand( "sacrifice",            do_not_here,        CMD_SACRIFICE,              POSITION_STANDING,  MEDIUM                  );  /*  300 */
	AddCommand( "nogossip",             do_plr_nogossip,    CMD_NOGOSSIP,               POSITION_RESTING,   TUTTI                   );
	AddCommand( "gossip",               do_gossip,          CMD_GOSSIP,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "noauction",            do_plr_noauction,   CMD_NOAUCTION,              POSITION_RESTING,   TUTTI                   );
	AddCommand( "auction",              do_auction,         CMD_AUCTION,                POSITION_RESTING,   ALLIEVO                 );
	AddCommand( "discon",               do_disconnect,      CMD_DISCON,                 POSITION_RESTING,   DIO                     );  /*  305 */
	AddCommand( "freeze",               do_freeze,          CMD_FREEZE,                 POSITION_SLEEPING,  QUESTMASTER             );
	AddCommand( "drain",                do_drainlevel,      CMD_DRAIN,                  POSITION_SLEEPING,  MAESTRO_DEI_CREATORI    );
	AddCommand( "oedit",                do_oedit,           CMD_OEDIT,                  POSITION_DEAD,      QUESTMASTER             );
	AddCommand( "report",               do_report,          CMD_REPORT,                 POSITION_RESTING,   1                       );
	AddCommand( "interven",             do_god_interven,    CMD_INTERVEN,               POSITION_DEAD,      MAESTRO_DEI_CREATORI    );  /*  310 */
	AddCommand( "gtell",                do_gtell,           CMD_GTELL,                  POSITION_SLEEPING,  1                       );
	AddCommand( "raise",                do_action,          CMD_RAISE,                  POSITION_RESTING,   1                       );
	AddCommand( "tap",                  do_action,          CMD_TAP,                    POSITION_STANDING,  1                       );
	AddCommand( "liege",                do_action,          CMD_LIEGE,                  POSITION_RESTING,   1                       );
	AddCommand( "sneer",                do_action,          CMD_SNEER,                  POSITION_RESTING,   1                       );  /*  315 */
	AddCommand( "howl",                 do_action,          CMD_HOWL,                   POSITION_RESTING,   1                       );
	AddCommand( "kneel",                do_action,          CMD_KNEEL,                  POSITION_STANDING,  1                       );
	AddCommand( "finger",               do_finger,          CMD_FINGER,                 POSITION_RESTING,   1                       );
	AddCommand( "pace",                 do_action,          CMD_PACE,                   POSITION_STANDING,  1                       );
	AddCommand( "tongue",               do_action,          CMD_TONGUE,                 POSITION_RESTING,   1                       );  /*  320 */
	AddCommand( "flex",                 do_action,          CMD_FLEX,                   POSITION_STANDING,  1                       );
	AddCommand( "ack",                  do_action,          CMD_ACK,                    POSITION_RESTING,   1                       );
	AddCommand( "ckeq",                 do_stat,            CMD_CKEQ,                   POSITION_RESTING,   MAESTRO_DEGLI_DEI       );
	AddCommand( "caress",               do_action,          CMD_CARESS,                 POSITION_RESTING,   1                       );
	AddCommand( "cheer",                do_action,          CMD_CHEER,                  POSITION_RESTING,   1                       );  /*  325 */
	AddCommand( "jump",                 do_action,          CMD_JUMP,                   POSITION_STANDING,  1                       );
	AddCommand( "join",                 do_action,          CMD_JOIN,                   POSITION_RESTING,   1                       );
	AddCommand( "split",                do_split,           CMD_SPLIT,                  POSITION_RESTING,   1                       );
	AddCommand( "berserk",              do_berserk,         CMD_BERSERK,                POSITION_FIGHTING,  1                       );
	AddCommand( "tan",                  do_tan,             CMD_TAN,                    POSITION_STANDING,  1                       );  /*  330 */
	AddCommand( "memorize",             do_memorize,        CMD_MEMORIZE,               POSITION_RESTING,   1                       );
	AddCommand( "find",                 do_find,            CMD_FIND,                   POSITION_STANDING,  1                       );
	AddCommand( "bellow",               do_bellow,          CMD_BELLOW,                 POSITION_FIGHTING,  1                       );
  //AddCommand( "store",                do_not_here,        CMD_STORE,                  POSITION_STANDING,  1                       );
	AddCommand( "carve",                do_carve,           CMD_CARVE,                  POSITION_STANDING,  1                       );  /*  335 */
	AddCommand( "nuke",                 do_nuke,            CMD_NUKE,                   POSITION_DEAD,      MAESTRO_DEI_CREATORI    );
	AddCommand( "skills",               do_show_skill,      CMD_SKILLS,                 POSITION_SLEEPING,  TUTTI                   );
	AddCommand( "doorway",              do_doorway,         CMD_DOORWAY,                POSITION_STANDING,  TUTTI                   );
	AddCommand( "portal",               do_psi_portal,      CMD_PORTAL,                 POSITION_STANDING,  TUTTI                   );
	AddCommand( "summon",               do_mindsummon,      CMD_SUMMON,                 POSITION_STANDING,  TUTTI                   );  /*  340 */
	AddCommand( "canibalize",           do_canibalize,      CMD_CANIBALIZE,             POSITION_STANDING,  TUTTI                   );  /*  341 */
	AddCommand( "cannibalize",          do_canibalize,      CMD_CANIBALIZE,             POSITION_STANDING,  TUTTI                   );  /*  341 */
	AddCommand( "flame",                do_flame_shroud,    CMD_FLAME,                  POSITION_STANDING,  TUTTI                   );
	AddCommand( "aura",                 do_aura_sight,      CMD_AURA,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "great",                do_great_sight,     CMD_GREAT,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "psionic invisibility", do_invisibililty,   CMD_PSIONIC_INVISIBILIY,    POSITION_STANDING,  TUTTI                   );  /*  345 */
	AddCommand( "blast",                do_blast,           CMD_BLAST,                  POSITION_FIGHTING,  TUTTI                   );
	AddCommand( "medit",                do_medit,           CMD_MEDIT,                  POSITION_DEAD,      QUESTMASTER             );
	AddCommand( "hypnotize",            do_hypnosis,        CMD_HYPNOTIZE,              POSITION_STANDING,  TUTTI                   );
	AddCommand( "scry",                 do_scry,            CMD_SCRY,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "adrenalize",           do_adrenalize,      CMD_ADRENALIZE,             POSITION_STANDING,  TUTTI                   );  /*  350 */
	AddCommand( "brew",                 do_brew,            CMD_BREW,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "meditate",             do_meditate,        CMD_MEDITATE,               POSITION_RESTING,   TUTTI                   );
	AddCommand( "forcerent",            do_force_rent,      CMD_FORCERENT,              POSITION_DEAD,      QUESTMASTER             );
	AddCommand( "warcry",               do_holy_warcry,     CMD_WARCRY,                 POSITION_FIGHTING,  TUTTI                   );
	AddCommand( "lay on hands",         do_lay_on_hands,    CMD_LAY_ON_HANDS,           POSITION_RESTING,   TUTTI                   );  /*  355 */
	AddCommand( "blessing",             do_blessing,        CMD_BLESSING,               POSITION_STANDING,  TUTTI                   );
	AddCommand( "heroic",               do_heroic_rescue,   CMD_HEROIC,                 POSITION_FIGHTING,  TUTTI                   );
	AddCommand( "scan",                 do_scan,            CMD_SCAN,                   POSITION_STANDING,  DIO_MINORE              );
	AddCommand( "shield",               do_psi_shield,      CMD_SHIELD,                 POSITION_STANDING,  TUTTI                   );
	AddCommand( "notell",               do_plr_notell,      CMD_NOTELL,                 POSITION_DEAD,      TUTTI                   );  /*  360 */
	AddCommand( "commands",             do_command_list,    CMD_COMMANDS,               POSITION_DEAD,      TUTTI                   );
	AddCommand( "ghost",                do_ghost,           CMD_GHOST,                  POSITION_DEAD,      MAESTRO_DEL_CREATO      );
	AddCommand( "speak",                do_speak,           CMD_SPEAK,                  POSITION_DEAD,      TUTTI                   );
	AddCommand( "setsev",               do_setsev,          CMD_SETSEV,                 POSITION_DEAD,      DIO_MINORE              );
	AddCommand( "esp",                  do_esp,             CMD_ESP,                    POSITION_STANDING,  TUTTI                   );  /*  365 */
	AddCommand( "mail",                 do_not_here,        CMD_MAIL,                   POSITION_STANDING,  ALLIEVO-1               );
	AddCommand( "check",                do_not_here,        CMD_CHECK,                  POSITION_STANDING,  TUTTI                   );
	AddCommand( "receive",              do_not_here,        CMD_RECEIVE,                POSITION_STANDING,  TUTTI                   );
	AddCommand( "telepathy",            do_telepathy,       CMD_TELEPATHY,              POSITION_RESTING,   TUTTI                   );
	AddCommand( "mind",                 do_cast,            CMD_MIND,                   POSITION_SITTING,   TUTTI                   );  /*  370 */
	AddCommand( "twist",                do_open_exit,       CMD_TWIST,                  POSITION_STANDING,  TUTTI                   );
	AddCommand( "turn",                 do_open_exit,       CMD_TURN,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "lift",                 do_open_exit,       CMD_LIFT,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "push",                 do_open_exit,       CMD_PUSH,                   POSITION_STANDING,  TUTTI                   );
  //AddCommand("zload",                 do_zload,           CMD_ZLOAD,                  POSITION_STANDING,  MAESTRO_DEGLI_DEI       );  /*  375 */
	AddCommand( "zsave",                do_zsave,           CMD_ZSAVE,                  POSITION_STANDING,  MAESTRO_DEGLI_DEI       );
	AddCommand( "zclean",               do_zclean,          CMD_ZCLEAN,                 POSITION_STANDING,  MAESTRO_DEGLI_DEI       );
	AddCommand( "wrebuild",             do_WorldSave,       CMD_WREBUILD,               POSITION_STANDING,  IMMENSO                 );
	AddCommand( "gwho",                 list_groups,        CMD_GWHO,                   POSITION_DEAD,      TUTTI                   );
	AddCommand( "mforce",               do_mforce,          CMD_MFORCE,                 POSITION_DEAD,      DIO_MINORE              );  /*  380 */
	AddCommand( "clone",                do_clone,           CMD_CLONE,                  POSITION_DEAD,      MAESTRO_DEGLI_DEI+2     );
	AddCommand( "bodyguard",            do_bodyguard,       CMD_BODYGUARD,              POSITION_STANDING,  MEDIUM                  );
	AddCommand( "throw",                do_throw,           CMD_THROW,                  POSITION_SITTING,   TUTTI                   );
	AddCommand( "run",                  do_run,             CMD_RUN,                    POSITION_STANDING,  TUTTI                   );
	AddCommand( "notch",                do_weapon_load,     CMD_NOTCH,                  POSITION_RESTING,   TUTTI                   );  /*  385 */
	AddCommand( "load",                 do_weapon_load,     CMD_NOTCH,                  POSITION_RESTING,   TUTTI                   );  /*  385 */
    AddCommand( "know",                 do_cast,            CMD_SPELLID,                POSITION_SITTING,   TUTTI                   );
	AddCommand( "spot",                 do_scan,            CMD_SPOT,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "view",                 do_viewfile,        CMD_VIEW,                   POSITION_DEAD,      DIO                     );
	AddCommand( "afk",                  do_set_afk,         CMD_AFK,                    POSITION_DEAD,      1                       );
	AddCommand( "stopfight",            do_stopfight,       CMD_STOPFIGHT,              POSITION_FIGHTING,  1                       );  /*  390 */
	AddCommand( "principi",             do_prince,          CMD_PRINCE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "tspy",                 do_tspy,            CMD_TSPY,                   POSITION_STANDING,  1                       );
	AddCommand( "bid",                  do_auction,         CMD_BID,                    POSITION_RESTING,   ALLIEVO                 );
	AddCommand( "eavesdrop",            do_eavesdrop,       CMD_EAVESDROP,              POSITION_STANDING,  INIZIATO                );
	AddCommand( "pquest",               do_pquest,          CMD_PQUEST,                 POSITION_STANDING,  QUESTMASTER             );  /*  395 */
    AddCommand( "setalign",             do_setalign,        CMD_SETALIGN,               POSITION_RESTING,   TUTTI                   );
    AddCommand( "wreset",               do_wreset,          CMD_WRESET,                 POSITION_STANDING,  CREATORE                );
	AddCommand( "parry",                do_parry,           CMD_PARRY,                  POSITION_RESTING,   TUTTI                   );
    AddCommand( "checktypos",           do_checktypos,      CMD_CHECKTYPOS,             POSITION_DEAD,      DIO                     );
	AddCommand( "adore",                do_action,          CMD_ADORE,                  POSITION_RESTING,   TUTTI                   );  /*  400 */
	AddCommand( "agree",                do_action,          CMD_AGREE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "bleed",                do_action,          CMD_BLEED,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "blink",                do_action,          CMD_BLINK,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "blow",                 do_action,          CMD_BLOW,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "blame",                do_action,          CMD_BLAME,                  POSITION_RESTING,   TUTTI                   );  /*  405 */
	AddCommand( "bark",                 do_action,          CMD_BARK,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "bhug",                 do_action,          CMD_BHUG,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "bcheck",               do_action,          CMD_BCHECK,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "boast",                do_action,          CMD_BOAST,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "chide",                do_action,          CMD_CHIDE,                  POSITION_RESTING,   TUTTI                   );  /*  410 */
	AddCommand( "compliment",           do_action,          CMD_COMPLIMENT,             POSITION_RESTING,   TUTTI                   );
	AddCommand( "ceyes",                do_action,          CMD_CEYES,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "cears",                do_action,          CMD_CEARS,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "cross",                do_action,          CMD_CROSS,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "console",              do_action,          CMD_CONSOLE,                POSITION_RESTING,   TUTTI                   );  /*  415 */
	AddCommand( "calm",                 do_action,          CMD_CALM,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "cower",                do_action,          CMD_COWER,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "confess",              do_action,          CMD_CONFESS,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "drool",                do_action,          CMD_DROOL,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "grit",                 do_action,          CMD_GRIT,                   POSITION_RESTING,   TUTTI                   );  /*  420 */
	AddCommand( "greet",                do_action,          CMD_GREET,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "gulp",                 do_action,          CMD_GULP,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "gloat",                do_action,          CMD_GLOAT,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "gaze",                 do_action,          CMD_GAZE,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "hum",                  do_action,          CMD_HUM,                    POSITION_RESTING,   TUTTI                   );  /*  425 */
	AddCommand( "hkiss",                do_action,          CMD_HKISS,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "ignore",               do_action,          CMD_IGNORE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "interrupt",            do_action,          CMD_INTERRUPT,              POSITION_RESTING,   TUTTI                   );
	AddCommand( "knock",                do_action,          CMD_KNOCK,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "listen",               do_action,          CMD_LISTEN,                 POSITION_RESTING,   TUTTI                   );  /*  430 */
	AddCommand( "muse",                 do_action,          CMD_MUSE,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "pinch",                do_action,          CMD_PINCH,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "praise",               do_action,          CMD_PRAISE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "plot",                 do_action,          CMD_PLOT,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "pie",                  do_action,          CMD_PIE,                    POSITION_RESTING,   TUTTI                   );  /*  435 */
	AddCommand("pleade",                do_action,          CMD_PLEADE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "pant",                 do_action,          CMD_PANT,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "rub",                  do_action,          CMD_RUB,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "roll",                 do_action,          CMD_ROLL,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "recoil",               do_action,          CMD_RECOIL,                 POSITION_RESTING,   TUTTI                   );  /*  440 */
	AddCommand( "roar",                 do_action,          CMD_ROAR,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "relax",                do_action,          CMD_RELAX,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "snap",                 do_action,          CMD_SNAP,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "strut",                do_action,          CMD_STRUT,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "stroke",               do_action,          CMD_STROKE,                 POSITION_RESTING,   TUTTI                   );  /*  445 */
	AddCommand( "stretch",              do_action,          CMD_STRETCH,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "swave",                do_action,          CMD_SWAVE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "sob",                  do_action,          CMD_SOB,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "scratch",              do_action,          CMD_SCRATCH,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "squirm",               do_action,          CMD_SQUIRM,                 POSITION_RESTING,   TUTTI                   );  /*  450 */
	AddCommand( "strangle",             do_action,          CMD_STRANGLE,               POSITION_RESTING,   TUTTI                   );
	AddCommand( "scowl",                do_action,          CMD_SCOWL,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "shudder",              do_action,          CMD_SHUDDER,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "strip",                do_action,          CMD_STRIP,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "scoff",                do_action,          CMD_SCOFF,                  POSITION_RESTING,   TUTTI                   );  /*  455 */
	AddCommand( "salute",               do_action,          CMD_SALUTE,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "scold",                do_action,          CMD_SCOLD,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "stagger",              do_action,          CMD_STAGGER,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "toss",                 do_action,          CMD_TOSS,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "twirl",                do_action,          CMD_TWIRL,                  POSITION_RESTING,   TUTTI                   );  /*  460 */
	AddCommand( "toast",                do_action,          CMD_TOAST,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "tug",                  do_action,          CMD_TUG,                    POSITION_RESTING,   TUTTI                   );
	AddCommand( "touch",                do_action,          CMD_TOUCH,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "tremble",              do_action,          CMD_TREMBLE,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "twitch",               do_action,          CMD_TWITCH,                 POSITION_RESTING,   TUTTI                   );  /*  465 */
	AddCommand( "whimper",              do_action,          CMD_WHIMPER,                POSITION_RESTING,   TUTTI                   );
	AddCommand( "whap",                 do_action,          CMD_WHAP,                   POSITION_RESTING,   TUTTI                   );
	AddCommand( "wedge",                do_action,          CMD_WEDGE,                  POSITION_RESTING,   TUTTI                   );
	AddCommand( "apologize",            do_action,          CMD_APOLOGIZE,              POSITION_RESTING,   TUTTI                   );
    AddCommand( "destroy",              do_destroy,         CMD_DESTROY,                POSITION_RESTING,   TUTTI                   );  /*  470 */
    AddCommand( "personalize",          do_personalize,     CMD_PERSONALIZE,            POSITION_DEAD,      MAESTRO_DEGLI_DEI       );
	/*          Liberi 472-479 (-488)*/
  //            DIMD stuff
  //AddCommand( "dmanage",              do_dmanage,         CMD_DMANAGE,                POSITION_RESTING,   MAESTRO_DEL_CREATO      );  /*  480 */
  //AddCommand( "drestrict",            do_drestrict,       CMD_DRESTRICT,              POSITION_RESTING,   DIO_MINORE              );
  //AddCommand( "dlink",                do_dgossip,         CMD_DLINK,                  POSITION_RESTING,   DIO_MINORE              );
  //AddCommand( "dunlink",              do_dgossip,         CMD_DUNLINK,                POSITION_RESTING,   DIO_MINORE              );
  //AddCommand( "dlist",                do_dlist,           CMD_DLIST,                  POSITION_RESTING,   2                       );
  //AddCommand( "dwho",                 do_dwho,            CMD_DWHO,                   POSITION_RESTING,   2                       );  /*  485 */
  //AddCommand( "dgossip",              do_dgossip,         CMD_DGOSSIP,                POSITION_RESTING,   MAESTRO_DEL_CREATO      );
  //AddCommand( "dtell",                do_dtell,           CMD_DTELL,                  POSITION_RESTING,   MAESTRO_DEL_CREATO      );
  //AddCommand( "dthink",               do_dthink,          CMD_DTHINK,                 POSITION_RESTING,   MAESTRO_DEL_CREATO      );
	AddCommand( "sending",              do_sending,         CMD_SEND,                   POSITION_STANDING,  TUTTI                   );
	AddCommand( "messenger",            do_sending,         CMD_MESSENGER,              POSITION_STANDING,  TUTTI                   );  /*  490 */
	AddCommand( "prompt",               do_set_prompt,      CMD_PROMPT,                 POSITION_RESTING,   TUTTI                   );
	AddCommand( "ooedit",               do_ooedit,          CMD_OOEDIT,                 POSITION_DEAD,      MAESTRO_DEGLI_DEI       );
	AddCommand( "whois",                do_whois,           CMD_WHOIS,                  POSITION_DEAD,      ALLIEVO-1               );
	AddCommand( "osave",                do_osave,           CMD_OSAVE,                  POSITION_DEAD,      QUESTMASTER             );
	AddCommand( "dig",                  do_open_exit,       CMD_DIG,                    POSITION_STANDING,  TUTTI                   );  /*  495 */
	AddCommand( "cut",                  do_open_exit,       CMD_SCYTHE,                 POSITION_STANDING,  TUTTI                   );
	AddCommand( "status",               do_status,          CMD_STATUS,                 POSITION_DEAD,      TUTTI                   );
	AddCommand( "showsk",               do_showskills,      CMD_SHOWSKILLS,             POSITION_DEAD,      DIO                     );
	AddCommand( "resetsk",              do_resetskills,     CMD_RESETSKILLS,            POSITION_DEAD,      MAESTRO_DEL_CREATO      );
	AddCommand( "setsk",                do_setskill,        CMD_SETSKILLS,              POSITION_DEAD,      MAESTRO_DEI_CREATORI    );  /*  500 */
  //AddCommand("perdono",               do_perdono,         CMD_PERDONO,                POSITION_STANDING,  TUTTI                   );
	AddCommand( "immolate",             do_immolation,      CMD_IMMOLATION,             POSITION_FIGHTING,  TUTTI                   );
	AddCommand( "SetTest",              do_imptest,         CMD_IMPTEST,                POSITION_DEAD,      MAESTRO_DEL_CREATO      );
}


/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */




/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char* name) {
	FILE* fl;
	char szFileName[ 41 ];

	sprintf(szFileName, "%s/%s.dat", PLAYERS_DIR, lower(name));
	if((fl = fopen(szFileName, "r")) != NULL) {
		fclose(fl);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/**
 * @return unsigned long long
 * 0 = standard login with pg name (name contains the standardized version)
 * 1 = invalid data
 * 2 = email
 * 3 = numeric id +2 (to cope with id lesser than 2)
 */
unsigned long long parse_name(const char* arg, char* name) {
	try {
		return 2+boost::lexical_cast<unsigned long long>(arg);
	}
	catch (...) {
	}
	string s(arg);
	string space(" ");
	boost::algorithm::erase_all(s,space);
	boost::algorithm::to_lower(s);

	strncpy(name,s.c_str(),99);
	if (s.find('@')!= string::npos) {
		return 2;
	}
	if (s.empty() or s.length() > 15 ) {
		return 1;
	}
	if (std::count_if(s.begin(), s.end(), [](char c){ return !std::isalpha(c); }) >0) {
		return 1;
	}
	return 0;
}

#define ASSHOLE_FNAME "asshole.list"

int _check_ass_name(char* name) {
	/* 0 - full match
	 *   1 - from start of string
	 *   2 - from end of string
	 *   3 - somewhere in string
	 */
	static struct shitlist {
		int how;
		char name[80];
	} *shitlist = NULL;
	FILE* f;
	char buf[512];
	int i,j,k;
	if(strlen(name)<4) {
		return(1);
	}

	if(!shitlist) {
		if((f=fopen(ASSHOLE_FNAME,"rt"))==NULL) {
			mudlog(LOG_ERROR, "can't open asshole names list");
			shitlist=(struct shitlist*)calloc(1,sizeof(struct shitlist));
			*shitlist[0].name=0;
			return 0;
		}
		for(i=0; fgets(buf,180,f)!=NULL; i++) ;
		shitlist=(struct shitlist*)calloc((i+3), sizeof(struct shitlist));
		rewind(f);
		for(i=0; fgets(buf,180,f)!=NULL; i++) {
			if(buf[strlen(buf)-1]=='\n' || buf[strlen(buf)-1]=='\r') {
				buf[strlen(buf)-1]=0;
			}
			if(buf[strlen(buf)-1]=='\n' || buf[strlen(buf)-1]=='\r') {
				buf[strlen(buf)-1]=0;
			}
			if(*buf=='*') {
				if(buf[strlen(buf)-1]=='*') {
					shitlist[i].how=3;
					buf[strlen(buf)-1]=0;
					strcpy(shitlist[i].name,buf+1);
				}
				else {
					shitlist[i].how=2;
					strcpy(shitlist[i].name,buf+1);
				}
			}
			else {
				if(buf[strlen(buf)-1]=='*') {
					shitlist[i].how=1;
					buf[strlen(buf)-1]=0;
					strcpy(shitlist[i].name,buf);
				}
				else {
					shitlist[i].how=0;
					strcpy(shitlist[i].name,buf);
				}
			}
		}
		*shitlist[i].name = 0;
		for(i=0; *shitlist[i].name; i++) {
			sprintf(buf, "mode: %d, name: %s", shitlist[i].how, shitlist[i].name);
			/*      log(buf);*/
		}
	}
#define NAME shitlist[j].name
	for(j=0; *NAME; j++)
		switch(shitlist[j].how) {
		case 0:
			if(!str_cmp(name,NAME)) {
				return 1;
			}
			break;
		case 1:
			if(!strn_cmp(name,NAME,strlen(NAME))) {
				return 1;
			}
			break;
		case 2:
			if(strlen(name)<strlen(NAME)) {
				break;
			}
			if(!str_cmp(name+(strlen(name)-strlen(NAME)), NAME)) {
				return 1;
			}
			break;
		case 3:
			if(strlen(name)<strlen(NAME)) {
				break;
			}
			for(k=0; k<=(int)strlen(name)-(int)strlen(NAME); k++)
				if(!strn_cmp(name+k, NAME, strlen(NAME))) {
					return 1;
				}
			break;
		default:
			mudlog(LOG_SYSERR,
				   "Invalid value in shitlist, interpereter.c _parse_name");
			return 1;
		}
#undef NAME
	return(0);
}

void ShowStatInstruction(struct descriptor_data* d) {
	char buf[ 100 ];

	sprintf(buf, "Seleziona le priorita` per le caratteristiche di %s, elencandole\n\r",
			GET_NAME(d->character));
	SEND_TO_Q(buf, d);
	SEND_TO_Q("dalla piu` alta a quella piu` bassa, separate da spazi senza duplicarle.\n\r", d);
	SEND_TO_Q("Per esempio: 'F I S A CO CA' dara` il punteggio piu` alto alla Forza,\n\r"
			  "seguite, nell'ordine, da Intelligenza, Saggezza, Agilita`, COstituzione e, per\n\r"
			  "ultimo, CArisma\n\r\n\r", d);
	SEND_TO_Q("   Considera che le abilita' influenzano (anche) questo: \n\r",d);
	SEND_TO_Q("F = capacita' di usare oggetti pesanti, bonus nel combattere\n\r",d);
	SEND_TO_Q("I = velocita' nell'apprendere e capacita' di usare spells\n\r",d);
	SEND_TO_Q("S = velocita' di recupero mana e migliori tiri salvezza\n\r",d);
	SEND_TO_Q("A = capacita' di schivare attacchi e (guerrieri) di caricare(bash)\n\r",d);
	SEND_TO_Q("CO= quantita' di punti ferita e velocita' nel recuperarli\n\r",d);
	SEND_TO_Q("CA= quantita' di seguaci, reazioni dei mostri e dei mercanti\n\r",d);
	SEND_TO_Q("Se non sai che pesci pigliare, puoi semplicemente premere [INVIO]\n\r",d);
	SEND_TO_Q("   Provvedera' il sistema ad assegnarti delle caratteristiche\n\r",d);
	SEND_TO_Q("   compatibili con la classe che sceglierai\n\r",d);
	SEND_TO_Q("Se invece ti senti un esperto, digita <nuovo>\n\r",d);
	SEND_TO_Q("   Potrai indicare dettagliatamente le tue caratteristiche\n\r",d);

	SEND_TO_Q("\n\rLa tua scelta ? (premi <b> per tornare indietro):\n\r",d);
}
void ShowRollInstruction(struct descriptor_data* d) {
	char buf[ 200 ];
	char temp[200];

	sprintf(buf, "Hai scelto la creazione del personaggio per esperti.\n\r");
	SEND_TO_Q(buf, d);
	sprintf(buf, "Ad ogni caratteristica viene assegnato il valore minimo  %d .\n\r",
			STAT_MIN_VAL);
	SEND_TO_Q(buf, d);
	sprintf(buf, "Hai a disposizione ulteriori  %d  punti da distribuire a piacere.\n\r",
			STAT_MAX_SUM);
	SEND_TO_Q(buf,d);
	SEND_TO_Q("Per assegnarli, inserisci i numeri in questo ordine:\n\r",d);
	SEND_TO_Q("FOrza INtelligenza SAggezza AGilita' COstituzione CArisma.\n\r",d);
	SEND_TO_Q("Puoi anche chiedere che i valori da te immessi siano \n\r",d);
	SEND_TO_Q("randomizzati aggiungendo un valore da -1 a +1.\n\r",d);
	SEND_TO_Q("Per chiedere la randomizzazione basta digitare qualsiasi cosa dopo i numeri.\n\r",d);
	SEND_TO_Q("Esempio, per avere un pg con:\n\r",d);
	sprintf(buf,"F=%d I=%d S=%d A=%d Co=%d Ca=%d senza randomizzazione:\n\r",
			STAT_MIN_VAL+18-STAT_MIN_VAL,
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*1)),
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*2)),
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*3)),
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*4)),
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*5)));
	sprintf(temp,"%sFO IN SA AG CO CA RN\n\r",buf);
	sprintf(buf,"%s%2d %2d %2d %2d %2d %2d\n\r\n\r",temp,
			18-STAT_MIN_VAL,
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*1)),
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*2)),
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*3)),
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*4)),
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*5)));
	SEND_TO_Q(buf,d);
	sprintf(buf,"F=%d I=%d S=%d A=%d Co=%d Ca=%d con randomizzazione:\n\r",
			STAT_MIN_VAL+18-STAT_MIN_VAL,
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*1)),
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*2)),
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*3)),
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*4)),
			STAT_MIN_VAL+MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*5)));
	sprintf(temp,"%sFO IN SA AG CO CA RN\n\r",buf);
	sprintf(buf,"%s%2d %2d %2d %2d %2d %2d %2s\n\r\n\r",temp,
			18-STAT_MIN_VAL,
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*1)),
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*2)),
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*3)),
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*4)),
			MAX(0,MIN(18-STAT_MIN_VAL,STAT_MAX_SUM-(18-STAT_MIN_VAL)*5)),
			"r");
	SEND_TO_Q(buf,d);


	SEND_TO_Q("Bene, premi invio per rollare. Auguri!.\n\r",d);
	SEND_TO_Q("\r\n[Batti INVIO] ", d);
}

void RollPrompt(struct descriptor_data* d) {
	char buf[254];
	SEND_TO_Q("FO IN SA AG CO CA RN (puoi usare <b> per rinunciare)\n\r",d);
	/* STAT LIMITS */
	sprintf(buf,"%2d %2d %2d %2d %2d %2d    (valori limite per la tua razza)\n\r",
			MaxStrForRace(d->character)-STAT_MIN_VAL,
			MaxIntForRace(d->character)-STAT_MIN_VAL,
			MaxWisForRace(d->character)-STAT_MIN_VAL,
			MaxDexForRace(d->character)-STAT_MIN_VAL,
			MaxConForRace(d->character)-STAT_MIN_VAL,
			MaxChrForRace(d->character)-STAT_MIN_VAL);
	SEND_TO_Q(buf,d);
}

void AskRollConfirm(struct descriptor_data* d) {
	SEND_TO_Q("Sei soddisfatto(S/N)? (puoi usare <b> per rinunciare)\n\r",d);
}

void InterpretaRoll(struct descriptor_data* d, char* riga)
#define BACKWARD 1
#define AGAIN    2
#define GOON     0
{
	char buf[ 512 ];
	short doafter=GOON;
	int FO,IN,SA,AG,CO,CA,t;
	char c7[2]="\0";
	if(strlen(riga)==1 && (*riga == 'B' || *riga == 'b')) {
		doafter=BACKWARD;
	}
	if(strlen(riga)>18-17) {
		sscanf(riga,"%2d %2d %2d %2d %2d %2d %s",&FO,&IN,&SA,&AG,&CO,&CA,c7);
	}
	else {
		FO=IN=SA=AG=CO=CA=0;
	}
	if(FO < 0) {
		FO=0;
	}
	if(IN < 0) {
		IN=0;
	}
	if(SA < 0) {
		SA=0;
	}
	if(AG < 0) {
		AG=0;
	}
	if(CO < 0) {
		CO=0;
	}
	if(CA < 0) {
		CA=0;
	}
	t=FO+IN+SA+AG+CO+CA;
	if(t < 0) {
		t=STAT_MAX_SUM+1;
	}
	if(t>STAT_MAX_SUM) {
		sprintf(buf,"Hai usato piu' dei %2d punti disponibili (%2d)\n\r",
				STAT_MAX_SUM,t);
		SEND_TO_Q(buf,d);
		doafter=AGAIN;
	}
	else {
		FO=MIN(MaxStrForRace(d->character)-STAT_MIN_VAL,FO);
		IN=MIN(MaxIntForRace(d->character)-STAT_MIN_VAL,IN);
		SA=MIN(MaxWisForRace(d->character)-STAT_MIN_VAL,SA);
		AG=MIN(MaxDexForRace(d->character)-STAT_MIN_VAL,AG);
		CO=MIN(MaxConForRace(d->character)-STAT_MIN_VAL,CO);
		CA=MIN(MaxChrForRace(d->character)-STAT_MIN_VAL,CA);
		SEND_TO_Q("Ecco le stats risultanti dalla tua scelta:\n\r",d);
		sprintf(buf,"%2d %2d %2d %2d %2d %2d %s\n\r",
				FO+STAT_MIN_VAL,
				IN+STAT_MIN_VAL,
				SA+STAT_MIN_VAL,
				AG+STAT_MIN_VAL,
				CO+STAT_MIN_VAL,
				CA+STAT_MIN_VAL,(!*c7?"\0":"piu' la randomizzazione (-1/+1)"));
		SEND_TO_Q(buf,d);

		if(t<STAT_MAX_SUM) {
			sprintf(buf,"ATTENZIONE. Hai usato solo %2d dei %2d disponibili\n\r",t,STAT_MAX_SUM);
			SEND_TO_Q(buf,d);
		}
	}
	d->stat[0]=(char)FO;
	d->stat[1]=(char)IN;
	d->stat[2]=(char)SA;
	d->stat[3]=(char)AG;
	d->stat[4]=(char)CO;
	d->stat[5]=(char)CA;
	d->TipoRoll=(*c7?'N':'R');
	mudlog(LOG_PLAYERS,"%s ha rollato: S%2d I%2d W%2d D%2d Co%2d Ch%2d %s",
		   GET_NAME(d->character),FO,IN,SA,AG,CO,CA,c7);
	switch(doafter) {
	case BACKWARD:
		ShowStatInstruction(d);
		STATE(d)=CON_STAT_LIST;
		break;
	case AGAIN:
		RollPrompt(d);
		break;
	case GOON:
		AskRollConfirm(d);
		STATE(d)=CON_CONF_ROLL;
	}
	return;
}
void slackNotify(const char* message, const char* emoj) {
	    slack::Slacking slack("xxx-xxx"); // where "xxx-xxx" is your Slack API token
		//slack.set_proxy("http://10.0.22.1:8080");
		slack.hook.Id = "/T9EQH2QB0/B9E96TMPV/rJPnMU8yqgv8NzbKWh8Twfgd";
	    slack.hook.channel_username_iconemoji("", "", emoj);
	    slack.hook.postMessage(message);
}
void toonList(struct descriptor_data* d,const string &optional_message="") {
	string message(optional_message);
	message.append("Scegli un personagggio\r\n").append(" q. Quit\n\r 0. Crea un nuovo pg o usane uno non ancora connesso all'account\r\n");
	if (d->AccountData.id) { short n=0;
		constexpr int nlen=5;
		char order[nlen]="";
		toonRows r=Sql::getAll<toon>(toonQuery::owner_id==d->AccountData.id);
		d->toons.clear();
		for(toonPtr pg : r) {
			++n;
			snprintf(order,nlen-1,"%2d",n);
			message.append(order).append(". ").append(pg->name).append(" ");
			message.append(ParseAnsiColors(true,pg->title.c_str())).append("\r\n");
			d->toons.emplace_back(pg->name);
		}
	}
	message.append(">");
	SEND_TO_Q(message.c_str(),d);
}
bool toonFromFileSystem(const char* nome) {
	using namespace boost::filesystem;
	path file(current_path());
	file/=PLAYERS_DIR; // Overloaded operator: concats adding path separator
	file/=lower(nome);
	file+=".dat";
	mudlog(LOG_CONNECT,"Checking %s",file.string());
	if(is_regular_file(file) and file.extension()==".dat") {
		mudlog(LOG_CONNECT,"Opening %s",file.string());
		FILE* pFile;
		struct char_file_u Player;
		if(!(pFile = fopen(file.c_str(), "r"))) {
			mudlog(LOG_CONNECT,"Could not open %s, return false",file.string());
			return false;
		}
		if(fread(&Player, 1, sizeof(Player), pFile)
				== sizeof(Player)) {
			fclose(pFile);
			if(strcasecmp(file.stem().c_str(),Player.name)) {
				mudlog(LOG_SYSERR,"Strangeness: %s contains wrong name %s",file.filename().c_str(),Player.name);
			}
			else {
				getFromDb(Player.name,Player.pwd,Player.title);
			}
			return true; //Returning true with no pg in the db causes an abort later
		}
		fclose(pFile);
	}
	return false;

}
bool check_impl_security(struct descriptor_data* d) {
	if(top_of_p_table > 0) {
		if(GetMaxLevel(d->character) >= 59) {
			switch(SecCheck(GET_NAME(d->character), d->host)) {
			case -1:
				SEND_TO_Q2("Security file not found\n\r", d);
				return true;
				break;
			case 0:
				SEND_TO_Q2("Security check reveals invalid site\n\r", d);
				SEND_TO_Q2("Speak to an implementor to fix problem\n\r", d);
				SEND_TO_Q2("If you are an implementor, add yourself to the\n\r",d);
				SEND_TO_Q2("Security directory (lib/security)\n\r",d);
				close_socket(d);
				return false;
			default:
				return true;
			}
		}

	}
	return true;
}
char firstChar(const string &riga,bool tolower=false) {
	char arg[2];
	strcpy(arg,riga.substr(0,1).c_str());
	arg[0]=::tolower(arg[0]);
	return arg[0];
}
unsigned char echo_on[]  = {IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0'};
unsigned char echo_off[] = {IAC, WILL, TELOPT_ECHO, '\0'};

void echoOn(struct descriptor_data* d) {
	write(d->descriptor, echo_on, 6);
}
void echoOff(struct descriptor_data* d) {
	write(d->descriptor, echo_off, 4);
}

#define oldarg(noempty) 	\
	char arg[MAX_INPUT_LENGTH]; \
	std::strcpy(arg,d->currentInput.substr(0,MAX_INPUT_LENGTH-1).c_str()); \
	if (noempty and !*arg) {close_socket(d); return false;}
/**
 * Attempts to login to a global account
 *
 */

NANNY_FUNC(con_account_name) {
	oldarg(false);
	SEND_TO_Q("Benvenuto, digita la tua password per favore (o b per ricominciare): ",d);
	echoOff(d);
	STATE(d)=CON_ACCOUNT_PWD;
	return false;
}
NANNY_FUNC(con_account_pwd) {
	echoOn(d);
	oldarg(false);
	if(!strcmp(arg,"b")) {
		STATE(d)=CON_NME;
		SEND_TO_Q("Ricomiciamo. Come ti chiami?\r\n",d);
		return false;
	}
	mudlog(LOG_CONNECT,"Id: %d ,email: %s toon: %s",d->AccountData.id,d->AccountData.email.c_str(),d->AccountData.choosen.c_str());
	d->AccountData.authorized=false;
	userQuery query=d->AccountData.id>0?(userQuery::id == d->AccountData.id):(userQuery::email==d->AccountData.email);
	userPtr u=Sql::getOne<user>(query);
	if (u) {
		d->AccountData.id=u->id;
		if (u->nickname.empty()) {
			d->AccountData.nickname=u->email;
		}
		else {
			d->AccountData.nickname=u->email;
		}
		d->AccountData.registered=u->registered;
		d->AccountData.password.assign(u->password);
		d->AccountData.level=u->level;
		d->AccountData.backup_email=u->backup_email;
		d->AccountData.ptr=u->ptr;
		d->AccountData.email=u->email;
		mudlog(LOG_CONNECT,"Id: %d ,email: %s toon: %s",d->AccountData.id,d->AccountData.email.c_str(),d->AccountData.choosen.c_str());
	}
	const char* check=d->AccountData.password.c_str();
	if(u and !strcmp(crypt(arg,check),check)) {

		if (PORT==DEVEL_PORT and d->AccountData.level<52) {
			mudlog(LOG_CONNECT,"%s level %d attempted to access devel",d->AccountData.email,d->AccountData.level);
			FLUSH_TO_Q("Al server di sviluppo possono accedere solo gli immortali",d);
			close_socket(d);
			return false;
		}
		/*
        if(PORT==MASTER_PORT and d->AccountData.level<52 and !d->AccountData.ptr) {
			mudlog(LOG_CONNECT,"%s level %d ptr %s attempted to access master",d->AccountData.email,d->AccountData.level,(d->AccountData.ptr?"ON":"OFF"));
			FLUSH_TO_Q("Per accedere al server di test devi chiedere l'autorizzazione",d);
			close_socket(d);
			return false;
		}
         */
		d->AccountData.authorized=true;
		string message("Benvenuto ");
		message.append(d->AccountData.nickname).append("\r\n");
		STATE(d)=CON_ACCOUNT_TOON;
		mudlog(LOG_CONNECT,"Succesfull connection for %s",d->AccountData.email.c_str());
		toonList(d,message);
	}
	else {
		SEND_TO_Q("Riprova (digita <b> per rinunciare).",d);
		echoOff(d);
	}
	return false;
}
NANNY_FUNC(con_account_toon) {
	try {
		if (d->currentInput=="q") {
			FLUSH_TO_Q("Bye bye",d);
			close_socket(d);
			return false;
		}
		short toonIndex=tonumber(d->currentInput,-1);
		if(toonIndex <0) {
			throw std::range_error("Invalid number");
		}
		else if(toonIndex==0) {
			SEND_TO_Q("Quale personaggio vuoi usare? (Verra` automaticamente associato alla tua email) ",d);
			STATE(d)=CON_NME;
			return false;
		}
		else {
			string name(d->toons.at(toonIndex-1));
			mudlog(LOG_CONNECT,"Choosen %s",name.c_str());
			d->AccountData.choosen=name;
			d->currentInput=name;
			STATE(d)=CON_PWDOK;
			return true;
		}
	}
	catch(std::range_error &e) {
		string message(d->currentInput);
		message.append(" non e` un numero valido\r\n");
		toonList(d,message);
		return false;
	}
}
NANNY_FUNC(con_nop) {
	mudlog(LOG_SYSERR,"Called nop in nanny for : %d ",STATE(d));
	return false;
}

NANNY_FUNC(con_qclass) {
	oldarg(true);
	int ii=0;
	/* skip whitespaces */

	d->character->player.iClass = 0;

	switch(firstChar(d->currentInput)) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': {
		switch(GET_RACE(d->character)) {
		case RACE_ELVEN:
		case RACE_GOLD_ELF:
		case RACE_SEA_ELF:
		case RACE_HALF_ELVEN: {
			ii=0;
			while(d->character->player.iClass==0 && elf_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=elf_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_WILD_ELF: {
			ii=0;
			while(d->character->player.iClass==0 &&
					wild_elf_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass = wild_elf_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_HUMAN: {
			ii=0;
			while(d->character->player.iClass==0 && human_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=human_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_HALFLING: {
			ii=0;
			while(d->character->player.iClass==0 && halfling_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=halfling_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_DEEP_GNOME:
			ii=0;
			while(d->character->player.iClass == 0 &&
					deep_gnome_class_choice[ii] != 0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass = deep_gnome_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d, GET_RACE(d->character));
			}
			break;

		case RACE_GNOME: {
			ii=0;
			while(d->character->player.iClass==0 && gnome_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=gnome_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_DWARF: {
			ii=0;
			while(d->character->player.iClass==0 && dwarf_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=dwarf_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_HALF_OGRE: {
			ii=0;
			while(d->character->player.iClass==0 && half_ogre_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=half_ogre_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_HALF_GIANT: {
			ii=0;
			while(d->character->player.iClass==0 && half_giant_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=half_giant_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_HALF_ORC: {
			ii=0;
			while(d->character->player.iClass==0 && half_orc_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=half_orc_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_ORC: {
			ii=0;
			while(d->character->player.iClass==0 && orc_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=orc_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_GOBLIN: {
			ii=0;
			while(d->character->player.iClass==0 && goblin_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=goblin_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_DARK_ELF: {
			ii=0;
			while(d->character->player.iClass==0 && dark_elf_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=dark_elf_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_DARK_DWARF: {
			ii=0;
			while(d->character->player.iClass==0 && dark_dwarf_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=dark_dwarf_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_DEMON: {
			ii=0;
			while(d->character->player.iClass==0 && demon_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=demon_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		case RACE_TROLL: {
			ii=0;
			while(d->character->player.iClass==0 && troll_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=troll_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}

		default: {
			ii=0;
			while(d->character->player.iClass==0 && default_class_choice[ii] !=0) {
				if(atoi(arg) == ii) {
					d->character->player.iClass=default_class_choice[ii];
				}
				ii++;
			} /* end while */
			if(d->character->player.iClass != 0) {
				STATE(d) = CON_RNEWD;
			}
			else {
				show_class_selection(d,GET_RACE(d->character));
			}
			break;
		}
		} /* end race switch */
		break;
	}

	case '?' : {
		page_string(d, CLASS_HELP, 1);
		STATE(d) = CON_ENDHELPCLASS;
		return false;
	}
	case 'b' :
	case 'B' : {
		ShowStatInstruction(d);
		STATE(d) = CON_STAT_LIST;
		return false;
	}
	default : {
		SEND_TO_Q("\n\rSelezione non valida!\n\r\n\r",d);
		show_class_selection(d,GET_RACE(d->character));
		break;
	}
	} /* end arg switch */

	if(STATE(d) != CON_QCLASS && IS_SET(SystemFlags, SYS_REQAPPROVE)) {
		STATE(d) = CON_AUTH;
		SEND_TO_Q("\r\n[Batti INVIO] ", d);
	}
	else {
		if(STATE(d) != CON_QCLASS) {
			mudlog(LOG_CONNECT, "%s [HOST:%s] nuovo giocatore.",
				   GET_NAME(d->character), d->host);
			/*
			  ** now that classes are set, initialize
			  */
			init_char(d->character);

			/* crea i files relativi a char */

			save_char(d->character, AUTO_RENT, 0);

			if(HasClass(d->character,CLASS_MAGIC_USER)) {
				SEND_TO_Q(RU_SORCERER, d);
				STATE(d) = CON_CHECK_MAGE_TYPE;
				return false;
			}
			else {
				/* show newbies a instructional note from interpreter.h */
				/*page_string(d,NEWBIE_NOTE,1);*/
				SEND_TO_Q(NEWBIE_NOTE, d);
				SEND_TO_Q("\n\r[Batti INVIO] ", d);
				STATE(d) = CON_RNEWD;
			}
		}
	}
	return false;
}

NANNY_FUNC(con_slct) {
	switch(firstChar(d->currentInput)) {
	case '0':
		toonUpdate(d);
		close_socket(d);
		break;

	case 'c': {
		if(GetMaxLevel(d->character)>=CHUMP) {
			SEND_TO_Q("Sei sicuro di volerti cancellare ? (si/no): ",d);
			STATE(d)=CON_DELETE_ME;
			break;
		}
	}
	/* no break */
	case '1':
	{
		reset_char(d->character);
		int Level=GetMaxLevel(d->character);
		if (PORT==RELEASE_PORT) {
			if (Level > PRINCIPE and Level < MAESTRO_DEL_CREATO) {
				slackNotify(string(d->character->player.name).append(" si e` connesso").c_str(),":dagger_knife:");
			}
		}
		toonUpdate(d);
		mudlog(LOG_PLAYERS, "M1.Loading %s's equipment",d->character->player.name);
		load_char_objs(d->character);
		mudlog(LOG_CHECK, "Sending Welcome message to %s",d->character->player.name);
		send_to_char(WELC_MESSG, d->character);
		mudlog(LOG_CHECK, "Putting %s in list",
			   d->character->player.name);
		d->character->next = character_list;
		character_list = d->character;
		mudlog(LOG_CHECK, "Putting %s in game",
			   d->character->player.name);
		if(d->character->in_room == NOWHERE ||
				d->character->in_room == AUTO_RENT) {
			/* returning from autorent */
			if (Level < DIO_MINORE) {
				/* Per gli IMMORTALI che rentavano ad Asgaard, gli tolgo
				la start room cosi' gli viene calcolata di nuovo */
				if(d->character->specials.start_room == 1001 && IS_IMMORTALE(d->character)) {
					d->character->specials.start_room = -1;
				}

				if(d->character->specials.start_room <= 0)
					/*GGPATCH Ogni razza ha la sua HomeTown definita in constants.c*/

				{
					mudlog(LOG_PLAYERS,"%s = Razza: %d Stanza0: %d Stanza2: %d",
						   GET_NAME(d->character),
						   GET_RACE(d->character),
						   RacialHome[GET_RACE(d->character)][0],
						   RacialHome[GET_RACE(d->character)][1]);
					char_to_room(d->character,
								 RacialHome[GET_RACE(d->character)][0]);
					d->character->player.hometown =
						RacialHome[GET_RACE(d->character)][0];
					mudlog(LOG_PLAYERS,"%s in room %5d by normal",
						   GET_NAME(d->character),d->character->player.hometown);
				}
				else {
					char_to_room(d->character, d->character->specials.start_room);
					d->character->player.hometown =
						d->character->specials.start_room;
					mudlog(LOG_PLAYERS,"%s in room %5d by special",
						   GET_NAME(d->character),d->character->player.hometown);
				}
			}
			else {
				/* Gli immortali tornano sempre ad Asgaard :-) */
				char_to_room(d->character, 1000);
				d->character->player.hometown = 1000;
			}
		}
		else {
			if(IS_DIO(d->character)) {
				d->character->in_room = 1000;
			}

			if(real_roomp(d->character->in_room)) {
				char_to_room(d->character, d->character->in_room);
				d->character->player.hometown = d->character->in_room;
			}
			else {
				/* Qualcosa e' andato storto o nuovo PC stanza di default */
				char_to_room(d->character,
							 RacialHome[GET_RACE(d->character)][1]);
				d->character->player.hometown =
					RacialHome[GET_RACE(d->character)][1];
				mudlog(LOG_PLAYERS,"%s in room %5d by default",
					   GET_NAME(d->character),d->character->player.hometown);
			}
		}

		d->character->specials.tick = plr_tick_count++;
		if(plr_tick_count == PLR_TICK_WRAP) {
			plr_tick_count=0;
		}

		act("$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0, TO_ROOM);
		STATE(d) = CON_PLYNG;
		if(!GetMaxLevel(d->character)) {
			do_start(d->character);
		}
		if(has_mail(d->character->player.name))
			send_to_char("$c0014C'e` posta per te dallo scriba.$c0007\n\r\n\r",
						 d->character);
		do_look(d->character, "",15);
		if(!d->character->specials.lastversion ||
				strcmp(d->character->specials.lastversion,version()))
			send_to_char(
				"$c0115           C'E` UNA NUOVA VERSIONE DI MYST IN LINEA                   $c0007.\n\r",
				d->character);
		if(IsTest())
			send_to_char(
				"                 $c0115SEI SU MYST2!!!!!!!!!!!!!!!!!!!!!                    $c0007\n\r",
				d->character);
		d->prompt_mode = 1;
		if(IS_SET(d->character->player.user_flags,RACE_WAR))
			send_to_char(
				"$c0115            RICORDATI CHE  SEI PKILL!!.\n\r",
				d->character);
		mudlog(LOG_CHECK, "%s is in game.", d->character->player.name);

		{
			struct room_data* rp = real_roomp(d->character->in_room);

			/* qui metto un controllo. Se un PG e' pkiller e il mud ha crashato
			   allora rischia di restare bloccato, lo sparo via da qualche parte :-)
			   Gaia 2001 */

			if(IS_AFFECTED2(d->character,AFF2_PKILLER) && rp->room_flags&PEACEFUL) {
				mudlog(LOG_CHECK, "A Pkill character has entered in game in a peaceful room");
				send_to_char("La magia che avvolge questo luogo di pace ti rigetta!\n\r",d->character);
				send_to_room("Un brezza sottile si alza improvvisamente \n\r", d->character->in_room);
				char_from_room(d->character);
				char_to_room(d->character, 3001); //mando il PG in piazza
			}
		}

		break;
	}
	case '2':
		SEND_TO_Q("Inserisci il testo che vuoi che venga visualizzato "
				  "quando gli altri\n\r", d);
		SEND_TO_Q("ti guardano. Concludilo con un '@'.\n\r", d);
		if(d->character->player.description) {
			SEND_TO_Q("Vecchia descrizione :\n\r", d);
			SEND_TO_Q(d->character->player.description, d);
			free(d->character->player.description);
			d->character->player.description = 0;
		}
		d->str = &d->character->player.description;
		d->max_str = 240;
		STATE(d) = CON_EXDSCR;
		break;

	case '3':
		SEND_TO_Q(STORY, d);
		STATE(d) = CON_WMOTD;
		break;

	case '4':
		if (d->AccountData.authorized) {
			string message("$c0001ATTENZIONE$c0007 Stai cambiando la password del tuo $c0001account$c0007 (");
			message.append(d->AccountData.email).append(")\r\n");
			send_to_char(message.c_str(),d->character);
		}
		SEND_TO_Q("Inserisci la nuova password: ", d);
		echoOff(d);
		STATE(d) = CON_PWDNEW;
		break;
	case '5':
		free_char(d->character);
		d->character=nullptr;
		if(d->AccountData.authorized) {
			toonList(d,"Cambia personaggio:\n\r");
			STATE(d) = CON_ACCOUNT_TOON;
		}
		else {
			STATE(d)=CON_NME;
		}
		break;
	default:
		SEND_TO_Q("Opzione errata.\n\r", d);
		SEND_TO_Q(MENU, d);
		break;
	}
	return false;
}
NANNY_FUNC(con_nme) {
	oldarg(true);
	char tmp_name[100];
	unsigned long long rc=parse_name(arg, tmp_name);
	mudlog(LOG_CONNECT,"Parsename result for %s: %d",arg,rc);
	if (rc>2ULL) {
		d->AccountData.id=rc-2;
		d->AccountData.email.clear();
		STATE(d)=CON_ACCOUNT_NAME;
		return true;
	}
	else if(rc==2ULL) {  // Il nome digitato contiene una @
		d->AccountData.id=0;
		d->AccountData.email.assign(arg);
		boost::replace_all(d->AccountData.email," ","");
		STATE(d)=CON_ACCOUNT_NAME;
		return true;
	}
	else if(rc==1ULL) {
		SEND_TO_Q("Nome non ammesso. Scegline un altro, per favore.\r\n", d);
		SEND_TO_Q("Nome: ", d);
		return false;
	}
	if(PORT!=RELEASE_PORT and not d->AccountData.authorized) {
		FLUSH_TO_Q("Per accedere al server di prova devi entrare con l'email\n\r",d);
		close_socket(d);
		return false;
	}
	bool found=false;
	toonPtr pg=Sql::getOne<toon>(toonQuery::name==string(tmp_name));
	if (!pg) {
		found=toonFromFileSystem(tmp_name);
		if (found) {
			pg=Sql::getOne<toon>(toonQuery::name==string(tmp_name));
			if (!pg) {
				// SOmething badly wrong, let's force this guy to restart
				FLUSH_TO_Q("Mi spiace, questo nome non va bene\n\r",d);
				close_socket(d);
				return false;
			}
		}
	}
	if(pg) {
		mudlog(LOG_CONNECT,"Toon found on db, registered to %d",pg->owner_id);
		found=true;
		strcpy(d->pwd,pg->password.substr(0,11).c_str());
		d->AccountData.choosen=pg->name;
		if(pg->owner_id) {
			if(pg->owner_id==d->AccountData.id) {
				STATE(d)=CON_PWDOK;
				return true;
			}
			else if (d->AccountData.level < MAESTRO_DEL_CREATO)  {
				SEND_TO_Q("Questo personaggio e` registrato, fai login con il tuo account per favore.\r\n",d);
				SEND_TO_Q("Nome: ", d);
				return false;
			}
		}
	}
	if(not found) {
		/* player unknown gotta make a new */
		if(_check_ass_name(tmp_name)) {
			if(d->AccountData.authorized and !strncmp(arg,"b",1)) {
				toonList(d,"Scegli un personaggio:\n\r");
				STATE(d)=CON_ACCOUNT_TOON;
				return false;
			}
			SEND_TO_Q("Nome non valido. Scegline un'altro, per favore.\n\r", d);
			SEND_TO_Q("Nome: ", d);
			return false;
		}
		if(!WizLock) {
			if(!d->character) {
				CREATE(d->character, struct char_data, 1);
				clear_char(d->character);
				d->character->desc = d;
				SET_BIT(d->character->player.user_flags, USE_PAGING);
			}
			CREATE(GET_NAME(d->character), char, strlen(tmp_name) + 1);
			CAP(tmp_name);
			strcpy(GET_NAME(d->character), tmp_name);
			string buf("E` realmente '");
			buf.append(tmp_name).append("' il nome che vuoi ? (si/no): ");
			d->AccountData.choosen.assign(tmp_name);
			SEND_TO_Q(buf.c_str(), d);
			STATE(d) = CON_NMECNF;
			return false;
		}
		else {
			SEND_TO_Q("Mi dispiace. Non sono ammessi nuovi personaggi, per il momento.\n\r",d);
			STATE(d) = CON_WIZLOCK;
			return false;
		}
	}
	d->AccountData.choosen.assign(tmp_name);
	/* Tutto ok, chiediamogli la password */
	if(d->AccountData.level >= MAESTRO_DEL_CREATO and PORT != RELEASE_PORT) {
		SEND_TO_Q(ParseAnsiColors(TRUE,
				"Non oserei mai chiederti la password, oh superno, ma $c0009ricorda che il pg non e` tuo$c0007\r\n")
				,d);
		mudlog(LOG_ALWAYS,"%s e` entrato come %s",d->AccountData.email,d->AccountData.choosen);
		d->impersonating=true;
		//Un immortale superiore puo' entrare con qualsiasi PG
		STATE(d)=CON_PWDOK;
		return true;
	}

	SEND_TO_Q("Password: ", d);
	echoOff(d);
	STATE(d) = CON_PWDNRM;
	return false;
}

NANNY_FUNC(con_nmecnf) {
	/* skip whitespaces */
	oldarg(false);
	if(*arg == 's' || *arg == 'S') {
		if(d->AccountData.authorized) {  // Authorized at account level no need to ask password again when creating a new toon
			echoOn(d);
			show_race_choice(d);
			STATE(d)=CON_QRACE;
			return false;
		}
		echoOn(d);
		SEND_TO_Q("Nuovo personaggio.\n\r", d);

		string buf("Inserisci una password per ");
		buf.append(GET_NAME(d->character));

		SEND_TO_Q(buf.c_str(), d);
		echoOff(d);
		STATE(d) = CON_PWDGET;
	}
	else if(*arg == 'n' || *arg == 'N') {
		SEND_TO_Q("Va bene. Allora, quale sarebbe il nome ? ", d);
		free(GET_NAME(d->character));
		GET_NAME(d->character) = NULL;
		STATE(d) = CON_NME;
	}
	else {
		/* Please do Y or N */
		SEND_TO_Q("Per favore, si o no ? ", d);
	}
	return false;
}
/**
 * Existent player password
 */
NANNY_FUNC(con_pwdnrm) {
	oldarg(true);
	if(strncmp(crypt(arg, d->pwd), d->pwd, strlen(d->pwd))) {
		write_to_descriptor(d->descriptor,"Password errata.\n\r");
		mudlog(LOG_PLAYERS,"%s [HOST:%s] ha inserito una password errata'",d->AccountData.choosen.c_str(),d->host);
		close_socket(d);
		return false;
	}
	STATE(d)=CON_REGISTER;
	return true;
}
NANNY_FUNC(con_register) {
	if(d->AccountData.authorized) {
		boost::format fmt(R"(UPDATE toon SET owner_id =%d WHERE name="%s")");
		fmt % d->AccountData.id % d->AccountData.choosen;
		try {
			DB* db=Sql::getMysql();
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			db->execute(fmt.str());
			t.commit();
		}
		catch(odb::exception &e) {
			mudlog(LOG_SYSERR,"Db error while registering %s: %s",d->AccountData.choosen.c_str(),e.what());
		}
	}
	STATE(d)=CON_PWDOK;
	return true;
}
NANNY_FUNC(con_pwdok) {
	/* Ok, il ragazzo ha azzeccato la password */
	if(!d->character) {
		CREATE(d->character, struct char_data, 1);
		clear_char(d->character);
		d->character->desc = d;
		SET_BIT(d->character->player.user_flags, USE_PAGING);
	}
	/* Newly created toons are fully loaded
	 */
	if(!d->justCreated) {
		char_file_u tmp_store;
		if(load_char(d->AccountData.choosen.c_str(), &tmp_store)) {
			store_to_char(&tmp_store, d->character);
		}	//TODO: Inserire qui load del pg
		else {
			//Something went terribly wrong
			mudlog(LOG_SYSERR,"Non trovo %s in CON_PWDOK ?!?",d->AccountData.choosen.c_str());
			FLUSH_TO_Q("Unable to load ",d);
			FLUSH_TO_Q(d->AccountData.choosen.c_str(),d);
			close_socket(d);
			return false;
		}
	}
	if (d->impersonating) {
		// Check relative level
		if (GetMaxLevel(d->character)>=d->AccountData.level) {
			FLUSH_TO_Q("Mi spiace, non puo impersonare personaggi di livello superiore o uguale al tuo\r\n",d);
			close_socket(d);
			return false;
		}
	}
	if(d->AccountData.authorized and !d->impersonating and d->AccountData.level < GetMaxLevel(d->character)) {
		d->AccountData.level = GetMaxLevel(d->character);
		d->AccountData.nickname.assign(GET_NAME(d->character));
		Sql::update(d->AccountData,true);
	}
#if IMPL_SECURITY
	if(not check_impl_security(d)) {
		return false;
	}
#endif
	d->wait=0;
	d->AlreadyInGame=false;
	/* Check if already playing with some or other name*/
	for(struct descriptor_data* k=descriptor_list; k; k = k->next) {
		if((k->character != d->character) && k->character) {
			struct char_data* test = (k->original?k->original:k->character);
			if((test and GET_NAME(test) and !str_cmp(GET_NAME(test),GET_NAME(d->character)))) {
				d->AlreadyInGame=true;
				d->ToBeKilled=k;
				mudlog(LOG_CONNECT,"%s : gia' in gioco.",GET_NAME(test));
			}
		}
	}
	/* Se era gia` in gioco assumo ld non riconosciuto e disconnetto il
	  * vecchio char*/
	if(d->AlreadyInGame) {
		mudlog(LOG_PLAYERS, "%s[HOST:%s] riconnesso su se stesso.", GET_NAME(d->character),d->host);
		close_socket(d->ToBeKilled);
	}
	for(struct char_data* tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
		if((!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) && !tmp_ch->desc && !IS_NPC(tmp_ch)) ||
				(IS_NPC(tmp_ch) && tmp_ch->orig &&!str_cmp(GET_NAME(d->character),GET_NAME(tmp_ch->orig)))) {
			/* Se riconnessione, abbandono il nuovo Char creato
			* e aggancio al descrittore corrente il char ld */

			echoOn(d);
			SEND_TO_Q("Riconnessione...\n\r", d);

			free_char(d->character);
			tmp_ch->desc = d;
			d->character = tmp_ch;
			tmp_ch->specials.timer = 0;
			if(!IS_IMMORTAL(tmp_ch)) {
				tmp_ch->invis_level = 0;
			}
			if(tmp_ch->orig) {
				tmp_ch->desc->original = tmp_ch->orig;
				tmp_ch->orig = 0;
			}
			d->character->persist = 0;
			STATE(d) = CON_PLYNG;

			act("$n si e` riconnesso.", TRUE, tmp_ch, 0, 0, TO_ROOM);
			mudlog(LOG_CONNECT, "%s[HOST:%s] has reconnected.",
				   GET_NAME(d->character), d->host);

			/* inserisco qui la ripartenza dei regen interrotti per i link dead */
			alter_hit(tmp_ch, 0) ;
			alter_mana(tmp_ch, 0) ;
			alter_move(tmp_ch, 0) ;

			return false;
		}
#if ACCESSI
		if(d->AccountData.level <= MAESTRO_DEGLI_DEI) {
			if(!d->AlreadyInGame &&
					tmp_ch->desc &&
					tmp_ch->desc->AccountData.id &&
					tmp_ch->desc->AccountData.id  == d->AccountData.id) {
				SEND_TO_Q("Hai gia' un personaggio nel gioco.\n\r", d);
				mudlog(LOG_PLAYERS,"WARNING %s respinto per violazione MP.",GET_NAME(d->character));
				if(d->AccountData.authorized) {
					string message("");
					toonList(d,message);
					STATE(d)=CON_ACCOUNT_TOON;
				}
				else {
					SEND_TO_Q("Scegli un nuovo personaggio: ", d);
					STATE(d)=CON_NME;
				}
				return false;
			}
		}
#endif
	}
	/* Ok, non si tratta di riconnessione...
	  * gli immortali entrano invisibili */
	if(IS_IMMORTAL(d->character)) {
		d->character->invis_level=ADEPT;
	}
	if(IS_DIO(d->character)) {
		d->character->invis_level = GetMaxLevel(d->character);
	}
	HowManyConnection(1);
	/* Le ombre vengono loggate ma non viene dato l'avviso on line*/
	if(GetMaxLevel(d->character) >= MAESTRO_DEL_CREATO) {
		mudlog(LOG_CONNECT, "%s [HOST:%s] has connected.",GET_NAME(d->character),d->host);
	}
	else {
		mudlog(LOG_CONNECT, "%s [HOST:%s] has connected.",GET_NAME(d->character),d->host);
	}
	SEND_TO_Q(ParseAnsiColors(IS_SET(d->character->player.user_flags,
									 USE_ANSI),
							  motd), d);
	SEND_TO_Q("\n\r[Batti INVIO] ", d);
	STATE(d) = CON_RMOTD;
	return false;
}
/**
 * New player password
 */
NANNY_FUNC(con_pwdget) {
	oldarg(false);
	if(!*arg || strlen(arg) > 10 ||
			!strcasecmp(arg,d->character->player.name)) {
		echoOn(d);
		SEND_TO_Q("Password non valida.(MAx 10 caratteri - diversa dal nome)\n\r", d);
		SEND_TO_Q("Password: ", d);
		echoOff(d);
		return false;
	}
	strncpy(d->pwd,crypt(arg, d->character->player.name), 10);
	*(d->pwd + 10) = '\0';
	echoOn(d);
	SEND_TO_Q("Per favore, reinserisci la password: ", d);
	echoOff(d);
	STATE(d) = CON_PWDCNF;
	return false;
}
NANNY_FUNC(con_pwdcnf) {
	oldarg(false);
	if(strncmp((char*)crypt(arg, d->pwd), d->pwd, strlen(d->pwd))) {
		echoOn(d);

		SEND_TO_Q("Le password non coincidono.\n\r", d);
		SEND_TO_Q("Reinserisci la password: ", d);
		STATE(d) = CON_PWDGET;
		echoOff(d);
	}
	else {
		echoOn(d);
		show_race_choice(d);
		STATE(d) = CON_QRACE;
	}
	return false;
}
NANNY_FUNC(con_qrace) {
	oldarg(false);
	if(!*arg) {
		show_race_choice(d);
		STATE(d) = CON_QRACE;
	}
	else {
		if(*arg == '?') {
			page_string(d, RACEHELP, 1);
			STATE(d) = CON_ENDHELPRACE;
		}
		else {
			int i=0,tmpi=0;
			while(race_choice[i]!=-1) {
				i++;
			}
			tmpi=atoi(arg);
			if(tmpi>=0 && tmpi <=i-1) {
				/* set the chars race to this */
				GET_RACE(d->character) = race_choice[tmpi];
				string buf("Quale'e` il sesso di ");
				buf.append(GET_NAME(d->character)).append("? (Maschio/Femmina) (b per tornare indietro): ");
				SEND_TO_Q(buf.c_str(), d);
				mudlog(LOG_CONNECT,"Razza scelta procedo con qsex");
				STATE(d) = CON_QSEX;
			}
			else {
				SEND_TO_Q("\n\rScelta non valida.\n\r\n\r", d);
				show_race_choice(d);
				STATE(d) = CON_QRACE;
				mudlog(LOG_CONNECT,"Razza non valida");
				/* bogus race selection! */
			}
		}
	}
	return false;
}
NANNY_FUNC(con_helprace) {
	SEND_TO_Q("\r\n[Batti INVIO] ", d);
	STATE(d) = CON_ENDHELPRACE;
	return false;
}
NANNY_FUNC(con_helproll) {
	RollPrompt(d);
	STATE(d) = CON_QROLL;
	return false;
}
NANNY_FUNC(con_qroll) {
	oldarg(false);
	InterpretaRoll(d,arg);
	/* Lo stato viene impostatto da InterpretaRoll */
	return false;
}
NANNY_FUNC(con_conf_roll) {
	switch(firstChar(d->currentInput,true)) {
	case 's':
		show_class_selection(d,GET_RACE(d->character));
		STATE(d) = CON_QCLASS;
		break;
	case 'n':
		ShowRollInstruction(d);
		STATE(d) = CON_HELPROLL;
		break;
	case 'b':
		ShowStatInstruction(d);
		STATE(d)=CON_STAT_LIST;
		break;
	}
	return false;
}
NANNY_FUNC(con_endhelprace) {
	show_race_choice(d);
	STATE(d) = CON_QRACE;
	return false;
}
NANNY_FUNC(con_qsex) {                /* query sex of new user        */
	/* skip whitespaces */
	switch(firstChar(d->currentInput,true)) {
	case 'm':
		d->character->player.sex = SEX_MALE;
		break;
	case 'f':
		d->character->player.sex = SEX_FEMALE;
		break;
	case 'b':
		/* backward */
		show_race_choice(d);
		STATE(d)=CON_QRACE;
		return false;
		break;
	default:
		SEND_TO_Q("Qui su Nebbie Arcane amiamo le cose semplici.... accontentati di due sessi.\n\r", d);
		SEND_TO_Q("Quel'e` il tuo sesso ? (maschio/femmina): (b per tornare indietro)", d);
		return false;
	}
	ShowStatInstruction(d);
	STATE(d) = CON_STAT_LIST;
	return false;
}

NANNY_FUNC(con_stat_list) {
	/* skip whitespaces */
	oldarg(false);
	if(strlen(arg)==1 && (*arg == 'B' || *arg == 'b')) {  /* Backward */
		string buf("Quale'e` il sesso di ");
		buf.assign(GET_NAME(d->character)).assign("maschio/Femmina) (b per tornare indietro): ");
		SEND_TO_Q(buf.c_str(),d);
		STATE(d) = CON_QSEX;
		return false;
	}
	if(!strncasecmp(arg,"nuovo",5) || !strncasecmp(arg,"new",3)) {  /* New roll */
		ShowRollInstruction(d);
		STATE(d) = CON_HELPROLL;
		return false;
	}

	if(!*arg) {
		d->TipoRoll='S';
		show_class_selection(d,GET_RACE(d->character));
		if(IS_SET(SystemFlags,SYS_REQAPPROVE)) {
			/* set the AUTH flags */
			/* (3 chances) */
			d->character->generic = NEWBIE_REQUEST+NEWBIE_CHANCES;
		}
		STATE(d) = CON_QCLASS;
		return false;
	}
	int index=0;
	char* p=&arg[0];
	while(*p && index < MAX_STAT) {
		if(*p == 'F' || *p == 'f') {
			d->stat[index++] = 's';
		}
		if(*p == 'I' || *p == 'i') {
			d->stat[index++] = 'i';
		}
		if(*p == 'S' || *p == 's') {
			d->stat[index++] = 'w';
		}
		if(*p == 'A' || *p == 'a') {
			d->stat[index++] = 'd';
		}
		if(*p == 'C' || *p == 'c') {
			p++;
			if(*p == 'O' || *p == 'o') {
				d->stat[index++] = 'o';
			}
			else if(*p == 'A' || *p == 'a') {
				d->stat[index++] = 'h';
			}
		}
		p++;
	}

	if(index < MAX_STAT) {
		SEND_TO_Q("Non hai inserito tutte le statistiche richieste o qualche scelta e` sbagliata.\n\r\n\r", d);
		ShowStatInstruction(d);
		STATE(d) = CON_STAT_LIST;
		return false;
	}
	else {
		d->TipoRoll='V';

		show_class_selection(d,GET_RACE(d->character));

		if(IS_SET(SystemFlags,SYS_REQAPPROVE)) {
			/* set the AUTH flags */
			/* (3 chances) */
			d->character->generic = NEWBIE_REQUEST+NEWBIE_CHANCES;
		}
		STATE(d) = CON_QCLASS;
		return false;
	}
}
NANNY_FUNC(con_helpclass) {
	SEND_TO_Q("\n\r[Batti INVIO] ", d);
	STATE(d) = CON_ENDHELPCLASS;
	return false;
}
NANNY_FUNC(con_endhelpclass) {
	show_class_selection(d,GET_RACE(d->character));
	STATE(d) = CON_QCLASS;
	return false;
}
NANNY_FUNC(con_rnewd) {
	mudlog(LOG_CONNECT,"Nome: %s Password: %s",d->character->player.name,d->pwd);
	SEND_TO_Q(ParseAnsiColors(IS_SET(d->character->player.user_flags,
									 USE_ANSI),
							  motd), d);
	d->justCreated=true;
	STATE(d) = CON_REGISTER;
	return true;
}
NANNY_FUNC(con_check_mage_type) {
	oldarg(false);
	if(!strcasecmp(arg,"si")) {
		d->character->player.iClass -=CLASS_MAGIC_USER;
		d->character->player.iClass +=CLASS_SORCERER;
	} /* end we wanted Sorcerer class! */
	SEND_TO_Q(NEWBIE_NOTE, d);
	SEND_TO_Q("\n\r[Batti INVIO] ", d);
	STATE(d) = CON_RNEWD;
	return false;
}
NANNY_FUNC(con_rmotd) {
	if(GetMaxLevel(d->character) > IMMORTALE) {
		SEND_TO_Q(ParseAnsiColors(IS_SET(d->character->player.user_flags,
										 USE_ANSI),
								  wmotd), d);
		SEND_TO_Q("\r\n[Batti INVIO] ", d);
		STATE(d) = CON_WMOTD;
		return false;
	}
	if(d->character->term != 0) {
		ScreenOff(d->character);
	}
	SEND_TO_Q(MENU, d);
	STATE(d) = CON_SLCT;
	if(WizLock) {
		if(GetMaxLevel(d->character) < DIO) {
			SEND_TO_Q("Sorry, the game is locked up for repair.\n\r",d);
			STATE(d) = CON_WIZLOCK;
			close_socket(d);
		}
	}
	return false;
}

NANNY_FUNC(con_wmotd) {
	SEND_TO_Q(MENU, d);
	STATE(d) = CON_SLCT;
	if(WizLock) {
		if(GetMaxLevel(d->character) < DIO) {
			SEND_TO_Q("Sorry, the game is locked up for repair.\n\r",d);
			STATE(d) = CON_WIZLOCK;
			close_socket(d);
			return false;
		}
	}
	return false;
}
NANNY_FUNC(con_wizlock) {
	close_socket(d);
	return false;
}
NANNY_FUNC(con_city_choice) {
	oldarg(false);

	if(d->character->in_room != NOWHERE) {
		SEND_TO_Q("This choice is only valid when you have been auto-saved\n\r",d);
		STATE(d) = CON_SLCT;
	}
	else {
		switch(*arg) {
		case '1':
			reset_char(d->character);
			mudlog(LOG_CONNECT, "1.Loading %s's equipment",
				   d->character->player.name);
			load_char_objs(d->character);
			SetStatus("int 1",NULL,NULL);
			save_char(d->character, AUTO_RENT, 0);
			SetStatus("int 2",NULL,NULL);
			send_to_char(WELC_MESSG, d->character);
			SetStatus("int 3",NULL,NULL);
			d->character->next = character_list;
			SetStatus("int 4",NULL,NULL);
			character_list = d->character;
			SetStatus("int 5",NULL,NULL);
			char_to_room(d->character, 3001);
			SetStatus("int 6",NULL,NULL);
			d->character->player.hometown = 3001;
			SetStatus("int 7",NULL,NULL);
			d->character->specials.tick = plr_tick_count++;
			if(plr_tick_count == PLR_TICK_WRAP) {
				plr_tick_count=0;
			}
			SetStatus("int 8",NULL,NULL);

			act("$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0, TO_ROOM);
			STATE(d) = CON_PLYNG;
			SetStatus("int 9",NULL,NULL);
			if(!GetMaxLevel(d->character)) {
				do_start(d->character);
			}
			SetStatus("int A",NULL,NULL);
			do_look(d->character, "", 15);
			SetStatus("int B",NULL,NULL);
			d->prompt_mode = 1;
			break;
		case '2':
			reset_char(d->character);
			mudlog(LOG_CONNECT, "2.Loading %s's equipment",d->character->player.name);
			load_char_objs(d->character);
			save_char(d->character, AUTO_RENT, 0);
			send_to_char(WELC_MESSG, d->character);
			d->character->next = character_list;
			character_list = d->character;
			char_to_room(d->character, 1103);
			d->character->player.hometown = 1103;
			d->character->specials.tick = plr_tick_count++;
			if(plr_tick_count == PLR_TICK_WRAP) {
				plr_tick_count=0;
			}
			act("$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0, TO_ROOM);
			STATE(d) = CON_PLYNG;
			if(!GetMaxLevel(d->character)) {
				do_start(d->character);
			}
			do_look(d->character, "",15);
			d->prompt_mode = 1;
			break;
		case '3':
			if(GetMaxLevel(d->character) > 5) {
				reset_char(d->character);
				mudlog(LOG_CONNECT, "3.Loading %s's equipment",
					   d->character->player.name);
				load_char_objs(d->character);
				save_char(d->character, AUTO_RENT, 0);
				send_to_char(WELC_MESSG, d->character);
				d->character->next = character_list;
				character_list = d->character;
				char_to_room(d->character, 18221);
				d->character->player.hometown = 18221;
				d->character->specials.tick = plr_tick_count++;
				if(plr_tick_count == PLR_TICK_WRAP) {
					plr_tick_count=0;
				}

				act("$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0,
					TO_ROOM);
				STATE(d) = CON_PLYNG;
				if(!GetMaxLevel(d->character)) {
					do_start(d->character);
				}
				do_look(d->character, "",15);
				d->prompt_mode = 1;
			}
			else {
				SEND_TO_Q("Questa scelta non e` valida.\n\r", d);
				STATE(d) = CON_SLCT;
			}
			break;
		case '4':
			if(GetMaxLevel(d->character) > 5) {
				reset_char(d->character);
				mudlog(LOG_CONNECT, "4.Loading %s's equipment",
					   d->character->player.name);
				load_char_objs(d->character);
				save_char(d->character, AUTO_RENT, 0);
				send_to_char(WELC_MESSG, d->character);
				d->character->next = character_list;
				character_list = d->character;

				char_to_room(d->character, 3606);
				d->character->player.hometown = 3606;

				d->character->specials.tick = plr_tick_count++;
				if(plr_tick_count == PLR_TICK_WRAP) {
					plr_tick_count=0;
				}

				act("$n e` entrat$b nel gioco.", TRUE, d->character, 0, 0,
					TO_ROOM);
				STATE(d) = CON_PLYNG;
				if(!GetMaxLevel(d->character)) {
					do_start(d->character);
				}
				do_look(d->character, "",15);
				d->prompt_mode = 1;
			}
			else {
				SEND_TO_Q("That was an illegal choice.\n\r", d);
				STATE(d) = CON_SLCT;
			}
			break;
		case '5':
			if(GetMaxLevel(d->character) > 5) {
				reset_char(d->character);
				mudlog(LOG_CONNECT, "5.Loading %s's equipment",
					   d->character->player.name);
				load_char_objs(d->character);
				save_char(d->character, AUTO_RENT, 0);
				send_to_char(WELC_MESSG, d->character);
				d->character->next = character_list;
				character_list = d->character;

				char_to_room(d->character, 16107);
				d->character->player.hometown = 16107;

				d->character->specials.tick = plr_tick_count++;
				if(plr_tick_count == PLR_TICK_WRAP) {
					plr_tick_count=0;
				}

				act("$n has entered the game.",
					TRUE, d->character, 0, 0, TO_ROOM);
				STATE(d) = CON_PLYNG;
				if(!GetMaxLevel(d->character)) {
					do_start(d->character);
				}
				do_look(d->character, "",15);
				d->prompt_mode = 1;
			}
			else {
				SEND_TO_Q("That was an illegal choice.\n\r", d);
				STATE(d) = CON_SLCT;
			}
			break;
		default:
			SEND_TO_Q("That was an illegal choice.\n\r", d);
			STATE(d) = CON_SLCT;
			break;
		}
	}
	return false;
}
NANNY_FUNC(con_delete_me) {
	oldarg(false);
	if(!strcmp(arg,"si") && strcmp("Guest",GET_NAME(d->character))) {
		char buf[MAX_INPUT_LENGTH * 2];

		mudlog(LOG_PLAYERS, "%s just killed self!",
			   GET_NAME(d->character));
		sprintf(buf, "rm %s/%s.dat", PLAYERS_DIR,
				lower(GET_NAME(d->character)));
		system(buf);
		sprintf(buf, "rm %s/%s", RENT_DIR, lower(GET_NAME(d->character)));
		system(buf);
		sprintf(buf, "rm %s/%s.aux", RENT_DIR, lower(GET_NAME(d->character)));
		system(buf);
		toonPtr pg=Sql::getOne<toon>(toonQuery::name==string(GET_NAME(d->character)));
		if (pg) Sql::erase(*pg,true);
		SEND_TO_Q("Done\n\t",d);
        if (d->AccountData.id) {    // controllo se ha un id
		SEND_TO_Q(MENU,d);
		STATE(d)= CON_SLCT;
        }
        else
        {
            Sql::update(d->AccountData);
            toonUpdate(d);
            close_socket(d);
            return false;
        }
	}
	else {
		SEND_TO_Q(MENU,d);
		STATE(d)= CON_SLCT;
	}
	return false;
}

NANNY_FUNC(con_pwdnew) {
	oldarg(false);
	if(!*arg || strlen(arg) > 10 || strlen(arg) <6) {
		echoOn(d);
		SEND_TO_Q("Password non valida (deve essere di lunghezza compresa fra 6 e 10 caratteri).\n\r", d);
		SEND_TO_Q("Password: ", d);
		echoOff(d);
		return false;
	}
	string salt(RandomWord());
	salt.append(RandomWord());
	random_shuffle(salt.begin(),salt.end());
	strncpy(d->pwd,crypt(arg, salt.c_str()), 10);
	*(d->pwd + 10) = '\0';
	echoOn(d);
	SEND_TO_Q("Reinserisci la password: ", d);
	STATE(d) = CON_PWDNCNF;
	echoOff(d);
	return false;
}
NANNY_FUNC(con_pwdncnf) {
	oldarg(false);
	if(strncmp(crypt(arg, d->pwd), d->pwd, strlen(d->pwd))) {
		echoOn(d);
		SEND_TO_Q("Password errata.\n\r", d);
		SEND_TO_Q("Reinserisci la password: ", d);
		echoOff(d);

		STATE(d) = CON_PWDNEW;
		return false;
	}
	if (d->AccountData.authorized) {
		d->AccountData.password.assign(crypt(arg, d->pwd));
		Sql::update(d->AccountData);
	}
	echoOn(d);

	SEND_TO_Q(
		"\n\rFatto. Devi entrare nel gioco per rendere attivo il cambio.\n\r",
		d);
	SEND_TO_Q(MENU, d);
	STATE(d) = CON_SLCT;
	return false;
}
void assign_nannies_pointers() {
	// Initializes with a stubg
	for(unsigned int i=0; i< E_CONNECTION_TYPES_COUNT; ++i) {
		nannyFuncs[i]=con_nop;
	}
	nannyFuncs[CON_ACCOUNT_NAME]=con_account_name;
	nannyFuncs[CON_ACCOUNT_PWD]=con_account_pwd;
	nannyFuncs[CON_ACCOUNT_TOON]=con_account_toon;
	nannyFuncs[CON_AUTH]=con_nop;
	nannyFuncs[CON_CHECK_MAGE_TYPE]=con_check_mage_type;
	nannyFuncs[CON_CITY_CHOICE]=con_city_choice;
	nannyFuncs[CON_CONF_ROLL]=con_conf_roll;
	nannyFuncs[CON_DELETE_ME]=con_delete_me;
	nannyFuncs[CON_EDITING]=con_nop;
	nannyFuncs[CON_ENDHELPCLASS]=con_endhelpclass;
	nannyFuncs[CON_ENDHELPRACE]=con_endhelprace;
	nannyFuncs[CON_EXDSCR]=con_nop;
	nannyFuncs[CON_EXTRA2]=con_nop;
	nannyFuncs[CON_HELPCLASS]=con_helpclass;
	nannyFuncs[CON_HELPRACE]=con_helprace;
	nannyFuncs[CON_HELPROLL]=con_helproll;
	nannyFuncs[CON_LDEAD]=con_nop;
	nannyFuncs[CON_MOB_EDITING]=con_nop;
	nannyFuncs[CON_NME]=con_nme;
	nannyFuncs[CON_NMECNF]=con_nmecnf;
	nannyFuncs[CON_OBJ_EDITING]=con_nop;
	nannyFuncs[CON_OBJ_FORGING]=con_nop;
	nannyFuncs[CON_PLYNG]=con_nop;
	nannyFuncs[CON_PWDCNF]=con_pwdcnf;
	nannyFuncs[CON_PWDGET]=con_pwdget;
	nannyFuncs[CON_PWDNCNF]=con_pwdncnf;
	nannyFuncs[CON_PWDNEW]=con_pwdnew;
	nannyFuncs[CON_PWDNRM]=con_pwdnrm;
	nannyFuncs[CON_PWDOK]=con_pwdok;
	nannyFuncs[CON_QCLASS]=con_qclass;
	nannyFuncs[CON_QDELETE]=con_nop;
	nannyFuncs[CON_QDELETE2]=con_nop;
	nannyFuncs[CON_QRACE]=con_qrace;
	nannyFuncs[CON_QROLL]=con_qroll;
	nannyFuncs[CON_QSEX]=con_qsex;
	nannyFuncs[CON_RACPAR]=con_nop;
	nannyFuncs[CON_REGISTER]=con_register;
	nannyFuncs[CON_RMOTD]=con_rmotd;
	nannyFuncs[CON_RNEWD]=con_rnewd;
	nannyFuncs[CON_SLCT]=con_slct;
	nannyFuncs[CON_STAT_LIST]=con_stat_list;
	nannyFuncs[CON_STAT_LISTV]=con_nop;
	nannyFuncs[CON_WIZLOCK]=con_wizlock;
	nannyFuncs[CON_WMOTD]=con_wmotd;
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data* d, char* arg) {
	d->currentInput.assign(arg);
	boost::algorithm::trim_all(d->currentInput);
	echoOn(d);
	bool moresteps=false;
	do {
		uint16_t index=static_cast<uint16_t>(STATE(d));
		try {
		nanny_func f=nannyFuncs.at(index);
		moresteps=f(d);
		}
		catch (std::out_of_range &e) {
			mudlog(LOG_SYSERR,"Invalid connection state, closing descriptor: %d %s",STATE(d),e.what());
			close_socket(d);
			moresteps=false;
		}
		// Gestione account: stati messi tutti all'inizio perché poi fanno fallback sulla procedura standard
	}
	while(moresteps);
}
void toonUpdate(const descriptor_data* d) {
	boost::format fmt(R"(UPDATE toon SET level=%d,lastlogin=now(),lasthost="%s" WHERE name="%s")");
	fmt % GetMaxLevel(d->character) % d->host % d->AccountData.choosen;
	try {
		DB* db=Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		db->execute(fmt.str());
		t.commit();
	}
	catch(odb::exception &e) {
		mudlog(LOG_SYSERR,"Db error while registering %s: %s",d->AccountData.choosen.c_str(),e.what());
	}
}
void show_class_selection(struct descriptor_data* d, int r) {
	int i=0;
	char buf[254],buf2[254];

	sprintf(buf, "\n\rSeleziona la classe di %s.\n\r\n\r",
			GET_NAME(d->character));
	SEND_TO_Q(buf, d);

	switch(r) {
	case RACE_ELVEN:
	case RACE_GOLD_ELF:
	case RACE_SEA_ELF:
	case RACE_HALF_ELVEN:
		for(i=0; elf_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)elf_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_WILD_ELF:
		for(i=0; wild_elf_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)wild_elf_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_DEEP_GNOME:
		for(i = 0; deep_gnome_class_choice[ i ] != 0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)deep_gnome_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_GNOME:
		for(i=0; gnome_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)gnome_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_DWARF:
		for(i=0; dwarf_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)dwarf_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_HALFLING:
		for(i=0; halfling_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)halfling_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_HUMAN:
		for(i=0; human_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)human_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_HALF_ORC:
		for(i=0; half_orc_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)half_orc_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_HALF_OGRE:
		for(i=0; half_ogre_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)half_ogre_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_HALF_GIANT:
		for(i=0; half_giant_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)half_giant_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_ORC:
		for(i=0; orc_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)orc_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_GOBLIN:
		for(i=0; goblin_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)goblin_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_DARK_ELF:
		for(i=0; dark_elf_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)dark_elf_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_DARK_DWARF:
		for(i=0; dark_dwarf_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)dark_dwarf_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	case RACE_TROLL:
		for(i=0; troll_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)troll_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;

	case RACE_DEMON:
		for(i=0; demon_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)demon_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;

	default:
		for(i=0; default_class_choice[i]!=0; i++) {
			sprintf(buf,"%d) ",i);
			sprintbit((unsigned)default_class_choice[i],pc_class_types, buf2);
			strcat(buf,buf2);
			strcat(buf,"\n\r");
			SEND_TO_Q(buf,d);
		} /* end for */
		break;
	} /* end switch */

	SEND_TO_Q("\n\r\n\rClasse (batti <?> per un aiuto, <b> per tornare indietro): ", d);
}

void show_race_choice(struct descriptor_data* d) {
	int ii,i=0;
	char buf[255],buf2[254];

	SEND_TO_Q("                               Limiti di livello\n\r",d);
	sprintf(buf,"%-4s %-15s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s\n\r",
			"#","Razza","ma","cl","wa","th","dr","mk","ba","so","pa","ra","ps");
	SEND_TO_Q(buf,d);

	while(race_choice[i]!=-1) {
		sprintf(buf,"%-3d) %-15s",i,RaceName[race_choice[i]]);

		/* show level limits */
		for(ii=0; ii<MAX_CLASS; ii++) {
			sprintf(buf2," %-3d",RacialMax[race_choice[i]][ii]);
			strcat(buf,buf2);
		} /* end for */

		strcat(buf,"\n\r");
		SEND_TO_Q(buf,d);
		i++;
	}

	SEND_TO_Q("  ma=magic user, cl=cleric, wa=warrior, th=thief, dr=druid, "
			  "mk=monk\n\r",d);
	SEND_TO_Q("  ba=barbarian, so=sorcerer, pa=paladin, ra=ranger,"
			  " ps=psionist\n\r\n\r",d);
	SEND_TO_Q("Razza (batti <?> per un aiuto): ", d);
}



#define A_LOG_NAME "affected.log"
#define A_MAX_LINES 5000
void check_affected(char* msg) {
	struct affected_type*  hjp;
	struct char_data* c;
	static FILE* f=NULL;
	static long lines=0;
	char* (b[5]);
	char buf[5000];
	int i,j;

	if(!f && !(f=fopen(A_LOG_NAME,"wr"))) {
		mudlog(LOG_ERROR,"%s:%s",A_LOG_NAME,strerror(errno));
		return;
	}

	if(lines>A_MAX_LINES) {
		rewind(f);
		for(i=0; i<5; i++) {
			b[i]=(char*)malloc(MAX_STRING_LENGTH);
			if(fgets(b[i], MAX_STRING_LENGTH, f)==NULL) {
				break;
			}
		}
		fclose(f);
		if(!(f=fopen(A_LOG_NAME,"wr"))) {
			mudlog(LOG_ERROR,"%s:%s",A_LOG_NAME,strerror(errno));
			return;
		}
		for(j=0; j<i; j++) {
			fputs(b[j],f);
			free(b[j]);
		}
		lines=0;
	}

	if(msg) {
		fprintf(f,"%s : ", msg);
	}
	else {
		fprintf(f,"check_affected: ");
	}

	for(c=character_list; c; c=c->next)
		if(c && c->affected)
			for(hjp=c->affected; hjp; hjp=hjp->next)
				if(hjp->type > MAX_EXIST_SPELL || hjp->type < 0) {
					sprintf(buf,"bogus hjp->type for (%s).", GET_NAME(c));
					fprintf(f,"%s",buf);
					/*          abort();    in test site this will be ok.. */
				}

	fprintf(f,"ok\n");
	fflush(f);
	lines++;

	return;
}


} // namespace Alarmud

