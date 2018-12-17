/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* AlarMUD
 * $Id: spec_procs.c,v 2.1 2002/03/27 19:50:22 Thunder Exp $
 * */
/***************************  System  include ************************************/
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <ctime>
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
#include "spec_procs.hpp"
#include "act.comm.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.obj1.hpp"
#include "act.obj2.hpp"
#include "act.social.hpp"
#include "act.wizard.hpp"
#include "aree.hpp"
#include "breath.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic2.hpp"
#include "mobact.hpp"
#include "opinion.hpp"
#include "reception.hpp"
#include "regen.hpp"
#include "skills.hpp"
#include "spec_procs2.hpp"
#include "spec_procs3.hpp"
#include "spell_parser.hpp"
#include "spells1.hpp"
#include "spells2.hpp"

namespace Alarmud {


#define INQ_SHOUT 1
#define INQ_LOOSE 0

#define SWORD_ANCIENTS 25000
/*
 *  list of room #s
 */
#define Elf_Home     1414
#define Bakery       3009
#define Dump         3030
#define Ivory_Gate   1499

/* Data declarations */

struct social_type {
	char* cmd;
	int next_line;
};


/*************************************/
/* predicates for find_path function */

FIND_FUNC(is_target_room_p) {
	return room == reinterpret_cast<const intptr_t>(tgt_room);
}

FIND_FUNC(named_object_on_ground) {
	const char* name = reinterpret_cast<const char*>(tgt_room);
	return 0 != get_obj_in_list(name, real_roomp(room)->contents);
}

/* predicates for find_path function */
/*************************************/



/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */

char* how_good(int percent) {
	static char buf[256];

	if(percent < 0) {
		strcpy(buf, " (ehm.... GUF! Avvisa Alar.....)");
	}
	else if(percent == 0) {
		strcpy(buf, " ($c0012sconosciuta$c0007)");
	}
	else if(percent <= 10) {
		strcpy(buf, " ($c0009pessima$c0007)");
	}
	else if(percent <= 20) {
		strcpy(buf, " ($c0009scarsa$c0007)");
	}
	else if(percent <= 40) {
		strcpy(buf, " ($c0011povera$c0007)");
	}
	else if(percent <= 55) {
		strcpy(buf, " ($c0011media$c0007)");
	}
	else if(percent <= 70) {
		strcpy(buf, " ($c0011discreta$c0007)");
	}
	else if(percent <= 80) {
		strcpy(buf, " ($c0010buona$c0007)");
	}
	else if(percent <= 85) {
		strcpy(buf, " ($c0010molto buona$c0007)");
	}
	else {
		strcpy(buf, " ($c0010Superba$c0007)");
	}

	return (buf);
}

int GainLevel(struct char_data* ch, int iClass) {
	/* PRIMA DEL NUOVO LIVELLO */
#if not NOREGISTER
	if(! GET_AUTHBY(ch) && GetMaxLevel(ch)>=CHUMP-2) {
		if(GetMaxLevel(ch)==CHUMP-2) {  /* Sta livellando il decimo */
			send_to_char("Prima del prossimo livello devi invocare il favore di un Dio! (help register)\n\r",ch);
		}
		else {
			send_to_char("Non hai il favore degli dei. Non puoi livellare. (help register)\n\r",ch);
			return(FALSE);
		}
	}
#endif
	if(GET_EXP(ch)>=
			titles[iClass][GET_LEVEL(ch, iClass)+1].exp) {
		if(GET_LEVEL(ch, iClass) < RacialMax[GET_RACE(ch)][iClass] && !IS_PRINCE(ch)) {

			send_to_char("Cresci di un livello!\n\r", ch);
			advance_level(ch, iClass);
			set_title(ch);
			return(TRUE);
		}
		else {
			send_to_char("Non puoi avanzare oltre in questa classe\n\r", ch);
		}
	}
	else {
		send_to_char("Non hai abbastanza punti esperienza!\n\r",ch);
	}
	return(FALSE);
}

struct char_data* FindMobInRoomWithFunction(int room, genericspecial_func func) {
	struct char_data* temp_char, *targ;

	targ = NULL;

	if(room > NOWHERE) {
		for(temp_char = real_roomp(room)->people; (!targ) && (temp_char);
				temp_char = temp_char->next_in_room) {

			if(IS_MOB(temp_char)) {
				if(mob_index[temp_char->nr].func == func) {
					targ = temp_char;
					break;
				}
			}
		}
	}
	else {
		return(NULL);
	}

	return(targ);

}

struct char_data* FindMobInRoomWithVNum(int room, int VNum) {
	struct char_data* temp_char, *targ;

	targ = 0;

	if(room > NOWHERE) {
		for(temp_char = real_roomp(room)->people; (!targ) && (temp_char);
				temp_char = temp_char->next_in_room) {
			if(IS_MOB(temp_char)) {
				if(mob_index[temp_char->nr].iVNum == VNum) {
					targ = temp_char;
				}
			}
		}
	}
	else {
		return(0);
	}

	return(targ);

}

MOBSPECIAL_FUNC(MageGuildMaster) {
	int number, i, max;
	char buf[MAX_INPUT_LENGTH];
	struct char_data* guildmaster;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(!ch->skills) {
		return(FALSE);
	}

	if(IS_IMMORTAL(ch)) {
		return(FALSE);
	}

	if(check_soundproof(ch)) {
		return(FALSE);
	}

	guildmaster = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(MageGuildMaster));

	if(!guildmaster) {
		return(FALSE);
	}

	if(cmd != CMD_PRACTICE && cmd != CMD_GAIN) {
		return(FALSE);
	}

	if(IS_NPC(ch)) {
		act("$N ti dice 'Ti sembro un addestratore di animali ?'", FALSE,
			ch, 0, guildmaster, TO_CHAR);
		return(FALSE);
	}

	for(; *arg == ' '; arg++);

	if(HasClass(ch, CLASS_MAGIC_USER) || HasClass(ch,CLASS_SORCERER)) {
		if(cmd == CMD_GAIN) {
			if(HasClass(ch,CLASS_MAGIC_USER)) {
				if(GET_LEVEL(ch,MAGE_LEVEL_IND) < GetMaxLevel(guildmaster)-10) {
					if(GET_EXP(ch) <
							titles[MAGE_LEVEL_IND][GET_LEVEL(ch, MAGE_LEVEL_IND)+1].exp) {
						act("Non sei ancora pront$b\n\r", FALSE, ch, 0, guildmaster,
							TO_CHAR);
						return FALSE;
					}
					else {
						GainLevel(ch, MAGE_LEVEL_IND);
						return TRUE;
					}
				}
				else {
					send_to_char("Non posso allenarti, devi cercare qualcun'altro.\n\r",
								 ch);
				}
			}
			else if(HasClass(ch,CLASS_SORCERER)) {
				if(GET_LEVEL(ch,SORCERER_LEVEL_IND) < GetMaxLevel(guildmaster)-10) {
					if(GET_EXP(ch) < titles[SORCERER_LEVEL_IND]
							[GET_LEVEL(ch, SORCERER_LEVEL_IND)+1].exp) {
						act("Non sei ancora pront$b\n\r", FALSE, ch, 0, guildmaster,
							TO_CHAR);
						return FALSE;
					}
					else {
						GainLevel(ch,SORCERER_LEVEL_IND);
						return TRUE;
					}
				}
				else {
					send_to_char("Non posso allenarti, devi cercare qualcun'altro.\n\r",
								 ch);
				}
			}
			return(TRUE);
		}
		else if(cmd == CMD_PRACTICE) {
			if(!*arg) {
				sprintf(buf,"Hai a disposizione %d sessioni di pratica.\n\r",
						ch->specials.spells_to_learn);
				send_to_char(buf, ch);
				send_to_char("Puoi praticare questi spells:\n\r", ch);

				for(max=1; max<=GET_LEVEL(ch,(HasClass(ch,CLASS_MAGIC_USER)) ? MAGE_LEVEL_IND :SORCERER_LEVEL_IND); max++) { // SALVO ordino le prac mageguild
					for(i=0; *spells[i] != '\n'; i++) {
						if(HasClass(ch,CLASS_MAGIC_USER)) {
							if(spell_info[ i + 1 ].min_level_magic != max) {
								continue;
							}
							if(spell_info[ i + 1 ].spell_pointer &&
									spell_info[ i + 1 ].min_level_magic <=
									GET_LEVEL_CASTER(ch, MAGE_LEVEL_IND) &&
									spell_info[ i + 1 ].min_level_magic <=
									GetMaxLevel(guildmaster) - 10) {
								sprintf(buf,"[%d] %s %s \n\r",
										spell_info[i+1].min_level_magic,
										spells[i],how_good(ch->skills[i+1].learned));
								send_to_char(buf, ch);
							}
						}
						else if(HasClass(ch,CLASS_SORCERER)) {
							if(spell_info[ i + 1 ].min_level_sorcerer != max) {
								continue;
							}
							if(spell_info[ i + 1 ].spell_pointer &&
									spell_info[ i + 1 ].min_level_sorcerer <= // SALVO mi pare che qui non sia min_level_magic
									GET_LEVEL_CASTER(ch, SORCERER_LEVEL_IND) &&
									spell_info[ i + 1 ].min_level_sorcerer <= // SALVO mi pare che qui non sia min_level_magic
									GetMaxLevel(guildmaster) - 10) {
								sprintf(buf,"[%d] %s %s \n\r",
										spell_info[ i + 1 ].min_level_sorcerer, // SALVO mi pare che qui non sia min_level_magic
										spells[i],how_good(ch->skills[i+1].learned));
								send_to_char(buf, ch);
							}
						}
					}
				} // for max
				return(TRUE);
			}
			else {
				for(; isspace(*arg); arg++);
				number = old_search_block(arg,0,strlen(arg),spells,FALSE);
				if(number == -1
						|| (HasClass(ch,CLASS_MAGIC_USER) && spell_info[ number ].min_level_magic <1) // SALVO non si praccano quelle sconosciute
						|| (HasClass(ch,CLASS_SORCERER) && spell_info[ number ].min_level_sorcerer <1)) { // SALVO non si praccano quelle sconosciute neppure i sorcerer
					send_to_char("Non so' nulla di questa magia.\n\r", ch);
					return(TRUE);
				}

				if((HasClass(ch, CLASS_MAGIC_USER) &&
						GET_LEVEL_CASTER(ch, MAGE_LEVEL_IND) <
						spell_info[ number ].min_level_magic) ||
						(HasClass(ch, CLASS_SORCERER) &&
						 GET_LEVEL_CASTER(ch, SORCERER_LEVEL_IND) <
						 spell_info[ number ].min_level_sorcerer)) {   // SALVO mi pare che qui non sia min_level_magic
					do_say(guildmaster,
						   "Non sei abbastanza potente per imparare questa magia.\n\r",
						   0);
					return TRUE;
				}

				if(GetMaxLevel(guildmaster) - 10 <
						spell_info[ number ].min_level_magic) {
					do_say(guildmaster,
						   "Non sono abbastanza potente per insegnarti questa magia",
						   0);
					return TRUE;
				}
				if(ch->specials.spells_to_learn <= 0) {
					do_say(guildmaster, "Non puoi imparare altro per ora.", 0);
					return TRUE;
				}

				if(ch->skills[ number ].learned >= 60) {
					do_say(guildmaster,
						   "Io non posso piu' insegnarti nulla su questa magia. "
						   "Puoi migliorare solo usandola.", 0);
					return(TRUE);
				}

				send_to_char("Ti alleni per un po'...\n\r", ch);
				ch->specials.spells_to_learn--;

				if(!IS_SET(ch->skills[ number ].flags, SKILL_KNOWN)) {
					SET_BIT(ch->skills[ number ].flags, SKILL_KNOWN);
					if(HasClass(ch, CLASS_SORCERER)) {
						SET_BIT(ch->skills[ number ].flags, SKILL_KNOWN_SORCERER);
					}
					else {
						SET_BIT(ch->skills[ number ].flags, SKILL_KNOWN_MAGE);
					}
				}
				else if(HasClass(ch, CLASS_SORCERER) && !IS_SET(ch->skills[ number ].flags,SKILL_KNOWN_SORCERER)) {    // SALVO metto un eventuale controllo sul sorcerer
					SET_BIT(ch->skills[ number ].flags, SKILL_KNOWN_SORCERER);
				}

				ch->skills[ number ].learned += int_app[(int)GET_INT(ch) ].learn;

				if(ch->skills[ number ].learned >= 95) {
					act("Tu sei espert$b in questa materia.\n\r", FALSE, ch, 0, 0,
						TO_CHAR);
				}
				return TRUE;
			}
		}
	}
	/**** SALVO skills prince ****/
	else if(IS_PRINCE(ch) && !HasClass(ch,CLASS_MAGIC_USER) && cmd !=CMD_GAIN) {
		if(!*arg) {
			sprintf(buf,"Hai ancora %d sessioni di pratica.\n\r",
					ch->specials.spells_to_learn);
			send_to_char(buf, ch);
			send_to_char("Puoi praticare questa skills:\n\r", ch);
			sprintf(buf,"[%d] %s %s \n\r",
					PRINCIPE,spells[SPELL_CHAIN_LIGHTNING-1],
					how_good(ch->skills[SPELL_CHAIN_LIGHTNING].learned));
			send_to_char(buf, ch);
			return(TRUE);
		}
		for(; isspace(*arg); arg++);
		number = old_search_block(arg,0,strlen(arg),spells,FALSE);
		if(number == -1) {
			send_to_char("Non conosco questa pratica...\n\r", ch);
			return(TRUE);
		}
		if(number !=SPELL_CHAIN_LIGHTNING) {
			send_to_char("Non posso insegnarti questa pratica...\n\r", ch);
			return(TRUE);
		}
		if(ch->specials.spells_to_learn <= 0) {
			send_to_char("Non hai pratiche a disposizione.\n\r", ch);
			return(TRUE);
		}
		if(ch->skills[number].learned >= 45) {
			send_to_char("Non posso aiutarti a migliorati ancora.\n\r", ch);
			return(TRUE);
		}
		send_to_char("Hai fatto pratica...\n\r", ch);
		ch->specials.spells_to_learn--;


		if(!IS_SET(ch->skills[number].flags, SKILL_KNOWN)) {
			SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
			SET_BIT(ch->skills[number].flags, SKILL_KNOWN_MAGE);
		}

		ch->skills[ number ].learned += int_app[(int)GET_INT(ch) ].learn;

		if(ch->skills[number].learned >= 95) {
			send_to_char("Hai imparato tutto.\n\r", ch);
		}
		return TRUE;
	}
	/**** fine skills prince ****/
	return FALSE;
}

MOBSPECIAL_FUNC(ClericGuildMaster) {

	int number, i, max;
	char buf[MAX_INPUT_LENGTH];
	struct char_data* guildmaster;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(!ch->skills) {
		return FALSE;
	}

	if(check_soundproof(ch)) {
		return FALSE;
	}

	if(IS_IMMORTAL(ch)) {
		return FALSE;
	}

	guildmaster = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(ClericGuildMaster));

	if(!guildmaster) {
		return FALSE;
	}

	if(cmd != CMD_PRACTICE && cmd != CMD_GAIN &&
			cmd != CMD_GIVE) {
		return(FALSE);
	}

	if(IS_NPC(ch)) {
		act("$N tells you 'What do i look like, an animal trainer?'", FALSE,
			ch, 0, guildmaster, TO_CHAR);
		return(FALSE);
	}

	for(; *arg == ' '; arg++);

	if(HasClass(ch, CLASS_CLERIC)) {
#if QUEST_GAIN
		if(cmd == CMD_GAIN || cmd == CMD_GIVE)   /*gain or give */
#else
		if(cmd == CMD_GAIN)   /*gain */
#endif
		{
			if(GET_LEVEL(ch,CLERIC_LEVEL_IND) < GetMaxLevel(guildmaster)-10) {
#if QUEST_GAIN
				MakeQuest(ch, guildmaster, CLERIC_LEVEL_IND, arg, cmd);
#else
				if(GET_EXP(ch) <
						titles[CLERIC_LEVEL_IND][GET_LEVEL(ch, CLERIC_LEVEL_IND)+1].exp) {
					act("Non sei ancora pront$b.", FALSE, ch, 0, 0, TO_CHAR);
					return(FALSE);
				}
				else {
					GainLevel(ch,CLERIC_LEVEL_IND);
					return(TRUE);
				}
#endif
			}
			else {
				send_to_char("I cannot train you.. You must find another.\n\r",ch);
			}
			return(TRUE);
		}

		if(!*arg) {
			sprintf(buf,"You have got %d practice sessions left.\n\r",
					ch->specials.spells_to_learn);
			send_to_char(buf, ch);
			send_to_char("You can practice any of these spells:\n\r", ch);
			for(max=1; max<=GET_LEVEL(ch,CLERIC_LEVEL_IND); max++) { // SALVO ordino le prac clericguild
				for(i=0; *spells[i] != '\n'; i++) {
					if(spell_info[i+1].min_level_cleric != max) {
						continue;
					}
					if(spell_info[i+1].spell_pointer &&
							(spell_info[i+1].min_level_cleric <=
							 GET_LEVEL_CASTER(ch,CLERIC_LEVEL_IND)) &&
							(spell_info[i+1].min_level_cleric <=
							 GetMaxLevel(guildmaster)-10)) {
						sprintf(buf,"[%d] %s %s \n\r",
								spell_info[i+1].min_level_cleric,spells[i],
								how_good(ch->skills[i+1].learned));
						send_to_char(buf, ch);
					}
				}
			} // for max
			return(TRUE);
		}
		for(; isspace(*arg); arg++);
		number = old_search_block(arg,0,strlen(arg),spells,FALSE);
		if(number == -1
				|| (HasClass(ch,CLASS_CLERIC) && spell_info[ number ].min_level_cleric <1)) { // SALVO non si praccano quelle sconosciute
			send_to_char("You do not know of this spell...\n\r", ch);
			return(TRUE);
		}
		if(GET_LEVEL_CASTER(ch,CLERIC_LEVEL_IND) < spell_info[number].min_level_cleric) {
			send_to_char("You do not know of this spell...\n\r", ch);
			return(TRUE);
		}
		if(GetMaxLevel(guildmaster)-10 < spell_info[number].min_level_cleric) {
			do_say(guildmaster, "I don't know of this spell.", 0);
			return(TRUE);
		}
		if(ch->specials.spells_to_learn <= 0) {
			send_to_char("You do not seem to be able to practice now.\n\r", ch);
			return(TRUE);
		}
		/* SALVO rimuovo questa procedura causa la memorizzazione non conteggiata, occhio rimane nei pc vecchi
		     if  (IS_SET(ch->skills[number].flags,SKILL_KNOWN_SORCERER))
		     {
			send_to_char("Now, you can cast this spell.\n\r", ch);
			REMOVE_BIT(ch->skills[number].flags, SKILL_KNOWN_SORCERER);
			ch->specials.spells_to_learn--;
			return(TRUE);
		     }
		*/
		if(ch->skills[number].learned >= 45) {
			send_to_char("You must use this spell to get any better.  I cannot train you further.\n\r", ch);
			return(TRUE);
		}
		send_to_char("You Practice for a while...\n\r", ch);
		ch->specials.spells_to_learn--;

		if(!IS_SET(ch->skills[number].flags, SKILL_KNOWN)) {
			SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
			SET_BIT(ch->skills[number].flags, SKILL_KNOWN_CLERIC);
		}

		ch->skills[ number ].learned += int_app[(int)GET_INT(ch) ].learn;

		if(ch->skills[number].learned >= 95) {
			send_to_char("You are now learned in this area.\n\r", ch);
		}
		return(TRUE);
	}
	/**** SALVO skills prince ****/
	else if(IS_PRINCE(ch) && !HasClass(ch, CLASS_CLERIC) && cmd !=CMD_GAIN) {
		if(!*arg) {
			sprintf(buf,"Hai ancora %d sessioni di pratica.\n\r",
					ch->specials.spells_to_learn);
			send_to_char(buf, ch);
			send_to_char("Puoi praticare questa skills:\n\r", ch);
			sprintf(buf,"[%d] %s %s \n\r",
					PRINCIPE,spells[SPELL_CURE_CRITIC-1],
					how_good(ch->skills[SPELL_CURE_CRITIC].learned));
			send_to_char(buf, ch);
			return(TRUE);
		}
		for(; isspace(*arg); arg++);
		number = old_search_block(arg,0,strlen(arg),spells,FALSE);
		if(number == -1) {
			send_to_char("Non conosco questa pratica...\n\r", ch);
			return(TRUE);
		}
		if(number !=SPELL_CURE_CRITIC) {
			send_to_char("Non posso insegnarti questa pratica...\n\r", ch);
			return(TRUE);
		}
		if(ch->specials.spells_to_learn <= 0) {
			send_to_char("Non hai pratiche a disposizione.\n\r", ch);
			return(TRUE);
		}
		if(ch->skills[number].learned >= 45) {
			send_to_char("Non posso aiutarti a migliorati ancora.\n\r", ch);
			return(TRUE);
		}
		send_to_char("Hai fatto pratica...\n\r", ch);
		ch->specials.spells_to_learn--;


		if(!IS_SET(ch->skills[number].flags, SKILL_KNOWN)) {
			SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
			SET_BIT(ch->skills[number].flags, SKILL_KNOWN_CLERIC);
		}

		ch->skills[ number ].learned += int_app[(int)GET_INT(ch) ].learn;

		if(ch->skills[number].learned >= 95) {
			send_to_char("Hai imparato tutto.\n\r", ch);
		}
		return TRUE;
	}
	/**** fine skills prince ****/
	return(FALSE);
}

