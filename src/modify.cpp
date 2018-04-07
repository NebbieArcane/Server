/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*AlarMUD
* $Id: modify.c,v 1.3 2002/02/24 18:42:47 Thunder Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cctype>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
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
#include "modify.hpp"
#include "act.info.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "mail.hpp"
#include "snew.hpp"
#include "utility.hpp"

namespace Alarmud {



#define TP_MOB    0
#define TP_OBJ    1
#define TP_ERROR  2

struct room_data* world;              /* dyn alloc'ed array of rooms     */


const char* string_fields[] = {
	"name",
	"short",
	"long",
	"description",
	"title",
	"delete-description",
	"delnoise",
	"delfarnoise",
	"\n"
};

const char* room_fields[] = {
	"name",   /* 1 */
	"desc",
	"fs",
	"exit",
	"exdsc",  /* 5 */
	"extra",  /* 6 */
	"riv",    /* 7 */
	"tele",   /* 8 */
	"tunn",   /* 9 */
	"\n"
};

/* maximum length for text field x+1 */
unsigned int length[] = {
	30,
	60,
	256,
	240,
	60,
};


unsigned int room_length[] = {
	80,
	1024,
	50,
	50,
	512,
	512,
	50,
	100,
	50
};

const char* skill_fields[] = {
	"learned",
	"affected",
	"duration",
	"recognize",
	"\n"
};

int max_value[] = {
	255,
	255,
	10000,
	1
};

/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data* d, char* str) {
	char* scan;
	int terminator = 0;

	/* determine if this is the terminal string, and truncate if so */
	for(scan = str; *scan; scan++)
		if((terminator = (*scan == '@'))) {
			*scan = '\0';
			break;
		}

	if(!(*d->str))        {
		if(strlen(str) > d->max_str)          {
			send_to_char("String too long - Truncated.\n\r",
						 d->character);
			*(str + d->max_str) = '\0';
			terminator = 1;
		}
		CREATE(*d->str, char, strlen(str) + 3);
		strcpy(*d->str, str);
	}
	else        {
		if(strlen(str) + strlen(*d->str) > d->max_str)        {
			send_to_char("String too long. Last line skipped.\n\r",
						 d->character);
			terminator = 1;
		}
		else         {
			if(!(*d->str = (char*) realloc(*d->str, strlen(*d->str) +
										   strlen(str) + 3)))             {
				perror("string_add");
				assert(0);
			}
			strcat(*d->str, str);
		}
	}

	if(terminator)        {
		if(!d->connected && (IS_SET(d->character->specials.act, PLR_MAILING))) {
			store_mail(d->name, d->character->player.name, *d->str);
			free(*d->str);
			free(d->str);
			free(d->name);
			d->name = 0;
			SEND_TO_Q("Message sent!\n\r", d);
			if(!IS_NPC(d->character)) {
				REMOVE_BIT(d->character->specials.act, PLR_MAILING);
			}
		}
		d->str = 0;
		if(d->connected == CON_EXDSCR)        {
			SEND_TO_Q(MENU, d);
			d->connected = CON_SLCT;
		}
	}
	else {
		strcat(*d->str, "\n\r");
	}
}


#undef MAX_STR

/* interpret an argument for do_string */
void quad_arg(const char* arg, int* type, char* name, int* field, char* buffer) {
	char buf[MAX_STRING_LENGTH];


	/* determine type */
	arg = one_argument(arg, buf);
	if(is_abbrev(buf, "char")) {
		*type = TP_MOB;
	}
	else if(is_abbrev(buf, "obj")) {
		*type = TP_OBJ;
	}
	else {
		*type = TP_ERROR;
		return;
	}

	/* find name */
	arg = one_argument(arg, name);

	/* field name and number */
	arg = one_argument(arg, buf);
	if(!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0))) {
		return;
	}

	/* string */
	for(; isspace(*arg); arg++);
	for(; (*buffer = *arg); arg++, buffer++);

	return;
}




