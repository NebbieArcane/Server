/*$Id: magicutils.c,v 1.2 2002/02/13 12:31:00 root Exp $
*/

#include <stdio.h>
#include <assert.h>

#include "fight.hpp"
#include "protos.hpp"
#include "snew.hpp"

/* Extern structures */
extern struct room_data* world;
extern struct obj_data*  object_list;
extern struct char_data* character_list;


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
