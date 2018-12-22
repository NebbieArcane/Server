/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMud
* $Id: act.info.c,v 1.6 2002/03/11 21:15:20 Thunder Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <cstdint>
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
#include "act.info.hpp"
#include "act.off.hpp"
#include "act.wizard.hpp"
#include "breath.hpp"
#include "cmdid.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "modify.hpp"
#include "multiclass.hpp"
#include "parser.hpp"
#include "signals.hpp"
#include "skills.hpp"
#include "snew.hpp"
#include "spec_procs.hpp"
#include "spec_procs2.hpp"
#include "specialproc_other.hpp"
#include "spell_parser.hpp"
#include "spells.hpp"        // for spell_info_type, SKILL_EVALUATE, SPELL_G...
#include "Sql.hpp"
#include "trap.hpp"
#include "utility.hpp"
#include "version.hpp"
#include "vt100c.hpp"

namespace Alarmud {

int attrefzone=0;


int HowManyConnection(int ToAdd) {
	static int NumberOfConnection=0;
	NumberOfConnection+=ToAdd;
	return(NumberOfConnection);
}

int singular(struct obj_data* o) {

	if(IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS) ||
			IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET) ||
			IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS) ||
			IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS)) {
		return(FALSE);
	}
	return(TRUE);
}

/* Procedures related to 'look' */

void argument_split_2(const char* argument,char* first_arg,char* second_arg) {
	int look_at, begin;
	begin = 0;

	/* Find first non blank */
	for(; *(argument + begin) == ' ' ; begin++);

	/* Find length of first word */
	for(look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

		/* Make all letters lower case, AND copy them to first_arg */
	{
		*(first_arg + look_at) = LOWER(*(argument + begin + look_at));
	}
	*(first_arg + look_at) = '\0';
	begin += look_at;

	/* Find first non blank */
	for(; *(argument + begin) == ' ' ; begin++);

	/* Find length of second word */
	for(look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

		/* Make all letters lower case, AND copy them to second_arg */
	{
		*(second_arg + look_at) = LOWER(*(argument + begin + look_at));
	}
	*(second_arg + look_at)='\0';
	begin += look_at;
}

struct obj_data* get_object_in_equip_vis(struct char_data* ch,const char* arg, struct obj_data* equipment[], int* j) {

	for((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
		if(equipment[(*j) ])
			if(CAN_SEE_OBJ(ch, equipment[(*j) ]))
				if(isname(arg, equipment[(*j) ]->name) ||
						isname2(arg, equipment[(*j) ]->name)) {
					return(equipment[(*j) ]);
				}

	return(0);
}

char* find_ex_description(char* word, struct extra_descr_data* list) {
	struct extra_descr_data* i;

	if(word && *word) {
		for(i = list; i; i = i->next)
			if(i->keyword)
				if(isname(word, i->keyword)) {
					return(i->description);
				}

#if 0
		for(i = list; i; i = i->next)
			if(i->keyword)
				if(isname2(word, i->keyword)) {
					return(i->description);
				}
#endif
	}

	return NULL;
}


void show_obj_to_char(struct obj_data* object, struct char_data* ch, int mode) {
	char buffer[MAX_STRING_LENGTH];

	buffer[0] = 0;
	if((mode == 0) && object->description && *object->description) {
		strcpy(buffer, object->description);
        if(buffer[1] == '$')
            buffer[7] = UPPER(buffer[7]);
        else
            CAP(buffer);
	}
	else if(object->short_description &&
			(mode == 1 || mode == 2 || mode == 3 || mode == 4)) {
		strcpy(buffer,object->short_description);
        if(buffer[1] == '$')
            buffer[7] = UPPER(buffer[7]);
        else
            CAP(buffer);
	}
	else if(mode == 5) {
		if(object->obj_flags.type_flag == ITEM_NOTE) {
			if(object->action_description && *object->action_description) {
				strcpy(buffer, "C'e' scritto sopra qualcosa:\n\r\n\r");
				strcat(buffer, object->action_description);
				page_string(ch->desc, buffer, 1);
			}
			else {
				act("Non c'e' scritto nulla.", FALSE, ch, 0, 0, TO_CHAR);
			}
			return;  /* mail fix, thanks brett */

		}
		else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
			strcpy(buffer,"Non vedi nulla di speciale...");
		}
		else {
			/* ITEM_TYPE == ITEM_DRINKCON */
			strcpy(buffer, "Sembra un contenitore per bevande.");
		}
	}

	if(mode != 3) {
		if(IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
			strcat(buffer,"$c0014 (invisibile)$c0007");
		}
		if(IS_OBJ_STAT(object, ITEM_ANTI_GOOD) &&
				IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
			if(singular(object)) {
				strcat(buffer, "$c0014 (ha un alone di luce rossa)$c0007");
			}
			else {
				strcat(buffer,"$c0014 (hanno un alone di luce rossa)$c0007");
			}
		}
		if(IS_OBJ_STAT(object, ITEM_MAGIC) &&
				IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
			if(singular(object)) {
				strcat(buffer,"$c0014 (ha un alone di luce blu)$c0007");
			}
			else {
				strcat(buffer,"$c0014 (hanno un alone di luce blu)$c0007");
			}
		}
		if(IS_OBJ_STAT(object, ITEM_GLOW)) {
			if(singular(object)) {
				strcat(buffer, "$c0014 (ha un alone luminoso)$c0007");
			}
			else {
				strcat(buffer,"$c0014 (hanno un alone luminoso)$c0007");
			}
		}
		if(IS_OBJ_STAT(object,ITEM_HUM)) {
			if(singular(object)) {
				strcat(buffer,"$c0014 (emette un forte ronzio)$c0007");
			}
			else {
				strcat(buffer,"$c0014 (emettono un forte ronzio)$c0007");
			}
		}
		if(object->obj_flags.type_flag == ITEM_ARMOR) {
			if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 4)) {
				if(singular(object)) {
					strcat(buffer, "$c0009 (distrutto$c0007)");
				}
				else {
					strcat(buffer,"$c0009 (sono distrutti$c0007)");
				}
			}
			else if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 3)) {
				if(singular(object)) {
					strcat(buffer, "$c0009 (ha bisogno di essere riparato)$c0007");
				}
				else {
					strcat(buffer,"$c0009 (hanno bisogno di essere riparati)$c0007");
				}
			}
			else if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 2)) {
				if(singular(object)) {
					strcat(buffer, "$c0011 (in buone condizioni)$c0007");
				}
				else {
					strcat(buffer,"$c0011 (in buone condizioni)$c0007");
				}
			}
			else if(object->obj_flags.value[ 0 ] <
					object->obj_flags.value[1]) {
				if(singular(object)) {
					strcat(buffer, "$c0010 (in ottime condizioni)$c0007");
				}
				else {
					strcat(buffer,"$c0010 (in ottime condizioni)$c0007");
				}
			}
			else {
				if(singular(object)) {
					strcat(buffer, "$c0010 (in condizioni eccellenti)$c0007");
				}
				else {
					strcat(buffer,"$c0010 (in condizioni eccellenti)$c0007");
				}
			}
		}
	}
	if(buffer[ 0 ]) {
		strcat(buffer, "\n\r");
		page_string(ch->desc, buffer, 1);
	}
}

void show_mult_obj_to_char(struct obj_data* object, struct char_data* ch,
						   int mode, int num) {
	char buffer[ MAX_STRING_LENGTH ];
	char tmp[ 10 ];

	buffer[0] = 0;
	tmp[0] = 0;

	if((mode == 0) && object->description && *object->description) {
		strcpy(buffer,object->description);
        if(buffer[1] == '$')
            buffer[7] = UPPER(buffer[7]);
        else
            CAP(buffer);
	}
	else if(object->short_description && ((mode == 1) ||
										  (mode == 2) || (mode == 3) || (mode == 4))) {
		strcpy(buffer, object->short_description);
        if(buffer[1] == '$')
            buffer[7] = UPPER(buffer[7]);
        else
            CAP(buffer);
	}
	else if(mode == 5) {
		if(object->obj_flags.type_flag == ITEM_NOTE) {
			if(object->action_description && *object->action_description) {
				strcpy(buffer, "C'e' scritto sopra qualcosa:\n\r\n\r");
				strcat(buffer, object->action_description);
				page_string(ch->desc, buffer, 1);
			}
			else {
				act("Non c'e' scritto nulla.", FALSE, ch, 0, 0, TO_CHAR);
			}
			return;
		}
		else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
			strcpy(buffer,"Non vedi nulla di speciale...");
		}
		else {
			/* ITEM_TYPE == ITEM_DRINKCON */
			strcpy(buffer, "Sembra un contenitore per bevande.");
		}
	}

	if(mode != 3) {
		if(IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
			strcat(buffer,"(invisibile)");
		}
		if(IS_OBJ_STAT(object, ITEM_ANTI_GOOD) &&
				IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
			strcat(buffer,"...Ha un alone di luce rossa!");
		}
		if(IS_OBJ_STAT(object,ITEM_MAGIC) &&
				IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
			strcat(buffer,"...Ha un alone di luce blue!");
		}
		if(IS_OBJ_STAT(object, ITEM_GLOW)) {
			strcat(buffer,"...Ha un alone luminoso!");
		}
		if(IS_OBJ_STAT(object,ITEM_HUM)) {
			strcat(buffer,"...Emette un forte ronzio!");
		}
	}

	if(num > 1) {
		snprintf(tmp,9,"[%d]", num);
		strcat(buffer, tmp);
	}
	if(buffer[ 0 ]) {
		strcat(buffer, "\n\r");
		page_string(ch->desc, buffer, 1);
	}
}

void list_obj_in_room(struct obj_data* list, struct char_data* ch) {
	struct obj_data* i, *cond_ptr[50];
	int Inventory_Num = 1;
	int k, cond_top, cond_tot[50], found=FALSE;
	char buf[MAX_STRING_LENGTH];

	cond_top = 0;

	for(i=list; i; i = i->next_content) {
		if(CAN_SEE_OBJ(ch, i)) {
			if(cond_top < 50) {
				found = FALSE;
				for(k=0; k < cond_top && !found; k++) {
					if(cond_top > 0) {
						if((i->item_number == cond_ptr[ k ]->item_number) &&
								(i->description && *i->description &&
								 cond_ptr[ k ]->description &&
								 ! strcmp(i->description, cond_ptr[ k ]->description))) {
							cond_tot[ k ] += 1;
							found=TRUE;
						}
					}
				}
				if(!found) {
					cond_ptr[ cond_top ] = i;
					cond_tot[ cond_top ] = 1;
					cond_top += 1;
				}
			}
			else {
				if((ITEM_TYPE(i) == ITEM_TRAP) || (GET_TRAP_CHARGES(i) > 0)) {
					if(CAN_SEE_OBJ(ch,i)) {
						show_obj_to_char(i, ch, 0);
					}
				} /* not a trap */
				else {
					show_obj_to_char(i,ch,0);
				}
			}
		}
	} /* for */

	if(cond_top) {
		for(k=0; k < cond_top; k++) {
			if((ITEM_TYPE(cond_ptr[ k ]) == ITEM_TRAP) &&
					(GET_TRAP_CHARGES(cond_ptr[ k ]) > 0)) {
				if(CAN_SEE_OBJ(ch,cond_ptr[ k ])) {
					/* Credo che l'if qui sopra sia inutile. */
					if(cond_tot[ k ] > 1) {
						snprintf(buf, MAX_STRING_LENGTH-1,"[%2d] ", Inventory_Num++);
						send_to_char(buf,ch);
						show_mult_obj_to_char(cond_ptr[ k ], ch, 0, cond_tot[ k ]);
					}
					else {
						show_obj_to_char(cond_ptr[ k ], ch, 0);
					}
				}
			}
			else {
				if(cond_tot[ k ] > 1) {
					snprintf(buf, MAX_STRING_LENGTH-1,"[%2d] ", Inventory_Num++);
					send_to_char(buf, ch);
					show_mult_obj_to_char(cond_ptr[ k ], ch, 0, cond_tot[ k ]);
				}
				else {
					show_obj_to_char(cond_ptr[ k ], ch, 0);
				}
			}
		}
	}
}


void list_obj_in_heap(struct obj_data* list, struct char_data* ch) {
	struct obj_data* i, *cond_ptr[50];
	int k, cond_top, cond_tot[50], found=FALSE;
	char buf[MAX_STRING_LENGTH];

	int Num_Inventory = 1;
	cond_top = 0;

	for(i=list; i; i = i->next_content) {
		if(CAN_SEE_OBJ(ch, i)) {
			if(cond_top < 50) {
				found = FALSE;
				for(k=0; k < cond_top && !found ; k++) {
					if(cond_top > 0) {
						if((i->item_number == cond_ptr[ k ]->item_number) &&
								(i->short_description && cond_ptr[ k ]->short_description &&
								 (!strcmp(i->short_description,
										  cond_ptr[ k ]->short_description)))) {
							cond_tot[ k ] += 1;
							found=TRUE;
						}
					}
				}
				if(!found) {
					cond_ptr[ cond_top ] = i;
					cond_tot[ cond_top ] = 1;
					cond_top += 1;
				}
			}
			else {
				show_obj_to_char(i, ch, 2);
			}
		} /* else can't see */
	} /* for */

	if(cond_top) {
		for(k=0; k < cond_top; k++) {
			snprintf(buf, MAX_STRING_LENGTH-1, "[%2d] ", Num_Inventory++);
			send_to_char(buf, ch);
			if(cond_tot[ k ] > 1) {
				Num_Inventory += cond_tot[ k ] - 1;
				show_mult_obj_to_char(cond_ptr[ k ], ch, 2, cond_tot[ k ]);
			}
			else {
				show_obj_to_char(cond_ptr[ k ], ch, 2);
			}
		}
	}
}

void list_obj_to_char(struct obj_data* list, struct char_data* ch, int mode,
					  bool show) {
	char buf[MAX_STRING_LENGTH];
	int Num_In_Bag = 1;
	struct obj_data* i;
	bool found;

	found = FALSE;
	for(i = list ; i ; i = i->next_content) {
		if(CAN_SEE_OBJ(ch, i)) {
			snprintf(buf, MAX_STRING_LENGTH-1, "[%2d] ", Num_In_Bag++);
			send_to_char(buf, ch);
			show_obj_to_char(i, ch, mode);
			found = TRUE;
		}
	}
	if((!found) && (show)) {
		send_to_char("Nulla\n\r", ch);
	}
}


void ShowAltezzaCostituzione(struct char_data* pChar, struct char_data* pTo) {
	float fRapp;
	char szBuf[ 256 ];

	const char* DescAltezze[] = {
		"altissim$b",
		"molto alt$b",
		"alt$b",
		"di altezza media",
		"bass$b",
		"molto bass$b"
	};

	const char* DescCostituzione[] = {
		"$c0010VERAMENTE gross$b$c0007",
		"gross$b",
		"robust$b",
		"di costituzione media",
		"magr$b",
		"gracile"
	};
	if(!pChar || !pTo) {
		mudlog(LOG_SYSERR,
			   "!pChar || !pTo in ShowAltezzaCostituzione (act.info.c).");
		return;
	}

	if(!GET_WEIGHT(pChar) || ! GET_HEIGHT(pChar)) {
		mudlog(LOG_ERROR,
			   "L'altezza od il peso di %s e' a zero.", GET_NAME_DESC(pChar));
		return;
	}

	if(!IsHumanoid(pChar)) {
		return;
	}

	strcpy(szBuf, "$n e' ");

	if(pChar->player.height > 250) {
		strcat(szBuf, DescAltezze[ 0 ]);
	}
	else if(pChar->player.height > 190) {
		strcat(szBuf, DescAltezze[ 1 ]);
	}
	else if(pChar->player.height > 170) {
		strcat(szBuf, DescAltezze[ 2 ]);
	}
	else if(pChar->player.height > 155) {
		strcat(szBuf, DescAltezze[ 3 ]);
	}
	else if(pChar->player.height > 140) {
		strcat(szBuf, DescAltezze[ 4 ]);
	}
	else {
		strcat(szBuf, DescAltezze[ 5 ]);
	}

	strcat(szBuf, " e ");

	fRapp = (float)GET_HEIGHT(pChar) /
			((float)pChar->player.weight * 0.4536);


	if(fRapp > 3.27) {
		strcat(szBuf, DescCostituzione[ 5 ]);
	}
	else if(fRapp > 3) {
		strcat(szBuf, DescCostituzione[ 4 ]);
	}
	else if(fRapp > 2.25) {
		strcat(szBuf, DescCostituzione[ 3 ]);
	}
	else if(fRapp > 2) {
		strcat(szBuf, DescCostituzione[ 2 ]);
	}
	else if(fRapp > 1.6) {
		strcat(szBuf, DescCostituzione[ 1 ]);
	}
	else {
		strcat(szBuf, DescCostituzione[ 0 ]);
	}

	strcat(szBuf, ".");
	act(szBuf, FALSE, pChar, NULL, pTo, TO_VICT);
}

