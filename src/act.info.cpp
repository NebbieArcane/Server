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
#include <array>
#include <iomanip>
#include <sstream>
#include <string>
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
#include "act.comm.hpp"
#include "act.off.hpp"
#include "act.other.hpp"
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
	return NumberOfConnection;
}

int singular(struct obj_data* o) {

	if(IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS) ||
			IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET) ||
			IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS) ||
			IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS)) {
		return FALSE;
	}
	return TRUE;
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

	return nullptr;
}


void show_obj_to_char(struct obj_data* object, struct char_data* ch, int mode) {
	char buffer[MAX_STRING_LENGTH];

	buffer[0] = 0;
	if((mode == 0) && object->description && *object->description) {
		std::strncpy(buffer, object->description, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';
		if(std::strlen(buffer) > 7 && buffer[1] == '$') {
			buffer[7] = UPPER(buffer[7]);
		}
		else if(buffer[0] != '\0') {
			CAP(buffer);
		}
	}
	else if(object->short_description &&
			(mode == 1 || mode == 2 || mode == 3 || mode == 4)) {
		std::strncpy(buffer, object->short_description, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';
		if(std::strlen(buffer) > 7 && buffer[1] == '$') {
			buffer[7] = UPPER(buffer[7]);
		}
		else if(buffer[0] != '\0') {
			CAP(buffer);
		}
	}
	else if(mode == 5) {
		if(object->obj_flags.type_flag == ITEM_NOTE) {
			if(object->action_description && *object->action_description) {
				std::string noteText = "C'e' scritto sopra qualcosa:\n\r\n\r";
				noteText += object->action_description;
				page_string(ch->desc, noteText.c_str(), 1);
			}
			else {
				act("Non c'e' scritto nulla.", FALSE, ch, 0, 0, TO_CHAR);
			}
			return;  /* mail fix, thanks brett */

		}
		else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
			std::strncpy(buffer, "Non vedi nulla di speciale...", sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
		}
		else {
			/* ITEM_TYPE == ITEM_DRINKCON */
			std::strncpy(buffer, "Sembra un contenitore per bevande.", sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
		}
	}

	std::string output = buffer;
	if(mode != 3) {
		if(IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
			output += "$c0011 (invisibile)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_ANTI_GOOD) &&
				IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
			if(singular(object)) {
				output += "$c0009 (ha un alone di luce rossa)$c0007";
			}
			else {
				output += "$c0009 (hanno un alone di luce rossa)$c0007";
			}
		}
		if(IS_OBJ_STAT(object, ITEM_MAGIC) &&
				IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
			if(singular(object)) {
				output += "$c0012 (ha un alone di luce blu)$c0007";
			}
			else {
				output += "$c0012 (hanno un alone di luce blu)$c0007";
			}
		}
		if(IS_OBJ_STAT(object, ITEM_GLOW)) {
			if(singular(object)) {
				output += "$c0015 (ha un alone luminoso)$c0007";
			}
			else {
				output += "$c0015 (hanno un alone luminoso)$c0007";
			}
		}
		if(IS_OBJ_STAT(object, ITEM_HUM)) {
			if(singular(object)) {
				output += "$c0008 (emette un forte ronzio)$c0007";
			}
			else {
				output += "$c0008 (emettono un forte ronzio)$c0007";
			}
		}
		if(object->obj_flags.type_flag == ITEM_ARMOR) {
			if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 4)) {
				if(singular(object)) {
					output += "$c0009 (distrutto)$c0007";
				}
				else {
					output += "$c0009 (sono distrutti)$c0007";
				}
			}
			else if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 3)) {
				if(singular(object)) {
					output += "$c0009 (ha bisogno di essere riparato)$c0007";
				}
				else {
					output += "$c0009 (hanno bisogno di essere riparati)$c0007";
				}
			}
			else if(object->obj_flags.value[ 0 ] <
					(object->obj_flags.value[ 1 ] / 2)) {
				if(singular(object)) {
					output += "$c0011 (in buone condizioni)$c0007";
				}
				else {
					output += "$c0011 (in buone condizioni)$c0007";
				}
			}
			else if(object->obj_flags.value[ 0 ] <
					object->obj_flags.value[1]) {
				if(singular(object)) {
					output += "$c0010 (in ottime condizioni)$c0007";
				}
				else {
					output += "$c0010 (in ottime condizioni)$c0007";
				}
			}
			else {
				if(singular(object)) {
					output += "$c0010 (in condizioni eccellenti)$c0007";
				}
				else {
					output += "$c0010 (in condizioni eccellenti)$c0007";
				}
			}
		}
	}
	if(!output.empty()) {
		output += "\n\r";
		page_string(ch->desc, output.c_str(), 1);
	}
}

