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
						send_to_char("Sei stato immobilizzato e non puoi fare "
									 "nulla!\n\r", ch);
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
	AddCommand("north", do_move, CMD_NORTH, POSITION_STANDING, TUTTI);
	AddCommand("east",  do_move, CMD_EAST,  POSITION_STANDING, TUTTI);
	AddCommand("south", do_move, CMD_SOUTH, POSITION_STANDING, TUTTI);
	AddCommand("west",  do_move, CMD_WEST,  POSITION_STANDING, TUTTI);
	AddCommand("up",    do_move, CMD_UP,    POSITION_STANDING, TUTTI);
	AddCommand("down",  do_move, CMD_DOWN,  POSITION_STANDING, TUTTI);
	AddCommand("enter", do_enter, CMD_ENTER, POSITION_STANDING, TUTTI);
	AddCommand("exits",do_exits,8,POSITION_RESTING,TUTTI);
	AddCommand("kiss",do_action,9,POSITION_RESTING,TUTTI);
	AddCommand("get", do_get, CMD_GET,POSITION_RESTING,1);
	AddCommand("drink",do_drink,11,POSITION_RESTING,1);
	AddCommand("eat",do_eat,12,POSITION_RESTING,1);
	AddCommand("wear",do_wear,13,POSITION_RESTING,TUTTI);
	AddCommand("wield",do_wield,14,POSITION_RESTING,1);
	AddCommand("look", do_look, CMD_LOOK, POSITION_RESTING, TUTTI);
	AddCommand("score",do_score,16,POSITION_DEAD,TUTTI);
	AddCommand("say", do_new_say, CMD_SAY, POSITION_RESTING, TUTTI);
	AddCommand("shout", do_shout, CMD_SHOUT,POSITION_RESTING,2);
	AddCommand("tell", do_tell, CMD_TELL, POSITION_RESTING, TUTTI);
	AddCommand("inventory",do_inventory,20,POSITION_DEAD,TUTTI);
	AddCommand("qui",do_qui,21,POSITION_DEAD,TUTTI);
	AddCommand("bounce",do_action,22,POSITION_STANDING,TUTTI);
	AddCommand("smile",do_action,23,POSITION_RESTING,TUTTI);
	AddCommand("dance",do_action,24,POSITION_STANDING,TUTTI);
	AddCommand("kill",do_kill,25,POSITION_FIGHTING,1);
	AddCommand("cackle",do_action,26,POSITION_RESTING,TUTTI);
	AddCommand("laugh",do_action,27,POSITION_RESTING,TUTTI);
	AddCommand("giggle",do_action,28,POSITION_RESTING,TUTTI);
	AddCommand("shake", do_action, CMD_SHAKE, POSITION_RESTING, TUTTI);
	AddCommand("puke",do_vomita,30,POSITION_RESTING,TUTTI);
	AddCommand("growl",do_action,31,POSITION_RESTING,TUTTI);
	AddCommand("scream",do_action,32,POSITION_RESTING,TUTTI);
	AddCommand("insult",do_insult,33,POSITION_RESTING,TUTTI);
	AddCommand("comfort",do_action,34,POSITION_RESTING,TUTTI);
	AddCommand("nod",do_action,35,POSITION_RESTING,TUTTI);
	AddCommand("sigh",do_action,36,POSITION_RESTING,TUTTI);
	AddCommand("sulk",do_action,37,POSITION_RESTING,TUTTI);
	AddCommand("help",do_help,38,POSITION_DEAD,TUTTI);
	AddCommand("who", do_who, CMD_WHO, POSITION_DEAD, TUTTI);
	AddCommand("emote",do_emote,40,POSITION_SLEEPING,TUTTI);
	AddCommand(":",do_emote,40,POSITION_SLEEPING,TUTTI);
	AddCommand(",",do_emote,177,POSITION_SLEEPING,TUTTI);
	AddCommand("echo",do_echo,41,POSITION_SLEEPING,1);
	AddCommand("stand",do_stand,42,POSITION_RESTING,TUTTI);
	AddCommand("sit",do_sit,43,POSITION_RESTING,TUTTI);
	AddCommand("rest",do_rest,44,POSITION_RESTING,TUTTI);
	AddCommand("sleep",do_sleep,45,POSITION_SLEEPING,TUTTI);
	AddCommand("wake",do_wake,46,POSITION_SLEEPING,TUTTI);
	AddCommand("force",do_force,47,POSITION_SLEEPING,MAESTRO_DEL_CREATO);
	AddCommand("transfer",do_trans,48,POSITION_SLEEPING,QUESTMASTER);
	AddCommand("hug",do_action,49,POSITION_RESTING,TUTTI);
	AddCommand("snuggle",do_action,50,POSITION_RESTING,TUTTI);
	AddCommand("cuddle",do_action,51,POSITION_RESTING,TUTTI);
	AddCommand("nuzzle",do_action,52,POSITION_RESTING,TUTTI);
	AddCommand("cry",do_action,53,POSITION_RESTING,TUTTI);
	AddCommand("news",do_news,54,POSITION_SLEEPING,TUTTI);
	AddCommand("equipment",do_equipment,55,POSITION_SLEEPING,TUTTI);
	AddCommand("buy", do_not_here, CMD_BUY, POSITION_STANDING, TUTTI);
	AddCommand("sell", do_not_here, CMD_SELL, POSITION_STANDING, TUTTI);
	AddCommand("value", do_value, CMD_VALUE, POSITION_RESTING, TUTTI);
	AddCommand("list", do_not_here, CMD_LIST, POSITION_STANDING, TUTTI);
	AddCommand("drop", do_drop, CMD_DROP, POSITION_RESTING, 1);
	AddCommand("goto",do_goto,61,POSITION_SLEEPING,IMMORTALE);
	AddCommand("weather",do_weather,62,POSITION_RESTING,TUTTI);
	AddCommand("read", do_read, CMD_READ, POSITION_RESTING, TUTTI);
	AddCommand("pour",do_pour,64,POSITION_STANDING,TUTTI);
	AddCommand("grab",do_grab,65,POSITION_RESTING,TUTTI);
	AddCommand("remove", do_remove, CMD_REMOVE, POSITION_RESTING, TUTTI);
	AddCommand("put",do_put,67,POSITION_RESTING,TUTTI);
	AddCommand("shutdow",do_shutdow,68,POSITION_DEAD,MAESTRO_DEI_CREATORI);
	AddCommand("save",do_save,69,POSITION_SLEEPING,TUTTI);
	AddCommand("hit",do_hit,70,POSITION_FIGHTING,1);
	AddCommand("string",do_string,71,POSITION_SLEEPING,DIO);
	AddCommand("give", do_give, CMD_GIVE, POSITION_RESTING, 1);
	AddCommand("quit",do_quit,73,POSITION_DEAD,TUTTI);
	AddCommand("wiznews",do_wiznews,74,POSITION_DEAD,IMMORTALE);
	AddCommand("guard",do_guard,75,POSITION_STANDING,1);
	AddCommand("time",do_time,76,POSITION_DEAD,TUTTI);
	AddCommand("oload",do_oload,77,POSITION_DEAD,QUESTMASTER);
	AddCommand("purge",do_purge,78,POSITION_DEAD,DIO_MINORE);
	AddCommand("shutdown", do_shutdown, 79, POSITION_DEAD,CREATORE);
	AddCommand("idea",do_action,80,POSITION_DEAD,TUTTI);
	AddCommand("typo",do_action,81,POSITION_DEAD,TUTTI);
	AddCommand("bug",do_action,82,POSITION_DEAD,TUTTI);
	AddCommand("whisper", do_whisper, CMD_WHISPER, POSITION_RESTING,TUTTI);
	AddCommand("cast", do_cast, CMD_CAST, POSITION_SITTING, 1);
	AddCommand("know", do_cast, CMD_SPELLID, POSITION_SITTING, 1);
	AddCommand("at",do_at,85,POSITION_DEAD,DIO);
	AddCommand("ask", do_ask, CMD_ASK, POSITION_RESTING,TUTTI);
	AddCommand("order",do_order,87,POSITION_RESTING,1);
	AddCommand("sip",do_sip, 88,POSITION_RESTING,TUTTI);
	AddCommand("taste",do_taste,89,POSITION_RESTING,TUTTI);
	AddCommand("snoop",do_snoop,90,POSITION_DEAD,QUESTMASTER);
	AddCommand("follow",do_follow,91,POSITION_RESTING,TUTTI);
	AddCommand("rent",do_not_here,CMD_RENT,POSITION_STANDING,1);
	AddCommand("offer",do_not_here,CMD_OFFER,POSITION_STANDING,1);
	//AddCommand("store",do_not_here,CMD_STORE,POSITION_STANDING,1);
	AddCommand("poke",do_action,94,POSITION_RESTING,TUTTI);
	AddCommand("advance",do_advance,95,POSITION_DEAD,MAESTRO_DEI_CREATORI);
	AddCommand("accuse",do_action,96,POSITION_SITTING,TUTTI);
	AddCommand("grin",do_action,97,POSITION_RESTING,TUTTI);
	AddCommand("bow",do_action,98,POSITION_STANDING,TUTTI);

	AddCommand("open",   do_open,   CMD_OPEN,   POSITION_SITTING, TUTTI);
	AddCommand("close",  do_close,  CMD_CLOSE,  POSITION_SITTING, TUTTI);
	AddCommand("lock",   do_lock,   CMD_LOCK,   POSITION_SITTING, TUTTI);
	AddCommand("unlock", do_unlock, CMD_UNLOCK, POSITION_SITTING, TUTTI);

	AddCommand("leave",do_leave,103,POSITION_STANDING,TUTTI);
	AddCommand("applaud",do_action,104,POSITION_RESTING,TUTTI);
	AddCommand("blush",do_action,105,POSITION_RESTING,TUTTI);
	AddCommand("burp",do_action,106,POSITION_RESTING,TUTTI);
	AddCommand("chuckle",do_action,107,POSITION_RESTING,TUTTI);
	AddCommand("nose",do_action,108,POSITION_RESTING,TUTTI);
	AddCommand("cough",do_action,109,POSITION_RESTING,TUTTI);
	AddCommand("curtsey",do_action,110,POSITION_STANDING,TUTTI);
	AddCommand("rise",do_action,111,POSITION_RESTING,DIO);
	AddCommand("flip",do_action,112,POSITION_STANDING,TUTTI);
	AddCommand("fondle",do_action,113,POSITION_RESTING,TUTTI);
	AddCommand("frown",do_action,114,POSITION_RESTING,TUTTI);
	AddCommand("gasp",do_action,115,POSITION_RESTING,TUTTI);
	AddCommand("glare", do_action, CMD_GLARE, POSITION_RESTING, TUTTI);
	AddCommand("groan",do_action,117,POSITION_RESTING,TUTTI);
	AddCommand("grope",do_action,118,POSITION_RESTING,TUTTI);
	AddCommand("hiccup",do_action,119,POSITION_RESTING,TUTTI);
	AddCommand("lick",do_action,120,POSITION_RESTING,TUTTI);
	AddCommand("love",do_action,121,POSITION_RESTING,TUTTI);
	AddCommand("moan",do_action,122,POSITION_RESTING,TUTTI);
	AddCommand("nibble",do_action,123,POSITION_RESTING,TUTTI);
	AddCommand("pout",do_action,124,POSITION_RESTING,TUTTI);
	AddCommand("purr",do_action,125,POSITION_RESTING,TUTTI);
	AddCommand("ruffle",do_action,126,POSITION_STANDING,TUTTI);
	AddCommand("shiver",do_action,127,POSITION_RESTING,TUTTI);
	AddCommand("shrug",do_action,128,POSITION_RESTING,TUTTI);
	AddCommand("sing",do_action,129,POSITION_RESTING,TUTTI);
	AddCommand("slap",do_action,130,POSITION_RESTING,TUTTI);
	AddCommand("smirk",do_action,131,POSITION_RESTING,TUTTI);
	AddCommand("support",do_support,CMD_SUPPORT,POSITION_STANDING,TUTTI);
	AddCommand("sneeze",do_action,133,POSITION_RESTING,TUTTI);
	AddCommand("snicker",do_action,134,POSITION_RESTING,TUTTI);
	AddCommand("sniff",do_action,135,POSITION_RESTING,TUTTI);
	AddCommand("snore",do_action,136,POSITION_SLEEPING,TUTTI);
	AddCommand("spit",do_action,137,POSITION_STANDING,TUTTI);
	AddCommand("squeeze",do_action,138,POSITION_RESTING,TUTTI);
	AddCommand("associa",do_associa,CMD_ASSOCIA,POSITION_STANDING,PRINCIPE);
	AddCommand("stare",do_action,139,POSITION_RESTING,TUTTI);
	AddCommand("associate",do_associa,CMD_ASSOCIA,POSITION_STANDING,PRINCIPE);
	AddCommand("thank",do_action,141,POSITION_RESTING,TUTTI);
	AddCommand("twiddle",do_action,142,POSITION_RESTING,TUTTI);
	AddCommand("wave",do_action,143,POSITION_RESTING,TUTTI);
	AddCommand("whistle",do_action,144,POSITION_RESTING,TUTTI);
	AddCommand("wiggle",do_action,145,POSITION_STANDING,TUTTI);
	AddCommand("wink",do_action,146,POSITION_RESTING,TUTTI);
	AddCommand("yawn",do_action,147,POSITION_RESTING,TUTTI);
	AddCommand("snowball",do_action,148,POSITION_STANDING,IMMORTALE);
	AddCommand("write", do_write, CMD_WRITE, POSITION_STANDING, 1);
	AddCommand("hold",do_grab,150,POSITION_RESTING,1);
	AddCommand("flee", do_flee, CMD_FLEE, POSITION_SITTING, 1);
	AddCommand("sneak",do_sneak,152,POSITION_STANDING,1);
	AddCommand("hide",do_hide,153,POSITION_RESTING,1);
	AddCommand("camouflage",do_hide,153,POSITION_STANDING,1);
	AddCommand("backstab",do_backstab,154,POSITION_STANDING,1);
	AddCommand("pick",do_pick,155,POSITION_STANDING,1);
	AddCommand("steal",do_steal,156,POSITION_STANDING,1);
	AddCommand("bash", do_bash, CMD_BASH, POSITION_FIGHTING, 1);
	AddCommand("rescue",do_rescue,158,POSITION_FIGHTING,1);
	AddCommand("kick",do_kick,159,POSITION_FIGHTING,1);
	AddCommand("french",do_action,160,POSITION_RESTING,ALLIEVO);
	AddCommand("comb",do_action,161,POSITION_RESTING,TUTTI);
	AddCommand("massage",do_action,162,POSITION_RESTING,TUTTI);
	AddCommand("tickle",do_action,163,POSITION_RESTING,TUTTI);
	AddCommand("practice", do_practice, CMD_PRACTICE, POSITION_RESTING, 1);
	AddCommand("practise", do_practice, CMD_PRACTICE, POSITION_RESTING, 1);
	AddCommand("pat", do_action, CMD_PAT, POSITION_RESTING, TUTTI);
	AddCommand("examine", do_examine, 166, POSITION_RESTING, TUTTI);
	AddCommand("take", do_get, CMD_TAKE, POSITION_RESTING, 1);  /* TAKE */
	AddCommand("info",do_info,168,POSITION_SLEEPING,TUTTI);
	AddCommand("'", do_new_say, CMD_SAY_APICE, POSITION_RESTING, TUTTI);
	AddCommand("curse",do_action,171,POSITION_RESTING,5);
	AddCommand("use", do_use, CMD_USE, POSITION_SITTING, 1);
	AddCommand("where",do_where,173,POSITION_DEAD,1);
	AddCommand("levels",do_levels,174,POSITION_DEAD,TUTTI);
	AddCommand("register",do_register,175,POSITION_DEAD,TUTTI);

	AddCommand("pray",do_pray,176,POSITION_SITTING,1);

	AddCommand("beg",do_action,178,POSITION_RESTING,TUTTI);
	AddCommand("bleed",do_not_here,179,POSITION_RESTING,TUTTI);
	AddCommand("cringe",do_action,180,POSITION_RESTING,TUTTI);
	AddCommand("daydream",do_action,181,POSITION_SLEEPING,TUTTI);
	AddCommand("fume",do_action,182,POSITION_RESTING,TUTTI);
	AddCommand("grovel",do_action,183,POSITION_RESTING,TUTTI);
	AddCommand("hop",do_action,184,POSITION_RESTING,TUTTI);
	AddCommand("nudge",do_action,185,POSITION_RESTING,TUTTI);
	AddCommand("peer",do_action,186,POSITION_RESTING,TUTTI);
	AddCommand("point",do_action,187,POSITION_RESTING,TUTTI);
	AddCommand("ponder",do_action,188,POSITION_RESTING,TUTTI);
	AddCommand("punch",do_action,189,POSITION_RESTING,TUTTI);
	AddCommand("snarl",do_action,190,POSITION_RESTING,TUTTI);
	AddCommand("spank",do_action,191,POSITION_RESTING,TUTTI);
	AddCommand("steam",do_action,192,POSITION_RESTING,TUTTI);
	AddCommand("tackle",do_action,193,POSITION_RESTING,TUTTI);
	AddCommand("taunt",do_action,194,POSITION_RESTING,TUTTI);
	AddCommand("think",do_commune,195,POSITION_RESTING,IMMORTALE);
	AddCommand("\"", do_commune, 195, POSITION_RESTING, IMMORTALE);
	AddCommand("?",do_commune,CMD_THINK_SUPERNI,POSITION_RESTING,CREATORE);
	AddCommand("whine",do_action,196,POSITION_RESTING,TUTTI);
	AddCommand("worship",do_action,197,POSITION_RESTING,TUTTI);
	AddCommand("yodel",do_action,198,POSITION_RESTING,TUTTI);
	AddCommand("brief",do_brief,199,POSITION_DEAD,TUTTI);
	AddCommand("wizlist",do_wizlist,200,POSITION_DEAD,TUTTI);
	AddCommand("consider",do_consider,201,POSITION_RESTING,TUTTI);
	AddCommand("group",do_group,202,POSITION_RESTING,1);
	AddCommand("restore",do_restore,203,POSITION_DEAD,MAESTRO_DEGLI_DEI);
	AddCommand("return",do_return,204,POSITION_RESTING,TUTTI);
	AddCommand("switch",do_switch,205,POSITION_DEAD,CREATORE);
	AddCommand("quaff",do_quaff,206,POSITION_RESTING,TUTTI);
	AddCommand("recite",do_recite,207,POSITION_STANDING,TUTTI);
	AddCommand("users",do_users,208,POSITION_DEAD,DIO_MINORE);
	AddCommand("pose",do_pose,209,POSITION_STANDING,TUTTI);
	AddCommand("noshout",do_noshout,210,POSITION_SLEEPING,IMMORTALE);
	AddCommand("wizhelp",do_wizhelp,211,POSITION_SLEEPING,DIO_MINORE);
	AddCommand("credits",do_credits,212,POSITION_DEAD,TUTTI);
	AddCommand("compact",do_compact,213,POSITION_DEAD,TUTTI);
	AddCommand("daimoku",do_daimoku,CMD_DAIMOKU,POSITION_SLEEPING,1);
	AddCommand("forge",do_forge,CMD_FORGE,POSITION_STANDING,1);
	AddCommand("slay",do_slay,216,POSITION_FIGHTING,DIO_MINORE);
	AddCommand("wimpy",do_wimp,217,POSITION_DEAD,TUTTI);
	AddCommand("junk",do_junk,218,POSITION_RESTING,1);
	AddCommand("deposit", do_not_here, CMD_DEPOSIT, POSITION_RESTING, 1);
	AddCommand("withdraw", do_not_here, CMD_WITHDRAW, POSITION_RESTING, 1);
	AddCommand("balance", do_not_here, CMD_BALANCE, POSITION_RESTING, 1);
	AddCommand("nohassle",do_nohassle,222,POSITION_DEAD,DIO);
	AddCommand("system",do_system,223,POSITION_DEAD,QUESTMASTER);
	AddCommand("pull", do_open_exit, CMD_PULL, POSITION_STANDING,1);
	AddCommand("stealth",do_stealth,225,POSITION_DEAD,DIO);
	AddCommand("edit",do_edit,226,POSITION_DEAD,MAESTRO_DEGLI_DEI);
	AddCommand("@",do_set,227,POSITION_DEAD,MAESTRO_DEI_CREATORI);
	AddCommand("rsave",do_rsave,228,POSITION_DEAD,MAESTRO_DEGLI_DEI);
	AddCommand("rload",do_rload,229,POSITION_DEAD,MAESTRO_DEGLI_DEI);
	AddCommand("track",do_track,230,POSITION_DEAD,1);
	AddCommand("wizlock",do_wizlock,231,POSITION_DEAD,MAESTRO_DEI_CREATORI);
	AddCommand("highfive",do_highfive,232,POSITION_DEAD,TUTTI);
	AddCommand("title",do_title,233,POSITION_DEAD,INIZIATO-1);
	AddCommand("whozone", do_who, CMD_WHOZONE, POSITION_DEAD, TUTTI);
	AddCommand("assist",do_assist,235,POSITION_FIGHTING,1);
	AddCommand("attribute",do_attribute,236,POSITION_DEAD,5);
	AddCommand("world",do_world,237,POSITION_DEAD,TUTTI);
	AddCommand("allspells",do_spells,238,POSITION_DEAD,TUTTI);
	AddCommand("breath",do_breath,239,POSITION_FIGHTING,1);
	AddCommand("show",do_show,240,POSITION_DEAD,DIO_MINORE);
	AddCommand("debug",do_debug,241,POSITION_DEAD,MAESTRO_DEI_CREATORI);
	AddCommand("invisible",do_invis,242,POSITION_DEAD,IMMORTALE);
	AddCommand("gain", do_gain, CMD_GAIN, POSITION_DEAD, 1);

	AddCommand("mload",do_mload,244,POSITION_DEAD,DIO);

	AddCommand("disarm",do_disarm,245,POSITION_FIGHTING,1);
	AddCommand("bonk",do_action,246,POSITION_SITTING,3);
	AddCommand("chpwd",do_passwd,247,POSITION_SITTING,MAESTRO_DEI_CREATORI);
	AddCommand("passwd",do_passwd,247,POSITION_SITTING,MAESTRO_DEI_CREATORI);
	AddCommand("fill",do_not_here,248,POSITION_SITTING,TUTTI);
	AddCommand("mantra",do_mantra,CMD_MANTRA,POSITION_SITTING,TUTTI);
	AddCommand("shoot",do_fire,250,POSITION_STANDING,  1);
	AddCommand("fire",do_fire,250, POSITION_DEAD,TUTTI);
	AddCommand("silence",do_silence,251,POSITION_STANDING, DIO);

	AddCommand("teams", do_not_here, CMD_TEAMS, POSITION_STANDING, MAESTRO_DEL_CREATO);
	AddCommand("player", do_not_here, CMD_PLAYER, POSITION_STANDING,MAESTRO_DEL_CREATO);

	AddCommand("create",do_create,254,POSITION_STANDING, QUESTMASTER);
	AddCommand("bamfin",do_bamfin,255,POSITION_STANDING,IMMORTALE);
	AddCommand("bamfout",do_bamfout,256,POSITION_STANDING, IMMORTALE);
	AddCommand("vis",do_invis,257,POSITION_RESTING,  TUTTI);
	AddCommand("doorbash", do_doorbash, CMD_DOORBASH, POSITION_STANDING, 1);
	AddCommand("mosh",do_action,259,POSITION_FIGHTING, 1);

	/* alias commands */
	AddCommand("alias",do_alias,260,POSITION_SLEEPING, 1);
	AddCommand("1", do_alias,261,POSITION_DEAD, 1);
	AddCommand("2", do_alias,262,POSITION_DEAD, 1);
	AddCommand("3", do_alias,263,POSITION_DEAD, 1);
	AddCommand("4", do_alias,264,POSITION_DEAD, 1);
	AddCommand("5", do_alias,265,POSITION_DEAD, 1);
	AddCommand("6", do_alias,266,POSITION_DEAD, 1);
	AddCommand("7", do_alias,267,POSITION_DEAD, 1);
	AddCommand("8", do_alias,268,POSITION_DEAD, 1);
	AddCommand("9", do_alias,269,POSITION_DEAD, 1);
	AddCommand("0", do_alias,270,POSITION_DEAD, 1);

	AddCommand("swim", do_swim,271,POSITION_STANDING, 1);
	AddCommand("spy", do_spy,272,POSITION_STANDING, 1);
	AddCommand("springleap",do_springleap,273,POSITION_RESTING, 1);
	AddCommand("quivering palm",do_quivering_palm,274,POSITION_FIGHTING, INIZIATO-1);
	AddCommand("feign death",do_feign_death,275,POSITION_FIGHTING, 1);
	AddCommand("mount",do_mount,276,POSITION_STANDING, 1);
	AddCommand("dismount",do_mount,277,POSITION_MOUNTED, 1);
	AddCommand("ride",do_mount,278,POSITION_STANDING, 1);
	AddCommand("sign",do_sign,279, POSITION_RESTING, 1);

	/* had to put this here BEFORE setsev so it would get this and not setsev */
	AddCommand("set",do_set_flags,280,POSITION_DEAD,TUTTI);

	AddCommand("first aid",do_first_aid,281, POSITION_RESTING, 1);
	AddCommand("log",do_set_log,282, POSITION_DEAD, MAESTRO_DEL_CREATO);
	AddCommand("recall", do_cast, CMD_RECALL, POSITION_SITTING, 1);
	AddCommand("reload",reboot_text,284, POSITION_DEAD, MAESTRO_DEI_CREATORI);
	AddCommand("event",do_event,285, POSITION_DEAD, MAESTRO_DEI_CREATORI);
	AddCommand("disguise",do_disguise,286, POSITION_STANDING, 1);
	AddCommand("climb",do_climb,287, POSITION_STANDING,1);
	AddCommand("beep",do_beep,288, POSITION_DEAD, DIO_MINORE);
	AddCommand("bite",do_action,289, POSITION_RESTING, 1);
	AddCommand("redit", do_redit, 290, POSITION_SLEEPING, MAESTRO_DEGLI_DEI);
	AddCommand("display", do_display, 291, POSITION_SLEEPING, 1);
	AddCommand("resize", do_resize, 292, POSITION_SLEEPING, 1);
	AddCommand("ripudia",do_ripudia,CMD_RIPUDIA, POSITION_STANDING,VASSALLO);
	AddCommand("#", do_cset, 294, POSITION_DEAD, QUESTMASTER);
	AddCommand("spell", do_stat, 295, POSITION_DEAD, DIO_MINORE);
	AddCommand("name", do_stat, 296, POSITION_DEAD, DIO_MINORE);
	AddCommand("stat", do_stat, 297, POSITION_DEAD, MAESTRO_DEGLI_DEI);
	AddCommand("immortali", do_immort, 298, POSITION_DEAD, 1);
	AddCommand("auth", do_auth, 299, POSITION_SLEEPING, MAESTRO_DEGLI_DEI);
	AddCommand("sacrifice", do_not_here, 300, POSITION_STANDING, MEDIUM);
	AddCommand("nogossip",do_plr_nogossip,301,POSITION_RESTING,TUTTI);
	AddCommand("gossip", do_gossip, CMD_GOSSIP, POSITION_RESTING,TUTTI);
	AddCommand("noauction",do_plr_noauction,303,POSITION_RESTING,TUTTI);
	AddCommand("auction", do_auction,CMD_AUCTION,POSITION_RESTING,ALLIEVO);
	AddCommand("discon",do_disconnect,305,POSITION_RESTING,DIO);
	AddCommand("freeze",do_freeze,306,POSITION_SLEEPING,QUESTMASTER);
	AddCommand("drain",do_drainlevel,307,POSITION_SLEEPING,MAESTRO_DEI_CREATORI);
	AddCommand("oedit",do_oedit,308,POSITION_DEAD,QUESTMASTER);
	AddCommand("report",do_report,309,POSITION_RESTING,1);
	AddCommand("interven",do_god_interven,310,POSITION_DEAD,MAESTRO_DEI_CREATORI);
	AddCommand("gtell", do_gtell, CMD_GTELL, POSITION_SLEEPING, 1);
	AddCommand("raise",do_action,312,POSITION_RESTING,1);
	AddCommand("tap",do_action,313,POSITION_STANDING,1);
	AddCommand("liege",do_action,314,POSITION_RESTING,1);
	AddCommand("sneer",do_action,315,POSITION_RESTING,1);
	AddCommand("howl",do_action,316,POSITION_RESTING,1);
	AddCommand("kneel", do_action, CMD_KNEEL, POSITION_STANDING, 1);
	AddCommand("finger",do_finger,318,POSITION_RESTING,1);
	AddCommand("pace",do_action,319,POSITION_STANDING,1);
	AddCommand("tongue",do_action,320,POSITION_RESTING,1);
	AddCommand("flex",do_action,321,POSITION_STANDING,1);
	AddCommand("ack",do_action,322,POSITION_RESTING,1);
	AddCommand("ckeq",do_stat,CMD_CKEQ,POSITION_RESTING,MAESTRO_DEGLI_DEI);
	AddCommand("caress",do_action,324,POSITION_RESTING,1);
	AddCommand("cheer",do_action,325,POSITION_RESTING,1);
	AddCommand("jump",do_action,326,POSITION_STANDING,1);

	AddCommand("join", do_action, CMD_JOIN, POSITION_RESTING, 1);

	AddCommand("split",do_split,328,POSITION_RESTING,1);
	AddCommand("berserk",do_berserk,329,POSITION_FIGHTING,1);
	AddCommand("tan",do_tan,330,POSITION_STANDING,1);

	AddCommand("memorize",do_memorize,331,POSITION_RESTING,1);

	AddCommand("find",do_find,332,POSITION_STANDING,1);
	AddCommand("bellow",do_bellow,333,POSITION_FIGHTING,1);
	AddCommand("carve",do_carve,335,POSITION_STANDING,1);
	AddCommand("nuke",do_nuke,336,POSITION_DEAD,MAESTRO_DEI_CREATORI);
	AddCommand("skills",do_show_skill,337,POSITION_SLEEPING,TUTTI);

	AddCommand("doorway",do_doorway,338,POSITION_STANDING,TUTTI);
	AddCommand("portal",do_psi_portal,339,POSITION_STANDING,TUTTI);
	AddCommand("summon",do_mindsummon,340,POSITION_STANDING,TUTTI);
	AddCommand("canibalize",do_canibalize,341,POSITION_STANDING,TUTTI);
	AddCommand("cannibalize",do_canibalize,341,POSITION_STANDING,TUTTI);
	AddCommand("flame",do_flame_shroud,342,POSITION_STANDING,TUTTI);
	AddCommand("aura",do_aura_sight,343,POSITION_RESTING,TUTTI);
	AddCommand("great",do_great_sight,344,POSITION_RESTING,TUTTI);
	AddCommand("psionic invisibility",do_invisibililty,345,POSITION_STANDING,TUTTI);
	AddCommand("blast",do_blast,346,POSITION_FIGHTING,TUTTI);

	AddCommand("medit",do_medit,347,POSITION_DEAD,QUESTMASTER);

	AddCommand("hypnotize",do_hypnosis,348,POSITION_STANDING,TUTTI);
	AddCommand("scry",do_scry,349,POSITION_RESTING,TUTTI);
	AddCommand("adrenalize",do_adrenalize,350,POSITION_STANDING,TUTTI);
	AddCommand("brew",do_brew,351,POSITION_STANDING,TUTTI);
	AddCommand("meditate",do_meditate,352,POSITION_RESTING,TUTTI);
	AddCommand("forcerent",do_force_rent,353,POSITION_DEAD,QUESTMASTER);
	AddCommand("warcry",do_holy_warcry,354,POSITION_FIGHTING,TUTTI);
	AddCommand("lay on hands",do_lay_on_hands,355,POSITION_RESTING,TUTTI);
	AddCommand("blessing",do_blessing,356,POSITION_STANDING,TUTTI);
	AddCommand("heroic",do_heroic_rescue,357,POSITION_FIGHTING,TUTTI);
	AddCommand("scan",do_scan,358,POSITION_STANDING,DIO_MINORE);
	AddCommand("shield",do_psi_shield,359,POSITION_STANDING,TUTTI);
	AddCommand("notell",do_plr_notell,360,POSITION_DEAD,TUTTI);
	AddCommand("commands",do_command_list,361,POSITION_DEAD,TUTTI);
	AddCommand("ghost",do_ghost,362,POSITION_DEAD,MAESTRO_DEL_CREATO);
	AddCommand("speak",do_speak,363,POSITION_DEAD,TUTTI);
	AddCommand("setsev",do_setsev,364, POSITION_DEAD, DIO_MINORE);
	AddCommand("esp",do_esp,365, POSITION_STANDING,TUTTI);
	AddCommand("mail",do_not_here,366, POSITION_STANDING,ALLIEVO-1);
	AddCommand("check",do_not_here,367, POSITION_STANDING,TUTTI);
	AddCommand("receive",do_not_here,368, POSITION_STANDING,TUTTI);
	AddCommand("telepathy",do_telepathy,369, POSITION_RESTING,TUTTI);
	AddCommand("mind", do_cast, CMD_MIND, POSITION_SITTING,TUTTI);

	AddCommand("twist", do_open_exit, CMD_TWIST, POSITION_STANDING,TUTTI);
	AddCommand("turn", do_open_exit, CMD_TURN, POSITION_STANDING,TUTTI);
	AddCommand("lift", do_open_exit, CMD_LIFT, POSITION_STANDING,TUTTI);
	AddCommand("push", do_open_exit, CMD_PUSH, POSITION_STANDING,TUTTI);

	/*AddCommand("zload", do_zload,375, POSITION_STANDING,MAESTRO_DEGLI_DEI); */
	AddCommand("zsave", do_zsave,376, POSITION_STANDING,MAESTRO_DEGLI_DEI);
	AddCommand("zclean",do_zclean,377, POSITION_STANDING,MAESTRO_DEGLI_DEI);
	AddCommand("wrebuild",do_WorldSave,378, POSITION_STANDING,IMMENSO);
	AddCommand("wreset",do_wreset, CMD_WRESET, POSITION_STANDING,CREATORE);  // SALVO aggiunto comando wreset
	AddCommand("gwho",list_groups,379, POSITION_DEAD,TUTTI);

	AddCommand("mforce",do_mforce,380, POSITION_DEAD,DIO_MINORE);    /* CREATOR */
	AddCommand("clone",do_clone,381, POSITION_DEAD,MAESTRO_DEGLI_DEI+2);
	AddCommand("bodyguard",do_bodyguard,CMD_BODYGUARD, POSITION_STANDING,MEDIUM);
	AddCommand("throw",do_throw,383, POSITION_SITTING,TUTTI);
	AddCommand("run", do_run, CMD_RUN, POSITION_STANDING,TUTTI);
	AddCommand("notch",do_weapon_load,385, POSITION_RESTING,TUTTI);
	AddCommand("load",do_weapon_load,385, POSITION_RESTING,TUTTI);

	AddCommand("spot",do_scan,387, POSITION_STANDING,TUTTI);
	AddCommand("view",do_viewfile,388, POSITION_DEAD,DIO);
	AddCommand("afk",do_set_afk,389, POSITION_DEAD,1);

	AddCommand("stopfight", do_stopfight, CMD_STOPFIGHT, POSITION_FIGHTING,1);
	AddCommand("principi",do_prince,CMD_PRINCE,POSITION_RESTING,TUTTI);
	AddCommand("tspy",do_tspy,CMD_TSPY,POSITION_STANDING,1);
	AddCommand("bid", do_auction, CMD_BID, POSITION_RESTING, ALLIEVO);
	AddCommand("eavesdrop", do_eavesdrop, CMD_EAVESDROP, POSITION_STANDING,INIZIATO);
	AddCommand("pquest", do_pquest, CMD_PQUEST, POSITION_STANDING,QUESTMASTER);
	AddCommand("parry", do_parry, CMD_PARRY, POSITION_RESTING, TUTTI);
	/* Liberi 397-399 */

	/* lots of Socials */
	AddCommand("adore",do_action,400,POSITION_RESTING,TUTTI);
	AddCommand("agree",do_action,401,POSITION_RESTING,TUTTI);
	AddCommand("bleed",do_action,402,POSITION_RESTING,TUTTI);
	AddCommand("blink",do_action,403,POSITION_RESTING,TUTTI);
	AddCommand("blow",do_action,404,POSITION_RESTING,TUTTI);
	AddCommand("blame",do_action,405,POSITION_RESTING,TUTTI);
	AddCommand("bark",do_action,406,POSITION_RESTING,TUTTI);
	AddCommand("bhug",do_action,407,POSITION_RESTING,TUTTI);
	AddCommand("bcheck",do_action,408,POSITION_RESTING,TUTTI);
	AddCommand("boast",do_action,409,POSITION_RESTING,TUTTI);
	AddCommand("chide",do_action,410,POSITION_RESTING,TUTTI);
	AddCommand("compliment",do_action,411,POSITION_RESTING,TUTTI);
	AddCommand("ceyes",do_action,412,POSITION_RESTING,TUTTI);
	AddCommand("cears",do_action,413,POSITION_RESTING,TUTTI);
	AddCommand("cross",do_action,414,POSITION_RESTING,TUTTI);
	AddCommand("console",do_action,415,POSITION_RESTING,TUTTI);
	AddCommand("calm",do_action,416,POSITION_RESTING,TUTTI);
	AddCommand("cower",do_action,417,POSITION_RESTING,TUTTI);
	AddCommand("confess",do_action,418,POSITION_RESTING,TUTTI);
	AddCommand("drool",do_action,419,POSITION_RESTING,TUTTI);
	AddCommand("grit",do_action,420,POSITION_RESTING,TUTTI);
	AddCommand("greet",do_action,421,POSITION_RESTING,TUTTI);
	AddCommand("gulp",do_action,422,POSITION_RESTING,TUTTI);
	AddCommand("gloat",do_action,423,POSITION_RESTING,TUTTI);
	AddCommand("gaze",do_action,424,POSITION_RESTING,TUTTI);
	AddCommand("hum",do_action,425,POSITION_RESTING,TUTTI);
	AddCommand("hkiss",do_action,426,POSITION_RESTING,TUTTI);
	AddCommand("ignore",do_action,427,POSITION_RESTING,TUTTI);
	AddCommand("interrupt",do_action,428,POSITION_RESTING,TUTTI);
	AddCommand("knock",do_action,429,POSITION_RESTING,TUTTI);
	AddCommand("listen",do_action,430,POSITION_RESTING,TUTTI);
	AddCommand("muse",do_action,431,POSITION_RESTING,TUTTI);
	AddCommand("pinch",do_action,432,POSITION_RESTING,TUTTI);
	AddCommand("praise",do_action,433,POSITION_RESTING,TUTTI);
	AddCommand("plot",do_action,434,POSITION_RESTING,TUTTI);
	AddCommand("pie",do_action,435,POSITION_RESTING,TUTTI);
	AddCommand("setalign",do_setalign,436,POSITION_RESTING,TUTTI);
	AddCommand("pant",do_action,437,POSITION_RESTING,TUTTI);
	AddCommand("rub",do_action,438,POSITION_RESTING,TUTTI);
	AddCommand("roll",do_action,439,POSITION_RESTING,TUTTI);
	AddCommand("recoil",do_action,440,POSITION_RESTING,TUTTI);
	AddCommand("roar",do_action,441,POSITION_RESTING,TUTTI);
	AddCommand("relax",do_action,442,POSITION_RESTING,TUTTI);
	AddCommand("snap",do_action,443,POSITION_RESTING,TUTTI);
	AddCommand("strut",do_action,444,POSITION_RESTING,TUTTI);
	AddCommand("stroke",do_action,445,POSITION_RESTING,TUTTI);
	AddCommand("stretch",do_action,446,POSITION_RESTING,TUTTI);
	AddCommand("swave",do_action,447,POSITION_RESTING,TUTTI);
	AddCommand("sob",do_action,448,POSITION_RESTING,TUTTI);
	AddCommand("scratch",do_action,449,POSITION_RESTING,TUTTI);
	AddCommand("squirm",do_action,450,POSITION_RESTING,TUTTI);
	AddCommand("strangle",do_action,451,POSITION_RESTING,TUTTI);
	AddCommand("scowl",do_action,452,POSITION_RESTING,TUTTI);
	AddCommand("shudder",do_action,453,POSITION_RESTING,TUTTI);
	AddCommand("strip",do_action,454,POSITION_RESTING,TUTTI);
	AddCommand("scoff",do_action,455,POSITION_RESTING,TUTTI);
	AddCommand("salute",do_action,456,POSITION_RESTING,TUTTI);
	AddCommand("scold",do_action,457,POSITION_RESTING,TUTTI);
	AddCommand("stagger",do_action,458,POSITION_RESTING,TUTTI);
	AddCommand("toss",do_action,459,POSITION_RESTING,TUTTI);
	AddCommand("twirl",do_action,460,POSITION_RESTING,TUTTI);
	AddCommand("toast",do_action,461,POSITION_RESTING,TUTTI);
	AddCommand("tug",do_action,462,POSITION_RESTING,TUTTI);
	AddCommand("touch",do_action,463,POSITION_RESTING,TUTTI);
	AddCommand("tremble",do_action,464,POSITION_RESTING,TUTTI);
	AddCommand("twitch",do_action,465,POSITION_RESTING,TUTTI);
	AddCommand("whimper",do_action,466,POSITION_RESTING,TUTTI);
	AddCommand("whap",do_action,467,POSITION_RESTING,TUTTI);
	AddCommand("wedge",do_action,468,POSITION_RESTING,TUTTI);
	AddCommand("apologize",do_action,469,POSITION_RESTING,TUTTI);

	/* Liberi 470-479 (-488)*/

	/* DIMD stuff
	 AddCommand("dmanage",do_dmanage,480,POSITION_RESTING,MAESTRO_DEL_CREATO);
	 AddCommand("drestrict",do_drestrict,481,POSITION_RESTING,DIO_MINORE);
	 AddCommand("dlink",do_dgossip,482,POSITION_RESTING,DIO_MINORE);
	 AddCommand("dunlink",do_dgossip,483,POSITION_RESTING,DIO_MINORE);
	 AddCommand("dlist",do_dlist,484,POSITION_RESTING,2);
	 AddCommand("dwho",do_dwho,485,POSITION_RESTING,2);
	 AddCommand("dgossip",do_dgossip,486,POSITION_RESTING,MAESTRO_DEL_CREATO);
	 AddCommand("dtell",do_dtell,487,POSITION_RESTING,MAESTRO_DEL_CREATO);
	 AddCommand("dthink",do_dthink,488,POSITION_RESTING,MAESTRO_DEL_CREATO);
	 */

	AddCommand("sending",do_sending,489,POSITION_STANDING,TUTTI);
	AddCommand("messenger",do_sending,490,POSITION_STANDING,TUTTI);
	AddCommand("prompt", do_set_prompt,491,POSITION_RESTING,TUTTI);

	AddCommand("ooedit",do_ooedit,492,POSITION_DEAD,MAESTRO_DEGLI_DEI);
	AddCommand("whois",do_whois,493,POSITION_DEAD,ALLIEVO-1);
	AddCommand("osave",do_osave,494,POSITION_DEAD,QUESTMASTER);
	AddCommand("dig", do_open_exit, CMD_DIG, POSITION_STANDING, TUTTI);
	AddCommand("cut", do_open_exit, CMD_SCYTHE, POSITION_STANDING, TUTTI);
	AddCommand("status", do_status, CMD_STATUS, POSITION_DEAD, TUTTI);
	AddCommand("showsk", do_showskills, CMD_SHOWSKILLS, POSITION_DEAD,
			   DIO);
	AddCommand("resetsk", do_resetskills, CMD_RESETSKILLS, POSITION_DEAD,
			   MAESTRO_DEL_CREATO);
	AddCommand("setsk", do_setskill, CMD_SETSKILLS, POSITION_DEAD,
			   MAESTRO_DEI_CREATORI);