void show_char_to_char(struct char_data* i, struct char_data* ch, int mode) {
	char buffer[MAX_STRING_LENGTH],buffer2[MAX_STRING_LENGTH];
	int j, found, percent, otype;
	struct obj_data* tmp_obj;
	struct affected_type* aff;



	if(!ch || !i) {
		mudlog(LOG_SYSERR, "!ch || !i in act.info.c show_char_to_char");
		return;
	}

	if(mode == 0) {

		if(IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) {
			if(IS_AFFECTED(ch, AFF_SENSE_LIFE) && !IS_IMMORTAL(i)) {
				send_to_char("Percepisci una forma di vita invisibile nella "
							 "stanza.\n\r", ch);
				return;
			}
			else {
				/* no see nothing */
				return;
			}
		}

		if(!(i->player.long_descr) ||
				(GET_POS(i) != i->specials.default_pos)) {
			/* A player char or a mobile without long descr, or not in default pos.*/
			if(!IS_NPC(i)) {
				strcpy(buffer, GET_NAME(i));
				strcat(buffer, " ");
				if(GET_TITLE(i)) {
					strcat(buffer, GET_TITLE(i));
				}
			}
			else {
				strcpy(buffer, i->player.short_descr);
                if(buffer[1] == '$')
                    buffer[7] = UPPER(buffer[7]);
                else
                    CAP(buffer);
			}

			if(IS_AFFECTED(i, AFF_INVISIBLE) || i->invis_level >= IMMORTALE) {
				strcat(buffer," (invisibile)");
			}
			if(IS_AFFECTED(i, AFF_CHARM)) {
				strcat(buffer," (schiavo)");
			}

			switch(GET_POS(i)) {
			case POSITION_STUNNED:
				strcat(buffer," giace qui, svenut$b.");
				break;
			case POSITION_INCAP:
				strcat(buffer," giace qui, incapacitat$b.");
				break;
			case POSITION_MORTALLYW:
				strcat(buffer," giace qui, ferit$b a morte.");
				break;
			case POSITION_DEAD:
				strcat(buffer," giace qui, mort$b.");
				break;
			case POSITION_MOUNTED:
				if(MOUNTED(i)) {
					strcat(buffer, " e' qui, cavalcando ");
					strcat(buffer, MOUNTED(i)->player.short_descr);
				}
				else {
					strcat(buffer, " e' qui, in piedi.");
				}
				break;
			case POSITION_STANDING :
				if(!IS_AFFECTED(i, AFF_FLYING) &&
						!affected_by_spell(i,SKILL_LEVITATION)) {
					if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
						strcat(buffer, " sta galleggiando qui.");
					}
					else {
						strcat(buffer," e' qui, in piedi.");
					}
				}
				else {
					strcat(buffer," vola qui intorno.");
				}
				break;
			case POSITION_SITTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					strcat(buffer, " sta galleggiando qui.");
				}
				else {
					strcat(buffer," e' sedut$b qui.");
				}
				break;
			case POSITION_RESTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					strcat(buffer, " sta facendo il morto nell'acqua.");
				}
				else {
					strcat(buffer," sta riposando qui.");
				}
				break;
			case POSITION_SLEEPING :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					strcat(buffer, " sta dormendo qui nell'acqua.");
				}
				else {
					strcat(buffer," sta dormendo qui.");
				}
				break;
			case POSITION_FIGHTING :
				if(i->specials.fighting) {
					strcat(buffer," e' qui, combattendo contro ");
					if(i->specials.fighting == ch) {
						strcat(buffer," DI TE!");
					}
					else {
						if(i->in_room == i->specials.fighting->in_room) {
							if(IS_NPC(i->specials.fighting)) {
								strcat(buffer, i->specials.fighting->player.short_descr);
							}
							else {
								strcat(buffer, GET_NAME(i->specials.fighting));
							}
						}
						else {
							strcat(buffer, "qualcuno che se ne e' appena andato.");
						}
					}
				}
				else { /* NIL fighting pointer */
					strcat(buffer," e' qui dimenandosi contro l'aria.");
				}
				break;
			default:
				strcat(buffer," fluttua qui intorno.");
				break;
			} /*switch */

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				strcat(buffer, "$c0006 (AFK)$c0007");
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if(IS_EVIL(i)) {
					strcat(buffer, "$c0009 (alone rosso)");
				}
			}

			if(IS_LINKDEAD(i)) {
				strcat(buffer, "$c0015 (link dead)$c0007");
			}

			act(buffer, FALSE, i, 0, ch, TO_VICT);
			if(!IS_AFFECTED(ch,AFF_TRUE_SIGHT)) {
				for(j=how_many_spell(i, SPELL_MIRROR_IMAGES); j>0; j--) {
					act(buffer,FALSE,i,0,ch,TO_VICT);
				}
			}

		}
		else {
			/* npc with long */

			if(IS_AFFECTED(i, AFF_INVISIBLE)) {
				strcpy(buffer,"*");
			}
			else {
				*buffer = '\0';
			}

			if(IS_LINKDEAD(i)) {
				strcat(buffer, "$c0015 (link dead)$c0007");
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if(IS_EVIL(i)) {
					strcat(buffer, "$c0009 (alone rosso)$c0007");
				}
			}


			if(IS_AFFECTED2(i, AFF2_AFK)) {
				strcat(buffer,"$c0006 (AFK)$c0007");
			}

			strcat(buffer, i->player.long_descr);

			/* strip \n\r's off */
			while((buffer[ strlen(buffer) - 1 ] == '\r') ||
					(buffer[ strlen(buffer) - 1 ] == '\n') ||
					(buffer[ strlen(buffer) - 1 ] == ' ')) {
				buffer[ strlen(buffer) - 1 ] = '\0';
			}
			act(buffer, FALSE, i, 0, ch, TO_VICT);
		}

        if(IS_PC(i))
        {
            strcpy(buffer2, i->player.name);
        }
        else
        {
            strcpy(buffer2, i->player.short_descr);
            RemColorString(buffer2);
            CAP(buffer2);
        }
        
		if(IS_AFFECTED(i, AFF_SANCTUARY))
        {
			if(!IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
            {
                sprintf(buffer,"$c0015");
                strcat(buffer,buffer2);
                strcat(buffer, " brilla di luce propria!");
				act(buffer, FALSE, i, 0, ch, TO_VICT);
			}
		}

		if(IS_AFFECTED(i, AFF_GROWTH))
        {
            sprintf(buffer,"$c0003");
            strcat(buffer,buffer2);
			strcat(buffer, " e' enorme!");
            act(buffer, FALSE, i, 0, ch, TO_VICT);
		}

		if(IS_AFFECTED(i, AFF_FIRESHIELD))
        {
			if(!IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
            {
                sprintf(buffer,"$c0001");
                strcat(buffer,buffer2);
                strcat(buffer, " e' avvolt$b in una luce fiammeggiante!");
                act(buffer, FALSE, i, 0, ch, TO_VICT);
            }
		}

		if(IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
        {
            sprintf(buffer,"$c0008");
            strcat(buffer,buffer2);
            strcat(buffer, " e' avvolt$b nell'oscurita'!");
            act(buffer, FALSE, i, 0, ch, TO_VICT);
		}

	}
	else if(mode == 1) {
		if(i->player.description) {
            strcpy(buffer, i->player.description);
            if(buffer[1] == '$')
                buffer[7] = UPPER(buffer[7]);
            else
                CAP(buffer);
            send_to_char(buffer, ch);
		}
		else {
			if(IS_MAESTRO_DEL_CREATO(i)) {
				act("Una pulsante aura di potere avvolge $n.", FALSE, i, 0, ch, TO_VICT);
			}
			else if(IS_MAESTRO_DEGLI_DEI(i))
				act("$n appartiene alla cerchia dei Demiurghi.",
					FALSE, i, 0, ch, TO_VICT);
			else if(IS_DIO(i))
				act("$n appartiene alla cerchia degli Dei.",
					FALSE, i, 0, ch, TO_VICT);
			else if(IS_IMMORTALE(i))
				act("$n appartiene ai ranghi degli Immortali.",
					FALSE, i, 0, ch, TO_VICT);
			else if(IS_PRINCE(i))
				act("$n appartiene ai ranghi dei Principi.",
					FALSE, i, 0, ch, TO_VICT);
			else {
				act("$n e' mortale.", FALSE, i, 0, ch, TO_VICT);
			}

		}
		if(HAS_PRINCE(i)) {
			snprintf(buffer,MAX_STRING_LENGTH-1,"$n appartiene al clan di %s.",GET_PRINCE(i));
			act(buffer,FALSE,i,0,ch,TO_VICT);
		}

		/*
		 * personal descriptions.
		 */

		ShowAltezzaCostituzione(i, ch);

		if(IS_PC(i)) {
			snprintf(buffer,MAX_STRING_LENGTH-1, "$n e' un$b %s", RaceName[ GET_RACE(i) ]);
			act(buffer, FALSE, i, 0, ch, TO_VICT);
		}


		if(MOUNTED(i)) {
			snprintf(buffer,MAX_STRING_LENGTH-1,"$n e' sopra a %s", MOUNTED(i)->player.short_descr);
			act(buffer, FALSE, i, 0, ch, TO_VICT);
		}

		if(RIDDEN(i)) {
			snprintf(buffer,MAX_STRING_LENGTH-1,"$n e' cavalcat$b da %s",
					 IS_NPC(RIDDEN(i)) ? RIDDEN(i)->player.short_descr :
					 GET_NAME(RIDDEN(i)));
			act(buffer, FALSE, i, 0, ch, TO_VICT);
		}

		/* Show a character to another */

		if(GET_MAX_HIT(i) > 0) {
			percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
		}
		else {
			percent = -1;    /* How could MAX_HIT be < 1?? */
		}

		if(IS_NPC(i)) {
			strcpy(buffer, i->player.short_descr);
            if(buffer[1] == '$')
                buffer[7] = UPPER(buffer[7]);
            else
                CAP(buffer);
		}
		else {
			strcpy(buffer, GET_NAME(i));
		}

		if(percent >= 100) {
			strcat(buffer, " e' in condizioni eccellenti.");
		}
		else if(percent >= 80) {
			strcat(buffer, " ha pochi graffi.");
		}
		else if(percent >= 60) {
			strcat(buffer, " ha alcuni tagli ed abrasioni.");
		}
		else if(percent >= 40) {
			strcat(buffer, " e' ferit$B.");
		}
		else if(percent >= 20) {
			strcat(buffer, " sanguina abbondatemente.");
		}
		else if(percent >= 0) {
			strcat(buffer, " $c0001ha grossi squarci aperti.");
		}
		else {
			strcat(buffer, " $c0009sta morendo per le ferite ed i colpi ricevuti.");
		}

		act(buffer,FALSE, ch,0,i,TO_CHAR);


		/*
		 * spell_descriptions, etc.
		 */
		otype = -1;
		for(aff = i->affected; aff; aff = aff->next) {
			if(aff->type < MAX_EXIST_SPELL) {
				if(spell_desc[ aff->type ] && *spell_desc[ aff->type ]) {
					if(aff->type != otype) {
						act(spell_desc[ aff->type ], FALSE, i, 0, ch, TO_VICT);
						otype = aff->type;
					}
				}
			}
		}


		found = FALSE;
		for(j=0; j< MAX_WEAR; j++) {
			if(i->equipment[ j ]) {
				if(CAN_SEE_OBJ(ch,i->equipment[ j ])) {
					found = TRUE;
				}
			}
		}
		if(found) {
			act("\n\r$n sta usando:", FALSE, i, 0, ch, TO_VICT);
			for(j=0; j< MAX_WEAR; j++) {
				if(i->equipment[ j ]) {
					if(CAN_SEE_OBJ(ch, i->equipment[ j ])) {
						send_to_char(eqWhere[ j ], ch);
						show_obj_to_char(i->equipment[ j ], ch, 1);
					}
				}
			}
		}
		if(HasClass(ch, CLASS_THIEF) && (ch != i) && (!IS_IMMORTAL(ch))) {
			found = FALSE;
			send_to_char("\n\rGuardandogli nelle tasche, vedi:\n\r", ch);
			for(tmp_obj = i->carrying; tmp_obj;
					tmp_obj = tmp_obj->next_content) {
				if(CAN_SEE_OBJ(ch, tmp_obj) &&
						(number(0, MAX_MORT) < GetMaxLevel(ch))) {
					show_obj_to_char(tmp_obj, ch, 1);
					found = TRUE;
				}
			}
			if(!found) {
				send_to_char("Nulla.\n\r", ch);
			}
		}
		else if(IS_IMMORTAL(ch)) {
			send_to_char("Inventario:\n\r",ch);
			for(tmp_obj = i->carrying; tmp_obj;
					tmp_obj = tmp_obj->next_content) {
				show_obj_to_char(tmp_obj, ch, 1);
				found = TRUE;
			}
			if(!found) {
				send_to_char("Nulla\n\r",ch);
			}
		}
	}
	else if(mode == 2) {
		/* Lists inventory */
		act("$n sta trasportando:", FALSE, i, 0, ch, TO_VICT);
		list_obj_in_heap(i->carrying, ch);
	}
}


void show_mult_char_to_char(struct char_data* i, struct char_data* ch,
							int mode, int num) {
	char buffer[MAX_STRING_LENGTH], buffer2[MAX_STRING_LENGTH];
	char tmp[10];
	int j, found, percent;
	struct obj_data* tmp_obj;

	if(mode == 0) {
		if(IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) {
			if(IS_AFFECTED(ch, AFF_SENSE_LIFE) || IS_IMMORTAL(i)) {
				if(num == 1)
					act("$c0002Percepisci una forma di vita invisibile qui intorno.",
						FALSE, ch,0,0,TO_CHAR);
				else
					act("$c0002Percepisci alcune forme di vita invisibile qui intorno.",
						FALSE, ch,0,0,TO_CHAR);
				return;
			}
			else {
				/* no see nothing */
				return;
			}
		}

		if(!(i->player.long_descr) ||
				(GET_POS(i) != i->specials.default_pos)) {
			/* A player char or a mobile without long descr, or not in default pos. */
			if(!IS_NPC(i)) {
				strcpy(buffer, GET_NAME(i));
				strcat(buffer," ");
				if(GET_TITLE(i)) {
					strcat(buffer,GET_TITLE(i));
				}
			}
			else {
				strcpy(buffer, i->player.short_descr);
                if(buffer[1] == '$')
                    buffer[7] = UPPER(buffer[7]);
                else
                    CAP(buffer);
			}

			if(IS_AFFECTED(i,AFF_INVISIBLE)) {
				strcat(buffer,"$c0011 (invisibile)");
			}
			if(IS_AFFECTED(i, AFF_CHARM)) {
				strcat(buffer,"$c0010 (schiavo)");
			}

			switch(GET_POS(i)) {
			case POSITION_STUNNED:
				strcat(buffer," giace qui, svenut$b.");
				break;
			case POSITION_INCAP:
				strcat(buffer," giace qui, incapacitat$b.");
				break;
			case POSITION_MORTALLYW:
				strcat(buffer," giace qui, ferit$b a morte.");
				break;
			case POSITION_DEAD:
				strcat(buffer," giace qui, mort$b.");
				break;
			case POSITION_MOUNTED:
				if(MOUNTED(i)) {
					strcat(buffer, " e' qui, montando ");
					strcat(buffer, MOUNTED(i)->player.short_descr);
				}
				else {
					strcat(buffer, " e' qui, in piedi.");
				}
				break;
			case POSITION_STANDING :
				if(!IS_AFFECTED(i, AFF_FLYING) &&
						!affected_by_spell(i,SKILL_LEVITATION)) {
					if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
						strcat(buffer, " sta galleggiano qui.");
					}
					else {
						strcat(buffer," e' qui, in piedi.");
					}
				}
				else {
					strcat(buffer," vola qui intorno.");
				}
				break;
			case POSITION_SITTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					strcat(buffer, " sta galleggiando qui.");
				}
				else {
					strcat(buffer," e' sedut$b qui.");
				}
				break;
			case POSITION_RESTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					strcat(buffer, " sta riposando qui nell'acqua.");
				}
				else {
					strcat(buffer," sta riposando qui.");
				}
				break;
			case POSITION_SLEEPING :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					strcat(buffer, " sta dormendo qui nell'acqua.");
				}
				else {
					strcat(buffer," sta dormendo qui.");
				}
				break;
			case POSITION_FIGHTING :
				if(i->specials.fighting) {
					strcat(buffer," e' qui, combattendo contro ");
					if(i->specials.fighting == ch) {
						strcat(buffer," DI TE!");
					}
					else {
						if(i->in_room == i->specials.fighting->in_room) {
							if(IS_NPC(i->specials.fighting)) {
								strcat(buffer, i->specials.fighting->player.short_descr);
							}
							else {
								strcat(buffer, GET_NAME(i->specials.fighting));
							}
						}
						else {
							strcat(buffer, "qualcuno che se ne e' appena andato.");
						}
					}
				}
				else { /* NIL fighting pointer */
					strcat(buffer," e' qui dimenandosi contro l'aria.");
				}
				break;
			default:
				strcat(buffer," fluttua qui intorno.");
				break;
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if(IS_EVIL(i)) {
					strcat(buffer, "$c0009 (alone rosso)");
				}
			}

			if(IS_LINKDEAD(i)) {
				strcat(buffer, "$c0015 (link dead)$c0007");
			}

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				strcat(buffer,"$c0006 (AFK)$c0007");
			}

			if(num > 1) {
				snprintf(tmp,9," [%d]", num);
				strcat(buffer, tmp);
			}
			act(buffer,FALSE, i, 0, ch, TO_VICT);
		}
		else {
			/* npc with long */

			if(IS_AFFECTED(i, AFF_INVISIBLE)) {
				strcpy(buffer,"*");
			}
			else {
				*buffer = '\0';
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if(IS_EVIL(i)) {
					strcat(buffer, "$c0009 (alone rosso)");
				}
			}

			if(IS_LINKDEAD(i)) {
				strcat(buffer, "$c0015 (link dead)$c0007");
			}

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				strcat(buffer, "$c0006 (AFK)$c0007");
			}

			strcat(buffer, i->player.long_descr);

			/* this gets a little annoying */

			if(num > 1) {
				while((buffer[ strlen(buffer) - 1 ] == '\r') ||
						(buffer[ strlen(buffer) - 1 ] == '\n') ||
						(buffer[ strlen(buffer) - 1 ] == ' ')) {
					buffer[ strlen(buffer) - 1 ] = '\0';
				}
				snprintf(tmp,9, " [%d]", num);
				strcat(buffer, tmp);
			}

			act(buffer, FALSE, i, 0, ch, TO_VICT);
		}

        if(IS_PC(i))
        {
            strcpy(buffer2, i->player.name);
        }
        else
        {
            strcpy(buffer2, i->player.short_descr);
            RemColorString(buffer2);
            CAP(buffer2);
        }
        
        if(IS_AFFECTED(i, AFF_SANCTUARY))
        {
            if(!IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
            {
                sprintf(buffer,"$c0015");
                strcat(buffer,buffer2);
                strcat(buffer, " brilla di luce propria!");
                act(buffer, FALSE, i, 0, ch, TO_VICT);
            }
        }
        
        if(IS_AFFECTED(i, AFF_GROWTH))
        {
            sprintf(buffer,"$c0003");
            strcat(buffer,buffer2);
            strcat(buffer, " e' enorme!");
            act(buffer, FALSE, i, 0, ch, TO_VICT);
        }
        
        if(IS_AFFECTED(i, AFF_FIRESHIELD))
        {
            if(!IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
            {
                sprintf(buffer,"$c0001");
                strcat(buffer,buffer2);
                strcat(buffer, " e' avvolt$b in una luce fiammeggiante!");
                act(buffer, FALSE, i, 0, ch, TO_VICT);
            }
        }
        
        if(IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
        {
            sprintf(buffer,"$c0008");
            strcat(buffer,buffer2);
            strcat(buffer, " e' avvolt$b nell'oscurita'!");
            act(buffer, FALSE, i, 0, ch, TO_VICT);
        }
        
	}
	else if(mode == 1) {
		if(i->player.description) {
			send_to_char(i->player.description, ch);
		}
		else {
			act("Non vedi nulla di speciale in $n.", FALSE, i, 0, ch, TO_VICT);
		}

		ShowAltezzaCostituzione(i, ch);

		/* Show a character to another */

		if(GET_MAX_HIT(i) > 0) {
			percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
		}
		else {
			percent = -1;    /* How could MAX_HIT be < 1?? */
		}

		if(IS_NPC(i)) {
			strcpy(buffer, i->player.short_descr);
		}
		else {
			strcpy(buffer, GET_NAME(i));
		}

		if(percent >= 100) {
			strcat(buffer, " e' in condizioni eccellenti.");
		}
		else if(percent >= 80) {
			strcat(buffer, " ha pochi graffi.");
		}
		else if(percent >= 60) {
			strcat(buffer, " ha alcuni tagli ed abrasioni.");
		}
		else if(percent >= 40) {
			strcat(buffer, " e' ferit$B.");
		}
		else if(percent >= 20) {
			strcat(buffer, " sanguina abbondatemente.");
		}
		else if(percent >= 0) {
			strcat(buffer, " $c0001ha grossi squarci aperti.");
		}
		else {
			strcat(buffer, " $c0009sta morendo per le ferite ed i colpi ricevuti.");
		}

		act(buffer, FALSE, ch, 0, i, TO_CHAR);

		found = FALSE;
		for(j=0; j< MAX_WEAR; j++) {
			if(i->equipment[ j ]) {
				if(CAN_SEE_OBJ(ch, i->equipment[ j ])) {
					found = TRUE;
				}
			}
		}
		if(found) {
			act("\n\r$n sta usando:", FALSE, i, 0, ch, TO_VICT);
			for(j=0; j< MAX_WEAR; j++) {
				if(i->equipment[ j ]) {
					if(CAN_SEE_OBJ(ch,i->equipment[j])) {
						send_to_char(eqWhere[j], ch);
						show_obj_to_char(i->equipment[ j ], ch, 1);
					}
				}
			}
		}
		if((HasClass(ch, CLASS_THIEF)) && (ch != i)) {
			found = FALSE;
			send_to_char("\n\rGuardandogli nelle tasche, vedi:\n\r", ch);
			for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
				if(CAN_SEE_OBJ(ch, tmp_obj) &&
						(number(0, MAX_MORT) < GetMaxLevel(ch))) {
					show_obj_to_char(tmp_obj, ch, 1);
					found = TRUE;
				}
			}
			if(!found) {
				send_to_char("Nulla.\n\r", ch);
			}
		}
	}
	else if(mode == 2) {

		/* Lists inventory */
		act("$n sta trasportando:", FALSE, i, 0, ch, TO_VICT);
		list_obj_in_heap(i->carrying, ch);
	}
}


void list_char_in_room(struct char_data* list, struct char_data* ch) {
	struct char_data* i, *cond_ptr[50];
	int k, cond_top, cond_tot[50], found=FALSE;

	cond_top = 0;

	for(i=list; i; i = i->next_in_room) {
		if((ch!=i) && (!RIDDEN(i)) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
									   (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE)))) {
			if((cond_top< 50) && !MOUNTED(i)) {
				found = FALSE;
				if(IS_NPC(i)) {
					for(k=0; (k<cond_top&& !found); k++) {
						if(cond_top>0) {
							if(i->nr == cond_ptr[k]->nr &&
									(GET_POS(i) == GET_POS(cond_ptr[k])) &&
									(i->specials.affected_by==cond_ptr[k]->specials.affected_by) &&
									(i->specials.fighting == cond_ptr[k]->specials.fighting) &&
									(i->player.short_descr && cond_ptr[k]->player.short_descr &&
									 0==strcmp(i->player.short_descr,cond_ptr[k]->player.short_descr))) {
								cond_tot[k] += 1;
								found=TRUE;
							}
						}
					}
				}
				if(!found) {
					cond_ptr[cond_top] = i;
					cond_tot[cond_top] = 1;
					cond_top+=1;
				}
			}
			else {
				show_char_to_char(i,ch,0);
			}
		}
	}

	if(cond_top) {
		for(k=0; k<cond_top; k++) {
			if(cond_tot[k] > 1) {
				show_mult_char_to_char(cond_ptr[k],ch,0,cond_tot[k]);
			}
			else {
				show_char_to_char(cond_ptr[k],ch,0);
			}
		}
	}
}


