/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <unordered_map>
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
#include "regen.hpp"
#include "spells.hpp"
#include "utility.hpp"

namespace Alarmud {
namespace {

constexpr int kInterruptPower = 25;
/** CD condiviso su tutte le interrupt, in round di violenza (PULSE_VIOLENCE). */
constexpr int kInterruptCooldownRounds = 5;
constexpr int kTargetLagRounds = 1;
constexpr int kInterruptMinLevel = 15;

std::unordered_map<const char_data*, unsigned long> g_interrupt_ready_pulse;

constexpr int kMindSpikeMana = 15;
constexpr int kInterruptMoveCost = 10;

/* CD e pratica seguono il personaggio, non il corpo polimorfato/switched. */
[[nodiscard]] char_data* interrupt_owner(char_data* ch) {
	if(ch != nullptr && ch->desc != nullptr && ch->desc->original != nullptr) {
		return ch->desc->original;
	}
	return ch;
}

[[nodiscard]] char_data* interrupt_skill_char(char_data* ch) {
	char_data* const owner = interrupt_owner(ch);
	if(owner != nullptr && owner->skills != nullptr) {
		return owner;
	}
	return ch;
}

[[nodiscard]] bool interrupt_on_cooldown(char_data* ch) {
	char_data* const key = interrupt_owner(ch);
	if(key == nullptr) {
		return false;
	}
	const auto it = g_interrupt_ready_pulse.find(key);
	if(it == g_interrupt_ready_pulse.end()) {
		return false;
	}
	if(pulse >= it->second) {
		g_interrupt_ready_pulse.erase(it);
		return false;
	}
	return true;
}

void set_interrupt_cooldown(char_data* ch) {
	char_data* const key = interrupt_owner(ch);
	if(key == nullptr) {
		return;
	}
	g_interrupt_ready_pulse[key] =
		pulse + static_cast<unsigned long>(kInterruptCooldownRounds) * PULSE_VIOLENCE;
}

[[nodiscard]] int interrupt_cooldown_rounds_left(char_data* ch) {
	char_data* const key = interrupt_owner(ch);
	if(key == nullptr) {
		return 0;
	}
	const auto it = g_interrupt_ready_pulse.find(key);
	if(it == g_interrupt_ready_pulse.end() || pulse >= it->second) {
		return 0;
	}
	const unsigned long left = it->second - pulse;
	return static_cast<int>((left + PULSE_VIOLENCE - 1) / PULSE_VIOLENCE);
}

/* Pratica sull'originale; i messaggi vanno al corpo che agisce (il desc e' li'). */
void interrupt_learn_from_mistake(char_data* ch, int sknum) {
	char_data* const learner = interrupt_skill_char(ch);
	if(learner == nullptr || learner->skills == nullptr) {
		return;
	}
	if(learner == ch || learner->desc != nullptr) {
		LearnFromMistake(learner, sknum, 0, 95);
		if(ch != learner && ch->skills != nullptr) {
			ch->skills[sknum].learned = learner->skills[sknum].learned;
		}
		return;
	}
	const int before = static_cast<int>(learner->skills[sknum].learned);
	LearnFromMistake(learner, sknum, 1, 95);
	if(ch->skills != nullptr) {
		ch->skills[sknum].learned = learner->skills[sknum].learned;
	}
	if(static_cast<int>(learner->skills[sknum].learned) <= before) {
		return;
	}
	send_to_char("Impari dai tuoi errori.\n\r", ch);
	int cap = 95;
	if(IS_IMMORTAL(learner)) {
		cap = 100;
	}
	else if(HowManyClasses(learner) >= 3) {
		cap = 81;
	}
	else if(HowManyClasses(learner) == 2) {
		cap = 86;
	}
	if(static_cast<int>(learner->skills[sknum].learned) >= cap) {
		send_to_char("Hai imparato tutto su questa abilita'!\n\r", ch);
	}
}

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
	const char* emote_self;  /* TO_CHAR — attaccante */
	const char* emote_vict;  /* TO_VICT — target (PvP) */
	const char* emote_room;  /* TO_NOTVICT — spettatori */
	const char* lag_self;
	const char* lag_room;
	const char* lag_vict;
};

constexpr InterruptDef kDefs[] = {
	{SKILL_HILT_SMASH, CLASS_WARRIOR, false,
	 "$c0011Colpisci $N di piatto con l'elsa!$c0007",
	 "$c0011$n ti colpisce di piatto con l'elsa!$c0007",
	 "$c0011$n colpisce $N di piatto con l'elsa!$c0007",
	 "$c0014Il colpo d'elsa spezza il ritmo di $N!$c0007",
	 "$c0014Il colpo d'elsa di $n spezza il ritmo di $N!$c0007",
	 "$c0009Il colpo d'elsa ti spezza il ritmo!$c0007"},
	{SKILL_FURY_HOWL, CLASS_BARBARIAN, false,
	 "$c0011Urli contro $N con furia selvaggia!$c0007",
	 "$c0011$n urla contro di te con furia selvaggia!$c0007",
	 "$c0011$n urla contro $N con furia selvaggia!$c0007",
	 "$c0014L'urlo fa vacillare la concentrazione di $N!$c0007",
	 "$c0014L'urlo di $n fa vacillare la concentrazione di $N!$c0007",
	 "$c0009L'urlo selvaggio ti squassa la concentrazione!$c0007"},
	{SKILL_PRESSURE_POINT, CLASS_MONK, false,
	 "$c0011Premi un punto vitale di $N!$c0007",
	 "$c0011$n preme un tuo punto vitale!$c0007",
	 "$c0011$n preme un punto vitale di $N!$c0007",
	 "$c0014Il punto vitale fa irrigidire $N!$c0007",
	 "$c0014Il punto vitale premuto da $n fa irrigidire $N!$c0007",
	 "$c0009Un dolore acuto ti taglia il respiro!$c0007"},
	{SKILL_CHEAP_SHOT, CLASS_THIEF, false,
	 "$c0011Colpisci $N dove meno se l$B aspetta!$c0007",
	 "$c0011$n ti colpisce a tradimento!$c0007",
	 "$c0011$n colpisce $N a tradimento!$c0007",
	 "$c0014$N sussulta, colt$B di sorpresa!$c0007",
	 "$c0014$N sussulta sotto il colpo a tradimento di $n!$c0007",
	 "$c0009Il colpo a tradimento ti fa perdere il filo!$c0007"},
	{SPELL_HOLY_REBUKE, CLASS_PALADIN, true,
	 "$c0015Scagli su $N il biasimo della tua fede!$c0007",
	 "$c0015$n scaglia su di te il biasimo della sua fede!$c0007",
	 "$c0015$n scaglia su $N il biasimo della sua fede!$c0007",
	 "$c0014Il biasimo sacro scuote $N!$c0007",
	 "$c0014Il biasimo sacro di $n scuote $N!$c0007",
	 "$c0009Il biasimo sacro ti scuote fin nelle ossa!$c0007"},
	{SKILL_TACTICAL_STRIKE, CLASS_RANGER, false,
	 "$c0011Intercetti $N con un fendente calibrato!$c0007",
	 "$c0011$n ti intercetta con un fendente calibrato!$c0007",
	 "$c0011$n intercetta $N con un fendente calibrato!$c0007",
	 "$c0014Il fendente spezza la cadenza di $N!$c0007",
	 "$c0014Il fendente di $n spezza la cadenza di $N!$c0007",
	 "$c0009Il fendente ti spezza la cadenza!$c0007"},
	{SPELL_SPELLSHATTER, CLASS_MAGIC_USER | CLASS_SORCERER, true,
	 "$c0012Scagli un'onda che incrina la magia di $N!$c0007",
	 "$c0012$n scaglia un'onda che incrina la tua magia!$c0007",
	 "$c0012$n scaglia un'onda che incrina la magia di $N!$c0007",
	 "$c0014La magia di $N incrina e sfarfalla!$c0007",
	 "$c0014La magia di $N incrina sotto l'onda di $n!$c0007",
	 "$c0009Un'onda arcana incrina la tua magia!$c0007"},
	{SPELL_DIVINE_REBUKE, CLASS_CLERIC, true,
	 "$c0015Pronunci su $N una parola di rimprovero divino!$c0007",
	 "$c0015$n pronuncia su di te una parola di rimprovero divino!$c0007",
	 "$c0015$n pronuncia su $N una parola di rimprovero divino!$c0007",
	 "$c0014La parola divina fa esitare $N!$c0007",
	 "$c0014La parola divina di $n fa esitare $N!$c0007",
	 "$c0009La parola divina ti inchioda un istante!$c0007"},
	{SPELL_THORNBIND, CLASS_DRUID, true,
	 "$c0010Invochi spine che legano la concentrazione di $N!$c0007",
	 "$c0010$n invoca spine che legano la tua concentrazione!$c0007",
	 "$c0010$n invoca spine che legano la concentrazione di $N!$c0007",
	 "$c0014Le spine legano il filo magico di $N!$c0007",
	 "$c0014Le spine di $n legano il filo magico di $N!$c0007",
	 "$c0009Le spine ti legano la concentrazione!$c0007"},
	{SKILL_MIND_SPIKE, CLASS_PSI, false,
	 "$c0013Scagli una lama psichica contro $N!$c0007",
	 "$c0013$n scaglia una lama psichica contro di te!$c0007",
	 "$c0013$n scaglia una lama psichica contro $N!$c0007",
	 "$c0014La mente di $N vacilla sotto la lama!$c0007",
	 "$c0014La mente di $N vacilla sotto la lama di $n!$c0007",
	 "$c0009Una lama psichica ti attraversa la mente!$c0007"},
};

const InterruptDef* find_def(int id) {
	for(const auto& d : kDefs) {
		if(d.id == id) {
			return &d;
		}
	}
	return nullptr;
}

[[nodiscard]] bool interrupt_prereqs_blocked(char_data* ch, const InterruptDef& def) {
	if(GetMaxLevel(ch) < IMMORTALE && !HasClass(ch, def.class_bits)) {
		send_to_char("Questa tecnica non e' della tua classe.\n\r", ch);
		return true;
	}
	if(GetMaxLevel(ch) < IMMORTALE && owning_class_level(ch, def.class_bits) < kInterruptMinLevel) {
		send_to_char("Non hai ancora l'esperienza necessaria.\n\r", ch);
		return true;
	}
	if(interrupt_on_cooldown(ch)) {
		char buf[128];
		snprintf(buf, sizeof(buf),
				 "Devi ancora recuperare l'equilibrio (%d round).\n\r",
				 interrupt_cooldown_rounds_left(ch));
		send_to_char(buf, ch);
		return true;
	}
	return false;
}

void apply_target_lag(struct char_data* ch, struct char_data* vict, const InterruptDef& def) {
	/* PhaseBoss: niente lag né emote di sbilanciamento (resta solo soft/hard). */
	if(phase_boss_resists_crowd_control(vict)) {
		return;
	}
	if(!IS_WAITING(vict)) {
		WAIT_STATE(vict, PULSE_VIOLENCE * kTargetLagRounds);
		act(def.lag_self, FALSE, ch, 0, vict, TO_CHAR);
		act(def.lag_room, FALSE, ch, 0, vict, TO_NOTVICT);
		act(def.lag_vict, FALSE, ch, 0, vict, TO_VICT);
	}
	else {
		act("$c0008$N e' gia' destabilizzat$B: non puoi romperne oltre la concentrazione.$c0007",
			FALSE, ch, 0, vict, TO_CHAR);
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

	if(interrupt_prereqs_blocked(ch, *def)) {
		return;
	}

	char_data* const learner = interrupt_skill_char(ch);

	if(!from_spell) {
		/* Path comando: cast ha gia' TAR_VIOLENT / pace / PK. */
		if(check_peaceful(ch, "C'e' troppa pace qui per essere violenti.\n\r")) {
			return;
		}
		if(combat_denied_notify(ch, vict, id) || immortal_pc_denied_notify(ch, vict)) {
			return;
		}
		if(!learner || !learner->skills || !learner->skills[id].learned) {
			send_to_char("Non hai ancora imparato la tecnica giusta.\n\r", ch);
			return;
		}
		if(GetMaxLevel(interrupt_owner(ch)) < IMMORTALE) {
			if(id == SKILL_MIND_SPIKE) {
				if(GET_MANA(ch) < kMindSpikeMana) {
					send_to_char("Non hai abbastanza $c0009energia mentale$c0007!\n\r", ch);
					return;
				}
				GET_MANA(ch) -= kMindSpikeMana;
				alter_mana(ch, 0);
			}
			else {
				if(GET_MOVE(ch) < kInterruptMoveCost) {
					send_to_char("Sei troppo stanco per farlo.\n\r", ch);
					return;
				}
				GET_MOVE(ch) -= kInterruptMoveCost;
				alter_move(ch, 0);
			}
		}
		if(number(1, 101) > learner->skills[id].learned) {
			send_to_char("Mancato!\n\r", ch);
			WAIT_STATE(ch, PULSE_VIOLENCE);
			/* Come bash/kick: il mancato apre il combat, senza cooldown. */
			if(GET_POS(vict) > POSITION_DEAD) {
				(void)SetCharFighting(ch, vict);
				(void)SetVictFighting(ch, vict);
			}
			interrupt_learn_from_mistake(ch, id);
			return;
		}
	}

	act(def->emote_self, FALSE, ch, 0, vict, TO_CHAR);
	act(def->emote_vict, FALSE, ch, 0, vict, TO_VICT);
	act(def->emote_room, FALSE, ch, 0, vict, TO_NOTVICT);

	/* Apre (o unisce) il combat: emote violenta senza fight lasciava il mob fermo. */
	(void)SetCharFighting(ch, vict);
	(void)SetVictFighting(ch, vict);

	apply_target_lag(ch, vict, *def);
	(void)phase_boss_try_interrupt(ch, vict, kInterruptPower);
	set_interrupt_cooldown(ch);

	if(!from_spell && learner && !IS_NPC(learner) && !IS_IMMORTAL(learner) && !(number(0, 3))) {
		if(learner->skills) {
			learner->skills[id].learned++;
			if(learner->skills[id].learned >= 95) {
				send_to_char("Hai padroneggiato questa tecnica!\n\r", ch);
				learner->skills[id].learned = 95;
			}
			if(ch != learner && ch->skills != nullptr) {
				ch->skills[id].learned = learner->skills[id].learned;
			}
		}
	}
}

void do_interrupt_skill(struct char_data* ch, const char* argument, int /*cmd*/, int skill_id) {
	/* Lag generico (bash ecc.) resta un blocco; il CD interrupt e' a parte. */
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

bool class_interrupt_refuse_cast(char_data* ch, int spell_id) {
	const InterruptDef* def = find_def(spell_id);
	if(def == nullptr || ch == nullptr) {
		return false;
	}
	return interrupt_prereqs_blocked(ch, *def);
}

void clear_class_interrupt_cooldown(char_data* ch) {
	if(ch != nullptr) {
		g_interrupt_ready_pulse.erase(ch);
	}
}

void do_hilt_smash(struct char_data* ch, const char* argument, int cmd) {
	if(ch->equipment[WIELD] == nullptr) {
		send_to_char("Ti serve un'arma impugnata per colpire con l'elsa.\n\r", ch);
		return;
	}
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

void spell_thornbind(byte /*level*/, struct char_data* ch, struct char_data* victim,
					 struct obj_data* /*obj*/) {
	resolve_interrupt(ch, victim, SPELL_THORNBIND, true);
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

void cast_thornbind(byte level, struct char_data* ch, const char* /*arg*/, int type,
					struct char_data* tar_ch, struct obj_data* tar_obj) {
	switch(type) {
	case SPELL_TYPE_SPELL:
		spell_thornbind(level, ch, tar_ch, tar_obj);
		break;
	default:
		mudlog(LOG_SYSERR, "Serious screw-up in thornbind!");
		break;
	}
}

} // namespace Alarmud
