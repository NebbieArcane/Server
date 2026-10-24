/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
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
#include "spec_dragon.hpp"
#include "breath.hpp"
#include "act.off.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "magic.hpp"
#include "regen.hpp"
#include "spells.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "utility.hpp"
namespace Alarmud {

/* Draghi e respiri: BreathWeapon, *Breather, draghi boss / iconici. */

static breath_func breaths[] = {
	cast_acid_breath,
	0,
	cast_frost_breath,
	0,
	cast_lightning_breath,
	0,
	cast_fire_breath,
	0,
	cast_acid_breath,
	cast_fire_breath,
	cast_lightning_breath,
	0
};

MOBSPECIAL_FUNC(BreathWeapon) {
	int        count;
	const char* p;
	char p2[255];
	int cost;
	int tipo;
	if(type != EVENT_TICK) {
		return FALSE;
	}


	if(AWAKE(mob) && mob->specials.fighting &&
			mob->specials.fighting->in_room == mob->in_room) {

		p=GET_SPEC_PARM(mob);
		p=one_argument(p,p2);
		cost=abs(atoi(p2));
		p=one_argument(p,p2);
		tipo=abs(atoi(p2));
		tipo=tipo>8?8:tipo;
		for(count=tipo; breaths[count]; count++)
			;

		use_breath_weapon(mob, mob->specials.fighting, cost,
						  breaths[dice(1,count-1)]);

	}

	return (FALSE);
}

MOBSPECIAL_FUNC(FireBreather) {
	struct char_data* tar_char;

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_fire_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		} /* end for */

		return(TRUE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(FrostBreather) {
	struct char_data* tar_char;
	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_frost_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		} /* end for */
		return(TRUE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(AcidBreather) {
	struct char_data* tar_char;
	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_acid_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		}
		return(TRUE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(GasBreather) {
	struct char_data* tar_char;

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_gas_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		}
		return(TRUE);
	}

	return(FALSE);
}

MOBSPECIAL_FUNC(LightningBreather) {
	struct char_data* tar_char;

	if(cmd) {
		return(FALSE);
	}

	if(ch->specials.fighting && number(0,2)) {
		act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
		act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
		for(tar_char=real_roomp(ch->in_room)->people; tar_char; tar_char=tar_char->next_in_room) {
			if(!IS_IMMORTAL(tar_char)) {
				spell_lightning_breath(GetMaxLevel(ch),ch,tar_char,0);
			}
		}
		return(TRUE);
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

MOBSPECIAL_FUNC(DracoLich) {
	return FALSE;
}

} // namespace Alarmud