//AddCommand("perdono", do_perdono, CMD_PERDONO, POSITION_STANDING, TUTTI ); //FLYP 2003 Perdono
	AddCommand("immolate", do_immolation, CMD_IMMOLATION, POSITION_FIGHTING, TUTTI); // Flyp 20180129: demon can sacrifice life for mana
	AddCommand("SetTest",do_imptest,CMD_IMPTEST,POSITION_DEAD,MAESTRO_DEL_CREATO);
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


int parse_name(const char* arg, char* name) {
	int i;

	/* skip whitespaces */
	for(; isspace(*arg); arg++);
	for(i = 0; (*name = *arg) != 0; arg++, i++, name++) {
		if((*arg <0) || !isalpha(*arg)) {
			// If the current char is a '@' we are in account mode
#if ACCOUNT_MODE
			if(*arg=='@') {
				return 2;
			}
#endif
			return 1 ;
		}
	}

	if(!i or i >15) {
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
	char buf[ 254 ];
	char temp[254];
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
		sprintf(buf,"Ecco le stats risultanti dalla tua scelta:\n\r");
		sprintf(temp,"%s%2d %2d %2d %2d %2d %2d %s\n\r",buf,FO+STAT_MIN_VAL,
				IN+STAT_MIN_VAL,
				SA+STAT_MIN_VAL,
				AG+STAT_MIN_VAL,
				CO+STAT_MIN_VAL,
				CA+STAT_MIN_VAL,(!*c7?"\0":"piu' la randomizzazione (-1/+1)"));
		if(t<STAT_MAX_SUM)
			sprintf(buf,"%sATTENZIONE. Hai usato solo %2d dei %2d disponibili\n\r",
					temp,t,STAT_MAX_SUM);
		SEND_TO_Q(buf,d);
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


void toonList(struct descriptor_data* d,const string &optional_message="") {
	const user &ac=d->AccountData;
	string message(optional_message);
	message.append("Scegli un personagggio\r\n").append(" q. Quit\n\r 0. Crea un nuovo pg o usane uno non ancora connesso all'account\r\n");
	if (ac.id) { short n=0;
		constexpr int nlen=5;
		char order[nlen]="";
		toonRows r=Sql::getAll<toon>(toonQuery::owner_id==ac.id);
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
	mudlog(LOG_ALWAYS,"ECHO ON");
	write(d->descriptor, echo_on, 6);
}
void echoOff(struct descriptor_data* d) {
	mudlog(LOG_ALWAYS,"ECHO OFF");
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
	string email(arg);
	boost::replace_all(email," ","");
	d->AccountData.email=email;
	email.insert(0,"Benvenuto ").append(". ").append("Digita la tua password per favore (o b per ricominciare): ");
	SEND_TO_Q(email.c_str(),d);
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
	user &ac=d->AccountData;
	ac.id=0;
	mudlog(LOG_CONNECT,"Current mail: %s Choosen: %s",ac.email.c_str(),ac.choosen.c_str());
	ac.authorized=false;
	userPtr u=Sql::getOne<user>(userQuery::email==ac.email);
	if (u) {
		ac.password.assign(u->password);
		ac.level=u->level;
		ac.registered=u->registered;
		ac.ptr=u->ptr;
		ac.id=u->id;
		if (u->nickname.empty()) {
			ac.nickname=u->email;
		}
		else {
			ac.nickname=u->email;
		}
	}
	const char* check=ac.password.c_str();
	if(u) {
		mudlog(LOG_CONNECT,"Db: %s Typed: %s",check,crypt(arg,check));
	}
	if(u and !strcmp(crypt(arg,check),check)) {

		if (PORT==DEVEL_PORT and ac.level<52) {
			mudlog(LOG_CONNECT,"%s level %d attempted to access devel",ac.email,ac.level);
			FLUSH_TO_Q("Al server di sviluppo possono accedere solo gli immortali",d);
			close_socket(d);
			return false;
		}
		if(PORT==MASTER_PORT and ac.level<52 and !ac.ptr) {
			mudlog(LOG_CONNECT,"%s level %d ptr %s attempted to access master",ac.email,ac.level,(ac.ptr?"ON":"OFF"));
			FLUSH_TO_Q("Per accedere al server di test devi chiedere l'autorizzazione",d);
			close_socket(d);
			return false;
		}
		ac.authorized=true;
		string message("Benvenuto ");
		message.append(ac.nickname).append("\r\n");
		STATE(d)=CON_ACCOUNT_TOON;
		mudlog(LOG_CONNECT,"Succesfull connection for %s",ac.email.c_str());
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
	mudlog(LOG_SYSERR,"Called nop in nanny for : %s ",G::translate(STATE(d)));
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
		reset_char(d->character);
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
			if(GetMaxLevel(d->character) < DIO_MINORE) {
				/* Per gli IMMORTALI che rentavano ad Asgaard, gli tolgo
				la start room cosi' gli viene calcolata di nuovo */
				if(d->character->specials.start_room == 1000 && IS_IMMORTALE(d->character)) {
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
		SEND_TO_Q("Inserisci la nuova password: ", d);
		echoOff(d);
		STATE(d) = CON_PWDNEW;
		break;
	case '5':
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
	d->AlreadyInGame=false;
	d->justCreated=false;
	char tmp_name[100];
	int rc=parse_name(arg, tmp_name);
	mudlog(LOG_CONNECT,"Parsename result %d",rc);
	if(rc==2) {  // Il nome digitato contiene una @
		STATE(d)=CON_ACCOUNT_NAME;
		mudlog(LOG_CONNECT,"Calling account login (%s)",G::translate(STATE(d)));
		return true;
	}
	if(rc==1) {
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
	if(pg) {
		mudlog(LOG_CONNECT,"Toon found on db, registered to %d",pg->id);
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
	if(d->AccountData.level >= MAESTRO_DEL_CREATO) {
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
			STATE(d)=CON_PWDOK;
			return true;
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
		Sql::save(d->AccountData,true);
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
				buf.assign(GET_NAME(d->character)).assign("maschio/Femmina) (b per tornare indietro): ");
				SEND_TO_Q(buf.c_str(), d);
				STATE(d) = CON_QSEX;
				mudlog(LOG_CONNECT,"Razza scelta procedo con qsex");
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
			mudlog(LOG_CONNECT, "2.Loading %s's equipment",
				   d->character->player.name);
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
		Registered toon(GET_NAME(d->character));
		toon.del();
		close_socket(d);
	}
	else {
		SEND_TO_Q(MENU,d);
		STATE(d)= CON_SLCT;
	}
	return false;
}

NANNY_FUNC(con_pwdnew) {
	oldarg(false);
	if(!*arg || strlen(arg) > 10) {
		echoOn(d);
		SEND_TO_Q("Password non valida.\n\r", d);
		SEND_TO_Q("Password: ", d);
		echoOff(d);
		return false;
	}

	strncpy(d->pwd,(char*) crypt(arg, d->character->player.name), 10);
	*(d->pwd + 10) = '\0';
	echoOn(d);
	SEND_TO_Q("Reinserisci la password: ", d);
	STATE(d) = CON_PWDNCNF;
	echoOff(d);
	return true;;
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
	mudlog(LOG_CONNECT,"Outer nanny %s (%s)",d->currentInput.c_str(),G::translate(STATE(d)));
	echoOn(d);
	bool moresteps=false;
	do {
		uint16_t index=static_cast<uint16_t>(STATE(d));
		mudlog(LOG_CONNECT,"Inner nanny %s (%s) index: %d",d->currentInput.c_str(),G::translate(STATE(d)),index);
//		try {
		nanny_func f=nannyFuncs.at(index);
		moresteps=f(d);
//		}
//		catch (std::out_of_range &e) {
//			mudlog(LOG_SYSERR,"Invalid connection state, closing descriptor: %d (%s) %s",STATE(d),G::translate(STATE(d)),e.what());
//			close_socket(d);
//			moresteps=false;
//		}
		// Gestione account: stati messi tutti all'inizio perché poi fanno fallback sulla procedura standard
	}
	while(moresteps);
	if(d and STATE(d) == CON_SLCT) {
		boost::format fmt(R"(UPDATE toon SET lastlogin=now() WHERE name="%s")");
		fmt % d->AccountData.choosen;
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