/* modification of malloc'ed strings in chars/objects */
ACTION_FUNC(do_string) {

	char name[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];
	struct extra_descr_data* ed, *tmp;
	int field, type;
	struct char_data* mob;
	struct obj_data* obj;
	if(IS_NPC(ch)) {
		return;
	}

	quad_arg(arg, &type, name, &field, buffer);

	if(type == TP_ERROR) {
		send_to_char("Syntax:\n\rstring ('obj'|'char') <name> <field> "
					 "[<string>].", ch);
		return;
	}

	if(!field) {
		send_to_char("No field by that name. Try 'help string'.\n\r", ch);
		return;
	}

	if(type == TP_MOB) {
		/* locate the beast */
		if(!(mob = get_char_vis(ch, name))) {
			send_to_char("I don't know anyone by that name...\n\r", ch);
			return;
		}

		switch(field) {
		case 1:
			if(!IS_NPC(mob) && GetMaxLevel(ch) < MAESTRO_DEI_CREATORI) {
				send_to_char("You can't change that field for players.", ch);
				return;
			}
			if(!*buffer) {
				send_to_char("You have to supply a name!\n\r", ch);
				return;
			}
			ch->desc->str = &mob->player.name;
			if(!IS_NPC(mob))
				send_to_char("WARNING: You have changed the name of a player.\n\r",
							 ch);
			break;
		case 2:
			if(!IS_NPC(mob)) {
				send_to_char("That field is for monsters only.\n\r", ch);
				return;
			}
			if(!*buffer) {
				send_to_char("You have to supply a description!\n\r", ch);
				return;
			}
			ch->desc->str = &mob->player.short_descr;
			break;
		case 3:
			if(!IS_NPC(mob)) {
				send_to_char("That field is for monsters only.\n\r", ch);
				return;
			}
			ch->desc->str = &mob->player.long_descr;
			break;
		case 4:
			ch->desc->str = &mob->player.description;
			break;
		case 5:
			if(IS_NPC(mob)) {
				send_to_char("Monsters have no titles.\n\r",ch);
				return;
			}
			if((GetMaxLevel(ch) >= GetMaxLevel(mob)) && (ch != mob)) {
				ch->desc->str = &mob->player.title;
			}
			else {
				send_to_char("Sorry, can't set the title of someone of higher "
							 "level.\n\r", ch);
				return;
			}
			break;
		case 7:
			if(mob->player.sounds) {
				free(mob->player.sounds);
				mob->player.sounds = 0;
			}
			return;
			break;
		case 8:
			if(mob->player.distant_snds) {
				free(mob->player.distant_snds);
				mob->player.distant_snds = 0;
			}
			return;
			break;
		default:
			send_to_char("That field is undefined for monsters.\n\r", ch);
			return;
			break;
		}
	}
	else { /* type == TP_OBJ */
		/* locate the object */
		if(!(obj = get_obj_vis(ch, name))) {
			send_to_char("Can't find such a thing here..\n\r", ch);
			return;
		}

		switch(field) {
		case 1:
			if(!*buffer) {
				send_to_char("You have to supply a keyword.\n\r", ch);
				return;
			}
			else {
				ch->desc->str = &obj->name;
			}
			break;
		case 2:
			if(!*buffer) {
				send_to_char("You have to supply a keyword.\n\r", ch);
				return;
			}
			else {
				ch->desc->str = &obj->short_description;
			}
			break;
		case 3:
			ch->desc->str = &obj->description;
			break;
		case 4:
			if(!*buffer) {
				send_to_char("You have to supply a keyword.\n\r", ch);
				return;
			}
			/* try to locate extra description */
			for(ed = obj->ex_description; ; ed = ed->next) {
				if(!ed) {
					CREATE(ed, struct extra_descr_data, 1);
					ed->nMagicNumber = EXDESC_VALID_MAGIC;
					ed->next = obj->ex_description;
					obj->ex_description = ed;
					ed->keyword =  strdup(buffer);
					ed->description = NULL;
					ch->desc->str = &ed->description;
					send_to_char("New field.\n\r", ch);
					break;
				}
				else if(!str_cmp(ed->keyword, buffer)) {  /* the field exists */
					free(ed->description);
					ed->description = NULL;
					ch->desc->str = &ed->description;
					send_to_char("Modifying description.\n\r", ch);
					break;
				}
			}
			ch->desc->max_str = MAX_STRING_LENGTH;
			return; /* the stndrd (see below) procedure does not apply here */
			break;
		case 6: /* deletion */
			if(!*buffer) {
				send_to_char("You must supply a field name.\n\r", ch);
				return;
			}
			/* try to locate field */
			for(ed = obj->ex_description; ; ed = ed->next) {
				if(ed && ed->nMagicNumber != EXDESC_VALID_MAGIC) {
					mudlog(LOG_SYSERR,
						   "Invalid exdesc freeing exdesc in do_string (modify.c).");
					send_to_char("Problemi con le descrizioni dell'oggetto.\n\r", ch);
					return;
				}

				if(!ed) {
					send_to_char("No field with that keyword.\n\r", ch);
					return;
				}
				else if(!str_cmp(ed->keyword, buffer)) {
					free(ed->keyword);
					free(ed->description);

					/* delete the entry in the desr list */
					if(ed == obj->ex_description) {
						obj->ex_description = ed->next;
					}
					else {
						for(tmp = obj->ex_description; tmp->next != ed;
								tmp = tmp->next);
						tmp->next = ed->next;
					}
					ed->nMagicNumber = EXDESC_FREED_MAGIC;
					free(ed);

					send_to_char("Field deleted.\n\r", ch);
					return;
				}
			}

			break;
		default:
			send_to_char("That field is undefined for objects.\n\r", ch);
			return;
			break;
		}
	}

	if(*ch->desc->str) {
		free(*ch->desc->str);
	}

	if(*buffer) {
		/* there was a string in the argument array */
		if(strlen(buffer) > length[field - 1]) {
			send_to_char("String too long - truncated.\n\r", ch);
			*(buffer + length[field - 1]) = '\0';
		}
		CREATE(*ch->desc->str, char, strlen(buffer) + 1);
		strcpy(*ch->desc->str, buffer);
		ch->desc->str = 0;
		send_to_char("Ok.\n\r", ch);
	}
	else {
		/* there was no string. enter string mode */
		send_to_char("Enter string. terminate with '@'.\n\r", ch);
		*ch->desc->str = 0;
		ch->desc->max_str = length[field - 1];
	}
}




