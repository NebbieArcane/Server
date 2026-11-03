/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <algorithm>
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
#include "class_interrupt.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "multiclass.hpp"
#include "phase_boss.hpp"
#include "spells.hpp"
#include "utility.hpp"

namespace Alarmud {
namespace {

constexpr int kInterruptPower = 40;
constexpr int kCasterWaitRounds = 2;
constexpr int kTargetLagRounds = 1;
constexpr int kInterruptMinLevel = 15;

int owning_class_level(const struct char_data* ch, unsigned long class_bits) {
	int lvl = 0;
	if(class_bits & CLASS_MAGIC_USER) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, MAGE_LEVEL_IND)));
	}
	if(class_bits & CLASS_CLERIC) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, CLERIC_LEVEL_IND)));
	}
	if(class_bits & CLASS_WARRIOR) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, WARRIOR_LEVEL_IND)));
	}
	if(class_bits & CLASS_THIEF) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, THIEF_LEVEL_IND)));
	}
	if(class_bits & CLASS_DRUID) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, DRUID_LEVEL_IND)));
	}
	if(class_bits & CLASS_MONK) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, MONK_LEVEL_IND)));
	}
	if(class_bits & CLASS_BARBARIAN) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, BARBARIAN_LEVEL_IND)));
	}
	if(class_bits & CLASS_SORCERER) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, SORCERER_LEVEL_IND)));
	}
	if(class_bits & CLASS_PALADIN) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, PALADIN_LEVEL_IND)));
	}
	if(class_bits & CLASS_RANGER) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, RANGER_LEVEL_IND)));
	}
	if(class_bits & CLASS_PSI) {
		lvl = std::max(lvl, static_cast<int>(GET_LEVEL(ch, PSI_LEVEL_IND)));
	}
	return lvl;
}

struct InterruptDef {
	int id;
	unsigned long class_bits;
	bool is_spell;
	const char* emote_self;
	const char* emote_room;
	const char* lag_self;
	const char* lag_room;
};

constexpr InterruptDef kDefs[] = {
	{SKILL_HILT_SMASH, CLASS_WARRIOR, false,
	 "$c0011Colpisci $N con l'elsa dell'arma!$c0007",
	 "$c0011$n colpisce $N con l'elsa dell'arma!$c0007",
	 "$c0014$N perde il ritmo sotto il colpo d'elsa!$c0007",
	 "$c0014$N perde il ritmo sotto il colpo d'elsa di $n!$c0007"},
	{SKILL_FURY_HOWL, CLASS_BARBARIAN, false,
	 "$c0011Scagli un ululato furioso contro $N!$c0007",
	 "$c0011$n scaglia un ululato furioso contro $N!$c0007",
	 "$c0014$N esita sotto il tuo ululato!$c0007",
	 "$c0014$N esita sotto l'ululato furioso di $n!$c0007"},
	{SKILL_PRESSURE_POINT, CLASS_MONK, false,
	 "$c0011Colpisci un nervo vitale di $N!$c0007",
	 "$c0011$n colpisce un nervo vitale di $N!$c0007",
	 "$c0014$N si irrigidisce sotto il tuo colpo!$c0007",
	 "$c0014$N si irrigidisce sotto il colpo di $n!$c0007"},
	{SKILL_CHEAP_SHOT, CLASS_THIEF, false,
	 "$c0011Colpisci $N a tradimento!$c0007",
	 "$c0011$n colpisce $N a tradimento!$c0007",
	 "$c0014$N sussulta, colt$B di sorpresa!$c0007",
	 "$c0014$N sussulta sotto il colpo a tradimento di $n!$c0007"},
	{SPELL_HOLY_REBUKE, CLASS_PALADIN, true,
	 "$c0015Scagli un'ammonizione sacra contro $N!$c0007",
	 "$c0015$n scaglia un'ammonizione sacra contro $N!$c0007",
	 "$c0014$N trema sotto il peso della tua ammonizione!$c0007",
	 "$c0014$N trema sotto l'ammonizione sacra di $n!$c0007"},
	{SKILL_TACTICAL_STRIKE, CLASS_RANGER, false,
	 "$c0011Porti un fendente preciso contro $N!$c0007",
	 "$c0011$n porta un fendente preciso contro $N!$c0007",
	 "$c0014$N perde l'equilibrio sotto il tuo fendente!$c0007",
	 "$c0014$N perde l'equilibrio sotto il fendente di $n!$c0007"},
	{SPELL_SPELLSHATTER, CLASS_MAGIC_USER | CLASS_SORCERER, true,
	 "$c0012Lanci un incanto che spezza la concentrazione di $N!$c0007",
	 "$c0012$n lancia un incanto che spezza la concentrazione di $N!$c0007",
	 "$c0014La concentrazione di $N si spezza!$c0007",
	 "$c0014La concentrazione di $N si spezza sotto l'incanto di $n!$c0007"},
	{SPELL_DIVINE_REBUKE, CLASS_CLERIC, true,
	 "$c0015Fulmini $N con un rimprovero divino!$c0007",
	 "$c0015$n fulmina $N con un rimprovero divino!$c0007",
	 "$c0014$N esita sotto la forza della tua parola!$c0007",
	 "$c0014$N esita sotto la parola divina di $n!$c0007"},
	{SPELL_NATURES_GRASP, CLASS_DRUID, true,
	 "$c0010Evochi radici che avvinghiano $N!$c0007",
	 "$c0010$n evoca radici che avvinghiano $N!$c0007",
	 "$c0014Le radici trattengono $N!$c0007",
	 "$c0014Le radici evocate da $n trattengono $N!$c0007"},
	{SKILL_MIND_SPIKE, CLASS_PSI, false,
	 "$c0013Scagli una spina nella mente di $N!$c0007",
	 "$c0013$n scaglia una spina nella mente di $N!$c0007",
	 "$c0014$N barcolla sotto la tua spina mentale!$c0007",
	 "$c0014$N barcolla sotto la spina mentale di $n!$c0007"},
};