void list_char_to_char(struct char_data* list, struct char_data* ch,
					   int mode) {
	struct char_data* i;

	for(i = list; i ; i = i->next_in_room) {
		if((ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
					   (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE)))) {
			show_char_to_char(i,ch,0);
		}
	}
}

/* Added by Mike Wilson 9/23/93 */

void list_exits_in_room(struct char_data* ch) {
	int door,seeit=FALSE;
	char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
	struct room_direction_data*        exitdata;

	*buf = '\0';

	for(door = 0; door <= 5; door++) {
		exitdata = EXIT(ch,door);
		if(exitdata) {
			if(real_roomp(exitdata->to_room)) {
				if(GET_RACE(ch) == RACE_ELVEN ||
						GET_RACE(ch) == RACE_GOLD_ELF ||
						GET_RACE(ch) == RACE_WILD_ELF ||
						GET_RACE(ch) == RACE_SEA_ELF)
					/* elves can see secret doors 1-3 on d6 */
				{
					seeit=(number(1,6)<=3);
				}
				else if(GET_RACE(ch)==RACE_HALF_ELVEN)
					/* half-elves can see exits, not as good as full */
				{
					seeit=(number(1,6)<=2);
				}
				else if(GET_RACE(ch)==RACE_DWARF ||
						GET_RACE(ch)==RACE_DARK_DWARF) {
					seeit=(number(1,12)<=7);
				}
				/* I nani le vedono meglio di tutti!!!! */
				else {
					seeit=FALSE;
				}

				if(exitdata->to_room != NOWHERE || IS_IMMORTAL(ch)) {
					if((!IS_SET(exitdata->exit_info, EX_CLOSED) ||
							IS_IMMORTAL(ch)) ||
							(IS_SET(exitdata->exit_info, EX_SECRET) && seeit)) {
						snprintf(buf2, MAX_STRING_LENGTH-1," %s", listexits[ door ]);
						strcat(buf,buf2);
						if(IS_SET(exitdata->exit_info, EX_CLOSED) && IS_IMMORTAL(ch)) {
							strcat(buf, " (chiuso)");
						}
						if(IS_SET(exitdata->exit_info, EX_SECRET) && (seeit ||
								IS_IMMORTAL(ch))) {
							strcat(buf, " $c5009(segreto)$c0007");    /* blink red */
						}
					} /* exit */
				} /* ! = NOWHERE */
			}    /* real_roomp */
		} /* exitdata */
	} /* for */


	if(*buf) {
		send_to_char("Uscite:", ch);
		act(buf, FALSE, ch, 0, 0, TO_CHAR);
	}
}