void bisect_arg(const char* arg, int* field, char* buffer) {
	char buf[MAX_INPUT_LENGTH];


	/* field name and number */
	arg = one_argument(arg, buf);
	if(!(*field = old_search_block(buf, 0, strlen(buf), room_fields, 0))) {
		return;
	}

	/* string */
	for(; isspace(*arg); arg++);
	for(; (*buffer = *arg); arg++, buffer++);

	return;
}


ACTION_FUNC(do_edit) {
	int field, dflags, dir, exroom, dkey, rspeed, rdir, open_cmd,
		tele_room, tele_time, tele_mask, moblim, tele_cnt;
	int r_flags;
	int s_type;
	unsigned int i;
	char buffer[512], sdflags[30];
	struct extra_descr_data* ed, *tmp;
	struct room_data*        rp;

	rp = real_roomp(ch->in_room);

	if((IS_NPC(ch)) || (GetMaxLevel(ch)<IMMORTALE)) {
		return;
	}

	if(!ch->desc) { /* someone is forced to do something. can be bad! */
		return;    /* the ch->desc->str field will cause problems... */
	}


	if((GetMaxLevel(ch) < MAESTRO_DEGLI_DEI) && rp->zone != GET_ZONE(ch))  {
		/*  (!IS_SET(ch->specials.permissions,PREV_AREA_MAKER)) )*/
		send_to_char("Spiacente, non sei autorizzato ad editare questa zona\n\r", ch);
		return;
	}

	bisect_arg(arg, &field, buffer);

	if(!field)        {
		send_to_char("No field by that name. Try 'help edit'.\n\r", ch);
		return;
	}

	r_flags = -1;
	s_type = -1;

	switch(field) {

	case 1:
		ch->desc->str = &rp->name;
		break;
	case 2:
		ch->desc->str = &rp->description;
		break;
	case 3:
		sscanf(buffer,"%u %d ",&r_flags,&s_type);
		if((r_flags < 0)  || (s_type < 0) || (s_type > 11)) {
			send_to_char("didn't quite get those, please try again.\n\r",ch);
			send_to_char("flags must be 0 or positive, and sectors must be from 0 to 11\n\r",ch);
			send_to_char("edit fs <flags> <sector_type>\n\r",ch);
			return;
		}
		rp->room_flags = r_flags;
		rp->sector_type = s_type;

		if(rp->sector_type == SECT_WATER_NOSWIM) {
			send_to_char("P.S. you need to do speed and flow\n\r",ch);
			send_to_char("For this river. (set to 0 as default)\n\r",ch);
			rp->river_speed = 0;
			rp->river_dir = 0;
			return;
		}
		return;
		break;

	case 4:

		open_cmd=-1; /* no cmd by default */
		sscanf(buffer,"%d %s %d %d %d", &dir, sdflags, &dkey, &exroom, &open_cmd);

		/*
		 * check if the exit exists
		  */
		if((dir < 0) || (dir > 5)) {
			send_to_char("You need to use numbers for that (0 - 5)",ch);
			return;
		}

		dflags=0;
		for(i=0; i<strlen(sdflags); i++) {
			switch(sdflags[i]) {
			case '0':
			case '-':
				dflags = 0;
				break;
			case '1':
				dflags = EX_ISDOOR;
				break;
			case '2':
				dflags = EX_ISDOOR | EX_PICKPROOF;
				break;
			case '3':
				dflags = EX_CLIMB;
				break;
			case '4':
				dflags = EX_CLIMB | EX_ISDOOR;
				break;
			case '5':
				dflags = EX_CLIMB | EX_ISDOOR | EX_PICKPROOF;
				break;
			case 'S':
			case 's':
				dflags |= EX_SECRET;
				break;
			case 'C':
			case 'c':
				dflags |= EX_CLIMB;
				break;
			case 'P':
			case 'p':
				dflags |= EX_PICKPROOF;
				break;
			case 'D':
			case 'd':
				dflags |= EX_ISDOOR;
				break;
			default:
				send_to_char("Invalid char in dflags value!\r\n", ch);
				break;
			}
		}

		if(rp->dir_option[dir]) {
			send_to_char("modifying exit\n\r",ch);

			rp->dir_option[dir]->exit_info=dflags;
			rp->dir_option[dir]->key = dkey;
			rp->dir_option[dir]->open_cmd = open_cmd;

			if(real_roomp(exroom) != NULL) {
				rp->dir_option[dir]->to_room = exroom;
			}
			else {
				send_to_char("Deleting exit.\n\r",ch);
				free(rp->dir_option[dir]);
				rp->dir_option[dir] = 0;
				return;
			}
		}
		else if(real_roomp(exroom)==NULL) {
			send_to_char("Hey, John Yaya, that's not a valid room.\n\r", ch);
			return;
		}
		else {
			send_to_char("New exit\n\r",ch);
			CREATE(rp->dir_option[dir],
				   struct room_direction_data, 1);

			rp->dir_option[dir]->exit_info=dflags;
			rp->dir_option[dir]->key = dkey;
			rp->dir_option[dir]->to_room = exroom;
			rp->dir_option[dir]->open_cmd = open_cmd;
		}

		if(rp->dir_option[dir]->exit_info>0) {
			buffer[0] = 0;
			send_to_char("enter keywords, 1 line only. \n\r",ch);
			send_to_char("terminate with an @ on the same line.\n\r",ch);
			ch->desc->str = &rp->dir_option[dir]->keyword;
			break;
		}
		else {
			return;
		}

	case 5:
		dir = -1;
		sscanf(buffer,"%d", &dir);
		if((dir >=0) && (dir <= 5)) {
			send_to_char("Enter text, term. with '@' on a blank line",ch);
			buffer[0] = 0;
			if(rp->dir_option[dir]) {
				ch->desc->str = &rp->dir_option[dir]->general_description;
			}
			else {
				CREATE(rp->dir_option[dir],
					   struct room_direction_data, 1);
				ch->desc->str = &rp->dir_option[dir]->general_description;
			}
		}
		else {
			send_to_char("Illegal direction\n\r",ch);
			send_to_char("Must enter 0-5.I will ask for text.\n\r",ch);
			return;
		}
		break;
	case 6:
		/*
		  extra descriptions
		  */
		if(!*buffer)          {
			send_to_char("You have to supply a keyword.\n\r", ch);
			return;
		}
		/* try to locate extra description */
		for(ed = rp->ex_description; ; ed = ed->next)
			if(!ed) {
				CREATE(ed, struct extra_descr_data, 1);
				ed->next = rp->ex_description;
				rp->ex_description = ed;
				CREATE(ed->keyword, char, strlen(buffer) + 1);
				strcpy(ed->keyword, buffer);
				ed->description = 0;
				ch->desc->str = &ed->description;
				send_to_char("New field.\n\r", ch);
				break;
			}
			else if(!str_cmp(ed->keyword, buffer)) {
				/* the field exists */
				free(ed->description);
				ed->description = 0;
				ch->desc->str = &ed->description;
				send_to_char("Modifying description.\n\r", ch);
				break;
			}
		ch->desc->max_str = MAX_STRING_LENGTH;
		return;
		break;

	case 7:
		/*  this is where the river stuff will go */
		rspeed = 0;
		rdir = 0;
		sscanf(buffer,"%d %d ",&rspeed,&rdir);
		if((rdir>= 0) && (rdir <= 5)) {
			rp->river_speed = rspeed;
			rp->river_dir = rdir;
		}
		else {
			send_to_char("Illegal dir. : edit riv <speed> <dir>\n\r",ch);
		}
		return;

	case 8:
		/*  this is where the teleport stuff will go */
		tele_room = -1;
		tele_time = -1;
		tele_mask = -1;
		sscanf(buffer,"%d %d %d",&tele_time,&tele_room,&tele_mask);
		if(tele_room < 0 || tele_time < 0 || tele_mask < 0) {
			send_to_char(" edit tele <time> <room_nr> <tele-flags>\n\r", ch);
			return;
			break;
		}
		else {
			if(IS_SET(TELE_COUNT, tele_mask)) {
				sscanf(buffer,"%d %d %d %d",
					   &tele_time, &tele_room, &tele_mask, &tele_cnt);
				if(tele_cnt < 0) {
					send_to_char
					(" edit tele <time> <room_nr> <tele-flags> [tele-count]\n\r", ch);
					return;
				}
				else {
					real_roomp(ch->in_room)->tele_time = tele_time;
					real_roomp(ch->in_room)->tele_targ = tele_room;
					real_roomp(ch->in_room)->tele_mask = tele_mask;
					real_roomp(ch->in_room)->tele_cnt  = tele_cnt;
				}
			}
			else {
				real_roomp(ch->in_room)->tele_time = tele_time;
				real_roomp(ch->in_room)->tele_targ = tele_room;
				real_roomp(ch->in_room)->tele_mask = tele_mask;
				real_roomp(ch->in_room)->tele_cnt  = 0;
				return;
			}
		}

		return;
	case 9:
		if(sscanf(buffer, "%d", &moblim) < 1) {
			send_to_char("edit tunn <mob_limit>\n\r", ch);
			return;
			break;
		}
		else {
			real_roomp(ch->in_room)->moblim = moblim;
			if(!IS_SET(real_roomp(ch->in_room)->room_flags, TUNNEL)) {
				SET_BIT(real_roomp(ch->in_room)->room_flags, TUNNEL);
			}
			return;
			break;
		}
	case 10:
		/*
		  deletion
		  */
		if(!*buffer)          {
			send_to_char("You must supply a field name.\n\r", ch);
			return;
		}
		/* try to locate field */
		for(ed = rp->ex_description; ; ed = ed->next)
			if(!ed) {
				send_to_char("No field with that keyword.\n\r", ch);
				return;
			}
			else if(!str_cmp(ed->keyword, buffer)) {
				free(ed->keyword);
				if(ed->description) {
					free(ed->description);
				}

				/* delete the entry in the desr list */
				if(ed == rp->ex_description) {
					rp->ex_description = ed->next;
				}
				else {
					for(tmp = rp->ex_description; tmp->next != ed;
							tmp = tmp->next);
					tmp->next = ed->next;
				}
				free(ed);

				send_to_char("Field deleted.\n\r", ch);
				return;
			}
		break;

	default:
		send_to_char("I'm so confused :-)\n\r",ch);
		return;
		break;
	}

	if(*ch->desc->str)        {
		free(*ch->desc->str);
	}

	if(*buffer) {    /* there was a string in the argument array */
		if(strlen(buffer) > room_length[field - 1])        {
			send_to_char("String too long - truncated.\n\r", ch);
			*(buffer + length[field - 1]) = '\0';
		}
		CREATE(*ch->desc->str, char, strlen(buffer) + 1);
		strcpy(*ch->desc->str, buffer);
		ch->desc->str = 0;
		send_to_char("Ok.\n\r", ch);
	}
	else {    /* there was no string. enter string mode */
		send_to_char("Enter string. terminate with '@'.\n\r", ch);
		*ch->desc->str = 0;
		ch->desc->max_str = room_length[field - 1];
	}

}