void show_mult_obj_to_char(struct obj_data* object, struct char_data* ch,
						   int mode, int num) {
	char buffer[ MAX_STRING_LENGTH ];

	buffer[0] = 0;

	if((mode == 0) && object->description && *object->description) {
		std::strncpy(buffer, object->description, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';
		if(std::strlen(buffer) > 7 && buffer[1] == '$') {
			buffer[7] = UPPER(buffer[7]);
		}
		else if(buffer[0] != '\0') {
			CAP(buffer);
		}
	}
	else if(object->short_description && ((mode == 1) ||
										  (mode == 2) || (mode == 3) || (mode == 4))) {
		std::strncpy(buffer, object->short_description, sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';
		if(std::strlen(buffer) > 7 && buffer[1] == '$') {
			buffer[7] = UPPER(buffer[7]);
		}
		else if(buffer[0] != '\0') {
			CAP(buffer);
		}
	}
	else if(mode == 5) {
		if(object->obj_flags.type_flag == ITEM_NOTE) {
			if(object->action_description && *object->action_description) {
				std::string noteText = "C'e' scritto sopra qualcosa:\n\r\n\r";
				noteText += object->action_description;
				page_string(ch->desc, noteText.c_str(), 1);
			}
			else {
				act("Non c'e' scritto nulla.", FALSE, ch, 0, 0, TO_CHAR);
			}
			return;
		}
		else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
			std::strncpy(buffer, "Non vedi nulla di speciale...", sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
		}
		else {
			/* ITEM_TYPE == ITEM_DRINKCON */
			std::strncpy(buffer, "Sembra un contenitore per bevande.", sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
		}
	}

	std::string output = buffer;
	if(mode != 3) {
		if(IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
			output += " $c0011(invisibile)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_ANTI_GOOD) &&
				IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
			output += " $c0009(ha un alone di luce rossa)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_MAGIC) &&
				IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
			output += " $c0012(ha un alone di luce blu)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_GLOW)) {
			output += " $c0015(ha un alone luminoso)$c0007";
		}
		if(IS_OBJ_STAT(object, ITEM_HUM)) {
			output += " $c0008(emette un forte ronzio)$c0007";
		}
	}

	if(num > 1) {
		output += "[";
		output += std::to_string(num);
		output += "]";
	}
	if(!output.empty()) {
		output += "\n\r";
		page_string(ch->desc, output.c_str(), 1);
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
	std::string szBuf = "$n e' ";

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

	if(pChar->player.height > 250) {
		szBuf += DescAltezze[ 0 ];
	}
	else if(pChar->player.height > 190) {
		szBuf += DescAltezze[ 1 ];
	}
	else if(pChar->player.height > 170) {
		szBuf += DescAltezze[ 2 ];
	}
	else if(pChar->player.height > 155) {
		szBuf += DescAltezze[ 3 ];
	}
	else if(pChar->player.height > 140) {
		szBuf += DescAltezze[ 4 ];
	}
	else {
		szBuf += DescAltezze[ 5 ];
	}

	szBuf += " e ";

	fRapp = static_cast<float>(GET_HEIGHT(pChar)) /
			(static_cast<float>(pChar->player.weight) * 0.4536f);


	if(fRapp > 3.27) {
		szBuf += DescCostituzione[ 5 ];
	}
	else if(fRapp > 3) {
		szBuf += DescCostituzione[ 4 ];
	}
	else if(fRapp > 2.25) {
		szBuf += DescCostituzione[ 3 ];
	}
	else if(fRapp > 2) {
		szBuf += DescCostituzione[ 2 ];
	}
	else if(fRapp > 1.6) {
		szBuf += DescCostituzione[ 1 ];
	}
	else {
		szBuf += DescCostituzione[ 0 ];
	}

	szBuf += ".";
	act(szBuf.c_str(), FALSE, pChar, NULL, pTo, TO_VICT);
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
			std::string line;
			if(!IS_NPC(i)) {
				std::string playerLine = GET_NAME(i);
				playerLine += " ";
				if(GET_TITLE(i)) {
					playerLine += GET_TITLE(i);
				}
				line = playerLine;
			}
			else {
				std::strncpy(buffer, i->player.short_descr, sizeof(buffer) - 1);
				buffer[sizeof(buffer) - 1] = '\0';
				if(buffer[1] == '$') {
					buffer[7] = UPPER(buffer[7]);
				}
				else {
					CAP(buffer);
				}
				line = buffer;
			}

			if(IS_AFFECTED(i, AFF_INVISIBLE) || i->invis_level >= IMMORTALE) {
				line += " $c0011(invisibile)$c0007";
			}
			if(IS_AFFECTED(i, AFF_CHARM)) {
				line += " $c0015(schiavo)$c0007";
			}

			switch(GET_POS(i)) {
			case POSITION_STUNNED:
				line += " giace qui, svenut$b.";
				break;
			case POSITION_INCAP:
				line += " giace qui, incapacitat$b.";
				break;
			case POSITION_MORTALLYW:
				line += " giace qui, ferit$b a morte.";
				break;
			case POSITION_DEAD:
				line += " giace qui, mort$b.";
				break;
			case POSITION_MOUNTED:
				if(MOUNTED(i)) {
					line += " e' qui, cavalcando ";
					line += MOUNTED(i)->player.short_descr;
				}
				else {
					line += " e' qui, in piedi.";
				}
				break;
			case POSITION_STANDING :
				if(!IS_AFFECTED(i, AFF_FLYING) &&
						!affected_by_spell(i,SKILL_LEVITATION)) {
					if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
						line += " sta galleggiando qui.";
					}
					else {
						line += " e' qui, in piedi.";
					}
				}
				else {
					line += " vola qui intorno.";
				}
				break;
			case POSITION_SITTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta galleggiando qui.";
				}
				else {
					line += " e' sedut$b qui.";
				}
				break;
			case POSITION_RESTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta facendo il morto nell'$c0012acqua$c0007.";
				}
				else {
					line += " sta riposando qui.";
				}
				break;
			case POSITION_SLEEPING :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta dormendo qui nell'$c0012acqua$c0007.";
				}
				else {
					line += " sta dormendo qui.";
				}
				break;
			case POSITION_FIGHTING :
				if(i->specials.fighting) {
					line += " e' qui, combattendo contro ";
					if(i->specials.fighting == ch) {
						line += " DI TE!";
					}
					else {
						if(i->in_room == i->specials.fighting->in_room) {
							if(IS_NPC(i->specials.fighting)) {
								line += i->specials.fighting->player.short_descr;
							}
							else {
								line += GET_NAME(i->specials.fighting);
							}
						}
						else {
							line += "qualcuno che se ne e' appena andato.";
						}
					}
				}
				else { /* NIL fighting pointer */
					line += " e' qui dimenandosi contro l'aria.";
				}
				break;
			default:
				line += " fluttua qui intorno.";
				break;
			} /*switch */

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				line += "$c0006 (AFK)$c0007";
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
				if(IS_EVIL(i)) {
					line += "$c0009 (alone rosso)$c0007";
				}
			}

			if(IS_LINKDEAD(i)) {
				line += "$c0015 (link dead)$c0007";
			}

			act(line.c_str(), FALSE, i, 0, ch, TO_VICT);
			if(!IS_AFFECTED(ch,AFF_TRUE_SIGHT)) {
				for(j=how_many_spell(i, SPELL_MIRROR_IMAGES); j>0; j--) {
					act(line.c_str(), FALSE, i, 0, ch, TO_VICT);
				}
			}

		}
		else {
			/* npc with long */

			std::string longLine;
			if(IS_AFFECTED(i, AFF_INVISIBLE)) {
				longLine = "*";
			}

			if(IS_LINKDEAD(i)) {
				longLine += "$c0015(link dead)$c0007 ";
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
				longLine += "$c0009(alone rosso)$c0007 ";
			}

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				longLine += "$c0006(AFK)$c0007 ";
			}

			longLine += i->player.long_descr;

			while(!longLine.empty() &&
					(longLine.back() == '\r' ||
					 longLine.back() == '\n' ||
					 longLine.back() == ' ')) {
				longLine.pop_back();
			}
			act(longLine.c_str(), FALSE, i, 0, ch, TO_VICT);
		}

        if(IS_PC(i))
        {
            std::strncpy(buffer2, i->player.name, sizeof(buffer2) - 1);
            buffer2[sizeof(buffer2) - 1] = '\0';
        }
        else
        {
            std::strncpy(buffer2, i->player.short_descr, sizeof(buffer2) - 1);
            buffer2[sizeof(buffer2) - 1] = '\0';
            RemColorString(buffer2);
            CAP(buffer2);
        }

		if(IS_AFFECTED(i, AFF_SANCTUARY))
        {
			if(!IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
            {
				std::string auraMsg = "$c0015";
				auraMsg += buffer2;
				auraMsg += " brilla di luce propria!";
				act(auraMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
			}
		}

		if(IS_AFFECTED(i, AFF_GROWTH))
        {
			std::string growthMsg = "$c0003";
			growthMsg += buffer2;
			growthMsg += " e' enorme!";
			act(growthMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
		}

		if(IS_AFFECTED(i, AFF_FIRESHIELD))
        {
			if(!IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
            {
				std::string fireMsg = "$c0001";
				fireMsg += buffer2;
				fireMsg += " e' avvolt$b in una luce fiammeggiante!";
				act(fireMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
            }
		}

		if(IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
        {
			std::string darkMsg = "$c0008";
			darkMsg += buffer2;
			darkMsg += " e' avvolt$b nell'oscurita'!";
			act(darkMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
		}

	}
	else if(mode == 1) {
		if(i->player.description) {
			std::string description = i->player.description;
			if(description.size() > 1 && description[1] == '$') {
				if(description.size() > 7) {
					description[7] = UPPER(description[7]);
				}
			}
			else if(!description.empty()) {
				description[0] = UPPER(description[0]);
			}
			send_to_char(description.c_str(), ch);
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
			std::string clanMsg = "$n appartiene al clan di ";
			clanMsg += (GET_PRINCE(i) ? GET_PRINCE(i) : "");
			clanMsg += ".";
			act(clanMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
		}

		/*
		 * personal descriptions.
		 */

		ShowAltezzaCostituzione(i, ch);

		if(IS_PC(i)) {
			std::string raceMsg = "$n e' un$b ";
			raceMsg += RaceName[ GET_RACE(i) ];
			raceMsg += ".";
			act(raceMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
		}


		if(MOUNTED(i)) {
			std::string mountMsg = "$n sta cavalcando ";
			mountMsg += MOUNTED(i)->player.short_descr;
			mountMsg += ".";
			act(mountMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
		}

		if(RIDDEN(i)) {
			std::string riddenMsg = "$n e' cavalcat$b da ";
			riddenMsg += IS_NPC(RIDDEN(i)) ? RIDDEN(i)->player.short_descr :
						 GET_NAME(RIDDEN(i));
			riddenMsg += ".";
			act(riddenMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
		}

		/* Show a character to another */

		if(GET_MAX_HIT(i) > 0) {
			percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
		}
		else {
			percent = -1;    /* How could MAX_HIT be < 1?? */
		}

		std::string conditionMsg;
		if(IS_NPC(i)) {
			std::strncpy(buffer, i->player.short_descr, sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
			if(buffer[1] == '$') {
				buffer[7] = UPPER(buffer[7]);
			}
			else {
				CAP(buffer);
			}
			conditionMsg = buffer;
		}
		else {
			std::strncpy(buffer, GET_NAME(i), sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
			conditionMsg = buffer;
		}

		if(percent >= 100) {
			conditionMsg += " e' in condizioni $c0010eccellenti$c0007.";
		}
		else if(percent >= 80) {
			conditionMsg += " ha pochi graffi.";
		}
		else if(percent >= 60) {
			conditionMsg += " ha alcuni tagli ed abrasioni.";
		}
		else if(percent >= 40) {
			conditionMsg += " e' ferit$B.";
		}
		else if(percent >= 20) {
			conditionMsg += " $c0001sanguina$c0007 abbondantemente.";
		}
		else if(percent >= 0) {
			conditionMsg += " $c0001ha grossi squarci aperti$c0007.";
		}
		else {
			conditionMsg += " $c0009sta morendo per le ferite ed i colpi ricevuti$c0007.";
		}

		act(conditionMsg.c_str(), FALSE, ch, 0, i, TO_CHAR);


		/*
		 * spell_descriptions, etc.
		 */
		otype = -1;
        if((IS_AFFECTED(i, AFF_GLOBE_DARKNESS) || affected_by_spell(i, SPELL_GLOBE_DARKNESS)) && (!is_same_group(i, ch) && !IS_IMMORTALE(ch) && ch != i && saves_spell(i, SAVING_SPELL)))
        {
            act(spell_desc[ SPELL_GLOBE_DARKNESS ], FALSE, i, 0, ch, TO_VICT);
        }
        else
        {
            for(aff = i->affected; aff; aff = aff->next)
            {
                if(aff->type < MAX_EXIST_SPELL)
                {
                    if(spell_desc[ aff->type ] && *spell_desc[ aff->type ])
                    {
                        if(aff->type != otype)
                        {
                            act(spell_desc[ aff->type ], FALSE, i, 0, ch, TO_VICT);
                            otype = aff->type;
                        }
                    }
                }
            }
            if(IS_AFFECTED(i, AFF_SANCTUARY) && !affected_by_spell(i, SPELL_SANCTUARY))
            {
                act(spell_desc[ SPELL_SANCTUARY ], FALSE, i, 0, ch, TO_VICT);
            }
            if(IS_AFFECTED(i, AFF_FIRESHIELD) && !affected_by_spell(i, SPELL_FIRESHIELD))
            {
                act(spell_desc[ SPELL_FIRESHIELD ], FALSE, i, 0, ch, TO_VICT);
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
				std::string playerLine = GET_NAME(i);
				playerLine += " ";
				if(GET_TITLE(i)) {
					playerLine += GET_TITLE(i);
				}
				std::strncpy(buffer, playerLine.c_str(), sizeof(buffer) - 1);
				buffer[sizeof(buffer) - 1] = '\0';
			}
			else {
				std::strncpy(buffer, i->player.short_descr, sizeof(buffer) - 1);
				buffer[sizeof(buffer) - 1] = '\0';
				if(buffer[1] == '$') {
					buffer[7] = UPPER(buffer[7]);
				}
				else {
					CAP(buffer);
				}
			}

			std::string line = buffer;
			if(IS_AFFECTED(i, AFF_INVISIBLE)) {
				line += "$c0011 (invisibile)";
			}
			if(IS_AFFECTED(i, AFF_CHARM)) {
				line += "$c0010 (schiavo)";
			}

			switch(GET_POS(i)) {
			case POSITION_STUNNED:
				line += " giace qui, svenut$b.";
				break;
			case POSITION_INCAP:
				line += " giace qui, incapacitat$b.";
				break;
			case POSITION_MORTALLYW:
				line += " giace qui, ferit$b a morte.";
				break;
			case POSITION_DEAD:
				line += " giace qui, mort$b.";
				break;
			case POSITION_MOUNTED:
				if(MOUNTED(i)) {
					line += " e' qui, montando ";
					line += MOUNTED(i)->player.short_descr;
				}
				else {
					line += " e' qui, in piedi.";
				}
				break;
			case POSITION_STANDING :
				if(!IS_AFFECTED(i, AFF_FLYING) &&
						!affected_by_spell(i,SKILL_LEVITATION)) {
					if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
						line += " sta galleggiano qui.";
					}
					else {
						line += " e' qui, in piedi.";
					}
				}
				else {
					line += " vola qui intorno.";
				}
				break;
			case POSITION_SITTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta galleggiando qui.";
				}
				else {
					line += " e' sedut$b qui.";
				}
				break;
			case POSITION_RESTING  :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta riposando qui nell'acqua.";
				}
				else {
					line += " sta riposando qui.";
				}
				break;
			case POSITION_SLEEPING :
				if(real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM) {
					line += " sta dormendo qui nell'acqua.";
				}
				else {
					line += " sta dormendo qui.";
				}
				break;
			case POSITION_FIGHTING :
				if(i->specials.fighting) {
					line += " e' qui, combattendo contro ";
					if(i->specials.fighting == ch) {
						line += " DI TE!";
					}
					else {
						if(i->in_room == i->specials.fighting->in_room) {
							if(IS_NPC(i->specials.fighting)) {
								line += i->specials.fighting->player.short_descr;
							}
							else {
								line += GET_NAME(i->specials.fighting);
							}
						}
						else {
							line += "qualcuno che se ne e' appena andato.";
						}
					}
				}
				else { /* NIL fighting pointer */
					line += " e' qui dimenandosi contro l'aria.";
				}
				break;
			default:
				line += " fluttua qui intorno.";
				break;
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
				line += "$c0009 (alone rosso)$c0007";
			}

			if(IS_LINKDEAD(i)) {
				line += "$c0015 (link dead)$c0007";
			}

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				line += "$c0006 (AFK)$c0007";
			}

			if(num > 1) {
				line += " [";
				line += std::to_string(num);
				line += "]";
			}
			act(line.c_str(), FALSE, i, 0, ch, TO_VICT);
		}
		else {
			/* npc with long */

			std::string longLine;
			if(IS_AFFECTED(i, AFF_INVISIBLE)) {
				longLine = "*";
			}

			if(IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
				longLine += "$c0009(alone rosso)$c0007 ";
			}

			if(IS_LINKDEAD(i)) {
				longLine += "$c0015(link dead)$c0007 ";
			}

			if(IS_AFFECTED2(i, AFF2_AFK)) {
				longLine += "$c0006(AFK)$c0007 ";
			}

			longLine += i->player.long_descr;

			/* this gets a little annoying */

			if(num > 1) {
				while(!longLine.empty() &&
						(longLine.back() == '\r' ||
						 longLine.back() == '\n' ||
						 longLine.back() == ' ')) {
					longLine.pop_back();
				}
				longLine += " [";
				longLine += std::to_string(num);
				longLine += "]";
			}

			act(longLine.c_str(), FALSE, i, 0, ch, TO_VICT);
		}

        if(IS_PC(i))
        {
            std::strncpy(buffer2, i->player.name, sizeof(buffer2) - 1);
            buffer2[sizeof(buffer2) - 1] = '\0';
        }
        else
        {
            std::strncpy(buffer2, i->player.short_descr, sizeof(buffer2) - 1);
            buffer2[sizeof(buffer2) - 1] = '\0';
            RemColorString(buffer2);
            CAP(buffer2);
        }

        if(IS_AFFECTED(i, AFF_SANCTUARY))
        {
            if(!IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
            {
                std::string auraMsg = "$c0015";
                auraMsg += buffer2;
                auraMsg += " brilla di luce propria!";
                act(auraMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
            }
        }

        if(IS_AFFECTED(i, AFF_GROWTH))
        {
            std::string growthMsg = "$c0003";
            growthMsg += buffer2;
            growthMsg += " e' enorme!";
            act(growthMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
        }

        if(IS_AFFECTED(i, AFF_FIRESHIELD))
        {
            if(!IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
            {
                std::string fireMsg = "$c0001";
                fireMsg += buffer2;
                fireMsg += " e' avvolt$b in una luce fiammeggiante!";
                act(fireMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
            }
        }

        if(IS_AFFECTED(i, AFF_GLOBE_DARKNESS))
        {
            std::string darkMsg = "$c0008";
            darkMsg += buffer2;
            darkMsg += " e' avvolt$b nell'oscurita'!";
            act(darkMsg.c_str(), FALSE, i, 0, ch, TO_VICT);
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

		std::string conditionMsg;
		if(IS_NPC(i)) {
			std::strncpy(buffer, i->player.short_descr, sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
			conditionMsg = buffer;
		}
		else {
			std::strncpy(buffer, GET_NAME(i), sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
			conditionMsg = buffer;
		}

		if(percent >= 100) {
			conditionMsg += " e' in condizioni eccellenti.";
		}
		else if(percent >= 80) {
			conditionMsg += " ha pochi graffi.";
		}
		else if(percent >= 60) {
			conditionMsg += " ha alcuni tagli ed abrasioni.";
		}
		else if(percent >= 40) {
			conditionMsg += " e' ferit$B.";
		}
		else if(percent >= 20) {
			conditionMsg += " sanguina abbondatemente.";
		}
		else if(percent >= 0) {
			conditionMsg += " $c0001ha grossi squarci aperti.";
		}
		else {
			conditionMsg += " $c0009sta morendo per le ferite ed i colpi ricevuti.";
		}

		act(conditionMsg.c_str(), FALSE, ch, 0, i, TO_CHAR);

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
	int door, seeit = FALSE;
	std::string buf;
	struct room_direction_data* exitdata;

	for(door = 0; door <= 5; door++) {
		exitdata = EXIT(ch, door);
		if(exitdata) {
			if(real_roomp(exitdata->to_room)) {
				if(GET_RACE(ch) == RACE_ELVEN ||
						GET_RACE(ch) == RACE_GOLD_ELF ||
						GET_RACE(ch) == RACE_WILD_ELF ||
						GET_RACE(ch) == RACE_SEA_ELF)
					/* elves can see secret doors 1-3 on d6 */
				{
					seeit = (number(1, 6) <= 3);
				}
				else if(GET_RACE(ch) == RACE_HALF_ELVEN)
					/* half-elves can see exits, not as good as full */
				{
					seeit = (number(1, 6) <= 2);
				}
				else if(GET_RACE(ch) == RACE_DWARF ||
						GET_RACE(ch) == RACE_DARK_DWARF) {
					seeit = (number(1, 12) <= 7);
				}
				/* I nani le vedono meglio di tutti!!!! */
				else {
					seeit = FALSE;
				}

				if(exitdata->to_room != NOWHERE || IS_IMMORTAL(ch)) {
					if((!IS_SET(exitdata->exit_info, EX_CLOSED) ||
							IS_IMMORTAL(ch)) ||
							(IS_SET(exitdata->exit_info, EX_SECRET) && seeit)) {
						buf += ' ';
						buf += listexits[door];
						if(IS_SET(exitdata->exit_info, EX_CLOSED) && IS_IMMORTAL(ch)) {
							buf += " (chiuso)";
						}
						if(IS_SET(exitdata->exit_info, EX_SECRET) && (seeit ||
								IS_IMMORTAL(ch))) {
							buf += " $c5009(segreto)$c0007";    /* blink red */
						}
					} /* exit */
				} /* ! = NOWHERE */
			}    /* real_roomp */
		} /* exitdata */
	} /* for */

	if(!buf.empty()) {
		send_to_char("Uscite:", ch);
		act(buf.c_str(), FALSE, ch, 0, 0, TO_CHAR);
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
				send_to_char("Guardare a cosa?\n\r", ch);
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
			//	mudlog(LOG_ERROR,"Blood trovato: %d",RM_BLOOD(ch->in_room));
				act(blood_messages[static_cast<int8_t>(RM_BLOOD(ch->in_room))], FALSE, ch, 0, 0, TO_CHAR);
			}
			else {
			//	mudlog(LOG_ERROR,"Blood trovato: %d",RM_BLOOD(ch->in_room));
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
	/* This is just for now - To be changed later.! */
	std::string lookArg = "at ";
	lookArg += (arg != nullptr) ? arg : "";
	do_look(ch, lookArg.c_str(), 15);
}



ACTION_FUNC(do_examine) {
	char name[1000];
	struct char_data* tmp_char;
	struct obj_data* tmp_object;

	std::string lookAtArg = "at ";
	lookAtArg += (arg != nullptr) ? arg : "";
	do_look(ch, lookAtArg.c_str(), 15);

	one_argument(arg, name);

	if(!*name) {
		send_to_char("Esaminare che cosa?\n\r", ch);
		return;
	}

	generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if(tmp_object) {
		if((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
				(GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
			send_to_char("Quando ci guardi dentro, vedi:\n\r", ch);
			std::string lookInArg = "in ";
			lookInArg += (arg != nullptr) ? arg : "";
			do_look(ch, lookInArg.c_str(), 15);
		}
	}
}

/**************************************************************************
 * do_exits visualizza le uscite della locazione.
 **************************************************************************/
ACTION_FUNC(do_exits) {
	/* NOTE: Input var 'cmd' is not used. */
	int door;
	std::string buf;
	struct room_direction_data* exitdata;

	for(door = 0; door <= 5; door++) {
		exitdata = EXIT(ch, door);
		if(exitdata && exitdata->to_room > 0) {
			if(!real_roomp(exitdata->to_room)) {
				/* don't print unless immortal */
				if(IS_IMMORTAL(ch)) {
					buf += "$c0007";
					buf += exits[door];
					buf += " $c0015- $c0007turbinante chaos di $c0015#";
					buf += std::to_string(exitdata->to_room);
					buf += "\n\r";
				}
			}
			else if(exitdata->to_room != NOWHERE) {
				if(IS_DIO(ch)) {
					buf += "$c0007";
					buf += exits[door];
					buf += " $c0015-$c0007 ";
					buf += real_roomp(exitdata->to_room)->name;
					if(IS_SET(exitdata->exit_info, EX_SECRET)) {
						buf += " $c0009(segreta)";
					}
					if(IS_SET(exitdata->exit_info, EX_CLOSED)) {
						buf += " $c0015(chiusa)";
					}
					if(IS_DARK(exitdata->to_room)) {
						buf += " $c0008(buia)";
					}
					buf += " $c0015#";
					buf += std::to_string(exitdata->to_room);
					buf += "\n\r";
				}
				else if(!IS_SET(exitdata->exit_info, EX_CLOSED) ||
						!IS_SET(exitdata->exit_info, EX_SECRET)) {
					buf += "$c0007";
					buf += exits[door];
					if(IS_DARK(exitdata->to_room)) {
						buf += " $c0015-$c0008 Troppo buio per dirlo\n\r";
					}
					else {
						buf += " $c0015-$c0007 ";
						buf += real_roomp(exitdata->to_room)->name;
						buf += "\n\r";
					}
				}
			}
		}
	}

	send_to_char("Uscite visibili:\n\r", ch);

	if(!buf.empty()) {
		send_to_char(buf.c_str(), ch);
	}
	else {
		send_to_char("$c0009Nessuna!\n\r", ch);
	}
}

ACTION_FUNC(do_status) {
	std::string statusMsg = "$c0005Tu hai $c0015";
	statusMsg += std::to_string(static_cast<int>(GET_HIT(ch)));
	statusMsg += "$c0005($c0011";
	statusMsg += std::to_string(GET_MAX_HIT(ch));
	statusMsg += "$c0005) hit, $c0015";
	statusMsg += std::to_string(static_cast<int>(GET_MANA(ch)));
	statusMsg += "$c0005($c0011";
	statusMsg += std::to_string(GET_MAX_MANA(ch));
	statusMsg += "$c0005) mana e $c0015";
	statusMsg += std::to_string(static_cast<int>(GET_MOVE(ch)));
	statusMsg += "$c0005($c0011";
	statusMsg += std::to_string(GET_MAX_MOVE(ch));
	statusMsg += "$c0005) punti di movimento.";
	act(statusMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	std::string expMsg = "$c0005Hai effettuato $c0015";
	expMsg += std::to_string(GET_EXP(ch));
	expMsg += "$c0005 exp, ed hai $c0015";
	expMsg += std::to_string(GET_GOLD(ch));
	expMsg += "$c0005 monete d'oro.";
	act(expMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
}

ACTION_FUNC(do_checkachielevel)
{
    int i, num = 0;
    std::string sb;

    if(strcmp(GET_NAME(ch), "Croneh") && strcmp(GET_NAME(ch), "Alar") && strcmp(GET_NAME(ch), "Requiem"))
    {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    sb.append("$c0011I seguenti achievement non hanno impostato il grado di difficolta':\n\r");
    for(i = 0; i < MAX_RACE_ACHIE; i++)
    {
        if(AchievementsList[RACESLAYER_ACHIE][i].grado_diff == 0)
        {
            num += 1;
            boost::format fmt("$c0009%6d $c0010%-12s %6d %-34s %s\n\r");
            fmt % num % "Raceslayer:" % AchievementsList[RACESLAYER_ACHIE][i].achie_number % AchievementsList[RACESLAYER_ACHIE][i].achie_string2 % (AchievementsList[RACESLAYER_ACHIE][i].classe == 0 ? "tutte le classi" : AchievementsList[RACESLAYER_ACHIE][i].classe == -1 ? "non abilitato" : pc_class_types[AchievementsList[RACESLAYER_ACHIE][i].classe]);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }

    for(i = 0; i < MAX_BOSS_ACHIE; i++)
    {
        if(AchievementsList[BOSSKILL_ACHIE][i].grado_diff == 0)
        {
            num += 1;
            boost::format fmt("$c0009%6d $c0011%-12s %6d %-34s %s\n\r");
            fmt % num % "Bosskill:" % AchievementsList[BOSSKILL_ACHIE][i].achie_number % AchievementsList[BOSSKILL_ACHIE][i].achie_string2 % (AchievementsList[BOSSKILL_ACHIE][i].classe == 0 ? "tutte le classi" : AchievementsList[BOSSKILL_ACHIE][i].classe == -1 ? "non abilitato" : pc_class_types[AchievementsList[BOSSKILL_ACHIE][i].classe]);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }

    for(i = 1; i < MAX_CLASS_ACHIE; i++)
    {
        if(AchievementsList[CLASS_ACHIE][i].grado_diff == 0)
        {
            num += 1;
            boost::format fmt("$c0009%6d $c0012%%-12s %6d %-34s %s\n\r");
            fmt % num % "Class:" % AchievementsList[CLASS_ACHIE][i].achie_number % AchievementsList[CLASS_ACHIE][i].achie_string2 % (AchievementsList[CLASS_ACHIE][i].classe == 0 ? "tutte le classi" : AchievementsList[CLASS_ACHIE][i].classe == -1 ? "non abilitato" : pc_class_types[AchievementsList[CLASS_ACHIE][i].classe]);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }

    for(i = 0; i < MAX_QUEST_ACHIE; i++)
    {
        if(AchievementsList[QUEST_ACHIE][i].grado_diff == 0)
        {
            num += 1;
            boost::format fmt("$c0009%6d $c0013%-12s %6d %-304s %s\n\r");
            fmt % num % "Quest:" % AchievementsList[QUEST_ACHIE][i].achie_number % AchievementsList[QUEST_ACHIE][i].achie_string2 % (AchievementsList[QUEST_ACHIE][i].classe == 0 ? "tutte le classi" : AchievementsList[QUEST_ACHIE][i].classe == -1 ? "non abilitato" : pc_class_types[AchievementsList[QUEST_ACHIE][i].classe]);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }

    for(i = 0; i < MAX_OTHER_ACHIE; i++)
    {
        if(AchievementsList[OTHER_ACHIE][i].grado_diff == 0)
        {
            num += 1;
            boost::format fmt("$c0009%6d $c0014%-12s %6d %-34s %s\n\r");
            fmt % num % "Other:" % AchievementsList[OTHER_ACHIE][i].achie_number % AchievementsList[OTHER_ACHIE][i].achie_string2 % (AchievementsList[OTHER_ACHIE][i].classe == 0 ? "tutte le classi" : AchievementsList[OTHER_ACHIE][i].classe == -1 ? "non abilitato" : pc_class_types[AchievementsList[OTHER_ACHIE][i].classe]);
            sb.append(fmt.str().c_str());
            fmt.clear();
        }
    }
    page_string(ch->desc, sb.c_str(), true);
}

ACTION_FUNC(do_achievements)
{
    int i;
    std::array<char, 128> arg1{};
    std::array<char, 128> arg2{};
    std::string sb;

    if(!IS_PC(ch)) {
        return;
    }

    arg = one_argument(arg, arg1.data());
    const auto isCommand = [&](const char* cmd) {
        return std::strcmp(arg1.data(), cmd) == 0;
    };
    const auto resolveVisiblePlayerTarget = [&](char_data*& target, const char* name, const char* notFoundMsg, const char* notPcMsg) {
        target = get_char_vis_world(ch, name, nullptr);
        if(target == nullptr)
        {
            send_to_char(notFoundMsg, ch);
            return false;
        }

        if(!IS_PC(target))
        {
            send_to_char(notPcMsg, ch);
            return false;
        }

        if(IS_POLY(target))
        {
            target = target->desc->original;
        }

        return true;
    };

    if(arg1[0] != '\0')
    {
        if(isCommand("all"))
        {
            char_data* tch = ch;

            if(IS_QUESTMASTER(ch))
            {
                arg = one_argument(arg, arg2.data());

                if(arg2[0] != '\0')
                {
                    tch = get_char_vis_world(ch, arg2.data(), nullptr);
                    if(tch == nullptr)
                    {
                        send_to_char("Non c'e' nessuno con quel nome qui.\n\r", ch);
                        return;
                    }
                }
            }

            if(!IS_PC(tch))
            {
                send_to_char("Vuoi veramente vedere gli achievements di uno stupido mob?\n\r", ch);
                return;
            }

            if(IS_POLY(tch))
            {
                tch = tch->desc->original;
            }

            if(IS_SET(tch->specials.act, PLR_ACHIE) && (hasAchievement(tch, RACESLAYER_ACHIE, 1) || hasAchievement(tch, QUEST_ACHIE, 1) || hasAchievement(tch, OTHER_ACHIE, 1) || hasAchievement(tch, BOSSKILL_ACHIE, 1) || hasAchievement(tch, CLASS_ACHIE, 1)))
            {
                int num = 0;

                if(ch == tch)
                {
                    send_to_char("\n\r$c0011Ecco tutti i tuoi achievements:\n\r", ch);
                }
                else
                {
                    act("\n\r$c0011Lista completa degli achievements di $c0015$N$c0011:", false, ch, nullptr, tch, TO_CHAR);
                }

                // Race Achievements
                if(hasAchievement(tch, RACESLAYER_ACHIE, 1) || IS_QUESTMASTER(ch))
                {
                    sb.append("$c0009");
                    sb.append(45u,'-').append(" $c0015Race  Achievements$c0009 ");
                    sb.append(45u,'-').append("\n\r");
                }
                for(i = 0; i < MAX_RACE_ACHIE; i++)
                {
                    if(AchievementsList[RACESLAYER_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[RACESLAYER_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, AchievementsList[RACESLAYER_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli - 1;
                    }
                    else if(tch->specials.achievements[RACESLAYER_ACHIE][i] > 0)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, AchievementsList[RACESLAYER_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli - 1;
                    }
                }

                // Boss Achievements
                if(hasAchievement(tch, BOSSKILL_ACHIE, 1) || IS_QUESTMASTER(ch))
                {
                    sb.append("$c0009");
                    sb.append(45u,'-').append(" $c0015Boss  Achievements$c0009 ");
                    sb.append(45u,'-').append("\n\r");
                }
                for(i = 0; i < MAX_BOSS_ACHIE; i++)
                {
                    if(AchievementsList[BOSSKILL_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[BOSSKILL_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, AchievementsList[BOSSKILL_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli - 1;
                    }
                    else if(tch->specials.achievements[BOSSKILL_ACHIE][i] > 0)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, AchievementsList[BOSSKILL_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli - 1;
                    }
                }

                // Class Skill Achievements
                if(hasAchievement(tch, CLASS_ACHIE, 1) || IS_QUESTMASTER(ch))
                {
                    sb.append("$c0009");
                    sb.append(45u,'-').append(" $c0015Skill Achievements$c0009 ");
                    sb.append(45u,'-').append("\n\r");
                }
                for(i = 1; i < MAX_CLASS_ACHIE; i++)
                {
                    if(AchievementsList[CLASS_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[CLASS_ACHIE][i].classe) || AchievementsList[CLASS_ACHIE][i].classe == 0)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, AchievementsList[CLASS_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[CLASS_ACHIE][i].n_livelli - 1;
                    }
                }

                // Quest Achievements
                if(hasAchievement(tch, QUEST_ACHIE, 1) || IS_QUESTMASTER(ch))
                {
                    sb.append("$c0009");
                    sb.append(45u,'-').append(" $c0015Quest Achievements$c0009 ");
                    sb.append(45u,'-').append("\n\r");
                }
                for(i = 0; i < MAX_QUEST_ACHIE; i++)
                {
                    if(AchievementsList[QUEST_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[QUEST_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, AchievementsList[QUEST_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[QUEST_ACHIE][i].n_livelli - 1;
                    }
                    else if(tch->specials.achievements[QUEST_ACHIE][i] > 0)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, AchievementsList[QUEST_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[QUEST_ACHIE][i].n_livelli - 1;
                    }
                }

                // Various Achievements
                if(hasAchievement(tch, OTHER_ACHIE, 1) || IS_QUESTMASTER(ch))
                {
                    sb.append("$c0009");
                    sb.append(45u,'-').append(" $c0015Other Achievements$c0009 ");
                    sb.append(45u,'-').append("\n\r");
                }
                for(i = 0; i < MAX_OTHER_ACHIE; i++)
                {
                    if(AchievementsList[OTHER_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[OTHER_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, AchievementsList[OTHER_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[OTHER_ACHIE][i].n_livelli - 1;
                    }
                    else if(tch->specials.achievements[OTHER_ACHIE][i] > 0)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, AchievementsList[OTHER_ACHIE][i].n_livelli, num, true, 0));
                        num += AchievementsList[OTHER_ACHIE][i].n_livelli - 1;
                    }
                }

                page_string(ch->desc, sb.c_str(), true);
            }
            else if(ch == tch)
            {
                send_to_char("Non hai completato nessun achievement.\n\r", ch);
            }
            else
            {
                act("$N non ha completato nessun achievement.", false, ch, nullptr, tch, TO_CHAR);
            }
        }
        else if(isCommand("spam"))
        {
            char_data* tch = ch;

            static const std::array<const char*, 20> rand_spamAchie = {{
                "$c0011$n$c0007 e' proprio $u sborone:",
                "$c0011$n$c0007 si gigioneggia mostrandoti il suo achievement:",
                "$c0011$n$c0007 ti dice: 'Si ho fatto anche questo!',",
                "$c0011$n$c0007 mostra a tutti il cartello 'W i criceti' e poi dice:",
                "$c0011$n$c0007 canticchia allegr$b 'In the name of my new Achievemeeent!':",
                "$c0011$n$c0007 sogghigna mentre ti mostra che",
                "Inizi a schiumare di rabbia appena $c0011$n$c0007 ti dice che",
                "Ma davero davero? Nuovo achievement di $c0011$n$c0007? WTF,",
                "$c0011$n$c0007 si muove come $u supercafone mentre esclama che",
                "Mentre tu stai a rosica' $c0011$n$c0007 urla 'Daje!',",
                "Fai lo sborone e mostri a tutti che",
                "Ti gigioneggi mentre fai vedere il tuo achievement:",
                "Dici: 'E anche questo l'ho fatto,",
                "Mostri il cartello 'W i criceti', poi dici che",
                "Canti allegramente 'In the name of my new Achievemeeeent!':",
                "Sogghigni mentre mostri a tutti che",
                "Fai schiumare i presenti dalla rabbia appena esclami che",
                "Fai rimanere tutti a bocca aperta:",
                "Con il passo da $c5011K$c0007oatto esclami che",
                "Esclami 'Daje!' e fai rosicare tutti urlando che"
            }};

            constexpr std::size_t spamVariantsPerTarget = rand_spamAchie.size() / 2;
            constexpr int nMaxSpamAchie = static_cast<int>(spamVariantsPerTarget - 1);

            arg = one_argument(arg, arg2.data());

            if(arg2[0] == '\0')
            {
                send_to_char("Quale achievement vuoi mostrare? Digita 'achievements all' per vedere il numero.\n\r", ch);
                return;
            }

            const int n_spam = std::atoi(arg2.data());

            if(IS_POLY(tch))
            {
                tch = tch->desc->original;
            }

            if(IS_SET(tch->specials.act, PLR_ACHIE))
            {
                int num = 0;
                bool trovato = false;

                // Race Achievements
                for(i = 0; i < MAX_RACE_ACHIE; i++)
                {
                    if(AchievementsList[RACESLAYER_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[RACESLAYER_ACHIE][i].classe) || IS_QUESTMASTER(tch))
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[RACESLAYER_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, AchievementsList[RACESLAYER_ACHIE][i].n_livelli, num, true, n_spam));
                            num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli;
                        }
                    }
                    else if(ch->specials.achievements[RACESLAYER_ACHIE][i] > 0)
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[RACESLAYER_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, AchievementsList[RACESLAYER_ACHIE][i].n_livelli, num, true, n_spam));
                            num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli;
                        }
                    }
                }

                // Boss Achievements
                for(i = 0; i < MAX_BOSS_ACHIE; i++)
                {
                    if(AchievementsList[BOSSKILL_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[BOSSKILL_ACHIE][i].classe) || IS_QUESTMASTER(tch))
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[BOSSKILL_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, AchievementsList[BOSSKILL_ACHIE][i].n_livelli, num, true, n_spam));
                            num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli;
                        }
                    }
                    else if(ch->specials.achievements[BOSSKILL_ACHIE][i] > 0)
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[BOSSKILL_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, AchievementsList[BOSSKILL_ACHIE][i].n_livelli, num, true, n_spam));
                            num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli;
                        }
                    }
                }

                // Class Skill Achievements
                for(i = 1; i < MAX_CLASS_ACHIE; i++)
                {
                    if(AchievementsList[CLASS_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[CLASS_ACHIE][i].classe) || AchievementsList[CLASS_ACHIE][i].classe == 0)
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[CLASS_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb = bufferAchie(tch, i, CLASS_ACHIE, AchievementsList[CLASS_ACHIE][i].n_livelli, num, true, n_spam);
                            num += AchievementsList[CLASS_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[CLASS_ACHIE][i].n_livelli;
                        }
                    }
                }

                // Quest Achievements
                for(i = 0; i < MAX_QUEST_ACHIE; i++)
                {
                    if(AchievementsList[QUEST_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[QUEST_ACHIE][i].classe) || IS_QUESTMASTER(tch))
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[QUEST_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb.append(bufferAchie(tch, i, QUEST_ACHIE, AchievementsList[QUEST_ACHIE][i].n_livelli, num, true, n_spam));
                            num += AchievementsList[QUEST_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[QUEST_ACHIE][i].n_livelli;
                        }
                    }
                    else if(ch->specials.achievements[QUEST_ACHIE][i] > 0)
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[QUEST_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb.append(bufferAchie(tch, i, QUEST_ACHIE, AchievementsList[QUEST_ACHIE][i].n_livelli, num, true, n_spam));
                            num += AchievementsList[QUEST_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[QUEST_ACHIE][i].n_livelli;
                        }
                    }
                }

                // Various Achievements
                for(i = 0; i < MAX_OTHER_ACHIE; i++)
                {
                    if(AchievementsList[OTHER_ACHIE][i].classe == -1)
                    {
                        // se l'achievement classe e' -1 viene skippato
                        continue;
                    }
                    else if(HasClass(tch, AchievementsList[OTHER_ACHIE][i].classe) || IS_QUESTMASTER(tch))
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[OTHER_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb.append(bufferAchie(tch, i, OTHER_ACHIE, AchievementsList[OTHER_ACHIE][i].n_livelli, num, true, n_spam));
                            num += AchievementsList[OTHER_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[OTHER_ACHIE][i].n_livelli;
                        }
                    }
                    else if(ch->specials.achievements[OTHER_ACHIE][i] > 0)
                    {
                        if(n_spam > num && n_spam <= (num + AchievementsList[OTHER_ACHIE][i].n_livelli))
                        {
                            num += 1;
                            sb.append(bufferAchie(tch, i, OTHER_ACHIE, AchievementsList[OTHER_ACHIE][i].n_livelli, num, true, n_spam));
                            num += AchievementsList[OTHER_ACHIE][i].n_livelli - 1;
                            trovato = true;
                        }
                        else
                        {
                            num += AchievementsList[OTHER_ACHIE][i].n_livelli;
                        }
                    }
                }

                if(!trovato)
                {
                    send_to_char("Quell'achievement esiste solo nella tua fantasia...\n\r", ch);
                    return;
                }

                const int spamIndex = number(0, nMaxSpamAchie);
                const std::size_t spamIndexSz = static_cast<std::size_t>(spamIndex);
                constexpr std::size_t selfSpamOffset = spamVariantsPerTarget;
                constexpr char completedPrefix[] = "com";
                constexpr std::size_t completedPrefixLen = sizeof(completedPrefix) - 1;
                const bool completedVerb = (sb.compare(0, completedPrefixLen, completedPrefix) == 0);
                const char* const toCharAux = completedVerb ? "hai" : "ti";
                const char* const toRoomAux = completedVerb ? "ha" : "$d";
                const auto emitSpamLine = [&](const char* prefix, const char* aux, int target) {
                    std::string spamLine = prefix;
                    spamLine += ' ';
                    spamLine += aux;
                    spamLine += ' ';
                    spamLine += sb;
                    spamLine += '.';
                    act(spamLine.c_str(), false, ch, nullptr, nullptr, target);
                };

                emitSpamLine(rand_spamAchie[spamIndexSz + selfSpamOffset], toCharAux, TO_CHAR);
                emitSpamLine(rand_spamAchie[spamIndexSz], toRoomAux, TO_ROOM);
            }
            else
            {
                send_to_char("Ma se non hai completato nessun achievement...\n\r", ch);
            }
        }
        else if(isCommand("delete") && IS_QUESTMASTER(ch))
        {
            char_data* tch = nullptr;
            std::array<char, 128> arg3{};
            std::array<char, MAX_STRING_LENGTH> buf1{};
            std::array<char, MAX_STRING_LENGTH> buf2{};
            bool trovato = false;

            arg = one_argument(arg, arg2.data());
            arg = one_argument(arg, arg3.data());

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui, a chi vorresti cancellare gli achievements?\n\r",
                    "Vuoi veramente cancellare gli achivements ad uno stupido mob?\n\r"))
                {
                    return;
                }

                if(arg3[0] != '\0')
                {
                    const int achi_d = std::atoi(arg3.data());

                    if(achi_d <= 0)
                    {
                        act("Quale achievement di $N vuoi cancellare? Il numero lo vedi con $c0015achievement all $N$c0007.", false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }

                    if(IS_SET(tch->specials.act, PLR_ACHIE))
                    {
                        int num = 0;

                        // Race Achievements
                        for(i = 0; i < MAX_RACE_ACHIE; i++)
                        {
                            if(AchievementsList[RACESLAYER_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[RACESLAYER_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                            {
                                if(achi_d > num && achi_d <= (num + AchievementsList[RACESLAYER_ACHIE][i].n_livelli))
                                {
                                    if(tch->specials.achievements[RACESLAYER_ACHIE][i] == 0)
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "L'achievement del numero di %s di $N e' gia' 0.", AchievementsList[RACESLAYER_ACHIE][i].achie_string2);
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[RACESLAYER_ACHIE][i] = 0;
                                        mudlog(LOG_PLAYERS, "%s deleted the achievement related on '%s' on %s", GET_NAME(ch), AchievementsList[RACESLAYER_ACHIE][i].achie_string2, GET_NAME(tch));
                                        std::snprintf(buf1.data(), buf1.size(), "Hai cancellato l'achievement del numero di %s di $N.", AchievementsList[RACESLAYER_ACHIE][i].achie_string2);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha cancellato l'achievement del numero di %s.", AchievementsList[RACESLAYER_ACHIE][i].achie_string2);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        // Boss Achievements
                        for(i = 0; i < MAX_BOSS_ACHIE; i++)
                        {
                            if(trovato)
                            {
                                break;
                            }

                            if(AchievementsList[BOSSKILL_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[BOSSKILL_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                            {
                                if(achi_d > num && achi_d <= (num + AchievementsList[BOSSKILL_ACHIE][i].n_livelli))
                                {
                                    if(tch->specials.achievements[BOSSKILL_ACHIE][i] == 0)
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "L'achievement del numero di %s di $N e' gia' 0.", AchievementsList[BOSSKILL_ACHIE][i].achie_string2);
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[BOSSKILL_ACHIE][i] = 0;
                                        mudlog(LOG_PLAYERS, "%s deleted the achievement related on '%s' on %s", GET_NAME(ch), AchievementsList[BOSSKILL_ACHIE][i].achie_string2, GET_NAME(tch));
                                        std::snprintf(buf1.data(), buf1.size(), "Hai cancellato l'achievement del numero di %s di $N.", AchievementsList[BOSSKILL_ACHIE][i].achie_string2);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha cancellato l'achievement del numero di %s.", AchievementsList[BOSSKILL_ACHIE][i].achie_string2);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        // Class Skill Achievements
                        for(i = 1; i < MAX_CLASS_ACHIE; i++)
                        {
                            if(trovato)
                            {
                                break;
                            }

                            if(AchievementsList[CLASS_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[CLASS_ACHIE][i].classe) || AchievementsList[CLASS_ACHIE][i].classe == 0)
                            {
                                if(achi_d > num && achi_d <= (num + AchievementsList[CLASS_ACHIE][i].n_livelli))
                                {
                                    if(tch->specials.achievements[CLASS_ACHIE][i] == 0)
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "L'achievement del numero di %s di $N e' gia' 0.", AchievementsList[CLASS_ACHIE][i].achie_string2);
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[CLASS_ACHIE][i] = 0;
                                        mudlog(LOG_PLAYERS, "%s deleted the achievement related on '%s' on %s", GET_NAME(ch), AchievementsList[CLASS_ACHIE][i].achie_string2, GET_NAME(tch));
                                        std::snprintf(buf1.data(), buf1.size(), "Hai cancellato l'achievement del numero di %s di $N.", AchievementsList[CLASS_ACHIE][i].achie_string2);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha cancellato l'achievement del numero di %s.", AchievementsList[CLASS_ACHIE][i].achie_string2);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[CLASS_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        // Quest Achievements
                        for(i = 0; i < MAX_QUEST_ACHIE; i++)
                        {
                            if(trovato)
                            {
                                break;
                            }

                            if(AchievementsList[QUEST_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[QUEST_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                            {
                                if(achi_d > num && achi_d <= (num + AchievementsList[QUEST_ACHIE][i].n_livelli))
                                {
                                    if(tch->specials.achievements[QUEST_ACHIE][i] == 0)
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "L'achievement del numero di %s di $N e' gia' 0.", AchievementsList[QUEST_ACHIE][i].achie_string2);
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[QUEST_ACHIE][i] = 0;
                                        mudlog(LOG_PLAYERS, "%s deleted the achievement related on '%s' on %s", GET_NAME(ch), AchievementsList[QUEST_ACHIE][i].achie_string2, GET_NAME(tch));
                                        std::snprintf(buf1.data(), buf1.size(), "Hai cancellato l'achievement del numero di %s di $N.", AchievementsList[QUEST_ACHIE][i].achie_string2);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha cancellato l'achievement del numero di %s.", AchievementsList[QUEST_ACHIE][i].achie_string2);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[QUEST_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        // Various Achievements
                        for(i = 0; i < MAX_OTHER_ACHIE; i++)
                        {
                            if(trovato)
                            {
                                break;
                            }

                            if(AchievementsList[OTHER_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[OTHER_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                            {
                                if(achi_d > num && achi_d <= (num + AchievementsList[OTHER_ACHIE][i].n_livelli))
                                {
                                    if(tch->specials.achievements[OTHER_ACHIE][i] == 0)
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "L'achievement del numero di %s di $N e' gia' 0.", AchievementsList[OTHER_ACHIE][i].achie_string2);
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[OTHER_ACHIE][i] = 0;
                                        mudlog(LOG_PLAYERS, "%s deleted the achievement related on '%s' on %s", GET_NAME(ch), AchievementsList[OTHER_ACHIE][i].achie_string2, GET_NAME(tch));
                                        std::snprintf(buf1.data(), buf1.size(), "Hai cancellato l'achievement del numero di %s di $N.", AchievementsList[OTHER_ACHIE][i].achie_string2);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha cancellato l'achievement del numero di %s.", AchievementsList[OTHER_ACHIE][i].achie_string2);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[OTHER_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                        tch = get_char_vis_world(ch, arg2.data(), nullptr);
                        act(buf2.data(), false, ch, nullptr, tch, TO_VICT);
                    }
                    else
                    {
                        act("$N non ha completato nessun achievement!", false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }
                }
                else
                {
                    act("Quale achievement di $N vuoi cancellare? Il numero lo vedi con $c0015achievement all $N$c0007.", false, ch, nullptr, tch, TO_CHAR);
                    return;
                }
            }
            else
            {
                send_to_char("A chi vorresti cancellare l'achievement?\n\r", ch);
                return;
            }
        }
        else if(isCommand("set") && IS_MAESTRO_DEL_CREATO(ch))
        {
            char_data* tch = nullptr;
            std::array<char, 128> arg3{};
            std::array<char, 128> arg4{};
            std::array<char, MAX_STRING_LENGTH> buf1{};
            std::array<char, MAX_STRING_LENGTH> buf2{};
            bool trovato = false;

            arg = one_argument(arg, arg2.data());
            arg = one_argument(arg, arg3.data());
            arg = one_argument(arg, arg4.data());

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui!\n\r",
                    "Vuoi veramente assegnare un achivement ad uno stupido mob?\n\r"))
                {
                    return;
                }

                if(arg3[0] != '\0')
                {
                    const int achi_n = std::atoi(arg3.data());

                    if(achi_n <= 0)
                    {
                        act("A quale achievement di $N vuoi assegnare un valore? Il numero lo vedi con $c0015achievement all $N$c0007.", false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }

                    if(achi_n > maxAchievements(tch))
                    {
                        std::snprintf(buf1.data(), buf1.size(), "Il numero massimo di achievements per $N e' %d.", maxAchievements(tch));
                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }

                    if(arg4[0] != '\0')
                    {
                        int num = 0;
                        const int achi_v = std::atoi(arg4.data());

                        if(achi_v <= 0)
                        {
                            act("Puoi assegnare solo un valore maggiore di zero.", false, ch, nullptr, tch, TO_CHAR);
                            return;
                        }

                        if(!IS_SET(tch->specials.act, PLR_ACHIE))
                        {
                            SET_BIT(tch->specials.act, PLR_ACHIE);
                        }

                        // Race Achievements
                        for(i = 0; i < MAX_RACE_ACHIE; i++)
                        {
                            if(AchievementsList[RACESLAYER_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[RACESLAYER_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                            {
                                if(achi_n > num && achi_n <= (num + AchievementsList[RACESLAYER_ACHIE][i].n_livelli))
                                {
                                    if(achi_v > MaxValueAchievement(RACESLAYER_ACHIE, i, AchievementsList[RACESLAYER_ACHIE][i].n_livelli))
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "Il numero massimo di %s assegnabili e' %d.", AchievementsList[RACESLAYER_ACHIE][i].achie_string2, MaxValueAchievement(RACESLAYER_ACHIE, i, AchievementsList[RACESLAYER_ACHIE][i].n_livelli));
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[RACESLAYER_ACHIE][i] = achi_v;
                                        mudlog(LOG_PLAYERS, "%s set the achievement related on '%s' on %s to %d", GET_NAME(ch), AchievementsList[RACESLAYER_ACHIE][i].achie_string2, GET_NAME(tch), achi_v);
                                        std::snprintf(buf1.data(), buf1.size(), "Hai assegnato all'achievement del numero di %s di $N un valore pari a %d.", AchievementsList[RACESLAYER_ACHIE][i].achie_string2, achi_v);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha assegnato all'achievement del numero di %s un valore pari a %d.", AchievementsList[RACESLAYER_ACHIE][i].achie_string2, achi_v);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[RACESLAYER_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        // Boss Achievements
                        for(i = 0; i < MAX_BOSS_ACHIE; i++)
                        {
                            if(trovato)
                            {
                                break;
                            }

                            if(AchievementsList[BOSSKILL_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[BOSSKILL_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                            {
                                if(achi_n > num && achi_n <= (num + AchievementsList[BOSSKILL_ACHIE][i].n_livelli))
                                {
                                    if(achi_v > MaxValueAchievement(BOSSKILL_ACHIE, i, AchievementsList[BOSSKILL_ACHIE][i].n_livelli))
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "Il numero massimo di %s assegnabili e' %d.", AchievementsList[BOSSKILL_ACHIE][i].achie_string2, MaxValueAchievement(BOSSKILL_ACHIE, i, AchievementsList[BOSSKILL_ACHIE][i].n_livelli));
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[BOSSKILL_ACHIE][i] = achi_v;
                                        mudlog(LOG_PLAYERS, "%s set the achievement related on '%s' on %s to %d", GET_NAME(ch), AchievementsList[BOSSKILL_ACHIE][i].achie_string2, GET_NAME(tch), achi_v);
                                        std::snprintf(buf1.data(), buf1.size(), "Hai assegnato all'achievement del numero di %s di $N un valore pari a %d.", AchievementsList[BOSSKILL_ACHIE][i].achie_string2, achi_v);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha assegnato all'achievement del numero di %s un valore pari a %d.", AchievementsList[BOSSKILL_ACHIE][i].achie_string2, achi_v);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[BOSSKILL_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        // Class Skill Achievements
                        for(i = 1; i < MAX_CLASS_ACHIE; i++)
                        {
                            if(trovato)
                            {
                                break;
                            }

                            if(AchievementsList[CLASS_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[CLASS_ACHIE][i].classe) || AchievementsList[CLASS_ACHIE][i].classe == 0)
                            {
                                if(achi_n > num && achi_n <= (num + AchievementsList[CLASS_ACHIE][i].n_livelli))
                                {
                                    if(achi_v > MaxValueAchievement(CLASS_ACHIE, i, AchievementsList[CLASS_ACHIE][i].n_livelli))
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "Il numero massimo di %s assegnabili e' %d.", AchievementsList[CLASS_ACHIE][i].achie_string2, MaxValueAchievement(CLASS_ACHIE, i, AchievementsList[CLASS_ACHIE][i].n_livelli));
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[CLASS_ACHIE][i] = achi_v;
                                        mudlog(LOG_PLAYERS, "%s set the achievement related on '%s' on %s to %d", GET_NAME(ch), AchievementsList[CLASS_ACHIE][i].achie_string2, GET_NAME(tch), achi_v);
                                        std::snprintf(buf1.data(), buf1.size(), "Hai assegnato all'achievement del numero di %s di $N un valore pari a %d.", AchievementsList[CLASS_ACHIE][i].achie_string2, achi_v);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha assegnato all'achievement del numero di %s un valore pari a %d.", AchievementsList[CLASS_ACHIE][i].achie_string2, achi_v);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[CLASS_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        // Quest Achievements
                        for(i = 0; i < MAX_QUEST_ACHIE; i++)
                        {
                            if(trovato)
                            {
                                break;
                            }

                            if(AchievementsList[QUEST_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[QUEST_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                            {
                                if(achi_n > num && achi_n <= (num + AchievementsList[QUEST_ACHIE][i].n_livelli))
                                {
                                    if(achi_v > MaxValueAchievement(QUEST_ACHIE, i, AchievementsList[QUEST_ACHIE][i].n_livelli))
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "Il numero massimo di %s assegnabili e' %d.", AchievementsList[QUEST_ACHIE][i].achie_string2, MaxValueAchievement(QUEST_ACHIE, i, AchievementsList[QUEST_ACHIE][i].n_livelli));
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[QUEST_ACHIE][i] = achi_v;
                                        mudlog(LOG_PLAYERS, "%s set the achievement related on '%s' on %s to %d", GET_NAME(ch), AchievementsList[QUEST_ACHIE][i].achie_string2, GET_NAME(tch), achi_v);
                                        std::snprintf(buf1.data(), buf1.size(), "Hai assegnato all'achievement del numero di %s di $N un valore pari a %d.", AchievementsList[QUEST_ACHIE][i].achie_string2, achi_v);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha assegnato all'achievement del numero di %s un valore pari a %d.", AchievementsList[QUEST_ACHIE][i].achie_string2, achi_v);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[QUEST_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        // Various Achievements
                        for(i = 0; i < MAX_OTHER_ACHIE; i++)
                        {
                            if(trovato)
                            {
                                break;
                            }

                            if(AchievementsList[OTHER_ACHIE][i].classe == -1)
                            {
                                // se l'achievement classe e' -1 viene skippato
                                continue;
                            }
                            else if(HasClass(tch, AchievementsList[OTHER_ACHIE][i].classe) || IS_QUESTMASTER(ch))
                            {
                                if(achi_n > num && achi_n <= (num + AchievementsList[OTHER_ACHIE][i].n_livelli))
                                {
                                    if(achi_v > MaxValueAchievement(OTHER_ACHIE, i, AchievementsList[OTHER_ACHIE][i].n_livelli))
                                    {
                                        std::snprintf(buf1.data(), buf1.size(), "Il numero massimo di %s assegnabili e' %d.", AchievementsList[OTHER_ACHIE][i].achie_string2, MaxValueAchievement(OTHER_ACHIE, i, AchievementsList[OTHER_ACHIE][i].n_livelli));
                                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                                        return;
                                    }
                                    else
                                    {
                                        tch->specials.achievements[OTHER_ACHIE][i] = achi_v;
                                        mudlog(LOG_PLAYERS, "%s set the achievement related on '%s' on %s to %d", GET_NAME(ch), AchievementsList[OTHER_ACHIE][i].achie_string2, GET_NAME(tch), achi_v);
                                        std::snprintf(buf1.data(), buf1.size(), "Hai assegnato all'achievement del numero di %s di $N un valore pari a %d.", AchievementsList[OTHER_ACHIE][i].achie_string2, achi_v);
                                        std::snprintf(buf2.data(), buf2.size(), "$n ti ha assegnato all'achievement del numero di %s un valore pari a %d.", AchievementsList[OTHER_ACHIE][i].achie_string2, achi_v);
                                        do_save(tch, "", 0);
                                        trovato = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    num += AchievementsList[OTHER_ACHIE][i].n_livelli;
                                }
                            }
                        }

                        act(buf1.data(), false, ch, nullptr, tch, TO_CHAR);
                        tch = get_char_vis_world(ch, arg2.data(), nullptr);
                        act(buf2.data(), false, ch, nullptr, tch, TO_VICT);
                    }
                    else
                    {
                        act("Quale valore vuoi assegnare all'achievement di $N?", false, ch, nullptr, tch, TO_CHAR);
                        return;
                    }
                }
                else
                {
                    act("Quale achievement di $N vuoi assegnare? Il numero lo vedi con $c0015achievement all $N$c0007.", false, ch, nullptr, tch, TO_CHAR);
                    return;
                }
            }
            else
            {
                send_to_char("A chi vorresti assegnare l'achievement?\n\r", ch);
                return;
            }
        }
        else if(isCommand("reset") && IS_MAESTRO_DEL_CREATO(ch))
        {
            char_data* tch = nullptr;

            arg = one_argument(arg, arg2.data());

            if(arg2[0] != '\0')
            {
                if(!resolveVisiblePlayerTarget(tch, arg2.data(),
                    "Non c'e' nessuno con quel nome qui, a chi vorresti resettare gli achievements?\n\r",
                    "Vuoi veramente resettare gli achievements di uno stupido mob?\n\r"))
                {
                    return;
                }

                mudlog(LOG_PLAYERS, "%s starts to delete all the achievements on %s", GET_NAME(ch), GET_NAME(tch));
                const auto resetAchievementCategory = [&](const int category, const int maxCount) {
                    if(hasAchievement(tch, category, 1))
                    {
                        for(int idx = 0; idx < maxCount; ++idx)
                        {
                            tch->specials.achievements[category][idx] = 0;
                        }
                    }
                };
                struct AchievementCategorySpec
                {
                    int category;
                    int maxCount;
                };
                constexpr std::array<AchievementCategorySpec, 5> achievementCategories = {{
                    {RACESLAYER_ACHIE, MAX_RACE_ACHIE},
                    {BOSSKILL_ACHIE, MAX_BOSS_ACHIE},
                    {CLASS_ACHIE, MAX_CLASS_ACHIE},
                    {QUEST_ACHIE, MAX_QUEST_ACHIE},
                    {OTHER_ACHIE, MAX_OTHER_ACHIE}
                }};
                for(const auto& categorySpec : achievementCategories)
                {
                    resetAchievementCategory(categorySpec.category, categorySpec.maxCount);
                }
                if(IS_SET(tch->specials.act, PLR_ACHIE))
                {
                    REMOVE_BIT(tch->specials.act, PLR_ACHIE);
                }
                mudlog(LOG_PLAYERS, "Done. %s has deleted all the achievements on %s", GET_NAME(ch), GET_NAME(tch));
                do_save(tch, "", 0);

                act("Tutti gli achievements di $N sono stati resettati.", false, ch, nullptr, tch, TO_CHAR);
                tch = get_char_vis_world(ch, arg2.data(), nullptr);
                act("$n ti ha resettato tutti gli achievements.", false, ch, nullptr, tch, TO_VICT);
            }
            else
            {
                send_to_char("A chi vorresti resettare gli achievements?\n\r", ch);
                return;
            }
        }
        else
        {
            send_to_char("$c0009Sintassi:\n\r", ch);
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements$c0007                                 - mostra gli achievements che hai completato\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements$c0007               - mostra gli achievements che hai completato\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements all$c0007                             - mostra tutti i tuoi achievements\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements all $c0007<$c0015nome_pg$c0007>                   - mostra tutti gli achievements del personaggio scelto\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements all$c0007           - mostra tutti i tuoi achievements\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements spam $c0007<$c0015numero$c0007>                   - mostra in stanza l'achievement scelto\n\r", ch);
            }
            else
            {
                send_to_char("$c0015Achievements spam $c0007<$c0015numero$c0007> - mostra in stanza l'achievement scelto\n\r", ch);
            }
            if(IS_QUESTMASTER(ch))
            {
                send_to_char("$c0015Achievements delete $c0007<$c0015nome_pg$c0007> <$c0015numero$c0007>       - cancella un determinato achievement dal personaggio scelto\n\r", ch);
            }
            if(IS_MAESTRO_DEL_CREATO(ch))
            {
                send_to_char("$c0015Achievements set $c0007<$c0015nome_pg$c0007> <$c0015numero$c0007> <$c0015valore$c0007> - assegna al personaggio scelto un valore ad un determinato achievement\n\r", ch);
            }
            if(IS_MAESTRO_DEL_CREATO(ch))
            {
                send_to_char("$c0015Achievements reset $c0007<$c0015nome_pg$c0007>                 - resetta tutti gli achievements del personaggio scelto\n\r", ch);
            }

        }
    }
    else
    {
        char_data* tch = ch;

        if(IS_POLY(tch))
        {
            tch = tch->desc->original;
        }

        if(IS_SET(tch->specials.act, PLR_ACHIE) && (hasAchievement(tch, RACESLAYER_ACHIE, 1) || hasAchievement(tch, QUEST_ACHIE, 1) || hasAchievement(tch, OTHER_ACHIE, 1) || hasAchievement(tch, BOSSKILL_ACHIE, 1) || hasAchievement(tch, CLASS_ACHIE, 1)))
        {
            int num = 0;
            send_to_char("\n\r$c0011Ecco i tuoi achievements:\n\r", ch);

            // Race Achievements
            if(hasAchievement(tch, RACESLAYER_ACHIE, 0))
            {
                sb.append("$c0009");
                sb.append(40u,'-').append(" $c0015Race  Achievements$c0009 ");
                sb.append(40u,'-').append("\n\r");
            }
            for(i = 0; i < MAX_RACE_ACHIE; i++)
            {
                if(tch->specials.achievements[RACESLAYER_ACHIE][i] > 0)
                {
                    if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl10_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 10)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 10, num, false, 0));
                        num += 9;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl9_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 9)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 9, num, false, 0));
                        num += 8;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl8_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 8)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 8, num, false, 0));
                        num += 7;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl7_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 7)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 7, num, false, 0));
                        num += 6;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl6_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 6)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 6, num, false, 0));
                        num += 5;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl5_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 5)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 5, num, false, 0));
                        num += 4;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl4_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 4)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 4, num, false, 0));
                        num += 3;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl3_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 3)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 3, num, false, 0));
                        num += 2;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl2_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 2)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 2, num, false, 0));
                        num += 1;
                    }
                    else if (tch->specials.achievements[RACESLAYER_ACHIE][i] >= AchievementsList[RACESLAYER_ACHIE][i].lvl1_val && AchievementsList[RACESLAYER_ACHIE][i].n_livelli >= 1)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, RACESLAYER_ACHIE, 1, num, false, 0));
                    }
                }
            }

            // Boss Achievements
            if(hasAchievement(tch, BOSSKILL_ACHIE, 0))
            {
                sb.append("$c0009");
                sb.append(40u,'-').append(" $c0015Boss  Achievements$c0009 ");
                sb.append(40u,'-').append("\n\r");
            }
            for(i = 0; i < MAX_BOSS_ACHIE; i++)
            {
                if(tch->specials.achievements[BOSSKILL_ACHIE][i] > 0)
                {
                    if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl10_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 10)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 10, num, false, 0));
                        num += 9;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl9_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 9)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 9, num, false, 0));
                        num += 8;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl8_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 8)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 8, num, false, 0));
                        num += 7;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl7_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 7)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 7, num, false, 0));
                        num += 6;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl6_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 6)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 6, num, false, 0));
                        num += 5;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl5_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 5)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 5, num, false, 0));
                        num += 4;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl4_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 4)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 4, num, false, 0));
                        num += 3;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl3_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 3)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 3, num, false, 0));
                        num += 2;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl2_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 2)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 2, num, false, 0));
                        num += 1;
                    }
                    else if (tch->specials.achievements[BOSSKILL_ACHIE][i] >= AchievementsList[BOSSKILL_ACHIE][i].lvl1_val && AchievementsList[BOSSKILL_ACHIE][i].n_livelli >= 1)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, BOSSKILL_ACHIE, 1, num, false, 0));
                    }
                }
            }

            // Class Skill Achievements
            if(hasAchievement(tch, CLASS_ACHIE, 0))
            {
                sb.append("$c0009");
                sb.append(40u,'-').append(" $c0015Skill Achievements$c0009 ");
                sb.append(40u,'-').append("\n\r");
            }
            for(i = 1; i < MAX_CLASS_ACHIE; i++)
            {
                if(tch->specials.achievements[CLASS_ACHIE][i] > 0)
                {
                    if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl10_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 10)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 10, num, false, 0));
                        num += 9;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl9_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 9)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 9, num, false, 0));
                        num += 8;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl8_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 8)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 8, num, false, 0));
                        num += 7;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl7_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 7)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 7, num, false, 0));
                        num += 6;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl6_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 6)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 6, num, false, 0));
                        num += 5;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl5_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 5)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 5, num, false, 0));
                        num += 4;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl4_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 4)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 4, num, false, 0));
                        num += 3;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl3_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 3)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 3, num, false, 0));
                        num += 2;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl2_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 2)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 2, num, false, 0));
                        num += 1;
                    }
                    else if (tch->specials.achievements[CLASS_ACHIE][i] >= AchievementsList[CLASS_ACHIE][i].lvl1_val && AchievementsList[CLASS_ACHIE][i].n_livelli >= 1)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, CLASS_ACHIE, 1, num, false, 0));
                    }
                }
            }

            // Quest Achievements
            if(hasAchievement(tch, QUEST_ACHIE, 0))
            {
                sb.append("$c0009");
                sb.append(40u,'-').append(" $c0015Quest Achievements$c0009 ");
                sb.append(40u,'-').append("\n\r");
            }
            for(i = 0; i < MAX_QUEST_ACHIE; i++)
            {
                if(tch->specials.achievements[QUEST_ACHIE][i] > 0)
                {
                    if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl10_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 10)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 10, num, false, 0));
                        num += 9;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl9_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 9)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 9, num, false, 0));
                        num += 8;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl8_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 8)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 8, num, false, 0));
                        num += 7;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl7_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 7)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 7, num, false, 0));
                        num += 6;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl6_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 6)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 6, num, false, 0));
                        num += 5;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl5_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 5)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 5, num, false, 0));
                        num += 4;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl4_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 4)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 4, num, false, 0));
                        num += 3;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl3_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 3)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 3, num, false, 0));
                        num += 2;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl2_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 2)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 2, num, false, 0));
                        num += 1;
                    }
                    else if (tch->specials.achievements[QUEST_ACHIE][i] >= AchievementsList[QUEST_ACHIE][i].lvl1_val && AchievementsList[QUEST_ACHIE][i].n_livelli >= 1)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, QUEST_ACHIE, 1, num, false, 0));
                    }
                }
            }

            // Various Achievements
            if(hasAchievement(tch, OTHER_ACHIE, 0))
            {
                sb.append("$c0009");
                sb.append(40u,'-').append(" $c0015Other Achievements$c0009 ");
                sb.append(40u,'-').append("\n\r");
            }
            for(i = 0; i < MAX_OTHER_ACHIE; i++)
            {
                if(tch->specials.achievements[OTHER_ACHIE][i] > 0)
                {
                    if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl10_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 10)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 10, num, false, 0));
                        num += 9;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl9_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 9)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 9, num, false, 0));
                        num += 8;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl8_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 8)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 8, num, false, 0));
                        num += 7;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl7_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 7)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 7, num, false, 0));
                        num += 6;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl6_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 6)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 6, num, false, 0));
                        num += 5;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl5_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 5)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 5, num, false, 0));
                        num += 4;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl4_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 4)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 4, num, false, 0));
                        num += 3;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl3_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 3)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 3, num, false, 0));
                        num += 2;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl2_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 2)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 2, num, false, 0));
                        num += 1;
                    }
                    else if (tch->specials.achievements[OTHER_ACHIE][i] >= AchievementsList[OTHER_ACHIE][i].lvl1_val && AchievementsList[OTHER_ACHIE][i].n_livelli >= 1)
                    {
                        num += 1;
                        sb.append(bufferAchie(tch, i, OTHER_ACHIE, 1, num, false, 0));
                    }
                }
            }

            page_string(ch->desc, sb.c_str(), true);
        }
        else
            send_to_char("Non hai completato nessun achievement.\n\r", ch);
    }
}

