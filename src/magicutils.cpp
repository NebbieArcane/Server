/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*$Id: magicutils.c,v 1.2 2002/02/13 12:31:00 root Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cassert>
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
#include "magicutils.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.off.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "magic.hpp"
#include "magic2.hpp"
#include "magicutils.hpp"
#include "modify.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "spell_parser.hpp"
namespace Alarmud {

void RelateMobToCaster( struct char_data* ch, struct char_data* mob ) {
    int i,char_bonus;

        char_bonus = GetMaxLevel(ch)/10  ;

        if (HasClass(ch,CLASS_PSI)) { char_bonus += 4; }
        if (HasClass(ch,CLASS_CLERIC)) { char_bonus += 6; }
        if (HasClass(ch,CLASS_MAGIC_USER)) { char_bonus += 8; }
        if (HasClass(ch,CLASS_DRUID)) { char_bonus += 10; }

        if (HasClass(ch,CLASS_THIEF)) { char_bonus =-2; }
        if (HasClass(ch,CLASS_PALADIN)) { char_bonus =- 4; }
        if (HasClass( mob, CLASS_WARRIOR | CLASS_RANGER |
                         CLASS_BARBARIAN | CLASS_MONK)) { char_bonus -= 6; }

        if (GET_DEX(ch) > 17) { char_bonus ++; }
        if (GET_DEX(ch) > 18) { char_bonus ++; }

        if (GET_RACE(ch) == RACE_TROLL) { char_bonus = 0; }
				else if (GET_RACE(ch) == RACE_GOLD_ELF) { char_bonus += 4; }
                else if (GET_RACE(ch) == RACE_DARK_ELF) { char_bonus ++; }
                else if (GET_RACE(ch) == RACE_WILD_ELF) { char_bonus += 2; }
                else if (GET_RACE(ch) == RACE_SEA_ELF) { char_bonus += 3; }
                else if (GET_RACE(ch) == RACE_DEMON) { char_bonus += 3; }
                else if (GET_RACE(ch) == RACE_MFLAYER) { char_bonus += 4; }

    /* Requiem 2018 - adjust mob power in relation to caster's level */

    if(char_bonus > 0) {
        if( HasClass( mob, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER |
                      CLASS_BARBARIAN | CLASS_MONK | CLASS_THIEF)) {
            mob->points.max_hit = GET_MAX_HIT(mob) + (char_bonus*number(1,5));
            GET_HIT(mob) = GET_MAX_HIT(mob);
            if (mob->specials.mobtype=='A' || mob->specials.mobtype=='L' || mob->specials.mobtype=='B') {
                i = (int)mob->mult_att;
            } else {
                i = 1;
            }

            /*mob->specials.damsizedice += final_bonus/i;
            mob->specials.damnodice += final_bonus/i;*/
            mob->points.hitroll += char_bonus/2;
            mob->points.damroll += (char_bonus/i)*2;
        }

        if( HasClass( mob, CLASS_CLERIC | CLASS_MAGIC_USER | CLASS_DRUID |
                      CLASS_SORCERER | CLASS_PSI)) {
            mob->points.max_mana = GET_MAX_MANA(mob) + (char_bonus*number(2,3));
            GET_MANA(mob) = GET_MAX_MANA(mob);
            mob->points.mana_gain += (char_bonus*number(3,5));
        }
    }
}

void SwitchStuff( struct char_data* giver, struct char_data* taker) {
	struct obj_data* obj, *next;
	float ratio;
	int j;

	/*
	 * experience
	 */

	if (!IS_IMMORTAL(taker)) {
		if (!IS_IMMORTAL(giver))
		{ GET_EXP(taker) = GET_EXP(giver); }

		GET_EXP(taker) = MIN(GET_EXP(taker), ABS_MAX_EXP);
	}

	/*
	 *  humanoid monsters can cast spells
	 */

	if( IS_NPC( taker ) ) {
		taker->player.iClass = giver->player.iClass;
		if( !taker->skills )
		{ SpaceForSkills( taker ); }
		for( j = 0; j < MAX_SKILLS; j++ ) {
			taker->skills[ j ].learned = giver->skills[ j ].learned;
			taker->skills[ j ].flags   = giver->skills[ j ].flags;
			taker->skills[ j ].special = giver->skills[ j ].special;
			taker->skills[ j ].nummem  = giver->skills[ j ].nummem;
		}
		for( j = MAGE_LEVEL_IND; j < MAX_CLASS; j++ ) {
			taker->player.level[ j ] = giver->player.level[ j ];
		}
	}
	else {
		if( taker->skills && giver->skills ) {
			for( j = 0; j < MAX_SKILLS; j++ ) {
				taker->skills[ j ].nummem = giver->skills[ j ].nummem;
			}
		}
	}

	GET_ALIGNMENT(taker) = GET_ALIGNMENT(giver);

	taker->player.user_flags = giver->player.user_flags;
	/*
	 *  take all the stuff from the giver, put in on the
	 *  taker
	 */

	for (j = 0; j< MAX_WEAR; j++) {
		if (giver->equipment[j]) {
			obj = unequip_char(giver, j);
			/* ALAR */
			if (IsHumanoid(taker) )
			{ equip_char(taker,obj,j); }
			else
			{ obj_to_char(obj, taker); }
		}
	}

	for (obj = giver->carrying; obj; obj = next) {
		next = obj->next_content;
		obj_from_char(obj);
		obj_to_char(obj, taker);
	}

	/*
	 *    gold...
	 */

	GET_GOLD(taker) = GET_GOLD(giver);

	/*
	 *   hit point ratio
	 */

	ratio = (float) GET_HIT(giver) / GET_MAX_HIT(giver);
	GET_HIT(taker) = (short)( ratio * GET_MAX_HIT(taker) );

	GET_HIT(taker) = MIN(GET_MAX_HIT(taker), GET_HIT(taker));
	alter_hit(taker,0);

	GET_MANA(taker) = GET_MANA(giver);
	alter_mana(taker,0);

}

void FailCharm(struct char_data* victim, struct char_data* ch) {
	if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
	{ return; }  /* give single classed mages a break. */

	if (!IS_PC(victim)) {

		AddHated(victim, ch);

		if (!victim->specials.fighting) {
			if (GET_POS(victim) > POSITION_SLEEPING)
			{ set_fighting(victim, ch); }
			else if (number(0,1)) {
				set_fighting(victim, ch);
			}
		}
	}
	else {
		send_to_char("Senti che cercano di incantarti, ma l'effetto passa.\n\r",victim);
	}
}

void FailSnare(struct char_data* victim, struct char_data* ch) {
	if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
	{ return; }  /* give single classed mages a break. */

	if (!IS_PC(victim)) {
		if (!victim->specials.fighting) {
			AddHated(victim,ch);
			set_fighting(victim, ch);
		}
	}
	else {
		send_to_char("Ti senti in trappola, ma la sensazione passa.\n\r",victim);
	}
}

void FailSleep(struct char_data* victim, struct char_data* ch) {
	if( OnlyClass( ch, CLASS_MAGIC_USER|CLASS_SORCERER ) &&
			number( 1, 100 ) > 50 )
	{ return; }  /* give single classed mages a break. */

	send_to_char("Hai molto sonno, ma resisti.\n\r",
				 victim);
	if( !IS_PC( victim ) ) {
		AddHated( victim, ch );
		if( !victim->specials.fighting && GET_POS(victim) > POSITION_SLEEPING )
		{ set_fighting(victim, ch); }
	}
}


void FailPara(struct char_data* victim, struct char_data* ch) {
	if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
	{ return; }  /* give single classed mages a break. */

	send_to_char("Ti senti paralizzare per un attimo, ma resisti.\n\r",victim);
	if (!IS_PC(victim)) {
		AddHated(victim, ch);
		if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
		{ set_fighting(victim, ch); }
	}
}

void FailCalm(struct char_data* victim, struct char_data* ch) {
	if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
	{ return; }  /* give single classed mages a break. */

	send_to_char("Ti senti felice ed in pace con il mondo, ma passa presto.\n\r",victim);
	if (!IS_PC(victim))
		if (!victim->specials.fighting) {
			if (!number(0,2))
			{ set_fighting(victim, ch); }
		}
}


void FailPoison(struct char_data* victim, struct char_data* ch) {
	if (!IS_PC(victim)) {
		if( !OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) || number(1,100) > 50 ) {
			AddHated(victim, ch);
			if( !victim->specials.fighting ) {
				if( GET_POS(victim) > POSITION_SLEEPING || number( 0, 1 ) ) {
					set_fighting(victim, ch);
				}
			}
		}
	}
	else {
		send_to_char( "Ti gira un momento la testa, ma poi tutto passa.\n\r",
					  victim);
	}
}
} // namespace Alarmud