/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */

#ifndef ALAR
ACTION_FUNC(do_setskill) {
	send_to_char("This routine is disabled untill it fitts\n\r", ch);
	send_to_char("The new structures (sorry Quinn) ....Bombman\n\r", ch);
	return;
}
#else
ACTION_FUNC(do_setskill) {
	char buf[ 256 ];
	struct char_data* mob;
	char sskill[256];
	char svalue[256];
	char sspecial[256];
	char sflags[256];
	int iskill;
	int ivalue;
	int ispecial;
	int iflags;
	arg = one_argument(arg, buf);
	arg = one_argument(arg, sskill);
	arg = one_argument(arg, svalue);
	arg = one_argument(arg, sspecial);
	arg = one_argument(arg, sflags);
	iskill=atoi(sskill);
	ivalue=atoi(svalue);
	ispecial=atoi(sspecial);
	iflags=atoi(sflags);
	if(!(iskill+ivalue)) {
		send_to_char("setsk numeroskill valore specializzato flags",ch); // SALVO aggiungo la possibilita' di modificare flags
		return;
	}

	if((mob = get_char_vis(ch, buf)) == NULL) {
		send_to_char("Non c'e` nessuno con quel nome qui.\n\r", ch);
	}
	else if(mob->skills == NULL) {
		send_to_char("Il giocatore non ha skills.\n\r", ch);
	}
	else {
		if(ivalue)    {
			mob->skills[ iskill ].learned = ivalue;
		}
		if(iflags)    {
			mob->skills[ iskill ].flags   = iflags;
		}
		if(ispecial)  {
			mob->skills[ iskill ].special = ispecial;
		}
		mob->skills[ iskill ].nummem  = 0;
		send_to_char("Fatto.\n\r", ch);
	}
}
#endif