MOBSPECIAL_FUNC(ThiefGuildMaster) {
	char buf[256];
	struct char_data* guildmaster;
	const static char* n_skills[] = {
		"sneak",     /* 1 */
		"hide",
		"steal",
		"backstab",
		"pick",         /* 5 */
		"spy",
		"retreat",
		"find trap",
		"disarm trap",
		"tspy",
		"eavesdrop",
		"\n",
	};
	int number=0;
	int sk_num;

	if(!AWAKE(ch)) {
		return(FALSE);
	}

	if(type != EVENT_COMMAND) {
		if(ch->specials.fighting) {
			return(fighter(ch, cmd, arg, ch, 0));
		}
		return(FALSE);
	}

	if(!ch->skills) {
		return FALSE;
	}

	if(check_soundproof(ch)) {
		return FALSE;
	}

	guildmaster = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(ThiefGuildMaster));

	for(; *arg==' '; arg++); /* ditch spaces */
	if(cmd == CMD_PRACTICE || cmd == CMD_GAIN) {
		if(IS_NPC(ch)) {
			act("$N tells you 'Cosa ti sembro, un ammaestratore di animali?'", FALSE,
				ch, 0, guildmaster, TO_CHAR);
			return(FALSE);
		}
		/**** SALVO skills prince            VVVVVVVVVVVVVVVVV ****/
		if(!HasClass(ch, CLASS_THIEF) && !IS_PRINCE(ch)) {
			send_to_char("Il ThiefGuildmaster dice 'Vattene via di qui!'\n\r",ch);
			return TRUE;
		}
		/**** SALVO skills prince ****/
		else if(IS_PRINCE(ch) && !HasClass(ch, CLASS_THIEF) && cmd !=CMD_GAIN) {
			if(!*arg) {
				sprintf(buf,"Hai ancora %d sessioni di pratica.\n\r",
						ch->specials.spells_to_learn);
				send_to_char(buf, ch);
				send_to_char("Puoi praticare questa skills:\n\r", ch);
				sprintf(buf,"[%d] %s %s \n\r",
						PRINCIPE,spells[SKILL_HIDE-1],
						how_good(ch->skills[SKILL_HIDE].learned));
				send_to_char(buf, ch);
				return(TRUE);
			}
			for(; isspace(*arg); arg++);
			number = old_search_block(arg,0,strlen(arg),spells,FALSE);
			if(number == -1) {
				send_to_char("Non conosco questa pratica...\n\r", ch);
				return(TRUE);
			}
			if(number !=SKILL_HIDE) {
				send_to_char("Non posso insegnarti questa pratica...\n\r", ch);
				return(TRUE);
			}
			if(ch->specials.spells_to_learn <= 0) {
				send_to_char("Non hai pratiche a disposizione.\n\r", ch);
				return(TRUE);
			}
			if(ch->skills[number].learned >= 45) {
				send_to_char("Non posso aiutarti a migliorati ancora.\n\r", ch);
				return(TRUE);
			}
			send_to_char("Hai fatto pratica...\n\r", ch);
			ch->specials.spells_to_learn--;


			if(!IS_SET(ch->skills[number].flags, SKILL_KNOWN)) {
				SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
				SET_BIT(ch->skills[number].flags, SKILL_KNOWN_THIEF);
			}

			ch->skills[ number ].learned += int_app[(int)GET_INT(ch) ].learn;

			if(ch->skills[number].learned >= 95) {
				send_to_char("Hai imparato tutto.\n\r", ch);
			}
			return TRUE;
		}
		/**** fine skills prince ****/

		if(cmd == CMD_GAIN) {
			if(GET_LEVEL(ch,THIEF_LEVEL_IND) >= GetMaxLevel(guildmaster)-10) {
				send_to_char("Devi cercare un altro maestro, io non posso piu' insegnarti niente al riguardo.\n\r",ch);
				return(TRUE);
			}

			if(GET_EXP(ch) <
					titles[THIEF_LEVEL_IND][GET_LEVEL(ch, THIEF_LEVEL_IND)+1].exp) {
				send_to_char("Non sei ancora in grado di apprendere questo.\n\r", ch);
				return(FALSE);
			}
			else {
				GainLevel(ch,THIEF_LEVEL_IND);
				return(TRUE);
			}
		} /* end gain */

		if(!arg || (strlen(arg) == 0)) {
			sprintf(buf,"Hai %d sessioni di pratica.\n\r",
					ch->specials.spells_to_learn);
			send_to_char(buf,ch);

			sprintf(buf," sneak           :  %s\n\r",how_good(ch->skills[SKILL_SNEAK].learned));
			send_to_char(buf,ch);
			sprintf(buf," hide            :  %s\n\r",how_good(ch->skills[SKILL_HIDE].learned));
			send_to_char(buf,ch);
			sprintf(buf," steal           :  %s\n\r",how_good(ch->skills[SKILL_STEAL].learned));
			send_to_char(buf,ch);
			sprintf(buf," backstab        :  %s\n\r",how_good(ch->skills[SKILL_BACKSTAB].learned));
			send_to_char(buf,ch);
			sprintf(buf," pick            :  %s\n\r",how_good(ch->skills[SKILL_PICK_LOCK].learned));
			send_to_char(buf,ch);
			sprintf(buf," spy             :  %s\n\r",how_good(ch->skills[SKILL_SPY].learned));
			send_to_char(buf,ch);
			sprintf(buf," retreat         :  %s\n\r",how_good(ch->skills[SKILL_RETREAT].learned));
			send_to_char(buf,ch);
			sprintf(buf," find trap       :  %s\n\r",how_good(ch->skills[SKILL_FIND_TRAP].learned));
			send_to_char(buf,ch);
			sprintf(buf," disarm trap     :  %s\n\r",how_good(ch->skills[SKILL_REMOVE_TRAP].learned));
			send_to_char(buf,ch);
			sprintf(buf," tspy            :  %s\n\r",how_good(ch->skills[SKILL_TSPY].learned));
			send_to_char(buf,ch);
			sprintf(buf," eavesdrop       :  %s\n\r",how_good(ch->skills[SKILL_EAVESDROP].learned));
			send_to_char(buf,ch);

			return(TRUE);
		}
		else {
			number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
			send_to_char("Il Maestro dei Ladri dice ",ch);

			if(number == -1) {
				send_to_char("'Non conosco questa abilita'.'\n\r", ch);
				return(TRUE);
			}

			switch(number) {
			case 0:
			case 1:
				sk_num = SKILL_SNEAK;
				break;
			case 2:
				sk_num = SKILL_HIDE;
				break;
			case 3:
				sk_num = SKILL_STEAL;
				break;
			case 4:
				sk_num = SKILL_BACKSTAB;
				break;
			case 5:
				sk_num = SKILL_PICK_LOCK;
				break;
			case 6:
				sk_num = SKILL_SPY;
				break;
			case 7:
				sk_num = SKILL_RETREAT;
				break;
			case 8:
				sk_num = SKILL_FIND_TRAP;
				break;
			case 9:
				sk_num = SKILL_REMOVE_TRAP;
				break;
			case 10:
				sk_num = SKILL_TSPY;
				break;
			case 11:
				sk_num = SKILL_EAVESDROP;
				break;

			default:
				mudlog(LOG_SYSERR, "Strangeness in Thief Guildmaster (%d)", number);
				send_to_char("'Ack!  Mi sento male!'\n\r", ch);
				return(TRUE);
			} /* end switch */

			if(ch->skills[sk_num].learned > 45
					&&sk_num!=SKILL_RETREAT
					&&sk_num!=SKILL_TSPY
					&&sk_num!=SKILL_EAVESDROP) {
				send_to_char("'You must learn from practice and experience now.'\n\r", ch);
				return(TRUE);
			}

			if(ch->specials.spells_to_learn <= 0) {
				send_to_char
				("'You must first use the knowledge you already have.'\n\r",ch);
				return(FALSE);
			}

			send_to_char("'This is how you do it...'\n\r",ch);
			ch->specials.spells_to_learn--;

			if(!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN)) {
				SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
				SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN_THIEF);
			}

			ch->skills[ sk_num ].learned += int_app[(int)GET_INT(ch) ].learn;

			if(ch->skills[sk_num].learned >= 95) {
				send_to_char("'You are now a master of this art.'\n\r", ch);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(WarriorGuildMaster) {
	char buf[256];
	struct char_data* guildmaster;
	const static char* n_skills[] = {
		"kick",     /* 1 */
		"bash",
		"rescue",
		"skin",
		"bellow",
		"\n",
	};
	int number=0;
	int sk_num;

	if(!AWAKE(ch)) {
		return(FALSE);
	}

	if(!cmd) {
		if(ch->specials.fighting) {
			return(fighter(ch, cmd, arg, ch, 0));
		}
		return(FALSE);
	}

	if(!ch->skills) {
		return(FALSE);
	}

	if(check_soundproof(ch)) {
		return(FALSE);
	}

	guildmaster = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(WarriorGuildMaster));

	for(; *arg==' '; arg++); /* ditch spaces */
	if(cmd==CMD_PRACTICE || cmd==CMD_GAIN) {
		if(IS_NPC(ch)) {
			act("$N tells you 'What do i look like, an animal trainer?'", FALSE,
				ch, 0, guildmaster, TO_CHAR);
			return(FALSE);
		}
		/**** SALVO skills prince            VVVVVVVVVVVVVVVVV ****/
		if(!HasClass(ch, CLASS_WARRIOR) && !IS_PRINCE(ch)) {
			send_to_char("The Warrior Guildmaster says 'Get out of here!'\n\r",ch);
			return(TRUE);
		}
		/**** SALVO skills prince ****/
		else if(IS_PRINCE(ch) && !HasClass(ch, CLASS_WARRIOR) && cmd !=CMD_GAIN) {
			if(!*arg) {
				sprintf(buf,"Hai ancora %d sessioni di pratica.\n\r",
						ch->specials.spells_to_learn);
				send_to_char(buf, ch);
				send_to_char("Puoi praticare questa skills:\n\r", ch);
				sprintf(buf,"[%d] %s %s \n\r",
						PRINCIPE,spells[SKILL_RESCUE-1],
						how_good(ch->skills[SKILL_RESCUE].learned));
				send_to_char(buf, ch);
				return(TRUE);
			}
			for(; isspace(*arg); arg++);
			number = old_search_block(arg,0,strlen(arg),spells,FALSE);
			if(number == -1) {
				send_to_char("Non conosco questa pratica...\n\r", ch);
				return(TRUE);
			}
			if(number !=SKILL_RESCUE) {
				send_to_char("Non posso insegnarti questa pratica...\n\r", ch);
				return(TRUE);
			}
			if(ch->specials.spells_to_learn <= 0) {
				send_to_char("Non hai pratiche a disposizione.\n\r", ch);
				return(TRUE);
			}
			if(ch->skills[number].learned >= 45) {
				send_to_char("Non posso aiutarti a migliorati ancora.\n\r", ch);
				return(TRUE);
			}
			send_to_char("Hai fatto pratica...\n\r", ch);
			ch->specials.spells_to_learn--;


			if(!IS_SET(ch->skills[number].flags, SKILL_KNOWN)) {
				SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
				SET_BIT(ch->skills[number].flags, SKILL_KNOWN_WARRIOR);
			}

			ch->skills[ number ].learned += int_app[(int)GET_INT(ch) ].learn;

			if(ch->skills[number].learned >= 95) {
				send_to_char("Hai imparato tutto.\n\r", ch);
			}
			return TRUE;
		}
		/**** fine skills prince ****/

		if(cmd==CMD_GAIN) {
			if(GET_LEVEL(ch,WARRIOR_LEVEL_IND) >= GetMaxLevel(guildmaster)-10) {
				send_to_char("You must learn from another, I can no longer train you.\n\r",ch);
				return(TRUE);
			}

			if(GET_EXP(ch)<
					titles[WARRIOR_LEVEL_IND][GET_LEVEL(ch, WARRIOR_LEVEL_IND)+1].exp) {
				send_to_char("You are not yet ready to gain.\n\r", ch);
				return(FALSE);
			}
			else {
				GainLevel(ch,WARRIOR_LEVEL_IND);
				return(TRUE);
			}
		} /* end gain */


		if(!arg || (strlen(arg) == 0)) {
			sprintf(buf,"You have got %d practice sessions left.\n\r",
					ch->specials.spells_to_learn);
			send_to_char(buf,ch);

			sprintf(buf," kick          :  %s\n\r",how_good(ch->skills[SKILL_KICK].learned));
			send_to_char(buf,ch);
			sprintf(buf," bash          :  %s\n\r",how_good(ch->skills[SKILL_BASH].learned));
			send_to_char(buf,ch);
			sprintf(buf," rescue        :  %s\n\r",how_good(ch->skills[SKILL_RESCUE].learned));
			send_to_char(buf,ch);
			sprintf(buf," skin          :  %s\n\r",how_good(ch->skills[SKILL_TAN].learned));
			send_to_char(buf,ch);
			sprintf(buf," bellow        :  %s\n\r",how_good(ch->skills[SKILL_BELLOW].learned));
			send_to_char(buf,ch);
			return(TRUE);
		}
		else {
			number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
			send_to_char("The Warrior Guildmaster says ",ch);

			if(number == -1) {
				send_to_char("'I do not know of this skill.'\n\r", ch);
				return(TRUE);
			}

			switch(number) {
			case 0:
			case 1:
				sk_num = SKILL_KICK;
				break;
			case 2:
				sk_num = SKILL_BASH;
				break;
			case 3:
				sk_num = SKILL_RESCUE;
				break;
			case 4:
				sk_num = SKILL_TAN;
				break;
			case 5:
				sk_num = SKILL_BELLOW;
				break;

			default:
				mudlog(LOG_SYSERR, "Strangeness in Warrior Guildmaster (%d)", number);
				send_to_char("'Ack!  I feel sick!'\n\r", ch);
				return(TRUE);
			} /* end switch */

			if(ch->skills[sk_num].learned > 45) {
				send_to_char("'You must learn from practice and experience now.'\n\r", ch);
				return(TRUE);
			}

			if(ch->specials.spells_to_learn <= 0) {
				send_to_char("'You must first use the knowledge you already have.'\n\r",ch);
				return(FALSE);
			}

			send_to_char("'This is how you do it...'\n\r",ch);
			ch->specials.spells_to_learn--;

			if(!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN)) {
				SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
				SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN_WARRIOR);
			}

			ch->skills[ sk_num ].learned += int_app[(int)GET_INT(ch) ].learn;

			if(ch->skills[sk_num].learned >= 95) {
				send_to_char("'You are now a master of this art.'\n\r", ch);
			}
			return(TRUE);
		}
	}
	else {
		return(FALSE);
	}
}


ROOMSPECIAL_FUNC(dump) {
	struct obj_data* k;
	char buf[100];
	struct char_data* tmp_char;
	int value=0;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	for(k = real_roomp(ch->in_room)->contents; k ;
			k = real_roomp(ch->in_room)->contents) {
		sprintf(buf, "The %s vanish in a puff of smoke.\n\r",fname(k->name));
		for(tmp_char = real_roomp(ch->in_room)->people; tmp_char;
				tmp_char = tmp_char->next_in_room)
			if(CAN_SEE_OBJ(tmp_char, k)) {
				send_to_char(buf,tmp_char);
			}
		extract_obj(k);
	}

	if(cmd!=CMD_DROP) {
		return(FALSE);
	}

	do_drop(ch, arg, cmd);

	value = 0;

	for(k = real_roomp(ch->in_room)->contents; k ;
			k = real_roomp(ch->in_room)->contents) {
		sprintf(buf, "The %s vanish in a puff of smoke.\n\r",fname(k->name));
		for(tmp_char = real_roomp(ch->in_room)->people; tmp_char;
				tmp_char = tmp_char->next_in_room) {
			if(CAN_SEE_OBJ(tmp_char, k)) {
				send_to_char(buf,tmp_char);
			}
		}
		value+=(MIN(1000,MAX(k->obj_flags.cost/4,1)));
		extract_obj(k);
	}

	if(value) {
		act("You are awarded for outstanding performance.", FALSE, ch, 0, 0,
			TO_CHAR);
		act("$n has been awarded for being a good citizen.", TRUE, ch, 0,0,
			TO_ROOM);

		if(GetMaxLevel(ch) < 3) {
			gain_exp(ch, MIN(100, value));
		}
		else {
			GET_GOLD(ch) += value;
		}
	}
	return TRUE;
}

MOBSPECIAL_FUNC(mayor) {
	static char open_path[] =
		"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

	static char close_path[] =
		"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

	static char* path;
	static int index;
	static bool move = FALSE;

	mob->lStartRoom = 0;

	if(type == EVENT_WINTER) {
		GET_POS(ch) = POSITION_STANDING;
		do_shout(ch, "Aieee!   The rats!  The rats are coming!  Aieeee!",0);
		return(TRUE);
	}


	if(!move) {
		if(time_info.hours == 6) {
			move = TRUE;
			path = open_path;
			index = 0;
		}
		else if(time_info.hours == 20) {
			move = TRUE;
			path = close_path;
			index = 0;
		}
	}

	if(cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
			(GET_POS(ch) == POSITION_FIGHTING)) {
		if(!ch->specials.fighting) {
			return(FALSE);
		}
		return(fighter(ch, cmd, arg, mob, type));
	}

	switch(path[index]) {
	case '0' :
	case '1' :
	case '2' :
	case '3' :
		do_move(ch,"",path[index]-'0'+1);
		break;

	case 'W' :
		GET_POS(ch) = POSITION_STANDING;
		act("$n awakens and groans loudly.",FALSE,ch,0,0,TO_ROOM);
		break;

	case 'S' :
		GET_POS(ch) = POSITION_SLEEPING;
		act("$n lies down and instantly falls asleep.",FALSE,ch,0,0,TO_ROOM);
		break;

	case 'a' :
		if(check_soundproof(ch)) {
			return(FALSE);
		}
		act("$n says 'Hello Honey!'",FALSE,ch,0,0,TO_ROOM);
		act("$n smirks.",FALSE,ch,0,0,TO_ROOM);
		break;

	case 'b' :
		if(check_soundproof(ch)) {
			return(FALSE);
		}
		act("$n says 'What a view! I must get something done about that dump!'",
			FALSE,ch,0,0,TO_ROOM);
		break;

	case 'c' :
		if(check_soundproof(ch)) {
			return(FALSE);
		}
		act("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
			FALSE,ch,0,0,TO_ROOM);
		break;

	case 'd' :
		if(check_soundproof(ch)) {
			return(FALSE);
		}
		act("$n says 'Good day, citizens!'", FALSE, ch, 0,0,TO_ROOM);
		break;

	case 'e' :
		if(check_soundproof(ch)) {
			return(FALSE);
		}
		act("$n says 'I hereby declare the bazaar open!'",FALSE,ch,0,0,TO_ROOM);
		break;

	case 'E' :
		if(check_soundproof(ch)) {
			return(FALSE);
		}
		act("$n says 'I hereby declare ShadowSpring closed!'",FALSE,ch,0,0,TO_ROOM);
		break;

	case 'O' :
		do_unlock(ch, "gate", 0);
		do_open(ch, "gate", 0);
		break;

	case 'C' :
		do_close(ch, "gate", 0);
		do_lock(ch, "gate", 0);
		break;

	case '.' :
		move = FALSE;
		break;

	}

	index++;
	return FALSE;
}

#define ACT_OVER_21 1
#define ACT_SNICKER 2

struct pub_beers {
	int        container;
	int   contains;
	int   howman;
	int   actflag;
};

struct pub_beers sold_here[] = {
	{3903, 3902, 6, 1 },
	{3905, 3904, 6, 1 },
	{3907, 3906, 6, 1 },
	{3909, 3908, 6, 3 },
	{3911, 3910, 6, 3 },
	{3913, 3912, 6, 3 },
	{3914, 0, 0, 1 },
	{3930, 0, 0, 0 },
	{3931, 0, 0, 0 },
	{3932, 0, 0, 0 },
	{3102, 0, 0, 0 },
	{-1,0,0,0}
};


MOBSPECIAL_FUNC(andy_wilcox)

{
	/* things you MUST change if you install this on another mud:
	 * THE_PUB, room number of the Pub where he will sell beer.
	 * sold_here, object numbers of the containers and the
	 * corresponding beer that they will contain.  If you don't
	 * have multi-buy code, change the #if 1 to #if 0.
	 * */
#define THE_PUB        3940
	static int        open=1; /* 0 closed;  1 open;  2 last call */
	char argm[100], newarg[100], buf[MAX_STRING_LENGTH];
	struct obj_data* temp1, *temp2;
	struct char_data* temp_char;
	struct char_data* andy;
	int num, i, cost;
	struct pub_beers* scan;


	andy = 0;

	if(check_soundproof(ch)) {
		return(FALSE);
	}
	for(temp_char = real_roomp(ch->in_room)->people; (!andy) && (temp_char) ;
			temp_char = temp_char->next_in_room)
		if(IS_MOB(temp_char))
			if(mob_index[temp_char->nr].func == reinterpret_cast<genericspecial_func>(andy_wilcox)) {
				andy = temp_char;
			}
	if(open==0 && time_info.hours == 11) {
		open = 1;
		do_unlock(andy, "door", 0);
		do_open(andy, "door", 0);
		act("$n says 'We're open for lunch, come on in.'", FALSE, andy, 0,0, TO_ROOM);
	}
	if(open==1 && time_info.hours == 1) {
		open = 2;
		act("$n says 'Last call, guys and gals.'", FALSE, andy, 0,0, TO_ROOM);
	}
	if(open==2 && time_info.hours == 2) {
		open = 0;
		act("$n says 'We're closing for the night.\n  Thanks for coming, all, and come again!'", FALSE, andy, 0,0, TO_ROOM);
		do_close(andy, "door", 0);
		do_lock(andy, "door", 0);
	}

	switch(cmd) {
	case CMD_KILL:
	case CMD_HIT:
	case CMD_BASH:
	case CMD_KICK:
	case CMD_DISARM:
		only_argument(arg, argm);

		if(andy == ch) {
			return TRUE;
		}
		if(andy == get_char_room(argm, ch->in_room)) {
			int        hitsleft;
			act("$n says 'Get this, $N wants to kill me', and\n falls down laughing.", FALSE, andy, 0, ch, TO_ROOM);
			hitsleft = dice(2,6) + 6;
			if(hitsleft < GET_HIT(ch) && GetMaxLevel(ch) <= MAX_MORT) {
				act("$n beats the shit out of $N.", FALSE, andy, 0, ch, TO_NOTVICT);
				act("$n beats the shit out of you.  OUCH!", FALSE, andy, 0, ch, TO_VICT);
				GET_HIT(ch) = hitsleft;
				alter_hit(ch,0);
			}
			else {
				act("$n grabs $N in a vicious sleeper hold.", FALSE, andy, 0, ch, TO_NOTVICT);
				act("$n puts you in a vicious sleeper hold.", FALSE, andy, 0, ch, TO_VICT);
			}
			GET_POS(ch) = POSITION_SLEEPING;
		}
		else {
			do_action(andy, ch->player.name, 130 /* slap */);
			act("$n says 'Hey guys, I run a quiet pub.  Take it outside.'",
				FALSE, andy, 0, 0, TO_ROOM);
		}
		return TRUE;
		break;

	case CMD_STEAL:
		if(andy == ch) {
			return TRUE;
		}
		do_action(andy, ch->player.name, 130 /* slap */);
		act("$n tells you 'Who the hell do you think you are?'",
			FALSE, andy, 0, ch, TO_VICT);
		do_action(andy, ch->player.name, 116 /* glare */);
		return TRUE;
		break;

	case CMD_CAST:
	case CMD_RECITE:
	case CMD_USE:
		if(andy == ch) {
			return TRUE;
		}
		do_action(andy, ch->player.name, 94 /* poke */);
		act("$n tells you 'Hey, no funny stuff.'.", FALSE, andy, 0, ch, TO_VICT);
		return TRUE;
		break;

	case CMD_BUY:
		if(ch->in_room != THE_PUB) {
			act("$n tells you 'Hey man, I'm on my own time, but stop by the Pub some time.'", FALSE, andy, 0, ch, TO_VICT);
			return TRUE;
		}
		if(open==0) {
			act("$n tells you 'Sorry, we're closed, come back for lunch.'",
				FALSE, andy, 0, ch, TO_VICT);
			return TRUE;
		}
		only_argument(arg, argm);
		if(!(*argm)) {
			act("$n tells you 'Sure, what do you want to buy?'",
				FALSE, andy, 0, ch, TO_VICT);
			return TRUE;
		}

#if 1
		/* multiple buy code */
		if((num = getabunch(argm, newarg)) != 0) {
			strcpy(argm,newarg);
		}
		if(num == 0) {
			num = 1;
		}
#endif

		if(!(temp1 = get_obj_in_list_vis(ch,argm,andy->carrying)))      {
			act("$n tells you 'Sorry, but I don't sell that.'", FALSE, andy, 0, ch, TO_VICT);
			return TRUE;
		}
		for(scan = sold_here; scan->container>=0; scan++) {
			if(temp1->item_number >= 0 &&
					scan->container == obj_index[temp1->item_number].iVNum) {
				break;
			}
		}
		if(scan->container<0)      {
			act("$n tells you 'Sorry, that's not for sale.'", FALSE, andy, 0, ch, TO_VICT);
			return TRUE;
		}

		if(scan->actflag&ACT_OVER_21 && GET_AGE(ch)<21) {
			if(IS_IMMORTAL(ch) || HasClass(ch, CLASS_THIEF)) {
				act("$N manages to slip a fake ID past $n.",
					FALSE, andy, 0, ch, TO_NOTVICT);
			}
			else if(!IS_NPC(ch)) {
				act("$n tells you 'Sorry, I could lose my license if I served you alcohol.'", FALSE, andy, 0, ch, TO_VICT);
				act("$n cards $N and $N is BUSTED.", FALSE, andy, 0, ch, TO_NOTVICT);
				return TRUE;
			}
		}

		temp2 = read_object(scan->contains, VIRTUAL);
		cost = (temp2 ? (scan->howman * temp2->obj_flags.cost) : 0)
			   + temp1->obj_flags.cost;
		cost *= 9;
		cost /=10;
		cost++;
		if(temp2) {
			extract_obj(temp2);
		}

		for(; num>0; num--) {
			if(GET_GOLD(ch) < cost) {
				act("$n tells you 'Sorry, man, no bar tabs.'",
					FALSE, andy, 0, ch, TO_VICT);
				return TRUE;
			}
			temp1 = read_object(temp1->item_number, REAL);
			for(i=0; i<scan->howman; i++) {
				temp2 = read_object(scan->contains, VIRTUAL);
				obj_to_obj(temp2, temp1);
			}
			obj_to_char(temp1, ch);
			GET_GOLD(ch) -= cost;
			act("$N buys a $p from $n", FALSE, andy, temp1, ch, TO_NOTVICT);
			if(scan->actflag&ACT_SNICKER) {
				act("$n snickers softly.", FALSE, andy, NULL, ch, TO_ROOM);
			}
			else {
				act((scan->actflag&ACT_OVER_21) ?
					"$n tells you 'Drink in good health' and gives you $p" :
					"$n tells you 'Enjoy' and gives you $p",
					FALSE, andy, temp1, ch, TO_VICT);
			}
		}
		return TRUE;
		break;

	case CMD_LIST:
		act("$n says 'We have", FALSE, andy, NULL, ch, TO_VICT);
		for(scan = sold_here; scan->container>=0; scan++) {
			temp1 = read_object(scan->container, VIRTUAL);
			temp2 = scan->contains ? read_object(scan->contains, VIRTUAL) : NULL;
			cost = (temp2 ? (scan->howman * temp2->obj_flags.cost) : 0)
				   + temp1->obj_flags.cost;
			cost *= 9;
			cost /=10;
			cost++;
			sprintf(buf,"%s for %d gold coins.\n\r", temp1->short_description, cost);
			send_to_char(buf, ch);
			extract_obj(temp1);
			if(temp2) {
				extract_obj(temp2);
			}
		}
		return TRUE;
		break;
	}

	return FALSE;
}

struct char_data* find_mobile_here_with_spec_proc(genericspecial_func fcn, int rnumber) {

	return(FindMobInRoomWithFunction(rnumber,fcn));
}

MOBSPECIAL_FUNC(eric_johnson)