const InterruptDef* find_def(int id) {
	for(const auto& d : kDefs) {
		if(d.id == id) {
			return &d;
		}
	}
	return nullptr;
}

void apply_target_lag(struct char_data* ch, struct char_data* vict, const InterruptDef& def) {
	if(!IS_WAITING(vict)) {
		WAIT_STATE(vict, PULSE_VIOLENCE * kTargetLagRounds);
		act(def.lag_self, FALSE, ch, 0, vict, TO_CHAR);
		act(def.lag_room, FALSE, ch, 0, vict, TO_NOTVICT);
		act("$c0009$n ti fa perdere l'equilibrio!$c0007", FALSE, ch, 0, vict, TO_VICT);
	}
	else {
		act("$c0008$N e' gia' fuori equilibrio: non puoi sbilanciarl$B oltre.$c0007", FALSE, ch,
			0, vict, TO_CHAR);
	}
}

void resolve_interrupt(struct char_data* ch, struct char_data* vict, int id, bool from_spell) {
	const InterruptDef* def = find_def(id);
	if(!def || !ch || !vict) {
		return;
	}

	if(ch == vict) {
		send_to_char("Non ha senso.\n\r", ch);
		return;
	}

	if(GetMaxLevel(ch) < IMMORTALE && !HasClass(ch, def->class_bits)) {
		send_to_char("Questa tecnica non e' della tua classe.\n\r", ch);
		return;
	}

	if(GetMaxLevel(ch) < IMMORTALE && owning_class_level(ch, def->class_bits) < kInterruptMinLevel) {
		send_to_char("Non hai ancora l'esperienza necessaria.\n\r", ch);
		return;
	}

	if(!from_spell) {
		if(!ch->skills || !ch->skills[id].learned) {
			send_to_char("Non hai ancora imparato la tecnica giusta.\n\r", ch);
			return;
		}
		if(number(1, 101) > ch->skills[id].learned) {
			send_to_char("Mancato!\n\r", ch);
			WAIT_STATE(ch, PULSE_VIOLENCE);
			return;
		}
		WAIT_STATE(ch, PULSE_VIOLENCE * kCasterWaitRounds);
	}

	act(def->emote_self, FALSE, ch, 0, vict, TO_CHAR);
	act(def->emote_room, FALSE, ch, 0, vict, TO_ROOM);

	apply_target_lag(ch, vict, *def);
	(void)phase_boss_try_interrupt(ch, vict, kInterruptPower);

	if(!from_spell && !IS_NPC(ch) && !IS_IMMORTAL(ch) && !(number(0, 3))) {
		if(ch->skills) {
			ch->skills[id].learned++;
			if(ch->skills[id].learned >= 95) {
				send_to_char("Hai padroneggiato questa tecnica!\n\r", ch);
				ch->skills[id].learned = 95;
			}
		}
	}
}