/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char* one_word(char* arg, char* first_arg) {
	int begin, look_at;

	begin = 0;

	do {
		for(; isspace(*(arg + begin)); begin++);

		if(*(arg+begin) == '\"') {
			/* is it a quote */

			begin++;

			for(look_at=0; (*(arg+begin+look_at) >= ' ') &&
					(*(arg+begin+look_at) != '\"') ; look_at++) {
				*(first_arg + look_at) = LOWER(*(arg + begin + look_at));
			}

			if(*(arg+begin+look_at) == '\"') {
				begin++;
			}

		}
		else {

			for(look_at=0; *(arg+begin+look_at) > ' ' ; look_at++) {
				*(first_arg + look_at) = LOWER(*(arg + begin + look_at));
			}

		}

		*(first_arg + look_at) = '\0';
		begin += look_at;
	}
	while(fill_word(first_arg));

	return(arg+begin);
}


struct help_index_element* build_help_index(FILE* fl, int* num) {
	int nr = -1, issorted, i;
	struct help_index_element* list = 0, mem;
	char buf[81], tmp[81], *scan;
	long pos;

	for(;;) {
		pos = ftell(fl);
		fgets(buf, 81, fl);
		*(buf + strlen(buf) - 1) = '\0';
		scan = buf;
		for(;;) {
			/* extract the keywords */
			scan = one_word(scan, tmp);

			if(!*tmp) {
				break;
			}

			if(!list) {
				CREATE(list, struct help_index_element, 1);
				nr = 0;
			}
			else {
				RECREATE(list, struct help_index_element, ++nr+1);
			}

			list[nr].pos = pos;
			CREATE(list[nr].keyword, char, strlen(tmp) + 1);
			strcpy(list[nr].keyword, tmp);
		}
		/* skip the text */
		do {
			fgets(buf, 81, fl);
		}
		while(*buf != '#');
		if(*(buf + 1) == '~') {
			break;
		}
	}
	/* we might as well sort the stuff */
	do {
		issorted = 1;
		for(i = 0; i < nr; i++) {
			if(str_cmp(list[i].keyword, list[i + 1].keyword) > 0) {
				mem = list[i];
				list[i] = list[i + 1];
				list[i + 1] = mem;
				issorted = 0;
			}
		}
	}
	while(!issorted);