{
	/* if more than one eric johnson exists in a game, it will
	   get confused because of the state variables */
#define E_HACKING        0
#define E_SLEEPING        1
#define E_SHORT_BEER_RUN 2
#define E_LONG_BEER_RUN        3
#define E_STOCK_FRIDGE        4
#define E_SKYDIVING        5
#define Erics_Lair        3941
#define DanjerKitchen        3904
#define DanjerLiving        3901
#define DanjerPorch        3900
	static int fighting=0, state=E_HACKING;
	struct obj_data* temp1;
	struct char_data* eric, *temp_char;
	char buf[100];

	eric = 0;

	if(check_soundproof(ch)) {
		return(FALSE);
	}

	eric = FindMobInRoomWithFunction(ch->in_room,reinterpret_cast<genericspecial_func>(eric_johnson));
	for(temp_char = real_roomp(ch->in_room)->people; (!eric) && (temp_char) ;
			temp_char = temp_char->next_in_room)
		if(IS_MOB(temp_char))
			if((void*)mob_index[temp_char->nr].func == (void*)eric_johnson) {
				eric = temp_char;
			}

	if(ch==eric) {

		if(cmd!=0) {
			return FALSE;    /* prevent recursion when eric wants to move */
		}

		if(!fighting && ch->specials.fighting) {
			act("$n says 'What the fuck?'", FALSE, eric, 0, 0, TO_ROOM);
			fighting=1;
		}
		if(fighting && !ch->specials.fighting) {
			act("$n says 'I wonder what their problem was.'", FALSE, eric, 0, 0, TO_ROOM);
			fighting=0;
		}
		if(fighting) {
			struct char_data*        target = eric->specials.fighting;
			act("$n yells for help.", FALSE, eric, 0, 0, TO_ROOM);
			act("$n throws some nasty judo on $N.", FALSE, eric, 0, target, TO_NOTVICT);
			act("$n throws some nasty judo on you.", FALSE, eric, 0, target, TO_VICT);
			if(damage(eric, target, dice(2,4), TYPE_HIT, 5) != VictimDead) {
				if(!saves_spell(target, SAVING_SPELL)) {
					struct affected_type af;
					af.type = SPELL_SLEEP;
					af.duration = 2;
					af.modifier = 0;
					af.location = APPLY_NONE;
					af.bitvector = AFF_SLEEP;
					affect_join(target, &af, FALSE, FALSE);
					if(target->specials.fighting) {
						stop_fighting(target);
					}
					if(eric->specials.fighting) {
						stop_fighting(eric);
					}
					act("$N is out cold.", FALSE, eric, 0, target, TO_NOTVICT);
					act("You are out cold.", FALSE, eric, 0, target, TO_VICT);
					GET_POS(target) = POSITION_SLEEPING;
					RemHated(eric, target);
					RemHated(target, eric);
				}
			}
			return FALSE;
		}

		switch(state) {
		case E_HACKING:
			if(GET_POS(eric)==POSITION_SLEEPING) {
				do_wake(eric, "", -1);
				return TRUE;
			}
			break;
		case E_SLEEPING:
			if(GET_POS(eric)!=POSITION_SLEEPING) {
				act("$n says 'Go away, I'm sleeping'", FALSE, eric, 0,0, TO_ROOM);
				do_sleep(eric, "", -1);
				return TRUE;
			}
			break;
		default:
			if(GET_POS(eric)==POSITION_SLEEPING) {
				do_wake(eric, "", -1);
				return TRUE;
			}
			else if(GET_POS(eric)!=POSITION_STANDING) {
				do_stand(eric, "", -1);
				return TRUE;
			}
			break;
		}
		const char* s="";
		switch(state) {
		case E_SLEEPING:
			if(time_info.hours>9 && time_info.hours<12) {
				do_wake(eric, "", -1);
				act("$n says 'Ahh, that was a good night's sleep'", FALSE, eric,
					0,0, TO_ROOM);
				state = E_HACKING;
				return TRUE;
			}
			return TRUE;
			break;
		case E_HACKING:
			if(eric->in_room != Erics_Lair) {
				/* he's not in his lair, get him there. */
				int dir;
				if(eric->in_room == DanjerLiving) {
					do_close(eric, "front",0);
					do_lock(eric, "front",0);
				}
				dir = choose_exit_global(eric->in_room, Erics_Lair, -100);
				if(dir<0) {
					if(eric->in_room == DanjerPorch) {
						do_unlock(eric, "front",0);
						do_open(eric, "front",0);
						return TRUE;
					}
					dir = choose_exit_global(eric->in_room, DanjerPorch, -100);
				}
				if(dir<0) {
					if(dice(1,2)==1) {
						act("$n says 'Shit, I'm totally lost.", FALSE, eric, 0,0,TO_ROOM);
					}
					else
						act("$n says 'Can you show me the way back to the DanjerHaus?'",
							FALSE, eric, 0,0, TO_ROOM);
				}
				else {
					go_direction(eric, dir);
				}

			}
			else {
				if(time_info.hours>22 || time_info.hours<3) {
					state = E_SLEEPING;
					do_sleep(eric, 0, -1);
					return TRUE;
				}

				do_sit(eric, "", -1);
				if(3==dice(1,5)) {
					/* he's in his lair, do lair things */
					switch(dice(1,5)) {
					case 1:
						s = "$n looks at you, then resumes hacking";
						break;
					case 2:
						s = "$n swears at the terminal and resumes hacking";
						break;
					case 3:
						s = "$n looks around and says 'Where's Big Guy?'";
						break;
					case 4:
						s = "$n says 'Dude, RS/6000s suck.'";
						break;
					case 5:
						temp1 = get_obj_in_list_vis(eric, "beer", eric->carrying);
						if(temp1==NULL ||
								temp1->obj_flags.type_flag != ITEM_DRINKCON ||
								temp1->obj_flags.value[1] <= 0) {
							s = "$n says 'Damn, out of beer'";
							do_stand(eric, "", -1);
							state = E_SHORT_BEER_RUN;
						}
						else {
							do_drink(eric, "beer", -1 /* irrelevant */);
							s = "$n licks his lips";
						}
						break;
					default:
						s = "$n says 'Questo e' impossibile!'";
						break;
					}
					act(s, FALSE, eric, 0, 0, TO_ROOM);
				}
			}
			break;
		case E_SHORT_BEER_RUN:
			if(eric->in_room != DanjerKitchen) {
				int        dir;
				dir = choose_exit_global(eric->in_room, DanjerKitchen, -100);
				if(dir<0) {
					if(dice(1,3)!=1)
						act("$n says 'Dammit, where's the beer?",
							FALSE, eric, 0,0,TO_ROOM);
					else
						act("$n says 'Christ, who stole my kitchen?'",
							FALSE, eric, 0,0, TO_ROOM);
				}
				else {
					go_direction(eric, dir);
				}
			}
			else {
				/* we're in the kitchen, find beer */
				temp1 = get_obj_in_list_vis(eric, "fridge",
											real_roomp(eric->in_room)->contents);
				if(temp1==NULL) {
					act("$n says 'Alright, who stole my refrigerator!'", FALSE, eric,
						0, 0, TO_ROOM);
				}
				else if(IS_SET(temp1->obj_flags.value[1], CONT_CLOSED)) {
					do_drop(eric, "bottle", -1 /* irrelevant */);
					do_open(eric, "fridge", -1 /* irrelevant */);
				}
				else if(NULL == (temp1 = get_obj_in_list_vis(eric, "sixpack",
										 eric->carrying))) {
					strcpy(buf, "get sixpack fridge");
					command_interpreter(eric, buf);
					if(NULL == get_obj_in_list_vis(eric, "sixpack",
												   eric->carrying)) {
						act("$n says 'Aw, man.  Someone's been drinking all the beer.",
							FALSE, eric, 0, 0, TO_ROOM);
						do_close(eric, "fridge", -1 /* irrelevant */);
						state = E_LONG_BEER_RUN;
					}
				}
				else if(NULL == (temp1 = get_obj_in_list_vis(eric, "beer",
										 eric->carrying))) {
					strcpy(buf, "get beer sixpack");
					command_interpreter(eric, buf);
					if(NULL == get_obj_in_list_vis(eric, "beer",
												   eric->carrying)) {
						act("$n says 'Well, that one's finished...'", FALSE, eric,
							0, 0, TO_ROOM);
						do_drop(eric, "sixpack", -1 /* irrelevant */);
					}
				}
				else {
					strcpy(buf, "put sixpack fridge");
					command_interpreter(eric, buf);
					do_close(eric, "fridge", -1 /* irrelevant */);
					state = E_HACKING;
				}
			}
			break;
		case E_LONG_BEER_RUN: {
			static struct char_data* andy = 0;
			int        dir;
			static const char** scan;
			static const char* shopping_list[] =
			{ "guinness", "harp", "sierra", "2.harp", NULL };

			for(temp_char = character_list; temp_char; temp_char = temp_char->next)
				if(IS_MOB(temp_char))
					if((void*)mob_index[temp_char->nr].func == (void*)andy_wilcox) {
						andy = temp_char;
					}

			if(eric->in_room != andy->in_room) {
				if(eric->in_room == DanjerPorch) {
					do_close(eric, "front",0);
					do_lock(eric, "front",0); /* this takes no time */
				}
				else if(eric->in_room == DanjerLiving) {
					do_unlock(eric, "front",0);
					do_open(eric, "front",0);
					return TRUE; /* this takes one turn */
				}
				dir = choose_exit_global(eric->in_room, andy->in_room, -100);
				if(dir<0) {
					dir = choose_exit_global(eric->in_room, DanjerLiving, -100);
				}
				if(dir<0) {
					act("$n says 'Aw, man.  Where am I going to get more beer?",
						FALSE, eric, 0,0, TO_ROOM);
					state = E_HACKING;
				}
				else {
					go_direction(eric, dir);
				}
			}
			else {
				for(scan = shopping_list; *scan; scan++) {
					if(NULL == get_obj_in_list_vis(eric, *scan,
												   eric->carrying)) {
						const char* search;
						search = (scan[0][1] == '.') ? scan[0]+2 : scan[0];
						sprintf(buf, "buy %s", search);
						command_interpreter(eric, buf);
						if(NULL == get_obj_in_list_vis(eric, *scan,
													   eric->carrying)) {
							act("$n says 'ARGH, where's my deadbeat roommate with the rent.'", FALSE, eric, 0,0, TO_ROOM);
							act("$n says 'I need beer money.'", FALSE, eric, 0,0, TO_ROOM);
							state = (scan==shopping_list) ? E_HACKING : E_STOCK_FRIDGE;
							return TRUE;
						}
						break;
					}
				}
				if(*scan==NULL || 1 == dice(1,4)) {
					act("$n says 'Catch you later, dude.'", FALSE, eric, 0,0, TO_ROOM);
					state = E_STOCK_FRIDGE;
				}
			}
		}
		break;
		case E_STOCK_FRIDGE:
			if(eric->in_room != DanjerKitchen) {
				int        dir;
				if(eric->in_room == DanjerLiving) {
					do_close(eric, "front",0);
					do_lock(eric, "front",0);
				}
				dir = choose_exit_global(eric->in_room, DanjerKitchen, -100);
				if(dir<0) {
					if(eric->in_room == DanjerPorch) {
						do_unlock(eric, "front",0);
						do_open(eric, "front",0);
						return TRUE;
					}
					dir = choose_exit_global(eric->in_room, DanjerPorch, -100);
				}
				if(dir<0) {
					if(dice(1,3)!=1)
						act("$n says 'Dammit, where's the fridge?",
							FALSE, eric, 0,0,TO_ROOM);
					else
						act("$n says 'Christ, who stole my kitchen?'",
							FALSE, eric, 0,0, TO_ROOM);
				}
				else {
					go_direction(eric, dir);
				}
			}
			else {
				/* we're in the kitchen, find beer */
				temp1 = get_obj_in_list_vis(eric, "fridge",
											real_roomp(eric->in_room)->contents);
				if(temp1==NULL) {
					act("$n says 'Alright, who stole my refrigerator!'", FALSE, eric,
						0, 0, TO_ROOM);
				}
				else if(IS_SET(temp1->obj_flags.value[1], CONT_CLOSED)) {
					do_open(eric, "fridge", -1 /* irrelevant */);
				}
				else if(NULL == (temp1 = get_obj_in_list_vis(eric, "beer",
										 eric->carrying))) {
					strcpy(buf, "get beer sixpack");
					command_interpreter(eric, buf);
					if(NULL == get_obj_in_list_vis(eric, "beer",
												   eric->carrying)) {
						act("$n says 'What the hell, I just bought this?!'", FALSE, eric,
							0, 0, TO_ROOM);
						do_drop(eric, "sixpack", -1 /* irrelevant */);
						if(NULL == get_obj_in_list_vis(eric, "sixpack", eric->carrying)) {
							state = E_HACKING;
						}
					}
				}
				else {
					strcpy(buf, "put all.sixpack fridge");
					command_interpreter(eric, buf);
					do_close(eric, "fridge", -1 /* irrelevant */);
					state = E_HACKING;
				}
			}
			break;
		}
	}

	return FALSE;
}

/* *******************************************************************
*  General special procedures for mobiles                            *
******************************************************************** */

/* SOCIAL GENERAL PROCEDURES

If first letter of the command is '!' this will mean that the following
command will be executed immediately.

"G",n      : Sets next line to n
"g",n      : Sets next line relative to n, fx. line+=n
"m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
"w",n      : Wake up and set standing (if possible)
"c<txt>",n : Look for a person named <txt> in the room
"o<txt>",n : Look for an object named <txt> in the room
"r<int>",n : Test if the npc in room number <int>?
"s",n      : Go to sleep, return false if can't go sleep
"e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
             contents of the **thing
"E<txt>",n : Send <txt> to person pointed to by thing
"B<txt>",n : Send <txt> to room, except to thing
"?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
             Will as usual advance one line upon sucess, and change
             relative n lines upon failure.
"O<txt>",n : Open <txt> if in sight.
"C<txt>",n : Close <txt> if in sight.
"L<txt>",n : Lock <txt> if in sight.
"U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
void exec_social(struct char_data* npc, char* cmd, int next_line,
				 int* cur_line, void** thing) {
	bool ok;

	if(GET_POS(npc) == POSITION_FIGHTING) {
		return;
	}

	ok = TRUE;

	switch(*cmd) {

	case 'G' :
		*cur_line = next_line;
		return;

	case 'g' :
		*cur_line += next_line;
		return;

	case 'e' :
		act(cmd+1, FALSE, npc, (struct obj_data*)*thing, *thing, TO_ROOM);
		break;

	case 'E' :
		act(cmd+1, FALSE, npc, 0, *thing, TO_VICT);
		break;

	case 'B' :
		act(cmd+1, FALSE, npc, 0, *thing, TO_NOTVICT);
		break;

	case 'm' :
		do_move(npc, "", *(cmd+1)-'0'+1);
		break;

	case 'w' :
		if(GET_POS(npc) != POSITION_SLEEPING) {
			ok = FALSE;
		}
		else {
			GET_POS(npc) = POSITION_STANDING;
		}
		break;

	case 's' :
		if(GET_POS(npc) <= POSITION_SLEEPING) {
			ok = FALSE;
		}
		else {
			GET_POS(npc) = POSITION_SLEEPING;
		}
		break;

	case 'c' :  /* Find char in room */
		*thing = get_char_room_vis(npc, cmd+1);
		ok = (*thing != 0);
		break;

	case 'o' : /* Find object in room */
		*thing = get_obj_in_list_vis(npc, cmd+1, real_roomp(npc->in_room)->contents);
		ok = (*thing != 0);
		break;

	case 'r' : /* Test if in a certain room */
		ok = (npc->in_room == atoi(cmd+1));
		break;

	case 'O' : /* Open something */
		do_open(npc, cmd+1, 0);
		break;

	case 'C' : /* Close something */
		do_close(npc, cmd+1, 0);
		break;

	case 'L' : /* Lock something  */
		do_lock(npc, cmd+1, 0);
		break;

	case 'U' : /* UnLock something  */
		do_unlock(npc, cmd+1, 0);
		break;

	case '?' : /* Test a random number */
		if(atoi(cmd+1) <= number(1,100)) {
			ok = FALSE;
		}
		break;

	default:
		break;
	}  /* End Switch */

	if(ok) {
		(*cur_line)++;
	}
	else {
		(*cur_line) += next_line;
	}
}



void npc_steal(struct char_data* ch,struct char_data* victim) {
	int gold;

	if(IS_NPC(victim)) {
		return;
	}
	if(GetMaxLevel(victim)>MAX_MORT) {
		return;
	}

	if(AWAKE(victim) && (number(0,GetMaxLevel(ch)) == 0)) {
		act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
		act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
	}
	else {
		/* Steal some gold coins */
		gold = (int)((GET_GOLD(victim)*number(1,10))/100);
		if(gold > 0) {
			GET_GOLD(ch) += gold;
			GET_GOLD(victim) -= gold;
		}
	}
}


MOBSPECIAL_FUNC(snake) {

	if(type != EVENT_TICK || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch) == POSITION_SITTING) {
		if(ch->specials.fighting) {
			GET_POS(ch) = POSITION_FIGHTING;
		}
		else {
			GET_POS(ch) = POSITION_STANDING;
		}
	}

	if(GET_POS(ch) != POSITION_FIGHTING) {
		return FALSE;
	}

	if(ch->specials.fighting &&
			ch->specials.fighting->in_room == ch->in_room) {
		act("$c0010$n morde $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
		act("$c0010$n ti morde!", 1, ch, 0, ch->specials.fighting, TO_VICT);
		cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, nullptr);
		return TRUE;
	}
	return FALSE;
}

//Come snake, ma fa anche le skill della classe del mob
MOBSPECIAL_FUNC(snake_plus) {

	if(type != EVENT_TICK || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch) == POSITION_SITTING) {
		if(ch->specials.fighting) {
			GET_POS(ch) = POSITION_FIGHTING;
		}
		else {
			GET_POS(ch) = POSITION_STANDING;
		}
	}

	if(GET_POS(ch) != POSITION_FIGHTING) {
		return FALSE;
	}

	if(ch->specials.fighting &&
			ch->specials.fighting->in_room == ch->in_room) {
		act("$c0010$n morde $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
		act("$c0010$n ti morde!", 1, ch, 0, ch->specials.fighting, TO_VICT);
		cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
					ch->specials.fighting, 0);
		return FALSE;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(Pungiglione) {

	if(type != EVENT_TICK || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch) == POSITION_SITTING) {
		if(ch->specials.fighting) {
			GET_POS(ch) = POSITION_FIGHTING;
		}
		else {
			GET_POS(ch) = POSITION_STANDING;
		}
	}

	if(GET_POS(ch) != POSITION_FIGHTING) {
		return FALSE;
	}

	if(ch->specials.fighting &&
			ch->specials.fighting->in_room == ch->in_room) {
		act("$c0010Pungi $N!", 1, ch, 0, ch->specials.fighting, TO_CHAR);
		act("$c0010$n punge $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
		act("$c0010$n ti punge!", 1, ch, 0, ch->specials.fighting, TO_VICT);
		cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, nullptr);
		return TRUE;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(Pungiglione_maggiore) {

	if(type != EVENT_TICK || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch) == POSITION_SITTING) {
		if(ch->specials.fighting) {
			GET_POS(ch) = POSITION_FIGHTING;
		}
		else {
			GET_POS(ch) = POSITION_STANDING;
		}
	}

	if(GET_POS(ch) != POSITION_FIGHTING) {
		return FALSE;
	}

	if(ch->specials.fighting &&
			ch->specials.fighting->in_room == ch->in_room) {
		act("$c0010Pungi $N!", 1, ch, 0, ch->specials.fighting, TO_CHAR);
		act("$c0010$n punge $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
		act("$c0010$n ti punge!", 1, ch, 0, ch->specials.fighting, TO_VICT);
		cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, nullptr);
		return TRUE;
	}
	return FALSE;
}


/*FLYP 2004 SporeCloud-Fenice*/
MOBSPECIAL_FUNC(SporeCloud) {
	struct char_data* tmp_victim, *temp;

	assert(ch);

	if(type == EVENT_COMMAND && cmd == CMD_BASH && strlen(arg)>0 &&
			strstr(GET_NAME(mob), arg)) {
		act("Ti schianti contro $n!\r\n",FALSE,mob,0,ch,TO_VICT);
		act("Da $n si leva una nuvola di spore!\r\n",FALSE,mob,0,ch,TO_VICT);
		act("La tua vista si offusca per qualche secondo, fatichi a respirare..\r\n",FALSE,mob,0,ch,TO_VICT);
		act("$N si schianta contro $n!\r\n",FALSE,mob,0,ch,TO_NOTVICT);
		act("Da $n si leva una nuvola di spore! Fatichi a respirare..\r\n",FALSE,mob,0,ch,TO_NOTVICT);

		for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
			temp = tmp_victim->next;
			if((ch->in_room == tmp_victim->in_room) && !IS_IMMORTAL(tmp_victim) && (mob != tmp_victim)) {
				damage(tmp_victim, tmp_victim, GetMaxLevel(mob),SPELL_POISON, 5);
			}
			else {
				act("Osservando le spore vedi le piante che diventeranno...",FALSE, ch, 0, tmp_victim, TO_VICT);
			}
		}
	}
	else {
		return FALSE;
	}

	return FALSE;
}


MOBSPECIAL_FUNC(Tsuchigumo) {
	struct affected_type af;
	int i;
	struct room_data* rp;
	struct char_data* tmp, *tmp2;

	if(type != EVENT_TICK || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch) == POSITION_SITTING) {
		if(ch->specials.fighting) {
			GET_POS(ch) = POSITION_FIGHTING;
		}
		else {
			GET_POS(ch) = POSITION_STANDING;
		}
	}

	if(GET_POS(ch) != POSITION_FIGHTING) {
		GET_DEX(ch) = 23;

		if(GET_HIT(ch) < GET_MAX_HIT(ch)) {
			GET_HIT(ch) += 30;
			GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
			alter_hit(ch,0);
			GET_MOVE(ch) +=6;
			alter_move(ch,0);
			act("$n rigenera la sua struttura fisica.", TRUE, ch, 0, 0, TO_ROOM);
		}
		return FALSE;
	}

	if(ch->specials.fighting &&
			ch->specials.fighting->in_room == ch->in_room) {
		GET_MOVE(ch) +=6;
		alter_move(ch,0);
		switch(number(1,5)) {
		case 1:
			act("$c0010Mordi $N!", 1, ch, 0, ch->specials.fighting, TO_CHAR);
			act("$c0010$n morde $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
			act("$c0010$n ti morde!", 1, ch, 0, ch->specials.fighting, TO_VICT);
			cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, nullptr);
			cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, nullptr);
			cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, nullptr);
			return TRUE;
			break;
		case 2:
			if(!affected_by_spell(ch, SPELL_MIRROR_IMAGES)) {
				act("$n moltiplica la sua immagine!.", TRUE, ch, 0, 0, TO_ROOM);
				send_to_char("Evochi delle immagini illusorie per confondere i nemici.\n\r", ch);
				act(" ", FALSE, mob, NULL, mob, TO_ROOM);
				for(i=1+(GetMaxLevel(ch)/10); i; i--) {
					af.type      = SPELL_MIRROR_IMAGES;
					af.duration  = number(1,4)+(GetMaxLevel(ch)/5);
					af.modifier  = 0;
					af.location  = APPLY_NONE;
					af.bitvector = 0;
					affect_to_char(ch, &af);
				}
			}
			else {
				return(magic_user(mob,cmd,arg,mob,type));
			}
			break;
		case 3:
			cast_gust_of_wind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
							  ch->specials.fighting, 0);
			break;
		case 4:
			if(GET_HIT(ch) < (GET_MAX_HIT(ch)/2)) {
				act("$n si scompone in tanti ragni piu' piccoli.",FALSE, mob, NULL, nullptr, TO_ROOM);
				act(" ", FALSE, mob, NULL, mob, TO_ROOM);
				struct char_data* pRagno=nullptr;
				for(i=10; i; i--) {
					if((pRagno = read_mobile(real_mobile(19935), REAL))) {
						char_to_room(pRagno, ch->in_room);
					}
				}
				char_from_room(ch);
			}
			else {
				return(magic_user(mob,cmd,arg,mob,type));
			}
			break;
		case 5:
			act("Gli occhi di $n si infuocano e senti il tuo corpo svanire.",
				FALSE, mob, NULL, mob, TO_ROOM);
			act(" ", FALSE, mob, NULL, mob, TO_ROOM);

			rp = real_roomp(ch->in_room);

			for(tmp = rp->people; tmp; tmp = tmp2) {
				tmp2 = tmp->next_in_room;
				if((GetMaxLevel(tmp) < IMMORTALE) && (tmp != ch)) {
					act("$n si dissolve con un lampo di luce.", FALSE, tmp, 0, 0, TO_ROOM);
					char_from_room(tmp);
					char_to_room(tmp, 19989);
					do_look(tmp, "", 15);
				}
			}

			break;
		default:
			break;
		} /* end switch */

	}
	return FALSE;
}