ACTION_FUNC(do_look) {
	char buffer[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int keyword_no, res;
	int j, bits, temp;
	struct room_data* roomp;
	bool found;
	struct obj_data* tmp_object, *found_object;
	struct char_data* tmp_char;
	char* tmp_desc;

	static int bNotShowTitle = 0;
	struct room_data* pRoomWithChar;

	const char* blood_messages[] = {
		"$c0009Non si deve vedere questo.",
		"$c0009C'e' un po di sangue qui a terra.",
		"$c0009C'e' del sangue ai tuoi piedi.",
		"$c0009Del sangue scorre in terra.",
		"$c0009C'e' tanto sangue da dare la nausea!",
		"$c0009C'e' sangue ovunque guardi.",
		"$c0009Che carneficina! Il Dio della Morte fara' festa questa notte!",
		"$c0009Non riesci a non sporcarti con il sangue che vedi ovunque!",
		"$c0009Tutto pare rivoltarsi per la morte e distruzione che aleggia qui!",
		"$c0009Gli Dei dovrebbero avere pieta' e ripulire questo orrido posto dal sangue!",
		"$c0009C'e' cosi' tanto sangue qui che rischi di affogarci dentro!",
		"\n"
	};



	if(!ch) {
		mudlog(LOG_SYSERR, "ch==NULL in do_look (act.info.c)");
		return;
	}

	if(!ch->desc) {
		return;
	}

	pRoomWithChar =  real_roomp(ch->in_room);

	if(GET_POS(ch) < POSITION_SLEEPING) {
		send_to_char("Non vedi nient'altro che stelle!\n\r", ch);
	}
	else if(GET_POS(ch) == POSITION_SLEEPING) {
		send_to_char("Non puoi vedere nulla, stai dormendo!\n\r", ch);
	}
	else if(IS_AFFECTED(ch, AFF_BLIND)) {
		send_to_char("Non riesci a vedere un tubo, sei cieco!\n\r", ch);
	}
	else if((IS_DARK_P(pRoomWithChar)) && (!IS_IMMORTAL(ch)) &&
			(!IS_AFFECTED(ch, AFF_TRUE_SIGHT))&&
			GET_RACE(ch)!=RACE_DARK_ELF && GET_RACE(ch)!=RACE_DARK_DWARF && // Gaia 2001
			GET_RACE(ch)!=RACE_DEEP_GNOME) {
		send_to_char("E' molto buio qui...\n\r", ch);
		if(IS_AFFECTED(ch, AFF_INFRAVISION)) {
			list_char_in_room(pRoomWithChar->people, ch);
		}
	}
	else {
		only_argument(arg, arg1);

		if(0==strn_cmp(arg1,"at",2) && isspace(arg1[2])) {
			only_argument(arg+3, arg2);
			keyword_no = 7;
		}
		else if(0==strn_cmp(arg1,"in",2) && isspace(arg1[2])) {
			only_argument(arg+3, arg2);
			keyword_no = 6;
		}
		else {
			keyword_no = search_block(arg1, lookKeywords, FALSE);
		}

		if((keyword_no == -1) && *arg1) {
			keyword_no = 7;
			only_argument(arg, arg2);
		}


		found = FALSE;
		tmp_object = 0;
		tmp_char         = 0;
		tmp_desc         = 0;

		switch(keyword_no) {
		/* look <dir> */
		case 0 :
		case 1 :
		case 2 :
		case 3 :
		case 4 :
		case 5 : {
			struct room_direction_data*        exitp;
			exitp = EXIT(ch, keyword_no);
			if(exitp) {
				if(exitp->general_description && *(exitp->general_description)) {
					char chLast = exitp->general_description
								  [ strlen(exitp->general_description) - 1 ];
					send_to_char(exitp->general_description, ch);

					if(chLast != '\n' && chLast != '\r') {
						send_to_char("\n\r", ch);
					}
				}
				else {
					struct room_data* pRoom = real_roomp(exitp->to_room);

					if(pRoom && pRoom->name && *(pRoom->name) &&
							((IS_SET(exitp->exit_info, EX_ISDOOR) &&
							  !IS_SET(exitp->exit_info, EX_CLOSED)) ||
							 !IS_SET(exitp->exit_info, EX_ISDOOR))) {
						send_to_char(pRoom->name, ch);
						if(pRoom->name[ strlen(pRoom->name) - 1 ] != '.') {
							send_to_char(".", ch);
						}
						send_to_char("\n\r", ch);
					}
					else if(exitp->keyword && *(exitp->keyword) &&
							!IS_SET(exitp->exit_info, EX_SECRET)) {
						snprintf(buffer, MAX_STRING_LENGTH-1, "%s %s.\n\r",
								 IS_SET(exitp->exit_info, EX_MALE) ? "Un" : "Una",
								 fname(exitp->keyword));
						send_to_char(buffer, ch);
					}
					else {
						send_to_char("Non vedi nulla di speciale.\n\r", ch);
					}
				}

				if(IS_SET(exitp->exit_info, EX_ISDOOR)) {
					if(IS_SET(exitp->exit_info, EX_CLOSED)) {
						if(!IS_SET(exitp->exit_info, EX_SECRET)) {
							if(exitp->keyword && *(exitp->keyword)) {
								snprintf(buffer, MAX_STRING_LENGTH-1, "%s %s e' chius%c.\n\r",
										 IS_SET(exitp->exit_info, EX_MALE) ? "Il" : "La",
										 fname(exitp->keyword),
										 IS_SET(exitp->exit_info, EX_MALE) ? 'o' : 'a');
								send_to_char(buffer, ch);
							}
							else {
								snprintf(buffer, MAX_STRING_LENGTH-1,"La porta %s e' chiusa.\n\r",
										 dirsTo[ keyword_no ]);
								send_to_char(buffer, ch);
							}
						}
					}
					else {
						if(!IS_SET(exitp->exit_info, EX_SECRET)) {
							if(exitp->keyword && *(exitp->keyword)) {
								snprintf(buffer, MAX_STRING_LENGTH-1,"%s %s e' apert%c.\n\r",
										 IS_SET(exitp->exit_info, EX_MALE) ? "Il" : "La",
										 fname(exitp->keyword),
										 IS_SET(exitp->exit_info, EX_MALE) ? 'o' : 'a');
								send_to_char(buffer, ch);
							}
							else {
								snprintf(buffer, MAX_STRING_LENGTH-1,"La porta %s e' aperta.\n\r",
										 dirsTo[ keyword_no ]);
								send_to_char(buffer, ch);
							}
						}
						else {
							snprintf(buffer, MAX_STRING_LENGTH-1,"Il passaggio %s e' aperto.\n\r",
									 dirsTo[ keyword_no ]);
							send_to_char(buffer, ch);
						}
					}
				}
			}
			else {
				send_to_char("Non vedi nulla di speciale.\n\r", ch);
			}

			if(exitp && exitp->to_room > 0 &&
					(!IS_SET(exitp->exit_info, EX_ISDOOR) ||
					 !IS_SET(exitp->exit_info, EX_CLOSED))) {

				if((IS_AFFECTED(ch, AFF_SCRYING) &&
						!IS_SET(exitp->exit_info, EX_NOLOOKT)) || IS_IMMORTAL(ch)) {
					struct room_data* rp;
					snprintf(buffer,MAX_STRING_LENGTH-1,
							 "Guardando %s vedi...\n\r", dirsTo[keyword_no]);
					send_to_char(buffer, ch);

					if(!IS_IMMORTAL(ch) || !IS_SET(ch->specials.act, PLR_STEALTH)) {
						snprintf(buffer,MAX_STRING_LENGTH-1,
								 "$n guarda %s.", dirsTo[keyword_no]);
						act(buffer, FALSE, ch, 0, 0, TO_ROOM);
					}

					rp = real_roomp(exitp->to_room);
					if(!rp) {
						send_to_char("Un turbinante chaos.\n\r", ch);
					}
					else if(exitp) {
						bNotShowTitle = TRUE;
						snprintf(buffer, MAX_STRING_LENGTH-1,"%ld look", exitp->to_room);
						do_at(ch, buffer, 0);
						bNotShowTitle = FALSE;
					}
					else { /* non arrivera' mai qui */
						send_to_char("Nulla di speciale.\n\r", ch);
					}
				}
			}
			break;
		}

		/* look 'in'        */
		case 6: {
			if(*arg2) {
				/* Item carried */
				bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
									FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

				if(bits) {
					/* Found something */
					if(GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON) {
						if(tmp_object->obj_flags.value[1] <= 0) {
							act("E' vuoto.", FALSE, ch, 0, 0, TO_CHAR);
						}
						else {
							temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
							snprintf(buffer,MAX_STRING_LENGTH-1,"E' %s di un liquido %s.\n\r",
									 fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
							send_to_char(buffer, ch);
						}
					}
					else if(GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
						if(!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
							send_to_char(fname(tmp_object->name), ch);
							switch(bits) {
							case FIND_OBJ_INV :
								send_to_char(" (trasporta) : \n\r", ch);
								break;
							case FIND_OBJ_ROOM :
								send_to_char(" (qui) : \n\r", ch);
								break;
							case FIND_OBJ_EQUIP :
								send_to_char(" (usa) : \n\r", ch);
								break;
							}
							list_obj_in_heap(tmp_object->contains, ch);
						}
						else {
							send_to_char("E' chiuso.\n\r", ch);
						}
					}
					else {
						send_to_char("Non e' un contenitore.\n\r", ch);
					}
				}
				else {
					/* wrong argument */
					send_to_char("Non lo vedi qui intorno.\n\r", ch);
				}
			}
			else {
				/* no argument */
				send_to_char("Guardare in che cosa?\n\r", ch);
			}
			break;
		}
		/* look 'at'        */
		case 7 : {
			if(*arg2) {
				bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
									FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
				if(tmp_char) {
					show_char_to_char(tmp_char, ch, 1);
					if(ch != tmp_char && !IS_AFFECTED(ch,AFF_SNEAK)) {
						act("$n ti guarda.", TRUE, ch, 0, tmp_char, TO_VICT);
						act("$n guarda a $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
					}
					return;
				}

				/*
				 * Search for Extra Descriptions in room and items
				 */

				/* Extra description in room?? */
				if(!found) {
					tmp_desc = find_ex_description(arg2,
												   pRoomWithChar->ex_description);
					if(tmp_desc) {
						page_string(ch->desc, tmp_desc, 0);
						return;
					}
				}

				/* extra descriptions in items */

				/* Equipment Used */
				if(!found) {
					for(j = 0; j< MAX_WEAR && !found; j++) {
						if(ch->equipment[j]) {
							if(CAN_SEE_OBJ(ch,ch->equipment[j])) {
								tmp_desc = find_ex_description(arg2,
															   ch->equipment[j]->ex_description);
								if(tmp_desc) {
									page_string(ch->desc, tmp_desc, 1);
									found = TRUE;
								}
							}
						}
					}
				}
				/* In inventory */
				if(!found) {
					for(tmp_object = ch->carrying;
							tmp_object && !found;
							tmp_object = tmp_object->next_content) {
						if(CAN_SEE_OBJ(ch, tmp_object)) {
							tmp_desc = find_ex_description(arg2,
														   tmp_object->ex_description);
							if(tmp_desc) {
								page_string(ch->desc, tmp_desc, 1);
								found = TRUE;
							}
						}
					}
				}
				/* Object In room */

				if(!found) {
					for(tmp_object = pRoomWithChar->contents;
							tmp_object && !found;
							tmp_object = tmp_object->next_content) {
						if(CAN_SEE_OBJ(ch, tmp_object)) {
							tmp_desc = find_ex_description(arg2,
														   tmp_object->ex_description);
							if(tmp_desc) {
								page_string(ch->desc, tmp_desc, 1);
								found = TRUE;
							}
						}
					}
				}
				/* wrong argument */
				if(bits) {
					/* If an object was found */
					if(!found) {
						show_obj_to_char(found_object, ch, 5);
					}
					/* Show no-description */
					else {
						show_obj_to_char(found_object, ch, 6);
					}
					/* Find hum, glow etc */
				}
				else if(!found) {
					send_to_char("Non lo vedi qui intorno.\n\r", ch);
				}
			}
			else {
				/* no argument */
				send_to_char("Guardare a cosa ?\n\r", ch);
			}

			break;
		}

		/* look ''                */
		case 8 : {
			if(!bNotShowTitle && cmd == CMD_LOOK) {
				send_to_char(pRoomWithChar->name, ch);
				if(IS_DARK_P(pRoomWithChar) && IS_IMMORTAL(ch)) {
					send_to_char(" (buio)", ch);
				}
				send_to_char("\n\r", ch);
			}

			if(ch->desc &&
					((ch->desc->original &&
					  !IS_SET(ch->desc->original->specials.act, PLR_BRIEF)) ||
					 (!ch->desc->original &&
					  !IS_SET(ch->specials.act, PLR_BRIEF)))) {
				send_to_char(pRoomWithChar->description, ch);
			}

			if(IS_SET(pRoomWithChar->room_flags, BRIGHT)) {
				if(IsDarkOutside(pRoomWithChar) &&
						pRoomWithChar->szWhenBrightAtNight &&
						*pRoomWithChar->szWhenBrightAtNight) {

					send_to_char(pRoomWithChar->szWhenBrightAtNight, ch);
				}
				else if(!IsDarkOutside(pRoomWithChar) &&
						pRoomWithChar->szWhenBrightAtDay &&
						*pRoomWithChar->szWhenBrightAtDay) {

					send_to_char(pRoomWithChar->szWhenBrightAtDay, ch);
				}
			}
			if(RM_BLOOD(ch->in_room) > 0) {
				/* mudlog(LOG_ERROR,"Blood trovato: %d",RM_BLOOD(ch->in_room));
				 * */
				act(blood_messages[static_cast<int8_t>(RM_BLOOD(ch->in_room))], FALSE, ch, 0, 0, TO_CHAR);
			}
			else {
				/*   mudlog(LOG_ERROR,"Blood trovato:
				 *   %d",RM_BLOOD(ch->in_room));*/
			}

			//ACIDUS 2003 skill know structure
			roomp = real_roomp(ch->in_room);
			if (ch->skills) {
				if(
					roomp && (roomp->tele_targ > 0) && (GET_RACE(ch)==RACE_DWARF)
					&& (number(1,100) < ch->skills[SKILL_DETERMINE].learned)
				) {
					act("$c0006Noti qualcosa di instabile nella struttura di questo luogo.", FALSE, ch, 0, 0, TO_CHAR);
				}
			}


			list_exits_in_room(ch);
			list_obj_in_room(pRoomWithChar->contents, ch);
			list_char_in_room(pRoomWithChar->people, ch);

			if(affected_by_spell(ch, SKILL_DANGER_SENSE) || IS_AFFECTED2(ch, AFF2_DANGER_SENSE)) { // Montero 16-Sep-18
				struct room_data* pRoom;
				int i;
				struct room_direction_data* pExit;

				for(i = 0; i < 6; i++) {
					pExit = EXIT(ch, i);
					if(pExit) {
						pRoom = real_roomp(pExit->to_room);
						if(pRoom && IS_SET(pRoom->room_flags, DEATH)) {
							act("$c0009Percepisci un grande pericolo qui intorno.",
								FALSE, ch, NULL, NULL, TO_CHAR);
							break;
						}
					}
				}
			}

			if(IS_PC(ch)) {
				if(IS_SET(ch->specials.act, (IS_POLY(ch)) ? ACT_HUNTING : PLR_HUNTING)) {  // SALVO fix track del poly
					if(ch->specials.hunting) {
						res = track(ch, ch->specials.hunting);
						if(!res) {
							ch->specials.hunting = 0;
							ch->hunt_dist = 0;
							REMOVE_BIT(ch->specials.act, (IS_POLY(ch)) ? ACT_HUNTING : PLR_HUNTING); // SALVO fix track del poly
						}
					}
					else {
						ch->hunt_dist = 0;
						REMOVE_BIT(ch->specials.act, (IS_POLY(ch)) ? ACT_HUNTING : PLR_HUNTING); // SALVO fix track del poly
					}
				}
			}
			else {
				if(IS_SET(ch->specials.act, ACT_HUNTING)) {
					if(ch->specials.hunting) {
						res = track(ch, ch->specials.hunting);
						if(!res) {
							ch->specials.hunting = 0;
							ch->hunt_dist = 0;
							REMOVE_BIT(ch->specials.act, ACT_HUNTING);
						}
					}
					else {
						ch->hunt_dist = 0;
						REMOVE_BIT(ch->specials.act, ACT_HUNTING);
					}
				}
			}

			break;
		}

		/* wrong arg        */
		case -1 :
			send_to_char("Mi spiace, ma non capisco!\n\r", ch);
			break;

		/* look 'room' */
		case 9 : {

			send_to_char(pRoomWithChar->name, ch);
			send_to_char("\n\r", ch);
			send_to_char(pRoomWithChar->description, ch);


			if(!IS_NPC(ch)) {
				if(IS_SET(ch->specials.act, PLR_HUNTING)) {
					if(ch->specials.hunting) {
						res = track(ch, ch->specials.hunting);
						if(!res) {
							ch->specials.hunting = 0;
							ch->hunt_dist = 0;
							REMOVE_BIT(ch->specials.act, PLR_HUNTING);
						}
					}
					else {
						ch->hunt_dist = 0;
						REMOVE_BIT(ch->specials.act, PLR_HUNTING);
					}
				}
			}
			else {
				if(IS_SET(ch->specials.act, ACT_HUNTING)) {
					if(ch->specials.hunting) {
						res = track(ch, ch->specials.hunting);
						if(!res) {
							ch->specials.hunting = 0;
							ch->hunt_dist = 0;
							REMOVE_BIT(ch->specials.act, ACT_HUNTING);
						}
					}
					else {
						ch->hunt_dist = 0;
						REMOVE_BIT(ch->specials.act, ACT_HUNTING);
					}
				}
			}
			if(RM_BLOOD(ch->in_room) > 0) {
				mudlog(LOG_ERROR,"Blood trovato: %d",RM_BLOOD(ch->in_room));
				act(blood_messages[static_cast<int8_t>(RM_BLOOD(ch->in_room))], FALSE, ch, 0, 0, TO_CHAR);
			}
			else {
				mudlog(LOG_ERROR,"Blood trovato: %d",RM_BLOOD(ch->in_room));
			}
			list_exits_in_room(ch);
			list_obj_in_room(pRoomWithChar->contents, ch);
			list_char_in_room(pRoomWithChar->people, ch);

			break;
		}
		}
	}
}

/* end of look */




ACTION_FUNC(do_read) {
	char buf[100];

	/* This is just for now - To be changed later.! */
	snprintf(buf,99,"at %s",arg);
	do_look(ch,buf,15);
}



ACTION_FUNC(do_examine) {
	char name[1000], buf[1000];
	struct char_data* tmp_char;
	struct obj_data* tmp_object;

	snprintf(buf,999,"at %s",arg);
	do_look(ch, buf, 15);

	one_argument(arg, name);

	if(!*name) {
		send_to_char("Esaminare che cosa ?\n\r", ch);
		return;
	}

	generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if(tmp_object) {
		if((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
				(GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
			send_to_char("Quando ci guardi dentro, vedi:\n\r", ch);
			snprintf(buf,999,"in %s",arg);
			do_look(ch,buf,15);
		}
	}
}

/**************************************************************************
 * do_exits visualizza le uscite della locazione.
 **************************************************************************/
ACTION_FUNC(do_exits) {
	/* NOTE: Input var 'cmd' is not used. */
	int door;
	char buf[1000];
	struct room_direction_data* exitdata;


	*buf = '\0';

	for(door = 0; door <= 5; door++) {
		exitdata = EXIT(ch,door);
		if(exitdata && exitdata->to_room > 0) {
			if(!real_roomp(exitdata->to_room)) {
				/* don't print unless immortal */
				if(IS_IMMORTAL(ch)) {
					sprintf(buf + strlen(buf), "%s - turbinante chaos di #%ld\n\r",
							exits[door], exitdata->to_room);
				}
			}
			else if(exitdata->to_room != NOWHERE) {
				if(IS_DIO(ch)) {
					sprintf(buf + strlen(buf), "%s - %s", exits[door],
							real_roomp(exitdata->to_room)->name);
					if(IS_SET(exitdata->exit_info, EX_SECRET)) {
						strcat(buf," (segreta)");
					}
					if(IS_SET(exitdata->exit_info, EX_CLOSED)) {
						strcat(buf, " (chiusa)");
					}
					if(IS_DARK(exitdata->to_room)) {
						strcat(buf, " (buia)");
					}
					sprintf(buf + strlen(buf), " #%ld\n\r", exitdata->to_room);
				}
				else if(!IS_SET(exitdata->exit_info, EX_CLOSED) ||
						!IS_SET(exitdata->exit_info, EX_SECRET)) {
					if(IS_DARK(exitdata->to_room))
						sprintf(buf + strlen(buf), "%s - Troppo buio per dirlo\n\r",
								exits[ door ]);
					else
						sprintf(buf + strlen(buf), "%s - %s\n\r", exits[ door ],
								real_roomp(exitdata->to_room)->name);
				}
			}
		}
	}

	send_to_char("Uscite visibili:\n\r", ch);

	if(*buf) {
		send_to_char(buf, ch);
	}
	else {
		send_to_char("Nessuna !\n\r", ch);
	}
}

ACTION_FUNC(do_status) {
	static char buf[1000];
	snprintf(buf, 999,
			 "$c0005Tu hai $c0015%d$c0005($c0011%d$c0005) hit, "
			 "$c0015%d$c0005($c0011%d$c0005) mana e "
			 "$c0015%d$c0005($c0011%d$c0005) punti di movimento.",
			 GET_HIT(ch),GET_MAX_HIT(ch),
			 GET_MANA(ch),GET_MAX_MANA(ch),
			 GET_MOVE(ch),GET_MAX_MOVE(ch));
	act(buf, FALSE, ch, 0, 0, TO_CHAR);
	snprintf(buf, 999,
			 "$c0005Hai effettuato $c0015%d$c0005 exp, ed "
			 "hai $c0015%d$c0005 monete d'oro.",
			 GET_EXP(ch), GET_GOLD(ch));
	act(buf, FALSE, ch, 0, 0, TO_CHAR);
}

ACTION_FUNC(do_score) {
	struct time_info_data playing_time;
	char buf[1000], buf2[1000];
	char datanasc[200];
	struct time_info_data my_age;
	struct time_info_data my_birth;

	//struct time_info_data mud_time_passed(time_t t2, time_t t1);
	int weekday, day;
	my_birth=mud_time_passed(beginning_of_time,ch->player.time.birth);
	age3(ch, &my_age);
	weekday = ((35*my_birth.month)+my_birth.day+1) % 7;/* 35 days in a month */
	day = my_birth.day + 1;   /* day in [1..35] */
	if(IS_IMMENSO(ch)) {
		my_birth.year=-1;
		my_age.year=time_info.year+1;
		my_age.ayear=my_age.year;
	}
	else {
		my_birth.year=(time_info.year-my_age.ayear);
	}
	snprintf(datanasc,sizeof datanasc,"$c0005Sei nat$b nel %s, %d^ del %s, nell'anno %d %s.",
			 weekdays[weekday]+3,
			 day,
			 month_name[(int)my_birth.month ],
			 my_birth.year,
			 (my_birth.year>0?" dopo Nebbie":" avanti Nebbie"));

	snprintf(buf,999,"%s\n\rHai $c0015%d$c0005 anni.", datanasc,my_age.ayear);
	//FIXME: check for buffer overrun
	if(my_age.year != my_age.ayear) {
		char app[50];
		snprintf(app,sizeof app,"\nMa ne dimostri $c0015%d$c0005.",my_age.year);
		strcat(buf,app);
	}

	if((my_age.month == my_birth.month) && (my_age.day == my_birth.month)) {
		strcat(buf,"\n$c0015 Oggi e' il tuo compleanno!!");
	}

	act(buf,FALSE, ch,0,0,TO_CHAR);

	snprintf(buf, 999,"$c0005Appartieni alla razza $c0015%s$c0005",
			 RaceName[GET_RACE(ch)]);
	act(buf,FALSE, ch,0,0,TO_CHAR);
	if(HAS_PRINCE(ch)) {
		snprintf(buf, 999,"$c0005Appartieni al clan di $c0015%s$c0005",
				 GET_PRINCE(ch));
		act(buf,FALSE, ch,0,0,TO_CHAR);
	}

	if(!IS_IMMORTAL(ch) && (!IS_NPC(ch))) {
		if(GET_COND(ch,DRUNK)>10) {
			act("$c0011Sei sbronz$b.",FALSE, ch,0,0,TO_CHAR);
		}
		if(GET_COND(ch,FULL)<2 && GET_COND(ch,FULL) != -1) {
			act("$c0005Hai $c0015fame$c0005...",FALSE, ch,0,0,TO_CHAR);
		}
		if(GET_COND(ch,THIRST)<2  && GET_COND(ch,THIRST) != -1) {
			act("$c0005Hai $c0015sete$c0005...",FALSE, ch,0,0,TO_CHAR);
		}
	}

	snprintf(buf, 999,
			 "$c0005Hai $c0015%d$c0005($c0011%d$c0005) hit, "
			 "$c0015%d$c0005($c0011%d$c0005) mana e "
			 "$c0015%d$c0005($c0011%d$c0005) movimento.",
			 GET_HIT(ch),GET_MAX_HIT(ch),
			 GET_MANA(ch),GET_MAX_MANA(ch),
			 GET_MOVE(ch),GET_MAX_MOVE(ch));
	act(buf,FALSE,ch,0,0,TO_CHAR);

	snprintf(buf, 999, "$c0005Il tuo allineamento e': $c0015%s",
			 AlignDesc(GET_ALIGNMENT(ch)));
	act(buf,FALSE,ch,0,0,TO_CHAR);

	snprintf(buf,999, "$c0005Hai effettuato $c0015%d$c0005 exp, ed "
			 "hai $c0015%d$c0005 monete d'oro.",
			 GET_EXP(ch),GET_GOLD(ch));
	act(buf,FALSE,ch,0,0,TO_CHAR);
	snprintf(buf,999, "$c0005Non puoi donare xp se hai meno di: $c0015%d$c0005 exp",
			 IS_PRINCE(ch)?PRINCEEXP:MIN_EXP(ch));
	act(buf,FALSE,ch,0,0,TO_CHAR);

	/* the mud will crash without this check! */
	if(GetMaxLevel(ch)>MAX_MORT ||
			(IS_NPC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF))) {
		/* do nothing! */
	}
	else {
		buf[0] = '\0';
		snprintf(buf,999,"$c0005Exp al prossimo livello: ");
		if(HasClass(ch, CLASS_MAGIC_USER)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"M:$c0015%d$c0005 ",
						 (titles[MAGE_LEVEL_IND]
						  [GET_LEVEL(ch, MAGE_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"M:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_CLERIC)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"C:$c0015%d$c0005 ",
						 (titles[CLERIC_LEVEL_IND]
						  [GET_LEVEL(ch, CLERIC_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"C:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_THIEF)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"T:$c0015%d$c0005 ",
						 (titles[THIEF_LEVEL_IND]
						  [GET_LEVEL(ch, THIEF_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"T:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_WARRIOR)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"W:$c0015%d$c0005 ",
						 (titles[WARRIOR_LEVEL_IND]
						  [GET_LEVEL(ch, WARRIOR_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"W:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_DRUID)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"D:$c0015%d$c0005 ",
						 (titles[DRUID_LEVEL_IND]
						  [GET_LEVEL(ch, DRUID_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"D:0 ");
			}
			strcat(buf,buf2);
		}
		if(HasClass(ch, CLASS_MONK)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"K:$c0015%d$c0005 ",
						 (titles[MONK_LEVEL_IND]
						  [GET_LEVEL(ch, MONK_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"K:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_BARBARIAN)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"B:$c0015%d$c0005 ",
						 (titles[BARBARIAN_LEVEL_IND]
						  [GET_LEVEL(ch, BARBARIAN_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"B:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_SORCERER)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"S:$c0015%d$c0005 ",
						 (titles[SORCERER_LEVEL_IND]
						  [GET_LEVEL(ch, SORCERER_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"S:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_PALADIN)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"P:$c0015%d$c0005 ",
						 (titles[PALADIN_LEVEL_IND]
						  [GET_LEVEL(ch, PALADIN_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"P:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_RANGER)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"R:$c0015%d$c0005 ",
						 (titles[RANGER_LEVEL_IND]
						  [GET_LEVEL(ch, RANGER_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"R:0 ");
			}
			strcat(buf,buf2);
		}

		if(HasClass(ch, CLASS_PSI)) {
			if(GetMaxLevel(ch)<MAX_IMMORT)
				snprintf(buf2,999,"I:$c0015%d$c0005 ",
						 (titles[PSI_LEVEL_IND]
						  [GET_LEVEL(ch, PSI_LEVEL_IND)+1].exp)-GET_EXP(ch));
			else {
				snprintf(buf2,999,"I:0 ");
			}
			strcat(buf,buf2);
		}

		act(buf,FALSE,ch,0,0,TO_CHAR);
	}

	buf[0] = '\0';
	snprintf(buf, 999,"$c0005I tuoi livelli:");
	if(HasClass(ch, CLASS_MAGIC_USER)) {
		snprintf(buf2, 999," M:$c0015%d$c0005", GET_LEVEL(ch, MAGE_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_CLERIC)) {
		snprintf(buf2, 999," C:$c0015%d$c0005", GET_LEVEL(ch, CLERIC_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_WARRIOR)) {
		snprintf(buf2, 999," W:$c0015%d$c0005", GET_LEVEL(ch, WARRIOR_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_THIEF)) {
		snprintf(buf2, 999," T:$c0015%d$c0005", GET_LEVEL(ch, THIEF_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_DRUID)) {
		snprintf(buf2, 999," D:$c0015%d$c0005", GET_LEVEL(ch, DRUID_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_MONK)) {
		snprintf(buf2, 999," K:$c0015%d$c0005", GET_LEVEL(ch, MONK_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_BARBARIAN)) {
		snprintf(buf2, 999," B:$c0015%d$c0005", GET_LEVEL(ch, BARBARIAN_LEVEL_IND));
		strcat(buf, buf2);
	}

	if(HasClass(ch, CLASS_SORCERER)) {
		snprintf(buf2, 999," S:$c0015%d$c0005", GET_LEVEL(ch, SORCERER_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_PALADIN)) {
		snprintf(buf2, 999," P:$c0015%d$c0005", GET_LEVEL(ch, PALADIN_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_RANGER)) {
		snprintf(buf2, 999," R:$c0015%d$c0005", GET_LEVEL(ch, RANGER_LEVEL_IND));
		strcat(buf, buf2);
	}
	if(HasClass(ch, CLASS_PSI)) {
		snprintf(buf2, 999," I:$c0015%d$c0005", GET_LEVEL(ch, PSI_LEVEL_IND));
		strcat(buf, buf2);
	}

	act(buf,FALSE,ch,0,0,TO_CHAR);

	if(GET_TITLE(ch)) {
		snprintf(buf,999,"$c0005Questo ti qualifica come $c0015%s $c0011%s",
				 GET_NAME(ch), GET_TITLE(ch));
		act(buf,FALSE,ch,0,0,TO_CHAR);
	}

	playing_time = real_time_passed((time(0)-ch->player.time.logon) + ch->player.time.played, 0);
	snprintf(buf,999,"$c0005Hai giocato per $c0015%d$c0005 giorni e $c0015%d$c0005 ore.",
			 playing_time.day,
			 playing_time.hours);
	act(buf,FALSE,ch,0,0,TO_CHAR);

	/* Drow fight -4 in lighted rooms! */
	if(!IS_DARK(ch->in_room) && GET_RACE(ch) == RACE_DARK_ELF &&
			!affected_by_spell(ch,SPELL_GLOBE_DARKNESS) && !IS_UNDERGROUND(ch)) {
		snprintf(buf,999,"$c0011La luce nell'area ti da molto dolore$c0009!");
		act(buf,FALSE,ch,0,0,TO_CHAR);
	}

	snprintf(buf,999,"$c0005Le rune degli dei tatuate sul tuo corpo sono: $c0015%d ",
			 GET_RUNEDEI(ch));
	act(buf,FALSE,ch,0,0,TO_CHAR);

	switch(GET_POS(ch)) {
	case POSITION_DEAD :
		act("$c0009Sei mort$b!",FALSE, ch,0,0,TO_CHAR);
		break;
	case POSITION_MORTALLYW :
		act("$c0009Sei ferit$b a morte e dovresti cercare aiuto !", FALSE, ch,
			0,0,TO_CHAR);
		break;
	case POSITION_INCAP :
		act("$c0009Sei incapacitat$b, e stai morendo lentamente",FALSE, ch,0,0,
			TO_CHAR);
		break;
	case POSITION_STUNNED :
		act("$c0011Sei svenut$b! Non ti puoi muovere.",FALSE, ch,0,0,TO_CHAR);
		break;
	case POSITION_SLEEPING :
		act("$c0010Stai dormendo.",FALSE,ch,0,0,TO_CHAR);
		break;
	case POSITION_RESTING  :
		act("$c0012Stai riposando.",FALSE,ch,0,0,TO_CHAR);
		break;
	case POSITION_SITTING  :
		act("$c0013Sei sedut$b.",FALSE,ch,0,0,TO_CHAR);
		break;
	case POSITION_FIGHTING :
		if(ch->specials.fighting)
			act("$c1009Stai combattendo contro $N.", FALSE, ch, 0,
				ch->specials.fighting, TO_CHAR);
		else {
			act("$c1009Stai combattendo contro l'aria.",FALSE, ch,0,0,TO_CHAR);
		}
		break;
	case POSITION_STANDING :
		act("$c0005Sei in piedi.",FALSE,ch,0,0,TO_CHAR);
		break;
	case POSITION_MOUNTED:
		if(MOUNTED(ch)) {
			snprintf(buf,999,"$c0005Stai montando su $c0015%s",
					 MOUNTED(ch)->player.short_descr);
			act(buf,FALSE,ch,0,0,TO_CHAR);
		}
		else {
			act("$c0005Sei in piedi.",FALSE,ch,0,0,TO_CHAR);
		}
		break;
	default :
		act("$c0005Sta fluttuando.",FALSE,ch,0,0,TO_CHAR);
		break;
	}
}


ACTION_FUNC(do_time) {
	char buf[100];
	int weekday, day;

	snprintf(buf, 99,"Sono le %d del%s, ",
			 ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
			 ((time_info.hours >= 12) ? " pomeriggio" : "la mattina"));

	weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */

	strcat(buf,weekdays[weekday]);
	strcat(buf,"\n\r");
	send_to_char(buf,ch);

	day = time_info.day + 1;   /* day in [1..35] */

	snprintf(buf, 99,"Il %d^ giorno del %s, nell'anno %d.\n\r",
			 day,
			 month_name[(int)time_info.month ],
			 time_info.year);

	send_to_char(buf,ch);
}


ACTION_FUNC(do_weather) {
	char buf[ 256 ];
	const char* sky_look[] = {
		"sereno",
		"nuvoloso",
		"piovoso",
		"illuminato dai lampi"
	};

	if(OUTSIDE(ch)) {
		snprintf(buf, 255,
				 "Il cielo e' %s e%s.\n\r",
				 sky_look[weather_info.sky],
				 (weather_info.change >=0 ? " senti un vento caldo da sud" :
				  "d i tuoi reumatismi ti dicono che il tempo cambiera'"));
		send_to_char(buf, ch);
	}
	else {
		send_to_char("Non riesci a vedere il tempo che fa fuori.\n\r", ch);
	}
}


ACTION_FUNC(do_help) {

	int chk, bot, top, mid, minlen;
	char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];


	if(!ch->desc) {
		return;
	}

	for(; isspace(*arg); arg++) ;



	if(*arg) {
		if(!help_index) {
			send_to_char("L'help non e' disponibile.\n\r", ch);
			return;
		}
		bot = 0;
		top = top_of_helpt;

		for(;;) {
			mid = (bot + top) / 2;
			minlen = strlen(arg);

			if(!(chk = strn_cmp(arg, help_index[mid].keyword, minlen))) {
				fseek(help_fl, help_index[mid].pos, 0);
				*buffer = '\0';
				for(;;) {
					fgets(buf, 80, help_fl);
					if(*buf == '#') {
						break;
					}
					if(strlen(buf)+strlen(buffer) > MAX_STRING_LENGTH-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
				page_string(ch->desc, buffer, 1);
				return;
			}
			else if(bot >= top) {
				send_to_char("Non c'e' aiuto per quella parola.\n\r", ch);
				return;
			}
			else if(chk > 0) {
				bot = ++mid;
			}
			else {
				top = --mid;
			}
		}
		return;
	}

	page_string(ch->desc, help, 1);
	/*send_to_char(help, ch);*/

}


ACTION_FUNC(do_wizhelp) {
	char buf[1000];
	int i, j = 1;
	NODE* n;

	if(IS_NPC(ch)) {
		return;
	}

	one_argument(arg,buf);                /* new msw */
	if(*arg) {
		/* asking for help on keyword, try looking in file */
		do_actual_wiz_help(ch,arg,cmd);
		return;
	}

	snprintf(buf, 999,"Wizhelp <keyword>\n\r"
			 "Wizard Commands disponibili per te:\n\r\n\r");

	for(i = 0; i < 27; i++) {
		n = radix_head[i].next;
		while(n) {
			if(n->min_level <= GetMaxLevel(ch) && n->min_level >= IMMORTALE) {
				sprintf((buf + strlen(buf)), "%-10s", n->name);
				if(!(j % 7)) {
					sprintf((buf + strlen(buf)), "\n\r");
				}
				j++;
			}
			n = n->next;
		}
	}

	strcat(buf, "\n\r");

	page_string(ch->desc, buf, 1);
}

ACTION_FUNC(do_actual_wiz_help) {

	int chk, bot, top, mid, minlen;
	char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];


	if(!ch->desc) {
		return;
	}

	for(; isspace(*arg); arg++)  ;


	if(*arg) {
		if(!wizhelp_index) {
			send_to_char("Il wizhelp non e' disponibile.\n\r", ch);
			return;
		}
		bot = 0;
		top = top_of_wizhelpt;

		for(;;) {
			mid = (bot + top) / 2;
			minlen = strlen(arg);

			if(!(chk = strn_cmp(arg, wizhelp_index[mid].keyword, minlen))) {
				fseek(wizhelp_fl, wizhelp_index[mid].pos, 0);
				*buffer = '\0';
				for(;;) {
					fgets(buf, 80, wizhelp_fl);
					if(*buf == '#') {
						break;
					}
					if(strlen(buf)+strlen(buffer) > MAX_STRING_LENGTH-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
				page_string(ch->desc, buffer, 1);
				return;
			}
			else if(bot >= top) {
				send_to_char("Non esiste aiuto per questa parola.\n\r", ch);
				return;
			}
			else if(chk > 0) {
				bot = ++mid;
			}
			else {
				top = --mid;
			}
		}
		return;
	}
	/* send a generic wizhelp menu like help I guess send_to_char(help, ch); */
}


ACTION_FUNC(do_command_list) {
	char buf[MAX_STRING_LENGTH];
	int i, j = 1;
	NODE* n;

	if(IS_NPC(ch)) {
		return;
	}

	snprintf(buf, MAX_STRING_LENGTH-1,"Comandi disponibili per te:\n\r\n\r");

	for(i = 0; i < 27; i++) {
		n = radix_head[i].next;
		while(n) {
			if(n->min_level <= GetMaxLevel(ch)) {
				if(strlen(buf)+strlen(n->name) <= MAX_STRING_LENGTH) {
					sprintf((buf + strlen(buf)), "%-10s", n->name);
				}
				if(!(j % 7))
					if(strlen(buf)+4 <= MAX_STRING_LENGTH) {
						sprintf((buf + strlen(buf)), "\n\r");
					}
				j++;
			}
			n = n->next;
		}
	}

	strcat(buf, "\n\r");

	page_string(ch->desc, buf, 1);
}

#define OK_NAME(name,mask)  (mask[0]=='\0' || \
							 strncmp(strcpy(tmpname1,lower(GET_NAME(name))),\
									 strcpy(tmpname2,lower(mask)),\
									 strlen(mask))==0)
/*
int OK_NAME(struct char_data *name,const char*mask)
{
  char n1[80],n2[80];
  if(!*mask) return 1;
  strcpy(n1,lower(GET_NAME(name)));
  strcpy(n2,lower(mask));
  return(strncmp(n1,n2,strlen(mask))==0);
}
*/


ACTION_FUNC(do_who) {
	struct char_data* person;
	char buffer[MAX_STRING_LENGTH*2]="",tbuf[512];
	int count;
	char flags[20]="";
	char name_mask[40]="";
	char tmpname1[80],tmpname2[80];
	int listed = 0;
	int lcount =0;
	int  skip = FALSE;
	long l;
	char ttbuf[256];

	/*  check for an arg */
	arg = one_argument(arg,tbuf);
	if(tbuf[0]=='-' && tbuf[1]!='\0') {
		strcpy(flags,tbuf+1);
	}
	else {
		strcpy(name_mask,tbuf);
	}
	if(*arg) {
		arg = one_argument(arg,tbuf);
		if(tbuf[0]=='-' && tbuf[1]!='\0') {
			strcpy(flags,tbuf+1);
		}
		else {
			strcpy(name_mask,tbuf);
		}
	}

	if((IS_DIO_MINORE(ch) && flags[0] == '\0') || !IS_DIO_MINORE(ch) ||
			cmd == CMD_WHOZONE) {
		if(IS_DIO_MINORE(ch))
			snprintf(buffer,(MAX_STRING_LENGTH*2)-1,
					 "$c0005                         Giocatori [God Version -? for Help]\n\r"
					 "                       -------------------------------------\n\r");
		else if(cmd == CMD_WHOZONE)
			snprintf(buffer,(MAX_STRING_LENGTH*2)-1,"$c0005Giocatori\n\r"
					 "---------\n\r");
		else
			snprintf(buffer,(MAX_STRING_LENGTH*2)-1,
					 "$c0005                         Nelle nebbie........\n\r"
					 "                       ---------------------------\n\r");
		count=0;
		for(person = character_list; person; person = person->next) {
			if(!IS_LINKDEAD(person) and IS_PC(person) and
					OK_NAME(person, name_mask) and
					person->invis_level <= GetMaxLevel(ch) and
					(cmd != CMD_WHOZONE or
					 (real_roomp(person->in_room) and
					  real_roomp(person->in_room)->zone ==
					  real_roomp(ch->in_room)->zone))and
					(!index(flags,'g') or IS_IMMORTAL(person))) {
				if(OK_NAME(person,name_mask)) {
					count++;

					if(cmd == CMD_WHOZONE) {
						/* it's a whozone command */
						if((!IS_AFFECTED(person, AFF_HIDE)) || (IS_IMMORTAL(ch))) {
							snprintf(tbuf,511,
									 "$c0012%-25s - %s", GET_NAME_DESC(person),
									 IS_AFFECTED(person,AFF_SNEAK)?
									 "sneaking around":
									 real_roomp(person->in_room)->name);
							if(GetMaxLevel(ch) >= IMMORTALE) {
								sprintf(tbuf+strlen(tbuf)," [%d]", person->in_room);
							}
						}
					}
					else {
						char levels[40]="", classes[20]="";
						const char* classname[]= { "Mu","Cl","Wa","Th","Dr","Mo","Ba","So","Pa",
												   "Ra","Ps"
												 };
						int i,total,classn;
						long bit;
						if(GetMaxLevel(person) < IMMORTALE) {
							for(bit=1,i=total=classn=0; i<PSI_LEVEL_IND+1; i++, bit<<=1) {
								if(HasClass(person,bit)) {
									classn++;
									total+=person->player.level[i];
									if(strlen(classes)!=0) {
										strcat(classes,"/");
									}
									if(IS_SET(person->player.user_flags,SHOW_CLASSES)) {
										sprintf(classes+strlen(classes),"%s",classname[i]);
									}
								}
							}
							/*		     total/=classn; */
							/* Modifica Urhar dello who in base alla classe piu' alta e non alla media */
							total= GetMaxLevel(person);
							/* fine modifica */
							if(IS_PRINCE(person) || total == PRINCIPE) {  // SALVO per riconoscere anche il poly
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0008Principe":
									   "$c0008Principessa");
							}
							else if(total<ALLIEVO)  /* 1-10 */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0002Novizio":
									   "$c0002Novizia");
							else if(total<APPRENDISTA) {   /* 11-20 */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0004Allievo":"$c0004Allieva");
							}
							else if(total<INIZIATO)       /* 21-30 */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0006Apprendista":
									   "$c0006Apprendista");
							else if(total<ESPERTO)      /* 31-40 */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0014Iniziato":
									   "$c0014Iniziata");
							else if(total<MAESTRO)      /* 41-45 */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0009Esperto":
									   "$c0009Esperta");
							else if(total<BARONE)       /* 46-49 */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0015Maestro":
									   "$c0015Maestra");
							else if(GET_EXP(person)<CONTE) /* 50 */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0015Barone":
									   "$c0015Baronessa");
							else if(GET_EXP(person)<MARCHESE)   /* fino ai 200M */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0015Conte":
									   "$c0015Contessa");
							else if(GET_EXP(person)<DUCA)       /* 250M */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0015Marchese":
									   "$c0015Marchesa");
							else if(GET_EXP(person)<GRANDUCA)   /* 300M */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0015Duca":
									   "$c0015Duchessa");
							else if(GET_EXP(person)<PRINCEEXP)   /* 350M */
								strcpy(levels,GET_SEX(person)==SEX_MALE?"$c0001Granduca":
									   "$c0001Granduchessa");
							snprintf(tbuf, 511,"%s $c0012%s",levels, classes);
							sprintf(levels,"%34s","");
							strcpy(levels+17-((strlen(tbuf))/2),tbuf);
							snprintf(tbuf, 511,
									 "%-34s $c0005: $c0007%s %s $c0007%s%s",
									 levels,
									 GET_NAME_DESC(person) ?
									 GET_NAME_DESC(person): "nemo",
									 person->player.title ?
									 person->player.title : "",
									 HAS_PRINCE(person)?"del clan di ":"",
									 HAS_PRINCE(person)?GET_PRINCE(person):"");

						}
						else {
							if(GET_SEX(person) == SEX_FEMALE) {
								switch(GetMaxLevel(person)) {
								case 52:
									sprintf(levels,"Immortale");
									break;
								case 53:
									sprintf(levels,"Dea Minore");
									break;
								case 54:
									sprintf(levels,"Dea");
									break;
								case 55:
									sprintf(levels,"Maestra degli Dei");
									break;
								case 56:
									sprintf(levels,"Creatrice");
									break;
								case 57:
									sprintf(levels,"Maestra del Fato");
									break;
								case 58:
									sprintf(levels,"Maestra del Creato");
									break;
								case 59:
									sprintf(levels,"Maestra dei Creatori");
									break;
								case 60:
									sprintf(levels,"Immensa");
									break;
								}
							}
							else {
								switch(GetMaxLevel(person)) {
								case 52:
									sprintf(levels,"Immortale");
									break;
								case 53:
									sprintf(levels,"Dio Minore");
									break;
								case 54:
									sprintf(levels,"Dio");
									break;
								case 55:
									sprintf(levels,"Maestro degli Dei");
									break;
								case 56:
									sprintf(levels,"Creatore");
									break;
								case 57:
									sprintf(levels,"Maestro del Fato");
									break;
								case 58:
									sprintf(levels,"Maestro del Creato");
									break;
								case 59:
									sprintf(levels,"Maestro dei Creatori");
									break;
								case 60:
									sprintf(levels,"Immenso");
									break;
								};
							}
							snprintf(tbuf,511, "$c0008%s $c0005",levels);
							sprintf(levels,"%34s","");
							strcpy(levels+17-(strlen(tbuf)/2),tbuf);
							snprintf(tbuf, 511,"%-34s $c0005: $c0011%-10s %s$c0007",levels,
									 GET_NAME_DESC(person),
									 person->player.title ? person->player.title : "");
						}
					}
					if(IS_AFFECTED2(person,AFF2_AFK)) {
						strcat(tbuf, "$c0008 AF");
					}
					if(IS_SET(person->player.user_flags,RACE_WAR)) {
						strcat(tbuf, "$c0001 PK");
					}
					strcat(tbuf, " $c0007\n\r");
					if(strlen(buffer)+strlen(tbuf) < (MAX_STRING_LENGTH*2)-512) {
						strcat(buffer,tbuf);
					}
				}
			}
		}
		if(index(flags,'g')) {
			snprintf(tbuf, 511,"\n\r$c0005Totale Divinita' visibili: $c0015%d\n\r", count);
		}
		else {
			snprintf(tbuf, 511,"\n\r$c0005Totale giocatori visibili: $c0015%d\n\r", count);
		}
		if(strlen(buffer)+strlen(tbuf) < MAX_STRING_LENGTH*2-512) {
			strcat(buffer,tbuf);
		}
		snprintf(tbuf, 511,"\n\r$c0005Max giocatori connessi dall'ultimo reboot: $c0015%d\r\n", update_max_usage());
		if(strlen(buffer)+strlen(tbuf) < MAX_STRING_LENGTH*2-512) {
			strcat(buffer,tbuf);
		}
	}
	else {
		/* GOD WHO */


		snprintf(buffer,(MAX_STRING_LENGTH*2)-1,
				 "$c0005Giocatori [God Version -? for Help]\n\r"
				 "---------\n\r");

		count=0;
		lcount=0;
		if(index(flags,'?')) {
			send_to_char(buffer,ch);
			send_to_char("$c0007[-]i=idle l=levels t=title h=hit/mana/move "
						 "s=stats r=race \n\r",ch);
			send_to_char("[-]d=linkdead e=eq g=God o=Mort a=align "
						 "[1]Mage[2]Cleric[3]War[4]Thief[5]Druid\n\r", ch);
			send_to_char("[-][6]Monk[7]Barb[8]Sorc[9]Paladin[!]Ranger[@]Psi\n\r",
						 ch);
			return;
		}

		for(person = character_list; person; person = person->next) {
			if(!IS_NPC(person) and CAN_SEE(ch, person) and OK_NAME(person,name_mask)) {
				count++;
				if(person->desc == NULL) {
					lcount ++;
				}
				skip = FALSE;
				if(index(flags,'g') != NULL)
					if(!IS_IMMORTAL(person)) {
						skip = TRUE;
					}
				if(index(flags,'o') != NULL)
					if(IS_IMMORTAL(person)) {
						skip = TRUE;
					}
				if(index(flags,'1') != NULL)
					if(!HasClass(person,CLASS_MAGIC_USER)) {
						skip = TRUE;
					}
				if(index(flags,'2') != NULL)
					if(!HasClass(person,CLASS_CLERIC)) {
						skip = TRUE;
					}
				if(index(flags,'3') != NULL)
					if(!HasClass(person,CLASS_WARRIOR)) {
						skip = TRUE;
					}
				if(index(flags,'4') != NULL)
					if(!HasClass(person,CLASS_THIEF)) {
						skip = TRUE;
					}
				if(index(flags,'5') != NULL)
					if(!HasClass(person,CLASS_DRUID)) {
						skip = TRUE;
					}
				if(index(flags,'6') != NULL)
					if(!HasClass(person,CLASS_MONK)) {
						skip = TRUE;
					}
				if(index(flags,'7') != NULL)
					if(!HasClass(person,CLASS_BARBARIAN)) {
						skip = TRUE;
					}
				if(index(flags,'8') != NULL)
					if(!HasClass(person,CLASS_SORCERER)) {
						skip = TRUE;
					}
				if(index(flags,'9') != NULL)
					if(!HasClass(person,CLASS_PALADIN)) {
						skip = TRUE;
					}
				if(index(flags,'!') != NULL)
					if(!HasClass(person,CLASS_RANGER)) {
						skip = TRUE;
					}
				if(index(flags,'@') != NULL)
					if(!HasClass(person,CLASS_PSI)) {
						skip = TRUE;
					}

				if(!skip) {
					if(person->desc == NULL) {
						if(index(flags,'d') != NULL) {
							snprintf(tbuf, 511, "$c0003[%-12s] ", GET_NAME(person));
							listed++;
						}
					}
					else {
						if(IS_NPC(person) && IS_SET(person->specials.act, ACT_POLYSELF)) {
							snprintf(tbuf, 511, "(%-12s) ", GET_NAME(person));
							listed++;
						}
						else {
							snprintf(tbuf, 511, "$c0012%-14s ", GET_NAME(person));
							listed++;
						}
					}
					if((person->desc != NULL) || (index(flags,'d') != NULL)) {
						for(l = 0; l < (int)strlen(flags) ; l++) {
							switch(flags[l]) {
							case 'r': {
								/* show race */
								char bbuf[256];
								sprinttype((person->race),RaceName,ttbuf);
								snprintf(bbuf,255," [%s] ",ttbuf);
								strcat(tbuf,bbuf);
								break;
							}
							case 'e':
								snprintf(ttbuf,255,"Eq:[%f] ",GetCharBonusIndex(person));
								strcat(tbuf,ttbuf);
								break;
							case 'a':
								snprintf(ttbuf,255,"Al:[%d] ",GET_ALIGNMENT(person));
								strcat(tbuf,ttbuf);
								break;
							case 'i':
								snprintf(ttbuf,255,"Idle:[%-3d] ",person->specials.timer);
								strcat(tbuf,ttbuf);
								break;
							case 'l':
								snprintf(ttbuf, 255,"Level:[%-2d/%-2d/%-2d/%-2d/%-2d/"
										 "%-2d/%-2d/%-2d/%-2d/%-2d/%-2d] ",
										 person->player.level[0],person->player.level[1],
										 person->player.level[2],person->player.level[3],
										 person->player.level[4],person->player.level[5],
										 person->player.level[6],person->player.level[7],
										 person->player.level[8],person->player.level[9],
										 person->player.level[10]);
								strcat(tbuf,ttbuf);
								break;
							case 'h':
								snprintf(ttbuf, 255,"Hit:[%-3d] Mana:[%-3d] Move:[%-3d] ",
										 GET_HIT(person), GET_MANA(person), GET_MOVE(person));
								strcat(tbuf,ttbuf);
								break;
							case 'c':
								if(HAS_PRINCE(person)) {
									snprintf(ttbuf,255, "del clan di %s",
											 GET_PRINCE(person));
									strcat(tbuf,ttbuf);
								}
								break;
							case 's':
								if(GET_STR(person) != 18)
									snprintf(ttbuf, 255,"[S:%-2d I:%-2d W:%-2d "
											 "C:%-2d D:%-2d CH:%-2d] ",
											 GET_STR(person),GET_INT(person),GET_WIS(person),
											 GET_CON(person),GET_DEX(person),GET_CHR(person));
								else
									snprintf(ttbuf, 255,"[S:%-2d(%1d) I:%-2d W:%-2d "
											 "C:%-2d D:%-2d CH:%-2d] ",
											 GET_STR(person),GET_ADD(person),GET_INT(person),
											 GET_WIS(person),GET_CON(person),GET_DEX(person),
											 GET_CHR(person));
								strcat(tbuf,ttbuf);
								break;
							case 't':
								snprintf(ttbuf, 255," %-16s ", (person->player.title ?
																person->player.title :
																"(null)"));
								strcat(tbuf,ttbuf);
								break;
							default:
								break;
							}
						}
					}
					if((person->desc != NULL) || (index(flags,'d') != NULL)) {
						if(OK_NAME(person,name_mask)) {
							if(strlen(buffer)+strlen(tbuf) < (MAX_STRING_LENGTH*2)-512) {
								strcat(buffer,tbuf);
								strcat(buffer,"\n\r");
							}
						}
					}
				}
			}
		}

		if(listed<=0)
			snprintf(tbuf, 255,"\n\r$c0005Nessun personaggio con le caratteristiche "
					 "richieste.\n\r");
		else
			snprintf(tbuf, 255,"\n\r$c0005Totale giocatori / Link dead [%d/%d] "
					 "(%2.0f%%)\n\r",
					 count, lcount, ((float)lcount / (int)count) * 100);
		if(strlen(buffer)+strlen(tbuf) < (MAX_STRING_LENGTH*2)-512) {
			strcat(buffer,tbuf);
		}
		snprintf(tbuf, 255,"\n\r$c0005Max giocatori connessi dall'ultimo reboot: $c0015%d\r\n", update_max_usage());
		if(strlen(buffer)+strlen(tbuf) < (MAX_STRING_LENGTH*2)-512) {
			strcat(buffer,tbuf);
		}
	}
	page_string(ch->desc, buffer, TRUE);
}

ACTION_FUNC(do_users) {
	char buf[MAX_STRING_LENGTH], line[200], buf2[255];

	struct descriptor_data* d;

	strcpy(buf, "Connessioni:\n\r"
		   "------------\n\r");

	for(d = descriptor_list; d; d = d->next) {
		if(CAN_SEE(ch, d->character) || GetMaxLevel(ch)==IMMENSO) {
			if(d->character && d->character->player.name) {
				if(d->original) {
					snprintf(line, 199,"%-16s: ", d->original->player.name);
				}
				else {
					snprintf(line, 199,"%-16s: ", d->character->player.name);
				}
			}
			else {
				snprintf(line,199, "NON DEFINITO  : ");
			}

			if(d->host && *d->host) {
				snprintf(buf2, 254,"%-22s [%s] %5d\n\r",
						 connected_types[d->connected],d->host,d->wait);
			}
			else {
				snprintf(buf2, 254,"%-22s [%s] %5d\n\r",
						 connected_types[d->connected],"????",d->wait);
			}
			strcat(line, buf2);
			strcat(buf, line);
		} /* could not see the person */
	} /* end for */

	/*  send_to_char(buf, ch); */
	page_string(ch->desc,buf,0);
}



ACTION_FUNC(do_inventory) {

	send_to_char("Stai trasportando:\n\r", ch);
	list_obj_in_heap(ch->carrying, ch);
}


ACTION_FUNC(do_equipment) {
	int j,Worn_Index;
	bool found;
	char String[256];

	send_to_char("Stai usando:\n\r", ch);
	found = FALSE;
	for(Worn_Index = j=0; j< MAX_WEAR; j++) {
		if(ch->equipment[j]) {
			Worn_Index++;
			snprintf(String,255,"[%2d] %s",Worn_Index,eqWhere[j]);
			send_to_char(String,ch);
			if(CAN_SEE_OBJ(ch,ch->equipment[j])) {
				show_obj_to_char(ch->equipment[j],ch,1);
				found = TRUE;
			}
			else {
				send_to_char("Qualcosa.\n\r",ch);
				found = TRUE;
			}
		}
	}
	if(!found) {
		send_to_char(" Nulla.\n\r", ch);
	}
}


ACTION_FUNC(do_credits) {
	SET_BIT(ch->player.user_flags,USE_PAGING);
	page_string(ch->desc, credits, 0);
}
ACTION_FUNC(do_news) {
	SET_BIT(ch->player.user_flags,USE_PAGING);
	page_string(ch->desc, news, 0);
}
ACTION_FUNC(do_wiznews) {
	SET_BIT(ch->player.user_flags,USE_PAGING);
	page_string(ch->desc, wiznews, 0);
}
ACTION_FUNC(do_info) {
	SET_BIT(ch->player.user_flags,USE_PAGING);
	page_string(ch->desc, info, 0);
}
ACTION_FUNC(do_wizlist) {
	SET_BIT(ch->player.user_flags,USE_PAGING);
	page_string(ch->desc, wizlist, 0);
}
ACTION_FUNC(do_prince) {
	SET_BIT(ch->player.user_flags,USE_PAGING);
	page_string(ch->desc, princelist, 0);
}
ACTION_FUNC(do_immortal) {
	SET_BIT(ch->player.user_flags,USE_PAGING);
	page_string(ch->desc, immlist, 0);
}

int which_number_object(struct obj_data* obj) {
	struct obj_data* i;
	const char* name;
	int number;

	name = fname(obj->name);
	for(i = object_list, number = 0; i; i = i->next) {
		if(isname(name, i->name)) {
			number++;
			if(i == obj) {
				return number;
			}
		}
	}
	return 0;
}

char* numbered_object(struct char_data* ch, struct obj_data* obj) {
	static char buf[MAX_STRING_LENGTH];
	if(IS_IMMORTAL(ch)) {
		snprintf(buf,MAX_STRING_LENGTH-1,"%d.%s", which_number_object(obj), fname(obj->name));
	}
	else {
		strcpy(buf, obj->short_description);
	}
	return buf;
}

int which_number_mobile(struct char_data* ch, struct char_data* mob) {
	struct char_data*        i;
	const char*        name;
	int        number;

	name = fname(mob->player.name);
	for(i=character_list, number=0; i; i=i->next) {
		if(isname(name, i->player.name) && i->in_room != NOWHERE) {
			number++;
			if(i==mob) {
				return number;
			}
		}
	}
	return 0;
}

char* numbered_person(struct char_data* ch, struct char_data* person) {
	static char buf[MAX_STRING_LENGTH];
	if(IS_NPC(person) && IS_IMMORTAL(ch)) {
		snprintf(buf,MAX_STRING_LENGTH-1,"%d.%s", which_number_mobile(ch, person),
				 fname(person->player.name));
	}
	else {
		strcpy(buf, PERS(person, ch));
	}
	return buf;
}

void do_where_person(struct char_data* ch, struct char_data* person,
					 struct string_block* sb) {
	char buf[MAX_STRING_LENGTH];

	if(!CAN_SEE(ch, person)) {
		return;
	}

	snprintf(buf, MAX_STRING_LENGTH-1,"%-40s- %s ", PERS(person, ch),
			 (person->in_room > -1 ? real_roomp(person->in_room)->name :
			  "Nowhere"));

	if(GetMaxLevel(ch) >= DIO) {
		sprintf(buf+strlen(buf),"[%d]", person->in_room);
	}

	strcpy(buf+strlen(buf), "\n\r");

	append_to_string_block(sb, buf);
}

void do_where_object(struct char_data* ch, struct obj_data* obj,
					 int recurse, struct string_block* sb) {
	char buf[MAX_STRING_LENGTH];
	if(obj->in_room != NOWHERE) {
		/* object in a room */
		snprintf(buf, MAX_STRING_LENGTH-1,
				 "%-40s- %s [%d]\n\r",
				 obj->short_description,
				 real_roomp(obj->in_room)->name,
				 obj->in_room);
	}
	else if(obj->carried_by != NULL) {
		/* object carried by monster */
		snprintf(buf, MAX_STRING_LENGTH-1,
				 "%-40s- trasportato da %s\n\r",
				 obj->short_description,
				 numbered_person(ch, obj->carried_by));
	}
	else if(obj->equipped_by != NULL) {
		/* object equipped by monster */
		snprintf(buf, MAX_STRING_LENGTH-1,
				 "%-40s- usato da %s\n\r",
				 obj->short_description,
				 numbered_person(ch, obj->equipped_by));
	}
	else if(obj->in_obj) {
		/* object in object */
		snprintf(buf, MAX_STRING_LENGTH-1,"%-40s- in %s\n\r",
				 obj->short_description,
				 numbered_object(ch, obj->in_obj));
	}
	else {
		snprintf(buf, MAX_STRING_LENGTH-1,"%-40s- Nemmeno Dio sa dove...\n\r",
				 obj->short_description);
	}
	if(*buf) {
		append_to_string_block(sb, buf);
	}

	if(recurse) {
		if(obj->in_room != NOWHERE) {
			return;
		}
		else if(obj->carried_by != NULL) {
			do_where_person(ch, obj->carried_by, sb);
		}
		else if(obj->equipped_by != NULL) {
			do_where_person(ch, obj->equipped_by, sb);
		}
		else if(obj->in_obj != NULL) {
			do_where_object(ch, obj->in_obj, TRUE, sb);
		}
	}
}

ACTION_FUNC(do_where) {
	char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	char*        nameonly;
	register struct char_data* i;
	register struct obj_data* k;
	struct descriptor_data* d;
	int        number, count;
	struct string_block        sb;

	only_argument(arg, name);

	if(!*name) {
		if(GetMaxLevel(ch) < DIO) {
			send_to_char("Cosa stai cercando ?\n\r", ch);
		}
		else {
			init_string_block(&sb);
			append_to_string_block(&sb, "Giocatori:\n\r"
								   "----------\n\r");

			for(d = descriptor_list; d; d = d->next) {
				if(d->character && (d->connected == CON_PLYNG) &&
						(d->character->in_room != NOWHERE) &&
						CAN_SEE(ch, d->character)) {
					if(d->original)    /* If switched */
						snprintf(buf, MAX_STRING_LENGTH-1,
								 "%-20s - %s [%d] Nel corpo di %s\n\r",
								 d->original->player.name,
								 real_roomp(d->character->in_room)->name,
								 d->character->in_room,
								 fname(d->character->player.name));
					else
						snprintf(buf, MAX_STRING_LENGTH-1,
								 "%-20s - %s [%d]\n\r",
								 d->character->player.name,
								 real_roomp(d->character->in_room)->name,
								 d->character->in_room);

					append_to_string_block(&sb, buf);
				}
			}
			page_string_block(&sb,ch);
			destroy_string_block(&sb);
		}
		return;
	}

	if(isdigit(*name)) {
		nameonly = name;
		count = number = get_number(&nameonly);
	}
	else {
		count = number = 0;
	}

	*buf = '\0';

	init_string_block(&sb);

	for(i = character_list; i; i = i->next) {
		if(isname(name, i->player.name) && CAN_SEE(ch, i)) {
            
            if(!IS_PC(i) && affected_by_spell(i,STATUS_QUEST) && GetMaxLevel(ch) < IMMORTALE) {
                act("Non si bara! ;)\n\r", FALSE, ch, 0, ch, TO_CHAR);
                break;
            }
            
			if((i->in_room != NOWHERE) &&
					((GetMaxLevel(ch)>=IMMORTALE) || (real_roomp(i->in_room)->zone ==
							real_roomp(ch->in_room)->zone))) {
				if(number==0 || (--count) == 0) {
					if(number==0) {
						snprintf(buf, MAX_STRING_LENGTH-1,
								 "[%2d] ", ++count); /* I love short circuiting :) */
						append_to_string_block(&sb, buf);
					}
					do_where_person(ch, i, &sb);
					*buf = 1;
					if(number!=0) {
						break;
					}
				}
				if(GetMaxLevel(ch) < IMMORTALE) {
					break;
				}
			}
		}
	}
	/*  count = number;*/

	if(GetMaxLevel(ch) >= DIO) {
		for(k = object_list; k; k = k->next) {
			if(isname(name, k->name) && CAN_SEE_OBJ(ch, k)) {
				if(number==0 || (--count)==0) {
					if(number==0) {
						snprintf(buf, MAX_STRING_LENGTH-1,"[%2d] ", ++count);
						append_to_string_block(&sb, buf);
					}
					do_where_object(ch, k, number!=0, &sb);
					*buf = 1;
					if(number!=0) {
						break;
					}
				}
			}
		}
	}

	if(!*sb.data) {
		send_to_char("Non trovo niente del genere.\n\r", ch);
	}
	else {
		page_string_block(&sb, ch);
	}
	destroy_string_block(&sb);
}




ACTION_FUNC(do_levels) {
	int i, RaceMax, iClass;
	char buf[MAX_STRING_LENGTH*2],buf2[MAX_STRING_LENGTH];


	if(IS_NPC(ch)) {
		send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
		return;
	}

	*buf = '\0';
	/*
	 *  get the class
	 */

	for(; isspace(*arg); arg++);

	if(!*arg) {
		send_to_char("Devi fornire una classe !\n\r", ch);
		return;
	}

	switch(*arg) {
	case 'C':
	case 'c':
		iClass = CLERIC_LEVEL_IND;
		break;
	case 'F':
	case 'f':
	case 'W':
	case 'w':
		iClass = WARRIOR_LEVEL_IND;
		break;
	case 'M':
	case 'm':
		iClass = MAGE_LEVEL_IND;
		break;
	case 'T':
	case 't':
		iClass = THIEF_LEVEL_IND;
		break;
	case 'D':
	case 'd':
		iClass = DRUID_LEVEL_IND;
		break;
	case 'K':
	case 'k':
		iClass = MONK_LEVEL_IND;
		break;
	case 'B':
	case 'b':
		iClass = BARBARIAN_LEVEL_IND;
		break;
	case 'S':
	case 's':
		iClass = SORCERER_LEVEL_IND;
		break;

	case 'P':
	case 'p':
		iClass = PALADIN_LEVEL_IND;
		break;

	case 'R':
	case 'r':
		iClass = RANGER_LEVEL_IND;
		break;

	case 'I':
	case 'i':
		iClass = PSI_LEVEL_IND;
		break;

	default:
		snprintf(buf, (MAX_STRING_LENGTH*2)-1,"Non riconosco %s\n\r", arg);
		send_to_char(buf,ch);
		return;
		break;
	}

	RaceMax = RacialMax[GET_RACE(ch)][iClass];

	buf[ 0 ] = 0;

	for(i = 1; i <= RaceMax; i++) {
		/* crashed in sprintf here, see if you can figure out why. msw, 8/24/94 */
		snprintf(buf2, MAX_STRING_LENGTH-1,
				 "[%2d] %9d-%-9d : %s\n\r", i,
				 titles[iClass][i].exp,
				 titles[iClass][i + 1].exp,
				 (GET_SEX(ch)==SEX_FEMALE ?
				  titles[iClass][i].title_f :
				  titles[iClass][i].title_m));
		/* send_to_char(buf, ch); */
		strcat(buf,buf2);
	}
	strcat(buf,"\n\r");
	page_string(ch->desc,buf,1);

}



ACTION_FUNC(do_consider) {
	struct char_data* victim;
	char name[256], buf[256];
	int diff;

	only_argument(arg, name);

	if(!(victim = get_char_room_vis(ch, name))) {
		send_to_char("Chi stai considerando di uccidere ?\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Non dovrebbe essere difficile.\n\r", ch);
		return;
	}

	if(!IS_NPC(victim)) {
		send_to_char("Non ci pensare nemmeno !!\n\r", ch);
		act("$n sta considerando di ucciderti", FALSE, ch, 0, victim, TO_VICT);
		return;
	}

	act("$n guarda a $N", FALSE, ch, 0, victim, TO_NOTVICT);
	act("$n ti guarda", FALSE, ch, 0, victim, TO_VICT);


	if(GetMaxLevel(ch)>=IMMORTALE && GetMaxLevel(ch)<IMMENSO) {
		act("Che bisogno hai di considerare $N ?", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	diff =  GetMaxLevel(victim) + HowManyClasses(victim) -
			GetMaxLevel(ch) - HowManyClasses(ch);

	diff += MobLevBonus(victim);

	if(diff <= -10) {
		send_to_char("Troppo facile per crederci.\n\r", ch);
	}
	else if(diff <= -5) {
		send_to_char("Non sara' un problema.\n\r", ch);
	}
	else if(diff <= -3) {
		send_to_char("Piu' che facile.\n\r",ch);
	}
	else if(diff <= -2) {
		send_to_char("Facile.\n\r", ch);
	}
	else if(diff <= -1) {
		send_to_char("Piuttosto facile.\n\r", ch);
	}
	else if(diff == 0) {
		send_to_char("L'incontro perfetto!\n\r", ch);
	}
	else if(diff <= 1) {
		send_to_char("Avrai bisogno di un po' di fortuna!\n\r", ch);
	}
	else if(diff <= 2) {
		send_to_char("Avrai bisogno di molta fortuna!\n\r", ch);
	}
	else if(diff <= 3)
		send_to_char("Avrai bisogno di molta fortuna ed un buon "
					 "equipaggiamento!\n\r", ch);
	else if(diff <= 5) {
		send_to_char("Non ti sarai un po' montato la testa?\n\r", ch);
	}
	else if(diff <= 10) {
		send_to_char("Sei scemo o cosa?\n\r", ch);
	}
	else if(diff <= 30) {
		send_to_char("Tu SEI matto!\n\r", ch);
	}
	else
		send_to_char("Perche' non ti butti semplicemente a mare e ci risparmi la "
					 "fatica?\n\r", ch);

	if(ch->skills) {
		int skill=0;
		int learn=0;
		int num, num2;
		float fnum;

		if(IsAnimal(victim) && ch->skills[SKILL_CONS_ANIMAL].learned) {
			skill = SKILL_CONS_ANIMAL;
			learn = ch->skills[skill].learned;
			act("$N sembra un animale.", FALSE, ch, 0, victim, TO_CHAR);
		}
		if(IsVeggie(victim) && ch->skills[SKILL_CONS_VEGGIE].learned) {
			if(!skill) {
				skill = SKILL_CONS_VEGGIE;
			}
			learn = MAX(learn, ch->skills[SKILL_CONS_VEGGIE].learned);
			act("$N sembra un vegetale ambulante.", FALSE, ch, 0, victim, TO_CHAR);
		}
		if(IsDiabolic(victim) && ch->skills[SKILL_CONS_DEMON].learned) {
			if(!skill) {
				skill = SKILL_CONS_DEMON;
			}
			learn = MAX(learn, ch->skills[SKILL_CONS_DEMON].learned);
			act("$N sembra un demone!", FALSE, ch, 0, victim, TO_CHAR);
		}
		if(IsReptile(victim) && ch->skills[SKILL_CONS_REPTILE].learned) {
			if(!skill) {
				skill = SKILL_CONS_REPTILE;
			}
			learn = MAX(learn, ch->skills[SKILL_CONS_REPTILE].learned);
			act("$N sembra un rettile.", FALSE, ch, 0, victim, TO_CHAR);
		}
		if(IsUndead(victim) && ch->skills[SKILL_CONS_UNDEAD].learned) {
			if(!skill) {
				skill = SKILL_CONS_UNDEAD;
			}
			learn = MAX(learn, ch->skills[SKILL_CONS_UNDEAD].learned);
			act("$N sembra un non morto.", FALSE, ch, 0, victim, TO_CHAR);
		}

		if(IsGiantish(victim)&& ch->skills[SKILL_CONS_GIANT].learned) {
			if(!skill) {
				skill = SKILL_CONS_GIANT;
			}
			learn = MAX(learn, ch->skills[SKILL_CONS_GIANT].learned);
			act("$N sembra una creatura gigantesca.", FALSE, ch, 0, victim, TO_CHAR);
		}
		if(IsPerson(victim) && ch->skills[SKILL_CONS_PEOPLE].learned) {
			if(!skill) {
				skill = SKILL_CONS_PEOPLE;
			}
			learn = MAX(learn, ch->skills[SKILL_CONS_PEOPLE].learned);
			act("$N sembra umano o mezzo umano.", FALSE, ch, 0, victim, TO_CHAR);
		}
		if(IsOther(victim)&& ch->skills[SKILL_CONS_OTHER].learned) {
			if(!skill) {
				skill = SKILL_CONS_OTHER;
			}
			learn = MAX(learn, ch->skills[SKILL_CONS_OTHER].learned/2);
			act("$N sembra essere un mostro che conosci.", FALSE, ch, 0, victim,
				TO_CHAR);
		}

		if(learn > 95) {
			learn = 95;
		}

		if(learn == 0) {
			return;
		}

		WAIT_STATE(ch, PULSE_VIOLENCE*2);

#if 1

		num = (int)GetApprox(GET_MAX_HIT(victim), learn);
		num2 = (int)GET_MAX_HIT(ch);
		if(!num2) {
			num2=1;
		}
		fnum = ((int)num/(int)num2);
		snprintf(buf, 255,"Il numero massimo di punti ferita e' %s.\n\r", DescRatio(fnum));
		send_to_char(buf, ch);

		num = 105+(int)GetApprox(GET_AC(victim), learn);
		num2 = 105+(int)GET_AC(ch);
//    if (!num2) num2=1;
		fnum = ((int)num-(int)num2);

		snprintf(buf, 255,"La classe di armatura e' %s.\n\r", DescArmorf(fnum));
		send_to_char(buf, ch);



		if(learn > 60) {
			snprintf(buf, 255,"Ha %s attacchi per turno.\n\r",
					 DescAttacks((int)GetApprox((int)victim->mult_att, learn)));
			send_to_char(buf, ch);
		}

		if(learn > 70) {

			num = (int)GetApprox((int)victim->specials.damnodice, learn);
			num2 = (int)GetApprox((int)victim->specials.damsizedice, learn);
			if(!num2) {
				num2=1;
			}
			fnum = (int)num*(num2/2.0);
			snprintf(buf, 255,"Il danno per attacco e' %s.\n\r", DescDamage(fnum));

			send_to_char(buf, ch);
		}

		if(learn > 80) {

			num = (int)GetApprox(GET_HITROLL(victim), learn);
			num2 = ((int)21 - CalcThaco(ch, NULL));
			if(!num2) {
				num2=1;
			}
			if(num2 > 0) {
				fnum = ((int)num/(int)num2);
			}
			else {
				fnum = 2.0;
			}

			snprintf(buf, 255,"La sua probabilita' di colpirti e' %s.\n\r", DescRatioF(fnum));

			send_to_char(buf, ch);


			num =   GetApprox(GET_DAMROLL(victim), learn);
			num2 =  GET_DAMROLL(ch);
			if(!num2) {
				num2=1;
			}
			fnum = ((int)num/(int)num2);

			snprintf(buf, 255,"Il bonus al danno e' %s.\n\r", DescRatio(fnum));

			send_to_char(buf, ch);

		}
#endif


	}

}

ACTION_FUNC(do_spells) {
	int spl, i;        /* 16384 */
	char buf[ 256 ];
	struct string_block sb;

	if(IS_NPC(ch)) {
		send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
		return;
	}

	init_string_block(&sb);
	*buf=0;

	sprintf(buf + strlen(buf),
			"[#  ] %-30s  MANA, Cl, Mu, Dr, Sc, Pa, Ra, Ps\n\r",
			"SPELL/SKILL\0");

	append_to_string_block(&sb, buf);

	for(i = 1, spl = 0; i <= MAX_EXIST_SPELL; i++, spl++) {
		if(GetMaxLevel(ch) > IMMORTALE ||
				(spell_info[i].min_level_cleric < ABS_MAX_LVL
				 && spell_info[i].min_level_cleric > 0)) {  // SALVO controllo spell_info
			if(!spells[spl]) {
				mudlog(LOG_ERROR, "!spells[spl] on %d, do_spells in act.info.c", i);
			}
			else {
				snprintf(buf, 255, "[%3d] %-30s  <%3d> %2d %3d %3d %3d %3d %3d %3d\n\r",
						 i, spells[spl],
						 spell_info[i].min_usesmana,
						 spell_info[i].min_level_cleric,
						 spell_info[i].min_level_magic,
						 spell_info[i].min_level_druid,
						 spell_info[i].min_level_sorcerer,
						 spell_info[i].min_level_paladin,
						 spell_info[i].min_level_ranger,
						 spell_info[i].min_level_psi
						);
				append_to_string_block(&sb, buf);
			}
		}
	}
	append_to_string_block(&sb, "\n\r");
	page_string_block(&sb, ch);
	destroy_string_block(&sb);
}

double GetLagIndex() {
	uint64_t lag=0.0;
	for(uint i=0; i< sizeof(aTimeCheck); ++i) {
		lag+=aTimeCheck[0];
	}
	return static_cast<double>(lag)/sizeof(aTimeCheck)/1000000.0;
}

ACTION_FUNC(do_world) {
	char buf[1000];

	long ct, ot;
	char* tmstr, *otmstr;

	snprintf(buf, 999,"$c0005Base Source: $c0014AlarMUD\n$c0005"
			 "Versione $c0015%s\n$c0005Commit: $c0015%s$c0005.",
			 version(), release());
	act(buf,FALSE, ch,0,0,TO_CHAR);
	snprintf(buf, 999,"$c0005Compilazione del : $c0014%s.", compilazione());
	act(buf,FALSE, ch,0,0,TO_CHAR);
	ot = Uptime;
	otmstr = asctime(localtime(&ot));
	*(otmstr + strlen(otmstr) - 1) = '\0';
	snprintf(buf, 999,"$c0005Orario di partenza: $c0015%s $c0005", otmstr);
	act(buf,FALSE, ch,0,0,TO_CHAR);

	ct = time(0);
	tmstr = asctime(localtime(&ct));
	*(tmstr + strlen(tmstr) - 1) = '\0';
	snprintf(buf, 999, "$c0005Orario attuale    : $c0015%s $c0005", tmstr);
	act(buf,FALSE, ch,0,0,TO_CHAR);

	snprintf(buf,999, "$c0005Indice di attesa desiderato: $c0015%.6f $c0005secs", static_cast<double>(OPT_USEC)/1000000.0);
	act(buf, FALSE, ch, 0, 0, TO_CHAR);
	snprintf(buf, 999,"$c0005Indice di attesa attuale   : $c0015%.6f $c0005sec", GetLagIndex());
	act(buf, FALSE, ch, 0, 0, TO_CHAR);

	if(GetMaxLevel(ch) >=IMMORTALE) {
		char tbuf[256];
		sprintbit((unsigned long)SystemFlags,system_flag_types,tbuf);
		snprintf(buf,255,"$c0005Flags di sistema:[$c0015%s$c0005]\n\r",tbuf);
		act(buf,FALSE,ch,0,0,TO_CHAR);
		snprintf(buf,255,"$c0005Connessioni dalla partenza:$c0015%d\n\r",
				 HowManyConnection(0));
		act(buf,FALSE,ch,0,0,TO_CHAR);

	}

#if HASH
	snprintf(buf,255,"$c0005Numero di stanze nel mondo          : $c0015%ld",
			 room_db.klistlen);
#else
	snprintf(buf,255,"$c0005Numero di stanze nel mondo          : $c0015%ld",
			 room_count);
#endif
	act(buf,FALSE, ch,0,0,TO_CHAR);
	snprintf(buf,999,"$c0005Numero di zone nel mondo            : $c0015%d",
			 top_of_zone_table + 1);
	act(buf,FALSE, ch,0,0,TO_CHAR);

	snprintf(buf,999,"$c0005Numero di personaggi attivi         : $c0015%d",
			 top_of_p_table);
	act(buf,FALSE, ch,0,0,TO_CHAR);

	// Result of an aggregate query contains only one element so let's
	// use the query_value() shortcut.
	//
	auto c =Sql::getOne<userCount>();
	snprintf(buf,999,"$c0005Numero di giocatori registrati      : $c0015%ld",
			 c->count);
	act(buf,FALSE, ch,0,0,TO_CHAR);

	snprintf(buf,999,"$c0005Numero di tipi di creature nel mondo: $c0015%d",
			 top_of_mobt);
	act(buf,FALSE, ch,0,0,TO_CHAR);
	snprintf(buf,999,"$c0005Numero di tipi di oggetti nel mondo  : $c0015%d",
			 top_of_objt);
	act(buf,FALSE, ch,0,0,TO_CHAR);

	snprintf(buf,999,"$c0005Numero di creature nel gioco         : $c0015%ld",
			 mob_count);
	act(buf,FALSE, ch,0,0,TO_CHAR);
	snprintf(buf,999,"$c0005Numero di oggetti nel gioco          : $c0015%ld",
			 obj_count);
	act(buf, FALSE,ch,0,0,TO_CHAR);
	/**** SALVO controllo lag refresh zone init */
	if(IS_IMMORTAL(ch)) {
		int i,c;
		for(i = c = 0; i <= top_of_zone_table; i++)
			if(zone_table[i].start != 0) {
				c++;
			}
		snprintf(buf,999,"$c0005Totale zone init nel mondo           : $c0015%d su $c0015%d",
				 c, top_of_zone_table+1);
		act(buf, FALSE,ch,0,0,TO_CHAR);
		snprintf(buf,999,"$c0005Indice attesa refresh zone init      : $c0015%d $c0005sec",
				 attrefzone);
		act(buf, FALSE,ch,0,0,TO_CHAR);
	}
	/****/
	snprintf(buf,999,"$c0005Valore medio dell'eq in gioco        : $c0015%f",
			 AverageEqIndex(-1));
	act(buf, FALSE,ch,0,0,TO_CHAR);

}

ACTION_FUNC(do_attribute) {
	char buf[MAX_STRING_LENGTH];
	struct affected_type* aff;

	struct time_info_data my_age;


	age3(ch, &my_age);

	snprintf(buf,MAX_STRING_LENGTH-1,
			 "$c0005Tu hai $c0014%d (ma ne dimostri %d) $c0005 anni e $c0014%d$c0005 mesi, "
			 "$c0014%d$c0005 cm, e pesi $c0014%d$c0005 chili.",
			 my_age.ayear, my_age.year,my_age.month,
			 ch->player.height,
			 (ch->player.weight * 4536) / 10000);

	act(buf,FALSE, ch,0,0,TO_CHAR);

	snprintf(buf, MAX_STRING_LENGTH-1,
			 "$c0005Stai trasportando $c0014%d$c0005 etti di "
			 "equipaggiamento su $c0014%d.",
			 (IS_CARRYING_W(ch) * 4536) / 1000,
			 (CAN_CARRY_W(ch) *4536) /1000);
	act(buf,FALSE, ch,0,0,TO_CHAR);
	snprintf(buf, MAX_STRING_LENGTH-1,
			 "$c0005Stai trasportando $c0014%d$c0005 oggetti "
			 "su $c0014%d.",
			 IS_CARRYING_N(ch),
			 CAN_CARRY_N(ch));
	act(buf,FALSE, ch,0,0,TO_CHAR);

	snprintf(buf,MAX_STRING_LENGTH-1,
			 "$c0005Tu sei$c0014 %s",ArmorDesc(ch->points.armor));
	act(buf,FALSE,ch,0,0,TO_CHAR);
	if(GetMaxLevel(ch)>=MAESTRO_DEL_CREATO) {
		snprintf(buf,MAX_STRING_LENGTH-1,
				 "$c0005Armor class: $c0014%d",ch->points.armor);
		act(buf,FALSE,ch,0,0,TO_CHAR);
	}
	//GGPATCH, inserita valutazione spellfail
	int iSpellfail = 0;
	if(IS_CASTER(ch)) {
		iSpellfail=ch->specials.spellfail;
		if(GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) {
			snprintf(buf,MAX_STRING_LENGTH-1,
					 "$c0005Spellfail : $c0014 %d",iSpellfail);
			act(buf,FALSE,ch,0,0,TO_CHAR);
		}
		iSpellfail = ch->specials.spellfail-
					 GET_LEVEL(ch, BestMagicClass(ch))+
					 25+
					 (HowManyClasses(ch)-1)*5;
		iSpellfail+=((
						 (EqWBits(ch,ITEM_ANTI_MAGE) && HasClass(ch,CLASS_MAGIC_USER)) ||
						 (EqWBits(ch,ITEM_ANTI_CLERIC) && HasClass(ch,CLASS_CLERIC)) ||
						 (EqWBits(ch,ITEM_ANTI_DRUID) && HasClass(ch,CLASS_DRUID)) ||
						 (EqWBits(ch,ITEM_ANTI_PALADIN) && HasClass(ch,CLASS_PALADIN)) ||
						 (EqWBits(ch,ITEM_ANTI_RANGER) && HasClass(ch,CLASS_RANGER)) ||
						 (EqWBits(ch,ITEM_ANTI_PSI) && HasClass(ch,CLASS_PSI)))?20:0);
		snprintf(buf,MAX_STRING_LENGTH-1,
				 "$c0005La tua capacita' di lanciare incantesimi e'$c0014 %s",
				 SpellfailDesc(IS_IMMORTAL(ch)?0:iSpellfail));
		act(buf,FALSE,ch,0,0,TO_CHAR);
	}
	if(GetMaxLevel(ch) >= CHUMP) {
		snprintf(buf,MAX_STRING_LENGTH-1,
				 "$c0005Tu hai $c0014%d$c0005/$c0015%d $c0005STR, "
				 "$c0014%d $c0005INT, $c0014%d $c0005WIS, $c0014%d "
				 "$c0005DEX, $c0014%d $c0005CON, $c0014%d $c0005CHR",
				 GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch),
				 GET_CON(ch), GET_CHR(ch));
		act(buf,FALSE,ch,0,0,TO_CHAR);
	}

	snprintf(buf, MAX_STRING_LENGTH-1,
			 "$c0005I tuoi hit e damage bonus sono $c0014%s$c0005 e "
			 "$c0014%s$c0005 rispettivamente.",
			 HitRollDesc(GET_HITROLL(ch)), DamRollDesc(GET_DAMROLL(ch)));
	act(buf,FALSE, ch,0,0,TO_CHAR);
	snprintf(buf, MAX_STRING_LENGTH-1,
			 "$c0005Il tuo equipaggiamento e' $c0014%s$c0005",
			 EqDesc(GetCharBonusIndex(ch)));
	act(buf,FALSE, ch,0,0,TO_CHAR);
	if(GetMaxLevel(ch)>=MAESTRO_DEL_CREATO) {
		snprintf(buf,MAX_STRING_LENGTH-1,
				 "$c0005Hit:$c0014+%d$c0005 Dam:$c0014+%d",
				 GET_HITROLL(ch),GET_DAMROLL(ch));
		act(buf,FALSE,ch,0,0,TO_CHAR);
	}

	/*
	 **   by popular demand -- affected stuff
	 */
	if(ch->affected) {
		int bFirstTime = TRUE;
		int otype = -1;
		for(aff = ch->affected; aff; aff = aff->next) {
			if(aff->type <= MAX_EXIST_SPELL && aff->type != otype) {
				otype = aff->type;

				switch(aff->type) {
				case SKILL_SNEAK:
				case SPELL_POISON:
				case SPELL_CURSE:
				case SPELL_PRAYER:
				case SPELL_FEEBLEMIND:
				case SKILL_SWIM:
				case SKILL_SPY:
				case SKILL_FIRST_AID:
				case SKILL_LAY_ON_HANDS:
				case SKILL_MEDITATE:
					break;

				default:
					if(bFirstTime) {
						act("\n\r$c0005Spells attivi:\n\r"
							"--------------",FALSE, ch, 0, 0, TO_CHAR);
						bFirstTime = FALSE;
					}
					if(*spells[aff->type-1] || *spells[aff->type-1]=='!') {

                        snprintf(buf, MAX_STRING_LENGTH-1,
                                 "$c0005Spell : '$c0014%s$c0005' - $c0014%d$c0005",spells[aff->type-1],aff->duration);
						act(buf,FALSE, ch,0,0,TO_CHAR);
					}

					break;
				}
			}
		}
	}
}

ACTION_FUNC(do_value) {
	char buf[1000],buf2[1000], name[1000];
	struct obj_data* obj=0;
	struct char_data* vict=0;

	/* Spell Names */


	/* For Objects */


	if(!HasClass(ch, CLASS_THIEF|CLASS_RANGER)) {
		send_to_char("Scordatelo!!", ch);
		return;
	}

	arg = one_argument(arg, name);

	if((obj = get_obj_in_list_vis(ch, name, ch->carrying))==0) {
		if((vict = get_char_room_vis(ch, name))==0) {
			send_to_char("Di chi o di cosa stai parlando ?\n\r", ch);
			return;
		}
		else {
			only_argument(arg, name);
			if((obj = get_obj_in_list_vis(ch, name, vict->carrying))==0) {
				act("Non vedi $p addosso a $N", FALSE, ch, obj, vict, TO_CHAR);
				act("$n ti sta esaminando", FALSE, ch, 0, vict, TO_VICT);
				act("$n esamina $N", FALSE, ch, 0, vict, TO_NOTVICT);
				return;
			}
		}
	}

	WAIT_STATE(ch, PULSE_VIOLENCE*2);

	if(!SpyCheck(ch)) {
		/* failed spying check */
		if(obj && vict) {
			act("$n ti guarda ed i suoi occhi indugiano su $p",
				FALSE, ch, obj, vict, TO_VICT);
			act("$n studia $N", FALSE, ch, 0, vict, TO_ROOM);
		}
		else if(obj) {
			act("$n studia $p intensamente", FALSE, ch, obj, 0, TO_ROOM);
		}
		else {
			return;
		}
	}


	snprintf(buf,999, "Oggetto: %s.  Tipo: ", obj->short_description);
	sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
	strcat(buf,buf2);
	strcat(buf,"\n\r");
	send_to_char(buf, ch);

	if(!ch->skills) {
		return;
	}


	if(number(1,101) < ch->skills[SKILL_EVALUATE].learned/3) {
		if(obj->obj_flags.bitvector) {
			snprintf(buf, 999,"%s ti da' le seguenti abilita': ", fname((obj)->name));
			sprintbit((unsigned long)obj->obj_flags.bitvector,affected_bits,buf2);
			strcat(buf, buf2);
			strcat(buf,"\n\r");
			send_to_char(buf, ch);
		}
	}

	if(number(1,101) < ch->skills[SKILL_EVALUATE].learned/2) {
		send_to_char("L'oggetto e': ", ch);
		sprintbit2((unsigned long)obj->obj_flags.extra_flags, extra_bits, (unsigned long)obj->obj_flags.extra_flags2, extra_bits2, buf);
		strcat(buf,"\n\r");
		send_to_char(buf,ch);
	}

	snprintf(buf,999,"Peso: %d, Valore: %d, Rent cost: %d  %s\n\r",
			 obj->obj_flags.weight,
			 GetApprox(obj->obj_flags.cost, ch->skills[SKILL_EVALUATE].learned-10),
			 GetApprox(obj->obj_flags.cost_per_day,
					   ch->skills[SKILL_EVALUATE].learned-10),
			 obj->obj_flags.cost >= LIM_ITEM_COST_MIN ? "[RARO]" : " ");
	send_to_char(buf, ch);

	if(ITEM_TYPE(obj) == ITEM_WEAPON) {
		snprintf(buf, 999,"Valore di danno: '%dD%d'\n\r",
				 GetApprox(obj->obj_flags.value[1],
						   ch->skills[SKILL_EVALUATE].learned-10),
				 GetApprox(obj->obj_flags.value[2],
						   ch->skills[SKILL_EVALUATE].learned-10));
		send_to_char(buf, ch);
	}
	else if(ITEM_TYPE(obj) == ITEM_ARMOR) {

		snprintf(buf, 999,"L'AC e' %d\n\r",
				 GetApprox(obj->obj_flags.value[0],
						   ch->skills[SKILL_EVALUATE].learned-10));
		send_to_char(buf, ch);
	}
}

const char* AlignDesc(int a) {
	if(a<= -990) {
		return("Perfid$b come un Demone!");
	}
	else if(a <= -900) {
		return("Veramente malvagi$b");
	}
	else if(a <= -500) {
		return("Malvagi$b");
	}
	else if(a <= -351) {
		return("Cattiv$b");
	}
	else if(a <= -100) {
		return("Antipatic$b");
	}
	else if(a <= 100) {
		return("Bilanciat$b");
	}
	else if(a <= 350) {
		return("Simpatic$b");
	}
	else if(a <= 500) {
		return("Brav$b ragazz$b");
	}
	else if(a <= 900) {
		return("Buon$b");
	}
	else if(a <=990) {
		return("Troppo buon$b");
	}
	else {
		return("Angelic$b");
	}
}
const char* EqDesc(float a) {
	if(a >= 1400) {
		return("meglio di quanto credevo possibile!!");
	}
	else if(a >= 1200) {
		return("qualcosa di mai visto!");
	}
	else if(a >= 900) {
		return("il massimo!");
	}
	else if(a >= 600) {
		return("davvero notevole");
	}
	else if(a >= 400) {
		return("piuttosto buono");
	}
	else if(a >= 200) {
		return("del tutto ordinario");
	}
	else if(a >= 100) {
		return("scarso");
	}
	else if(a >= 10) {
		return("il minimo per non essere nudo");
	}
	else {
		return("ai limiti dell'ascesi!");
	}
}

const char* SpellfailDesc(int a) {
	if(a >= 100) {
		return("Terribile");
	}
	else if(a >= 90) {
		return("Scarsa");
	}
	else if(a >= 75) {
		return("Nella norma");
	}
	else if(a >= 55) {
		return("buona");
	}
	else if(a >= 40) {
		return("eccellente");
	}
	else {
		return("veramente eccezionale!");
	}
}


const char* ArmorDesc(int a) {
	if(a >= 90) {
		return("come se fossi nud$b");
	}
	else if(a >= 50) {
		return("difes$b");
	}
	else if(a >= 30) {
		return("ben difes$b");
	}
	else if(a >= 10) {
		return("protett$b");
	}
	else if(a >= -10) {
		return("ben protett$b");
	}
	else if(a >= -30) {
		return("corazzat$b");
	}
	else if(a >= -50) {
		return("ben corazzat$b");
	}
	else if(a >= -90) {
		return("molto ben corazzat$b");
	}
	else {
		return("corazzat$b come un Demone");
	}
}

const char* HitRollDesc(int a) {
	if(a < -5) {
		return("pessimo");
	}
	else if(a < -1) {
		return("basso");
	}
	else if(a <= 1) {
		return("nella media");
	}
	else if(a < 3) {
		return("non male");
	}
	else if(a < 8) {
		return("buono");
	}
	else if(a < 12) {
		return("molto buono");
	}
	else {
		return("eccellente");
	}

}

const char* DamRollDesc(int a) {
	if(a < -5) {
		return("pessimo");
	}
	else if(a < -1) {
		return("basso");
	}
	else if(a <= 1) {
		return("nella media");
	}
	else if(a < 3) {
		return("non male");
	}
	else if(a < 8) {
		return("buono");
	}
	else if(a< 12) {
		return("molto buono");
	}
	else {
		return("eccellente");
	}
}

const char* DescRatio(float f) { /* theirs / yours */
	if(f > 1.0) {
		return("maggiore del tuo");
	}
	else if(f > .75) {
		return("piu' di una volta e mezzo il tuo");
	}
	else if(f > .6) {
		return("almeno un terzo piu' grande del tuo");
	}
	else if(f > .4) {
		return("piu' o meno come il tuo");
	}
	else if(f > .3) {
		return("un po' piu' basso del tuo");
	}
	else if(f > .1) {
		return("molto piu' basso del tuo");
	}
	else {
		return("estremamente piu' basso del tuo");
	}
}
const char* DescArmorf(float f) { /* theirs / yours */
	if(f > 110.0) {
		return("estremamente peggiore della tua");
	}
	else if(f > 70.0) {
		return("molto peggiore della tua");
	}
	else if(f > 20.0) {
		return("peggiore della tua");
	}
	else if(f > -20.0) {
		return("piu' o meno come la tua");
	}
	else if(f > -70.) {
		return("migliore della tua");
	}
	else if(f > -110.0) {
		return("molto migliore della tua");
	}
	else {
		return("estremamente migliore della tua");
	}
}


const char* DescRatioF(float f) { /* theirs / yours */
	if(f > 1.0) {
		return("maggiore della tua");
	}
	else if(f > .75) {
		return("piu' di una volta e mezzo la tua");
	}
	else if(f > .6) {
		return("almeno un terzo piu' grande della tua");
	}
	else if(f > .4) {
		return("piu' o meno come la tua");
	}
	else if(f > .3) {
		return("un po' piu' bassa della tua");
	}
	else if(f > .1) {
		return("molto piu' bassa della tua");
	}
	else {
		return("estremamente piu' bassa della tua");
	}
}

const char* DescDamage(float dam) {
	if(dam < 1.0) {
		return("minimo");
	}
	else if(dam <= 2.0) {
		return("basso");
	}
	else if(dam <= 4.0) {
		return("avvertibile");
	}
	else if(dam <= 10.0) {
		return("alto");
	}
	else if(dam <= 15.0) {
		return("molto alto");
	}
	else if(dam <= 25.0) {
		return("doloroso");
	}
	else if(dam <= 35.0) {
		return("estremamente doloroso");
	}
	else {
		return("mortale");
	}
}

const char* DescAttacks(float a) {
	if(a < 1.0) {
		return("pochi");
	}
	else if(a < 2.0) {
		return("non piu' di due");
	}
	else if(a < 3.0) {
		return("un po' di");
	}
	else if(a < 5.0) {
		return("un bel po' di");
	}
	else if(a < 9.0) {
		return("molti");
	}
	else {
		return("un'enormita' di");
	}
}


ACTION_FUNC(do_display) {
	int i;

	if(IS_NPC(ch)) {
		return;
	}

	i = atoi(arg);

	switch(i) {
	case 0:
		if(ch->term == 0) {
			send_to_char("Modo di visualizzazione invariato.\n\r", ch);
			return;
		}
		ch->term = 0;
		ScreenOff(ch);
		send_to_char("Modo VT100 disattivato.\n\r", ch);
		return;

	case 1:
		if(ch->term == 1) {
			send_to_char("Modo di visualizzazione invariato.\n\r", ch);
			return;
		}
		ch->term = VT100;
		InitScreen(ch);
		send_to_char("Modo VT100 attivo.\n\r", ch);
		return;

	default:
		if(ch->term == VT100) {
			send_to_char("Modo VT100 attivo.\n\r", ch);
			return;
		}
		send_to_char("Modo VT100 disattivo.\n\r", ch);
		return;
	}
}

void ScreenOff(struct char_data* ch) {
	char buf[255];

	snprintf(buf, 254,VT_MARGSET, 0, ch->size- 1);
	send_to_char(buf, ch);
	send_to_char(VT_HOMECLR, ch);
}

ACTION_FUNC(do_resize) {
	int i;

	if(IS_NPC(ch)) {
		return;
	}

	i = atoi(arg);

	if(i < 7) {
		send_to_char("L'altezza dello schermo deve essere maggiore di 7.\n\r", ch);
		return;
	}

	if(i > 50) {
		send_to_char("L'altezza dello schermo deve essere minore di 50.\n\r",ch);
		return;
	}

	ch->size = i;

	if(ch->term == VT100) {
		ScreenOff(ch);
		InitScreen(ch);
	}

	send_to_char("Ok.\n\r", ch);
	return;
}

int MobLevBonus(struct char_data* ch) {
	int t=0;

	if(reinterpret_cast<uintptr_t>(mob_index[ ch->nr ].func) == reinterpret_cast<uintptr_t>(magic_user) ||IS_SET(ch->specials.act,ACT_MAGIC_USER)) {
		t+=5;
	}

	if(reinterpret_cast<uintptr_t>(mob_index[ch->nr].func) == reinterpret_cast<uintptr_t>(BreathWeapon)) {
		t+=7;
	}
	if(reinterpret_cast<uintptr_t>(mob_index[ch->nr].func) == reinterpret_cast<uintptr_t>(fighter) || IS_SET(ch->specials.act,ACT_WARRIOR)) {
		t+=3;
	}

	if(reinterpret_cast<uintptr_t>(mob_index[ch->nr].func) == reinterpret_cast<uintptr_t>(snake)) {
		t+=3;
	}

	t += (int)((ch->mult_att - 1) * 3);

	if(GET_HIT(ch) > GetMaxLevel(ch)*8) {
		t+=1;
	}
	if(GET_HIT(ch) > GetMaxLevel(ch)*12) {
		t+=2;
	}
	if(GET_HIT(ch) > GetMaxLevel(ch)*16) {
		t+=3;
	}
	if(GET_HIT(ch) > GetMaxLevel(ch)*20) {
		t+=4;
	}

	return(t);
}

ACTION_FUNC(do_show_skill) {
	char buf[254], buffer[MAX_STRING_LENGTH];
	int i,max;

	buffer[0]='\0';

	if(!ch->skills) {
		return;
	}

	for(; isspace(*arg); arg++);

	if(!arg) {
		send_to_char("Dovresti dirmi di che classe vuoi la lista.",ch);
		return;
	}

	switch(*arg) {
	case 'w':
	case 'W':
	case 'f':
	case 'F': {
		if(!HasClass(ch, CLASS_WARRIOR)) {
			send_to_char("Scommetto che pensi di essere un guerriero.\n\r", ch);
			return;
		}
		send_to_char("Non implementato per i guerrieri, per il momento\n\r", ch);
		break;
	}

	case 't':
	case 'T': {
		if(!HasClass(ch, CLASS_THIEF)) {
			send_to_char("Scommetto che pensi di essere un ladro.\n\r", ch);
			return;
		}
		send_to_char("Non implementato per i ladri, per il momento\n\r", ch);
		break;
	}
	case 'M':
	case 'm': {
		if(!HasClass(ch, CLASS_MAGIC_USER)) {
			send_to_char("Scommetto che pensi di essere un mago.\n\r", ch);
			return;
		}
		send_to_char("La tua classe puo' imparare i seguenti incantesimi:\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(max=0; max<IMMORTALE; max++) {
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						spell_info[i+1].min_level_magic==max) {
					snprintf(buf,254,"[%d] %s %s", spell_info[i+1].min_level_magic,
							 spells[i],how_good(ch->skills[i+1].learned));
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
		break;
	}
	case 'C':
	case 'c': {
		if(!HasClass(ch, CLASS_CLERIC)) {
			send_to_char("Scommetto che pensi di essere un sacerdote.\n\r", ch);
			return;
		}
		send_to_char("La tua classe puo' imparare i seguenti incantesimi:\n\r", ch);
		for(max=0; max<IMMORTALE; max++) {
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						spell_info[i+1].min_level_cleric==max) {
					snprintf(buf,254,"[%d] %s %s", spell_info[i+1].min_level_cleric,
							 spells[i],how_good(ch->skills[i+1].learned));
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
		break;
	}
	case 'D':
	case 'd': {
		if(!HasClass(ch, CLASS_DRUID)) {
			send_to_char("Scommetto che pensi di essere un druido.\n\r", ch);
			return;
		}
		send_to_char("La tua classe puo' imparare i seguenti incantesimi:\n\r", ch);
		for(max=0; max<IMMORTALE; max++) {
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						spell_info[i+1].min_level_druid==max) {
					snprintf(buf,254,"[%d] %s %s", spell_info[i+1].min_level_druid,
							 spells[i],how_good(ch->skills[i+1].learned));
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
		break;
	}
	case 'K':
	case 'k': {
		if(!HasClass(ch, CLASS_MONK)) {
			send_to_char("Scommetto che pensi di essere un monaco.\n\r", ch);
			return;
		}
		send_to_char("Non implementato per i monaci, per il momento\n\r", ch);

		break;
	}
	case 'b':
	case 'B': {
		if(!HasClass(ch, CLASS_BARBARIAN)) {
			send_to_char("Scommetto che pensi di essere un barbaro.\n\r", ch);
			return;
		}
		send_to_char("Non implementato per i barbari, per il momento\n\r", ch);
		break;
	}

	case 'S':
	case 's': {
		if(!HasClass(ch, CLASS_SORCERER)) {
			send_to_char("Scommetto che pensi di essere uno stregone.\n\r", ch);
			return;
		}
		send_to_char("La tua classe puo' imparare i seguenti incantesimi:\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(max=0; max<IMMORTALE; max++) {
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						spell_info[i+1].min_level_magic==max) {
					snprintf(buf,254,"[%d] %s %s", spell_info[i+1].min_level_magic,
							 spells[i],how_good(ch->skills[i+1].learned));
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
		}
		page_string(ch->desc, buffer, 1);
		return;

		break;
	}

	case 'p':
	case 'P': {
		if(!HasClass(ch, CLASS_PALADIN)) {
			send_to_char("Scommetto che pensi di essere un paladino.\n\r", ch);
			return;
		}
		send_to_char("La tua classe puo' imparare le seguenti abilita':\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(max=0; max<IMMORTALE; max++) {
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						spell_info[i+1].min_level_paladin==max) {
					snprintf(buf,254,"[%d] %s %s", spell_info[i+1].min_level_paladin,
							 spells[i],how_good(ch->skills[i+1].learned));
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
		break;
	}
	case 'R':
	case 'r': {
		if(!HasClass(ch, CLASS_RANGER)) {
			send_to_char("Scommetto che pensi di essere un ranger.\n\r", ch);
			return;
		}
		send_to_char("La tua classe puo' imparare le seguenti abilita':\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(max=0; max<IMMORTALE; max++) {
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer &&
						spell_info[i+1].min_level_ranger==max) {
					snprintf(buf,254,"[%d] %s %s", spell_info[i+1].min_level_ranger,
							 spells[i],how_good(ch->skills[i+1].learned));
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
		break;
	}
	case 'i':
	case 'I': {
		if(!HasClass(ch, CLASS_PSI)) {
			send_to_char("Scommetto che pensi di essere uno psionist.\n\r", ch);
			return;
		}
		send_to_char("La tua classe puo' imparare le seguenti abilita':\n\r", ch);
		SET_BIT(ch->player.user_flags,USE_PAGING);
		for(max=0; max<IMMORTALE; max++) {
			for(i=0; *spells[i] != '\n'; i++) {
				if(spell_info[i+1].spell_pointer && spell_info[i+1].min_level_psi==max) {
					snprintf(buf,254,"[%d] %s %s", spell_info[i+1].min_level_psi,
							 spells[i],how_good(ch->skills[i+1].learned));
					strcat(buf," \n\r");
					if(strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2) {
						break;
					}
					strcat(buffer, buf);
					strcat(buffer, "\r");
				}
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
		break;
	}
	default:
		send_to_char("Quale classe ?\n\r", ch);
	}
}

/* this command will only be used for immorts as I am using it as a way */
/* to figure out how to look into rooms next to this room. Will be using*/
/* the code for throwing items. I figure there is no IC reason for a PC */
/* to have a command like this. Do what ya want on your on MUD                 */
ACTION_FUNC(do_scan) {
	static const char* dir_desc[] = {
		"a nord",
		"ad est",
		"a sud",
		"ad ovest",
		"in alto",
		"in basso"
	};
	static const char* rng_desc[] = {
		"qui",
		"qui accanto",
		"nelle vicinanze",
		"andando",
		"andando",
		"laggiu'",
		"laggiu'",
		"in lontananza",
		"in lontananza"
	};
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
	int sd, smin, smax, swt, i, max_range = 6, range, rm, nfnd;
	struct char_data* spud;

	/*
	 * Check mortals spot skill, and give THEM a max scan of
	 * 2 rooms.
	 */

	if(!ch->skills) {
		send_to_char("Non hai nessuno skill!\n\r",ch);
		return;
	}

	if(GetMaxLevel(ch)<IMMORTALE) {
		if(!ch->skills[SKILL_SPOT].learned) {
			send_to_char("Non sei stato allenato per localizzare (spot).\n\r",ch);
			return;
		}

		if(dice(1,101) > ch->skills[SKILL_SPOT].learned) {
			send_to_char("Assolutamente nessuno da nessuna parte.\n\r",ch);
			WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
			return;
		}  /* failed */

		max_range=2;       /* morts can only spot two rooms away */

	}  /* was mortal */

	argument_split_2(arg,arg1,arg2);
	sd = search_block(arg1, dirs, FALSE);
	if(sd==-1) {
		smin = 0;
		smax = 5;
		swt = 3;
		snprintf(buf,MAX_STRING_LENGTH-1,"$n scruta intensamente i dintorni.");
		snprintf(buf2,MAX_STRING_LENGTH-1,"Guardandoti intorno con attenzione vedi...\n\r");
	}
	else {
		smin = sd;
		smax = sd;
		swt = 1;
		snprintf(buf,MAX_STRING_LENGTH-1,"$n scruta intensamente %s.",dirsTo[sd]);
		snprintf(buf2,MAX_STRING_LENGTH-1,"Guardando attentamente %s, vedi...\n\r",dirsTo[sd]);
	}

	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	send_to_char(buf2,ch);
	nfnd = 0;
	/* Check in room first */
	for(spud = real_roomp(ch->in_room)->people; spud;
			spud = spud->next_in_room) {
		if(CAN_SEE(ch, spud) && !IS_SET(spud->specials.affected_by, AFF_HIDE) &&
				spud != ch) {
			if(IS_NPC(spud)) {
				snprintf(buf,MAX_STRING_LENGTH-1, "%30s : qui\n\r", spud->player.short_descr);
			}
			else {
				snprintf(buf,MAX_STRING_LENGTH-1, "%30s : qui\n\r", GET_NAME(spud));
			}

			send_to_char(buf,ch);
			nfnd++;
		}
	}
	for(i=smin; i<=smax; i++) {
		rm = ch->in_room;
		range = 0;
		while(range<max_range) {
			range++;
			if(clearpath(ch, rm,i)) {
				if((rm = real_roomp(rm)->dir_option[i]->to_room) != ch->in_room) {
					for(spud=real_roomp(rm)->people; spud; spud=spud->next_in_room) {
						if(CAN_SEE(ch,spud) &&
								!IS_SET(spud->specials.affected_by,AFF_HIDE)) {
							if(IS_NPC(spud))
								snprintf(buf, MAX_STRING_LENGTH-1,
										 "%30s : %s %s\n\r", spud->player.short_descr,
										 rng_desc[range], dir_desc[ i ]);
							else
								snprintf(buf, MAX_STRING_LENGTH-1,
										 "%30s : %s %s\n\r", GET_NAME(spud),
										 rng_desc[range], dir_desc[i]);
							send_to_char(buf, ch);
							nfnd++;
						}
					}
				}
			}
			else {
				range = max_range + 1;
			}
		}
	}
	if(nfnd==0) {
		send_to_char("Assolutamente nessuno da nessuna parte.\n\r",ch);
	}
	WAIT_STATE(ch, swt * PULSE_VIOLENCE);
}

void CheckCharAffected(char* msg);

void list_groups(struct char_data* ch,const char* szArg, int iCmd) {
	struct descriptor_data* i;
	struct char_data* person;
	struct follow_type* f;
	int count = 0;
	char buf[MAX_STRING_LENGTH],tbuf[255];

	snprintf(buf,MAX_STRING_LENGTH-1,"$c0015[------- Gruppi di avventurieri -------]\n\r");

	/* go through the descriptor list */
	for(i = descriptor_list; i; i=i->next) {
		/* find everyone who is a master  */
		if(!i->connected) {
			person = i->character;
			/* list the master and the group name */
			if(person && !person->master && IS_AFFECTED(person, AFF_GROUP)) {
				if(person->specials.group_name && CAN_SEE(ch, person)) {
					snprintf(tbuf,254, "          $c0015%s\n\r$c0014%s\n\r",
							 person->specials.group_name, fname(GET_NAME(person)));
					strcat(buf,tbuf);

					/* list the members that ch can see */
					count = 0;
					for(f=person->followers; f; f=f->next) {
						if(f == NULL) {
							mudlog(LOG_ERROR, "person is affected by AFF_GROUP and don't "
								   "have followers.");
						}
						else if(IS_AFFECTED(f->follower, AFF_GROUP) && IS_PC(f->follower)) {
							count++;
							if(CAN_SEE(ch, f->follower) &&
									strlen(GET_NAME(f->follower)) > 1) {
								snprintf(tbuf,254,"$c0013%s\n\r", fname(GET_NAME(f->follower)));
								strcat(buf,tbuf);
							}
							else {
								snprintf(tbuf,254,"$c0013Qualcuno\n\r");
								strcat(buf,tbuf);
							}
						}
					}
					/* if there are no group members, then remove the group title */
					if(count < 1) {
						send_to_char("Il nome del tuo gruppo e' stato rimosso "
									 "poiche' il gruppo e' troppo piccolo.\n\r", person);
						free(person->specials.group_name);
						person->specials.group_name = 0;
					}
				}
			}
		}
	}
	strcat(buf,"\n\r$c0015[---------- Fine lista --------------]\n\r");
	page_string(ch->desc,buf,1);
}

int can_see_linear(struct char_data* ch, struct char_data* targ, int* rng,
				   int* dr) {
	int i, rm, max_range = 6, range = 0;
	struct char_data* spud;

	for(i=0; i<6; i++) {
		rm = ch->in_room;
		range = 0;
		while(range<max_range) {
			range++;
			if(clearpath(ch, rm,i)) {
				rm = real_roomp(rm)->dir_option[i]->to_room;
				for(spud = real_roomp(rm)->people; spud; spud = spud->next_in_room) {
					if((spud == targ) && CAN_SEE(ch, spud)) {
						*rng = range;
						*dr = i;
						return i;
					}
				}
			}
		}
	}
	return -1;
}

/**************************************************************************
 * Questa funzione restituisce il puntatore ad un personaggio se questo
 * esiste e se e' nella direzione indicata. La stringa arg deve essere nel
 * seguente formato: <dir> at <target> o semplicemente <target>.
 * Nel primo caso, cerca il personaggio con il nome <target> nella direzione
 * <dir>, nel secondo caso cerca il personaggio <target> nella locazione
 * di ch.
 * ***********************************************************************/

struct char_data* get_char_linear(struct char_data* ch,const char* arg, int* rf,
								  int* df) {
	long rm;
	int range = 0;
	struct char_data* spud;
	char tmpname[ MAX_STRING_LENGTH ];



	arg = one_argument(arg, tmpname);

	if((spud = get_char_room_vis(ch, tmpname)) != NULL) {
		*rf = 0;
		*df = -1;
		return spud;
	}

	*df = search_block(tmpname, exitKeywords, FALSE);
	if(*df < 0) {
		return NULL;
	}

	arg = one_argument(arg, tmpname);
	if(strn_cmp(tmpname, "at", 2) && isspace(tmpname[ 2 ])) {
		arg = one_argument(arg, tmpname);
	}

	rm = ch->in_room;
	range = 0;
	while(1) {
		range++;
		if(clearpath(ch, rm, *df)) {
			rm = real_roomp(rm)->dir_option[ *df ]->to_room;
			if((spud = get_char_near_room_vis(ch, tmpname, rm)) != NULL) {
				*rf = range;
				return spud;
			}
		}
		else {
			*rf = range;
			break;
		}
	}
	return NULL;
}
} // namespace Alarmud

