/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdlib>
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
#include "spec_abyss.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "opinion.hpp"
#include "spec_procs.hpp"
#include "spell_parser.hpp"
#include "spells1.hpp"
#include "spells2.hpp"
#include "utility.hpp"

namespace Alarmud {

/* Procedure speciali dell'Abisso. */

namespace {

constexpr int kSwordAncients = 25000;
constexpr int kSwordAncientsLast = kSwordAncients + 20;
constexpr int kHuntMaxLevel = 30;

/** Room 25003: The End of the Trail — ingresso abisso e' down. */
constexpr int kAbyssTrailEnd = 25003;
/** Room 25011: The Gate — uscita verso 25013 e' north. */
constexpr int kAbyssGateRoom = 25011;

bool abyss_gatekeeper_blocks(const struct char_data* mob, int cmd) {
	if(mob == nullptr) {
		return false;
	}
	if(cmd == CMD_DOWN && mob->in_room == kAbyssTrailEnd) {
		return true;
	}
	if(cmd == CMD_NORTH && mob->in_room == kAbyssGateRoom) {
		return true;
	}
	return false;
}

void abyss_gatekeeper_deny(struct char_data* ch, struct char_data* mob) {
	if(ch == nullptr || mob == nullptr) {
		return;
	}
	send_to_char("Il guardiano scuote la testa e ti blocca il passaggio.\n\r", ch);
	act("$n scuote la testa e blocca il passaggio a $N.", TRUE, mob, nullptr, ch, TO_NOTVICT);
}

bool keftab_target_has_ancient_sword(struct char_data* victim) {
	if(victim == nullptr) {
		return false;
	}
	for(int vnum = kSwordAncients; vnum <= kSwordAncientsLast; ++vnum) {
		if(HasObject(victim, vnum)) {
			return true;
		}
	}
	return false;
}

} // namespace

MOBSPECIAL_FUNC(AbyssGateKeeper) {
	if(ch == nullptr || mob == nullptr) {
		return FALSE;
	}
	if(!AWAKE(mob)) {
		return FALSE;
	}

	if(type == EVENT_TICK) {
		if(mob->specials.fighting != nullptr) {
			fighter(mob, cmd, arg, mob, type);
		}
		return FALSE;
	}

	if(type != EVENT_COMMAND) {
		return FALSE;
	}

	if(abyss_gatekeeper_blocks(mob, cmd)) {
		abyss_gatekeeper_deny(ch, mob);
		return TRUE;
	}
	return FALSE;
}

MOBSPECIAL_FUNC(Keftab) {
	if(ch == nullptr || mob == nullptr) {
		return FALSE;
	}
	if(type != EVENT_TICK) {
		return FALSE;
	}

	if(ch->specials.hunting == nullptr) {
		for(struct char_data* victim = character_list; victim != nullptr; victim = victim->next) {
			if(IS_NPC(victim) || GetMaxLevel(victim) >= kHuntMaxLevel) {
				continue;
			}
			if(!keftab_target_has_ancient_sword(victim)) {
				continue;
			}
			AddHated(ch, victim);
			SetHunting(ch, victim);
			return TRUE;
		}
		return FALSE;
	}

	/* La vittima ha ancora una Spada degli Antichi? Altrimenti smetti di cacciare. */
	if(keftab_target_has_ancient_sword(ch->specials.hunting)) {
		return FALSE;
	}
	ch->specials.hunting = nullptr;
	return FALSE;
}

MOBSPECIAL_FUNC(StormGiant) {
	if(ch == nullptr || mob == nullptr) {
		return FALSE;
	}
	if(type != EVENT_TICK) {
		return FALSE;
	}
	if(ch->specials.fighting == nullptr) {
		return FALSE;
	}

	if(GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
		StandUp(ch);
		return FALSE;
	}

	if(number(0, 5) != 0) {
		fighter(ch, cmd, arg, mob, type);
		return FALSE;
	}

	act("$n crea un fulmine!", TRUE, ch, nullptr, nullptr, TO_ROOM);
	struct char_data* vict = FindAHatee(ch);
	if(vict == nullptr) {
		vict = FindVictim(ch);
	}
	if(vict == nullptr) {
		return FALSE;
	}
	cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, nullptr);
	return FALSE;
}

MOBSPECIAL_FUNC(Manticore) {
	/* Nessun comportamento speciale (stub storico). */
	(void)ch;
	(void)cmd;
	(void)arg;
	(void)mob;
	(void)type;
	return FALSE;
}

MOBSPECIAL_FUNC(Kraken) {
	/* Nessun comportamento speciale (stub storico). */
	(void)ch;
	(void)cmd;
	(void)arg;
	(void)mob;
	(void)type;
	return FALSE;
}

} // namespace Alarmud