MOBSPECIAL_FUNC(SputoVelenoso) {

	if(type != EVENT_TICK || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch) == POSITION_SITTING) {
		if(ch->specials.fighting) {
			GET_POS(ch) = POSITION_FIGHTING;
		}
		else {
			GET_POS(ch) = POSITION_STANDING;
		}
	}

	if(GET_POS(ch) != POSITION_FIGHTING) {
		return FALSE;
	}

	if(ch->specials.fighting &&
			ch->specials.fighting->in_room == ch->in_room) {
		act("$c0010Sputi a $N!", 1, ch, 0, ch->specials.fighting, TO_CHAR);
		act("$c0010$n sputa a $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
		act("$c0010$n ti sputa!", 1, ch, 0, ch->specials.fighting, TO_VICT);
		cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
					ch->specials.fighting, 0);
		return TRUE;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(PaladinGuildGuard) {

	if(!AWAKE(ch)) {
		return(FALSE);
	}

	if(!cmd) {
		if(ch->specials.fighting) {
			fighter(ch, cmd, arg, mob, type);
		}
	}
	else if(cmd >= CMD_NORTH && cmd <= CMD_DOWN) {
		if((cmd == CMD_EAST) || (cmd == CMD_NORTH)  || (cmd == CMD_WEST)) {
			return(FALSE);
		}
		if(!HasClass(ch, CLASS_PALADIN)) {
			send_to_char
			("The guard shakes his head, and blocks your way.\n\r", ch);
			act("The guard shakes his head, and blocks $n's way.",
				TRUE, ch, 0, 0, TO_ROOM);
			return(TRUE);
		}
	}
	return(FALSE);
}

#if 0

MOBSPECIAL_FUNC(GameGuard) {

	if(!cmd) {
		if(ch->specials.fighting) {
			fighter(ch, cmd, arg,mob,type);
		}
	}

	if(cmd == CMD_WEST) {   /* West is field */
		if((IS_AFFECTED(ch,AFF_TEAM_GREY)) ||
				(IS_AFFECTED(ch,AFF_TEAM_AMBER))) {
			send_to_char
			("The guard wishes you good luck on the field.\n\r", ch);
			return(FALSE);
		}
		else {
			send_to_char
			("The guard shakes his head, and blocks your way.\n\r", ch);
			act("The guard shakes his head, and blocks $n's way.",
				TRUE, ch, 0, 0, TO_ROOM);
			send_to_char
			("The guard says 'Your not a player! You can't enter the field!'.\n\r", ch);
			return(TRUE);
		}
		return(FALSE);
	}
	else {
		return(FALSE);
	}
}

int GreyParamedic(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int types) {
	struct char_data* vict, *most_hurt;

	if(!cmd) {
		if(ch->specials.fighting) {
			return(cleric(ch, 0, "", mob, types));
		}
		else {
			if(GET_POS(ch) == POSITION_STANDING) {

				/* Find a dude to do good things upon ! */

				most_hurt = real_roomp(ch->in_room)->people;
				for(vict = real_roomp(ch->in_room)->people; vict;
						vict = vict->next_in_room) {
					if(((float)GET_HIT(vict)/(float)hit_limit(vict) <
							(float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt))
							&& (CAN_SEE(ch, vict))) {
						most_hurt = vict;
					}
				}
				if(!most_hurt) {
					return(FALSE);    /* nobody here */
				}
				if(IS_AFFECTED(most_hurt,AFF_TEAM_GREY)) {

					if((float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt) >
							0.66) {
						if(number(0,5)==0) {
							act("$n shrugs helplessly.", 1, ch, 0, 0, TO_ROOM);
						}
						return TRUE;        /* not hurt enough */
					}

					if(!check_soundproof(ch)) {
						if(number(0,4)==0) {
							if(most_hurt != ch) {
								act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
								act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
							}

							if(check_nomagic(ch, 0, 0)) {
								return(TRUE);
							}

							act("$n utters the words 'judicandus dies'.",
								1, ch, 0, 0, TO_ROOM);
							cast_cure_light(GetMaxLevel(ch), ch, "",
											SPELL_TYPE_SPELL, most_hurt, 0);
							return(TRUE);
						}
					}
				}
				else {   /* Other Team? */
					if(IS_AFFECTED(most_hurt,AFF_TEAM_AMBER)) {

						if(check_nomagic(ch, 0, 0)) {
							return(TRUE);
						}

						act("$n utters the words 'die punk'.",
							1, ch, 0, 0, TO_ROOM);
						cast_cause_light(GetMaxLevel(ch), ch, "",
										 SPELL_TYPE_SPELL, most_hurt, 0);
						return(TRUE);
					}
					else {
						return(FALSE);
					}
				}
			}
			else {  /* I'm asleep or sitting */
				return(FALSE);
			}
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(AmberParamedic) {
	struct char_data* vict, *most_hurt;

	if(!cmd) {
		if(ch->specials.fighting) {
			return(cleric(ch, 0, "",mob,type));
		}
		else {
			if(GET_POS(ch) == POSITION_STANDING) {

				/* Find a dude to do good things upon ! */

				most_hurt = real_roomp(ch->in_room)->people;
				for(vict = real_roomp(ch->in_room)->people; vict;
						vict = vict->next_in_room) {
					if(((float)GET_HIT(vict)/(float)hit_limit(vict) <
							(float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt))
							&& (CAN_SEE(ch, vict))) {
						most_hurt = vict;
					}
				}
				if(!most_hurt) {
					return(FALSE);    /* nobody here */
				}
				if(IS_AFFECTED(most_hurt,AFF_TEAM_AMBER)) {

					if((float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt) >
							0.66) {
						if(number(0,5)==0) {
							act("$n shrugs helplessly.", 1, ch, 0, 0, TO_ROOM);
						}
						return TRUE;        /* not hurt enough */
					}

					if(!check_soundproof(ch)) {
						if(number(0,4)==0) {
							if(most_hurt != ch) {
								act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
								act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
							}

							if(check_nomagic(ch, 0, 0)) {
								return(TRUE);
							}

							act("$n utters the words 'judicandus dies'.",
								1, ch, 0, 0, TO_ROOM);
							cast_cure_light(GetMaxLevel(ch), ch, "",
											SPELL_TYPE_SPELL, most_hurt, 0);
							return(TRUE);
						}
					}
				}
				else {   /* Other Team? */
					if(IS_AFFECTED(most_hurt,AFF_TEAM_GREY)) {

						if(check_nomagic(ch, 0, 0)) {
							return(TRUE);
						}

						act("$n utters the words 'die punk'.",
							1, ch, 0, 0, TO_ROOM);
						cast_cause_light(GetMaxLevel(ch), ch, "",
										 SPELL_TYPE_SPELL, most_hurt, 0);
						return(TRUE);
					}
					else {
						return(FALSE);
					}
				}
			}
			else {  /* I'm asleep or sitting */
				return(FALSE);
			}
		}
	}
	return(FALSE);
}
#endif


MOBSPECIAL_FUNC(blink) {
	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}


	if(GET_HIT(ch) < (int)GET_MAX_HIT(ch) / 3) {
		act("$n blinks.",TRUE,ch,0,0,TO_ROOM);
		cast_teleport(12, ch, "", SPELL_TYPE_SPELL, ch, 0);
		return(TRUE);
	}
	else {
		return(FALSE);
	}
}



MOBSPECIAL_FUNC(MidgaardCitizen) {
	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		fighter(ch, cmd, arg,mob,type);

		if(check_soundproof(ch)) {
			return(FALSE);
		}

		if(number(0,18) == 0) {
			do_shout(ch, "Guards! Help me! Please!", 0);
		}
		else {
			act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
		}

		if(ch->specials.fighting) {
			CallForGuard(ch, ch->specials.fighting, 3, MIDGAARD);
		}

		return(TRUE);

	}
	else {
		return(FALSE);
	}
}

MOBSPECIAL_FUNC(ghoul) {
	struct char_data* tar;


	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	tar = ch->specials.fighting;

	if(tar && tar->in_room == ch->in_room) {
		if(!affected_by_spell(tar, SPELL_PROTECT_FROM_EVIL) &&
				!IS_AFFECTED(tar, AFF_SANCTUARY)) {
			if(HitOrMiss(ch, tar, CalcThaco(ch, tar))) {
				act("$n tocca $N!", 1, ch, 0, tar, TO_NOTVICT);
				act("$n ti tocca!", 1, ch, 0, tar, TO_VICT);
				if(!IS_AFFECTED(tar, AFF_PARALYSIS)) {
					cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,tar, 0);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(CarrionCrawler) {
	struct char_data* tar;
	int i;


	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	for(i=0; i<8; i++) {
		if((tar = FindAHatee(ch)) == NULL) {
			tar = FindVictim(ch);
		}

		if(tar && tar->in_room == ch->in_room) {
			if(HitOrMiss(ch, tar, CalcThaco(ch, tar))) {
				act("$n tocca $N!", 1, ch, 0, tar, TO_NOTVICT);
				act("$n ti tocca!", 1, ch, 0, tar, TO_VICT);
				if(!IS_AFFECTED(tar, AFF_PARALYSIS)) {
					cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,tar, 0);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(WizardGuard) {
	struct char_data* tch, *evil;
	int max_evil;

	if(cmd || !AWAKE(ch)) {
		return (FALSE);
	}

	if(ch->specials.fighting) {
		fighter(ch, cmd, arg,mob,type);
		CallForGuard(ch, ch->specials.fighting, 9, MIDGAARD);
	}
	max_evil = 1000;
	evil = 0;

	for(tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
		if(tch->specials.fighting) {
			if((GET_ALIGNMENT(tch) < max_evil) &&
					(IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
				max_evil = GET_ALIGNMENT(tch);
				evil = tch;
			}
		}
	}

	if(evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0) &&
			!check_peaceful(ch, "")) {
		if(!check_soundproof(ch)) {
			act("$n screams 'DEATH!!!!!!!!'",
				FALSE, ch, 0, 0, TO_ROOM);
		}
		hit(ch, evil, TYPE_UNDEFINED);
		return(TRUE);
	}
	return(FALSE);
}



MOBSPECIAL_FUNC(vampire) {
	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting &&
			(ch->specials.fighting->in_room == ch->in_room)) {
		act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
		act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
		cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, 0);
		if(ch->specials.fighting &&
				(ch->specials.fighting->in_room == ch->in_room)) {
			cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, 0);
		}
		return TRUE;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(wraith) {

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}


	if(ch->specials.fighting &&
			(ch->specials.fighting->in_room == ch->in_room)) {
		act("$n tocca $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
		act("$n ti tocca!", 1, ch, 0, ch->specials.fighting, TO_VICT);
		cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
						  ch->specials.fighting, 0);
		return TRUE;
	}
	return FALSE;
}


MOBSPECIAL_FUNC(shadow) {


	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting &&
			(ch->specials.fighting->in_room == ch->in_room)) {
		act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
		act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
		cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, nullptr);
		if(ch->specials.fighting) {
			cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,ch->specials.fighting, nullptr);
		}
		return TRUE;
	}
	return FALSE;
}



MOBSPECIAL_FUNC(geyser) {

	if(cmd || !AWAKE(ch)) {
		return FALSE;
	}

	if(number(0, 3) == 0) {
		act("Tu erutti.", 1, ch, 0, 0, TO_CHAR);
		cast_geyser(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
		send_to_zone("Il vulcano emette un lieve rombo.\n\r", ch);
		return TRUE;
	}
	return FALSE;
}


MOBSPECIAL_FUNC(green_slime) {
	struct char_data* cons;

	if(cmd || !AWAKE(ch)) {
		return FALSE;
	}

	for(cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room)
		if((!IS_NPC(cons)) && (GetMaxLevel(cons)<IMMORTALE)) {
			cast_green_slime(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, cons, 0);
		}

	return FALSE;
}


MOBSPECIAL_FUNC(DracoLich) {
	return FALSE;
}


MOBSPECIAL_FUNC(Drow) {

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(!ch->specials.fighting) {
		if(!IS_UNDERGROUND(ch) && !IS_DARK(ch->in_room) &&
				!affected_by_spell(ch, SPELL_GLOBE_DARKNESS)) {
			act("$n uses $s innate powers of darkness",FALSE,ch,0,0,TO_ROOM);
			cast_globe_darkness(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
			return(TRUE);
		}
		if(ch->specials.hunting) {
			act("$n uses $s innate powers of levitation",FALSE,ch,0,0,TO_ROOM);
			cast_flying(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
			return(TRUE);
		}
	} /* end not fighting */
	else {

		if(!affected_by_spell(ch->specials.fighting, SPELL_BLINDNESS) &&
				number(1,100) > 25) {
			/* simulate casting darkness on a person... */
			act("$n uses $s innate powers of darkness on $N!", FALSE, ch, 0,
				ch->specials.fighting,TO_NOTVICT);
			act("$n drops a pitch black globe around you!", FALSE, ch, 0,
				ch->specials.fighting, TO_VICT);
			cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
						   ch->specials.fighting, 0);
			return TRUE;
		} /* end darkness */

		if(!affected_by_spell(ch->specials.fighting, SPELL_FAERIE_FIRE) &&
				number(1,100)>50) {
			/* simulate faerie fire */
			act("$n tries to outline $N with $s faerie fire!", FALSE, ch, 0,
				ch->specials.fighting, TO_NOTVICT);
			act("$n tries to outline you with a faerie fire glow!", FALSE, ch, 0,
				ch->specials.fighting,TO_VICT);
			cast_faerie_fire(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
							 ch->specials.fighting, 0);
			return TRUE;
		} /* end faerie fire */
	} /* end was fighting */

	return(archer(ch, cmd, arg, mob, type));
}

MOBSPECIAL_FUNC(Leader) {
	return FALSE;
}


MOBSPECIAL_FUNC(thief) {
	struct char_data* cons;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(GET_POS(ch)!=POSITION_STANDING) {
		return FALSE;
	}

	for(cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room)
		if((!IS_NPC(cons)) && (GetMaxLevel(cons)<IMMORTALE) && (number(1,5)==1)) {
			npc_steal(ch,cons);
		}

	return(FALSE);
}





/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

MOBSPECIAL_FUNC(guild_guard) {

	if(!cmd) {
		if(ch->specials.fighting) {
			return(fighter(ch, cmd, arg,mob,type));
		}
	}
	else {

		switch(ch->in_room) {
		case 3017:
			return(CheckForBlockedMove(
					   ch, cmd, arg, 3017, 2, CLASS_MAGIC_USER|CLASS_SORCERER));
			break;
		case 3004:
			return(CheckForBlockedMove(ch, cmd, arg, 3004, 0, CLASS_CLERIC));
			break;
		case 3027:
			return(CheckForBlockedMove(ch, cmd, arg, 3027, 1, CLASS_THIEF));
			break;
		case 3021:
			return(CheckForBlockedMove(ch, cmd, arg, 3021, 1, CLASS_WARRIOR));
			break;
		}
	}

	return FALSE;

}




MOBSPECIAL_FUNC(Inquisitor) {
	if(cmd || !AWAKE(ch)) {
		return FALSE;
	}

	if(ch->specials.fighting) {
		return(fighter(ch, cmd, arg, mob, type));
	}

	if(ch->generic == INQ_SHOUT) {
		if(!check_soundproof(ch)) {
			do_shout(ch, "NESSUNO vuole l'Inquisizione Spagnola!", 0);
		}
		ch->generic = 0;
		return TRUE;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(puff) {
	struct char_data* i, *tmp_ch;
	char buf[80];

	if(type == EVENT_DWARVES_STRIKE) {
		do_shout(ch, "Ack! Of all the stupid things! Those damned dwarves are on strike again!", 0);
		return(TRUE);
	}

	if(type == EVENT_END_STRIKE) {
		do_shout(ch, "Gee, about time those dwarves stopped striking!", 0);
		return(TRUE);
	}

	if(type == EVENT_DEATH) {
		do_shout(ch, "Ack! I've been killed! Have some God Load me again!!!", 0);
		return(TRUE);
	}

	if(type == EVENT_SPRING) {
		do_shout(ch, "Ahhh, spring is in the air.", 0);
		return(TRUE);
	}

	if(cmd) {
		return FALSE;
	}

	if(ch->generic == 1) {
		do_shout(ch, "When will we get there?", 0);
		ch->generic = 0;
	}


	if(check_soundproof(ch)) {
		return(FALSE);
	}

	switch(number(0, 250)) {
	case 0:
		sprintf(buf,"Anyone know where I am at?\n");
		do_say(ch, buf, 0);
		return(1);
	case 1:
		do_say(ch, "How'd all those fish get up here?", 0);
		return(1);
	case 2:
		do_say(ch, "I'm a very female dragon.", 0);
		return(1);
	case 3:
		do_say(ch, "Haven't I seen you at the Temple?", 0);
		return(1);
	case 4:
		do_shout(ch, "Bring out your dead, bring out your dead!", 0);
		return(1);
	case 5:
		do_emote(ch, "gropes you.", 0);
		return(1);
	case 6:
		do_emote(ch, "gives you a long and passionate kiss.  It seems to last forever.", 0);
		return(1);
	case 7: {
		for(i = character_list; i; i = i->next) {
			if(!IS_NPC(i)) {
				if(number(0,5)==0) {
					if(!strcmp(GET_NAME(i),"Alar")) {
						do_shout(ch,"Alar, come ravish me now!",0);
					}
					else if(!strcmp(GET_NAME(i), "Fiona")) {
						do_shout(ch,"I'm Puff the PMS dragon!",0);
					}
					else if(!strcmp(GET_NAME(i), "Stranger")) {
						do_shout(ch, "People are strange, when they're with Stranger!",0);
					}
					else if(!strcmp(GET_NAME(i), "God")) {
						do_shout(ch, "God!  Theres only room for one smartass robot on this mud!",0);
					}
					else if(GET_SEX(i)==SEX_MALE) {
						sprintf(buf,"Hey, %s, how about some MUDSex?",GET_NAME(i));
						do_say(ch,buf,0);
					}
					else {
						sprintf(buf,"I'm much prettier than %s, don't you think?",GET_NAME(i));
						do_say(ch,buf,0);
					}
				}
			}
			break;
		}
	}
	return(1);
	case 8:
		do_say(ch, "Alar is my hero!", 0);
		return(1);
	case 9:
		do_say(ch, "So, wanna neck?", 0);
		return(1);
	case 10: {
		tmp_ch = (struct char_data*)FindAnyVictim(ch);
		if(!IS_NPC(ch)) {
			sprintf(buf, "Party on, %s", GET_NAME(tmp_ch));
			do_say(ch, buf, 0);
			return(1);
		}
		else {
			return(0);
		}
	}
	case 11:
		if(!number(0,30)) {
			do_shout(ch, "NOT!!!", 0);
		}
		return(1);
	case 12:
		do_say(ch, "Bad news.  Termites.", 0);
		return(1);
	case 13:
		for(i = character_list; i; i = i->next) {
			if(!IS_NPC(i)) {
				if(number(0,30)==0) {
					sprintf(buf, "%s shout I love Joy!",GET_NAME(i));
					do_force(ch, buf, 0);
					sprintf(buf, "%s bounce", GET_NAME(i));
					do_force(ch, buf, 0);
					do_restore(ch, GET_NAME(i), 0);
					return(TRUE);
				}
			}
		}
		return(1);
	case 14:
		do_say(ch, "I'll be back.", 0);
		return(1);
	case 15:
		do_say(ch, "Aren't wombat's so cute?", 0);
		return(1);
	case 16:
		do_emote(ch, "fondly fondles you.", 0);
		return(1);
	case 17:
		do_emote(ch, "winks at you.", 0);
		return(1);
	case 18:
		do_say(ch, "This mud is too silly!", 0);
		return(1);
	case 19:
		do_say(ch, "If the Mayor is in a room alone, ", 0);
		do_say(ch, "Does he say 'Good morning citizens.'?",0);
		return(0);
	case 20:
		for(i = character_list; i; i = i->next) {
			if(!IS_NPC(i)) {
				if(number(0,30)==0) {
					sprintf(buf, "Top of the morning to you %s!", GET_NAME(i));
					do_shout(ch, buf, 0);
					return(TRUE);
				}
			}
		}
		break;
	case 21:
		for(i = real_roomp(ch->in_room)->people; i; i= i->next_in_room) {
			if(!IS_NPC(i)) {
				if(number(0,3)==0) {
					sprintf(buf, "Pardon me, %s, but are those bugle boy jeans you are wearing?", GET_NAME(i));
					do_say(ch, buf, 0);
					return(TRUE);
				}
			}
		}
		break;
	case 22:
		for(i = real_roomp(ch->in_room)->people; i; i= i->next_in_room) {
			if(!IS_NPC(i)) {
				if(number(0,3)==0) {
					sprintf(buf, "Pardon me, %s, but do you have any Grey Poupon?", GET_NAME(i));
					do_say(ch, buf, 0);
					return(TRUE);
				}
			}
		}
		break;
	case 23:
		if(number(0,80)==0) {
			do_shout(ch, "Where are we going?", 0);
			ch->generic = 1;
		}
		break;
	case 24:
		do_say(ch, "Blackstaff is a wimp!", 0);
		return(TRUE);
		break;
	case 25:
		do_say(ch, "Better be nice or I will user spellfire on you!", 0);
		return(TRUE);
		break;
	case 26:
		if(number(0,100)==0) {
			do_shout(ch, "What is the greatest joy?", 0);
		}
		break;
	case 27:
		do_say(ch, "Have you see Seneca ? Gads he is slow...", 0);
		return(TRUE);
	case 28:
		if(number(0,50)) {
			do_shout(ch, "SAVE!  I'm running out of cute things to say!", 0);
		}
		do_force(ch, "all save", 0);
		return(TRUE);
	case 29:
		do_say(ch, "I hear Strahd is a really mean vampire.", 0);
		return(TRUE);
	case 30:
		do_say(ch, "I heard there was a sword that would kill frost giants.", 0);
		return(TRUE);
	case 31:
		do_say(ch, "Hear about the sword that kills Red Dragons?", 0);
		return(TRUE);
	case 32:
		if(number(0,100)==0) {
			do_shout(ch, "Help yourself and help a newbie!", 0);
			return(TRUE);
		}
		break;
	case 33:
		if(number(0,100)==0) {
			/* do_shout(ch, "Kill all other dragons!", 0); */
			return(TRUE);
		}
		break;
	case 34:
		if(number(0,50)==0) {
			for(i = character_list; i; i=i->next) {
				if((void*)mob_index[i->nr].func == (void*)Inquisitor) {
					do_shout(ch, "I wasn't expecting the Spanish Inquisition!", 0);
					i->generic = INQ_SHOUT;
					return(TRUE);
				}
			}
			return(TRUE);
		}
		break;
	case 35:
		do_say(ch, "Are you crazy, is that your problem?", 0);
		return(TRUE);
	case 36:
		for(i = real_roomp(ch->in_room)->people; i; i=i->next_in_room) {
			if(!IS_NPC(i)) {
				if(number(0,3)==0) {
					sprintf(buf, "%s, do you think I'm going bald?",GET_NAME(i));
					do_say(ch, buf, 0);
					return(TRUE);
				}
			}
		}
		break;
	case 37:
		do_say(ch, "This is your brain.", 0);
		do_say(ch, "This is MUD.", 0);
		do_say(ch, "This is your brain on MUD.", 0);
		do_say(ch, "Any questions?", 0);
		return(TRUE);
	case 38:
		for(i = character_list; i; i=i->next) {
			if(!IS_NPC(i)) {
				if(number(0,20) == 0) {
					if(i->in_room != NOWHERE) {
						sprintf(buf, "%s save", GET_NAME(i));
						do_force(ch, buf, 0);
						return(TRUE);
					}
				}
			}
		}
		return(TRUE);
	case 39:
		do_say(ch, "I'm Puff the Magic Dragon, who the hell are you?", 0);
		return(TRUE);
	case 40:
		do_say(ch, "Attention all planets of the Solar Federation!", 0);
		do_say(ch, "We have assumed control.", 0);
		return(TRUE);
	case 41:
		if(number(0,50)==0) {
			do_shout(ch, "We need more explorers!", 0);
			return(TRUE);
		}
		break;
	case 42:
		if(number(0,50)==0) {
			do_shout(ch, "Pray to Alar, he might be in a good mood!", 0);
			return(TRUE);
		}
		break;
	case 43:
		do_say(ch, "Pardon me boys, is this the road to Great Cthulhu?", 0);
		return(TRUE);
	case 44:
		do_say(ch, "May the Force be with you... Always.", 0);
		return(TRUE);
	case 45:
		do_say(ch, "Eddies in the space time continuum.", 0);
		return(TRUE);
	case 46:
		do_say(ch, "Quick!  Reverse the polarity of the neutron flow!", 0);
		return(TRUE);
	case 47:
		if(number(0,50) == 0) {
			do_shout(ch, "Someone pray to DarkStar, she is lonely.",
					 0);
			return(TRUE);
		}
		break;
	case 48:
		do_say(ch, "Shh...  I'm beta testing.  I need complete silence!", 0);
		return(TRUE);
	case 49:
		do_say(ch, "Do you have any more of that Plutonium Nyborg!", 0);
		return(TRUE);
	case 50:
		do_say(ch, "I'm the real implementor, you know.", 0);
		return(TRUE);
	case 51:
		do_emote(ch, "moshes into you almost causing you to fall.", 0);
		return(TRUE);
	case 52:
		if(!number(0,30)) {
			do_shout(ch, "Everybody pray to Xanathon!", 0);
		}
		return(TRUE);
	case 53:
		do_say(ch, "You know I always liked you the best don't you?", 0);
		do_emote(ch, "winks seductively at you.", 0);
		return(TRUE);
	case 54:
		if(!number(0,30)) {
			do_shout(ch, "Ack! Who prayed to Wert!", 0);
		}
		return(TRUE);

	default:
		return(0);
	}
	return FALSE;
}

MOBSPECIAL_FUNC(regenerator) {
	if(type == EVENT_TICK) {
		if(GET_HIT(ch) < GET_MAX_HIT(ch)) {
			GET_HIT(ch) +=9;
			GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
			alter_hit(ch,0);
			act("$n rigenera.", TRUE, ch, 0, 0, TO_ROOM);
			return(TRUE);
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(replicant) {
	struct char_data* mymob;

	if(type == EVENT_TICK) {
		if(GET_HIT(ch) < GET_MAX_HIT(ch)) {
			act("Gocce del sangue di $n cadono a terra e si trasformano!",
				TRUE, ch, 0, 0, TO_ROOM);
			mymob = read_mobile(ch->nr, REAL);
			char_to_room(mymob, ch->in_room);
			act("Ora ci sono due opponenti integri di fronte a te.",
				TRUE, ch, 0, 0, TO_ROOM);
			GET_HIT(ch) = GET_MAX_HIT(ch);
			return TRUE;
		}
	}

	return FALSE;
}

#define TYT_NONE 0
#define TYT_CIT  1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT  4

MOBSPECIAL_FUNC(Tytan) {
	struct char_data* vict;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		return(magic_user(ch, cmd, arg, mob, type));
	}
	else {
		switch(ch->generic) {
		case TYT_NONE:
			if((vict = FindVictim(ch))) {
				ch->generic = TYT_CIT;
				SetHunting(ch, vict);
			}
			break;
		case TYT_CIT:
			if(ch->specials.hunting) {
				if(IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
					REMOVE_BIT(ch->specials.act, ACT_AGGRESSIVE);
				}
				if(ch->in_room == ch->specials.hunting->in_room) {
					act("Where is the Citadel?", TRUE, ch, 0, 0, TO_ROOM);
					ch->generic = TYT_WHAT;
				}
			}
			else {
				ch->generic = TYT_NONE;
			}
			break;
		case TYT_WHAT:
			if(ch->specials.hunting) {
				if(ch->in_room == ch->specials.hunting->in_room) {
					act("What must we do?", TRUE, ch, 0, 0, TO_ROOM);
					ch->generic = TYT_TELL;
				}
			}
			else {
				ch->generic = TYT_NONE;
			}
			break;
		case TYT_TELL:
			if(ch->specials.hunting) {
				if(ch->in_room == ch->specials.hunting->in_room) {
					act("Tell Us!  Command Us!", TRUE, ch, 0, 0, TO_ROOM);
					ch->generic = TYT_HIT;
				}
			}
			else {
				ch->generic = TYT_NONE;
			}
			break;
		case TYT_HIT:
			if(ch->specials.hunting) {
				if(ch->in_room == ch->specials.hunting->in_room) {
					if(!check_peaceful(ch, "The Tytan screams in anger")) {
						if(!IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
							SET_BIT(ch->specials.act, ACT_AGGRESSIVE);
						}
						ch->generic = TYT_NONE;
						hit(ch, ch->specials.hunting, TYPE_UNDEFINED);
					}
					else {
						ch->generic = TYT_CIT;
					}
				}
			}
			else {
				ch->generic = TYT_NONE;
			}
			break;
		default:
			ch->generic = TYT_NONE;
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(AbbarachDragon) {

	struct char_data* targ;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(!ch->specials.fighting) {
		targ = (struct char_data*)FindAnyVictim(ch);
		if(targ && !check_peaceful(ch, "")) {
			hit(ch, targ, TYPE_UNDEFINED);
			act("You have now payed the price of crossing.", TRUE, ch, 0, 0, TO_ROOM);
			return(TRUE);
		}
	}
	else {
		return(BreathWeapon(ch, cmd, arg,mob,type));
	}
	return FALSE;
}


MOBSPECIAL_FUNC(fido) {

	register struct obj_data* i, *temp, *next_obj, *next_r_obj;
	register struct char_data* v, *next;
	register struct room_data* rp;
	char found = FALSE;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if((rp = real_roomp(ch->in_room)) == 0) {
		return(FALSE);
	}

	for(v = rp->people; (v && (!found)); v = next) {
		next = v->next_in_room;
		if((IS_NPC(v)) && (mob_index[v->nr].iVNum == 100) &&
				CAN_SEE(ch, v)) {  /* is a zombie */
			if(v->specials.fighting) {
				stop_fighting(v);
			}
			make_corpse(v, 0);
			extract_char(v);
			found = TRUE;
		}
	}


	for(i = real_roomp(ch->in_room)->contents; i; i = next_r_obj) {
		next_r_obj = i->next_content;
		if(GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3]) {
			act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
			for(temp = i->contains; temp; temp=next_obj)        {
				next_obj = temp->next_content;
				obj_from_obj(temp);
				obj_to_room(temp,ch->in_room);
			}
			extract_obj(i);
			return(TRUE);
		}
	}
	return(FALSE);
}



MOBSPECIAL_FUNC(janitor) {
	struct obj_data* i;
	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}
	PushStatus("janitor");

	for(i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
		if(IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) &&
				((i->obj_flags.type_flag == ITEM_DRINKCON) ||
				 (i->obj_flags.cost <= 10))) {
			act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);

			obj_from_room(i);
			obj_to_char(i, ch);
			PopStatus();
			return(TRUE);
		}
	}
	PopStatus();
	return(FALSE);
}

MOBSPECIAL_FUNC(tormentor) {
	return(FALSE);
	mudlog(LOG_CHECK,"tormentor 1");
	if(!ch) {
		return(FALSE);
	}
	mudlog(LOG_CHECK,"tormentor 2");
	if(!cmd) {
		return(FALSE);
	}
	mudlog(LOG_CHECK,"tormentor 3");

	if(IS_IMMORTAL(ch)) {
		return(FALSE);
	}
	mudlog(LOG_CHECK,"tormentor 4");

	if(!IS_PC(ch)) {
		return(FALSE);
	}
	mudlog(LOG_CHECK,"tormentor 5");

	return(TRUE);

}

MOBSPECIAL_FUNC(RustMonster) {
	struct char_data* vict;
	struct obj_data* t_item;
	int t_pos;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	/*
	 **   find a victim
	 */
	vict = ch->specials.fighting;

	if(!vict) {
		vict = FindVictim(ch);
	}

	if(!vict) {
		return(FALSE);
	}

	/*
	 **   choose an item of armor or a weapon that is metal
	 **  since metal isn't defined, we'll just use armor and weapons
	 */

	/*
	 **  choose a weapon first, then if no weapon, choose a shield,
	 **  if no shield, choose breast plate, then leg plate, sleeves,
	 **  helm
	 */

	if(vict->equipment[WIELD]) {
		t_item = vict->equipment[WIELD];
		t_pos = WIELD;
	}
	else if(vict->equipment[WEAR_SHIELD]) {
		t_item = vict->equipment[WEAR_SHIELD];
		t_pos = WEAR_SHIELD;
	}
	else if(vict->equipment[WEAR_BODY]) {
		t_item = vict->equipment[WEAR_BODY];
		t_pos = WEAR_BODY;
	}
	else if(vict->equipment[WEAR_LEGS]) {
		t_item = vict->equipment[WEAR_LEGS];
		t_pos = WEAR_LEGS;
	}
	else if(vict->equipment[WEAR_ARMS]) {
		t_item = vict->equipment[WEAR_ARMS];
		t_pos = WEAR_ARMS;
	}
	else if(vict->equipment[WEAR_HEAD]) {
		t_item = vict->equipment[WEAR_HEAD];
		t_pos = WEAR_HEAD;
	}
	else {
		return(FALSE);
	}

	/*
	 **  item makes save (or not)
	 */
	if(DamageOneItem(vict, ACID_DAMAGE, t_item)) {
		t_item = unequip_char(vict, t_pos);
		if(t_item) {
			/*
			 **  if it doesn't make save, falls into a pile of scraps
			 */
			MakeScrap(vict,NULL, t_item);
		}
	}

	return(FALSE);

}

MOBSPECIAL_FUNC(temple_labrynth_liar) {

	if(cmd || !AWAKE(ch)) {
		return(0);
	}

	if(check_soundproof(ch)) {
		return(FALSE);
	}

	switch(number(0, 15)) {
	case 0:
		do_say(ch, "I'd go west if I were you.", 0);
		return(1);
	case 1:
		do_say(ch, "I heard that Vile is a cute babe.", 0);
		return(1);
	case 2:
		do_say(ch, "Going east will avoid the beast!", 0);
		return(1);
	case 4:
		do_say(ch, "North is the way to go.", 0);
		return(1);
	case 6:
		do_say(ch, "Dont dilly dally go south.", 0);
		return(1);
	case 8:
		do_say(ch, "Great treasure lies ahead", 0);
		return(1);
	case 10:
		do_say(ch, "I wouldn't kill the sentry if I were more than level 9. No way!", 0);
		return(1);
	case 12:
		do_say(ch, "I am a very clever liar.", 0);
		return(1);
	case 14:
		do_say(ch, "Loki is a really great guy!", 0);
		do_say(ch, "Well.... maybe not...", 0);
		return(1);
	default:
		do_say(ch, "Then again I could be wrong!", 0);
		return(1);
	}
}

MOBSPECIAL_FUNC(temple_labrynth_sentry) {
	struct char_data* tch;
	int counter;

	if(cmd || !AWAKE(ch)) {
		return FALSE;
	}

	if(GET_POS(ch)!=POSITION_FIGHTING) {
		return FALSE;
	}

	if(!ch->specials.fighting) {
		return FALSE;
	}

	if(check_soundproof(ch)) {
		return(FALSE);
	}

	/* Find a dude to do very evil things upon ! */

	for(tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
		if(GetMaxLevel(tch)>10 && CAN_SEE(ch, tch)) {
			act("The sentry snaps out of his trance and ...", 1, ch, 0, 0, TO_ROOM);
			do_say(ch, "You will die for your insolence, pig-dog!", 0);
			for(counter = 0 ; counter < 4 ; counter++)
				if(check_nomagic(ch, 0, 0)) {
					return(FALSE);
				}
			if(GET_POS(tch) > POSITION_SITTING) {
				cast_fireball(15, ch, "", SPELL_TYPE_SPELL, tch, 0);
			}
			else {
				return TRUE;
			}
			return TRUE;
		}
		else {
			act("The sentry looks concerned and continues to push you away",
				1, ch, 0, 0, TO_ROOM);
			do_say(ch, "Leave me alone. My vows do not permit me to kill you!", 0);
		}
	}
	return TRUE;
}

#if 0
#define WW_LOOSE 0
#define WW_FOLLOW 1

int Whirlwind(struct char_data* ch, int cmd, char* arg, struct char_data* mob, int type) {
	struct char_data* tmp;
	const char* names[] = { "Loki", "Belgarath", 0};
	int i = 0;

	if(ch->in_room == -1) {
		return(FALSE);
	}

	if(cmd == 0 && ch->generic == WW_LOOSE)  {
		for(tmp = real_roomp(ch->in_room)->people; tmp ; tmp = tmp->next_in_room) {
			while(names[i])  {
				if(!strcmp(GET_NAME(tmp), names[i]) && ch->generic == WW_LOOSE)  {
					/* start following */
					if(circle_follow(ch, tmp)) {
						return(FALSE);
					}
					if(ch->master) {
						stop_follower(ch);
					}
					add_follower(ch, tmp);
					ch->generic = WW_FOLLOW;
				}
				i++;
			}
		}
		if(ch->generic == WW_LOOSE && !cmd)  {
			act("The $n suddenly dissispates into nothingness.",0,ch,0,0,TO_ROOM);
			extract_char(ch);
		}
	}
}
#endif

#define NN_LOOSE  0
#define NN_FOLLOW 1
#define NN_STOP   2

MOBSPECIAL_FUNC(NudgeNudge) {

	struct char_data* vict;

	if(cmd || !AWAKE(ch)) {
		return (FALSE);
	}

	if(ch->specials.fighting) {
		return(FALSE);
	}


	switch(ch->generic) {
	case NN_LOOSE:
		/*
		** find a victim
		*/
		vict = FindVictim(ch);
		if(!vict) {
			return(FALSE);
		}
		/* start following */
		if(circle_follow(ch, vict)) {
			return(FALSE);
		}
		if(ch->master) {
			stop_follower(ch);
		}
		add_follower(ch, vict);
		ch->generic = NN_FOLLOW;
		if(!check_soundproof(ch)) {
			do_say(ch, "Good Evenin' Squire!", 0);
		}
		act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
		break;
	case NN_FOLLOW:
		switch(number(0,20)) {
		case 0:
			if(!check_soundproof(ch)) {
				do_say(ch, "Is your wife a goer?  Know what I mean, eh?", 0);
			}
			act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			break;
		case 1:
			act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			if(!check_soundproof(ch)) {
				do_say(ch, "Say no more!  Say no MORE!", 0);
			}
			break;
		case 2:
			if(!check_soundproof(ch)) {
				do_say(ch, "You been around, eh?", 0);
				do_say(ch, "...I mean you've ..... done it, eh?", 0);
			}
			act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			break;
		case 3:
			if(!check_soundproof(ch)) {
				do_say(ch, "A nod's as good as a wink to a blind bat, eh?", 0);
			}
			act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			break;
		case 4:
			if(!check_soundproof(ch)) {
				do_say(ch, "You're WICKED, eh!  WICKED!", 0);
			}
			act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
			break;
		case 5:
			if(!check_soundproof(ch)) {
				do_say(ch, "Wink. Wink.", 0);
			}
			break;
		case 6:
			if(!check_soundproof(ch)) {
				do_say(ch, "Nudge. Nudge.", 0);
			}
			break;
		case 7:
		case 8:
			ch->generic = NN_STOP;
			break;
		default:
			break;
		}
		break;
	case NN_STOP:
		/*
		**  Stop following
		*/
		if(!check_soundproof(ch)) {
			do_say(ch, "Evening, Squire", 0);
		}
		stop_follower(ch);
		ch->generic = NN_LOOSE;
		break;
	default:
		ch->generic = NN_LOOSE;
		break;
	}
	return(TRUE);
}

MOBSPECIAL_FUNC(AGGRESSIVE) {
	struct char_data* i, *next;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(check_peaceful(ch, "")) {
		return FALSE;
	}

	if(ch->in_room > -1) {
		for(i = real_roomp(ch->in_room)->people; i; i = next) {
			next = i->next_in_room;
			if(GET_RACE(i) != GET_RACE(ch)) {
				if(!IS_IMMORTAL(i) && CAN_SEE(ch, i)) {
					hit(ch, i, TYPE_UNDEFINED);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(citizen) {
	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		fighter(ch, cmd, arg,mob,type);

		if(check_soundproof(ch)) {
			return(FALSE);
		}

		if(number(0,18) == 0) {
			do_shout(ch, "Guards! Help me! Please!", 0);
		}
		else {
			act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(MidgaardCityguard) {
	return(GenericCityguardHateUndead(ch, cmd, arg, mob, MIDGAARD));
}



#define ONE_RING 1105
MOBSPECIAL_FUNC(Ringwraith) {
	static char      buf[ 256 ];
	struct char_data* victim;
	static int       howmanyrings = 0;
	struct obj_data*  ring;
	struct wraith_hunt {
		int        ringnumber;
		int        chances;
	} *wh;
	int rnum,dir;

	if(!AWAKE(mob) || !IS_NPC(mob) || type == EVENT_COMMAND) {
		return FALSE;
	}

	if(type == EVENT_TICK) {
		if(ch->specials.fighting) {
			if(GET_POS(ch) > POSITION_SLEEPING &&
					GET_POS(ch) < POSITION_FIGHTING) {
				do_stand(ch, "", 0);
			}
			else {
				wraith(ch, cmd, arg, mob, type);
			}

			return FALSE ;
		}

		/* how many one rings are in the game? */
		if(howmanyrings == 0) {
			if(ch->generic == 0) {
				howmanyrings = 0;
				get_obj_vis_world(ch, "999.primo anello.", &howmanyrings);
				ch->generic = 20;
			}
			else {
				ch->generic--;
			}
		}

		if(howmanyrings > 0) {
			/* does our ringwraith have his state info? */
			if(ch->act_ptr == 0) {
				ch->act_ptr = (struct wraith_hunt*)malloc(sizeof(*wh));
				wh = (struct wraith_hunt*)ch->act_ptr;
				wh->ringnumber = 0;
			}
			else {
				wh = (struct wraith_hunt*)ch->act_ptr;
			}

			if(!wh->ringnumber) {
				/* is he currently tracking a ring */
				wh->chances=0;
				wh->ringnumber = number(1, howmanyrings++);
			}

			sprintf(buf, "%d.primo anello.", (int)wh->ringnumber);
			/* where is this ring? */
			if((ring = get_obj_vis_world(ch, buf, NULL)) != NULL) {
				rnum = room_of_object(ring);

				if(rnum != ch->in_room) {
					dir = find_path(ch->in_room, is_target_room_p, &rnum,
									-5000, 0);
					if(dir < 0) {
						/* we can't find the ring */
						wh->ringnumber = 0;
						return FALSE;
					}
					go_direction(ch, dir);
				}
				else if((victim = char_holding(ring))) {
					/* the ring is in the same room! */
					if(victim == ch) {
						obj_from_char(ring);
						extract_obj(ring);
						wh->ringnumber = 0;
						howmanyrings--;
						ch->generic = 20;
						act("$n brilla contento.", FALSE, ch, NULL, victim, TO_ROOM);
					}
					else {
						switch(wh->chances) {
						case 0:
							do_wake(ch, GET_NAME(victim), 0);
							if(!check_soundproof(ch))
								act("$c0015[$c0005$n$c0015] dice '$N, dammi l'Anello'.",
									FALSE, ch, NULL, victim, TO_ROOM);
							else
								act("$n ti infila un dito nelle costole.", FALSE, ch, NULL,
									victim, TO_ROOM);
							wh->chances++;
							break;
						case 1:
							if(IS_NPC(victim)) {
								act("$N da' velocemente l'Anello a $n.", FALSE, ch, NULL,
									victim, TO_ROOM);
								if(ring->carried_by) {
									obj_from_char(ring);
								}
								else if(ring->equipped_by) {
									unequip_char(victim, ring->eq_pos);
								}
								else if(ring->in_obj) { // SALVO l'anello e' in un obj
									obj_from_obj(ring);
								}
								obj_to_char(ring, ch);
							}
							else {
								if(!check_soundproof(ch))
									act("$c0015[$c0005$n$c0015] dice '$N, dammi l'Anello "
										"ADESSO'.", FALSE, ch, NULL, victim, TO_ROOM);
								else {
									act("$n ti da' una dolorosa gomitata.",
										FALSE, ch, NULL, victim, TO_ROOM);
								}
								wh->chances++;
							}
							break;
						default:
							if(check_peaceful(ch, "Dannazione, e' in salvo.")) {
								if(!check_soundproof(ch))
									act("$c0015[$c0005$n$c0015] dice 'Non puoi star qui per "
										"sempre, $N'.", FALSE, ch, NULL, victim, TO_ROOM);
							}
							else {
								if(!check_soundproof(ch))
									act("$c0015[$c0005$n$c0015] dice 'Va bene, credo che "
										"dovro' prendermelo da solo'.", FALSE, ch, NULL,
										victim, TO_ROOM);
								hit(ch, victim, TYPE_UNDEFINED);
							}
							break;
						}
					}
				}
				else if(ring->in_obj) {
					/* the ring is in an object */
					obj_from_obj(ring);
					obj_to_char(ring, ch);
					act("$n prende $p.", FALSE, ch, ring, NULL, TO_ROOM);
				}
				else if(ring->in_room != NOWHERE) {
					obj_from_room(ring);
					obj_to_char(ring, ch);
					act("$n prende $p.", FALSE, ch, ring, NULL, TO_ROOM);
				}
				else {
					mudlog(LOG_SYSERR, "a One Ring was completely disconnected!?");
					wh->ringnumber = 0;
					return FALSE;
				}

				return TRUE;
			}
			else {
				/* there aren't as many one rings in the game as we thought */
				howmanyrings = 0;
				get_obj_vis_world(ch, "999.primo anello.", &howmanyrings);
				wh->ringnumber = 0;
				ch->generic = 20;
			}
		}
	}
	else if(type == EVENT_DEATH) {
		if(ch->act_ptr) {
			free(ch->act_ptr);
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(WarrenGuard) {
	struct char_data* tch, *good;
	int max_good;

	if(cmd || !AWAKE(ch)) {
		return (FALSE);
	}

	if(ch->specials.fighting) {
		fighter(ch, cmd, arg,mob,type);
		return(TRUE);
	}

	max_good = -1000;
	good = 0;

	if(check_peaceful(ch, "")) {
		return FALSE;
	}

	for(tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
		if(tch->specials.fighting) {
			if((GET_ALIGNMENT(tch) > max_good) &&
					(IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
				max_good = GET_ALIGNMENT(tch);
				good = tch;
			}
		}
	}

	if(good && (GET_ALIGNMENT(good->specials.fighting) <= 0)) {
		if(!check_soundproof(ch))
			act("$n screams 'DEATH TO GOODY-GOODIES!!!!'",
				FALSE, ch, 0, 0, TO_ROOM);
		hit(ch, good, TYPE_UNDEFINED);
		return(TRUE);
	}

	return(FALSE);
}



int zm_tired(struct char_data* zmaster) {
	return GET_HIT(zmaster) < GET_MAX_HIT(zmaster)/2 ||
		   GET_MANA(zmaster) < 40;
}

int zm_stunned_followers(struct char_data* zmaster) {
	struct follow_type*        fwr;
	for(fwr = zmaster->followers; fwr; fwr = fwr->next)
		if(GET_POS(fwr->follower)==POSITION_STUNNED) {
			return TRUE;
		}
	return FALSE;
}

void zm_init_combat(struct char_data* zmaster, struct char_data* target) {
	struct follow_type*        fwr;
	for(fwr = zmaster->followers; fwr; fwr = fwr->next) {
		if(IS_AFFECTED(fwr->follower, AFF_CHARM) &&
				fwr->follower->specials.fighting==NULL &&
				fwr->follower->in_room == target->in_room) {
			if(GET_POS(fwr->follower) == POSITION_STANDING) {
				hit(fwr->follower, target, TYPE_UNDEFINED);
			}
			else if(GET_POS(fwr->follower)>POSITION_SLEEPING &&
					GET_POS(fwr->follower)<POSITION_FIGHTING) {
				do_stand(fwr->follower, "", -1);
			}
		}
	}
}

int zm_kill_fidos(struct char_data* zmaster) {
	struct char_data*        fido_b;
	fido_b = find_mobile_here_with_spec_proc(reinterpret_cast<genericspecial_func>(fido), zmaster->in_room);
	if(fido_b) {
		if(!check_soundproof(zmaster)) {
			act("$n shrilly screams 'Kill that carrion beast!'", FALSE,
				zmaster, 0,0, TO_ROOM);
			zm_init_combat(zmaster, fido_b);
		}
		return TRUE;
	}
	return FALSE;
}

int zm_kill_aggressor(struct char_data* zmaster) {
	struct follow_type*        fwr;
	if(zmaster->specials.fighting) {
		if(!check_soundproof(zmaster)) {
			act("$n bellows 'Kill that mortal that dares lay hands on me!'",
				FALSE, zmaster, 0,0, TO_ROOM);
			zm_init_combat(zmaster, zmaster->specials.fighting);
			return TRUE;
		}
	}
	for(fwr = zmaster->followers; fwr; fwr = fwr->next)
		if(fwr->follower->specials.fighting &&
				IS_AFFECTED(fwr->follower, AFF_CHARM)) {
			if(!check_soundproof(zmaster)) {
				act("$n bellows 'Assist your brethren, my loyal servants!'", FALSE,
					zmaster, 0,0, TO_ROOM);
				zm_init_combat(zmaster, fwr->follower->specials.fighting);
				return TRUE;
			}
		}
	return FALSE;
}

MOBSPECIAL_FUNC(zombie_master)
#define ZM_MANA        10
#define ZM_NEMESIS 3060
{
	struct obj_data* temp1;
	struct char_data*        zmaster;
	int        dir;

	zmaster = find_mobile_here_with_spec_proc(reinterpret_cast<genericspecial_func>(zombie_master), ch->in_room);

	if(cmd!=0 || ch != zmaster || !AWAKE(ch)) {
		return FALSE;
	}

	if(!check_peaceful(ch, "") &&
			(zm_kill_fidos(zmaster) || zm_kill_aggressor(zmaster))
	  ) {
		do_stand(zmaster, "", -1);
		return TRUE;
	}

	switch(GET_POS(zmaster)) {
	case POSITION_RESTING:
		if(!zm_tired(zmaster)) {
			do_stand(zmaster, "", -1);
		}
		break;
	case POSITION_SITTING:
		if(!zm_stunned_followers(zmaster)) {
			if(!check_soundproof(ch))
				act("$n says 'It took you long enough...'", FALSE,
					zmaster, 0, 0, TO_ROOM);
			do_stand(zmaster, "", -1);
		}
		break;
	case POSITION_STANDING:
		if(zm_tired(zmaster)) {
			do_rest(zmaster, "", -1);
			return TRUE;
		}

		temp1 = get_obj_in_list_vis(zmaster, "corpse",
									real_roomp(zmaster->in_room)->contents);

		if(temp1) {
			if(GET_MANA(zmaster) < ZM_MANA) {
				if(1==dice(1,20))
					if(!check_soundproof(ch))
						act("$n says 'So many bodies, so little time' and sighs.",
							FALSE, zmaster, 0,0, TO_ROOM);
			}
			else {
				if(check_nomagic(ch, 0, 0)) {
					return(FALSE);
				}

				if(!check_soundproof(ch)) {
					act("$n says 'Wonderful, another loyal follower!' and grins maniacly.",
						FALSE, zmaster, 0,0, TO_ROOM);
					GET_MANA(zmaster) -= ZM_MANA;
					alter_mana(zmaster,0);
					spell_animate_dead(GetMaxLevel(zmaster), ch, NULL, temp1);
					if(zmaster->followers) {
						/* assume the new follower is top of the list? */
						AddHatred(zmaster->followers->follower,OP_VNUM,ZM_NEMESIS);
					}
				}
			}
			return TRUE;
		}
		else if(zm_stunned_followers(zmaster)) {
			do_sit(zmaster, "", -1);
			return TRUE;
		}
		else if(1==dice(1,20)) {
			act("$n searches for bodies.", FALSE, zmaster, 0,0, TO_ROOM);
			return TRUE;
		}
		else if(0<=(dir = find_path(zmaster->in_room,named_object_on_ground, "corpse", -200, 0))) {
			go_direction(zmaster, dir);
			return TRUE;
		}
		else if(1==dice(1,5)) {
			act("$n can't find any bodies.", FALSE, zmaster, 0,0, TO_ROOM);
			return TRUE;
		}
		else {
			mobile_wander(zmaster);
		}
	}

	return FALSE;
}

ROOMSPECIAL_FUNC(pet_shops) {
	char buf[MAX_STRING_LENGTH], pet_name[256];
	int pet_room;
	struct char_data* pet;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	pet_room = ch->in_room+1;

	if(cmd==CMD_LIST) {  /* List */
		send_to_char("Available pets are:\n\r", ch);
		for(pet = real_roomp(pet_room)->people; pet; pet = pet->next_in_room) {
			sprintf(buf, "%8d - %s\n\r", 24*GET_EXP(pet), pet->player.short_descr);
			send_to_char(buf, ch);
		}
		return(TRUE);
	}
	else if(cmd==CMD_BUY) {    /* Buy */

		arg = one_argument(arg, buf);
		only_argument(arg, pet_name);
		/* Pet_Name is for later use when I feel like it */

		if(!(pet = get_char_room(buf, pet_room))) {
			send_to_char("There is no such pet!\n\r", ch);
			return(TRUE);
		}

		if(GET_GOLD(ch) < (GET_EXP(pet)*10)) {
			send_to_char("You don't have enough gold!\n\r", ch);
			return(TRUE);
		}

		GET_GOLD(ch) -= GET_EXP(pet)*10;

		pet = read_mobile(pet->nr, REAL);
		GET_EXP(pet) = 0;
		SET_BIT(pet->specials.affected_by, AFF_CHARM);

		if(*pet_name) {
			sprintf(buf,"%s %s", pet->player.name, pet_name);
			free(pet->player.name);
			pet->player.name = (char*)strdup(buf);

			sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
					pet->player.description, pet_name);
			free(pet->player.description);
			pet->player.description = (char*)strdup(buf);
		}

		char_to_room(pet, ch->in_room);
		add_follower(pet, ch);

		IS_CARRYING_W(pet) = 0;
		IS_CARRYING_N(pet) = 0;

		send_to_char("May you enjoy your pet.\n\r", ch);
		act("$n bought $N as a pet.",FALSE,ch,0,pet,TO_ROOM);

		return(TRUE);
	}

	/* All commands except list and buy */
	return(FALSE);
}

ROOMSPECIAL_FUNC(Fountain) {

	int bits, water;
	char buf[MAX_INPUT_LENGTH];
	struct char_data* tmp_char;
	char container[20];           /* so we can be flexible */
	struct obj_data* obj;

	void name_to_drinkcon(struct obj_data *obj,int type);
	void name_from_drinkcon(struct obj_data *obj);


	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(cmd==CMD_FILL) {
		/* fill */

		arg = one_argument(arg, buf); /* buf = object */
		bits = generic_find(buf, FIND_OBJ_INV | FIND_OBJ_ROOM |
							FIND_OBJ_EQUIP, ch, &tmp_char, &obj);

		if(!bits) {
			return(FALSE);
		}

		if(ITEM_TYPE(obj) !=ITEM_DRINKCON) {
			send_to_char("Questo non e' un contenitore per liquidi!\n\r", ch);
			return(TRUE);
		}

		if((obj->obj_flags.value[2] != LIQ_WATER) &&
				(obj->obj_flags.value[1] != 0)) {
			name_from_drinkcon(obj);
			obj->obj_flags.value[2] = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);
		}
		else {
			/* Calculate water it can contain */
			water = obj->obj_flags.value[0]-obj->obj_flags.value[1];

			if(water > 0) {
				obj->obj_flags.value[2] = LIQ_WATER;
				obj->obj_flags.value[1] += water;
				weight_change_object(obj, water);
				name_from_drinkcon(obj);
				name_to_drinkcon(obj, LIQ_WATER);
				act("$p e' pieno.", FALSE, ch,obj,0,TO_CHAR);
				act("$n riempie $p con acqua.", FALSE, ch,obj,0,TO_ROOM);
			}
		}
		return(TRUE);

	}
	else if(cmd==CMD_DRINK) {
		/* drink */
		switch(ch->in_room) {
		case 13518:
		case 11014:
		case 5234:
		case 3141:
		case 13406:
			strncpy(container,"fountain",20);
			break;
		case 22642:
		case 22644:
		case 22646:
		case 22648:
			strncpy(container,"brook",20);
			break;
		default:
			strncpy(container,"fontana",20);
		};


		only_argument(arg,buf);

		if(str_cmp(buf, container) && str_cmp(buf, "acqua")) {
			return(FALSE);
		}

		sprintf(buf,"Stai bevendo dalla %s.\n\r",container);
		send_to_char(buf, ch);

		sprintf(buf,"$n beve dalla %s.",container);
		act(buf, FALSE, ch, 0, 0, TO_ROOM);


		GET_COND(ch,THIRST) = 24;

		if(GET_COND(ch,THIRST)>20) {
			act("Ti sei dissetato.",FALSE,ch,0,0,TO_CHAR);
		}

		return(TRUE);
	}

	/* All commands except fill and drink */
	return(FALSE);
}

ROOMSPECIAL_FUNC(bank) {

	static char buf[256];
	int money,tassa;
	float tasso_bancario=0.05;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	money = atoi(arg);

	if(IS_NPC(ch)) {
		return(FALSE);
	}

	save_char(ch, ch->in_room, 0);

	if(GET_BANK(ch) > GetMaxLevel(ch)*40000 && GetMaxLevel(ch)<40) {
		send_to_char("I'm sorry, but we can no longer hold more than 40000 coins per level.\n\r", ch);
		GET_GOLD(ch) += GET_BANK(ch)-GetMaxLevel(ch)*40000;
		GET_BANK(ch) = GetMaxLevel(ch)*40000;
	}


	/*deposit*/
	if(cmd==CMD_DEPOSIT) {
		if(HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
			send_to_char("Your vows forbid you to retain personal wealth\n\r", ch);
			return(TRUE);
		}


		if(money > GET_GOLD(ch)) {
			send_to_char("You don't have enough for that!\n\r", ch);
			return(TRUE);
		}
		else if(money <= 0) {
			send_to_char("Go away, you bother me.\n\r", ch);
			return(TRUE);
		}
		else if((money + GET_BANK(ch) > GetMaxLevel(ch)*40000) &&
				(GetMaxLevel(ch)<40)) {
			send_to_char("I'm sorry, Regulations only allow us to ensure 40000 coins per level.\n\r",ch);
			return(TRUE);
		}
		else {
			send_to_char("La ringraziamo.\n\r",ch);
			GET_GOLD(ch) = GET_GOLD(ch) - money;
			/* inizio procedura calcolo interessi operazione bancaria */
			tassa =(int)(money*tasso_bancario);
			money -= tassa;
			/* termina procedura calcolo interessi operazione bancaria */
			sprintf(buf,"Tassa applicata alla sua operazione: %d monete d'oro.\n\r",tassa);
			send_to_char(buf, ch);
			GET_BANK(ch) = GET_BANK(ch) + money;
			sprintf(buf,"Il suo bilancio attuale e' %d.\n\r", GET_BANK(ch));
			send_to_char(buf, ch);
			return(TRUE);
		}
		/*withdraw*/
	}
	else if(cmd==CMD_WITHDRAW) {

		if(HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
			send_to_char("Your vows forbid you to retain personal wealth\n\r", ch);
			return(TRUE);
		}


		if(money > GET_BANK(ch)) {
			send_to_char("You don't have enough in the bank for that!\n\r", ch);
			return(TRUE);
		}
		else if(money <= 0) {
			send_to_char("Go away, you bother me.\n\r", ch);
			return(TRUE);
		}
		else {
			send_to_char("La ringraziamo.\n\r",ch);
			GET_BANK(ch) = GET_BANK(ch) - money;
			/* inizio procedura calcolo interessi operazione bancaria */
			/*tassa = (int)(money*tasso_bancario);*/
			/*money -= tassa;*/
			/*sprintf(buf,"Tassa applicata alla sua operazione: %d monete d'oro.\n\r",tassa);*/
			/*send_to_char(buf, ch);*/
			/* termina procedura calcolo interessi operazione bancaria */
			GET_GOLD(ch) = GET_GOLD(ch) + money;
			sprintf(buf,"Il suo bilancio attuale e' %d.\n\r", GET_BANK(ch));
			send_to_char(buf, ch);
			return(TRUE);
		}
		/* Balance */
	}
	else if(cmd == CMD_BALANCE) {
		sprintf(buf,"Il suo bilancio attuale e' %d.\n\r", GET_BANK(ch));
		send_to_char(buf, ch);
		return(TRUE);
	}
	return(FALSE);
}



/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */

ROOMSPECIAL_FUNC(pray_for_items) {
	char buf[256];
	int key_room, gold;
	bool found;
	struct obj_data* tmp_obj, *obj;
	struct extra_descr_data* ext;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(cmd != CMD_PRAY) { /* You must pray to get the stuff */
		return FALSE;
	}

	key_room = 1+ch->in_room;

	strcpy(buf, "item_for_");
	strcat(buf, GET_NAME(ch));

	gold = 0;
	found = FALSE;

	for(tmp_obj = real_roomp(key_room)->contents; tmp_obj;
			tmp_obj = tmp_obj->next_content) {

		for(ext = tmp_obj->ex_description; ext; ext = ext->next) {
			if(ext->nMagicNumber == EXDESC_VALID_MAGIC) {
				if(str_cmp(buf, ext->keyword) == 0) {
					if(gold == 0) {
						gold = 1;
						act("$n kneels and at the altar and chants a prayer to Odin.",
							FALSE, ch, 0, 0, TO_ROOM);
						act("You notice a faint light in Odin's eye.",
							FALSE, ch, 0, 0, TO_CHAR);
					}
					obj = read_object(tmp_obj->item_number, REAL);
					obj_to_room(obj, ch->in_room);
					act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_ROOM);
					act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_CHAR);
					gold += obj->obj_flags.cost;
					found = TRUE;
				}
			}
		}
	}


	if(found) {
		GET_GOLD(ch) -= gold;
		GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
		return TRUE;
	}

	return FALSE;
}


/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */



#define CHAL_ACT "You are torn out of reality!\n\rYou roll and tumble through endless voids for what seems like eternity...\n\r\n\rAfter a time, a new reality comes into focus... you are elsewhere.\n\r"


int chalice(struct char_data* ch, int cmd, char* arg) {
	/* 222 is the normal chalice, 223 is chalice-on-altar */

	struct obj_data* chalice;
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
	static int chl = -1, achl = -1;

	if(chl < 1) {
		chl = real_object(222);
		achl = real_object(223);
	}

	switch(cmd) {
	case 10:    /* get */
		if(!(chalice = get_obj_in_list_num(chl,
										   real_roomp(ch->in_room)->contents))
				&& CAN_SEE_OBJ(ch, chalice))
			if(!(chalice = get_obj_in_list_num(achl,
											   real_roomp(ch->in_room)->contents)) && CAN_SEE_OBJ(ch, chalice)) {
				return(0);
			}

		/* we found a chalice.. now try to get us */
		do_get(ch, arg, cmd);
		/* if got the altar one, switch her */
		if(chalice == get_obj_in_list_num(achl, ch->carrying)) {
			extract_obj(chalice);
			chalice = read_object(chl, VIRTUAL);
			obj_to_char(chalice, ch);
		}
		return(1);
		break;
	case 67: /* put */
		if(!(chalice = get_obj_in_list_num(chl, ch->carrying))) {
			return(0);
		}

		argument_interpreter(arg, buf1, buf2);
		if(!str_cmp(buf1, "chalice") && !str_cmp(buf2, "altar")) {
			extract_obj(chalice);
			chalice = read_object(achl, VIRTUAL);
			obj_to_room(chalice, ch->in_room);
			send_to_char("Ok.\n\r", ch);
		}
		return(1);
		break;
	case 176: /* pray */
		if(!(chalice = get_obj_in_list_num(achl,
										   real_roomp(ch->in_room)->contents))) {
			return(0);
		}

		do_action(ch, arg, cmd);  /* pray */
		send_to_char(CHAL_ACT, ch);
		extract_obj(chalice);
		act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, 2500);   /* before the fiery gates */
		do_look(ch, "", 15);
		return(1);
		break;
	default:
		return(0);
		break;
	}
}




int kings_hall(struct char_data* ch, int cmd, char* arg) {
	if(cmd != CMD_PRAY) {
		return(0);
	}

	do_action(ch, arg, CMD_PRAY);

	send_to_char("You feel as if some mighty force has been offended.\n\r", ch);
	send_to_char(CHAL_ACT, ch);
	act("$n is struck by an intense beam of light and vanishes.",
		TRUE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, 1420);  /* behind the altar */
	do_look(ch, "", 15);
	return(1);
}



/*
**  donation room
*/
ROOMSPECIAL_FUNC(Donation) {
	char check[40];

	if(type != EVENT_COMMAND || (cmd != CMD_GET && cmd != CMD_TAKE)) {
		return(FALSE);
	}

	one_argument(arg, check);

	if(*check) {
		if(strncmp(check, "all", 3)==0) {
			send_to_char("Su su, non siamo avidi!\n\r", ch);
			return(TRUE);
		}
	}
	return(FALSE);
}

/*
 * house routine for saved items.
*/

ROOMSPECIAL_FUNC(House) {
	struct obj_cost cost;
	int save_room;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(IS_NPC(ch)) {
		return(FALSE);
	}

	/* if (cmd != rent) ignore */
	if(cmd != CMD_RENT) {
		return(FALSE);
	}

	/*  verify the owner */
	if(!strstr(real_roomp(ch->in_room)->specparms,GET_NAME(ch))) {
		send_to_char("Mi spiace, ma non sei in casa tua!\n\r",ch);
		if(!HAS_PRINCE(ch) ||
				!strstr(real_roomp(ch->in_room)->specparms,GET_PRINCE(ch))) {
			send_to_char("E nemmeno in casa del tuo Principe!!\n\r",ch);
			return(FALSE);
		}
		else {
			send_to_char("Pero' sei in casa del tuo Principe!!\n\r",ch);
		}

	}
	DontShow=1;
	if(recep_offer(ch,NULL, &cost,0)) {
		cost.total_cost=(int)(cost.total_cost*(IS_PRINCE(ch)?0.8:0.9));
		act("Il maggiordomo mette le tue cose nella cassaforte e "
			"ti accompagna in stanza.",
			FALSE, ch, 0, ch, TO_VICT);
		act("Il maggiordomo accompagna $n nella sua stanza.",FALSE,ch,0,ch,TO_NOTVICT);
		save_obj(ch, &cost,1);
		save_room = ch->in_room;
		if(ch->specials.start_room != 2) { /* hell */
			ch->specials.start_room = save_room;
		}
		extract_char(ch);  /* you don't delete CHARACTERS when you extract
them */
		save_char(ch, save_room, 0);
		ch->in_room = save_room;
	}
	else {
		act("Il maggiordomo ti dice: 'Stai scherzando? Non puoi farlo in queste condizioni!",
			FALSE, ch, 0, ch, TO_VICT);
		act("Il maggiordomo dice qualcosa a $n.",FALSE,ch,0,ch,TO_NOTVICT);
	}

	return TRUE;
}

/***********************************************************************
* SaveRoomContens
* Salva tutti gli oggetti presenti nella stanza se questa ha il flag
* SAVE_ROOM. Dovrebbe venir chiamata solo se i comandi sono CMD_GET,
* CMD_TAKE o CMD_DROP.
***********************************************************************/
int SaveRoomContens(struct char_data* pChar, int nCmd, char* pchArg,
					struct room_data* pRoom, int nEvent) {
	return FALSE;
}

/***********************************************************************

                           CHESSBOARD PROCS

 ***********************************************************************/

#define SISYPHUS_MAX_LEVEL 15

/* This is the highest level of PC that can enter.  The highest level
   monster currently in the section is 14th.  It should require a fairly
   large party to sweep the section. */

MOBSPECIAL_FUNC(sisyphus) {
	static int b=1;  /* use this as a switch, to avoid double challenges */

	if(cmd) {

		if(cmd <= CMD_DOWN && cmd >= CMD_NORTH && IS_PC(ch)) {
			if(b) {
				b = 0;
				do_look(mob, GET_NAME(ch), 0);
			}
			else {
				b = 1;
			}
			if((ch->in_room == Ivory_Gate) && (cmd == CMD_WEST)) {
				if((SISYPHUS_MAX_LEVEL < GetMaxLevel(ch)) &&
						(GetMaxLevel(ch) < IMMORTALE)) {
					if(!check_soundproof(ch)) {
						act("$n ti ferma minaccioso e dice: "
							"'Se vuoi entrare dovrai passare sul mio corpo.'", TRUE,
							mob, 0, ch, TO_VICT);
					}
					act("Una smorfia cattiva di $n ti gela il sangue.", TRUE, mob, 0,
						ch, TO_VICT);
					return(TRUE);
				}
			}
			return(FALSE);
		} /* cmd 1 - 6 */

		/* doorbash */
		if(cmd == CMD_DOORBASH) {
			act("$n dice: 'Non credo che ci riuscirai.'",FALSE,mob,0,ch,TO_VICT);
			return(TRUE);
		}

		return(FALSE);
	}
	else {
		if(ch->specials.fighting) {
			if((GET_POS(ch) < POSITION_FIGHTING) &&
					(GET_POS(ch) > POSITION_STUNNED)) {
				StandUp(ch);
			}
			else {
				switch(number(1,10)) {
				case 1:
					do_say(ch, "heal", 0);
					break;

				case 2:
					do_say(ch, "pzar", 0);
					break;

				default:
					FighterMove(ch);
					break;
				}
			}
			return(FALSE);
		}
	}
	return(FALSE);
} /* end sisyphus */


MOBSPECIAL_FUNC(jabberwocky) {
	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		if((GET_POS(ch) < POSITION_FIGHTING) &&
				(GET_POS(ch) > POSITION_STUNNED)) {
			StandUp(ch);
		}
		else {
			FighterMove(ch);
		}
		return(FALSE);
	}
	return FALSE;
}

MOBSPECIAL_FUNC(flame) {
	if(cmd) {
		return(FALSE);
	}
	if(ch->specials.fighting) {
		if((GET_POS(ch) < POSITION_FIGHTING) &&
				(GET_POS(ch) > POSITION_STUNNED)) {
			StandUp(ch);
		}
		else {
			FighterMove(ch);
		}
		return(FALSE);
	}
	return FALSE;
}

MOBSPECIAL_FUNC(banana) {
	if(!cmd) {
		return(FALSE);
	}

	if((cmd >= 1) && (cmd <= 6) &&
			(GET_POS(ch) == POSITION_STANDING) &&
			(!IS_NPC(ch))) {
		if(!saves_spell(ch, SAVING_PARA)) {
			act("$N tries to leave, but slips on a banana and falls.",
				TRUE, ch, 0, ch, TO_NOTVICT);
			act("As you try to leave, you slip on a banana.",
				TRUE, ch, 0, ch, TO_VICT);
			GET_POS(ch) = POSITION_SITTING;
			return(TRUE); /* stuck */
		}
		return(FALSE);        /* he got away */
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(paramedics) {
	struct char_data* vict, *most_hurt;

	if(!cmd) {
		if(ch->specials.fighting) {
			return(cleric(ch, 0, "",mob,type));
		}
		else {
			if(GET_POS(ch) == POSITION_STANDING) {

				/* Find a dude to do good things upon ! */

				most_hurt = real_roomp(ch->in_room)->people;
				for(vict = real_roomp(ch->in_room)->people; vict;
						vict = vict->next_in_room) {
					if(((float)GET_HIT(vict)/(float)hit_limit(vict) <
							(float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt))
							&& (CAN_SEE(ch, vict))) {
						most_hurt = vict;
					}
				}
				if(!most_hurt) {
					return(FALSE);    /* nobody here */
				}

				if((float)GET_HIT(most_hurt)/(float)hit_limit(most_hurt) >
						0.66) {
					if(number(0,5)==0) {
						act("$n shrugs helplessly in unison.", 1, ch, 0, 0, TO_ROOM);
					}
					return TRUE;        /* not hurt enough */
				}

				if(!check_soundproof(ch)) {
					if(number(0,4)==0) {
						if(most_hurt != ch) {
							act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
							act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
						}
						if(check_nomagic(ch, 0, 0)) {
							return(FALSE);
						}

						act("$n utters the words 'judicandus dies' in unison.",
							1, ch, 0, 0, TO_ROOM);
						cast_cure_light(GetMaxLevel(ch), ch, "",
										SPELL_TYPE_SPELL, most_hurt, 0);
						return(TRUE);
					}
				}
			}
			else {  /* I'm asleep or sitting */
				return(FALSE);
			}
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(jugglernaut) {
	struct obj_data* tmp_obj;
	int i, j;

	if(cmd) {
		return(FALSE);
	}

	if(GET_POS(ch) == POSITION_STANDING) {

		if(random()%3) {
			return FALSE;
		}

		/* juggle something */

		if(IS_CARRYING_N(ch) < 1) {
			return FALSE;
		}

		i = random()%IS_CARRYING_N(ch);
		j = 0;
		for(tmp_obj = ch->carrying; (tmp_obj) && (j < i); j++) {
			tmp_obj = tmp_obj->next_content;
		}

		if(random()%6) {
			if(random()%2) {
				act("$n tosses $p high into the air and catches it.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
			}
			else {
				act("$n sends $p whirling.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
			}
		}
		else {
			act("$n tosses $p but fumbles it!", TRUE, ch, tmp_obj, NULL, TO_ROOM);
			obj_from_char(tmp_obj);
			obj_to_room(tmp_obj, ch->in_room);
		}
		return(TRUE);        /* don't move, I dropped something */
	}
	else  {   /* I'm asleep or sitting */
		return FALSE;
	}
	return(FALSE);
}
#if 0
static char* elf_comm[] = {
	"wake", "yawn",
	"stand", "say Well, back to work.", "get all",
	"eat bread", "wink",
	"w", "w", "s", "s", "s", "d", "open gate", "e",  /* home to gate*/
	"close gate",
	"e", "e", "e", "e", "n", "w", "n", /* gate to baker */
	"give all.bread baker", /* pretend to give a bread */
	"give all.pastry baker", /* pretend to give a pastry */
	"say That'll be 33 coins, please.",
	"echo The baker gives some coins to the Elf",
	"wave",
	"s", "e", "n", "n", "e", "drop all.bread", "drop all.pastry",
	"w", "s", "s", /* to main square */
	"s", "w", "w", "w", "w", /* back to gate */
	"pat sisyphus",
	"open gate", "w", "close gate", "u", "n", "n", "n", "e", "e", /* to home */
	"say Whew, I'm exhausted.", "rest", "$"
};
#endif

MOBSPECIAL_FUNC(delivery_elf) {
#define ELF_INIT     0
#define ELF_RESTING  1
#define ELF_GETTING  2
#define ELF_DELIVERY 3
#define ELF_DUMP 4
#define ELF_RETURN_TOWER   5
#define ELF_RETURN_HOME    6

	if(cmd) {
		return(FALSE);
	}

	return(FALSE);

	if(ch->specials.fighting) {
		return FALSE;
	}

	switch(ch->generic) {

	case ELF_INIT:
		if(ch->in_room == 0) {
			/* he has been banished to the Void */
		}
		else if(ch->in_room != Elf_Home) {
			if(GET_POS(ch) == POSITION_SLEEPING) {
				do_wake(ch, "", 0);
				do_stand(ch, "", 0);
			}
			do_say(ch, "Woah! How did i get here!", 0);
			do_emote(ch, "waves his arm, and vanishes!", 0);
			char_from_room(ch);
			char_to_room(ch, Elf_Home);
			do_emote(ch, "arrives with a Bamf!", 0);
			do_emote(ch, "yawns", 0);
			do_sleep(ch, "", 0);
			ch->generic = ELF_RESTING;
		}
		else {
			ch->generic = ELF_RESTING;
		}
		return(FALSE);
		break;
	case ELF_RESTING: {
		if((time_info.hours > 6) && (time_info.hours < 9)) {
			do_wake(ch, "", 0);
			do_stand(ch, "", 0);
			ch->generic = ELF_GETTING;
		}
		return(FALSE);
	}
	break;

	case ELF_GETTING: {
		do_get(ch, "all.loaf", 0);
		do_get(ch, "all.biscuit", 0);
		ch->generic = ELF_DELIVERY;
		return(FALSE);
	}
	break;
	case ELF_DELIVERY: {
		if(ch->in_room != Bakery) {
			int        dir;
			dir = choose_exit_global(ch->in_room, Bakery, -100);
			if(dir<0) {
				ch->generic = ELF_INIT;
				return(FALSE);
			}
			else {
				go_direction(ch, dir);
			}
		}
		else {
			do_give(ch, "6*biscuit baker", 0);
			do_give(ch, "6*loaf baker", 0);
			do_say(ch, "That'll be 33 coins, please.", 0);
			ch->generic = ELF_DUMP;
		}
		return(FALSE);
	}
	break;
	case ELF_DUMP: {
		if(ch->in_room != Dump)   {
			int        dir;
			dir = choose_exit_global(ch->in_room, Dump, -100);
			if(dir<0) {
				ch->generic = ELF_INIT;
				return(FALSE);
			}
			else {
				go_direction(ch, dir);
			}
		}
		else {
			do_drop(ch, "10*biscuit", 0);
			do_drop(ch, "10*loaf", 0);
			ch->generic = ELF_RETURN_TOWER;
		}
		return(FALSE);
	}
	break;
	case ELF_RETURN_TOWER: {
		if(ch->in_room != Ivory_Gate)   {
			int        dir;
			dir = choose_exit_global(ch->in_room, Ivory_Gate, -200);
			if(dir<0) {
				ch->generic = ELF_INIT;
				return(FALSE);
			}
			else {
				go_direction(ch, dir);
			}
		}
		else {
			ch->generic = ELF_RETURN_HOME;
		}
		return(FALSE);
	}
	break;
	case ELF_RETURN_HOME:
		if(ch->in_room != Elf_Home)   {
			int        dir;
			dir = choose_exit_global(ch->in_room, Elf_Home, -200);
			if(dir<0) {
				ch->generic = ELF_INIT;
				return(FALSE);
			}
			else {
				go_direction(ch, dir);
			}
		}
		else {
			if(time_info.hours > 21) {
				do_say(ch, "Done at last!", 0);
				do_sleep(ch, "", 0);
				ch->generic = ELF_RESTING;
			}
			else {
				do_say(ch, "An elf's work is never done.", 0);
				ch->generic = ELF_GETTING;
			}
		}
		return(FALSE);
		break;
	default:
		ch->generic = ELF_INIT;
		return(FALSE);
	}
}



MOBSPECIAL_FUNC(delivery_beast) {
	struct obj_data* o;

	if(cmd) {
		return(FALSE);
	}

	return(FALSE);

	if(time_info.hours == 6) {
		do_drop(ch, "all.loaf",0);
		do_drop(ch, "all.biscuit", 0);
	}
	else if(time_info.hours < 2) {
		if(!number(0,1)) {
			o = read_object(3012, VIRTUAL);
			obj_to_char(o, ch);
		}
		else {
			o = read_object(3013, VIRTUAL);
			obj_to_char(o, ch);
		}
	}
	else {
		if(GET_POS(ch) > POSITION_SLEEPING) {
			do_sleep(ch, "", 0);
		}
	}
}

MOBSPECIAL_FUNC(Keftab) {
	int found, targ_item;
	struct char_data* i;

	if(cmd) {
		return(FALSE);
	}

	if(!ch->specials.hunting) {
		/* find a victim */

		for(i = character_list; i; i = i->next) {
			if(!IS_NPC(ch)) {
				targ_item = SWORD_ANCIENTS;
				found = FALSE;
				while(!found) {
					if((HasObject(i, targ_item))&&(GetMaxLevel(i) < 30)) {
						AddHated(ch, i);
						SetHunting(ch, i);
						return(TRUE);
					}
					else {
						targ_item++;
						if(targ_item > SWORD_ANCIENTS+20) {
							found = TRUE;
						}
					}
				}
			}
		}
	}
	else {

		/* check to make sure that the victim still has an item */
		found = FALSE;
		targ_item = SWORD_ANCIENTS;
		while(!found) {
			if(HasObject(ch->specials.hunting, targ_item)) {
				return(FALSE);
			}
			else {
				targ_item++;
				if(targ_item > SWORD_ANCIENTS+20) {
					found = TRUE;
				}
			}
		}
		ch->specials.hunting = 0;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(StormGiant) {
	struct char_data* vict;

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		if((GET_POS(ch) < POSITION_FIGHTING) &&
				(GET_POS(ch) > POSITION_STUNNED)) {
			StandUp(ch);
		}
		else {
			if(number(0, 5)) {
				fighter(ch, cmd, arg,mob,type);
			}
			else {
				act("$n creates a lightning bolt", TRUE, ch, 0,0,TO_ROOM);
				if((vict = FindAHatee(ch)) == NULL) {
					vict = FindVictim(ch);
				}
				if(!vict) {
					return(FALSE);
				}
				cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

				/* do nothing */
			}
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(Manticore) {
	return(FALSE);
}

MOBSPECIAL_FUNC(Kraken) {
	return(FALSE);
}


MOBSPECIAL_FUNC(fighter) {

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		if(GET_POS(ch) == POSITION_FIGHTING) {
			FighterMove(ch);
		}
		else {
			StandUp(ch);
		}
		FindABetterWeapon(ch);
	}
	return(FALSE);
}





/*
**  NEW THALOS MOBS:******************************************************
*/


#define NTMOFFICE  13554
#define NTMNGATE   13422
#define NTMEGATE   13431
#define NTMSGATE   13413
#define NTMWGATE   13423

#define NTMWMORN    0
#define NTMSTARTM   1
#define NTMGOALNM   2
#define NTMGOALEM   3
#define NTMGOALSM   4
#define NTMGOALWM   5
#define NTMGOALOM   6
#define NTMWNIGHT   7
#define NTMSTARTN   8
#define NTMGOALNN   9
#define NTMGOALEN   10
#define NTMGOALSN   11
#define NTMGOALWN   12
#define NTMGOALON   13
#define NTMSUSP     14
#define NTM_FIX     15

MOBSPECIAL_FUNC(NewThalosMayor) {

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		return(FALSE);
	}
	else {
		switch(ch->generic) {
		/* state info */
		case NTMWMORN:  /* wait for morning */
			if(time_info.hours == 6) {
				ch->generic = NTMGOALNM;
				return(FALSE);
			}
			break;
		case NTMGOALNM: { /* north gate */
			if(ch->in_room != NTMNGATE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMNGATE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				/*
				 * unlock and open door.
				 */
				do_unlock(ch, " gate", 0);
				do_open(ch, " gate", 0);
				ch->generic = NTMGOALEM;
			}
			return(FALSE);
		}
		break;
		case NTMGOALEM: {
			if(ch->in_room != NTMEGATE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMEGATE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				/*
				 * unlock and open door.
				 */
				do_unlock(ch, " gate", 0);
				do_open(ch, " gate", 0);
				ch->generic = NTMGOALSM;
			}
			return(FALSE);
		}
		case NTMGOALSM: {
			if(ch->in_room != NTMSGATE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMSGATE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				/*
				 * unlock and open door.
				 */
				do_unlock(ch, " gate", 0);
				do_open(ch, " gate", 0);
				ch->generic = NTMGOALWM;
			}
			return(FALSE);
		}
		case NTMGOALWM: {
			if(ch->in_room != NTMWGATE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMWGATE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				/*
				 * unlock and open door.
				 */
				do_unlock(ch, " gate", 0);
				do_open(ch, " gate", 0);
				ch->generic = NTMGOALOM;
			}
			return(FALSE);
		}
		case NTMGOALOM: {
			if(ch->in_room != NTMOFFICE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMOFFICE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				ch->generic = NTMWNIGHT;
			}
			return(FALSE);
		}
		case NTMWNIGHT:  /* go back to wait for 7pm */
			if(time_info.hours == 19) {
				ch->generic = NTMGOALNN;
			}
		case NTMGOALNN: { /* north gate */
			if(ch->in_room != NTMNGATE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMNGATE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				/*
				 * lock and open door.
				 */
				do_lock(ch, " gate", 0);
				do_close(ch, " gate", 0);
				ch->generic = NTMGOALEN;
			}
			return(FALSE);
			break;
		}
		case NTMGOALEN:       {
			if(ch->in_room != NTMEGATE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMEGATE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				/*
				 * lock and open door.
				 */
				do_lock(ch, " gate", 0);
				do_close(ch, " gate", 0);
				ch->generic = NTMGOALSN;
			}
			return(FALSE);
		}
		case NTMGOALSN:       {
			if(ch->in_room != NTMSGATE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMSGATE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				/*
				 * lock and open door.
				 */
				do_lock(ch, " gate", 0);
				do_close(ch, " gate", 0);
				ch->generic = NTMGOALWN;
			}
			return(FALSE);
		}
		case NTMGOALWN:       {
			if(ch->in_room != NTMWGATE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMWGATE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				/*
				 * unlock and open door.
				 */
				do_lock(ch, " gate", 0);
				do_close(ch, " gate", 0);
				ch->generic = NTMGOALOM;
			}
			return(FALSE);
		}
		case NTMGOALON:      {
			if(ch->in_room != NTMOFFICE) {
				int        dir;
				dir = choose_exit_global(ch->in_room, NTMOFFICE, -100);
				if(dir<0) {
					ch->generic = NTM_FIX;
					return(FALSE);
				}
				else {
					go_direction(ch, dir);
				}
			}
			else {
				ch->generic = NTMWMORN;
			}
			return(FALSE);
			break;
		}
		case NTM_FIX: {
			/*
			 * move to correct spot (office)
			 */
			do_say(ch, "Woah! How did i get here!", 0);
			char_from_room(ch);
			char_to_room(ch, NTMOFFICE);
			ch->generic = NTMWMORN;
			return(FALSE);
			break;
		}
		default: {
			ch->generic = NTM_FIX;
			return(FALSE);
			break;
		}
		}
	}
	return FALSE;
}

MOBSPECIAL_FUNC(SultanGuard) {
	return(GenericCityguard(ch,cmd,arg,mob,NEWTHALOS));
}


MOBSPECIAL_FUNC(NewThalosCitizen) {
	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(ch->specials.fighting) {
		fighter(ch, cmd, arg, mob, type);

		if(!check_soundproof(ch)) {

			if(number(0,18) == 0) {
				do_shout(ch, "Guards! Help me! Please!", 0);
			}
			else {
				act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
			}

			if(ch->specials.fighting) {
				CallForGuard(ch, ch->specials.fighting, 3, NEWTHALOS);
			}

			return(TRUE);
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(NewThalosGuildGuard) {

	if(!cmd) {
		if(ch->specials.fighting) {
			return(fighter(ch, cmd, arg, mob, type));
		}
	}
	else {
		if(cmd >= 1 && cmd <= 6) {
			switch(ch->in_room) {
			case 13532:
				return(CheckForBlockedMove(ch,cmd,arg,13532,2,CLASS_THIEF));
				break;
			case 13512:
				return(CheckForBlockedMove(ch, cmd, arg, 13512, 2, CLASS_CLERIC));
				break;
			case 13526:
				return(CheckForBlockedMove(ch,cmd, arg, 13526, 2, CLASS_WARRIOR));
				break;
			case 13525:
				return(CheckForBlockedMove(
						   ch, cmd, arg, 13525,0, CLASS_MAGIC_USER|CLASS_SORCERER));
				break;
			}
		}
	}
	return(FALSE);
}

/*
 * New improved magic_user
 */


MOBSPECIAL_FUNC(magic_user2) {
	struct char_data* vict;
	byte lspell;

	if(cmd || !AWAKE(ch)) {
		return(FALSE);
	}

	if(!ch->specials.fighting) {
		return FALSE;
	}

	if((GET_POS(ch) > POSITION_STUNNED) &&
			(GET_POS(ch) < POSITION_FIGHTING)) {
		StandUp(ch);
		return(TRUE);
	}

	if(check_nomagic(ch, 0, 0)) {
		return(FALSE);
	}

	vict = ch->specials.fighting;

	if(!vict) {
		vict = FindVictim(ch);
	}

	if(!vict) {
		return(FALSE);
	}

	lspell = number(0,GetMaxLevel(ch)); /* gen number from 0 to level */

	if(lspell < 1) {
		lspell = 1;
	}

	if((vict!=ch->specials.fighting) && (lspell>19) && (!IS_AFFECTED(ch, AFF_FIRESHIELD))) {
		act("$n utters the words 'Fireshield'.", 1, ch, 0, 0, TO_ROOM);
		cast_fireshield(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		return TRUE;
	}

	switch(lspell) {
	case 1:
		act("$n utters the words 'Magic Missile'.", 1, ch, 0, 0, TO_ROOM);
		cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 2:
		act("$n utters the words 'Shocking Grasp'.", 1, ch, 0, 0, TO_ROOM);
		cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 3:
	case 4:
		act("$n utters the words 'Chill Touch'.", 1, ch, 0, 0, TO_ROOM);
		cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 5:
		act("$n utters the words 'Burning Hands'.", 1, ch, 0, 0, TO_ROOM);
		cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 6:
		if(!IS_AFFECTED(vict, AFF_SANCTUARY)) {
			act("$n utters the words 'Dispel Magic'.", 1, ch, 0, 0, TO_ROOM);
			cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		}
		else {
			act("$n utters the words 'Chill Touch'.", 1, ch, 0, 0, TO_ROOM);
			cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		}
		break;
	case 7:
		act("$n utters the words 'Ice Storm'.", 1, ch, 0, 0, TO_ROOM);
		cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 8:
		act("$n utters the words 'Blindness'.", 1, ch, 0, 0, TO_ROOM);
		cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 9:
		act("$n utters the words 'Fear'.", 1, ch, 0, 0, TO_ROOM);
		cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 10:
	case 11:
		act("$n utters the words 'Lightning Bolt'.", 1, ch, 0, 0, TO_ROOM);
		cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 12:
	case 13:
		act("$n utters the words 'Color Spray'.", 1, ch, 0, 0, TO_ROOM);
		cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 14:
		act("$n utters the words 'Cone Of Cold'.", 1, ch, 0, 0, TO_ROOM);
		cast_cone_of_cold(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
		act("$n utters the words 'Fireball'.", 1, ch, 0, 0, TO_ROOM);
		cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;

	default:
		act("$n utters the words 'frag'.", 1, ch,0,0,TO_ROOM);
		cast_meteor_swarm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;

	}

	return TRUE;

}


/******************Mordilnia citizens************************************/

MOBSPECIAL_FUNC(MordGuard) {
	return(GenericCityguardHateUndead(ch,cmd,arg,mob,MORDILNIA));
}

MOBSPECIAL_FUNC(MordGuildGuard) {

	if(!cmd) {
		if(ch->specials.fighting) {
			return(fighter(ch, cmd, arg, mob, type));
		}
	}
	else {
		if(cmd >= 1 && cmd <= 6) {
			switch(ch->in_room) {
			case 18266:
				return(CheckForBlockedMove(
						   ch,cmd,arg,18266,2,CLASS_MAGIC_USER|CLASS_SORCERER));
				break;
			case 18276:
				return(CheckForBlockedMove(ch, cmd, arg, 18276, 2, CLASS_CLERIC));
				break;
			case 18272:
				return(CheckForBlockedMove(ch, cmd, arg, 18272, 2, CLASS_THIEF));
				break;
			case 18256:
				return(CheckForBlockedMove(ch, cmd, arg, 18256, 0, CLASS_WARRIOR));
				break;
			}
		}
		else {
			return(FALSE);
		}
	}
	return(FALSE);
}


MOBSPECIAL_FUNC(CaravanGuildGuard) {

	if(!cmd) {
		if(ch->specials.fighting) {
			return(fighter(ch, cmd, arg, mob, type));
		}
	}
	else {
		if(cmd >= 1 && cmd <= 6) {
			switch(ch->in_room) {
			case 16115:
				return(CheckForBlockedMove(
						   ch,cmd,arg,16115,1,CLASS_MAGIC_USER|CLASS_SORCERER));
				break;
			case 16126:
				return(CheckForBlockedMove(ch, cmd, arg, 16116, 1, CLASS_CLERIC));
				break;
			case 16117:
				return(CheckForBlockedMove(ch, cmd, arg, 16117, 3, CLASS_THIEF));
				break;
			case 16110:
				return(CheckForBlockedMove(ch, cmd, arg, 16110, 3, CLASS_WARRIOR));
				break;
			}
		}
		else {
			return(FALSE);
		}
	}
	return(FALSE);
}

MOBSPECIAL_FUNC(StatTeller) {
	int choice;
	char buf[200];

	if(cmd) {
		if(cmd == CMD_BUY) {  /* buy */

			/*
			** randomly tells a player 3 of his/her stats.. for a price
			*/
			if(GET_GOLD(ch)< 1000) {
				send_to_char("You do not have the money to pay me.\n\r", ch);
				return(TRUE);
			}
			else {
				GET_GOLD(ch)-=1000;
			}

			choice = number(0,2);
			switch(choice) {
			case 0:
				sprintf(buf, "STR: %d, WIS: %d, DEX: %d\n\r", GET_STR(ch), GET_WIS(ch), GET_DEX(ch));
				send_to_char(buf, ch);
				break;
			case 1:
				sprintf(buf, "INT: %d, DEX:  %d, CON: %d \n\r", GET_INT(ch), GET_DEX(ch), GET_CON(ch));
				send_to_char(buf, ch);
				break;
			case 2:
				sprintf(buf, "CON: %d, INT: %d , WIS: %d \n\r", GET_CON(ch), GET_INT(ch), GET_WIS(ch));
				send_to_char(buf, ch);
				break;
			default:
				send_to_char("We are experiencing Technical difficulties\n\r", ch);
				return(TRUE);
			}

		}
		else {
			return(FALSE);
		}
	}
	else {

		/*
		**  in combat, issues a more potent curse.
		*/

		if(ch->specials.fighting) {
			act("$n gives you the evil eye!  You feel your hitpoints ebbing away",
				FALSE, ch, 0, ch->specials.fighting, TO_VICT);
			act("$n gives $N the evil eye!  $N seems weaker!",
				FALSE, ch, 0, ch->specials.fighting, TO_NOTVICT);
			ch->specials.fighting->points.max_hit -= 10;
			ch->specials.fighting->points.hit -= 10;
			return(FALSE);
		}

	}
	return(FALSE);
}

void ThrowChar(struct char_data* ch, struct char_data* v, int dir) {
	struct room_data* rp;
	int _or;
	char buf[200];

	rp = real_roomp(v->in_room);
	if(rp && rp->dir_option[dir] &&
			rp->dir_option[dir]->to_room &&
			(EXIT(v, dir)->to_room != NOWHERE)) {
		if(v->specials.fighting) {
			stop_fighting(v);
		}
		sprintf(buf, "%s picks you up and throws you %s\n\r",
				ch->player.short_descr, dirs[dir]);
		send_to_char(buf,v);
		_or = v->in_room;
		char_from_room(v);
		char_to_room(v,(real_roomp(_or))->dir_option[dir]->to_room);
		do_look(v, "\0",15);

		if(IS_SET(RM_FLAGS(v->in_room), DEATH) &&
				GetMaxLevel(v) < IMMORTALE) {
			NailThisSucker(v);
		}
	}
}

MOBSPECIAL_FUNC(ThrowerMob) {
	struct char_data* vict;

	/*
	**  Throws people in various directions
	*/

	if(!cmd) {
		if(AWAKE(ch) && ch->specials.fighting) {
			/*
			**  take this person and throw them
			*/
			vict = ch->specials.fighting;
			switch(ch->in_room) {
			case 13912:
				ThrowChar(ch, vict, 1);  /* throw chars to the east */
				return(FALSE);
				break;
			default:
				return(FALSE);
			}
		}
	}
	else {
		switch(ch->in_room) {
		case 13912: {
			if(cmd == CMD_NORTH) {    /* north+1 */
				send_to_char("The Troll blocks your way.\n",ch);
				return(TRUE);
			}
			break;
		}
		default:
			return(FALSE);
		}
	}
	return(FALSE);
}



#if 0
/*
Smart thief special
*/

Thief(struct char_data* ch, char* arg, ind cmd, struct char_data* mob, int type) {

	if(cmd || !AWAKE(ch)) {
		return;
	}

}

#endif



/*
Swallower special
*/

MOBSPECIAL_FUNC(Tyrannosaurus_swallower) {
	struct obj_data* co, *o;
	struct char_data* targ;
	struct room_data* rp;
	int i;

	if(cmd && cmd != CMD_STEAL) {
		return(FALSE);
	}

	if(cmd == CMD_STEAL) {
		send_to_char("You're much too afraid to steal anything!\n\r", ch);
		return(TRUE);
	}

	/*
	** damage stuff
	*/
	DestroyedItems = 0;

	DamageStuff(ch, SPELL_ACID_BLAST, 100, 5);

	if(DestroyedItems) {
		act("$n lets off a real rip-roarer!", FALSE, ch, 0, 0, TO_ROOM);
		DestroyedItems = 0;
	}

	/*
	**  swallow
	*/

	if(AWAKE(ch)) {
		if((targ = FindAnAttacker(ch)) != NULL) {
			act("$n opens $s gaping mouth", TRUE, ch, 0, 0, TO_ROOM);
			if(!CAN_SEE(ch, targ)) {
				if(saves_spell(targ, SAVING_PARA)) {
					act("$N barely misses being swallowed whole!",
						FALSE, ch, 0, targ, TO_NOTVICT);
					act("You barely avoid being swallowed whole!",
						FALSE, ch, 0, targ, TO_VICT);
					return(FALSE);
				}
			}
			if(!saves_spell(targ, SAVING_PARA)) {
				act("In a single gulp, $N is swallowed whole!",
					TRUE, ch, 0, targ, TO_ROOM);
				send_to_char("In a single gulp, you are swallowed whole!\n\r", targ);
				send_to_char("The horror!  The horror!\n\r", targ);
				send_to_char("MMM.  yum!\n\r", ch);
				/* kill target: */
				GET_HIT(targ) = 0;
				alter_hit(targ,0);
				mudlog(LOG_PLAYERS, "%s killed by being swallowed whole",
					   GET_NAME(targ));
				die(targ, 0, NULL);
				/*
				  all stuff to monster:  this one is tricky.  assume that corpse is
				  top item on item_list now that corpse has been made.
				  */
				rp = real_roomp(ch->in_room);
				if(!rp) {
					return(FALSE);
				}
				for(co = rp->contents; co; co = co->next_content) {
					if(IS_CORPSE(co)) {
						/* assume 1st corpse is victim's */
						while(co->contains) {
							o = co->contains;
							obj_from_obj(o);
							obj_to_char(o, ch);
							if(ITEM_TYPE(o) == ITEM_POTION) {
								/* do the effects of the potion: */
								for(i=1; i<4; i++)
									if(o->obj_flags.value[i] >= 1)
										((*spell_info[o->obj_flags.value[i]].spell_pointer)
										 ((byte) o->obj_flags.value[0], ch, "",
										  SPELL_TYPE_POTION, ch, o));
								extract_obj(o);

							}
						}
						extract_obj(co);  /* remove the corpse */
						return(TRUE);
					}
				}
			}
			else {
				act("$N barely misses being swallowed whole!", FALSE, ch, 0, targ, TO_NOTVICT);
				act("You barely avoid being swallowed whole!", FALSE, ch, 0, targ, TO_VICT);
			}
		}
	}
	return FALSE;
}


OBJSPECIAL_FUNC(enter_obj) {
	char obj_key[80], chiave[100];
	int numero;


	if(type != EVENT_COMMAND) {
		return(FALSE);
	}

	if(cmd != CMD_ENTER) {
		return(FALSE);
	}

	arg = one_argument(arg,obj_key);

	if(!*obj_key) {
		return(FALSE);
	}
	char* p=chiave;
	sscanf(obj_index[obj->item_number].specparms,"%100s %d",p,&numero);

	if((ch) && (ch->specials.fighting)) {
		send_to_char("Non mentre combatti!\n\r",ch);
		return(FALSE);
	}

	if(!strcmp(obj_key,chiave)) {
		send_to_char("\n\r",ch);
		act("$c0008Entri in $p...il tuo corpo si dissolve...si ricompone...e ti trovi altrove.$c0007",
			FALSE, ch, obj, 0, TO_CHAR);
		send_to_char("\n\r",ch);
		act("$c0008$n entra in $p e il suo corpo si dissolve velocemente.$c0007",
			FALSE, ch, obj, 0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch,numero);
		do_look(ch, "", 15);
		act("$c0008$n compare all'improvviso dal nulla.$c0007",
			FALSE, ch, obj, 0, TO_ROOM);
	}
	else {
		return(FALSE);
	}

	return(TRUE);
}

OBJSPECIAL_FUNC(zone_obj) {
	char buf[255];
	int numero;
	int room_da, room_a;

	//if (type != EVENT_COMMAND) return(FALSE);

	sscanf(obj_index[obj->item_number].specparms,"%d %d",&room_da,&room_a);

	numero = room_of_object(obj);

	if((numero<room_da) || (numero>room_a)) {
		send_to_room("\n\r", numero);
		sprintf(buf, "$c0006Un'arcana forza fa dissolvere %s nel nulla.\n\r$c0007", obj->short_description);
		send_to_room(buf, numero);
		send_to_room("\n\r", numero);
		if(obj->carried_by) {
			obj_from_char(obj);
		}
		else if(obj->equipped_by) {
			obj = unequip_char(obj->equipped_by, obj->eq_pos);
		}
		extract_obj(obj);
	}

	return(FALSE);
}

OBJSPECIAL_FUNC(soap) {
	struct char_data* t;
	char dummy[80], name[80];

	if(type != EVENT_COMMAND) {
		return(FALSE);
	}

	if(cmd != CMD_USE) {
		return(FALSE);
	}

	if(!(obj = ch->equipment[HOLD])) {
		return(FALSE);
	}
	if(obj->item_number < 0 || reinterpret_cast<objspecial_func>(obj_index[obj->item_number].func) != soap) {
		return(FALSE);
	}

	arg = one_argument(arg, dummy);
	if(!(*dummy)) {
		return(FALSE);
	}
	only_argument(arg, name);
	if(!(*name)) {
		return(FALSE);
	}

	t = get_char_room_vis(ch, name);
	if(!t) {
		return(FALSE);
	}

	if(affected_by_spell(t,SPELL_WEB)) {
		affect_from_char(t,SPELL_WEB);
		act("$n washes some webbing off $N with $p.",TRUE,ch,obj,t,TO_ROOM);
		act("You wash some webbing off $N with $p.",FALSE,ch,obj,t,TO_CHAR);
	}
	else {
		act("$n gives $N a good lathering with $p.",TRUE,ch,obj,t,TO_ROOM);
		act("You give $N a good lathering with $p.",FALSE,ch,obj,t,TO_CHAR);
	}

	obj->obj_flags.value[0]--;
	if(!obj->obj_flags.value[0]) {
		act("That used up $p.",FALSE,ch,obj,t,TO_CHAR);
		extract_obj(obj);
	}
	return TRUE;
}

void String_mob(struct char_data* ch, struct char_data* vict, const char* stringa, int campo) {
	switch(campo) {
	case 1: //short_descr
		ch->desc->str = &vict->player.short_descr;
		break;
	case 2: //long_descr
		ch->desc->str = &vict->player.long_descr;
		break;
	case 3: //description
		ch->desc->str = &vict->player.description;
		break;
	case 4: //name
		ch->desc->str = &vict->player.name;
		break;
	default:
		break;
	}

	if(*ch->desc->str) {
		free(*ch->desc->str);
	}
	CREATE(*ch->desc->str, char, strlen(stringa) + 1);
	strcpy(*ch->desc->str, stringa);
	ch->desc->str = 0;
}

void RakdaGraphic(struct char_data* ch, struct char_data* vict1) {
	int i;
	char buf[250];
	struct char_data* vict2;

//send_to_all("graphic.\n\r");

	i = number(1,60);

	if(!(vict2=vict1)) {
		vict2 = (struct char_data*)FindAnyVictim(ch);
	}

	switch(i) {
	case 1:
		send_to_room("$c0013All'improvviso ti ritrovi in un ameno campo fiorito, ricoperto di violette e mughetti!$c0007\n\r", ch->in_room);
		break;
	case 2:
		if(vict2) {
			act("$c0013Un girasole spunta rapidamente sulla testa di $n. $c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Un girasole ti spunta rapidamente sulla testa$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 3:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Santamagicatabale, ti trasformo in un maiale.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Un maiale",1);
			String_mob(ch,vict1,"Un grosso maiale emana un tremendo odore. ",2);
			String_mob(ch,vict1,"Un grosso maiale rosa e abbastanza sporco. ",3);
			String_mob(ch,vict1,"maiale grosso",4);
		}
		break;
	case 4:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Issu Zuko Bale Fuko, ti trasformo in un eunuco.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Un eunuco",1);
			String_mob(ch,vict1,"Un eunuco si aggira con lo sguardo terrorizzato.",2);
			String_mob(ch,vict1,"Un omone con la vocina da soprano. ",3);
			String_mob(ch,vict1,"eunuco",4);
		}
		break;
	case 5:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Cika Cika Wone Wone, ti trasformo in un piccione.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Un piccione",1);
			String_mob(ch,vict1,"Un piccione grigio cammina sgraziatamente. ",2);
			String_mob(ch,vict1,"Il classico piccione brutto, sgraziato e rumoroso. ",3);
			String_mob(ch,vict1,"piccione",4);
		}
		break;
	case 6:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Tisherami suona pilla, ti trasformo in un'anguilla.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Un'anguilla",1);
			String_mob(ch,vict1,"Una viscida anguilla si dimena. ",2);
			String_mob(ch,vict1,"Lunga e scivolosa, BLEAHH!! ",3);
			String_mob(ch,vict1,"anguilla",4);
		}
		break;
	case 7:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Sgh sgh sgh babbule', ti trasformo in un perche'?$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Un punto interrogativo",1);
			String_mob(ch,vict1,"Un grosso punto interrogativo animato si muove con fare perplesso. ",2);
			String_mob(ch,vict1,"Ma come, non hai mai visto un '?' camminare?? ",3);
			String_mob(ch,vict1,"perche punto interrogativo",4);
		}
		break;
	case 8:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Inde Gusso Strate Gosso, ti trasformo in un osso.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Una tibia gigante",1);
			String_mob(ch,vict1,"Una tibia gigante cammina da sola. ",2);
			String_mob(ch,vict1,"Per fortuna che non c'e' il resto dello scheletro... ",3);
			String_mob(ch,vict1,"osso tibia gigante",4);
		}
		break;
	case 9:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Ajaja' Trallalla', ti trasformo in un baccala'.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Un baccala'",1);
			String_mob(ch,vict1,"Un baccala' appesta l'ambiente. ",2);
			String_mob(ch,vict1,"Magari e' meglio mangiarlo che guardarlo o annusarlo! ",3);
			String_mob(ch,vict1,"baccala",4);
		}
		break;
	case 10:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Erjugandebesgorosgo, ti trasformo in un rospo.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Un grosso rospo",1);
			String_mob(ch,vict1,"Un grosso rospo ti guarda con i suoi occhioni. ",2);
			String_mob(ch,vict1,"Ti guarda con i suoi occhioni tutti pupille sognanti.  ",3);
			String_mob(ch,vict1,"rospo grosso",4);
		}
		break;
	case 11:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Faha Tame Geno Rino, ti trasformo in un pinguino.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Un pinguino",1);
			String_mob(ch,vict1,"Un pinguino procede con la sua andatura barcollante. ",2);
			String_mob(ch,vict1,"Cammina un po' con difficolta' ma e' tanto simpatico!  ",3);
			String_mob(ch,vict1,"pinguino",4);
		}
		break;
	case 12:
		if(vict1 && IS_NPC(vict1) && !IS_POLY(vict1)) {
			act("$c0013Alga Malga Toregalpa, ti trasformo in una talpa.$c0007", TRUE, vict1, 0,0,TO_ROOM);
			String_mob(ch,vict1,"Una piccola talpa",1);
			String_mob(ch,vict1,"Un piccola talpa si muove incerta e a tastoni. ",2);
			String_mob(ch,vict1,"Praticamente non vede un cavolo...pero' sottoterra e' una trivella!!  ",3);
			String_mob(ch,vict1,"talpa piccola",4);
		}
		break;
	case 13:
		if(vict2) {
			act("$c0013Una folta barba bianca spunta sul mento di $n.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Una folta barba bianca ti spunta sul mento.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 14:
		send_to_room("$c0013Mille campanelli dorati e infiocchettati iniziano a suonare convulsamente...DLIN DLIN DLIN!!$c0007\n\r", ch->in_room);
		break;
	case 15:
		if(vict2) {
			act("$c0013$n apre le enormi fauci e ruggisce con rabbia!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Apri le enormi fauci e ruggisci con rabbia!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 16:
		if(vict2) {
			act("$c0013$n inizia a scodinzolare con la sua coda spelacchiata.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Inizi a scodinzolare con la tua coda spelacchiata.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 17:
		if(vict2) {
			act("$c0013Un grosso polpo arriva improvvisamente in faccia a $n!!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Un grosso polpo ti arriva improvvisamente in faccia!!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 18:
		send_to_room("$c0013Alcuni salmoni iniziano a risalire la corrente.$c0007\n\r", ch->in_room);
		break;
	case 19:
		if(vict2) {
			act("$c0013Una nuvoletta di piccole farfalle fuorisce dalla bocca di $n.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Una nuvoletta di piccole farfalle fuorisce dalla tua bocca.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 20:
		send_to_room("$c0013Ismeria la panettiera ti prende alle spalle e ti fa il solletico ai fianchi!$c0007\n\r", ch->in_room);
		break;
	case 21:
		if(vict2) {
			act("$c0013Uno stormo di mosche bianche morte precipita su $n!!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Uno stormo di mosche bianche morte precipita su di te!!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 22:
		if(vict2)
			if((vict1 = get_char_vis(ch, "niobe"))) {
				sprintf(buf, "shout %s, la smetti di lasciare i tuoi perizomi sul mio bancone??", GET_NAME(vict2));
				command_interpreter(vict1, buf);
			}
		break;
	case 23:
		if(vict2) {
			act("$c0013Una minacciosa incudine appare sulla testa di $n, ma poi scompare!!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Una minacciosa incudine appare sulla tua testa, ma poi scompare!!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 24:
		if(vict2) {
			act("$c0013$n inizia a sibilare mostrando la sua lingua biforcuta da serpente.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Inizi a sibilare mostrando la tua lingua biforcuta da serpente.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 25:
		if(vict2) {
			act("$c0013Due grandi orecchie da mammouth compaiono sventolando sulla testa di $n!!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Due grandi orecchie da mammouth compaiono sventolando sulla tua testa!!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 26:
		send_to_room("$c0013Il suono di una campana attrae la tua attenzione.$c0007\n\r", ch->in_room);
		break;
	case 27:
		send_to_room("$c0013All'improvviso senti un formicolio su tutto il corpo...Cavolo, sei ricoperto di formiche!!$c0007\n\r", ch->in_room);
		break;
	case 28:
		send_to_room("$c0013Un gufo arriva da est, si posa su un palo e ti guarda insistentemente...che vorra' significare?$c0007\n\r", ch->in_room);
		break;
	case 29:
		send_to_room("$c0013Una compagnia di bardi compare all'improvviso e inizia a scuonare una marcia nuziale.$c0007\n\r", ch->in_room);
		break;
	case 30:
		if(vict2) {
			act("$c0013Sulla schiena di $n spuntano due enormi ali di pipistrello.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Sulla tua schiena spuntano due enormi ali di pipistrello.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 31:
		if(vict2) {
			act("$c0013Il terreno sotto i piedi di $n inizia a prendere fuoco!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Il terreno sotto i tuoi piedi inizia a prendere fuoco!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 32:
		if(vict2) {
			act("$c0013Grosse macchie rosa compaiono sul corpo di $n.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Grosse macchie rosa compaiono sul tuo corpo.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 33:
		if(vict2) {
			act("$c0013Due braccia pelose compaiono a $n e impugnando una lira iniziano a suonare.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Ti compaiono due braccia pelose e impugnando una lira inizi a suonare.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 34:
		if(vict2) {
			act("$c0013$n inizia ad agitare sinuosamente le sue rotondita'...$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Inizi ad agitare sinuosamente le tue rotondita'...$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 35:
		if(vict2) {
			act("$c0013I piedi di $n crescono a dismisura e poi si sgonfiano!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013I tuoi piedi crescono a dismisura e poi si sgonfiano!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 36:
		if(vict2) {
			act("$c0013Un piccolo treant arriva saltellando agilmente, si avvicina a $n e gli stringe la mano!?!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Un piccolo treant arriva saltellando agilmente, ti si avvicina e ti stringe la mano!?!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 37:
		send_to_room("$c0013Una grossa tartaruga arriva sonnecchiando, si guarda intorno, sbarra gli occhi e scappa urlando terrorizzata...$c0007\n\r", ch->in_room);
		break;
	case 38:
		send_to_room("$c0013Un piccolo riccio attraversa la stanza rotolando su se stesso e ridendo a crepapelle!!$c0007\n\r", ch->in_room);
		break;
	case 39:
		send_to_room("$c0013Un bardo vestito di giallo e verde cade dall'alto, si rialza ridendo e inizia a danzare in tondo!$c0007\n\r", ch->in_room);
		break;
	case 40:
		if(vict2) {
			act("$c0013Una figura incappucciata si avvicina a $n e con gli occhi semichiusi lo indica sospettosamente...$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Una figura incappucciata ti si avvicina e con gli occhi semichiusi ti indica sospettosamente...$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 41:
		if(vict2) {
			sprintf(buf, "gos Quasi quasi mi faccio un bagno, sono 3 mesi che non lo faccio...mmmm forse e' ancora presto.");
			command_interpreter(vict2, buf);
		}
		break;
	case 42:
		if(vict2) {
			sprintf(buf, "gos Oggi sono proprio in splendida forma eh? Guardate che roba...eccezionale direi!!");
			command_interpreter(vict2, buf);
		}
		break;
	case 43:
		if(vict2) {
			sprintf(buf, "gos Ehi, un asino con le ali sopra le nostre teste!!");
			command_interpreter(vict2, buf);
		}
		break;
	case 44:
		if(vict2) {
			sprintf(buf, "gos Firuli', firuli', firulaaaaa', ce ne andiamo a passeggiaaaaar...");
			command_interpreter(vict2, buf);
		}
		break;
	case 45:
		if(vict2) {
			sprintf(buf, "gos Maaaaaa....2+2 fa veramente 4??");
			command_interpreter(vict2, buf);
		}
		break;
	case 46:
		if(vict2) {
			sprintf(buf, "gos Andiamo ad Arachnos a eliminare un po' di ragnacci?");
			command_interpreter(vict2, buf);
		}
		break;
	case 47:
		if(vict2) {
			sprintf(buf, "gos Beh, penso che me ne andro' a dormire, sono molto stanco");
			command_interpreter(vict2, buf);
		}
		break;
	case 48:
		if(vict2) {
			sprintf(buf, "gos Qualcuno puo' astralare per favore?");
			command_interpreter(vict2, buf);
		}
		break;
	case 49:
		if(vict2) {
			sprintf(buf, "gos Avete visto quella lucertola con la testa di maiale?? Pazzesca!!");
			command_interpreter(vict2, buf);
		}
		break;
	case 50:
		if(vict2) {
			sprintf(buf, "gos Ehi, vi voglio bene, sapete? Voi me ne volete?");
			command_interpreter(vict2, buf);
		}
		break;
	case 51:
		if(vict2) {
			act("$c0013Piccoli giochi pirotecnici escono dalle orecchie di $n!!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Piccoli giochi pirotecnici escono dalle tue orecchie!!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 52:
		send_to_room("$c0013Un nano di fosso all'improvviso ti si attacca ad una gamba e, tremando, implora pieta'!$c0007\n\r", ch->in_room);
		break;
	case 53:
		if(vict2) {
			act("$c0013Un'anatra muschiata si posa sulla testa di $n e depone un uovo.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Un'anatra muschiata ti si posa sulla testa e depone un uovo.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 54:
		if(vict2) {
			act("$c0013Un vecchio scarpone di pelle marrone arriva in faccia a $n!!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Un vecchio scarpone di pelle marrone ti arriva in faccia!!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 55:
		if(vict2) {
			act("$c0013Una grossa trota atterra sulla nuca di $n!!$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Una grossa trota atterra sulla tua nuca!!$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 56:
		send_to_room("$c0013All'improvviso vedi in lontananza Lord Soth vestito con un candido abito di seta bianca e fiori rossi che saluta ad ampie bracciate!!$c0007\n\r", ch->in_room);
		break;
	case 57:
		if(vict2) {
			act("$c0013La testa di un unicorno si avvicina a $n e lo lecca vigorosamente.$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013La testa di un unicorno ti si avvicina e ti lecca vigorosamente.$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 58:
		if(vict2) {
			act("$c0013Una tetra figura si avvicina a $n e gli dice: 'Ricordati...che devi morire...'$c0007", TRUE, vict2, 0,0,TO_ROOM);
			act("$c0013Una tetra figura ti si avvicina e ti dice: 'Ricordati...che devi morire...'$c0007", TRUE, vict2, 0,0,TO_CHAR);
		}
		break;
	case 59:
		send_to_room("$c0013Dal terreno spunta fuori una talpa che interdetta chiede: 'Che c' sta mo'gghiema?$c0007\n\r", ch->in_room);
		break;
	case 60:
		if(vict2) {
			sprintf(buf, "gos Che bello se fossi uno gnomo, qualcuno puo' reincarnarmi?");
			command_interpreter(vict2, buf);
		}
		break;

	default:
		mudlog(LOG_SYSERR, "Strangeness in Rakda Graphic", i);
	} /* end switch */
}

void RakdaMinor(struct char_data* ch, struct char_data* vict) {
	int i;
	struct affected_type af;

//  send_to_all("minor.\n\r");

	i=number(1,10);
	switch(i) {
	case 1:
		act("$c0013Dagli occhi di $n spuntano dei lunghi e folti peli neri.$c0007", TRUE, vict, 0,0,TO_ROOM);
		act("$c0013Dai tuoi occhi spuntano dei lunghi e folti peli neri.$c0007", TRUE, vict, 0,0,TO_CHAR);
		cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 2:
		act("$c0013Dalla bocca di $n iniziano a sgorgare rivoli d'acqua.$c0007", TRUE, vict, 0,0,TO_ROOM);
		act("$c0013Dalla tua bocca iniziano a sgorgare rivoli d'acqua.$c0007", TRUE, vict, 0,0,TO_CHAR);
		cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 3:
		act("$c0013Alume' alume', la vera vista venga a te.$c0007", TRUE, ch, 0,0,TO_CHAR);
		cast_true_seeing(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		break;
	case 4:
		act("$c0013Il libro delle magiche burle inizia a volare sopra di te sbattendo le pagine.$c0007", TRUE, ch, 0,0,TO_CHAR);
		act("$c0013Uno strano libro colorato inizia a volare sopra $n sbattendo le pagine.$c0007", TRUE, ch, 0,0,TO_ROOM);
		cast_globe_minor_inv(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		break;
	case 5:
		act("$c0013Dalle narici di $n esce talmente tanto fumo che non lo si vede quasi piu'!$c0007", TRUE, ch, 0,0,TO_ROOM);
		act("$c0013Dalle tue narici esce talmente tanto fumo che non ti vedi quasi piu'!$c0007", TRUE, ch, 0,0,TO_CHAR);
		if(!affected_by_spell(ch, SPELL_MIRROR_IMAGES)) {
			for(i=2; i; i--) {
				af.type      = SPELL_MIRROR_IMAGES;
				af.duration  = number(1,4)+(GetMaxLevel(ch)/5);
				af.modifier  = 0;
				af.location  = APPLY_NONE;
				af.bitvector = 0;
				affect_to_char(ch, &af);
			}
		}
		break;
	case 6:
		act("$c0013Dalla manica di $n esce un serpente che morde $N!$c0007", TRUE, ch, 0,vict,TO_NOTVICT);
		act("$c0013Dalla manica di $n esce un serpente che ti morde!$c0007", TRUE, ch, 0,vict,TO_VICT);
		act("$c0013Dalla tua manica esce un serpente che morde $N!$c0007", TRUE, ch, 0,vict,TO_CHAR);
		cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 7:
		act("$c0013Una pagina di Rakda si stacca e si posa sulle tue ferite.$c0007", TRUE, ch, 0,0,TO_CHAR);
		act("$c0013Una strana pagina colorata svolazzando si posa sulle ferite di $n.$c0007", TRUE, ch, 0,0,TO_ROOM);
		cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		break;
	case 8:
		act("$c0013Attorno a $n crescono decine di biancospini che lo ostacolano$c0007", TRUE, vict, 0,0,TO_ROOM);
		act("$c0013Attorno a te crescono decine di biancospini che ti ostacolano$c0007", TRUE, vict, 0,0,TO_CHAR);
		GET_MOVE(vict) -=10;
		alter_move(vict,0);
		break;
	case 9:
		act("$c0013Un folletto con un pennello rosa esalta i contorni di $n!$c0007", TRUE, vict, 0,0,TO_ROOM);
		act("$c0013Un folletto con un pennello rosa esalta i tuoi contorni!$c0007", TRUE, vict, 0,0,TO_CHAR);
		cast_faerie_fire(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 10:
		act("$c0013$n mastica un fungo e nuova magia fluisce in lui.$c0007", TRUE, ch, 0,0,TO_ROOM);
		act("$c0013Mastichi un fungo e nuova magia fluisce in te.$c0007", TRUE, ch, 0,0,TO_CHAR);
		GET_MANA(ch) +=100;
		alter_mana(ch,0);
		break;
	default:
		mudlog(LOG_SYSERR, "Strangeness in RakdaMinor (%d)", i);
	} /* end switch */
}

void RakdaMajor(struct char_data* ch, struct char_data* vict) {
	int i;
	int amnt = 300;
	struct affected_type af;
	struct room_data* pRoom;

//  send_to_all("major.\n\r");

	i=number(1,5);
	switch(i) {
	case 1:
		act("$c0013Un enorme piede calloso si materializza e schiaccia $n!!$c0007", TRUE, vict, 0,0,TO_ROOM);
		act("$c0013Un enorme piede calloso si materializza e ti schiaccia!!$c0007", TRUE, vict, 0,0,TO_CHAR);

		amnt = SkipImmortals(vict, amnt,TYPE_BLUDGEON);
		if(amnt == -1) {
			return;
		}

		if(IS_AFFECTED(vict, AFF_SANCTUARY)) {
			amnt = MAX((int)(amnt/2), 0);
		}

		amnt = PreProcDam(vict, TYPE_BLUDGEON, amnt, -1);
		DamageStuff(vict, TYPE_BLUDGEON, amnt, 5);
		amnt=MAX(amnt,0);
		GET_HIT(vict)-=amnt;
		alter_hit(vict,0);
		update_pos(vict);
		break;
	case 2:
		if(!IsHumanoid(ch)) {
			return;
		}
		act("$c0013Due grosse catene si materializzano alle caviglie di $n!!$c0007", TRUE, vict, 0,0,TO_ROOM);
		act("$c0013Due grosse catene si materializzano alle tue caviglie!!$c0007", TRUE, vict, 0,0,TO_CHAR);

		if(IsImmune(vict, IMM_HOLD)) {
			return;
		}
		if((IsResist(vict, IMM_HOLD)) && (number(1,2)==1)) {
			return;
		}

		GET_MOVE(vict)=0;
		alter_move(vict,0);
		break;
	case 3:
		pRoom =  real_roomp(vict->in_room);

		if((pRoom->sector_type == SECT_MOUNTAIN) || (pRoom->sector_type == SECT_HILLS)) {
			act("$c0013Il terreno si anima e ingloba $n che viene pietrificato!!$c0007", TRUE, vict, 0,0,TO_ROOM);
			act("$c0013Il terreno si anima, ti ingloba e vieni pietrificato!!$c0007", TRUE, vict, 0,0,TO_CHAR);

			if(IsImmune(vict, IMM_HOLD)) {
				return;
			}
			if((IsResist(vict, IMM_HOLD)) && (number(1,2)==1)) {
				return;
			}

			af.type      = SPELL_PARALYSIS;
			af.duration  = 10;
			af.modifier  = 0;
			af.location  = APPLY_NONE;
			af.bitvector = AFF_PARALYSIS;
			affect_join(vict, &af, FALSE, FALSE);

			GET_POS(vict)=POSITION_STUNNED;
		}
		break;
	case 4:
		pRoom =  real_roomp(vict->in_room);

		if((pRoom->sector_type == SECT_FOREST) || (pRoom->sector_type == SECT_FIELD)
				|| (pRoom->sector_type == SECT_AIR) || (pRoom->sector_type == SECT_CITY)) {
			act("$c0013Un'enorme quantita' di pietre incandescenti si addensa sopra $n!!$c0007", TRUE, vict, 0,0,TO_ROOM);
			act("$c0013Un'enorme quantita' di pietre incandescenti si addensa sopra di te!!$c0007", TRUE, vict, 0,0,TO_CHAR);

			cast_meteor_swarm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
			cast_meteor_swarm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		}
		break;
	case 5:
		if(!IsHumanoid(ch)) {
			return;
		}
		act("$c0013La bocca di $n si riempie di fango!!$c0007", TRUE, vict, 0,0,TO_ROOM);
		act("$c0013La tua bocca si riempie di fango!!$c0007", TRUE, vict, 0,0,TO_CHAR);

		af.type      = SPELL_CREEPING_DEATH;
		af.duration  = 1;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = 0;
		affect_to_char(vict, &af);
		break;
	default:
		mudlog(LOG_SYSERR, "Strangeness in RakdaMajor (%d)", i);
	} /* end switch */
}

void RakdaCast(struct char_data* ch, struct char_data* vict) {
	int i;
//  send_to_all("cast.\n\r");

	if((check_nomagic(ch, 0, 0)) || (check_soundproof(ch)) || (check_peaceful(ch, ""))) {
		act("$c0013Rakda inizia a frignare e poi a piangere sommessamente: qui non puo' lanciare incantesimi!$c0007", TRUE, vict, 0,0,TO_CHAR);
		act("$c0013Senti un lontano pianto sommesso...$c0007", TRUE, vict, 0,0,TO_ROOM);
		return;
	}

	i=number(1,10);
	switch(i) {
	case 1:
		cast_slow(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 2:
		cast_disintegrate(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 3:
		cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 4:
		cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 5:
		cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 6:
		cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 7:
		cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 8:
		cast_meteor_swarm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 9:
		cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	case 10:
		cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		break;
	default:
		mudlog(LOG_SYSERR, "Strangeness in RakdaCast (%d)", i);
	} /* end switch */
}

OBJSPECIAL_FUNC(Rakda) {
	struct obj_data* rakda=obj;
	char buf[256], appo[256];
	char_data* mage;
	char_data* vict;
	static int primo = -1;
	int i,j;

	if(primo < 1) {
		if(rakda != NULL) {
			if((mage = char_holding(rakda))) {
				primo = 1;
				send_to_char("$c0013' Buongiorno caro, aaahhhhh speriamo che sia una giornata divertente...'$c0007\n\r",mage);
				return(0);
			}
		}
	}

	//se e' droppato non si attiva
	if(rakda->in_room != NOWHERE) {
		return(0);
	}

	//esegue controlli di EGO una volta su 20, per non appesantire il sistema
	if(number(1,10)==1) {
		mage = char_holding(rakda);
		if((mage) && (!IS_IMMORTAL(mage)) && (!IS_NPC(mage))) {
			//se ne va se non lo possiede un mago
			if(!(HasClass(mage, CLASS_MAGIC_USER) || HasClass(mage,CLASS_SORCERER))) {
				send_to_char("\n\r",mage);
				send_to_char("$c0013' Ehi, ma che ci faccio nelle tue mani? Io voglio un MAGOOOO!!! ADDIO!!! '$c0007\n\r",mage);
				send_to_char("\n\r",mage);
				send_to_char("$c0011' Rakda, $c0013il libro delle magiche burle $c0013scompare dalle tue mani con un sonoro PUFFF!! '$c0007\n\r",mage);
				send_to_char("\n\r",mage);
				if(rakda->equipped_by) {
					rakda = unequip_char(rakda->equipped_by, rakda->eq_pos);
				}
				else {
					obj_from_char(rakda);
				}

				extract_obj(rakda);
				return(0);
			}
			//se ne va incazzato se lo possiede un evil, pacificamente se un neutrale
			if(IS_EVIL(mage)) {
				send_to_char("\n\r",mage);
				send_to_char("$c0013' Ehi, brutta anima lercia, come osi trattenermi!! ADDIO!!! '$c0007\n\r",mage);
				send_to_char("\n\r",mage);
				send_to_char("$c0011' Rakda, $c0013il libro delle magiche burle $c0013ti si para davanti e ti ESPLODE IN FACCIA!!! '$c0007\n\r",mage);
				send_to_char("\n\r",mage);
				GET_HIT(mage) -= (GetMaxLevel(mage)*2);
				alter_hit(mage,0);
				if(rakda->equipped_by) {
					rakda = unequip_char(rakda->equipped_by, rakda->eq_pos);
				}
				else {
					obj_from_char(rakda);
				}

				extract_obj(rakda);
				return(0);
			}
			if(IS_NEUTRAL(mage)) {
				send_to_char("\n\r",mage);
				send_to_char("$c0013' Mmmmmm...no no no, il tuo animo non mi convince!! ADDIO!!! '$c0007\n\r",mage);
				send_to_char("\n\r",mage);
				send_to_char("$c0011' Rakda, $c0013il libro delle magiche burle $c0013ti guarda storto e scompare con un sonoro PUFF!! '$c0007\n\r",mage);
				send_to_char("\n\r",mage);
				if(rakda->equipped_by) {
					rakda = unequip_char(rakda->equipped_by, rakda->eq_pos);
				}
				else {
					obj_from_char(rakda);
				}

				extract_obj(rakda);
				return(0);
			}
		}
	}

	if(!(mage=rakda->equipped_by)) {
		return(0);
	}
	if(IS_NPC(mage)) {
		return(0);
	}

	//vedo se e' in combattimento o no (lo e' anche se il suo tank lo e')
	if(!(vict = mage->specials.fighting)) {
		if(mage->master) {
			vict = mage->master->specials.fighting;
		}
	}


	//evento pulse, se e' in combat allora agisce. In combat agisce soltanto con evento pulse
	//in modo che sia scandito dal mud e non dal pg
	if(!ch) {
		//in combattimento
		if(vict) {
			if((GET_RACE(vict) == RACE_DEMON) || (GET_RACE(vict) == RACE_DEVIL)) {
				j = number(1,100);
				if(j<=50) {
					i=80;    //spell aggressive
				}
				else if(j<=70) {
					i=100;    //attacchi speciali maggiori
				}
				else {
					i=85;    //attacchi speciali minori
				}
			}
			else {
				i = number(1,100);
			}

			if(i<=40) {
				return(0);
			}
			else if(i<=70) {
				RakdaGraphic(mage,vict);
			}
			else if(i<=80) {
				RakdaCast(mage,vict);
			}
			else if(i<=85) {
				RakdaMinor(mage,vict);
			}
			else if(i<=98) {
				RakdaGraphic(mage,vict);
			}
			else if(IS_EVIL(vict)) {
				RakdaMajor(mage,vict);
			}
			return(0);
		}
		return(0);
	}


	//fuori combattimento e a seguito di comando del pg, quindi ch e' valorizzato
	if(!vict) {
		switch(cmd) {
		case 302:
		case 17:
		case 18:
		case 19:
		case 83:
			arg = one_argument(arg,appo);
			if(!strcmp(appo,"rakda")) {
				i = number(1,4);
				switch(i) {
				case 1:
					send_to_room("\n\r", ch->in_room);
					send_to_room("$c0009[$c0015Rakda$c0009] alza una pagina e grida forte$c0007\n\r", ch->in_room);
					send_to_room("$c0009[$c0015Rakda$c0009] grida 'Rakda nel suo magico splendore vi saluta in allegria!!$c0007\n\r", ch->in_room);
					send_to_room("\n\r", ch->in_room);
					break;
				case 2:
					send_to_room("\n\r", ch->in_room);
					send_to_room("$c0013Una voce ti risuona dentro: 'Uccio Uccia, cappuccio e babbuccia!!'$c0007\n\r", ch->in_room);
					send_to_room("\n\r", ch->in_room);
					break;
				case 3:
					send_to_room("\n\r", ch->in_room);
					send_to_room("$c0013Una scritta compare dal nulla: 'Benvenuti nel magico mondo delle Illusioni!!'$c0007\n\r", ch->in_room);
					send_to_room("\n\r", ch->in_room);
					break;
				case 4:
					send_to_room("\n\r", ch->in_room);
					send_to_room("$c0013All'improvviso nella stanza  compare uno gnometto sorridente'$c0007\n\r", ch->in_room);
					send_to_room("$c0013seduto su un fungone $c0015bianco $c0013e $c0009rosso$c0013, ammicca  e poi scompare'$c0007\n\r", ch->in_room);
					send_to_room("$c0013con un *BOP* in una nuvoletta di fumo dai riflessi argentati.'$c0007\n\r", ch->in_room);
					send_to_room("\n\r", ch->in_room);
					break;
				default:
					break;
				}
			}
			return(0);
			break;
		case 279:   /* sign */
			send_to_room("\n\r", ch->in_room);
			send_to_room("$c0013Improvvisamente compare un enorme cartello di legno con la scritta:$c0011\n\r", ch->in_room);
			send_to_room("\n\r", ch->in_room);
			send_to_room(arg, ch->in_room);
			send_to_room("\n\r \n\r", ch->in_room);
			sprintf(buf, "$c0013firmato:   %s \n\r$c0007",GET_NAME(ch));
			send_to_room(buf, ch->in_room);
			return(1);
			break;
		case 11:    /* drink */
		case 12:    /* eat */
			i = number(1,5);
			if(i == 1) {
				act("Lo stomaco di $n gorgoglia rumorosamente.", FALSE, ch, rakda, 0, TO_ROOM);
				act("Il tuo stomaco gorgoglia rumorosamente.", FALSE, ch, rakda, 0, TO_CHAR);
			}
			return(0);
			break;
		case 281:   /* first aid */
			if(!affected_by_spell(ch, SKILL_FIRST_AID)) {
				if(GET_SEX(ch)==SEX_FEMALE) {
					act("$c0001Uno stupendo satiro seminudo compare dal nulla e avvolge $n con erbe mediche e teneri bacetti.$c0007",
						FALSE, ch, rakda, 0, TO_ROOM);
					act("$c0001Uno stupendo satiro seminudo compare dal nulla e ti avvolge con erbe mediche e teneri bacetti.$c0007",
						FALSE, ch, rakda, 0, TO_CHAR);
				}
				else {
					act("$c0002Una stupenda ninfa seminuda compare dal nulla e avvolge $n con erbe mediche e teneri bacetti.$c0007",
						FALSE, ch, rakda, 0, TO_ROOM);
					act("$c0002Una stupenda ninfa seminuda compare dal nulla e ti avvolge con erbe mediche e teneri bacetti.$c0007",
						FALSE, ch, rakda, 0, TO_CHAR);
				}
			}
			return(0);
			break;
		case 84:   /* cast */
			if(number(1,3)==1) {
				RakdaGraphic(ch, vict);
			}
			return(0);
			break;
		default:
			if(number(1,15)==1) {
				RakdaGraphic(ch, vict);
			}
			return(0);
			break;
		}


	}
	return(0);
}

OBJSPECIAL_FUNC(nodrop) {
	struct char_data* t;
	struct obj_data* xobj, *i;
	char buf[80], obj_name[80], vict_name[80], *name;
	bool do_all;
	int j, num;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	switch(cmd) {
	case CMD_GET: /* Get */
	case CMD_DROP: /* Drop */
	case CMD_GIVE: /* Give */
	case CMD_STEAL: /* Steal */
		break;
	default:
		return(FALSE);
	}


	if(type != EVENT_COMMAND) {
		return(FALSE);
	}

	arg = one_argument(arg, obj_name);
	if(!*obj_name) {
		return(FALSE);
	}

	xobj = 0x0;
	do_all = FALSE;

	if(!(strncmp(obj_name,"all",3))) {
		do_all = TRUE;
		num = IS_CARRYING_N(ch);
	}
	else {
		strcpy(buf,obj_name);
		name = buf;
		if(!(num = get_number(&name))) {
			return(FALSE);
		}
	}

	/* Look in the room first, in get case */
	if(cmd == CMD_GET)  {
		for(i=real_roomp(ch->in_room)->contents,j=1; i&&(j<=num); i=i->next_content) {
			if(i->item_number>=0) {
				if(do_all || isname(name, i->name)) {
					if(do_all || j == num) {
						if(obj_index[i->item_number].func == reinterpret_cast<genericspecial_func>(nodrop)) {
							xobj = i;
							break;
						}
					}
					else {
						++j;
					}
				}
			}
		}
	}
	/* Check the character's inventory for give, drop, steal. */
	if(!xobj) {
		/* Don't bother with get anymore */
		if(cmd == CMD_GET) {
			return(FALSE);
		}
	}
	for(i = ch->carrying, j = 1; i && (j <= num); i = i->next_content) {
		if(i->item_number >= 0) {
			if(do_all || isname(name, i->name)) {
				if(do_all || j == num) {
					if(obj_index[i->item_number].func == reinterpret_cast<genericspecial_func>(nodrop)) {
						xobj = i;
						break;
					}
					else {
						if(!do_all) {
							return (FALSE);
						}
					}
				}
				else {
					++j;
				}
			}
		}
	}
	/* Musta been something else */
	if(!xobj) {
		return(FALSE);
	}

	if(cmd == CMD_GIVE || cmd == CMD_STEAL) {
		only_argument(arg, vict_name);
		if(!*vict_name) {
			return FALSE;
		}
		else if(!(t = get_char_room_vis(ch, vict_name))) {
			return FALSE;
		}
	}
	else {
		t = NULL;    /* Non viene usato per gli altri comandi. */
	}

	switch(cmd) {
	case 10:
		if(GetMaxLevel(ch)<=MAX_MORT) {
			act("$p disintegrates when you try to pick it up!",
				FALSE, ch, xobj, 0, TO_CHAR);
			act("$n tries to get $p, but it disintegrates in his hand!",
				FALSE, ch, xobj, 0, TO_ROOM);
			extract_obj(xobj);
			if(do_all) {
				return(FALSE);
			}
			else {
				return(TRUE);
			}
		}
		else {
			return(FALSE);
		}

	case 60:
		if(!IS_SET(xobj->obj_flags.extra_flags,ITEM_NODROP)) {
			act("You drop $p to the ground, and it shatters!",
				FALSE, ch, xobj, 0, TO_CHAR);
			act("$n drops $p, and it shatters!", FALSE, ch, xobj, 0, TO_ROOM);
			i = read_object(30, VIRTUAL);
			sprintf(buf, "Scraps from %s lie in a pile here.",
					xobj->short_description);
			i->description = (char*)strdup(buf);
			obj_to_room(i, ch->in_room);
			obj_from_char(xobj);
			extract_obj(xobj);
			if(do_all) {
				return(FALSE);
			}
			else {
				return(TRUE);
			}
		}
		else {
			return(FALSE);
		}

	case 72:
		if(!IS_SET(xobj->obj_flags.extra_flags,ITEM_NODROP)) {
			if(GetMaxLevel(ch)<=MAX_MORT) {
				act("You try to give $p to $N, but it vanishes!",
					FALSE, ch, xobj, t, TO_CHAR);
				act("$N tries to give $p to you, but it fades away!",
					FALSE, t, xobj, ch, TO_CHAR);
				act("As $n tries to give $p to $N, it vanishes!",
					FALSE, ch, xobj, t, TO_ROOM);
				extract_obj(xobj);
				if(do_all) {
					return(FALSE);
				}
				else {
					return(TRUE);
				}
			}
			else {
				return(FALSE);
			}
		}
		else {
			return(FALSE);
		}

	case 156: /* Steal */
		if(!IS_SET(xobj->obj_flags.extra_flags,ITEM_NODROP)) {
			act("You cannot seem to steal $p from $N.",
				FALSE, ch, xobj, t, TO_CHAR);
			act("$N tried to steal something from you!",FALSE,t,xobj,ch,TO_CHAR);
			act("$N tried to steal something from $n!",FALSE,t,xobj,ch,TO_ROOM);
			return(TRUE);
		}
		else {
			return(FALSE);
		}

	case CMD_REMOVE:
		if(!IS_SET(xobj->obj_flags.extra_flags,ITEM_NODROP)) {
			act("Oh no, non vorresti mai farlo!",
				FALSE, ch, xobj, t, TO_CHAR);
			return(TRUE);
		}
		else {
			return(FALSE);
		}

	default:
		return(FALSE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(BiosKaiThanatos) {
#define MIN_WEARING 10
	FILE* fdeath;
	char buf[MAX_INPUT_LENGTH];
	struct char_data* god;

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	god = FindMobInRoomWithFunction(ch->in_room, reinterpret_cast<genericspecial_func>(BiosKaiThanatos));

	if(!god) {
		return(FALSE);
	}
	switch(cmd) {
	case CMD_SACRIFICE :
		break;
	case CMD_GIVE :
	case CMD_DROP :
	case CMD_REMOVE :
		if(!IS_MAESTRO_DEL_CREATO(ch)) {
			act("Non davanti a $N!", FALSE,
				ch, 0, god, TO_CHAR);
			return(TRUE);
		}
		else {
			return(FALSE);
		}
	default:
		break;
	}

	if(cmd != CMD_SACRIFICE) {
		return(FALSE);
	}

	if(IS_NPC(ch)) {
		act("$N ti dice 'Non hai dunque il coraggio di mostrare il tuo vero volto?'",
			FALSE,ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		return(TRUE);
	}
	int dummy,result=0;
	WEARING_N(ch,dummy,result);
	if(result<MIN_WEARING) {
		act("$N ti dice 'Torna da me vestito dei tuoi abiti migliori'",
			FALSE,ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		return(TRUE);
	}
	sprintf(buf,"%s/%s.dead",PLAYERS_DIR,lower(GET_NAME(ch)));
	if(!(fdeath=fopen(buf,"r"))) {

		act("$N ti dice 'Impostore! Vattene via subito!'", FALSE,
			ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		mudlog(LOG_CHECK, "No dead file for %s", GET_NAME(ch));
		return(TRUE);
	}
	int xp=0;
	long ora=0;
	fscanf(fdeath,"%d : %ld",&xp,&ora);

	if(((long)time(0)-ora)>(4 * 60 *60)) {

		act("$N ti dice 'Troppo tardi! Il tuo karma si e' compiuto.'", FALSE,
			ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		mudlog(LOG_CHECK, "%s: to late on sacrifice", GET_NAME(ch));
		fclose(fdeath);
		return(TRUE);
	}
	if((GET_RCON(ch)<=3) && (number(1,100)<90)) {

		act("$N ti dice 'Troppe volte hai danzato!'", FALSE,
			ch, 0, god, TO_CHAR);
		act("$N dice qualcosa a $n", FALSE,
			ch, 0, god, TO_ROOM);
		mudlog(LOG_CHECK, "%s: no CON on sacrifice", GET_NAME(ch));
		fclose(fdeath);
		return(TRUE);
	}
	GET_RCON(ch)=MAX(GET_RCON(ch)-1,3);
	act("$N ti dice 'Il karma e' stato benevolo con te!'", FALSE,
		ch, 0, god, TO_CHAR);
	act("$N ti dice 'Il tuo sacrificio e' stato accettato!'", FALSE,
		ch, 0, god, TO_CHAR);
	act("$N dice qualcosa a $n", FALSE,
		ch, 0, god, TO_ROOM);
	GET_EXP(ch)=xp;

	act("$c0001Senti le viscere rivoltarsi, mentre energie sconosciute ti strappano l'anima", FALSE,
		ch, 0, god, TO_CHAR);
	act("$n si solleva da terra e inizia a brillare.", FALSE,
		ch, 0, god, TO_ROOM);
	for(xp=TYPE_GENERIC_FIRST; xp<=TYPE_GENERIC_LAST; xp++) {
#if NOSCRAP
		DamageStuff(ch,xp,200,5);
		DamageStuff(ch,xp,200,5);
		DamageStuff(ch,xp,200,5);
#endif
		DamageStuff(ch,xp,200,5);
	}
	act("$c0001L'urlo di mille gole sgozzate ti assorda", FALSE,
		ch, 0, god, TO_CHAR);
	act("$n si contorce come in agonia, mentre un vento impetuoso si alza dal nulla.", FALSE,
		ch, 0, god, TO_ROOM);
	GET_HIT(ch)=MIN(GET_HIT(ch),10);
	alter_hit(ch,0);
	for(xp=TYPE_GENERIC_FIRST; xp<=TYPE_GENERIC_LAST; xp++) {
#if NOSCRAP
		DamageStuff(ch,xp,200,5);
		DamageStuff(ch,xp,200,5);
		DamageStuff(ch,xp,200,5);
#endif
		DamageStuff(ch,xp,200,5);
	}
	act("Alla fine, giaci a terra spossat$b", FALSE,
		ch, 0, god, TO_CHAR);
	act("$n cade a terra spossato.", FALSE,
		ch, 0, god, TO_ROOM);
	GET_POS(ch)=POSITION_STUNNED;
	mudlog(LOG_CHECK, "%s: sacrifice accepted", GET_NAME(ch));
	fclose(fdeath);
	return TRUE;
}
} // namespace Alarmud