void do_interrupt_skill(struct char_data* ch, const char* argument, int /*cmd*/, int skill_id) {
	if(IS_WAITING(ch)) {
		send_to_char("Devi ancora recuperare!\n\r", ch);
		return;
	}

	char name[MAX_INPUT_LENGTH];
	only_argument(argument, name);
	struct char_data* vict = nullptr;
	if(*name) {
		vict = get_char_room_vis(ch, name);
	}
	else if(ch->specials.fighting) {
		vict = ch->specials.fighting;
	}

	if(!vict) {
		send_to_char("Chi?\n\r", ch);
		return;
	}

	resolve_interrupt(ch, vict, skill_id, false);
}

} // namespace

bool is_class_interrupt_id(int id) noexcept {
	return find_def(id) != nullptr;
}

void do_hilt_smash(struct char_data* ch, const char* argument, int cmd) {
	do_interrupt_skill(ch, argument, cmd, SKILL_HILT_SMASH);
}

void do_fury_howl(struct char_data* ch, const char* argument, int cmd) {
	do_interrupt_skill(ch, argument, cmd, SKILL_FURY_HOWL);
}

void do_pressure_point(struct char_data* ch, const char* argument, int cmd) {
	do_interrupt_skill(ch, argument, cmd, SKILL_PRESSURE_POINT);
}

void do_cheap_shot(struct char_data* ch, const char* argument, int cmd) {
	do_interrupt_skill(ch, argument, cmd, SKILL_CHEAP_SHOT);
}

void do_tactical_strike(struct char_data* ch, const char* argument, int cmd) {
	do_interrupt_skill(ch, argument, cmd, SKILL_TACTICAL_STRIKE);
}

void do_mind_spike(struct char_data* ch, const char* argument, int cmd) {
	do_interrupt_skill(ch, argument, cmd, SKILL_MIND_SPIKE);
}

void spell_holy_rebuke(byte /*level*/, struct char_data* ch, struct char_data* victim,
					   struct obj_data* /*obj*/) {
	resolve_interrupt(ch, victim, SPELL_HOLY_REBUKE, true);
}

void spell_spellshatter(byte /*level*/, struct char_data* ch, struct char_data* victim,
						struct obj_data* /*obj*/) {
	resolve_interrupt(ch, victim, SPELL_SPELLSHATTER, true);
}

void spell_divine_rebuke(byte /*level*/, struct char_data* ch, struct char_data* victim,
						 struct obj_data* /*obj*/) {
	resolve_interrupt(ch, victim, SPELL_DIVINE_REBUKE, true);
}

void spell_natures_grasp(byte /*level*/, struct char_data* ch, struct char_data* victim,
						 struct obj_data* /*obj*/) {
	resolve_interrupt(ch, victim, SPELL_NATURES_GRASP, true);
}

void cast_holy_rebuke(byte level, struct char_data* ch, const char* /*arg*/, int type,
					  struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_holy_rebuke(level, ch, tar_ch, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in holy rebuke!");
		break;
	}
}

void cast_spellshatter(byte level, struct char_data* ch, const char* /*arg*/, int type,
					   struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_spellshatter(level, ch, tar_ch, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in spellshatter!");
		break;
	}
}

void cast_divine_rebuke(byte level, struct char_data* ch, const char* /*arg*/, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_divine_rebuke(level, ch, tar_ch, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in divine rebuke!");
		break;
	}
}

void cast_natures_grasp(byte level, struct char_data* ch, const char* /*arg*/, int type,
						struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_natures_grasp(level, ch, tar_ch, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in nature's grasp!");
		break;
	}
}

} // namespace Alarmud