ACTION_FUNC(do_score) {
	struct time_info_data playing_time;
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
	std::string ageMsg = "$c0005Sei nat$b nel ";
	ageMsg += weekdays[weekday] + 3;
	ageMsg += "$c0005, ";
	ageMsg += std::to_string(day);
	ageMsg += "^ del ";
	ageMsg += month_name[static_cast<int>(my_birth.month)];
	ageMsg += "$c0005, nell'anno ";
	ageMsg += std::to_string(static_cast<int>(my_birth.year));
	ageMsg += " ";
	ageMsg += (my_birth.year > 0) ? "dopo Nebbie" : "avanti Nebbie";
	ageMsg += ".";
	ageMsg += "\n\rHai $c0015";
	ageMsg += std::to_string(static_cast<int>(my_age.ayear));
	ageMsg += "$c0005 anni.";
	if(my_age.year != my_age.ayear) {
		ageMsg += "\nMa ne dimostri $c0015";
		ageMsg += std::to_string(static_cast<int>(my_age.year));
		ageMsg += "$c0005.";
	}

	if((my_age.month == my_birth.month) && (my_age.day == my_birth.month)) {
		ageMsg += "\n$c0015 Oggi e' il tuo compleanno!!";
	}

	act(ageMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	std::string raceMsg = "$c0005Appartieni alla razza $c0015";
	raceMsg += RaceName[GET_RACE(ch)];
	raceMsg += "$c0005";
	act(raceMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	if(HAS_PRINCE(ch)) {
		std::string princeMsg = "$c0005Appartieni al clan di $c0015";
		princeMsg += GET_PRINCE(ch);
		princeMsg += "$c0005";
		act(princeMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
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

	std::string statsMsg = "$c0005Hai $c0015";
	statsMsg += std::to_string(static_cast<int>(GET_HIT(ch)));
	statsMsg += "$c0005($c0011";
	statsMsg += std::to_string(GET_MAX_HIT(ch));
	statsMsg += "$c0005) hit, $c0015";
	statsMsg += std::to_string(static_cast<int>(GET_MANA(ch)));
	statsMsg += "$c0005($c0011";
	statsMsg += std::to_string(GET_MAX_MANA(ch));
	statsMsg += "$c0005) mana e $c0015";
	statsMsg += std::to_string(static_cast<int>(GET_MOVE(ch)));
	statsMsg += "$c0005($c0011";
	statsMsg += std::to_string(GET_MAX_MOVE(ch));
	statsMsg += "$c0005) movimento.";
	act(statsMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	std::string alignMsg = "$c0005Il tuo allineamento e': $c0015";
	alignMsg += AlignDesc(GET_ALIGNMENT(ch));
	act(alignMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	std::string expMsg = "$c0005Hai effettuato $c0015";
	expMsg += std::to_string(GET_EXP(ch));
	expMsg += "$c0005 exp, ed hai $c0015";
	expMsg += std::to_string(GET_GOLD(ch));
	expMsg += "$c0005 monete d'oro.";
	act(expMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	std::string donateMsg = "$c0005Non puoi donare xp se hai meno di: $c0015";
	donateMsg += std::to_string(IS_PRINCE(ch) ? PRINCEEXP : MIN_EXP(ch));
	donateMsg += "$c0005 exp";
	act(donateMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	/* the mud will crash without this check! */
	if(GetMaxLevel(ch)>MAX_MORT ||
			(IS_NPC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF))) {
		/* do nothing! */
	}
	else {
		std::string nextLevelMsg = "$c0005Exp al prossimo livello: ";
		if(HasClass(ch, CLASS_MAGIC_USER)) {
			nextLevelMsg += "M:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[MAGE_LEVEL_IND]
					[GET_LEVEL(ch, MAGE_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_CLERIC)) {
			nextLevelMsg += "C:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[CLERIC_LEVEL_IND]
					[GET_LEVEL(ch, CLERIC_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_THIEF)) {
			nextLevelMsg += "T:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[THIEF_LEVEL_IND]
					[GET_LEVEL(ch, THIEF_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_WARRIOR)) {
			nextLevelMsg += "W:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[WARRIOR_LEVEL_IND]
					[GET_LEVEL(ch, WARRIOR_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_DRUID)) {
			nextLevelMsg += "D:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[DRUID_LEVEL_IND]
					[GET_LEVEL(ch, DRUID_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}
		if(HasClass(ch, CLASS_MONK)) {
			nextLevelMsg += "K:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[MONK_LEVEL_IND]
					[GET_LEVEL(ch, MONK_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_BARBARIAN)) {
			nextLevelMsg += "B:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[BARBARIAN_LEVEL_IND]
					[GET_LEVEL(ch, BARBARIAN_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_SORCERER)) {
			nextLevelMsg += "S:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[SORCERER_LEVEL_IND]
					[GET_LEVEL(ch, SORCERER_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_PALADIN)) {
			nextLevelMsg += "P:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[PALADIN_LEVEL_IND]
					[GET_LEVEL(ch, PALADIN_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_RANGER)) {
			nextLevelMsg += "R:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[RANGER_LEVEL_IND]
					[GET_LEVEL(ch, RANGER_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		if(HasClass(ch, CLASS_PSI)) {
			nextLevelMsg += "I:";
			if(GetMaxLevel(ch)<MAX_IMMORT) {
				nextLevelMsg += "$c0015";
				nextLevelMsg += std::to_string((titles[PSI_LEVEL_IND]
					[GET_LEVEL(ch, PSI_LEVEL_IND)+1].exp)-GET_EXP(ch));
				nextLevelMsg += "$c0005 ";
			}
			else {
				nextLevelMsg += "0 ";
			}
		}

		act(nextLevelMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	std::string levelsMsg = "$c0005I tuoi livelli:";
	if(HasClass(ch, CLASS_MAGIC_USER)) {
		levelsMsg += " M:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, MAGE_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_CLERIC)) {
		levelsMsg += " C:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, CLERIC_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_WARRIOR)) {
		levelsMsg += " W:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, WARRIOR_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_THIEF)) {
		levelsMsg += " T:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, THIEF_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_DRUID)) {
		levelsMsg += " D:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, DRUID_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_MONK)) {
		levelsMsg += " K:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, MONK_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_BARBARIAN)) {
		levelsMsg += " B:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, BARBARIAN_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_SORCERER)) {
		levelsMsg += " S:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, SORCERER_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_PALADIN)) {
		levelsMsg += " P:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, PALADIN_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_RANGER)) {
		levelsMsg += " R:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, RANGER_LEVEL_IND)));
		levelsMsg += "$c0005";
	}
	if(HasClass(ch, CLASS_PSI)) {
		levelsMsg += " I:$c0015";
		levelsMsg += std::to_string(static_cast<int>(GET_LEVEL(ch, PSI_LEVEL_IND)));
		levelsMsg += "$c0005";
	}

	act(levelsMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	if(GET_TITLE(ch)) {
		std::string titleMsg = "$c0005Questo ti qualifica come $c0015";
		titleMsg += GET_NAME(ch);
		titleMsg += " $c0011";
		titleMsg += GET_TITLE(ch);
		act(titleMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	playing_time = real_time_passed((time(0)-ch->player.time.logon) + ch->player.time.played, 0);
	std::string playingMsg = "$c0005Hai giocato per $c0015";
	playingMsg += std::to_string(static_cast<int>(playing_time.day));
	playingMsg += "$c0005 giorni e $c0015";
	playingMsg += std::to_string(static_cast<int>(playing_time.hours));
	playingMsg += "$c0005 ore.";
	act(playingMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	/* Drow fight -4 in lighted rooms! */
	if(!IS_DARK(ch->in_room) && GET_RACE(ch) == RACE_DARK_ELF &&
			!affected_by_spell(ch,SPELL_GLOBE_DARKNESS) && !IS_UNDERGROUND(ch)) {
		act("$c0011La luce nell'area ti provoca molto dolore$c0009!", FALSE, ch, 0, 0, TO_CHAR);
	}

	std::string settingsMsg = "$c0005I tuoi set sono: "
		"$c0015Achie$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, ACHIE_MODE) ? "on" : "off";
	settingsMsg += "$c0005) $c0015Ansi$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, USE_ANSI) ? "on" : "off";
	settingsMsg += "$c0005) $c0015Autoexits$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, SHOW_EXITS) ? "on" : "off";
	settingsMsg += "$c0005) $c0015Pwp$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, PWP_MODE) ? "on" : "off";
	settingsMsg += "$c0005) $c0015Warnings$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, WARNINGS_MODE_OFF) ? "off" : "on";
	settingsMsg += "$c0005) $c0015Who$c0005($c0015";
	settingsMsg += IS_SET(ch->player.user_flags, SHOW_CLASSES) ? "on" : "off";
	settingsMsg += "$c0005)";
	act(settingsMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	std::string runesMsg = "$c0005Le rune degli Dei tatuate sul tuo corpo sono: $c0015";
	runesMsg += std::to_string(static_cast<int>(GET_RUNEDEI(ch)));
	runesMsg += " ";
	act(runesMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);

	switch(GET_POS(ch)) {
	case POSITION_DEAD :
		act("$c0009Sei mort$b!",FALSE, ch,0,0,TO_CHAR);
		break;
	case POSITION_MORTALLYW :
		act("$c0009Sei ferit$b a morte e dovresti cercare aiuto!", FALSE, ch,
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
			std::string mountedMsg = "$c0005Stai montando su $c0015";
			mountedMsg += MOUNTED(ch)->player.short_descr;
			act(mountedMsg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
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
	int weekday, day;
	std::string buf;

	buf = "Sono le ";
	{
		const int hour12 = static_cast<int>(time_info.hours) % 12;
		buf += std::to_string(hour12 == 0 ? 12 : hour12);
	}
	buf += " del";
	buf += (time_info.hours >= 12) ? " pomeriggio" : "la mattina";
	buf += ", de ";

	weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */

	buf += weekdays[weekday];
	buf += ".\n\r";
	send_to_char(buf.c_str(), ch);

	day = time_info.day + 1;   /* day in [1..35] */

	buf = std::to_string(day);
	buf += "^ giorno del ";
	buf += month_name[static_cast<int>(time_info.month)];
	buf += ", nell'anno ";
	buf += std::to_string(static_cast<int>(time_info.year));
	buf += ".\n\r";

	send_to_char(buf.c_str(), ch);
}


ACTION_FUNC(do_weather) {
	std::string weatherMsg;
	const char* sky_look[] = {
		"$c0014sereno$c0007",
		"$c0015nuvoloso$c0007",
		"$c0012piovoso$c0007",
		"$c0011illuminato dai lampi$c0007"
	};

	if(OUTSIDE(ch)) {
		weatherMsg = "Il cielo e' ";
		weatherMsg += sky_look[weather_info.sky];
		weatherMsg += " e";
		weatherMsg += (weather_info.change >= 0 ?
			" senti un vento $c0009caldo$c0007 da sud" :
			"d i tuoi reumatismi ti dicono che il tempo cambiera'");
		weatherMsg += ".\n\r";
		send_to_char(weatherMsg.c_str(), ch);
	}
	else {
		send_to_char("Non riesci a vedere il tempo che fa fuori.\n\r", ch);
	}
}


ACTION_FUNC(do_help) {

	int chk, bot, top, mid, minlen;
	std::array<char, 80> buf{};


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
				std::string buffer;
				for(;;) {
					fgets(buf.data(), static_cast<int>(buf.size()), help_fl);
					if(buf[0] == '#') {
						break;
					}
					if(buffer.size() + std::strlen(buf.data()) + 1 > MAX_STRING_LENGTH - 2) {
						break;
					}
					buffer += buf.data();
					buffer += "\r";
				}
				page_string(ch->desc, buffer.c_str(), 1);
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
	std::array<char, 1000> queryBuf{};
	int i, j = 1;
	NODE* n;

	if(IS_NPC(ch)) {
		return;
	}

	one_argument(arg, queryBuf.data());                /* new msw */
	if(*arg) {
		/* asking for help on keyword, try looking in file */
		do_actual_wiz_help(ch,arg,cmd);
		return;
	}

	std::string buf = "Wizhelp <keyword>\n\rWizard Commands disponibili per te:\n\r\n\r";

	for(i = 0; i < 27; i++) {
		n = radix_head[i].next;
		while(n) {
			if(n->min_level <= GetMaxLevel(ch) && n->min_level >= IMMORTALE) {
				char formattedName[32];
				snprintf(formattedName, sizeof(formattedName), "%-10s", n->name);
				buf += formattedName;
				if(!(j % 7)) {
					buf += "\n\r";
				}
				j++;
			}
			n = n->next;
		}
	}

	buf += "\n\r";

	page_string(ch->desc, buf.c_str(), 1);
}

ACTION_FUNC(do_actual_wiz_help) {

	int chk, bot, top, mid, minlen;
	std::array<char, 80> buf{};


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
				std::string buffer;
				for(;;) {
					fgets(buf.data(), static_cast<int>(buf.size()), wizhelp_fl);
					if(buf[0] == '#') {
						break;
					}
					if(buffer.size() + std::strlen(buf.data()) + 1 > MAX_STRING_LENGTH - 2) {
						break;
					}
					buffer += buf.data();
					buffer += "\r";
				}
				page_string(ch->desc, buffer.c_str(), 1);
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
	int i, j = 1;
	NODE* n;

	if(IS_NPC(ch)) {
		return;
	}

	std::string buf = "Comandi disponibili per te:\n\r\n\r";

	for(i = 0; i < 27; i++) {
		n = radix_head[i].next;
		while(n) {
			if(n->min_level <= GetMaxLevel(ch)) {
				char formattedName[32];
				snprintf(formattedName, sizeof(formattedName), "%-10s", n->name);
				if(buf.size() + std::strlen(formattedName) <= MAX_STRING_LENGTH) {
					buf += formattedName;
				}
				if(!(j % 7))
					if(buf.size() + 2 <= MAX_STRING_LENGTH) {
						buf += "\n\r";
					}
				j++;
			}
			n = n->next;
		}
	}

	if(buf.size() + 2 <= MAX_STRING_LENGTH) {
		buf += "\n\r";
	}

	page_string(ch->desc, buf.c_str(), 1);
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

namespace {
/* Centra la stringa colorata livello+classi nel campo a 34 caratteri (logica who originale). */
std::string who_center_label34(const std::string& inner) {
	std::string pad(34, ' ');
	const int len = static_cast<int>(inner.size());
	int start = 17 - len / 2;
	if(start < 0) {
		start = 0;
	}
	const int n = std::min(len, 34 - start);
	for(int i = 0; i < n; ++i) {
		pad[static_cast<size_t>(start + i)] = inner[static_cast<size_t>(i)];
	}
	return pad;
}
} // namespace

ACTION_FUNC(do_who) {
	struct char_data* person;
	std::string whoBuf;
	std::array<char, 512> argBuf{};
	int count;
	char flags[20]="";
	char name_mask[40]="";
	char tmpname1[80],tmpname2[80];
	int listed = 0;
	int lcount =0;
	int  skip = FALSE;
	char ttbuf[256];

	/*  check for an arg */
	auto storeWhoToken = [&](const char* token) {
		if(token[0] == '-' && token[1] != '\0') {
			std::snprintf(flags, sizeof(flags), "%s", token + 1);
		}
		else {
			std::snprintf(name_mask, sizeof(name_mask), "%s", token);
		}
	};

	arg = one_argument(arg, argBuf.data());
	storeWhoToken(argBuf.data());
	if(*arg) {
		arg = one_argument(arg, argBuf.data());
		storeWhoToken(argBuf.data());
	}

	const auto hasFlag = [&](char f) {
		return std::strchr(flags, f) != nullptr;
	};

	if((IS_DIO_MINORE(ch) && flags[0] == '\0') || !IS_DIO_MINORE(ch) ||
			cmd == CMD_WHOZONE) {
		if(IS_DIO_MINORE(ch)) {
			whoBuf =
				"$c0005                         Giocatori [God Version -? for Help]\n\r"
				"                       -------------------------------------\n\r";
		}
		else if(cmd == CMD_WHOZONE) {
			whoBuf = "$c0005Giocatori\n\r"
					 "---------\n\r";
		}
		else {
			whoBuf =
				"$c0005                         Nelle nebbie........\n\r"
				"                       ---------------------------\n\r";
		}
		count=0;
		for(person = character_list; person; person = person->next) {
			if(!IS_LINKDEAD(person) and IS_PC(person) and
					OK_NAME(person, name_mask) and
					person->invis_level <= GetMaxLevel(ch) and
					(cmd != CMD_WHOZONE or
					 (real_roomp(person->in_room) and
					  real_roomp(person->in_room)->zone ==
					  real_roomp(ch->in_room)->zone))and
					(!hasFlag('g') or IS_IMMORTAL(person))) {
				if(OK_NAME(person,name_mask)) {
					count++;

					std::string row;
					if(cmd == CMD_WHOZONE) {
						/* it's a whozone command */
						if((!IS_AFFECTED(person, AFF_HIDE)) || (IS_IMMORTAL(ch))) {
							std::ostringstream wz;
							wz << "$c0012" << std::left << std::setw(25)
							   << (GET_NAME_DESC(person) ? GET_NAME_DESC(person) : "")
							   << " - "
							   << (IS_AFFECTED(person, AFF_SNEAK) ?
								   "sneaking around" :
								   real_roomp(person->in_room)->name);
							row = wz.str();
							if(GetMaxLevel(ch) >= IMMORTALE) {
								row += " [";
								row += std::to_string(person->in_room);
								row += "]";
							}
						}
					}
					else {
						std::string levelsStr;
						std::string classesStr;
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
									if(!classesStr.empty()) {
										classesStr += "/";
									}
									if(IS_SET(person->player.user_flags,SHOW_CLASSES)) {
										classesStr += classname[i];
									}
								}
							}
							/*		     total/=classn; */
							/* Modifica Urhar dello who in base alla classe piu' alta e non alla media */
							total= GetMaxLevel(person);
							/* fine modifica */
							if(IS_PRINCE(person) || total == PRINCIPE) {  // SALVO per riconoscere anche il poly
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0008Principe":
									   "$c0008Principessa";
							}
							else if(total<ALLIEVO)  /* 1-10 */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0002Novizio":
									   "$c0002Novizia";
							else if(total<APPRENDISTA) {   /* 11-20 */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0004Allievo":"$c0004Allieva";
							}
							else if(total<INIZIATO)       /* 21-30 */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0006Apprendista":
									   "$c0006Apprendista";
							else if(total<ESPERTO)      /* 31-40 */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0014Iniziato":
									   "$c0014Iniziata";
							else if(total<MAESTRO)      /* 41-45 */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0009Esperto":
									   "$c0009Esperta";
							else if(total<BARONE)       /* 46-49 */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Maestro":
									   "$c0015Maestra";
							else if(GET_EXP(person)<CONTE) /* 50 */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Barone":
									   "$c0015Baronessa";
							else if(GET_EXP(person)<MARCHESE)   /* fino ai 200M */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Conte":
									   "$c0015Contessa";
							else if(GET_EXP(person)<DUCA)       /* 250M */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Marchese":
									   "$c0015Marchesa";
							else if(GET_EXP(person)<GRANDUCA)   /* 300M */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0015Duca":
									   "$c0015Duchessa";
							else if(GET_EXP(person)<PRINCEEXP)   /* 350M */
								levelsStr = GET_SEX(person)==SEX_MALE?"$c0001Granduca":
									   "$c0001Granduchessa";
							const std::string mid = levelsStr + " $c0012" + classesStr;
							const std::string centered = who_center_label34(mid);
							const char* nm = GET_NAME_DESC(person) ? GET_NAME_DESC(person) : "nemo";
							row = centered + "$c0005: $c0007" + std::string(nm) + " ";
							row += person->player.title ? person->player.title : "";
							row += " $c0007";
							row += HAS_PRINCE(person) ? "del clan di " : "";
							row += HAS_PRINCE(person) && GET_PRINCE(person) ? GET_PRINCE(person) : "";

						}
						else {
							if(GET_SEX(person) == SEX_FEMALE) {
								switch(GetMaxLevel(person)) {
								case 52:
									levelsStr = "Immortale";
									break;
								case 53:
									levelsStr = "Dea Minore";
									break;
								case 54:
									levelsStr = "Dea";
									break;
								case 55:
									levelsStr = "Maestra degli Dei";
									break;
								case 56:
									levelsStr = "Creatrice";
									break;
								case 57:
									levelsStr = "Maestra del Fato";
									break;
								case 58:
									levelsStr = "Maestra del Creato";
									break;
								case 59:
									levelsStr = "Maestra dei Creatori";
									break;
								case 60:
									levelsStr = "Immensa";
									break;
								}
							}
							else {
								switch(GetMaxLevel(person)) {
								case 52:
									levelsStr = "Immortale";
									break;
								case 53:
									levelsStr = "Dio Minore";
									break;
								case 54:
									levelsStr = "Dio";
									break;
								case 55:
									levelsStr = "Maestro degli Dei";
									break;
								case 56:
									levelsStr = "Creatore";
									break;
								case 57:
									levelsStr = "Maestro del Fato";
									break;
								case 58:
									levelsStr = "Maestro del Creato";
									break;
								case 59:
									levelsStr = "Maestro dei Creatori";
									break;
								case 60:
									levelsStr = "Immenso";
									break;
								};
							}
							const std::string immMid = std::string("$c0008") + levelsStr + " $c0005";
							const std::string centered = who_center_label34(immMid);
							std::ostringstream nm10;
							nm10 << std::left << std::setw(10)
								 << (GET_NAME_DESC(person) ? GET_NAME_DESC(person) : "");
							row = centered + "$c0005: $c0011" + nm10.str() + " ";
							row += person->player.title ? person->player.title : "";
							row += "$c0007";
						}
					}
					if(!row.empty()) {
						if(IS_AFFECTED2(person,AFF2_AFK)) {
							row += "$c0008 AF";
						}
						if(IS_SET(person->player.user_flags,RACE_WAR)) {
							row += "$c0001 PK";
						}
						row += " $c0007\n\r";
						if(whoBuf.size() + row.size() < (MAX_STRING_LENGTH * 2) - 512) {
							whoBuf += row;
						}
					}
				}
			}
		}
		if(hasFlag('g')) {
			const std::string totalsLine =
				"\n\r$c0005Totale Divinita' visibili: $c0015" + std::to_string(count) + "\n\r";
			if(whoBuf.size() + totalsLine.size() < (MAX_STRING_LENGTH * 2) - 512) {
				whoBuf += totalsLine;
			}
		}
		else {
			const std::string totalsLine =
				"\n\r$c0005Totale giocatori visibili: $c0015" + std::to_string(count) + "\n\r";
			if(whoBuf.size() + totalsLine.size() < (MAX_STRING_LENGTH * 2) - 512) {
				whoBuf += totalsLine;
			}
		}
		const std::string maxLine =
			"\n\r$c0005Max giocatori connessi dall'ultimo reboot: $c0015"
			+ std::to_string(update_max_usage()) + "\r\n";
		if(whoBuf.size() + maxLine.size() < (MAX_STRING_LENGTH * 2) - 512) {
			whoBuf += maxLine;
		}
	}
	else {
		/* GOD WHO */


		whoBuf =
			"$c0005Giocatori [God Version -? for Help]\n\r"
			"---------\n\r";

		count=0;
		lcount=0;
		if(hasFlag('?')) {
			send_to_char(whoBuf.c_str(), ch);
			send_to_char("$c0007[-]i=idle l=levels t=title h=hit/mana/move "
						 "s=stats r=race \n\r",ch);
			send_to_char("[-]d=linkdead e=eq g=God o=Mort a=align "
						 "[1]Mage[2]Cleric[3]War[4]Thief[5]Druid\n\r", ch);
			send_to_char("[-][6]Monk[7]Barb[8]Sorc[9]Paladin[!]Ranger[@]Psi\n\r",
						 ch);
			return;
		}
		const bool filterGods = hasFlag('g');
		const bool filterMortals = hasFlag('o');
		const bool filterMage = hasFlag('1');
		const bool filterCleric = hasFlag('2');
		const bool filterWarrior = hasFlag('3');
		const bool filterThief = hasFlag('4');
		const bool filterDruid = hasFlag('5');
		const bool filterMonk = hasFlag('6');
		const bool filterBarbarian = hasFlag('7');
		const bool filterSorcerer = hasFlag('8');
		const bool filterPaladin = hasFlag('9');
		const bool filterRanger = hasFlag('!');
		const bool filterPsi = hasFlag('@');
		const bool showLinkdead = hasFlag('d');

		for(person = character_list; person; person = person->next) {
			if(!IS_NPC(person) and CAN_SEE(ch, person) and OK_NAME(person,name_mask)) {
				count++;
				if(person->desc == NULL) {
					lcount ++;
				}
				skip = (filterGods && !IS_IMMORTAL(person))
					   || (filterMortals && IS_IMMORTAL(person))
					   || (filterMage && !HasClass(person, CLASS_MAGIC_USER))
					   || (filterCleric && !HasClass(person, CLASS_CLERIC))
					   || (filterWarrior && !HasClass(person, CLASS_WARRIOR))
					   || (filterThief && !HasClass(person, CLASS_THIEF))
					   || (filterDruid && !HasClass(person, CLASS_DRUID))
					   || (filterMonk && !HasClass(person, CLASS_MONK))
					   || (filterBarbarian && !HasClass(person, CLASS_BARBARIAN))
					   || (filterSorcerer && !HasClass(person, CLASS_SORCERER))
					   || (filterPaladin && !HasClass(person, CLASS_PALADIN))
					   || (filterRanger && !HasClass(person, CLASS_RANGER))
					   || (filterPsi && !HasClass(person, CLASS_PSI));

				if(!skip) {
					std::string godLine;
					std::array<char, 512> line0{};
					if(person->desc == NULL) {
						if(showLinkdead) {
							std::snprintf(line0.data(), line0.size(),
										  "$c0003[%-12s] ", GET_NAME(person));
							godLine = line0.data();
							listed++;
						}
					}
					else if(IS_NPC(person) && IS_SET(person->specials.act, ACT_POLYSELF)) {
						std::snprintf(line0.data(), line0.size(), "(%-12s) ", GET_NAME(person));
						godLine = line0.data();
						listed++;
					}
					else {
						std::snprintf(line0.data(), line0.size(), "$c0012%-14s ",
									  GET_NAME(person));
						godLine = line0.data();
						listed++;
					}
					const bool appendDetail =
						(person->desc != NULL) || showLinkdead;
					if(appendDetail) {
						for(const char* fp = flags; *fp != '\0'; ++fp) {
							const char fc = *fp;
							switch(fc) {
							case 'r': {
								/* show race */
								sprinttype((person->race),RaceName,ttbuf);
								godLine += " [";
								godLine += ttbuf;
								godLine += "] ";
								break;
							}
							case 'e': {
								char u[256];
								std::snprintf(u, sizeof(u), "Eq:[%f] ", GetCharBonusIndex(person));
								godLine += u;
								break;
							}
							case 'a': {
								char u[256];
								std::snprintf(u, sizeof(u), "Al:[%d] ", GET_ALIGNMENT(person));
								godLine += u;
								break;
							}
							case 'i': {
								char u[256];
								std::snprintf(u, sizeof(u), "Idle:[%-3d] ", person->specials.timer);
								godLine += u;
								break;
							}
							case 'l': {
								char u[256];
								std::snprintf(u, sizeof(u), "Level:[%-2d/%-2d/%-2d/%-2d/%-2d/"
										 "%-2d/%-2d/%-2d/%-2d/%-2d/%-2d] ",
										 person->player.level[0],person->player.level[1],
										 person->player.level[2],person->player.level[3],
										 person->player.level[4],person->player.level[5],
										 person->player.level[6],person->player.level[7],
										 person->player.level[8],person->player.level[9],
										 person->player.level[10]);
								godLine += u;
								break;
							}
							case 'h': {
								char u[256];
								std::snprintf(u, sizeof(u), "Hit:[%-3d] Mana:[%-3d] Move:[%-3d] ",
										 GET_HIT(person), GET_MANA(person), GET_MOVE(person));
								godLine += u;
								break;
							}
							case 'c':
								if(HAS_PRINCE(person)) {
									char u[256];
									std::snprintf(u, sizeof(u), "del clan di %s",
											 GET_PRINCE(person));
									godLine += u;
								}
								break;
							case 's':
								if(GET_STR(person) != 18) {
									std::snprintf(ttbuf, sizeof(ttbuf), "[S:%-2d I:%-2d W:%-2d "
											 "C:%-2d D:%-2d CH:%-2d] ",
											 GET_STR(person),GET_INT(person),GET_WIS(person),
											 GET_CON(person),GET_DEX(person),GET_CHR(person));
								}
								else {
									std::snprintf(ttbuf, sizeof(ttbuf), "[S:%-2d(%1d) I:%-2d W:%-2d "
											 "C:%-2d D:%-2d CH:%-2d] ",
											 GET_STR(person),GET_ADD(person),GET_INT(person),
											 GET_WIS(person),GET_CON(person),GET_DEX(person),
											 GET_CHR(person));
								}
								godLine += ttbuf;
								break;
							case 't': {
								char u[256];
								std::snprintf(u, sizeof(u), " %-16s ", (person->player.title ?
																person->player.title :
																"(null)"));
								godLine += u;
								break;
							}
							default:
								break;
							}
						}
					}
					if(appendDetail) {
						if(OK_NAME(person,name_mask)) {
							if(whoBuf.size() + godLine.size() < (MAX_STRING_LENGTH * 2) - 512) {
								whoBuf += godLine;
								whoBuf += "\n\r";
							}
						}
					}
				}
			}
		}

		std::string totalsLine;
		if(listed<=0) {
			totalsLine = "\n\r$c0005Nessun personaggio con le caratteristiche richieste.\n\r";
		}
		else {
			char totalsBuf[256];
			std::snprintf(totalsBuf, sizeof(totalsBuf),
					  "\n\r$c0005Totale giocatori / Link dead [%d/%d] (%2.0f%%)\n\r",
					  count, lcount, ((float)lcount / (int)count) * 100);
			totalsLine = totalsBuf;
		}
		if(whoBuf.size() + totalsLine.size() < (MAX_STRING_LENGTH * 2) - 512) {
			whoBuf += totalsLine;
		}
		const std::string maxLine =
			"\n\r$c0005Max giocatori connessi dall'ultimo reboot: $c0015"
			+ std::to_string(update_max_usage()) + "\r\n";
		if(whoBuf.size() + maxLine.size() < (MAX_STRING_LENGTH * 2) - 512) {
			whoBuf += maxLine;
		}
	}
	page_string(ch->desc, whoBuf.c_str(), TRUE);
}

ACTION_FUNC(do_users) {
	std::string buf = "Connessioni:\n\r------------\n\r";

	struct descriptor_data* d;

	for(d = descriptor_list; d; d = d->next) {
		if(CAN_SEE(ch, d->character) || GetMaxLevel(ch)==IMMENSO) {
			std::ostringstream row;
			if(d->character && d->character->player.name) {
				row << std::left << std::setw(16)
					<< (d->original ? d->original->player.name : d->character->player.name)
					<< ": ";
			}
			else {
				row << "NON DEFINITO  : ";
			}

			row << std::left << std::setw(22) << connected_types[d->connected]
				<< " [" << ((*d->host) ? d->host : "????") << "] "
				<< std::right << std::setw(5) << d->wait << "\n\r";
			buf += row.str();
		} /* could not see the person */
	} /* end for */

	/*  send_to_char(buf, ch); */
	page_string(ch->desc, buf.c_str(), 0);
}



ACTION_FUNC(do_inventory) {

	send_to_char("Stai trasportando:\n\r", ch);
	list_obj_in_heap(ch->carrying, ch);
}


ACTION_FUNC(do_equipment) {
	int j,Worn_Index;
	bool found;

	send_to_char("Stai usando:\n\r", ch);
	found = FALSE;
	for(Worn_Index = j=0; j< MAX_WEAR; j++) {
		if(ch->equipment[j]) {
			Worn_Index++;
			std::ostringstream itemLine;
			itemLine << "[" << std::setw(2) << Worn_Index << "] " << eqWhere[j];
			send_to_char(itemLine.str().c_str(), ch);
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
	static std::array<char, MAX_STRING_LENGTH> buf{};
	std::string value;
	if(IS_IMMORTAL(ch)) {
		value = std::to_string(which_number_object(obj));
		value += ".";
		value += fname(obj->name);
	}
	else {
		value = obj->short_description ? obj->short_description : "";
	}
	std::strncpy(buf.data(), value.c_str(), buf.size() - 1);
	buf[buf.size() - 1] = '\0';
	return buf.data();
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
	static std::array<char, MAX_STRING_LENGTH> buf{};
	std::string value;
	if(IS_NPC(person) && IS_IMMORTAL(ch)) {
		value = std::to_string(which_number_mobile(ch, person));
		value += ".";
		value += fname(person->player.name);
	}
	else {
		value = PERS(person, ch);
	}
	std::strncpy(buf.data(), value.c_str(), buf.size() - 1);
	buf[buf.size() - 1] = '\0';
	return buf.data();
}

void do_where_person(struct char_data* ch, struct char_data* person,
					 struct string_block* sb) {
	if(!CAN_SEE(ch, person)) {
		return;
	}
	std::ostringstream row;
	row << std::left << std::setw(40) << PERS(person, ch) << "- "
		<< (person->in_room > -1 ? real_roomp(person->in_room)->name : "Nowhere") << " ";
	if(GetMaxLevel(ch) >= DIO) {
		row << "[" << person->in_room << "]";
	}
	row << "\n\r";
	append_to_string_block(sb, row.str().c_str());
}

void do_where_object(struct char_data* ch, struct obj_data* obj,
					 int recurse, struct string_block* sb) {
	const char* rawShortDesc = obj->short_description ? obj->short_description : "(null)";
	int diff = static_cast<int>(std::strlen(rawShortDesc))
		- static_cast<int>(std::strlen(ParseAnsiColors(0, rawShortDesc)));
	std::ostringstream descBuilder;
	descBuilder << std::left << std::setw(diff + 55) << rawShortDesc;
	std::string shortDesc = descBuilder.str();
	std::string line;

	if(obj->in_room != NOWHERE) {
		/* object in a room */
		line = shortDesc;
		line += "- ";
		line += real_roomp(obj->in_room)->name;
		line += " [";
		line += std::to_string(obj->in_room);
		line += "]\n\r";
	}
	else if(obj->carried_by != NULL) {
		/* object carried by monster */
		line = shortDesc;
		line += "- trasportato da ";
		line += numbered_person(ch, obj->carried_by);
		line += "\n\r";
	}
	else if(obj->equipped_by != NULL) {
		/* object equipped by monster */
		line = shortDesc;
		line += "- usato da ";
		line += numbered_person(ch, obj->equipped_by);
		line += "\n\r";
	}
	else if(obj->in_obj) {
		/* object in object */
		line = shortDesc;
		line += "- in ";
		line += numbered_object(ch, obj->in_obj);
		line += "\n\r";
	}
	else {
		line = shortDesc;
		line += "- Nemmeno Dio sa dove...\n\r";
	}
	if(!line.empty()) {
		append_to_string_block(sb, line.c_str());
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

void owhere(struct char_data* ch, char* nome)
{
	std::array<char, MAX_STRING_LENGTH> name{};
//    struct char_data* i;
	struct obj_data* k;
//    struct descriptor_data* d;
	int        number = 0, count = 0;
	struct string_block        sb;
 //   string sb_rent_pg;

	only_argument(nome, name.data());

	int N_oggetto = std::atoi(name.data());

	init_string_block(&sb);

	if(!is_number(name.data()))
	{
		for(k = object_list; k; k = k->next)
		{
			if(isname(name.data(), k->name) && CAN_SEE_OBJ(ch, k))
			{
				if(number==0 || (--count)==0)
				{
					if(number==0)
					{
						std::ostringstream countPrefix;
						countPrefix << "[" << std::setw(3) << ++count << "] ";
						append_to_string_block(&sb, countPrefix.str().c_str());
					}
					do_where_object(ch, k, number!=0, &sb);
					if(number!=0)
					{
						break;
					}
				}
			}
		}
	}

	count++;
	bool found = false;
	struct stringa_valore sb_count;
	if(N_oggetto < 1)
	{
		for(number = 0; number < top_of_objt; number++)
		{
			if(isname(name.data(), obj_index[number].name))
			{
				sb_count = find_obj(ch, obj_index[number].iVNum, count++);
				found = TRUE;
				append_to_string_block(&sb, sb_count.sb.c_str());
				count = sb_count.conteggio;
			}
		}
		if(number >= top_of_objt)
		{
			number = -1;
		}
	}

	if((number < 0 || number >= top_of_objt) && !*sb.data)
	{
		send_to_char("Non trovo niente del genere da nessuna parte.\n\r", ch);
	}
	else
	{
		if(N_oggetto > 0 && N_oggetto < 99999)
		{
			sb_count = find_obj(ch, N_oggetto, count++);
			found = TRUE;
			append_to_string_block(&sb, sb_count.sb.c_str());
			count = sb_count.conteggio;
		}

		if(!*sb.data)
		{
			send_to_char("Non trovo niente del genere da nessuna parte.\n\r", ch);
		}
		else if(!found)
		{
			append_to_string_block(&sb,"Non trovo niente del genere nei personaggi rentati.\n\r");
		}
		else
		{
			page_string_block(&sb, ch);
		}
	}
	destroy_string_block(&sb);
}

ACTION_FUNC(do_where) {
	std::array<char, MAX_INPUT_LENGTH> name{};
	char*        nameonly;
	struct char_data* i;
	struct obj_data* k;
	struct descriptor_data* d;
	int        number, count;
	struct string_block        sb;
	std::array<char, 10> tipo{};
	const char* copia = arg;

	only_argument(arg, name.data());

	copia = one_argument(copia, tipo.data());
	if(std::strcmp(tipo.data(), "obj") == 0 && IS_DIO(ch))
	{
		only_argument(copia, name.data());
		if(is_number(name.data()))
		{
			mudlog(LOG_PLAYERS, "Looking for object #%s on rented toon", name.data());
		}
		else
		{
			mudlog(LOG_PLAYERS, "Looking for '%s' in game and on rented toon", name.data());
		}
		owhere(ch, name.data());
		return;
	}

	if(name[0] == '\0') {
		if(GetMaxLevel(ch) < DIO) {
			send_to_char("Cosa stai cercando?\n\r", ch);
		}
		else {
			init_string_block(&sb);
			append_to_string_block(&sb, "Giocatori:\n\r"
								   "----------\n\r");

			for(d = descriptor_list; d; d = d->next) {
				if(d->character && (d->connected == CON_PLYNG) &&
						(d->character->in_room != NOWHERE) &&
						CAN_SEE(ch, d->character)) {
					std::ostringstream row;
					row << std::left << std::setw(20)
						<< (d->original ? d->original->player.name : d->character->player.name)
						<< " - " << real_roomp(d->character->in_room)->name
						<< " [" << std::setw(3) << d->character->in_room << "]";
					if(d->original) {   /* If switched */
						row << " Nel corpo di " << fname(d->character->player.name);
					}
					row << "\n\r";
					append_to_string_block(&sb, row.str().c_str());
				}
			}
			page_string_block(&sb,ch);
			destroy_string_block(&sb);
		}
		return;
	}

	if(std::isdigit(static_cast<unsigned char>(name[0]))) {
		nameonly = name.data();
		count = number = get_number(&nameonly);
	}
	else {
		count = number = 0;
	}

	init_string_block(&sb);

	for(i = character_list; i; i = i->next) {
		if(isname(name.data(), i->player.name) && CAN_SEE(ch, i)) {

			if(!IS_PC(i) && affected_by_spell(i,STATUS_QUEST) && GetMaxLevel(ch) < IMMORTALE) {
				act("Non si bara! ;)\n\r", FALSE, ch, 0, ch, TO_CHAR);
				break;
			}

			if((i->in_room != NOWHERE) &&
					((GetMaxLevel(ch)>=IMMORTALE) || (real_roomp(i->in_room)->zone ==
							real_roomp(ch->in_room)->zone))) {
				if(number==0 || (--count) == 0) {
					if(number==0) {
						std::ostringstream countPrefix;
						countPrefix << "[" << std::setw(3) << ++count << "] "; /* I love short circuiting :) */
						append_to_string_block(&sb, countPrefix.str().c_str());
					}
					do_where_person(ch, i, &sb);
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
			if(isname(name.data(), k->name) && CAN_SEE_OBJ(ch, k)) {
				if(number==0 || (--count)==0) {
					if(number==0) {
						std::ostringstream countPrefix;
						countPrefix << "[" << std::setw(3) << ++count << "] ";
						append_to_string_block(&sb, countPrefix.str().c_str());
					}
					do_where_object(ch, k, number!=0, &sb);
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
	std::string buf;


	if(IS_NPC(ch)) {
		send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
		return;
	}

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
		{
			std::string unknownClassMsg = "Non riconosco ";
			unknownClassMsg += arg;
			unknownClassMsg += "\n\r";
			send_to_char(unknownClassMsg.c_str(), ch);
		}
		return;
		break;
	}

	RaceMax = RacialMax[GET_RACE(ch)][iClass];

	buf.clear();

	for(i = 1; i <= RaceMax; i++) {
		std::ostringstream levelLine;
		levelLine << "[" << std::setw(2) << i << "] "
				  << std::setw(9) << titles[iClass][i].exp
				  << "-" << std::left << std::setw(9) << titles[iClass][i + 1].exp
				  << " : "
				  << (GET_SEX(ch)==SEX_FEMALE ? titles[iClass][i].title_f : titles[iClass][i].title_m)
				  << "\n\r";
		buf += levelLine.str();
	}
	buf += "\n\r";
	page_string(ch->desc, buf.c_str(), 1);

}



ACTION_FUNC(do_consider) {
	struct char_data* victim;
	std::array<char, 256> name{};
	int diff;

	only_argument(arg, name.data());

	if(!(victim = get_char_room_vis(ch, name.data()))) {
		send_to_char("Chi stai considerando di uccidere?\n\r", ch);
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
		act("Che bisogno hai di considerare $N?", FALSE, ch, 0, victim, TO_CHAR);
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
		{
			std::string msg = "Il numero massimo di punti ferita e' ";
			msg += DescRatio(fnum);
			msg += ".\n\r";
			send_to_char(msg.c_str(), ch);
		}

		num = 105+(int)GetApprox(GET_AC(victim), learn);
		num2 = 105+(int)GET_AC(ch);
//    if (!num2) num2=1;
		fnum = ((int)num-(int)num2);

		{
			std::string msg = "La classe di armatura e' ";
			msg += DescArmorf(fnum);
			msg += ".\n\r";
			send_to_char(msg.c_str(), ch);
		}



		if(learn > 60) {
			std::string msg = "Ha ";
			msg += DescAttacks((int)GetApprox((int)victim->mult_att, learn));
			msg += " attacchi per turno.\n\r";
			send_to_char(msg.c_str(), ch);
		}

		if(learn > 70) {

			num = (int)GetApprox((int)victim->specials.damnodice, learn);
			num2 = (int)GetApprox((int)victim->specials.damsizedice, learn);
			if(!num2) {
				num2=1;
			}
			fnum = (int)num*(num2/2.0);
			{
				std::string msg = "Il danno per attacco e' ";
				msg += DescDamage(fnum);
				msg += ".\n\r";
				send_to_char(msg.c_str(), ch);
			}
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

			{
				std::string msg = "La sua probabilita' di colpirti e' ";
				msg += DescRatioF(fnum);
				msg += ".\n\r";
				send_to_char(msg.c_str(), ch);
			}


			num =   GetApprox(GET_DAMROLL(victim), learn);
			num2 =  GET_DAMROLL(ch);
			if(!num2) {
				num2=1;
			}
			fnum = ((int)num/(int)num2);

			{
				std::string msg = "Il bonus al danno e' ";
				msg += DescRatio(fnum);
				msg += ".\n\r";
				send_to_char(msg.c_str(), ch);
			}

		}
#endif


	}

}

ACTION_FUNC(do_spells) {
	int spl, i;        /* 16384 */
	struct string_block sb;

	if(IS_NPC(ch)) {
		send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
		return;
	}

	init_string_block(&sb);
	{
		std::ostringstream header;
		header << "[#  ] " << std::left << std::setw(30) << "SPELL/SKILL"
			   << "  MANA, Cl, Mu, Dr, Sc, Pa, Ra, Ps\n\r";
		append_to_string_block(&sb, header.str().c_str());
	}

	for(i = 1, spl = 0; i <= MAX_EXIST_SPELL; i++, spl++) {
		if(GetMaxLevel(ch) > IMMORTALE ||
				(spell_info[i].min_level_cleric < ABS_MAX_LVL
				 && spell_info[i].min_level_cleric > 0)) {  // SALVO controllo spell_info
			if(!spells[spl]) {
				mudlog(LOG_ERROR, "!spells[spl] on %d, do_spells in act.info.c", i);
			}
			else {
				std::ostringstream line;
				line << "[" << std::setw(3) << i << "] " << std::left << std::setw(30) << spells[spl]
					 << "  <" << std::setw(3) << spell_info[i].min_usesmana << "> "
					 << std::right << std::setw(2) << spell_info[i].min_level_cleric << " "
					 << std::setw(3) << spell_info[i].min_level_magic << " "
					 << std::setw(3) << spell_info[i].min_level_druid << " "
					 << std::setw(3) << spell_info[i].min_level_sorcerer << " "
					 << std::setw(3) << spell_info[i].min_level_paladin << " "
					 << std::setw(3) << spell_info[i].min_level_ranger << " "
					 << std::setw(3) << spell_info[i].min_level_psi << "\n\r";
				append_to_string_block(&sb, line.str().c_str());
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

	long ct, ot;
	char* tmstr, *otmstr;

	{
		std::ostringstream o;
		o << "$c0005Base Source: $c0014AlarMUD\n$c0005"
			 "Versione $c0015" << version() << "\n$c0005Commit: $c0015" << release() << "$c0005.";
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	{
		std::ostringstream o;
		o << "$c0005Compilazione del : $c0014" << compilazione() << ".";
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	ot = Uptime;
	otmstr = asctime(localtime(&ot));
	*(otmstr + strlen(otmstr) - 1) = '\0';
	{
		std::ostringstream o;
		o << "$c0005Orario di partenza: $c0015" << otmstr << " $c0005";
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	ct = time(0);
	tmstr = asctime(localtime(&ct));
	*(tmstr + strlen(tmstr) - 1) = '\0';
	{
		std::ostringstream o;
		o << "$c0005Orario attuale    : $c0015" << tmstr << " $c0005";
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	{
		std::ostringstream o;
		o << std::fixed << std::setprecision(6);
		o << "$c0005Indice di attesa desiderato: $c0015" << (static_cast<double>(OPT_USEC) / 1000000.0)
		  << " $c0005secs";
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	{
		std::ostringstream o;
		o << std::fixed << std::setprecision(6);
		o << "$c0005Indice di attesa attuale   : $c0015" << GetLagIndex() << " $c0005sec";
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	if(GetMaxLevel(ch) >=IMMORTALE) {
		std::array<char, 256> tbuf{};
		sprintbit((unsigned long)SystemFlags, system_flag_types, tbuf.data());
		{
			std::ostringstream o;
			o << "$c0005Flags di sistema:[$c0015" << tbuf.data() << "$c0005]\n\r";
			act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
		}
		{
			std::ostringstream o;
			o << "$c0005Connessioni dalla partenza:$c0015" << HowManyConnection(0) << "\n\r";
			act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
		}

	}

#if HASH
	{
		std::ostringstream o;
		o << "$c0005Numero di stanze nel mondo          : $c0015" << room_db.klistlen;
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
#else
	{
		std::ostringstream o;
		o << "$c0005Numero di stanze nel mondo          : $c0015" << room_count;
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
#endif
	{
		std::ostringstream o;
		o << "$c0005Numero di zone nel mondo            : $c0015" << (top_of_zone_table + 1);
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	{
		std::ostringstream o;
		o << "$c0005Numero di personaggi attivi         : $c0015" << top_of_p_table;
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	// Result of an aggregate query contains only one element so let's
	// use the query_value() shortcut.
	//
	auto c =Sql::getOne<userCount>();
	{
		std::ostringstream o;
		o << "$c0005Numero di giocatori registrati      : $c0015" << c->count;
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	{
		std::ostringstream o;
		o << "$c0005Numero di tipi di creature nel mondo: $c0015" << top_of_mobt;
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	{
		std::ostringstream o;
		o << "$c0005Numero di tipi di oggetti nel mondo  : $c0015" << top_of_objt;
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	{
		std::ostringstream o;
		o << "$c0005Numero di creature nel gioco         : $c0015" << mob_count;
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	{
		std::ostringstream o;
		o << "$c0005Numero di oggetti nel gioco          : $c0015" << obj_count;
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	/**** SALVO controllo lag refresh zone init */
	if(IS_IMMORTAL(ch)) {
		int i,c;
		for(i = c = 0; i <= top_of_zone_table; i++)
			if(zone_table[i].start != 0) {
				c++;
			}
		{
			std::ostringstream o;
			o << "$c0005Totale zone init nel mondo           : $c0015" << c << " su $c0015" << (top_of_zone_table + 1);
			act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
		}
		{
			std::ostringstream o;
			o << "$c0005Indice attesa refresh zone init      : $c0015" << attrefzone << " $c0005sec";
			act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
		}
	}
	/****/
	{
		std::ostringstream o;
		o << std::fixed << std::setprecision(6);
		o << "$c0005Valore medio dell'eq in gioco        : $c0015" << AverageEqIndex(-1);
		act(o.str().c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

}

ACTION_FUNC(do_attribute) {
	struct affected_type* aff;

	struct time_info_data my_age;


	age3(ch, &my_age);

	{
		std::string msg = "$c0005Tu hai $c0014";
		msg += std::to_string(static_cast<int>(my_age.ayear));
		msg += "$c0005 (ma ne dimostri $c0014";
		msg += std::to_string(static_cast<int>(my_age.year));
		msg += "$c0005) anni e $c0014";
		msg += std::to_string(static_cast<int>(my_age.month));
		msg += "$c0005 mesi, sei ";
		msg += (GET_SEX(ch) == SEX_FEMALE) ? "alta " : "alto ";
		msg += "$c0014";
		msg += std::to_string(static_cast<int>(ch->player.height));
		msg += "$c0005 cm, e pesi $c0014";
		msg += std::to_string(static_cast<int>((ch->player.weight * 4536) / 10000));
		msg += "$c0005 chili.";
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	{
		std::string msg = "$c0005Stai trasportando $c0014";
		msg += std::to_string(static_cast<int>((IS_CARRYING_W(ch) * 4536) / 1000));
		msg += "$c0005 etti di equipaggiamento su $c0014";
		msg += std::to_string(static_cast<int>((CAN_CARRY_W(ch) * 4536) / 1000));
		msg += ".";
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	{
		std::string msg = "$c0005Stai trasportando $c0014";
		msg += std::to_string(static_cast<int>(IS_CARRYING_N(ch)));
		msg += "$c0005 oggetti su $c0014";
		msg += std::to_string(static_cast<int>(CAN_CARRY_N(ch)));
		msg += ".";
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	{
		std::string msg = "$c0005Tu sei$c0014 ";
		msg += ArmorDesc(ch->points.armor);
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	if(GetMaxLevel(ch)>=MAESTRO_DEL_CREATO) {
		std::string msg = "$c0005Armor class: $c0014";
		msg += std::to_string(static_cast<int>(ch->points.armor));
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	//GGPATCH, inserita valutazione spellfail
	int iSpellfail = 0;
	if(IS_CASTER(ch)) {
		iSpellfail=ch->specials.spellfail;
		if(GetMaxLevel(ch) >= MAESTRO_DEL_CREATO) {
			std::string msg = "$c0005Spellfail : $c0014 ";
			msg += std::to_string(iSpellfail);
			act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
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
		{
			std::string msg = "$c0005La tua capacita' di lanciare incantesimi e'$c0014 ";
			msg += SpellfailDesc(IS_IMMORTAL(ch)?0:iSpellfail);
			act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
		}
	}
	if(GetMaxLevel(ch) >= CHUMP) {
		std::string msg = "$c0005Tu hai $c0014";
		msg += std::to_string(static_cast<int>(GET_STR(ch)));
		msg += "$c0005/$c0015";
		msg += std::to_string(static_cast<int>(GET_ADD(ch)));
		msg += " $c0005STR, $c0014";
		msg += std::to_string(static_cast<int>(GET_INT(ch)));
		msg += " $c0005INT, $c0014";
		msg += std::to_string(static_cast<int>(GET_WIS(ch)));
		msg += " $c0005WIS, $c0014";
		msg += std::to_string(static_cast<int>(GET_DEX(ch)));
		msg += " $c0005DEX, $c0014";
		msg += std::to_string(static_cast<int>(GET_CON(ch)));
		msg += " $c0005CON, $c0014";
		msg += std::to_string(static_cast<int>(GET_CHR(ch)));
		msg += " $c0005CHR";
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}

	{
		std::string msg = "$c0005I tuoi hit e damage bonus sono $c0014";
		msg += HitRollDesc(GET_HITROLL(ch));
		msg += "$c0005 e $c0014";
		msg += DamRollDesc(GET_DAMROLL(ch));
		msg += "$c0005 rispettivamente.";
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	{
		std::string msg = "$c0005Il tuo equipaggiamento e' $c0014";
		msg += EqDesc(GetCharBonusIndex(ch));
		msg += "$c0005";
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
	}
	if(GetMaxLevel(ch)>=MAESTRO_DEL_CREATO) {
		std::string msg = "$c0005Hit:$c0014+";
		msg += std::to_string(static_cast<int>(GET_HITROLL(ch)));
		msg += "$c0005 Dam:$c0014+";
		msg += std::to_string(static_cast<int>(GET_DAMROLL(ch)));
		act(msg.c_str(), FALSE, ch, 0, 0, TO_CHAR);
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

						std::string spellLine = "$c0005Spell : '$c0014";
						spellLine += spells[aff->type - 1];
						spellLine += "$c0005' - $c0014";
						spellLine += std::to_string(static_cast<int>(aff->duration));
						spellLine += "$c0005";
						act(spellLine.c_str(), FALSE, ch, 0, 0, TO_CHAR);
					}

					break;
				}
			}
		}
	}
}

ACTION_FUNC(do_value) {
	std::array<char, MAX_INPUT_LENGTH> name{};
	struct obj_data* obj=0;
	struct char_data* vict=0;

	/* Spell Names */


	/* For Objects */


	if(!HasClass(ch, CLASS_THIEF|CLASS_RANGER)) {
		send_to_char("Scordatelo!!", ch);
		return;
	}

	arg = one_argument(arg, name.data());

	if((obj = get_obj_in_list_vis(ch, name.data(), ch->carrying))==0) {
		if((vict = get_char_room_vis(ch, name.data()))==0) {
			send_to_char("Di chi o di cosa stai parlando?\n\r", ch);
			return;
		}
		else {
			only_argument(arg, name.data());
			if((obj = get_obj_in_list_vis(ch, name.data(), vict->carrying))==0) {
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


	{
		std::string header = "Oggetto: ";
		header += (obj->short_description ? obj->short_description : "(null)");
		header += ".  Tipo: ";
		std::array<char, 512> typeBuf{};
		sprinttype(GET_ITEM_TYPE(obj), item_types, typeBuf.data());
		header += typeBuf.data();
		header += "\n\r";
		send_to_char(header.c_str(), ch);
	}

	if(!ch->skills) {
		return;
	}


	if(number(1,101) < ch->skills[SKILL_EVALUATE].learned/3) {
		if(obj->obj_flags.bitvector) {
			std::string affLine = fname(obj->name);
			affLine += " ti da' le seguenti abilita': ";
			std::array<char, MAX_STRING_LENGTH> affBuf{};
			sprintbit((unsigned long)obj->obj_flags.bitvector, affected_bits, affBuf.data());
			affLine += affBuf.data();
			affLine += "\n\r";
			send_to_char(affLine.c_str(), ch);
		}
	}

	if(number(1,101) < ch->skills[SKILL_EVALUATE].learned/2) {
		send_to_char("L'oggetto e': ", ch);
		std::array<char, MAX_STRING_LENGTH> extraBuf{};
		sprintbit2((unsigned long)obj->obj_flags.extra_flags, extra_bits,
				   (unsigned long)obj->obj_flags.extra_flags2, extra_bits2, extraBuf.data());
		std::string extraLine(extraBuf.data());
		extraLine += "\n\r";
		send_to_char(extraLine.c_str(), ch);
	}

	{
		std::ostringstream statsLine;
		statsLine << "Peso: " << obj->obj_flags.weight
				  << ", Valore: " << GetApprox(obj->obj_flags.cost, ch->skills[SKILL_EVALUATE].learned - 10)
				  << ", Rent cost: " << GetApprox(obj->obj_flags.cost_per_day,
					  ch->skills[SKILL_EVALUATE].learned - 10)
				  << "  " << (obj->obj_flags.cost >= LIM_ITEM_COST_MIN ? "[RARO]" : " ") << "\n\r";
		send_to_char(statsLine.str().c_str(), ch);
	}

	if(ITEM_TYPE(obj) == ITEM_WEAPON) {
		std::ostringstream weaponLine;
		weaponLine << "Valore di danno: '"
				   << GetApprox(obj->obj_flags.value[1], ch->skills[SKILL_EVALUATE].learned - 10)
				   << "D"
				   << GetApprox(obj->obj_flags.value[2], ch->skills[SKILL_EVALUATE].learned - 10)
				   << "'\n\r";
		send_to_char(weaponLine.str().c_str(), ch);
	}
	else if(ITEM_TYPE(obj) == ITEM_ARMOR) {

		std::ostringstream armorLine;
		armorLine << "L'AC e' "
				  << GetApprox(obj->obj_flags.value[0], ch->skills[SKILL_EVALUATE].learned - 10)
				  << "\n\r";
		send_to_char(armorLine.str().c_str(), ch);
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

	i = std::atoi(arg);

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

	i = std::atoi(arg);

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
	int i,max;

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
		{
			std::string buffer;
			for(max=0; max<IMMORTALE; max++) {
				for(i=0; *spells[i] != '\n'; i++) {
					if(spell_info[i+1].spell_pointer &&
							spell_info[i+1].min_level_magic==max) {
						std::string line = "[";
						line += std::to_string(static_cast<int>(spell_info[i+1].min_level_magic));
						line += "] ";
						line += spells[i];
						line += " ";
						line += how_good(ch->skills[i+1].learned);
						line += " \n\r";
						if(buffer.size() + line.size() + 1 > (MAX_STRING_LENGTH * 2) - 2) {
							break;
						}
						buffer += line;
						buffer += "\r";
					}
				}
			}
			page_string(ch->desc, buffer.c_str(), 1);
		}
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
		{
			std::string buffer;
			for(max=0; max<IMMORTALE; max++) {
				for(i=0; *spells[i] != '\n'; i++) {
					if(spell_info[i+1].spell_pointer &&
							spell_info[i+1].min_level_cleric==max) {
						std::string line = "[";
						line += std::to_string(static_cast<int>(spell_info[i+1].min_level_cleric));
						line += "] ";
						line += spells[i];
						line += " ";
						line += how_good(ch->skills[i+1].learned);
						line += " \n\r";
						if(buffer.size() + line.size() + 1 > (MAX_STRING_LENGTH * 2) - 2) {
							break;
						}
						buffer += line;
						buffer += "\r";
					}
				}
			}
			page_string(ch->desc, buffer.c_str(), 1);
		}
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
		{
			std::string buffer;
			for(max=0; max<IMMORTALE; max++) {
				for(i=0; *spells[i] != '\n'; i++) {
					if(spell_info[i+1].spell_pointer &&
							spell_info[i+1].min_level_druid==max) {
						std::string line = "[";
						line += std::to_string(static_cast<int>(spell_info[i+1].min_level_druid));
						line += "] ";
						line += spells[i];
						line += " ";
						line += how_good(ch->skills[i+1].learned);
						line += " \n\r";
						if(buffer.size() + line.size() + 1 > (MAX_STRING_LENGTH * 2) - 2) {
							break;
						}
						buffer += line;
						buffer += "\r";
					}
				}
			}
			page_string(ch->desc, buffer.c_str(), 1);
		}
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
		{
			std::string buffer;
			for(max=0; max<IMMORTALE; max++) {
				for(i=0; *spells[i] != '\n'; i++) {
					if(spell_info[i+1].spell_pointer &&
							spell_info[i+1].min_level_sorcerer==max) {
						std::string line = "[";
						line += std::to_string(static_cast<int>(spell_info[i+1].min_level_sorcerer));
						line += "] ";
						line += spells[i];
						line += " ";
						line += how_good(ch->skills[i+1].learned);
						line += " \n\r";
						if(buffer.size() + line.size() + 1 > (MAX_STRING_LENGTH * 2) - 2) {
							break;
						}
						buffer += line;
						buffer += "\r";
					}
				}
			}
			page_string(ch->desc, buffer.c_str(), 1);
		}
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
		{
			std::string buffer;
			for(max=0; max<IMMORTALE; max++) {
				for(i=0; *spells[i] != '\n'; i++) {
					if(spell_info[i+1].spell_pointer &&
							spell_info[i+1].min_level_paladin==max) {
						std::string line = "[";
						line += std::to_string(static_cast<int>(spell_info[i+1].min_level_paladin));
						line += "] ";
						line += spells[i];
						line += " ";
						line += how_good(ch->skills[i+1].learned);
						line += " \n\r";
						if(buffer.size() + line.size() + 1 > (MAX_STRING_LENGTH * 2) - 2) {
							break;
						}
						buffer += line;
						buffer += "\r";
					}
				}
			}
			page_string(ch->desc, buffer.c_str(), 1);
		}
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
		{
			std::string buffer;
			for(max=0; max<IMMORTALE; max++) {
				for(i=0; *spells[i] != '\n'; i++) {
					if(spell_info[i+1].spell_pointer &&
							spell_info[i+1].min_level_ranger==max) {
						std::string line = "[";
						line += std::to_string(static_cast<int>(spell_info[i+1].min_level_ranger));
						line += "] ";
						line += spells[i];
						line += " ";
						line += how_good(ch->skills[i+1].learned);
						line += " \n\r";
						if(buffer.size() + line.size() + 1 > (MAX_STRING_LENGTH * 2) - 2) {
							break;
						}
						buffer += line;
						buffer += "\r";
					}
				}
			}
			page_string(ch->desc, buffer.c_str(), 1);
		}
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
		{
			std::string buffer;
			for(max=0; max<IMMORTALE; max++) {
				for(i=0; *spells[i] != '\n'; i++) {
					if(spell_info[i+1].spell_pointer && spell_info[i+1].min_level_psi==max) {
						std::string line = "[";
						line += std::to_string(static_cast<int>(spell_info[i+1].min_level_psi));
						line += "] ";
						line += spells[i];
						line += " ";
						line += how_good(ch->skills[i+1].learned);
						line += " \n\r";
						if(buffer.size() + line.size() + 1 > (MAX_STRING_LENGTH * 2) - 2) {
							break;
						}
						buffer += line;
						buffer += "\r";
					}
				}
			}
			page_string(ch->desc, buffer.c_str(), 1);
		}
		return;
		break;
	}
	default:
		send_to_char("Quale classe?\n\r", ch);
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
	std::array<char, MAX_STRING_LENGTH> arg1{};
	std::array<char, MAX_STRING_LENGTH> arg2{};
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

	argument_split_2(arg, arg1.data(), arg2.data());
	sd = search_block(arg1.data(), dirs, FALSE);
	std::string roomAct;
	std::string viewerMsg;
	if(sd == -1) {
		smin = 0;
		smax = 5;
		swt = 3;
		roomAct = "$n scruta intensamente i dintorni.";
		viewerMsg = "Guardandoti intorno con attenzione vedi...\n\r";
	}
	else {
		smin = sd;
		smax = sd;
		swt = 1;
		roomAct = "$n scruta intensamente ";
		roomAct += dirsTo[sd];
		roomAct += ".";
		viewerMsg = "Guardando attentamente ";
		viewerMsg += dirsTo[sd];
		viewerMsg += ", vedi...\n\r";
	}

	act(roomAct.c_str(), FALSE, ch, 0, 0, TO_ROOM);
	send_to_char(viewerMsg.c_str(), ch);
	nfnd = 0;
	/* Check in room first */
	for(spud = real_roomp(ch->in_room)->people; spud;
			spud = spud->next_in_room) {
		if(CAN_SEE(ch, spud) && !IS_SET(spud->specials.affected_by, AFF_HIDE) &&
				spud != ch) {
			const char* label = IS_NPC(spud) ? spud->player.short_descr : GET_NAME(spud);
			if(!label) {
				label = "";
			}
			std::ostringstream hereLine;
			hereLine << std::setw(30) << std::right << label << " : qui\n\r";
			send_to_char(hereLine.str().c_str(), ch);
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
							const char* label = IS_NPC(spud) ? spud->player.short_descr : GET_NAME(spud);
							if(!label) {
								label = "";
							}
							std::ostringstream scanLine;
							scanLine << std::setw(30) << std::right << label << " : "
									 << rng_desc[range] << " " << dir_desc[i] << "\n\r";
							send_to_char(scanLine.str().c_str(), ch);
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
	std::string buf;

	buf += "$c0015[------- Gruppi di avventurieri -------]\n\r";

	/* go through the descriptor list */
	for(i = descriptor_list; i; i=i->next) {
		/* find everyone who is a master  */
		if(!i->connected) {
			person = i->character;
			/* list the master and the group name */
			if(person && !person->master && IS_AFFECTED(person, AFF_GROUP)) {
				if(person->specials.group_name && CAN_SEE(ch, person)) {
					buf += "          $c0015";
					buf += person->specials.group_name;
					buf += "\n\r$c0014";
					buf += fname(GET_NAME(person));
					buf += "\n\r";

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
									std::strlen(GET_NAME(f->follower)) > 1) {
								buf += "$c0013";
								buf += fname(GET_NAME(f->follower));
								buf += "\n\r";
							}
							else {
								buf += "$c0013Qualcuno\n\r";
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
	buf += "\n\r$c0015[---------- Fine lista --------------]\n\r";
	page_string(ch->desc, buf.c_str(), 1);
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