	*num = nr;
	return(list);
}



void page_string(struct descriptor_data* d, const char* str, int keep_internal) {
	if(!d) {
		return;
	}

	if(keep_internal)        {
		CREATE(d->showstr_head, char, strlen(str) + 1);
		// Let's hope the caller is right: I assume that the passed string is a temporary one and need to be stored in showstr_head
		strcpy(d->showstr_head, const_cast<char*>(str));
		d->showstr_point = d->showstr_head;
	}
	else {
		free(d->showstr_head);
		d->showstr_head=nullptr;
		d->showstr_point = str;
	}

	show_string(d, "");
}

void show_string(struct descriptor_data* d, const char* input) {
	char buffer[ MAX_STRING_LENGTH ], buf[ MAX_INPUT_LENGTH ];
	int lines = 0, toggle = 1;
	int i;
	one_argument(input, buf);

	if(*buf) {
		if(d->showstr_head) {
			free(d->showstr_head);
			d->showstr_head = nullptr;
		}
		d->showstr_point = nullptr;
		return;
	}

	if(!d->character) {
		i = 20;
	}
	else if(IS_SET(d->character->player.user_flags, USE_PAGING)) {
		if(d->character->term == 0) {
			i = d->character->size - 4;
		}
		else {
			i = d->character->size - 8;
		}
	}
	else {
		i = 1000;
	}

	/* show a chunk */
	for(char* scan = buffer;; scan++, d->showstr_point++) {
		if((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) && ((toggle = -toggle) < 0)) {
			lines++;
			if(strlen(buffer) > MAX_STRING_LENGTH - 265) {
				i = lines;
			}
		}
		else if(!*scan || (lines >= i)) {
			*scan = '\0';

			SEND_TO_Q(ParseAnsiColors(IS_SET(d->character->player.user_flags,
											 USE_ANSI), buffer), d);
			/* see if this is the end (or near the end) of the string */
			const char* chk;
			for(chk = d->showstr_point; *chk && isspace(*chk); chk++);
			if(!*chk) {
				if(d->showstr_head) {
					free(d->showstr_head);
					d->showstr_head = nullptr;
				}
				d->showstr_point = nullptr;
			}
			return;
		}
	}
}

