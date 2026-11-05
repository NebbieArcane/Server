/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstring>
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
#include "spec_bot.hpp"
#include "act.comm.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "regen.hpp"
#include "spec_procs2.hpp"
#include "spell_parser.hpp"
#include "spells2.hpp"
#include "utility.hpp"

namespace Alarmud {
namespace {

[[nodiscard]] bool is_keyword(const char* hay, const char* needle) noexcept {
	return hay != nullptr && needle != nullptr && *needle != '\0'
		&& isname(needle, hay);
}

[[nodiscard]] bool is_ally(char_data* mob, char_data* t) noexcept {
	if(mob == nullptr || t == nullptr || t->in_room != mob->in_room) {
		return false;
	}
	if(t == mob || t == mob->master) {
		return true;
	}
	if(mob->master != nullptr && t->master == mob->master) {
		return true;
	}
	return in_group(mob, t) != 0;
}

void dismiss_bot(char_data* mob) {
	if(mob == nullptr) {
		return;
	}
	/* Togli charm prima di stop_follower: evita i messaggi "ti odia". */
	REMOVE_BIT(mob->specials.affected_by, AFF_CHARM);
	if(mob->master != nullptr) {
		stop_follower(mob);
	}
}

bool try_recruit(char_data* ch, char_data* mob, const char* accept_msg) {
	if(ch == nullptr || mob == nullptr) {
		return false;
	}
	if(IS_NPC(ch)) {
		do_say(mob, "Solo i mortali possono reclutarmi.", 0);
		return true;
	}
	if(mob->master == ch) {
		do_say(mob, "Ti sto gia' seguendo.", 0);
		return true;
	}
	if(mob->master != nullptr) {
		do_say(mob, "Sto gia' seguendo qualcun altro. Chiedimi di andarmene prima.", 0);
		return true;
	}
	if(circle_follow(mob, ch)) {
		do_say(mob, "Non posso seguirti: creerebbe un ciclo di follow.", 0);
		return true;
	}

	add_follower(mob, ch);
	SET_BIT(mob->specials.affected_by, AFF_CHARM);
	do_say(mob, accept_msg != nullptr ? accept_msg : "Va bene, ti seguo.", 0);
	return true;
}

/** true se l'ask e' rivolto a questo mob; scrive il resto della frase in *rest_out. */
[[nodiscard]] bool parse_ask_to_mob(char_data* ch, char_data* mob, const char* arg,
								   const char** rest_out) {
	if(ch == nullptr || mob == nullptr || arg == nullptr || rest_out == nullptr) {
		return false;
	}
	char name[MAX_INPUT_LENGTH]{};
	const char* rest = one_argument(arg, name);
	if(name[0] == '\0' || get_char_room_vis(ch, name) != mob) {
		return false;
	}
	*rest_out = rest;
	return true;
}

/** Sanctuary sul primo alleato in stanza senza sanc (priorita': master, altri, se'). */
bool try_group_sanctuary(char_data* mob) {
	if(mob == nullptr || mob->master == nullptr) {
		return false;
	}
	if(mob->in_room != mob->master->in_room) {
		return false;
	}
	if(check_nomagic(mob, 0, 0) || check_soundproof(mob)) {
		return false;
	}
	if(GET_MANA(mob) < 40) {
		return false;
	}

	auto* rp = real_roomp(mob->in_room);
	if(rp == nullptr) {
		return false;
	}

	char_data* target = nullptr;
	if(!HasActiveSanctuary(mob->master)) {
		target = mob->master;
	}
	else {
		for(char_data* t = rp->people; t != nullptr; t = t->next_in_room) {
			if(!is_ally(mob, t) || HasActiveSanctuary(t)) {
				continue;
			}
			if(t == mob) {
				continue; /* se' per ultimo */
			}
			target = t;
			break;
		}
		if(target == nullptr && !HasActiveSanctuary(mob)) {
			target = mob;
		}
	}
	if(target == nullptr) {
		return false;
	}

	act("$n pronuncia le parole, '$c0015Oooh, bello!$c0007'.", FALSE, mob, 0, 0, TO_ROOM);
	cast_sanctuary(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, target, 0);
	GET_MANA(mob) -= 40;
	alter_mana(mob, 0);
	return true;
}

} // namespace

MOBSPECIAL_FUNC(ClericBot) {
	if(mob == nullptr) {
		return FALSE;
	}

	if(type == EVENT_COMMAND) {
		if(cmd != CMD_ASK || ch == nullptr || IS_NPC(ch)) {
			return FALSE;
		}

		const char* rest = nullptr;
		if(!parse_ask_to_mob(ch, mob, arg, &rest)) {
			return FALSE;
		}

		if(!CAN_SEE(mob, ch)) {
			do_say(mob, "Chi ha parlato?", 0);
			return TRUE;
		}

		if(is_keyword(rest, "seguimi") || is_keyword(rest, "follow")) {
			try_recruit(ch, mob, "Va bene, ti seguo. Curero' te e il tuo gruppo.");
			return TRUE;
		}
		if(is_keyword(rest, "vattene") || is_keyword(rest, "fermati")
				|| is_keyword(rest, "stop") || is_keyword(rest, "libera")) {
			if(mob->master != ch) {
				do_say(mob, "Non sto seguendo te.", 0);
				return TRUE;
			}
			do_say(mob, "Come vuoi. Torno a riposarmi.", 0);
			dismiss_bot(mob);
			return TRUE;
		}

		do_say(mob, "Puoi chiedermi: seguimi, oppure vattene.", 0);
		return TRUE;
	}

	if(type != EVENT_TICK || !AWAKE(mob)) {
		return FALSE;
	}

	/* Reclutato: gestiamo noi cura+sanc (evita doppio cleric da ACT_CLERIC). */
	if(mob->master == nullptr) {
		return FALSE;
	}

	if(cleric(mob, 0, "", mob, EVENT_TICK)) {
		return TRUE;
	}
	if(try_group_sanctuary(mob)) {
		return TRUE;
	}
	return TRUE;
}

} // namespace Alarmud
