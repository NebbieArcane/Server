/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*AlarMUD
* $Id: modify.c,v 1.3 2002/02/24 18:42:47 Thunder Exp $
 * */
/***************************  System  include ************************************/
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>
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
#include "modify.hpp"
#include "server_text.hpp"
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
				mudlog(LOG_ERROR,"%s:%s","string_add",strerror(errno));
				assert(0);
			}
			strcat(*d->str, str);
		}
	}

	if(terminator)        {
		if(d->character && server_text_finish_body_write(d)) {
			/* finish libera il testo e azzera d->str; non fare free(d->str):
			 * punta nel map g_body_pending, non a un char** malloc (mail). */
			d->str = 0;
		}
		else if(!d->connected && (IS_SET(d->character->specials.act, PLR_MAILING))) {
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
			d->idle_since = time(nullptr);
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
		sscanf(buffer,"%d %d ",&r_flags,&s_type);
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
		send_to_char("Non c'e' nessuno con quel nome qui.\n\r", ch);
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


/* "#", "# 0014", "#0014" -> 0..15; missing/invalid -> 15. "#~" is end marker. */
static int parse_help_title_color(std::string_view hash_line) {
	if(hash_line.empty() || hash_line.front() != '#') {
		return 15;
	}
	hash_line.remove_prefix(1);
	while(!hash_line.empty() && (hash_line.front() == ' ' || hash_line.front() == '\t')) {
		hash_line.remove_prefix(1);
	}
	if(hash_line.empty() || hash_line.front() == '~') {
		return 15;
	}
	if(!isdigit(static_cast<unsigned char>(hash_line.front()))) {
		return 15;
	}
	int value = 0;
	int digits = 0;
	while(!hash_line.empty() && isdigit(static_cast<unsigned char>(hash_line.front())) && digits < 4) {
		value = value * 10 + (hash_line.front() - '0');
		hash_line.remove_prefix(1);
		++digits;
	}
	if(value < 0 || value > 15) {
		return 15;
	}
	return value;
}

static void help_line_chomp(std::string& line) {
	while(!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
		line.pop_back();
	}
}

bool help_read_line(FILE* fl, std::string& line) {
	line.clear();
	if(fl == nullptr) {
		return false;
	}
	constexpr std::size_t kChunk = 1024;
	std::array<char, kChunk> chunk{};
	while(fgets(chunk.data(), static_cast<int>(chunk.size()), fl) != nullptr) {
		line.append(chunk.data());
		if(line.find('\n') != std::string::npos || line.find('\r') != std::string::npos) {
			return true;
		}
		/* Chunk filled without EOL: keep reading so long lines stay intact. */
	}
	return !line.empty();
}

struct help_index_element* build_help_index(FILE* fl, int* num) {
	int nr = -1;
	struct help_index_element* list = nullptr;
	std::string line;
	std::array<char, 256> keywordBuf{};
	long pos = 0;
	int pending_color = 15;
	bool have_line = false;

	for(;;) {
		if(!have_line) {
			pos = ftell(fl);
			if(!help_read_line(fl, line)) {
				break;
			}
		}
		else {
			have_line = false;
		}
		help_line_chomp(line);

		/* Headers: "#", "# 0014", "#~". Keywords follow on the next line. */
		while(!line.empty() && line.front() == '#') {
			if(line.size() > 1 && line[1] == '~') {
				goto finished;
			}
			pending_color = parse_help_title_color(line);
			pos = ftell(fl);
			if(!help_read_line(fl, line)) {
				goto finished;
			}
			help_line_chomp(line);
		}

		char* scan = line.data();
		for(;;) {
			scan = one_word(scan, keywordBuf.data());
			if(keywordBuf[0] == '\0') {
				break;
			}

			if(list == nullptr) {
				CREATE(list, struct help_index_element, 1);
				nr = 0;
			}
			else {
				RECREATE(list, struct help_index_element, ++nr + 1);
			}

			list[nr].pos = pos;
			list[nr].title_color = pending_color;
			CREATE(list[nr].keyword, char, std::strlen(keywordBuf.data()) + 1);
			std::strcpy(list[nr].keyword, keywordBuf.data());
		}

		/* skip the text until the next # header */
		do {
			if(!help_read_line(fl, line)) {
				goto finished;
			}
		}
		while(line.empty() || line.front() != '#');
		have_line = true;
	}

finished:
	if(list != nullptr && nr >= 0) {
		std::sort(list, list + nr + 1, [](const help_index_element& a, const help_index_element& b) {
			return str_cmp(a.keyword, b.keyword) < 0;
		});
	}

	*num = nr;
	return list;
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

static bool s_rebootSequenceStarted = FALSE;
static int s_shutdownLevel = 0;

static constexpr const char* kRebootNowFile = "REBOOT.NOW";
static constexpr const char* kRebootDoneFile = "REBOOT.DONE";
static constexpr const char* kRebootScheduleFile = "REBOOT.SCHEDULE";
static constexpr size_t kMaxRebootSlots = 8;
static constexpr int kDefaultRebootSlotsMin[] = {
	4 * 60 + 0,   /* 04:00 */
	11 * 60 + 0,  /* 11:00 */
	18 * 60 + 0,  /* 18:00 */
};

static std::vector<int> s_rebootSlots;
static bool s_rebootSlotsLoaded = false;

static void format_day(const struct tm& now, char* out, size_t outlen) {
	std::snprintf(out, outlen, "%04d-%02d-%02d", now.tm_year + 1900, now.tm_mon + 1,
				  now.tm_mday);
}

static void format_slot(int slot_min, char* out, size_t outlen) {
	std::snprintf(out, outlen, "%02d%02d", slot_min / 60, slot_min % 60);
}

static void format_slot_colon(int slot_min, char* out, size_t outlen) {
	std::snprintf(out, outlen, "%02d:%02d", slot_min / 60, slot_min % 60);
}

static void reboot_slots_apply_defaults() {
	s_rebootSlots = {
		kDefaultRebootSlotsMin[0],
		kDefaultRebootSlotsMin[1],
		kDefaultRebootSlotsMin[2],
	};
}

static void reboot_slots_normalize(std::vector<int>& slots) {
	std::sort(slots.begin(), slots.end());
	slots.erase(std::unique(slots.begin(), slots.end()), slots.end());
	if(slots.size() > kMaxRebootSlots) {
		slots.resize(kMaxRebootSlots);
	}
}

/* Accetta HH:MM, H:MM, HHMM. */
static bool parse_reboot_time_token(const char* tok, int* out_min) {
	if(!tok || !*tok || !out_min) {
		return false;
	}
	int h = -1;
	int m = -1;
	if(std::strchr(tok, ':')) {
		if(std::sscanf(tok, "%d:%d", &h, &m) != 2) {
			return false;
		}
	}
	else {
		const size_t n = std::strlen(tok);
		if(n < 3 || n > 4) {
			return false;
		}
		for(size_t i = 0; i < n; ++i) {
			if(!std::isdigit(static_cast<unsigned char>(tok[i]))) {
				return false;
			}
		}
		if(n == 3) {
			h = tok[0] - '0';
			m = (tok[1] - '0') * 10 + (tok[2] - '0');
		}
		else {
			h = (tok[0] - '0') * 10 + (tok[1] - '0');
			m = (tok[2] - '0') * 10 + (tok[3] - '0');
		}
	}
	if(h < 0 || h > 23 || m < 0 || m > 59) {
		return false;
	}
	*out_min = h * 60 + m;
	return true;
}

static bool reboot_slots_save() {
	FILE* f = fopen(kRebootScheduleFile, "w");
	if(!f) {
		mudlog(LOG_CHECK, "Impossibile scrivere %s", kRebootScheduleFile);
		return false;
	}
	fprintf(f, "# Orari reboot automatici (HH:MM, uno per riga). Max %zu.\n",
			kMaxRebootSlots);
	fprintf(f, "# Modificabile anche col comando wiz: reboottime set ...\n");
	for(int slot : s_rebootSlots) {
		char buf[8] = {};
		format_slot_colon(slot, buf, sizeof(buf));
		fprintf(f, "%s\n", buf);
	}
	fclose(f);
	return true;
}

static void reboot_slots_load() {
	if(s_rebootSlotsLoaded) {
		return;
	}
	s_rebootSlotsLoaded = true;
	s_rebootSlots.clear();

	FILE* f = fopen(kRebootScheduleFile, "r");
	if(!f) {
		reboot_slots_apply_defaults();
		mudlog(LOG_CHECK, "%s assente: uso default 04:00 11:00 18:00",
			   kRebootScheduleFile);
		return;
	}

	char line[64] = {};
	while(fgets(line, sizeof(line), f)) {
		for(char* p = line; *p; ++p) {
			if(*p == '\n' || *p == '\r') {
				*p = '\0';
				break;
			}
		}
		char* p = line;
		while(*p && std::isspace(static_cast<unsigned char>(*p))) {
			++p;
		}
		if(!*p || *p == '#') {
			continue;
		}
		char* end = p + std::strlen(p);
		while(end > p && std::isspace(static_cast<unsigned char>(end[-1]))) {
			*--end = '\0';
		}
		int slot = 0;
		if(parse_reboot_time_token(p, &slot)) {
			s_rebootSlots.push_back(slot);
		}
		else {
			mudlog(LOG_CHECK, "%s: orario non valido '%s' (ignorato)",
				   kRebootScheduleFile, p);
		}
	}
	fclose(f);

	reboot_slots_normalize(s_rebootSlots);
	if(s_rebootSlots.empty()) {
		mudlog(LOG_CHECK, "%s senza orari validi: reboot automatici disattivati",
			   kRebootScheduleFile);
	}
}

static const std::vector<int>& reboot_slots_get() {
	reboot_slots_load();
	return s_rebootSlots;
}

/*
 * REBOOT.DONE — riga data YYYY-MM-DD, poi HHMM degli slot gia' eseguiti oggi.
 */
static bool slot_done_today(const struct tm& now, int slot_min) {
	FILE* f = fopen(kRebootDoneFile, "r");
	if(!f) {
		return false;
	}
	char day[16] = {};
	char want_day[16] = {};
	format_day(now, want_day, sizeof(want_day));
	if(!fgets(day, sizeof(day), f)) {
		fclose(f);
		return false;
	}
	for(char* p = day; *p; ++p) {
		if(*p == '\n' || *p == '\r') {
			*p = '\0';
			break;
		}
	}
	if(std::strcmp(day, want_day) != 0) {
		fclose(f);
		return false;
	}
	char want_slot[8] = {};
	format_slot(slot_min, want_slot, sizeof(want_slot));
	char line[16] = {};
	bool found = false;
	bool any_slot_line = false;
	while(fgets(line, sizeof(line), f)) {
		for(char* p = line; *p; ++p) {
			if(*p == '\n' || *p == '\r') {
				*p = '\0';
				break;
			}
		}
		if(!line[0]) {
			continue;
		}
		any_slot_line = true;
		if(std::strcmp(line, want_slot) == 0) {
			found = true;
			break;
		}
	}
	fclose(f);
	/* Formato vecchio: solo YYYY-MM-DD → considera tutta la giornata gia' fatta
	 * (evita reboot immediato al deploy nel pomeriggio). */
	if(!any_slot_line) {
		return true;
	}
	return found;
}

static void mark_slots_done_today(const struct tm& now, const int* slots, size_t nslots) {
	char want_day[16] = {};
	format_day(now, want_day, sizeof(want_day));

	std::array<std::array<char, 8>, 16> existing {};
	size_t nexist = 0;
	FILE* f = fopen(kRebootDoneFile, "r");
	if(f) {
		char day[16] = {};
		if(fgets(day, sizeof(day), f)) {
			for(char* p = day; *p; ++p) {
				if(*p == '\n' || *p == '\r') {
					*p = '\0';
					break;
				}
			}
			if(std::strcmp(day, want_day) == 0) {
				char line[16] = {};
				while(nexist < existing.size() && fgets(line, sizeof(line), f)) {
					for(char* p = line; *p; ++p) {
						if(*p == '\n' || *p == '\r') {
							*p = '\0';
							break;
						}
					}
					if(line[0]) {
						std::snprintf(existing[nexist].data(), existing[nexist].size(), "%s",
									  line);
						++nexist;
					}
				}
			}
		}
		fclose(f);
	}

	f = fopen(kRebootDoneFile, "w");
	if(!f) {
		mudlog(LOG_CHECK, "Impossibile scrivere %s", kRebootDoneFile);
		return;
	}
	fprintf(f, "%s\n", want_day);
	for(size_t i = 0; i < nexist; ++i) {
		fprintf(f, "%s\n", existing[i].data());
	}
	for(size_t i = 0; i < nslots; ++i) {
		char slot[8] = {};
		format_slot(slots[i], slot, sizeof(slot));
		bool already = false;
		for(size_t j = 0; j < nexist; ++j) {
			if(std::strcmp(existing[j].data(), slot) == 0) {
				already = true;
				break;
			}
		}
		if(!already) {
			fprintf(f, "%s\n", slot);
		}
	}
	fclose(f);
}

/*
 * Slot da REBOOT.SCHEDULE (default 04:00 / 11:00 / 18:00). Se ne sono saltati
 * piu' di uno, una sola sequenza e tutti gli slot <= ora marcati fatti (no cascata).
 */
static bool scheduled_reboot_due(const struct tm& now) {
	const std::vector<int>& slots = reboot_slots_get();
	if(slots.empty()) {
		return false;
	}

	const int now_min = now.tm_hour * 60 + now.tm_min;
	int latest_due = -1;
	std::vector<int> due_buf;
	due_buf.reserve(slots.size());

	for(int slot : slots) {
		if(now_min >= slot && !slot_done_today(now, slot)) {
			latest_due = slot;
			due_buf.push_back(slot);
		}
	}
	if(latest_due < 0 || due_buf.empty()) {
		return false;
	}

	mark_slots_done_today(now, due_buf.data(), due_buf.size());
	{
		char slot_label[8] = {};
		format_slot_colon(latest_due, slot_label, sizeof(slot_label));
		mudlog(LOG_CHECK, "Scheduled daily reboot due (slot %s, marked %d pending)",
			   slot_label, static_cast<int>(due_buf.size()));
	}
	return true;
}

static void reboottime_show(struct char_data* ch) {
	const std::vector<int>& slots = reboot_slots_get();
	time_t tc = time(0);
	struct tm now_tm {};
	localtime_r(&tc, &now_tm);
	char day[16] = {};
	format_day(now_tm, day, sizeof(day));

	char buf[MAX_STRING_LENGTH];
	snprintf(buf, sizeof(buf),
			 "$c0014Orari reboot automatici$c0007 (file %s, oggi %s):\n\r",
			 kRebootScheduleFile, day);
	send_to_char(buf, ch);

	if(slots.empty()) {
		send_to_char("  (nessuno — reboot automatici disattivati)\n\r", ch);
	}
	else {
		for(int slot : slots) {
			char colon[8] = {};
			format_slot_colon(slot, colon, sizeof(colon));
			const bool done = slot_done_today(now_tm, slot);
			const int now_min = now_tm.tm_hour * 60 + now_tm.tm_min;
			const char* stato = done ? "fatto"
								: (now_min >= slot ? "in ritardo" : "in attesa");
			snprintf(buf, sizeof(buf), "  %s  — %s\n\r", colon, stato);
			send_to_char(buf, ch);
		}
	}
	send_to_char(
		"\n\rSintassi: reboottime | reboottime set HH:MM ... | reboottime reset | "
		"reboottime clear\n\r",
		ch);
}

ACTION_FUNC(do_reboottime) {
	char sub[MAX_INPUT_LENGTH];

	if(IS_NPC(ch)) {
		return;
	}

	arg = one_argument(arg, sub);
	if(!*sub) {
		reboottime_show(ch);
		return;
	}

	if(!str_cmp(sub, "reset")) {
		reboot_slots_apply_defaults();
		s_rebootSlotsLoaded = true;
		if(!reboot_slots_save()) {
			send_to_char("Orari impostati in memoria ma non riesco a scrivere "
						 "REBOOT.SCHEDULE.\n\r",
						 ch);
			return;
		}
		mudlog(LOG_PLAYERS, "reboottime reset by %s", GET_NAME(ch));
		send_to_char("Orari ripristinati ai default (04:00 11:00 18:00).\n\r", ch);
		reboottime_show(ch);
		return;
	}

	if(!str_cmp(sub, "clear") || !str_cmp(sub, "cleardone")) {
		if(unlink(kRebootDoneFile) == 0) {
			mudlog(LOG_PLAYERS, "reboottime clear (REBOOT.DONE) by %s", GET_NAME(ch));
			send_to_char("Cancellato REBOOT.DONE: gli slot di oggi tornano "
						 "eseguibili.\n\r",
						 ch);
		}
		else {
			send_to_char("REBOOT.DONE assente o non cancellabile "
						 "(gia' pulito?).\n\r",
						 ch);
		}
		reboottime_show(ch);
		return;
	}

	if(!str_cmp(sub, "set") || !str_cmp(sub, "imposta")) {
		std::vector<int> neu;
		char tok[MAX_INPUT_LENGTH];
		bool first = true;
		while(arg && *arg) {
			arg = one_argument(arg, tok);
			if(!*tok) {
				break;
			}
			if(first &&
			   (!str_cmp(tok, "off") || !str_cmp(tok, "none") ||
				!str_cmp(tok, "disable"))) {
				s_rebootSlots.clear();
				s_rebootSlotsLoaded = true;
				if(!reboot_slots_save()) {
					send_to_char("Disattivati in memoria ma non riesco a scrivere "
								 "REBOOT.SCHEDULE.\n\r",
								 ch);
					return;
				}
				mudlog(LOG_PLAYERS, "reboottime disabled by %s", GET_NAME(ch));
				send_to_char("Reboot automatici disattivati.\n\r", ch);
				reboottime_show(ch);
				return;
			}
			first = false;
			int slot = 0;
			if(!parse_reboot_time_token(tok, &slot)) {
				send_to_char("Orario non valido (usa HH:MM o HHMM).\n\r", ch);
				return;
			}
			neu.push_back(slot);
		}
		if(neu.empty()) {
			send_to_char(
				"Uso: reboottime set HH:MM [HH:MM ...]\n\r"
				"Esempio: reboottime set 04:00 11:00 18:00\n\r"
				"Per disattivare: reboottime set off\n\r",
				ch);
			return;
		}
		reboot_slots_normalize(neu);
		s_rebootSlots = std::move(neu);
		s_rebootSlotsLoaded = true;
		if(!reboot_slots_save()) {
			send_to_char("Orari impostati in memoria ma non riesco a scrivere "
						 "REBOOT.SCHEDULE.\n\r",
						 ch);
			return;
		}
		mudlog(LOG_PLAYERS, "reboottime set by %s (%d slots)", GET_NAME(ch),
			   static_cast<int>(s_rebootSlots.size()));
		send_to_char("Orari reboot aggiornati.\n\r", ch);
		{
			time_t tc = time(0);
			struct tm now_tm {};
			localtime_r(&tc, &now_tm);
			const int now_min = now_tm.tm_hour * 60 + now_tm.tm_min;
			bool overdue = false;
			for(int slot : s_rebootSlots) {
				if(now_min >= slot && !slot_done_today(now_tm, slot)) {
					overdue = true;
					break;
				}
			}
			if(overdue) {
				send_to_char(
					"$c0015Attenzione$c0007: uno o piu' orari sono gia' scaduti "
					"e non risultano fatti oggi — la sequenza reboot partira' "
					"entro un minuto.\n\r",
					ch);
			}
		}
		reboottime_show(ch);
		return;
	}

	if(!str_cmp(sub, "off") || !str_cmp(sub, "none") || !str_cmp(sub, "disable")) {
		s_rebootSlots.clear();
		s_rebootSlotsLoaded = true;
		if(!reboot_slots_save()) {
			send_to_char("Disattivati in memoria ma non riesco a scrivere "
						 "REBOOT.SCHEDULE.\n\r",
						 ch);
			return;
		}
		mudlog(LOG_PLAYERS, "reboottime disabled by %s", GET_NAME(ch));
		send_to_char("Reboot automatici disattivati.\n\r", ch);
		reboottime_show(ch);
		return;
	}

	send_to_char(
		"Sintassi: reboottime | reboottime set HH:MM ... | reboottime reset | "
		"reboottime clear | reboottime off\n\r",
		ch);
}

bool auction_blocked_near_reboot(void) {
	if(mudshutdown) {
		return true;
	}
	/* shutdownlevel 20 = avviso "entro 10 minuti" in check_reboot */
	return s_rebootSequenceStarted && s_shutdownLevel >= 20;
}

void check_reboot() {
	static time_t lastCheck=time(0);
	time_t tc;
	FILE* boot;
	static int TooMuchLag=-1;
	static int forceshutdown=0;
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
	struct tm now_tm {};
	localtime_r(&tc, &now_tm);
	if(forceshutdown) {
		s_shutdownLevel=25;
	}
	// If we already on a reboot sequence, checking is pointless
	if(!s_rebootSequenceStarted && (tc-lastCheck) >=60) {  //Once every minute
		update_max_usage();
		mudlog(LOG_CHECK,"Shutdown status: %d %d %d",s_shutdownLevel,s_rebootSequenceStarted,(tc-lastCheck));
		lastCheck=tc;
		/* REBOOT.NOW ha priorità (sequenza rapida); non marca gli slot giornalieri. */
		if((boot = fopen(kRebootNowFile, "r+"))) {
			fclose(boot);
			unlink(kRebootNowFile);
			s_rebootSequenceStarted=TRUE;
			s_shutdownLevel=19;
		}
		else if(scheduled_reboot_due(now_tm)) {
			s_rebootSequenceStarted=TRUE;
			s_shutdownLevel=0;
		}
	}
	else if(s_rebootSequenceStarted) {
		mudlog(LOG_CHECK,"Shutdown status: %d %d %d",s_shutdownLevel,s_rebootSequenceStarted,(tc-lastCheck));
		s_shutdownLevel+=((tc-lastCheck)/60);
		lastCheck=tc;
		if(s_shutdownLevel > 30) {
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
		else if(s_shutdownLevel <= 30) {
			if(s_shutdownLevel > 29) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira' entro un minuto!\n\r");
			}
			else if(s_shutdownLevel >= 28) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira' entro 2 minuti.\n\r");
			}
			else if(s_shutdownLevel >= 27) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira' entro 3 minuti.\n\r");
			}
			else if(s_shutdownLevel >= 26) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira' entro 4 minuti.\n\r");
			}
			else if(s_shutdownLevel >= 25) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira' entro 5 minuti.\n\r");
			}
			else if(s_shutdownLevel == 20) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira' entro 10 minuti.\n\r");
			}
			else if(s_shutdownLevel == 15) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira' entro 15 minuti.\n\r");
			}
			else if(s_shutdownLevel == 10) {
				send_to_all("$c0015ATTENZIONE! $c0014Nebbie Arcane ripartira' entro 20 minuti.\n\r");
			}
		}
	}
	if(TooMuchLag>10 && !forceshutdown) {
		send_to_all("$c0015ATTENZIONE! $c0014Lag eccessivo. Iniziata sequenza di shutdown!\n\r");
		s_rebootSequenceStarted=TRUE;
		forceshutdown=now_tm.tm_min;
		if(!forceshutdown) {
			forceshutdown=1;
		}
	}
	if(TooMuchLag<5 && forceshutdown) {

		send_to_all("$c0015ATTENZIONE! $c0014Lag risolto. Shutdown cancellato!\n\r");
		s_rebootSequenceStarted=FALSE;
		s_shutdownLevel=0;
		forceshutdown=0;
	}

	return;
}

} // namespace Alarmud