void check_reboot() {
	static time_t lastCheck=time(0);
	time_t tc;
	struct tm* t_info;
	FILE* boot;
	static int TooMuchLag=-1;
	static bool bBootSequenceStarted = FALSE;
	static int shutdownlevel=0;
	static int forceshutdown=0;
	char REBOOTFILE[15];
	if(GetLagIndex()> 400000) {
		if(TooMuchLag<20) {
			TooMuchLag++;
		}
	}
	if(GetLagIndex()<250000) {
		if(TooMuchLag>-20)  {
			TooMuchLag--;
		}
	}

	tc = time(0);
	t_info = localtime(&tc);
	if(forceshutdown) {
		shutdownlevel=25;
	}
	// If we already on a reboot sequence, checking is pointless
	if(!bBootSequenceStarted && (tc-lastCheck) >=60) {  //Once every minute
		mudlog(LOG_CHECK,"Shutdown status: %d %d %d",shutdownlevel,bBootSequenceStarted,(tc-lastCheck));
		lastCheck=tc;
		sprintf(REBOOTFILE,"REBOOT%02d%1d0",t_info->tm_hour,(t_info->tm_min / 10));
		if((boot = fopen(REBOOTFILE, "r+"))) {
			fclose(boot);
			bBootSequenceStarted=TRUE;
			shutdownlevel=0;
		}
		else {
			sprintf(REBOOTFILE,"REBOOT.NOW");
			if((boot = fopen(REBOOTFILE, "r+"))) {
				fclose(boot);
				unlink(REBOOTFILE);
				bBootSequenceStarted=TRUE;
				shutdownlevel=19;
			}
		}
	}
	else if(bBootSequenceStarted) {
		mudlog(LOG_CHECK,"Shutdown status: %d %d %d",shutdownlevel,bBootSequenceStarted,(tc-lastCheck));
		shutdownlevel+=((tc-lastCheck)/60);
		lastCheck=tc;
		if(shutdownlevel > 30) {
			struct descriptor_data* pDesc;
			for(pDesc = descriptor_list; pDesc; pDesc = pDesc->next) {
				/* send_to_all qui non funziona a causa della bufferizzazione. */
				if(pDesc->connected == CON_PLYNG)
					write_to_descriptor(pDesc->descriptor,
										ParseAnsiColors(IS_SET(pDesc->character->player.user_flags,
														USE_ANSI),
														"Reboot automatico. "
														"Ci rivediamo tra poco.\n\r"));
			}
			raw_force_all("return");
			raw_force_all("save");
			mudshutdown = rebootgame = 1;
		}
		else if(shutdownlevel <= 30) {
			if(shutdownlevel > 29) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira` entro un minuto!\n\r");
			}
			else if(shutdownlevel >= 28) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira` entro 2 minuti.\n\r");
			}
			else if(shutdownlevel >= 27) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira` entro 3 minuti.\n\r");
			}
			else if(shutdownlevel >= 26) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira` entro 4 minuti.\n\r");
			}
			else if(shutdownlevel >= 25) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira` entro 5 minuti.\n\r");
			}
			else if(shutdownlevel == 20) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira` entro 10 minuti.\n\r");
			}
			else if(shutdownlevel == 15) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira` entro 15 minuti.\n\r");
			}
			else if(shutdownlevel == 10) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira` entro 20 minuti.\n\r");
			}
		}
	}
	if(TooMuchLag>10 && !forceshutdown) {
		send_to_all("$c0015ATTENZIONE! $c0014Lag eccessivo. Iniziata sequenza di shutdown!\n\r");
		bBootSequenceStarted=TRUE;
		forceshutdown=t_info->tm_min;
		if(!forceshutdown) {
			forceshutdown=1;
		}
	}
	if(TooMuchLag<5 && forceshutdown) {

		send_to_all("$c0015ATTENZIONE! $c0014Lag risolto. Shutdown cancellato!\n\r");
		bBootSequenceStarted=FALSE;
		forceshutdown=0;
	}

	return;
}

} // namespace Alarmud

