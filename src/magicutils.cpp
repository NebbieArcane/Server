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

void RelateMobToCaster(struct char_data* ch, struct char_data* mob) {

	int char_bonus = GetMaxLevel(ch)/10  ;

	if(HasClass(ch,CLASS_PSI)) {
		char_bonus += 4;
	}
	if(HasClass(ch,CLASS_CLERIC)) {
		char_bonus += 6;
	}
	if(HasClass(ch,CLASS_MAGIC_USER)) {
		char_bonus += 8;
	}
	if(HasClass(ch,CLASS_DRUID)) {
		char_bonus += 10;
	}

	if(HasClass(ch,CLASS_THIEF)) {
		char_bonus -= 2;
	}
	if(HasClass(ch,CLASS_PALADIN)) {
		char_bonus -= 4;
	}
	if(HasClass(ch, CLASS_WARRIOR | CLASS_RANGER |
				CLASS_BARBARIAN | CLASS_MONK)) {
		char_bonus -= 6;
	}

	if(GET_INT(ch) > 17) {
		char_bonus ++;
	}
	if(GET_INT(ch) > 18) {
		char_bonus ++;
	}

	if(GET_RACE(ch) == RACE_TROLL) {
		char_bonus = 0;
	}
	else if(GET_RACE(ch) == RACE_GOLD_ELF) {
		char_bonus += 4;
	}
	else if(GET_RACE(ch) == RACE_DARK_ELF) {
		char_bonus ++;
	}
	else if(GET_RACE(ch) == RACE_WILD_ELF) {
		char_bonus += 2;
	}
	else if(GET_RACE(ch) == RACE_SEA_ELF) {
		char_bonus += 3;
	}
	else if(GET_RACE(ch) == RACE_DEMON) {
		char_bonus += 3;
	}
	else if(GET_RACE(ch) == RACE_MFLAYER) {
		char_bonus += 4;
	}

	/* Requiem 2018 - adjust mob power in relation to caster's level */

	if(char_bonus > 0) {
		if(IS_SET(mob->specials.act, ACT_MONK) || IS_SET(mob->specials.act, ACT_WARRIOR) || IS_SET(mob->specials.act, ACT_THIEF) || IS_SET(mob->specials.act, ACT_BARBARIAN) || IS_SET(mob->specials.act, ACT_PALADIN || IS_SET(mob->specials.act, ACT_RANGER))) {
			mob->points.max_hit += (char_bonus*number(1,5));
			GET_HIT(mob) = GET_MAX_HIT(mob);

            int multiplier=2;

			if(mob->specials.mobtype=='A' || mob->specials.mobtype=='L' || mob->specials.mobtype=='B') {
				multiplier = int(mob->mult_att) * 2;
			}

			/*mob->specials.damsizedice += final_bonus/i;
			mob->specials.damnodice += final_bonus/i;*/
			mob->points.hitroll += char_bonus;
			mob->points.damroll += char_bonus/multiplier;
		}

		if(IS_SET(mob->specials.act, ACT_MAGIC_USER) || IS_SET(mob->specials.act, ACT_DRUID) || IS_SET(mob->specials.act, ACT_CLERIC) || IS_SET(mob->specials.act, ACT_PSI)) {
			mob->points.max_mana = GET_MAX_MANA(mob) + (char_bonus*number(2,3));
			GET_MANA(mob) = GET_MAX_MANA(mob);
			mob->points.mana_gain += (char_bonus*number(3,5));
		}
	}
}
void SwitchStuff(struct char_data* giver, struct char_data* taker) {
	struct obj_data* obj, *next;
	float ratio;
	int j;
    struct affected_type* af;
    struct affected_type af2;


	/*
	 * experience
	 */

	if(!IS_IMMORTAL(taker)) {
		if(!IS_IMMORTAL(giver)) {
			GET_EXP(taker) = GET_EXP(giver);
		}

		GET_EXP(taker) = MIN(GET_EXP(taker), ABS_MAX_EXP);
	}

    /*
     *  switch affects
     */

    for(af = taker->affected; af; af = af->next) {
        if(!affected_by_spell(giver,af->type)) {
            affect_from_char(taker, af->type);
        }
    }

    for(af = giver->affected; af; af = af->next)
    {
        if(IS_NPC(giver) && (af->type == SPELL_POLY_SELF || af->type == SPELL_TREE))
        {
            if(affected_by_spell(taker,af->type))
            {
                affect_from_char(taker, af->type);
            }
        }
        else
        {
            if(!affected_by_spell(taker,af->type))
            {
                af2.type      = af->type;
                af2.duration  = af->duration;
                af2.modifier  = af->modifier;
                af2.location  = af->location;
                af2.bitvector = af->bitvector;

                affect_to_char(taker, &af2);
            }

            if(af->type == STATUS_QUEST)
            {
                taker->specials.quest_ref = giver->specials.quest_ref;
                if(giver->specials.quest_ref)
                {
                    (giver->specials.quest_ref)->specials.quest_ref = taker;
                    giver->specials.quest_ref = NULL;
                }
            }
        }
    }

    if(giver->lastpkill != NULL) {
        free(taker->lastpkill);
        taker->lastpkill = strdup(giver->lastpkill);
    }

    if(giver->lastmkill != NULL) {
        free(taker->lastmkill);
            taker->lastmkill = strdup(giver->lastmkill);
    }

	/*
	 *  humanoid monsters can cast spells
	 */

	if(IS_NPC(taker)) {
		taker->player.iClass = giver->player.iClass;
		if(!taker->skills) {
			SpaceForSkills(taker);
		}
		for(j = 0; j < MAX_SKILLS; j++) {
			taker->skills[ j ].learned = giver->skills[ j ].learned;
			taker->skills[ j ].flags   = giver->skills[ j ].flags;
			taker->skills[ j ].special = giver->skills[ j ].special;
			taker->skills[ j ].nummem  = giver->skills[ j ].nummem;
		}
		for(j = MAGE_LEVEL_IND; j < MAX_CLASS; j++) {
			taker->player.level[ j ] = giver->player.level[ j ];
		}
	}
	else {
		if(taker->skills && giver->skills) {
			for(j = 0; j < MAX_SKILLS; j++) {
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

	for(j = 0; j< MAX_WEAR; j++)
    {
		if(giver->equipment[j])
        {
			obj = unequip_char(giver, j);
            obj_to_char(obj, taker);
		}
	}

	for(obj = giver->carrying; obj; obj = next) {
		next = obj->next_content;
		obj_from_char(obj);
		obj_to_char(obj, taker);
	}

    // rune

    GET_RUNEDEI(taker) = GET_RUNEDEI(giver);

	/*
	 *    gold...
	 */

	GET_GOLD(taker) = GET_GOLD(giver);

	/*
	 *   hit point ratio
	 */

	ratio = (float) GET_HIT(giver) / GET_MAX_HIT(giver);
	GET_HIT(taker) = (short)(ratio * GET_MAX_HIT(taker));

	GET_HIT(taker) = MIN(GET_MAX_HIT(taker), GET_HIT(taker));
	alter_hit(taker,0);

	GET_MANA(taker) = GET_MANA(giver);
	alter_mana(taker,0);
    affect_total(taker);

}

void FailCharm(struct char_data* victim, struct char_data* ch) {
	if(OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50)) {
		return;    /* give single classed mages a break. */
	}

	if(!IS_PC(victim)) {

		AddHated(victim, ch);

		if(!victim->specials.fighting) {
			if(GET_POS(victim) > POSITION_SLEEPING) {
				set_fighting(victim, ch);
			}
			else if(number(0,1)) {
				set_fighting(victim, ch);
			}
		}
	}
	else {
        act("Il tuo tentativo di incantare $N fallisce.", FALSE, ch, 0, victim, TO_CHAR);
		send_to_char("Senti che cercano di incantarti, ma l'effetto passa.\n\r",victim);
	}
}

void FailSnare(struct char_data* victim, struct char_data* ch) {
	if(OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50)) {
		return;    /* give single classed mages a break. */
	}

	if(!IS_PC(victim)) {
		if(!victim->specials.fighting) {
			AddHated(victim,ch);
			set_fighting(victim, ch);
		}
	}
	else {
        act("Provi a bloccare $N, ma fallisci.", FALSE, ch, 0, victim, TO_CHAR);
		send_to_char("Ti senti in trappola, ma la sensazione passa.\n\r",victim);
	}
}

void FailSleep(struct char_data* victim, struct char_data* ch) {
	if(OnlyClass(ch, CLASS_MAGIC_USER|CLASS_SORCERER) &&
			number(1, 100) > 50) {
		return;    /* give single classed mages a break. */
	}

    act("Il tuo tentativo di addormentare $N fallisce.", FALSE, ch, 0, victim, TO_CHAR);
	send_to_char("Hai molto sonno, ma resisti.\n\r",
				 victim);
	if(!IS_PC(victim)) {
		AddHated(victim, ch);
		if(!victim->specials.fighting && GET_POS(victim) > POSITION_SLEEPING) {
			set_fighting(victim, ch);
		}
	}
}


void FailPara(struct char_data* victim, struct char_data* ch) {
	if(OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50)) {
		return;    /* give single classed mages a break. */
	}

    act("Provi a paralizzare $N, ma resiste.", FALSE, ch, 0, victim, TO_CHAR);
	send_to_char("Ti senti paralizzare per un attimo, ma resisti.\n\r",victim);
	if(!IS_PC(victim)) {
		AddHated(victim, ch);
		if((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING)) {
			set_fighting(victim, ch);
		}
	}
}

void FailCalm(struct char_data* victim, struct char_data* ch) {
	if(OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50)) {
		return;    /* give single classed mages a break. */
	}

    act("Provi a calmare la rabbia di $N, ma non ci riesci.", FALSE, ch, 0, victim, TO_CHAR);
	send_to_char("Ti senti felice ed in pace con il mondo, ma passa presto.\n\r",victim);
	if(!IS_PC(victim))
		if(!victim->specials.fighting) {
			if(!number(0,2)) {
				set_fighting(victim, ch);
			}
		}
}


void FailPoison(struct char_data* victim, struct char_data* ch) {
	if(!IS_PC(victim)) {
		if(!OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) || number(1,100) > 50) {
			AddHated(victim, ch);
			if(!victim->specials.fighting) {
				if(GET_POS(victim) > POSITION_SLEEPING || number(0, 1)) {
					set_fighting(victim, ch);
				}
			}
		}
	}
	else {
		send_to_char("Ti gira un momento la testa, ma poi tutto passa.\n\r",
					 victim);
	}
}
} // namespace Alarmud
