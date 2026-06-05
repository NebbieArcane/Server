/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/*
* DaleMUD v2.0        Released 2/1994
* See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
* AlarMUD
* $Id: act.off.c,v 1.5 2002/03/23 20:49:38 Thunder Exp $
*/
/***************************  System  include ************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
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
#include "act.off.hpp"
#include "act.info.hpp"
#include "act.move.hpp"
#include "act.other.hpp"
#include "comm.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "maximums.hpp"
#include "multiclass.hpp"
#include "opinion.hpp"
#include "regen.hpp"
#include "snew.hpp"
#include "spell_parser.hpp"

namespace Alarmud {

namespace {

bool off_is_followers_keyword(const char* name) {
	return name != nullptr && *name != '\0' &&
	       (str_cmp(name, "follower") == 0 || str_cmp(name, "followers") == 0);
}

struct char_data* off_visible_in_room(struct char_data* ch, const char* name) {
	if(ch == nullptr || name == nullptr || name[0] == '\0') {
		return nullptr;
	}
	return get_char_room_vis(ch, name);
}

/** Target in room, else current opponent; optional message if neither. */
struct char_data* off_visible_or_fighting(struct char_data* ch, const char* name,
                                          const char* missingMsg) {
	struct char_data* victim = off_visible_in_room(ch, name);
	if(victim == nullptr && ch != nullptr && ch->specials.fighting != nullptr) {
		victim = ch->specials.fighting;
	}
	if(victim == nullptr && missingMsg != nullptr) {
		send_to_char(missingMsg, ch);
	}
	return victim;
}

bool off_at_peace(struct char_data* ch, const char* msg) {
	return check_peaceful(ch, msg);
}

/** Resto della riga dopo trim iniziale (come only_argument). */
std::string off_parse_first_token(const char* arg) {
	if(arg == nullptr) {
		return {};
	}
	std::string dest(MAX_INPUT_LENGTH, '\0');
	only_argument(arg, dest.data());
	dest.resize(std::strlen(dest.c_str()));
	return dest;
}

bool off_combat_blocks_vassal_act(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr) {
		return true;
	}
	if(victim != nullptr &&
			(ch->specials.fighting != nullptr || victim->specials.fighting != nullptr)) {
		send_to_char("Pensate a combattere!\n\r", ch);
		return true;
	}
	return false;
}

void off_clear_prince_link(struct char_data* ch) {
	if(ch == nullptr || GET_PRINCE(ch) == nullptr) {
		return;
	}
	free(GET_PRINCE(ch));
	GET_PRINCE(ch) = nullptr;
}

bool off_poly_blocks_vassal_act(struct char_data* ch) {
	if(ch != nullptr && IS_POLY(ch)) {
		send_to_char("Non puoi farlo in questa forma.\n\r", ch);
		return true;
	}
	return false;
}

/** ripudia senza bersaglio in stanza: il vassallo rinuncia al principe nominato. */
void off_ripudia_renounce_absent_prince(struct char_data* ch, const std::string& princeName) {
	if(ch == nullptr || princeName.empty() || !IS_VASSALLOOF(ch, princeName.c_str())) {
		act("Capisco la concitazione... ma non ne sei vassall$b!",
		    TRUE, ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	off_clear_prince_link(ch);
	act("Beh... a quanto sembra il coraggio non e' il tuo forte...\n\r"
	    "in ogni modo... adesso sei liber$b.",
	    TRUE, ch, nullptr, nullptr, TO_CHAR);
	GET_EXP(ch) -= static_cast<int>(GET_EXP(ch) / 100 * 5);
}

void off_ripudia_vassal_breaks_oath(struct char_data* ch, struct char_data* prince) {
	if(ch == nullptr || prince == nullptr || off_poly_blocks_vassal_act(ch)) {
		return;
	}
	act("Guardi negli occhi $N e rompi il tuo giuramento di fedelta'!",
	    TRUE, ch, nullptr, prince, TO_CHAR);
	act("$n rompe il suo giuramento di fedelta'!",
	    TRUE, ch, nullptr, prince, TO_VICT);
	act("$n rompe il suo giuramento di fedelta' a $N!",
	    TRUE, ch, nullptr, prince, TO_NOTVICT);
	off_clear_prince_link(ch);
}

void off_ripudia_prince_expels(struct char_data* ch, struct char_data* vassal) {
	if(ch == nullptr || vassal == nullptr || off_poly_blocks_vassal_act(vassal)) {
		return;
	}
	act("Fissi $N con uno sguardo severo e l$b bandisci dal tuo casato!",
	    TRUE, ch, nullptr, vassal, TO_CHAR);
	act("$n ti bandisce dal su$b casato!",
	    TRUE, ch, nullptr, vassal, TO_VICT);
	act("$n bandisce $N dal su$b casato!",
	    TRUE, ch, nullptr, vassal, TO_NOTVICT);
	off_clear_prince_link(vassal);
}

void off_set_prince_link(struct char_data* vassal, const char* princeName) {
	if(vassal == nullptr || princeName == nullptr || princeName[0] == '\0') {
		return;
	}
	off_clear_prince_link(vassal);
	GET_PRINCE(vassal) = strdup(princeName);
}

long off_associa_nomination_cost(struct char_data* ch, struct char_data* victim) {
	constexpr long kCostBase = 500000L;
	if(ch == nullptr || victim == nullptr) {
		return kCostBase;
	}
	long cost = (17 - GET_RCHR(ch)) * 50000L + (GET_RCHR(victim) - 12) * 25000L;
	return cost + kCostBase;
}

/** true = bloccato */
bool off_associa_target_missing(struct char_data* ch, struct char_data* victim) {
	if(victim != nullptr) {
		return false;
	}
	send_to_char("Ottima idea nominare dei vassalli..."
	             "ma almeno cerca di scrivere bene il loro nome!\n\r", ch);
	return true;
}

bool off_associa_victim_is_poly(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr || !IS_POLY(victim)) {
		return false;
	}
	send_to_char("Fare tuo vassallo un animale? Ma chi ti credi d'essere?"
	             " Caligola???\n\r", ch);
	return true;
}

bool off_associa_victim_too_young(struct char_data* ch, struct char_data* victim) {
	if(victim == nullptr || GetMaxLevel(victim) >= VASSALLO) {
		return false;
	}
	act("$N e' troppo giovane per giurarti fedelta'.",
	    TRUE, ch, nullptr, victim, TO_CHAR);
	act("Sei troppo giovane per giurare fedelta' a $n.",
	    TRUE, ch, nullptr, victim, TO_VICT);
	return true;
}

bool off_associa_victim_already_sworn(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr || GET_PRINCE(victim) == nullptr) {
		return false;
	}
	if(IS_VASSALLOOF(victim, GET_NAME(ch))) {
		act("$N e' gia' tu$b vassall$b!",
		    TRUE, ch, nullptr, victim, TO_CHAR);
		act("$n ha cercato di nominarti ANCORA su$b vassall$b!!",
		    TRUE, ch, nullptr, victim, TO_VICT);
	}
	else {
		act("$N ha' gia' giurato fedelta' a $T!",
		    TRUE, ch, nullptr, GET_PRINCE(victim), TO_CHAR);
	}
	return true;
}

void off_associa_complete(struct char_data* ch, struct char_data* victim, long cost) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}
	GET_GOLD(ch) -= cost;
	off_set_prince_link(victim, GET_NAME(ch));

	const std::string costMsg =
		"Il che ti costa " + std::to_string(cost) + " monete d'oro!";

	act("Nomini $N tu$B vassall$B.", TRUE, ch, nullptr, victim, TO_CHAR);
	act(costMsg.c_str(), TRUE, ch, nullptr, victim, TO_CHAR);
	act("Ti inginocchi e giuri fedelta' a $n.", TRUE, ch, nullptr, victim, TO_VICT);
	act("$N si inginocchia e $n l$B nomina su$B vassall$B!",
	    TRUE, ch, nullptr, victim, TO_NOTVICT);
}

void off_vomita_reset_conditions(struct char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	GET_COND(ch, FULL) = 0;
	GET_COND(ch, DRUNK) = 0;
	GET_COND(ch, THIRST) = 0;
}

void off_vomita_emit_start(struct char_data* ch) {
	act("Ti ficchi un dito in gola e cerchi di vomitare... che schifo!",
	    TRUE, ch, nullptr, nullptr, TO_CHAR);
	act("$n si ficca un dito in gola...",
	    TRUE, ch, nullptr, nullptr, TO_ROOM);
}

void off_vomita_emit_success(struct char_data* ch) {
	act("Il risultato... beh, e' quello che puoi immaginare!",
	    TRUE, ch, nullptr, nullptr, TO_CHAR);
	act("$n si vomita addosso!",
	    TRUE, ch, nullptr, nullptr, TO_ROOM);
	off_vomita_reset_conditions(ch);
}

void off_vomita_emit_fail_bite(struct char_data* ch) {
	act("Oltretutto riesci solo a morderti un dito!",
	    TRUE, ch, nullptr, nullptr, TO_CHAR);
	act("$n si morde un dito!",
	    TRUE, ch, nullptr, nullptr, TO_ROOM);
}

void off_vomita_perform(struct char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	off_vomita_emit_start(ch);
	if(number(0, 1)) {
		off_vomita_emit_success(ch);
		WAIT_STATE(ch, PULSE_VIOLENCE * 3);
	}
	else {
		off_vomita_emit_fail_bite(ch);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

void off_hit_self(struct char_data* ch) {
	if(ch == nullptr) {
		return;
	}
	send_to_char("Ti colpisci... AHI!\n\r", ch);
	act("$n si colpisce e grida AHI!", FALSE, ch, nullptr, ch, TO_ROOM);
	GET_EXP(ch) -= 5;
}

bool off_hit_quest_npc_blocked(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr) {
		return false;
	}
	if(!IS_PC(ch) && !IS_PC(victim) && affected_by_spell(victim, STATUS_QUEST)) {
		act("Hai l'impressione che $n voglia aggredire $N... ma qualcosa lo frena.",
		    FALSE, ch, nullptr, victim, TO_ROOM);
		return true;
	}
	return false;
}

bool off_hit_charm_blocks(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr) {
		return false;
	}
	if(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N e' troppo car$b: non puoi colpire $L.",
		    FALSE, ch, nullptr, victim, TO_CHAR);
		return true;
	}
	return false;
}

void off_hit_start_fight(struct char_data* ch, struct char_data* victim, int cmd) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}
	/* AllLiving: align check on a dead victim could crash the mud */
	if(hit(ch, victim, TYPE_UNDEFINED) == AllLiving) {
		ActionAlignMod(ch, victim, cmd);
		WAIT_STATE(ch, PULSE_VIOLENCE + 2);
	}
}

void off_hit_try_switch_opponent(struct char_data* ch, struct char_data* victim, int cmd) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}
	if(victim == ch->specials.fighting) {
		send_to_char("Fai del tuo meglio!\n\r", ch);
		return;
	}
	if(!ch->skills || !ch->skills[SKILL_SWITCH_OPP].learned) {
		send_to_char("Fai del tuo meglio!\n\r", ch);
		return;
	}
	if(number(1, 101) < ch->skills[SKILL_SWITCH_OPP].learned) {
		stop_fighting(ch);
		if(victim->attackers < 5) {
			ActionAlignMod(ch, victim, cmd);
			set_fighting(ch, victim);
		}
		else {
			send_to_char("Non c'e' spazio per cambiare bersaglio!\n\r", ch);
		}
		send_to_char("Cambi bersaglio.\n\r", ch);
		act("$n cambia bersaglio.", FALSE, ch, nullptr, nullptr, TO_ROOM);
		WAIT_STATE(ch, PULSE_VIOLENCE + 2);
	}
	else {
		send_to_char("Cerchi di cambiare bersaglio, ma ti confondi!\n\r", ch);
		stop_fighting(ch);
		LearnFromMistake(ch, SKILL_SWITCH_OPP, 0, 95);
		WAIT_STATE(ch, PULSE_VIOLENCE * 2);
	}
}

void off_hit_strike(struct char_data* ch, struct char_data* victim, int cmd) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}
	if(off_hit_quest_npc_blocked(ch, victim) || off_hit_charm_blocks(ch, victim)) {
		return;
	}
	if(GET_POS(ch) >= POSITION_STANDING && ch->specials.fighting == nullptr) {
		off_hit_start_fight(ch, victim, cmd);
	}
	else {
		off_hit_try_switch_opponent(ch, victim, cmd);
	}
}

void off_slay_record_quest_kill(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr || !victim->specials.quest_ref) {
		return;
	}
	free(ch->lastmkill);
	ch->lastmkill = strdup(GET_NAME(victim));
}

void off_slay_perform(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}
	if(ch == victim) {
		send_to_char("Tua madre potrebbe rattristarsi per questo... :(\n\r", ch);
		return;
	}
	act("Distruggi senza alcuna pieta' $N!", FALSE, ch, nullptr, victim, TO_CHAR);
	off_slay_record_quest_kill(ch, victim);
	raw_kill(victim, 0);
}

void off_kill_perform(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}
	if(ch == victim) {
		send_to_char("Tua madre ne sarebbe addolorata... :(\n\r", ch);
		return;
	}
	act("Tranci $N a pezzi! Ah, il sangue!", FALSE, ch, nullptr, victim, TO_CHAR);
	act("$N ti trancia a pezzi!", FALSE, victim, nullptr, ch, TO_CHAR);
	act("$n massacra brutalmente $N!", FALSE, ch, nullptr, victim, TO_NOTVICT);
	off_slay_record_quest_kill(ch, victim);
	raw_kill(victim, 0);
}

bool off_kill_delegates_to_hit(struct char_data* ch) {
	return ch == nullptr || GetMaxLevel(ch) < CREATORE || IS_NPC(ch);
}

constexpr int OFF_BACKSTAB_LOCATION = 12;

void off_backstab_add_hated(struct char_data* ch, struct char_data* victim) {
	if(victim != nullptr && IS_NPC(victim)) {
		AddHated(victim, ch);
	}
}

bool off_backstab_weapon_valid(struct char_data* ch) {
	if(ch == nullptr || ch->equipment[WIELD] == nullptr) {
		return false;
	}
	const int weaponType = ch->equipment[WIELD]->obj_flags.value[3];
	return weaponType == 11 || weaponType == 1 || weaponType == 10;
}

/** true = azione bloccata */
bool off_backstab_validate(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr) {
		return true;
	}
	if(victim == ch) {
		send_to_char("Come puoi pugnalare te stesso?\n\r", ch);
		return true;
	}
	if(!HasClass(ch, CLASS_THIEF)) {
		send_to_char("Non sei un ladro!\n\r", ch);
		return true;
	}
	if(ch->equipment[WIELD] == nullptr) {
		send_to_char("E' necessario impugnare un'arma.\n\r", ch);
		return true;
	}
	if(ch->attackers) {
		send_to_char("Non c'e' modo di raggiungere la schiena mentre stai "
		             "combattendo!\n\r", ch);
		return true;
	}
	if(victim->attackers >= 3) {
		send_to_char("Non c'e' abbastanza spazio!\n\r", ch);
		return true;
	}
	if(IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE) && !IsGiant(ch)) {
		act("$N e' troppo gross$B per pugnalarl$B alla schiena.",
		    FALSE, ch, nullptr, victim, TO_CHAR);
		return true;
	}
	if(!off_backstab_weapon_valid(ch)) {
		send_to_char("Puoi pugnalare solo con armi taglienti od appuntite.\n\r", ch);
		return true;
	}
	if(ch->specials.fighting) {
		act("Sei troppo impegnat$b, ora.", FALSE, ch, nullptr, nullptr, TO_CHAR);
		return true;
	}
	if(MOUNTED(ch)) {
		send_to_char("Come puoi sorprendere qualcuno con tutto il rumore che fa "
		             "la tua cavalcatura?\n\r", ch);
		return true;
	}
	return false;
}

int off_backstab_surprise_bonus(struct char_data* victim) {
	return (victim != nullptr && victim->specials.fighting) ? 0 : 4;
}

void off_backstab_barbarian_avoid_achievement(struct char_data* victim) {
	if(victim == nullptr || !HasClass(victim, CLASS_BARBARIAN) || !IS_PC(victim)) {
		return;
	}
	if(IS_POLY(victim)) {
		if(victim->desc == nullptr || victim->desc->original == nullptr) {
			return;
		}
		victim->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_BARBARIAN_2] += 1;
		if(!IS_SET(victim->desc->original->specials.act, PLR_ACHIE)) {
			SET_BIT(victim->desc->original->specials.act, PLR_ACHIE);
		}
	}
	else {
		victim->specials.achievements[CLASS_ACHIE][ACHIE_BARBARIAN_2] += 1;
		if(!IS_SET(victim->specials.act, PLR_ACHIE)) {
			SET_BIT(victim->specials.act, PLR_ACHIE);
		}
	}
	CheckAchie(victim, ACHIE_BARBARIAN_2, CLASS_ACHIE);
}

/** true = la vittima ha evitato e il fight e' gia' stato avviato */
bool off_backstab_try_avoid(struct char_data* ch, struct char_data* victim) {
	if(ch == nullptr || victim == nullptr || victim->skills == nullptr) {
		return false;
	}
	if(!victim->skills[SKILL_AVOID_BACK_ATTACK].learned ||
			GET_POS(victim) <= POSITION_SITTING) {
		return false;
	}

	const byte percent = number(1, 101);
	if(percent < victim->skills[SKILL_AVOID_BACK_ATTACK].learned) {
		act("Ti accorgi del tentativo di attacco di $N e lo eviti abilmente!",
		    FALSE, victim, nullptr, ch, TO_CHAR);
		act("$n evita l'attacco alla schiena di $N!",
		    FALSE, victim, nullptr, ch, TO_ROOM);
		off_backstab_barbarian_avoid_achievement(victim);
		SetVictFighting(ch, victim);
		SetCharFighting(ch, victim);
		off_backstab_add_hated(ch, victim);
		return true;
	}

	act("Non ti sei accorto dell'attacco alla schiena di $N!",
	    FALSE, victim, nullptr, ch, TO_CHAR);
	act("$n non si accorge dell'attacco alla schiena di $N!",
	    FALSE, victim, nullptr, ch, TO_ROOM);
	LearnFromMistake(victim, SKILL_AVOID_BACK_ATTACK, 0, 95);
	return false;
}

bool off_backstab_strike(struct char_data* ch, struct char_data* victim, int hitBonus) {
	if(ch == nullptr || victim == nullptr) {
		return false;
	}
	GET_HITROLL(ch) += hitBonus;
	const DamageResult result = hit(ch, victim, SKILL_BACKSTAB);
	GET_HITROLL(ch) -= hitBonus;
	return result == SubjectDead;
}

/** true = il PG non deve attendere (vittima gia' morta) */
bool off_backstab_resolve(struct char_data* ch, struct char_data* victim, int cmd, int base) {
	if(ch == nullptr || victim == nullptr || ch->skills == nullptr) {
		return true;
	}

	const byte percent = number(1, 101);
	if(!ch->skills[SKILL_BACKSTAB].learned) {
		off_backstab_add_hated(ch, victim);
		return damage(ch, victim, 0, SKILL_BACKSTAB, OFF_BACKSTAB_LOCATION) == SubjectDead;
	}

	if(percent > MIN(100, ch->skills[SKILL_BACKSTAB].learned)) {
		LearnFromMistake(ch, SKILL_BACKSTAB, 0, 95);
		if(AWAKE(victim)) {
			if(damage(ch, victim, 0, SKILL_BACKSTAB, OFF_BACKSTAB_LOCATION) == AllLiving) {
				off_backstab_add_hated(ch, victim);
			}
			return false;
		}
		ActionAlignMod(ch, victim, cmd);
		off_backstab_add_hated(ch, victim);
		return off_backstab_strike(ch, victim, base + 2);
	}

	ActionAlignMod(ch, victim, cmd);
	off_backstab_add_hated(ch, victim);
#if !defined NEW_ALIGN
	if(IS_PC(ch) && IS_PC(victim)) {
		GET_ALIGNMENT(ch) -= 50;
	}
#endif
	return off_backstab_strike(ch, victim, base);
}

void off_backstab_perform(struct char_data* ch, struct char_data* victim, int cmd) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}
	if(off_backstab_try_avoid(ch, victim)) {
		return;
	}
	const int base = off_backstab_surprise_bonus(victim);
	if(!off_backstab_resolve(ch, victim, cmd, base)) {
		WAIT_STATE(ch, PULSE_VIOLENCE * 2);
	}
}

std::string off_order_tell_msg(const char* message) {
	std::string msg = "$N ti ordina di '";
	msg += message;
	msg += "'";
	return msg;
}

std::string off_order_room_msg(const char* message) {
	std::string msg = "$n ordina '";
	msg += message;
	msg += "'.";
	return msg;
}

struct char_data* off_order_resolve_victim(struct char_data* ch, const std::string& target) {
	if(ch == nullptr || off_is_followers_keyword(target.c_str())) {
		return nullptr;
	}
	return off_visible_in_room(ch, target.c_str());
}

void off_order_send_ok(struct char_data* ch) {
	send_to_char("Ok.\n\r", ch);
}

void off_order_emit_indifferent(struct char_data* victim) {
	act("$n fa l'indifferente.", FALSE, victim, nullptr, nullptr, TO_ROOM);
}

void off_order_emit_tell(struct char_data* ch, struct char_data* victim, const std::string& command) {
	const std::string orderMsg = off_order_tell_msg(command.c_str());
	act(orderMsg.c_str(), FALSE, victim, nullptr, ch, TO_CHAR);
	if(GetMaxLevel(ch) < IMMORTALE) {
		act("$n da' un ordine a $N. ", FALSE, ch, nullptr, victim, TO_NOTVICT);
	}
}

bool off_order_charm_blocks(struct char_data* ch) {
	if(ch != nullptr && IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Il tuo padrone di sicuro non approverebbe che tu ti metta a dare ordini...\n\r",
		             ch);
		return true;
	}
	return false;
}

void off_order_mount_rebuke(struct char_data* ch, struct char_data* mount) {
	Dismount(ch, mount, POSITION_SITTING);
	act("$n s'incazza e $N finisce sedut$b per terra!",
	    FALSE, mount, nullptr, ch, TO_NOTVICT);
	act("$n s'incazza e tu cadi di sella!", FALSE, mount, nullptr, ch, TO_VICT);
}

void off_order_handle_mount(struct char_data* ch, struct char_data* mount, const std::string& command) {
	const int ego = MountEgoCheck(ch, mount);
	if(ego > 5) {
		if(RideCheck(ch, -5)) {
			off_order_emit_indifferent(mount);
		}
		else {
			off_order_mount_rebuke(ch, mount);
		}
		return;
	}
	if(ego > 0) {
		off_order_emit_indifferent(mount);
		return;
	}
	off_order_send_ok(ch);
	command_interpreter(mount, command.c_str());
}

bool off_order_victim_ready_for_command(struct char_data* victim) {
	return victim != nullptr &&
	       ((!victim->desc && !victim->specials.tick_to_lag) ||
	        (victim->desc && victim->desc->wait <= 1));
}

void off_order_obey_charmed(struct char_data* ch, struct char_data* victim, const std::string& command) {
	off_order_send_ok(ch);
	WAIT_STATE(victim, (19 - GET_CHR(ch)) * PULSE_VIOLENCE);
	command_interpreter(victim, command.c_str());
}

void off_order_try_non_charmed_obey(struct char_data* ch, struct char_data* victim,
                                  const std::string& command) {
	if(RIDDEN(victim) == ch) {
		off_order_handle_mount(ch, victim, command);
		return;
	}
	if(IS_AFFECTED2(victim, AFF2_CON_ORDER) && victim->master == ch &&
			off_order_victim_ready_for_command(victim)) {
		off_order_send_ok(ch);
		command_interpreter(victim, command.c_str());
		return;
	}
	off_order_emit_indifferent(victim);
}

void off_order_single_target(struct char_data* ch, struct char_data* victim,
                             const std::string& command) {
	if(check_soundproof(victim)) {
		return;
	}
	off_order_emit_tell(ch, victim, command);

	if(victim->master != ch || !IS_AFFECTED(victim, AFF_CHARM)) {
		off_order_try_non_charmed_obey(ch, victim, command);
	}
	else {
		off_order_obey_charmed(ch, victim, command);
	}
}

void off_order_followers(struct char_data* ch, const std::string& command) {
	if(ch == nullptr) {
		return;
	}
	if(!IS_IMMORTALE(ch)) {
		const std::string orderRoom = off_order_room_msg(command.c_str());
		act(orderRoom.c_str(), FALSE, ch, nullptr, nullptr, TO_ROOM);
	}

	const int orgRoom = ch->in_room;
	bool found = false;

	for(struct follow_type* k = ch->followers; k != nullptr; k = k->next) {
		if(orgRoom == k->follower->in_room && IS_AFFECTED(k->follower, AFF_CHARM)) {
			found = true;
			break;
		}
	}

	if(!found) {
		send_to_char("Nessuno da queste parti e' un tuo suddito fedele!\n\r", ch);
		return;
	}

	for(struct follow_type* k = ch->followers; k != nullptr; k = k->next) {
		if(orgRoom == k->follower->in_room && IS_AFFECTED(k->follower, AFF_CHARM)) {
			command_interpreter(k->follower, command.c_str());
		}
	}
	off_order_send_ok(ch);
}

void off_kick_append_pwp_attacker(std::string& msg, int damage) {
	msg += " $c0003[";
	msg += std::to_string(damage);
	msg += "]$c0007";
}

void off_kick_append_pwp_victim(std::string& msg, int damage) {
	msg += " $c0001[";
	if(damage > 0) {
		msg += '-';
	}
	msg += std::to_string(damage);
	msg += "]$c0007";
}

void off_kick_act_char(struct char_data* ch, struct char_data* victim,
                      const std::string& text) {
	char buf[MAX_STRING_LENGTH];
	std::snprintf(buf, sizeof(buf), "%s", text.c_str());
	act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
}

void off_kick_act_victim(struct char_data* ch, struct char_data* victim,
                         const std::string& text) {
	char buf[MAX_STRING_LENGTH];
	std::snprintf(buf, sizeof(buf), "%s", text.c_str());
	act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
}

void off_kick_act_room(struct char_data* ch, struct char_data* victim,
                       const char* roomMsg) {
	act(roomMsg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
}

void off_kick_emit_set(struct char_data* ch, struct char_data* victim,
                       const char* chLine, const char* victLine, const char* roomLine,
                       int damage) {
	std::string msg = chLine;
	if(IS_SET(ch->player.user_flags, PWP_MODE)) {
		off_kick_append_pwp_attacker(msg, damage);
	}
	off_kick_act_char(ch, victim, msg);

	msg = victLine;
	if(IS_SET(victim->player.user_flags, PWP_MODE)) {
		off_kick_append_pwp_victim(msg, damage);
	}
	off_kick_act_victim(ch, victim, msg);
	off_kick_act_room(ch, victim, roomLine);
}

int off_kick_race_message_index(struct char_data* victim) {
	switch(GET_RACE(victim)) {
	case RACE_HUMAN:
	case RACE_ELVEN:
	case RACE_DWARF:
	case RACE_DARK_ELF:
	case RACE_ORC:
	case RACE_LYCANTH:
	case RACE_TROLL:
	case RACE_DEMON:
	case RACE_DEVIL:
	case RACE_MFLAYER:
	case RACE_ASTRAL:
	case RACE_PATRYN:
	case RACE_SARTAN:
	case RACE_DRAAGDIM:
	case RACE_GOLEM:
	case RACE_TROGMAN:
	case RACE_LIZARDMAN:
	case RACE_HALF_ELVEN:
	case RACE_HALF_OGRE:
	case RACE_HALF_ORC:
	case RACE_HALF_GIANT:
		return number(0, 3);
	case RACE_PREDATOR:
	case RACE_HERBIV:
	case RACE_LABRAT:
		return number(4, 6);
	case RACE_REPTILE:
	case RACE_DRAGON:
	case RACE_DRAGON_RED:
	case RACE_DRAGON_BLACK:
	case RACE_DRAGON_GREEN:
	case RACE_DRAGON_WHITE:
	case RACE_DRAGON_BLUE:
	case RACE_DRAGON_SILVER:
	case RACE_DRAGON_GOLD:
	case RACE_DRAGON_BRONZE:
	case RACE_DRAGON_COPPER:
	case RACE_DRAGON_BRASS:
		return number(4, 7);
	case RACE_TREE:
		return 8;
	case RACE_PARASITE:
	case RACE_SLIME:
	case RACE_VEGGIE:
	case RACE_VEGMAN:
		return 9;
	case RACE_ROO:
	case RACE_GNOME:
	case RACE_HALFLING:
	case RACE_GOBLIN:
	case RACE_SMURF:
	case RACE_ENFAN:
		return 10;
	case RACE_GIANT:
	case RACE_GIANT_HILL:
	case RACE_GIANT_FROST:
	case RACE_GIANT_FIRE:
	case RACE_GIANT_CLOUD:
	case RACE_GIANT_STORM:
	case RACE_GIANT_STONE:
	case RACE_TYTAN:
	case RACE_GOD:
		return 11;
	case RACE_GHOST:
		return 12;
	case RACE_BIRD:
	case RACE_SKEXIE:
		return 13;
	case RACE_UNDEAD:
	case RACE_UNDEAD_VAMPIRE:
	case RACE_UNDEAD_LICH:
	case RACE_UNDEAD_WIGHT:
	case RACE_UNDEAD_GHAST:
	case RACE_UNDEAD_SPECTRE:
	case RACE_UNDEAD_ZOMBIE:
	case RACE_UNDEAD_SKELETON:
	case RACE_UNDEAD_GHOUL:
		return 14;
	case RACE_DINOSAUR:
		return 15;
	case RACE_INSECT:
	case RACE_ARACHNID:
		return 16;
	case RACE_FISH:
		return 17;
	default:
		return 18;
	}
}

int off_kick_fighter_class(struct char_data* ch) {
	int dummy = 0;
	int carry = 0;
	WEARING_N(ch, dummy, carry);
	const unsigned carried =
		static_cast<unsigned>(IS_CARRYING_N(ch)) + static_cast<unsigned>(carry);
	if(HasClass(ch, CLASS_MONK) &&
	   !((ch->equipment[WIELD]) &&
	     (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) &&
	   !((ch->equipment[HOLD]) &&
	     (ch->equipment[HOLD]->obj_flags.type_flag == ITEM_WEAPON)) &&
	   (carried < static_cast<unsigned>(MONK_MAX_RENT) + 5u)) {
		return CLASS_MONK;
	}
	if(HasClass(ch, CLASS_BARBARIAN)) {
		return CLASS_BARBARIAN;
	}
	return CLASS_WARRIOR;
}

void off_kick_adjust_damage(struct char_data* victim, int fighterClass, int& damage) {
	if(fighterClass == CLASS_MONK) {
		return;
	}
	if(IS_SET(victim->susc, IMM_BLUNT)) {
		damage <<= 1;
	}
	if(IS_SET(victim->immune, IMM_BLUNT)) {
		damage >>= 1;
	}
	if(fighterClass != CLASS_BARBARIAN) {
		if(IS_SET(victim->M_immune, IMM_BLUNT)) {
			damage = 0;
		}
	}
	else if(IS_SET(victim->M_immune, IMM_BLUNT)) {
		damage >>= 1;
	}
}

} // namespace

ACTION_FUNC(do_ripudia) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_ripudia (act.off.cpp)");
		return;
	}

	const std::string target = off_parse_first_token(arg);

	struct char_data* victim = nullptr;
	if(!target.empty()) {
		victim = off_visible_in_room(ch, target.c_str());
	}

	if(victim == nullptr) {
		off_ripudia_renounce_absent_prince(ch, target);
		return;
	}

	if(off_combat_blocks_vassal_act(ch, victim)) {
		return;
	}

	if(IS_VASSALLOOF(ch, GET_NAME(victim))) {
		off_ripudia_vassal_breaks_oath(ch, victim);
	}
	else if(IS_PRINCEOF(GET_NAME(ch), victim)) {
		off_ripudia_prince_expels(ch, victim);
	}
}


ACTION_FUNC(do_associa) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_associa (act.off.cpp)");
		return;
	}

	if(!IS_PRINCE(ch)) {
		send_to_char("Presuntuosetto, eh?\n\r", ch);
		return;
	}

	const std::string target = off_parse_first_token(arg);
	struct char_data* victim = nullptr;
	if(!target.empty()) {
		victim = off_visible_in_room(ch, target.c_str());
	}

	if(off_associa_target_missing(ch, victim) ||
			off_associa_victim_is_poly(ch, victim) ||
			off_combat_blocks_vassal_act(ch, victim) ||
			off_associa_victim_too_young(ch, victim) ||
			off_associa_victim_already_sworn(ch, victim)) {
		return;
	}

	const long cost = off_associa_nomination_cost(ch, victim);
	if(GET_GOLD(ch) < cost) {
		act("Ti costerebbe troppo...", TRUE, ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	off_associa_complete(ch, victim, cost);
}

ACTION_FUNC(do_vomita) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_vomita (act.off.cpp)");
		return;
	}

	off_vomita_perform(ch);
}

ACTION_FUNC(do_hit) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_hit (act.off.cpp)");
		return;
	}

	if(off_at_peace(ch, "Non in questo luogo di pace.\n\r")) {
		return;
	}

	const std::string target = off_parse_first_token(arg);
	if(target.empty()) {
		send_to_char("Chi vuoi colpire?\n\r", ch);
		return;
	}

	struct char_data* victim = off_visible_in_room(ch, target.c_str());
	if(victim == nullptr) {
		send_to_char("Non e' qui.\n\r", ch);
		return;
	}

	if(victim == ch) {
		off_hit_self(ch);
		return;
	}

	off_hit_strike(ch, victim, cmd);
}

ACTION_FUNC(do_slay) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_slay (act.off.cpp)");
		return;
	}

	const std::string target = off_parse_first_token(arg);
	if(target.empty()) {
		send_to_char("Uccidere chi?\n\r", ch);
		return;
	}

	struct char_data* victim = off_visible_in_room(ch, target.c_str());
	if(victim == nullptr) {
		send_to_char("Non e' qui.\n\r", ch);
		return;
	}

	if(!IS_NPC(victim)) {
		act("Non e' un bersaglio valido.", FALSE, ch, nullptr, victim, TO_CHAR);
		return;
	}

	off_slay_perform(ch, victim);
}
ACTION_FUNC(do_kill) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_kill (act.off.cpp)");
		return;
	}

	if(off_kill_delegates_to_hit(ch)) {
		do_hit(ch, arg, 0);
		return;
	}

	const std::string target = off_parse_first_token(arg);
	if(target.empty()) {
		send_to_char("Chi vuoi uccidere?\n\r", ch);
		return;
	}

	struct char_data* victim = off_visible_in_room(ch, target.c_str());
	if(victim == nullptr) {
		send_to_char("Non e' qui.\n\r", ch);
		return;
	}

	off_kill_perform(ch, victim);
}



ACTION_FUNC(do_backstab) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_backstab (act.off.cpp)");
		return;
	}

	if(ch->skills == nullptr) {
		return;
	}

	if(off_at_peace(ch, "C'e' troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	const std::string target = off_parse_first_token(arg);
	struct char_data* victim = off_visible_in_room(ch, target.c_str());
	if(victim == nullptr) {
		send_to_char("Chi vuoi accoltellare?\n\r", ch);
		return;
	}

	if(off_backstab_validate(ch, victim)) {
		return;
	}

	off_backstab_perform(ch, victim, cmd);
}



ACTION_FUNC(do_order) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_order (act.off.cpp)");
		return;
	}

	if(apply_soundproof(ch)) {
		return;
	}

	const auto [target, command] = chop_argument(arg, 99, 255);
	if(target.empty() || command.empty()) {
		send_to_char("Ordinare a chi di fare cosa?!?\n\r", ch);
		return;
	}

	struct char_data* victim = off_order_resolve_victim(ch, target);
	if(victim == nullptr && !off_is_followers_keyword(target.c_str())) {
		send_to_char("Quella persona non e' qui.\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Mi sa che soffri di seri disturbi della personalita'...\n\r", ch);
		return;
	}

	if(off_order_charm_blocks(ch)) {
		return;
	}

	if(victim != nullptr) {
		off_order_single_target(ch, victim, command);
	}
	else {
		off_order_followers(ch, command);
	}
}

ACTION_FUNC(do_order_old) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_order_old (act.off.cpp)");
		return;
	}

	bool found = FALSE;
	int org_room;
	struct char_data* victim;
	struct follow_type* k;


	if(apply_soundproof(ch)) {
		return;
	}

	const auto [targetName, orderCmd] = chop_argument(arg, 99, 255);

	if(targetName.empty() || orderCmd.empty()) {
		send_to_char("Ordinare a chi di fare cosa?!?\n\r", ch);
	}
	else if(!(victim = off_visible_in_room(ch, targetName.c_str())) &&
	        !off_is_followers_keyword(targetName.c_str())) {
		send_to_char("Quella persona non e' qui.\n\r", ch);
	}
	else if(ch == victim) {
		send_to_char("Mi sa che soffri di seri disturbi della personalita'...\n\r", ch);
	}

	else {
		if(IS_AFFECTED(ch, AFF_CHARM)) {
			send_to_char("Il tuo padrone di sicuro non approverebbe che tu ti metta a dare ordini...\n\r",
			             ch);
			return;
		}

		if(victim) {
			if(check_soundproof(victim)) {
				return;
			}
			{
				const std::string order_msg = off_order_tell_msg(orderCmd.c_str());
				act(order_msg.c_str(), FALSE, victim, 0, ch, TO_CHAR);
			}
			act("$n da' un ordine a $N.", FALSE, ch, 0, victim, TO_NOTVICT);

			if(victim->master != ch || !IS_AFFECTED(victim, AFF_CHARM)) {
				if(RIDDEN(victim) == ch) {
					int check;
					check = MountEgoCheck(ch, victim);
					if(check > 5) {
						if(RideCheck(ch, -5)) {
							act("$n fa l'indifferente.", FALSE, victim, 0, 0, TO_ROOM);
						}
						else {
							Dismount(ch, victim, POSITION_SITTING);
							act("$n s'incazza e $N finisce sedut$b per terra!",
								FALSE, victim, 0, ch, TO_NOTVICT);
							act("$n s'incazza e tu cadi di sella!", FALSE, victim, 0, ch, TO_VICT);
						}
					}
					else if(check > 0) {
						act("$n fa l'indifferente.", FALSE, victim, 0, 0, TO_ROOM);
					}
					else {
						send_to_char("Ok.\n\r", ch);
						command_interpreter(victim, orderCmd.c_str());
					}
				}
				else if(IS_AFFECTED2(victim, AFF2_CON_ORDER) && victim->master==ch &&
						((!victim->desc && !victim->specials.tick_to_lag) ||
						 (victim->desc && victim->desc->wait <= 1))) {
					send_to_char("Ok.\n\r", ch);
					command_interpreter(victim, orderCmd.c_str());
				}
				else {
					act("$n fa l'indifferente.", FALSE, victim, 0, 0, TO_ROOM);
				}
			}
			else {
				send_to_char("Ok.\n\r", ch);
				WAIT_STATE(victim, (19-GET_CHR(ch)) * PULSE_VIOLENCE);   // order

				command_interpreter(victim, orderCmd.c_str());
			}
		}
		else {
			/* This is order "followers" */
			{
				const std::string order_room = off_order_room_msg(orderCmd.c_str());
				act(order_room.c_str(), FALSE, ch, 0, victim, TO_ROOM);
			}

			org_room = ch->in_room;

			for(k = ch->followers; k; k = k->next) {
				if(org_room == k->follower->in_room) {
					if(IS_AFFECTED(k->follower, AFF_CHARM)) {
						found = TRUE;
					}
				}
			}

			if(found) {
				for(k = ch->followers; k; k = k->next) {
					if(org_room == k->follower->in_room) {
						if(IS_AFFECTED(k->follower, AFF_CHARM)) {
							command_interpreter(k->follower, orderCmd.c_str());
						}
					}
				}
				send_to_char("Ok.\n\r", ch);
			}
			else {
				send_to_char("Nessuno da queste parti e' un tuo suddito fedele!\n\r", ch);
			}
		}
	}
}



ACTION_FUNC(do_flee) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_flee (act.off.cpp)");
		return;
	}

	int i, attempt, loose, die, percent, charm;
	int panic=FALSE;
	int bonus=0;

	void gain_exp(struct char_data *ch, int gain);
	int special(struct char_data *ch, int cmd, char* tmp);
	char buf[250];
	if(IS_AFFECTED(ch, AFF_PARALYSIS)) {
		return;
	}

	if(GET_POS(ch) < POSITION_SLEEPING) {
		send_to_char("Non in questa posizione!\n\r", ch);
		return;
	}

	if(IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
		send_to_char("Non pensi ad altro che alla battaglia!\n\r", ch);
		return;
	}

	if(affected_by_spell(ch, SPELL_WEB)) {
		if(!saves_spell(ch, SAVING_PARA)) {
			WAIT_STATE(ch, PULSE_VIOLENCE); // flee
			send_to_char("Sei intrappolat$b nelle ragnatele, non puoi muoverti!\n\r", ch);
			act("$n si dibatte nelle ragnatele.", FALSE, ch, 0, 0, TO_ROOM);
			return;
		}
		else {
			send_to_char("Ti liberi dalla ragnatela appiccicosa!\n\r", ch);
			act("$n riesce a liberarsi dalla ragnatela!", FALSE, ch, 0, 0, TO_ROOM);
			affect_from_char(ch,SPELL_WEB);
			GET_MOVE(ch) -=50;
			alter_move(ch,0);
		}
	}

	if(GET_POS(ch) <= POSITION_SITTING) {
		GET_MOVE(ch) -=10;
		alter_move(ch,0);
		act("$n si rialza in preda al panico.", TRUE, ch, 0, 0, TO_ROOM);
		act("In preda al panico, riesci a malapena ad alzarti.", TRUE, ch, 0, 0,
			TO_CHAR);
		GET_POS(ch) = POSITION_STANDING;
		WAIT_STATE(ch, PULSE_VIOLENCE); // flee
		return;
	}

	if(!ch->specials.fighting) {
		for(i = 0; i < 6; i++) {
			attempt = number(0, 5);  /* Select a random direction */
			if(CAN_GO(ch, attempt) &&
					!IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {
				snprintf(buf,249,"$n cerca di fuggire %s, apparentemente senza motivo.",
						 dirsTo[attempt]);
				act(buf, TRUE, ch, 0, 0, TO_ROOM);

				if(RIDDEN(ch)) {
					if((die = MoveOne(RIDDEN(ch), attempt, TRUE)) == TRUE) {
						/* The escape has succeded */
						snprintf(buf,249,"Corri a gambe levate %s.\n\r",dirsTo[attempt]);
						send_to_char(buf, ch);
						return;
					}
					else {
						if(!die)
							act("$n cerca di correre via ma e' incapace di muoversi!", TRUE, ch, 0, 0,
								TO_ROOM);
						return;
					}
				}
				else {
					if((die = MoveOne(ch, attempt, TRUE)) == TRUE) {
						/* The escape has succeded */
						snprintf(buf,249,"Corri a gambe levate %s.\n\r",dirsTo[attempt]);
						send_to_char(buf, ch);
						StopAllFightingWith(ch);
						return;
					}
					else {
						if(!die)
							act("$n cerca di fuggire ma e' incapace di muoversi!", TRUE, ch, 0, 0,
								TO_ROOM);
						return;
					}
				}
			}
		} /* for */
		/* No exits was found */
		send_to_char("PANICO! Non riesci a scappare!\n\r", ch);
		return;
	} /* fine se non fighting */

	/* Bene , stiamo combattendo.. le cose sono un po' piu' incasinate! */
	if(IS_PC(ch->specials.fighting)) {
		bonus=20;
	}
	for(i = 0; i < 3; i++) {
		attempt = number(0, 5);  /* Select a random direction */
		if(CAN_GO(ch, attempt) &&
				!IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {

			if(!ch->skills || (number(1,101) > ch->skills[SKILL_RETREAT].learned+bonus)) {
				snprintf(buf,249,"$n va in panico e cerca di fuggire %s",dirsTo[attempt]);
				act(buf, TRUE, ch, 0, 0, TO_ROOM);
				panic = TRUE;
				LearnFromMistake(ch, SKILL_RETREAT, 0, 90);
			}
			else {
				snprintf(buf,249,"$n abilmente si ritira %s.",dirsTo[attempt]);
				act(buf, TRUE, ch, 0, 0, TO_ROOM);
				panic = FALSE;
			}


			if(IS_AFFECTED(ch, AFF_CHARM)) {
				charm = TRUE;
				REMOVE_BIT(ch->specials.affected_by, AFF_CHARM);
			}
			else {
				charm = FALSE;
			}

			if(RIDDEN(ch)) {
				die = MoveOne(RIDDEN(ch), attempt, TRUE);
			}
			else {
				die = MoveOne(ch, attempt, TRUE);
			}

			if(charm) {
				SET_BIT(ch->specials.affected_by, AFF_CHARM);
			}

			if(die == TRUE) {
				loose = 0;
				/* The escape has succeded. We'll be nice. */
				if(bonus>0) {
					panic =FALSE;
				}
				if(GetMaxLevel(ch) > CHUMP) {
					if(panic) {
						/*loose = GetMaxLevel(ch)+( GetSecMaxLev(ch) / 2 )+( GetThirdMaxLev(ch) / 3 );
						loose -= GetMaxLevel(ch->specials.fighting)+
							( GetSecMaxLev(ch->specials.fighting) / 2 )+
							( GetThirdMaxLev(ch->specials.fighting ) / 3 );
						if( loose < 0 )
							loose = 1;
						loose *=10000;*/
					}
				}
				if(IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF) &&
								   !(IS_SET(ch->specials.act, ACT_AGGRESSIVE)))) {
					AddFeared(ch, ch->specials.fighting);
				}
				else {
					percent = (100 * GET_HIT(ch->specials.fighting)) /
							  GET_MAX_HIT(ch->specials.fighting);
					if(Hates(ch->specials.fighting, ch)) {
						SetHunting(ch->specials.fighting, ch);
					}
					else if((IS_GOOD(ch) && (IS_EVIL(ch->specials.fighting))) ||
							(IS_EVIL(ch) && (IS_GOOD(ch->specials.fighting)))) {
						AddHated(ch->specials.fighting, ch);
						SetHunting(ch->specials.fighting, ch);
					}
					else if(number(1,101) < percent) {
						AddHated(ch->specials.fighting, ch);
						SetHunting(ch->specials.fighting, ch);
					}
				}

				if(IS_PC(ch) && panic) {
					if(HasClass(ch, CLASS_MONK | CLASS_WARRIOR | CLASS_BARBARIAN |
								CLASS_PALADIN | CLASS_RANGER))
						if(loose>0 && cmd!=999) {
							gain_exp(ch, -(loose/HowManyClasses(ch)));
							snprintf(buf,249,"La tua vigliaccheria ti e' costata"
									 " %d punti esperienza.\n\r", loose);
							send_to_char(buf,ch);
						}

				}
				if(panic) {
					send_to_char("Scappi a gambe levate.\n\r", ch);
				}
				else {
					char szBuffer[ 100 ];
					snprintf(szBuffer, 99,"Ti ritiri abilmente %s.\n\r",
							 dirsTo[ attempt ]);
					send_to_char(szBuffer, ch);
				}
				StopAllFightingWith(ch);
				if(ch->specials.fighting) {
					stop_fighting(ch);
				}
				return;
			}
			else {
				if(!die)
					act("$n cerca di scappare ma e' incapace di muoversi!", TRUE, ch, 0, 0,
						TO_ROOM);
				return;
			}
		} /*If can go */
	} /* for */
	/* No exits were found */
	send_to_char("PANICO! Non riesci a scappare!\n\r", ch);
}



ACTION_FUNC(do_bash) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_bash (act.off.cpp)");
		return;
	}

	struct char_data* victim;
	char name[256];
	int percent=0;
	int pesi=0;
	room_data* pRoom = real_roomp(ch->in_room);
	int location = 5 ;  /* Gaia 2001 */

	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch,"C'e' troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
    {
		if(!HasClass(ch, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER | CLASS_BARBARIAN) && cmd == CMD_BASH)
        {
            if(IS_POLY(ch) && IS_SET(ch->specials.act, ACT_WARRIOR | ACT_RANGER | ACT_BARBARIAN | ACT_PALADIN) && cmd == 0)
            {
                // do nothing
            }
            else
            {
                send_to_char("Solo i combattenti possono farlo!\n\r", ch);
                return;
            }
		}

		if(!IsHumanoid(ch)) {
			send_to_char("Non hai la forma adatta!\n\r", ch);
			return;
		}
	}

	if(pRoom->sector_type == SECT_UNDERWATER) {
		send_to_char("Sotto'acqua? Meglio di no.\n", ch);
		return;
	}


	only_argument(arg, name);

	if(!(victim = off_visible_in_room(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Chi vuoi colpire?\n\r", ch);
			return;
		}
	}


	if(victim == ch) {
		act("Molto spiritos$b...", FALSE, ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	if(IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
		if(!IsGiant(ch)) {
			act("$N e' TROPPO gross$B per essere colpit$B!", FALSE, ch, 0, victim,
				TO_CHAR);
			return;
		}
	}

	if(MOUNTED(victim)) {
		send_to_char("Non puoi colpire la cavalcatura di qualcun altro!\n\r", ch);
		return;
	}

	if(MOUNTED(ch)) {
		send_to_char("Non puoi colpire mentre cavalchi!\n\r", ch);
		return;
	}

	if(ch->attackers > 3) {
		send_to_char("Non c'e' abbastanza spazio!\n\r",ch);
		return;
	}

	if(victim->attackers >= 6) {
		send_to_char("Non riesci ad avvicinarti abbastanza per colpirlo!\n\r", ch);
		return;
	}

	percent = number(10, 100);   /* Lo skill bash arriva al massimo a 90 */
	/* GGPATCH : migliora la percentuale di riuscita del primo bash*/
	if(!ch->specials.fighting) {

		percent = number(10,60);
		/*
		 mudlog( LOG_CHECK,   "percent for 1st bash %d vs %d", percent,
		       ch->skills[ SKILL_BASH ].learned );
		*/
	}
	else {
		mudlog(LOG_CHECK, "percent for    bash %d vs %d", percent,
			   ch->skills[ SKILL_BASH ].learned);
	}


	/* GGEND   */
	/* some modifications to account for dexterity, and level */

	percent -= dex_app[(int)GET_DEX(ch) ].reaction * 10;
	percent += dex_app[(int)GET_DEX(victim) ].reaction * 10;
	percent += (int)(((GetMaxLevel(victim)-10) - GetMaxLevel(ch)) / 1.5);
#if NEW_BASH
	/* aggiungo un coefficiente di peggioramento legato al numero di oggetti
	 * trasportati e al loro peso - Aggiungo una parentesi Gaia 2001 */
	pesi += (((35 * IS_CARRYING_W(ch))/CAN_CARRY_W(ch))-20);
	pesi += (((30 * IS_CARRYING_N(ch))/CAN_CARRY_N(ch))-15);
	percent+=MAX(30,MAX(-5,pesi));
#endif
	/*  mudlog( LOG_CHECK, "percent for bash adj %d vs %d", percent,
	          ch->skills[ SKILL_BASH ].learned );
	 */
	if(percent > MIN(100, ch->skills[ SKILL_BASH ].learned)) {
		if(GET_POS(victim) > POSITION_DEAD) {
			if(damage(ch, victim, 0, SKILL_BASH, location) == SubjectDead) {
				return;
			}

			/* ACIDUS 2003: viene messo a sedere anche se \E8 polymorphato (\E8 un pg) */
			if(HasHands(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
				GET_POS(ch) = POSITION_SITTING;
			}

		}
		LearnFromMistake(ch, SKILL_BASH, 0, 90);
		if(CheckEquilibrium(ch)) {   //Acidus 2003 - skill better equilibrium
			WAIT_STATE(ch, PULSE_VIOLENCE * 2);   // bash
		}
		else {
			WAIT_STATE(ch, PULSE_VIOLENCE * 3);    // bash
		}

        if(HasClass(ch, CLASS_WARRIOR) && IS_PC(ch))
        {
            if(IS_POLY(ch))
            {
                ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_WARRIOR_2] += 1;
                if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                {
                    SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                }
            }
            else
            {
                ch->specials.achievements[CLASS_ACHIE][ACHIE_WARRIOR_2] += 1;
                if(!IS_SET(ch->specials.act,PLR_ACHIE))
                {
                    SET_BIT(ch->specials.act, PLR_ACHIE);
                }
            }

            CheckAchie(ch, ACHIE_WARRIOR_2, CLASS_ACHIE);
        }

	}
	else {
		if(!CheckMirror(victim)) {
			ActionAlignMod(ch,victim,cmd);
			if(GET_POS(victim) > POSITION_DEAD) {
				if(damage(ch, victim, 2, SKILL_BASH, location) != VictimDead) {
					if(CheckEquilibrium(victim)) {
						WAIT_STATE(victim, PULSE_VIOLENCE * 1);   // bash
					}
					else {
						WAIT_STATE(victim, PULSE_VIOLENCE * 2);    // bash
					}

					if(HasHands(victim) || IS_SET(victim->specials.act,ACT_POLYSELF)) {
						GET_POS(victim) = POSITION_SITTING;
					}
				}
			}
			WAIT_STATE(ch, PULSE_VIOLENCE * 2);
		}

        if(HasClass(ch, CLASS_WARRIOR) && IS_PC(ch))
        {
            if(IS_POLY(ch))
            {
                ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_WARRIOR_1] += 1;
                if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                {
                    SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                }
            }
            else
            {
                ch->specials.achievements[CLASS_ACHIE][ACHIE_WARRIOR_1] += 1;
                if(!IS_SET(ch->specials.act,PLR_ACHIE))
                {
                    SET_BIT(ch->specials.act, PLR_ACHIE);
                }
            }

            CheckAchie(ch, ACHIE_WARRIOR_1, CLASS_ACHIE);
        }

	}
}




ACTION_FUNC(do_rescue) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_rescue (act.off.cpp)");
		return;
	}

	struct char_data* victim, *tmp_ch;
	int percent;
	char victim_name[240];


	if(!ch->skills) {
		send_to_char("Non riesci a completare il salvataggio.\n\r", ch);
		return;
	}

#if 0
	if(!IS_PC(ch) && cmd && !IS_SET(ch->specials.act,ACT_POLYSELF)) {
		return;
	}
#endif

	if(check_peaceful(ch, "Qui non c'e' bisogno di salvare nessuno.\n\r")) {
		return;
	}

    if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
    {
        if(!HasClass(ch, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER | CLASS_BARBARIAN) && cmd == CMD_RESCUE)
        {
            if(IS_POLY(ch) && IS_SET(ch->specials.act, ACT_WARRIOR | ACT_RANGER | ACT_BARBARIAN | ACT_PALADIN) && cmd == 0)
            {
                // do nothing
            }
            else if(!IS_PRINCE(ch))
            {
                send_to_char("Non sei un guerriero!\n\r", ch);
                return;
            }
        }
	}

	only_argument(arg, victim_name);

	if(!(victim = off_visible_in_room(ch, victim_name))) {
		if(HAS_PRINCE(ch)) {
			victim=off_visible_in_room(ch,GET_PRINCE(ch));
		}
		if(!victim) {
			send_to_char("Chi vuoi salvare?\n\r", ch);
			return;
		}
	}

	if(victim == ch) {
		send_to_char("Perche' non provi a fuggire?\n\r", ch);
		return;
	}

	if(MOUNTED(victim)) {
		send_to_char("Non puoi salvare qualcuno a cavallo!\n\r", ch);
		return;
	}

	if(ch->specials.fighting == victim) {
		send_to_char("Come puoi salvare qualcuno che stai cercando di uccidere?\n\r", ch);
		return;
	}

	if(victim->attackers >= 6) {
		act("Non riesci ad avvicinarti abbastanza per salvare $L!", FALSE, ch, 0, victim,
			TO_CHAR);
		return;
	}

	for(tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
			(tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room) ;

	if(!tmp_ch) {
		act("Ma nessuno sta combattendo contro $L?", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}


	percent=number(1,101); /* 101% is a complete failure */
	if(IS_PRINCEOF(GET_NAME(victim),ch)) {
		percent=0;
	}
	if((percent > ch->skills[SKILL_RESCUE].learned)) {
		send_to_char("Non riesci a completare il salvataggio.\n\r", ch);
		LearnFromMistake(ch, SKILL_RESCUE, 0, 90);
		WAIT_STATE(ch, PULSE_VIOLENCE); // rescue
		return;
	}
	ActionAlignMod(ch,victim,cmd);
	act("Banzai! Corri in soccorso di $N!", FALSE, ch, 0, victim, TO_CHAR);
	act("$N ti salva, sei confus$b!", FALSE, victim, 0, ch, TO_CHAR);
	act("$n interviene eroicamente a salvare $N.", FALSE, ch, 0, victim, TO_NOTVICT);
	if(IS_PC(ch) && IS_PC(victim) && !IS_IMMORTAL(ch)) {
		GET_ALIGNMENT(ch)+=20;
	}

	if(victim->specials.fighting == tmp_ch) {
		stop_fighting(victim);
	}
	if(tmp_ch->specials.fighting) {
		stop_fighting(tmp_ch);
	}
	if(ch->specials.fighting) {
		stop_fighting(ch);
	}

	set_fighting(ch, tmp_ch);
	set_fighting(tmp_ch, ch);

	WAIT_STATE(victim, 2*PULSE_VIOLENCE); // rescue

    if(HasClass(ch, CLASS_WARRIOR) && IS_PC(victim) && IS_PC(ch))
    {
        if(IS_POLY(ch))
        {
            ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_WARRIOR_3] += 1;
            if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
            {
                SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
            }
        }
        else
        {
            ch->specials.achievements[CLASS_ACHIE][ACHIE_WARRIOR_3] += 1;
            if(!IS_SET(ch->specials.act,PLR_ACHIE))
            {
                SET_BIT(ch->specials.act, PLR_ACHIE);
            }
        }

        CheckAchie(ch, ACHIE_WARRIOR_3, CLASS_ACHIE);
    }

}



ACTION_FUNC(do_support) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_support (act.off.cpp)");
		return;
	}

	struct char_data* victim;
	char victim_name[240];
    char buf[255];

	only_argument(arg, victim_name);

    if(!*victim_name && ch->specials.supporting && !ch->specials.fighting)
    {
        sprintf(buf, "Al momento stai supportando %s, non te lo ricordavi?\n\r", ch->specials.supporting);
        send_to_char(buf, ch);
        return;
    }

	if(!(victim = off_visible_in_room(ch, victim_name))) {
		send_to_char("Chi vorresti supportare, esattamente?\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Oh beh, chi fa da se' fa per tre!\n\r", ch);
		if(ch->specials.supporting) { //ACIDUS 2003 - il support deve andare via come con il bodyguard
			free(ch->specials.supporting);
			ch->specials.supporting=NULL;
		}
		return;
	}

	if(ch->specials.fighting == victim) {
		send_to_char("Non mi sembra una grande idea.\n\r",ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Pensa a combattere!\n\r",ch);
		return;
	}
	if(ch->specials.supporting) {
		free(ch->specials.supporting);
	}
	ActionAlignMod(ch,victim,cmd);
	ch->specials.supporting=strdup(victim->player.name);
    sprintf(buf, "Ok, ora supporti %s.\n\r", ch->specials.supporting);
    send_to_char(buf, ch);

}

ACTION_FUNC(do_bodyguard) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_bodyguard (act.off.cpp)");
		return;
	}

	struct char_data* victim,*lg;
	char victim_name[240];

	only_argument(arg, victim_name);

	if(!(victim = off_visible_in_room(ch, victim_name))) {
		send_to_char("Di chi vorresti fare la guardia del corpo.. esattamente?\n\r", ch);
		return;
	}

	if(victim == ch) {
		if(ch->specials.bodyguarding) {
			lg=get_char(ch->specials.bodyguarding);
			if(lg) {
				actall("Smetti di proteggere",
					   "smette di proteggerti",
					   "smette di proteggere",
					   ch,lg);
				free(ch->specials.bodyguarding);
				ch->specials.bodyguarding=(char*)NULL;
				free(lg->specials.bodyguard);
				lg->specials.bodyguard=(char*)NULL;
			}
			else {
				send_to_char("Chi fa da se'... fa per tre!", ch);
			}
		}
		return;
	}

	if(ch->specials.fighting == victim) {
		send_to_char("Non mi sembra una grande idea.\n\r",ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Pensa a combattere!\n\r",ch);
		return;
	}
	if(GetMaxLevel(ch)-GetMaxLevel(victim)>5) {
		act("BODYGUARD.. non BALIA! Cerca uno piu' grosso di $N a cui offrire"
			" i tuoi servigi!",FALSE,ch,0,victim,TO_CHAR);
		act("$n e' di livello troppo alto per farti da guardia del corpo!",
			FALSE,ch,0,victim,TO_VICT);
		return;
	}

	if(ch->specials.bodyguarding) {
		free(ch->specials.bodyguarding);
	}
	ch->specials.bodyguarding=strdup(GET_NAME(victim)); // SALVO meglio il vero nome
	if(victim->specials.bodyguard) {
		free(victim->specials.bodyguard);
	}
	victim->specials.bodyguard=strdup(GET_NAME(ch));
	act("Giuri di proteggere $N a rischio della tua stessa vita.",
		FALSE,ch,0,victim,TO_CHAR);
	act("$n si impegna a proteggerti, a rischio della sua stessa vita!",
		FALSE,ch,0,victim,TO_VICT);
	act("$n si impegna a proteggere $N a rischio della sua stessa vita!",
		FALSE,ch,0,victim,TO_NOTVICT);
	if(victim->desc) {
		ActionAlignMod(ch,victim,cmd);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE); /* same as hit */
}

ACTION_FUNC(do_assist) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_assist (act.off.cpp)");
		return;
	}

	struct char_data* victim, *tmp_ch;
	char victim_name[240];

	if(check_peaceful(ch, "Qui nessuno ha bisogno di aiuto.\n\r")) {
		return;
	}

	only_argument(arg, victim_name);

	if(!(victim = off_visible_in_room(ch, victim_name))) {
		send_to_char("Al fianco di chi vuoi entrare in combattimento?\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Oh, certo, aiutati da solo...\n\r", ch);
		return;
	}

	if(ch->specials.fighting == victim) {
		send_to_char("Sarebbe controproducente, no?\n\r", ch);
		return;
	}

	if(ch->specials.fighting) {
		send_to_char("Hai gia' le mani occupate.\n\r", ch);
		return;
	}

	if(victim->attackers >= 6) {
		send_to_char("Non riesci ad avvicinarti abbastanza per entrare in combattimento!\n\r",
			ch);
		return;
	}


	tmp_ch = victim->specials.fighting;
	if(!tmp_ch) {
		act("Ma $L non sta combattendo con nessuno.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if(tmp_ch->in_room !=ch->in_room) {
		send_to_char("Ops, se n'e' andato di corsa, devi averlo spaventato!\n\r", ch);
		return;
	}

	/*if(ch->specials.supporting) ACIDUS 2003 - il support non deve andare via da solo
	{
	   free(ch->specials.supporting);
	   ch->specials.supporting=NULL;
	}*/
	hit(ch, tmp_ch, TYPE_UNDEFINED);
	ActionAlignMod(ch,victim,cmd);
	WAIT_STATE(victim, PULSE_VIOLENCE+2); /* same as hit */
}



ACTION_FUNC(do_kick) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_kick (act.off.cpp)");
		return;
	}

	struct char_data* victim;
	int dam;
	byte percent;
	int location = 5;    /* Gaia 2001 */
	if(!ch->skills) {
		return;
	}

	if(off_at_peace(ch, "Non in questo luogo di pace.\n\r")) {
		return;
	}

    if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
    {
        if(!HasClass(ch, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER | CLASS_BARBARIAN | CLASS_MONK) && cmd == CMD_KICK)
        {
            if(IS_POLY(ch) && IS_SET(ch->specials.act, ACT_WARRIOR | ACT_RANGER | ACT_BARBARIAN | ACT_PALADIN | ACT_MONK) && cmd == 0)
            {
                // do nothing
            }
            else
            {
                send_to_char("Non puoi farlo!\n\r", ch);
                return;
            }
        }

		if(!IsHumanoid(ch)) {
			send_to_char("Non hai la forma adatta!\n\r", ch);
			return;
		}
	}

	const std::string target = off_parse_first_token(arg);

	victim = off_visible_or_fighting(ch, target.c_str(), "Prendere a calci chi?\n\r");
	if(victim == nullptr) {
		return;
	}

	if(victim == ch) {
		act("Molto spiritos$b oggi...", FALSE, ch, nullptr, nullptr, TO_CHAR);
		return;
	}


	if(MOUNTED(victim)) {
		send_to_char("Non puoi calciare a cavallo!\n\r", ch);
		return;
	}

	if(ch->attackers > 3) {
		send_to_char("Non c'e' abbastanza spazio!\n\r",ch);
		return;
	}

	if(victim->attackers >= 4) {
		send_to_char("Non riesci ad avvicinarti abbastanza per calciare!\n\r", ch);
		return;
	}

	percent=((10-(GET_AC(victim)/10))) + number(1,101);
	/* 101% is a complete failure */

	if(GET_RACE(victim)==RACE_GHOST) {
		kick_messages(ch,victim,0);
		SetVictFighting(ch,victim);
		return;
	}
	else if(!IS_NPC(victim) && (GetMaxLevel(victim) > MAX_MORT)) {
		kick_messages(ch,victim,0);
		SetVictFighting(ch,victim);
		SetCharFighting(ch,victim);
		return;
	}

	if(GET_POS(victim) <= POSITION_SLEEPING) {
		percent = 1;
	}

	if(percent > MIN(100, ch->skills[SKILL_KICK].learned)) {
		LearnFromMistake(ch, SKILL_KICK, 0, 90);
		if(GET_POS(victim) > POSITION_DEAD) {
			kick_messages(ch,victim,0);
			damage(ch, victim, 0, SKILL_KICK, location);
		}
	}
	else {
		if(!CheckMirror(victim)) {
			ActionAlignMod(ch,victim,cmd);
			if(GET_POS(victim) > POSITION_DEAD) {
				dam = GET_LEVEL(ch, BestFightingClass(ch));
				if(!HasClass(ch, CLASS_MONK) || IS_NPC(ch))
					/* so guards use fighter dam */
				{
					dam/=2;
				}
				if(OnlyClass(ch, CLASS_MONK)) {
					dam = (int)(dam * 1.3);
				}
				kick_messages(ch,victim,dam);
				if(damage(ch, victim, dam, SKILL_KICK, location) != VictimDead) {
					WAIT_STATE(victim, PULSE_VIOLENCE*1);
				}
			}
		}
	}
	if(HasClass(ch,CLASS_MONK)) {
		WAIT_STATE(ch, PULSE_VIOLENCE*1);
	}
	else {
		WAIT_STATE(ch, PULSE_VIOLENCE*2);
	}
}

/* Skill di parata, evita un numero di attacchi pari al proprio
   Se lo skill riesce gli attacchi vengono automaticamente
   indirizzati sullo scudo. Gaia( 7/2000 ) */

ACTION_FUNC(do_parry) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_parry (act.off.cpp)");
		return;
	}

	struct char_data* victim;

	if(off_at_peace(ch, "Non in questo luogo di pace.\n\r")) {
		return;
	}

	const std::string target = off_parse_first_token(arg);

	victim = off_visible_or_fighting(ch, target.c_str(),
	                                 "Se stai combattendo con qualcuno riesce meglio.\n\r");
	if(victim == nullptr) {
		return;
	}
	if(IS_SET(ch->specials.act,ACT_POLYSELF)) {
		send_to_char("In questa forma non sei in grado di parare i colpi.\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Impugni la tua arma e colpisci con forza lo scudo.\n\r", ch);
		return;
	}

	/* il PARRY resta attivo come sistema di combattimento
	   di default finche il comando non viene ridato o il
	   combattimento finisce */

	if(IS_SET(ch->specials.affected_by2, AFF2_PARRY)) {
		send_to_char("Smetti di proteggerti con lo scudo.\n\r",ch);
		REMOVE_BIT(ch->specials.affected_by2, AFF2_PARRY);
		return;
	}

	/* Fine dei controlli, si attiva il flag. Che lo skill riesca o meno
	   viene controllato nelle routine di combat (fight.c) volta per volta. */

	if(GET_POS(victim) > POSITION_DEAD)     {
		SET_BIT(ch->specials.affected_by2, AFF2_PARRY);
		act("$c1012$n cerca di proteggersi dai colpi usando lo scudo!", FALSE, ch, 0, victim, TO_ROOM);
		act("$c1012Cerchi di proteggerti con lo scudo.",FALSE,ch,0,0,TO_CHAR);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE);
}




ACTION_FUNC(do_wimp) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_wimp (act.off.cpp)");
		return;
	}

	/* sets the character in wimpy mode.  */
	char name[MAX_INPUT_LENGTH];
	short value;
	only_argument(arg, name);
	if(!*name) {
		snprintf(name, sizeof(name) - 1, "Soglia wimpy attuale: %d\n\r",
			ch->specials.WimpyLevel);
		send_to_char(name, ch);
	}
	else {
		value=MAX(atoi(name),0);
		ch->specials.WimpyLevel=value;
		if(!value) {
			REMOVE_BIT(ch->specials.act, PLR_WIMPY);
		}
		else {
			SET_BIT(ch->specials.act, PLR_WIMPY);
		}
		snprintf(name, sizeof(name) - 1, "Nuova soglia wimpy: %d\n\r",
			ch->specials.WimpyLevel);
		send_to_char(name, ch);
	}

}

ACTION_FUNC(do_shoot) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_shoot (act.off.cpp)");
		return;
	}

#if 0
	char tmp[MAX_INPUT_LENGTH],dirstr[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH];
	char buf[255];
	struct char_data* victim;
	struct room_data* this_room,*to_room,*next_room;
	struct room_direction_data* exitp;
	struct char_data* mob;
	struct obj_data* weapon;
	int i,dir,room_num=0,room_count, MAX_DISTANCE_SHOOT;

	if(check_peaceful(ch,"Non in questo luogo di pace.\n\r")) {
		return;
	}

	arg =  one_argument(arg, tmp);

	mudlog(LOG_CHECK, "begin do_shoot");

	if(*tmp) {
		victim = off_visible_in_room(ch, tmp);

		if(!victim) {
			i = ch->in_room;
			room_count=1;
			weapon = ch->equipment[HOLD];

			if(!weapon) {
				send_to_char("You do not hold a missile weapon?!?!\n\r",ch);
				return;
			}

# if 0
			MAX_DISTANCE_SHOOT = weapon->obj_flags.value[1];
# else
			MAX_DISTANCE_SHOOT = 1;
# endif



			switch(*tmp) {
			case 'N':
			case 'n':
				dir=0;
				break;
			case 'S':
			case 's':
				dir=2;
				break;
			case 'E':
			case 'e':
				dir=1;
				break;
			case 'W':
			case 'w':
				dir=3;
				break;
			case 'd':
			case 'D':
				dir=4;
				break;
			case 'u':
			case 'U':
				dir=5;
				break;

			default:
				send_to_char("What direction did you wish to fire?\n\r",ch);
				return;
				break;
			} /* end switch */

			arg= one_argument(arg,name);
			if(strn_cmp(name,"at",2) && isspace(name[2])) {
				arg=one_argument(arg,name);
			}


			if(!exit_ok(EXIT_NUM(i,dir),NULL)) {
				send_to_char("You can't shoot in that direction.\n\r",ch);
				return;
			}

			while(room_count<=MAX_DISTANCE_SHOOT && !victim &&
					exit_ok(EXIT_NUM(i,dir),NULL)) {

				this_room = real_roomp(i);
				to_room   = real_roomp(this_room->dir_option[dir]->to_room);
				room_num  = this_room->dir_option[dir]->to_room;

				mob = get_char_near_room_vis(ch,name,room_num);
				if(mob) {
					snprintf(buf,254,"You spot your quarry %s.\n",listexits[dir]);
					act(buf,FALSE,ch,0,0,TO_CHAR);
					victim=mob;
				}

				i = room_num;
				room_count++;

			} /* end while */
		} /* !victim */

		if(victim) {
			if(victim == ch) {
				send_to_char("You can't shoot things at yourself!", ch);
				return;
			}
			else {
				if(IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
					act("$N is just such a good friend, you simply can't shoot at $M.",
						FALSE, ch,0,victim,TO_CHAR);
					return;
				}
				if(ch->specials.fighting) {
					send_to_char("You're at too close range to fire a weapon!\n\r", ch);
					return;
				}

				if(check_peaceful(victim,"")) {
					send_to_char("That is a peaceful room\n\r",ch);
					return;
				}
				ActionAlignMod(ch,victim,cmd);
				shoot(ch, victim);
				WAIT_STATE(ch, PULSE_VIOLENCE); // shoot
			}
		}
		else {
			send_to_char("They aren't here.\n\r", ch);
		}
	}
	else {
		send_to_char("Shoot who?\n\r", ch);
	}
	mudlog(LOG_CHECK, "end do_shoot, act.off.c");

#endif
}


ACTION_FUNC(do_springleap) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_springleap (act.off.cpp)");
		return;
	}

	struct char_data* victim;
	char name[256];
	byte percent;
	int danno=0;
	int location = 5 ;
	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch, "C'e' troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch, CLASS_MONK)) {
			send_to_char("Non sei un monaco!\n\r", ch);
			return;
		}

	only_argument(arg, name);

	if(!(victim = off_visible_in_room(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Spring-leap a chi?\n\r", ch);
			return;
		}
	}

	if(GET_POS(ch) > POSITION_SITTING || !ch->specials.fighting) {
		send_to_char("Non sei nella giusta posizione per farlo!\n\r", ch);
		return;
	}

	if(victim == ch) {
		send_to_char("Non mi va di scherzare oggi...\n\r", ch);
		return;
	}

	if(ch->attackers > 3) {
		send_to_char("Non c'e' abbastanza spazio!\n\r",ch);
		return;
	}

	if(victim->attackers >= 3) {
		send_to_char("Non riesci ad avvicinarti abbastanza.\n\r", ch);
		return;
	}

	percent=number(1,101);

	act("$n fa un'abile mossa allungando una gamba verso $N.", FALSE,
		ch, 0, victim, TO_ROOM);
	act("Sali a gamba tesa verso $N.", FALSE, ch, 0, victim, TO_CHAR);
	act("$n salta a gamba tesa verso te.", FALSE, ch, 0, victim, TO_VICT);


	if(percent > ch->skills[SKILL_SPRING_LEAP].learned) {
		if(GET_POS(victim) > POSITION_DEAD) {
			damage(ch, victim, 0, SKILL_KICK, location);
			LearnFromMistake(ch, SKILL_SPRING_LEAP, 0, 90);
			send_to_char("Rovini a terra rumorosamente.\n\r", ch);
			act("$n rovina a terra rumorosamente", FALSE, ch, 0, 0, TO_ROOM);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*2); // spring
		return;

	}
	else {
		ActionAlignMod(ch,victim,cmd);
		danno=GET_LEVEL(ch,BestFightingClass(ch));
		danno*=2;
		if(OnlyClass(ch,CLASS_MONK)) {
			danno = (int)(danno * 1.3);
		}
		danno+=(danno*(20-number(0,40))/100);
		kick_messages(ch, victim,  danno);
		if(GET_POS(victim) > POSITION_DEAD) {

			if(damage(ch, victim, danno, SKILL_KICK, location) != VictimDead) {

				WAIT_STATE(victim, PULSE_VIOLENCE);
			}
		}

	}
	WAIT_STATE(ch, PULSE_VIOLENCE*1);
	GET_POS(ch)=POSITION_STANDING;
	update_pos(ch);
	return;
}


ACTION_FUNC(do_quivering_palm) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_quivering_palm (act.off.cpp)");
		return;
	}

	struct char_data* victim;
	struct affected_type af;
	byte percent;
	char name[256];
	int location = 12 ;
	if(!ch->skills) {
		return;
	}

	if(check_peaceful(ch, "C'e' troppa pace qui per essere violenti.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch, CLASS_MONK)) {
			send_to_char("Non sei un monaco!\n\r", ch);
			return;
		}

	only_argument(arg, name);

	if(!(victim = off_visible_in_room(ch, name))) {
		if(ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Su chi vuoi usare il favoloso palmo vibrante?\n\r", ch);
			return;
		}
	}

	if(ch->attackers > 3) {
		send_to_char("Non hai abbastanza spazio!\n\r",ch);
		return;
	}

	if(victim->attackers >= 3) {
		send_to_char("Non riesci ad avvicinarti abbastanza.\n\r", ch);
		return;
	}

	if(!IsHumanoid(victim)) {
		send_to_char("Rispondono alle vibrazioni solo gli umanoidi.\n\r", ch);
		return;
	}

	send_to_char("Cominci a generare una vibrazione con il palmo della tua "
				 "mano.\n\r", ch);
	act("$n comincia a generare una vibrazione con il palmo della mano rivolto verso di te.",
		TRUE,ch,NULL,victim,TO_VICT);

	if(affected_by_spell(ch, SKILL_QUIV_PALM)) {
		send_to_char("Puoi farlo solo una volta alla settimana.\n\r", ch);
		return;
	}

	percent=number(1,101);

	if(percent > ch->skills[SKILL_QUIV_PALM].learned) {
		send_to_char("La vibrazione si spegne inefficace.\n\r", ch);
		if(GET_POS(victim) > POSITION_DEAD) {
			LearnFromMistake(ch, SKILL_QUIV_PALM, 0, 95);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE*3); // quivering
		return;

	}
	else {
		if(GET_MAX_HIT(victim) > GET_MAX_HIT(ch) * 2 ||
				GetMaxLevel(victim) > GetMaxLevel(ch)) {
			damage(ch, victim, 0, SKILL_QUIV_PALM, location);
			return;
		}
		if(saves_spell(victim, SAVING_SPELL)) {
			send_to_char("La tua vittima resiste al palmo vibrante.\n\r", ch);
			act("Resisti al palmo vibrante di $n.",
				TRUE,ch,NULL,victim,TO_VICT);
			WAIT_STATE(ch, PULSE_VIOLENCE * 2);
			return;
		}
		if(HitOrMiss(ch, victim, CalcThaco(ch, victim))) {
			ActionAlignMod(ch,victim,cmd);
			if(GET_POS(victim) > POSITION_DEAD) {
				damage(ch, victim, GET_MAX_HIT(victim)*20,SKILL_QUIV_PALM, location);
			}

            if(HasClass(ch, CLASS_MONK) && IS_PC(ch))
            {
                if(IS_POLY(ch))
                {
                    ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_MONK_2] += 1;
                    if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                    {
                        SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                    }
                }
                else
                {
                    ch->specials.achievements[CLASS_ACHIE][ACHIE_MONK_2] += 1;
                    if(!IS_SET(ch->specials.act,PLR_ACHIE))
                    {
                        SET_BIT(ch->specials.act, PLR_ACHIE);
                    }
                }

                CheckAchie(ch, ACHIE_MONK_2, CLASS_ACHIE);
            }
		}
	}
	WAIT_STATE(ch, PULSE_VIOLENCE * 1);   // quivering

	af.type = SKILL_QUIV_PALM;
	af.duration = 168;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(ch, &af);
}


void kick_messages(struct char_data* ch, struct char_data* victim, int damage) {
	if(ch == nullptr || victim == nullptr) {
		return;
	}

	const int msgIdx = off_kick_race_message_index(victim);
	const int fighterClass = off_kick_fighter_class(ch);
	off_kick_adjust_damage(victim, fighterClass, damage);

	if(!damage) {
		off_kick_emit_set(ch, victim, att_kick_miss_ch[msgIdx],
		                  att_kick_miss_victim[msgIdx], att_kick_miss_room[msgIdx], damage);
	}
	else if(GET_HIT(victim) - DamageTrivia(ch, victim, damage, SKILL_KICK, 7) < -10) {
		off_kick_emit_set(ch, victim, att_kick_kill_ch[msgIdx],
		                  att_kick_kill_victim[msgIdx], att_kick_kill_room[msgIdx], damage);
	}
	else {
		off_kick_emit_set(ch, victim, att_kick_hit_ch[msgIdx],
		                  att_kick_hit_victim[msgIdx], att_kick_hit_room[msgIdx], damage);
	}
}

ACTION_FUNC(do_berserk) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_berserk (act.off.cpp)");
		return;
	}

	int skillcheck=0;
	struct char_data* victim;

	if((!ch->skills) && IS_PC(ch)) {
		send_to_char("Non conosci nessuna abilita'!\n\r", ch);
		return;
	}

	if(off_at_peace(ch, "Non in questo luogo di pace.\n\r")) {
		return;
	}

	if(IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
		if(!HasClass(ch, CLASS_BARBARIAN) && cmd !=0)  {
			send_to_char("Non hai il sangue della furia berserker!\n\r", ch);
			return;
		}

#if 0
	if(!IS_PC(ch) && cmd && !IS_SET(ch->specials.act,ACT_POLYSELF)) {
		return;
	}
#endif

	const std::string target = off_parse_first_token(arg);

	if(!(victim = off_visible_in_room(ch, target.c_str())) || !(ch->specials.fighting)) {
		if(ch->specials.fighting)     {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Devi essere in combattimento per andare in berserk.\n\r", ch);
			return;
		}
	}

	if(victim == ch) {
		act("Molto spiritos$b oggi...", FALSE, ch, nullptr, nullptr, TO_CHAR);
		return;
	}


	if(MOUNTED(victim)) {
		send_to_char("Non puoi andare in berserk a cavallo!\n\r", ch);
		return;
	}

	if(IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
		send_to_char("Sei gia' in berserk!\n\r", ch);
		return;
	}

	if(GET_MANA(ch)<=15)    {  // SALVO non si entra in berserk con poco mana
		send_to_char("Non hai energia sufficiente per andare in berserk!\n\r", ch);
		return;
	}
	/* all the checks passed, now get on with it! */

	skillcheck = number(0,101);


	if(IS_PC(ch) && (skillcheck > ch->skills[SKILL_BERSERK].learned))  {
		act("$c1012$n ringhia, sembr$b furios$b!", FALSE, ch, 0, victim, TO_ROOM);
		act("$c1012Non riesci a infuriarti abbastanza.", FALSE, ch, 0, 0, TO_CHAR);
		LearnFromMistake(ch, SKILL_BERSERK, 0, 90);
	}
	else   {
		if(GET_POS(victim) > POSITION_DEAD)     {
			GET_MANA(ch) -=15;
			alter_mana(ch,0);  /* cost 15 mana to do it.. */
			SET_BIT(ch->specials.affected_by2,AFF2_BERSERK);
			act("$c1012$n ringhia e si lancia in una furia omicida!", FALSE, ch, 0, victim,
				TO_ROOM);
			act("$c1012La frenesia ti travolge all'improvviso!", FALSE, ch, 0, 0, TO_CHAR);

            if(HasClass(ch, CLASS_BARBARIAN) && IS_PC(ch))
            {
                if(IS_POLY(ch))
                {
                    ch->desc->original->specials.achievements[CLASS_ACHIE][ACHIE_BARBARIAN_1] += 1;
                    if(!IS_SET(ch->desc->original->specials.act,PLR_ACHIE))
                    {
                        SET_BIT(ch->desc->original->specials.act, PLR_ACHIE);
                    }
                }
                else
                {
                    ch->specials.achievements[CLASS_ACHIE][ACHIE_BARBARIAN_1] += 1;
                    if(!IS_SET(ch->specials.act,PLR_ACHIE))
                    {
                        SET_BIT(ch->specials.act, PLR_ACHIE);
                    }
                }

                CheckAchie(ch, ACHIE_BARBARIAN_1, CLASS_ACHIE);
            }
		}
		WAIT_STATE(victim, PULSE_VIOLENCE); // berserk
	}

	WAIT_STATE(ch, PULSE_VIOLENCE*1); // Messo il tempo ad 1 per compensare GAIA2002
	//WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

void throw_weapon(struct obj_data* o, int dir, struct char_data* targ,
				  struct char_data* ch) {
	int w = o->obj_flags.weight, sz, max_range, range, there;
	int rm = ch->in_room, opdir[] = { 2, 3, 0, 1, 5, 4 };
	int broken=FALSE;
	char buf[MAX_STRING_LENGTH];
	struct char_data* spud, *next_spud;
	const char* dir_name[] = {
		"da nord",
		"da est",
		"da sud",
		"da ovest",
		"dall'alto",
		"dal basso"
	};

	if(w > 100) {
		sz = 3;
	}
	else if(w > 25) {
		sz = 2;
	}
	else if(w > 5) {
		sz = 1;
	}
	else {
		sz = 0;
	}
	max_range = (((GET_STR(ch) + GET_ADD(ch) / 30) - 3) / 8) + 2;
	max_range = max_range / (sz + 1);
	if(o->obj_flags.type_flag == ITEM_MISSILE &&
			ch->equipment[ WIELD ] &&
			ch->equipment[ WIELD ]->obj_flags.type_flag == ITEM_FIREWEAPON) {
		/* Add bow's range bonus */
		max_range += ch->equipment[ WIELD ]->obj_flags.value[ 2 ];
	}
	if(max_range == 0) {
		act("$p colpisce il terreno davanti a $n.", TRUE, ch, o, 0, TO_ROOM);
		act("$p cade fiaccamente in terra davanti a te.", TRUE, ch, o, 0,
			TO_CHAR);
		obj_to_room(o, ch->in_room);
		return;
	}
	range = 0;
	while(range < max_range && broken == FALSE) {
		/* Check for target */
		there = 0;
		for(spud = real_roomp(rm)->people; spud; spud = next_spud) {
			next_spud = spud->next_in_room;
			if(spud == targ) {
				there = 1;
				if(range_hit(ch, targ, range, o, dir, max_range)) {
					if(targ && GET_POS(targ) > POSITION_DEAD) {
						if(o->obj_flags.type_flag == ITEM_MISSILE &&
								number(1, 100) < o->obj_flags.value[ 0 ]) {
							act("$p finisce in pezzi.", TRUE, targ, o, 0, TO_ROOM);
							broken = TRUE;
							obj_to_room(o, 3);   /* storage for broken arrows */
							/* for some reason this causes the obj to get placed in very
							   weird places */
#if 0
							obj_from_room(o);
							extract_obj(o);
#endif
						}
						else {
							obj_to_room(o, rm);
						}
					}
					mudlog(LOG_CHECK, "Throw weapon has hit!");
					return;
				}
				break;
			}
		}
		if(dir >= 0) {
			if(broken == FALSE) {
				if(clearpath(ch, rm, dir)) {
					if(!there && rm != ch->in_room) {
						snprintf(buf, MAX_STRING_LENGTH-1,"%s passa veloce %s!\n\r", o->short_description,
								 dir_name[ opdir[ dir ] ]);
						send_to_room(buf, rm);
					}
					else {
						there = 0;
					}
					rm = real_roomp(rm)->dir_option[ dir ]->to_room;
				}
				else {
					if(range > 1 && dir >= 0) {
						snprintf(buf,MAX_STRING_LENGTH-1,"%s vola %s, colpisce una parete ",
								 o->short_description, dir_name[ opdir[ dir ] ]);
					}
					else {
						snprintf(buf,MAX_STRING_LENGTH-1,"%s colpisce una parete ",o->short_description);
					}
					send_to_room(buf, rm);
					if(o->obj_flags.type_flag == ITEM_MISSILE &&
							number(1, 100) < o->obj_flags.value[ 0 ]) {
						snprintf(buf,MAX_STRING_LENGTH-1, "e finisce in pezzi.\n\r");
						obj_to_room(o, 3);   /* storage for broken arrows */
						broken=TRUE;
					}
					else {
						snprintf(buf, MAX_STRING_LENGTH-1,"e cade in terra.\n\r");
						obj_to_room(o, rm);
					}
					send_to_room(buf, rm);
					return;
				}
			}
		}
		else {
			break;
		}
		range++;
	}
	if(broken == FALSE) {
		snprintf(buf,MAX_STRING_LENGTH-1,"%s cade in terra.\n\r", o->short_description);
		send_to_room(buf, rm);
		obj_to_room(o, rm);
	}
}

void throw_object(struct obj_data* o, int dir, int from) {
#if 0
	struct char_data* catcher;
#endif
	const char* directions[][2]= {
		{ "nord",     "da sud"    },
		{ "est",      "da ovest"  },
		{ "sud",      "da nord"   },
		{ "ovest",    "da est"    },
		{ "l'alto",   "dal basso" },
		{ "il basso", "dall'alto" }
	};

	char buf1[100];
	int distance=0;

	while(distance < 20 && real_roomp(from)->dir_option[ dir ] &&
			real_roomp(from)->dir_option[ dir ]->exit_info < 2 &&
			real_roomp(from)->dir_option[ dir ]->to_room > 0) {
		if(distance) {
			snprintf(buf1,99, "%s vola %s.\n\r",
					 o->short_description,directions[ dir ][ 1 ]);
			send_to_room(buf1,from);

#if 0
			for(catcher=real_roomp(from)->people; catcher; catcher=catcher->next_in_room) {
				if(!strcmp(catcher->catch,o->name)) {
					switch(number(1,3)) {
					case 1:
						act("$n dives and catches $p",FALSE,catcher,o,0,TO_ROOM);
						break;
					default:
						act("$n catches $p",FALSE,catcher,o,0,TO_ROOM);
					}
					send_to_char("You caught it!\n\r",catcher);
					obj_from_room(o);
					obj_to_char(o,catcher);
					return;
				}
			}
#endif

			snprintf(buf1, 99,"%s vola verso %s.\n\r",
					 o->short_description,directions[ dir ][ 0 ]);
			send_to_room(buf1, from);
		}
		distance++;
		obj_from_room(o);
		from = real_roomp(from)->dir_option[dir]->to_room;
		obj_to_room(o,from);
	}
	if(distance == 20) {
		snprintf(buf1,99, "%s vola %s ed atterra qui.\n\r",
				 o->short_description, directions[ dir ][ 1 ]);
		send_to_room(buf1, from);
		return;
	}
	snprintf(buf1, 99,"%s vola %s e colpisce una parete.\n\r",
			 o->short_description,directions[dir][1]);
	send_to_room(buf1,from);
}



int clearpath(struct char_data* ch, long room, int direc) {
	static const int opdir[] = {2, 3, 0, 1, 5, 4};
	struct room_direction_data* exitdata;

	if(ch == nullptr || ch->nMagicNumber != CHAR_VALID_MAGIC) {
		return 0;
	}
	exitdata = real_roomp(room)->dir_option[ direc ];

	if(exitdata &&
			!real_roomp(exitdata->to_room)) {
		return 0;
	}
	if(!CAN_GO(ch, direc)) {
		return 0;
	}
	if(!real_roomp(room)->dir_option[ direc ]) {
		return 0;
	}
	if(real_roomp(room)->dir_option[ direc ]->to_room < 1) {
		return 0;
	}
#if 0
	if(real_roomp(room)->zone !=
			real_roomp(real_roomp(room)->dir_option[ direc ]->to_room)->zone) {
		return 0;
	}
#endif
	if(IS_SET(real_roomp(room)->dir_option[ direc ]->exit_info, EX_CLOSED)) {
		return 0;
	}
	if(!IS_SET(real_roomp(room)->dir_option[ direc ]->exit_info, EX_ISDOOR)
			&& real_roomp(room)->dir_option[ direc ]->exit_info > 0) {
		return 0;
	}
	/* One-way windows are allowed... no see through 1-way exits */
	if(!real_roomp(real_roomp(room)->dir_option[ direc ]->to_room)->dir_option[ opdir[ direc ] ]) {
		return 0;
	}

	if(real_roomp(real_roomp(room)->dir_option[ direc ]->to_room)->dir_option[ opdir[ direc ] ]->to_room < 1) {
		return 0;
	}

	if(real_roomp(real_roomp(room)->dir_option[ direc ]->to_room)->dir_option[ opdir[ direc ] ]->to_room != room) {
		return 0;
	}


	return real_roomp(room)->dir_option[ direc ]->to_room;
}

ACTION_FUNC(do_weapon_load) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_weapon_load (act.off.cpp)");
		return;
	}

	struct obj_data* fw, *ms;

	fw = ch->equipment[WIELD];
	if(!fw || fw->obj_flags.type_flag != ITEM_FIREWEAPON) {
		send_to_char("Devi impugnare un arma per lanciare proiettili.\n\r", ch);
		return;
	}
    if((GET_STR(ch) + GET_ADD(ch) / 3) < fw->obj_flags.value[ 0 ] && !IS_NPC(ch) ) // Montero 10-Sep-2018 act.off.cpp: se è un mob non considero la forza per il load missile
    {
		mudlog(LOG_CHECK, "(%s) can't load (%s) because it requires (%d) strength "
			   "to wield",
			   GET_NAME(ch), fw->name, fw->obj_flags.value[ 0 ]);
		send_to_char("Non sei abbastanza forte per usare un arma cosi' "
					 "potente.\n\r", ch);
		return;
	}
	if(ch->equipment[ LOADED_WEAPON ]) {
		if(CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
			ms = unequip_char(ch, LOADED_WEAPON);
			act("Prima togli $p.", TRUE, ch, ms, 0, TO_CHAR);
			obj_to_char(ms, ch);
			act("$n scarica $p.", FALSE, ch, ms, 0, TO_ROOM);
		}
		else {
			send_to_char("Hai le mani troppo piene per scaricare l'arma.\n\r", ch);
			return;
		}
	}

	const auto [missileName, ammoExtra] = chop_argument(arg, MAX_STRING_LENGTH - 1,
	                                                    MAX_STRING_LENGTH - 1);
	(void)ammoExtra;
	if(missileName.empty()) {
		send_to_char("Che proiettile vuoi caricare ?\n\r",ch);
		return;
	}
	ms = get_obj_in_list_vis(ch, missileName.c_str(), ch->carrying);
	if(!ms) {
		send_to_char("Non hai niente del genere.\n\r", ch);
		return;
	}
	if(ms->obj_flags.type_flag != ITEM_MISSILE) {
		act("Non puoi lanciare $p.",TRUE,ch,ms,0,TO_CHAR);
		return;
	}
	if(ms->obj_flags.value[ 3 ] != fw->obj_flags.value[ 3 ]) {
		act("Non puoi usare $p con l'arma che stai impugnando.", TRUE, ch, ms, 0,
			TO_CHAR);
		return;
	}

	obj_from_char(ms);
	equip_char(ch, ms, LOADED_WEAPON);
	act("Carichi $p.", TRUE, ch, ms, 0, TO_CHAR);
	act("$n carica $p.", FALSE, ch, ms, 0, TO_ROOM);
	WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACTION_FUNC(do_fire) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_fire (act.off.cpp)");
		return;
	}

	struct obj_data* fw, *ms;
	struct char_data* targ;
	int tdir, rng, dr;

	fw = ch->equipment[WIELD];
	if(!fw || fw->obj_flags.type_flag != ITEM_FIREWEAPON) {
		send_to_char("Devi impugnare un'arma per lanciare proiettili!\n\r", ch);
		return;
	}

	const std::string target = off_parse_first_token(arg);
	if(target.empty()) {
		send_to_char("Il giusto formato per fire (o shoot) e': "
					 "fire [<dir> at] <target>\n\r",ch);
		return;
	}
	targ = get_char_linear(ch, target.c_str(), &rng, &dr);
	if(targ && targ == ch) {
		send_to_char("Non puoi colpire te stesso!\n\r", ch);
		return;
	}

	if(dr == -1 && !targ) {
		send_to_char("Quella non e' ne' una direzione, ne' la descrizione di una "
					 "creatura.\n\r", ch);
		return;
	}

	if(!targ) {
		send_to_char("Non vedi nessuno con quella descrizione.\n\r", ch);
		return;
	}
	else {
		tdir = dr;
	}
	if(check_peaceful(targ,
					  "Qualcuno ha cercato di disturbare la tua pace.")) {
		send_to_char("Mi dispiace ma c'e' troppa pace li' per lanciarci "
					 "qualcosa.", ch);
		return;
	}

	if(ch->equipment[ LOADED_WEAPON ]) {
		ms = unequip_char(ch, LOADED_WEAPON);
	}
	else {
		act("$p non e' caricata!", TRUE, ch, fw, 0, TO_CHAR);
		return;
	}

	act("Lanci $p verso $N.", TRUE, ch, ms, targ, TO_CHAR);
	act("$n lancia $p!", TRUE, ch, ms, 0, TO_ROOM);
	throw_weapon(ms, tdir, targ, ch);
}



ACTION_FUNC(do_throw) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_throw (act.off.cpp)");
		return;
	}

	struct obj_data* pObjThrow;
	int rng, tdir;
	struct char_data* targ;

	const auto [objName, throwTarget] = chop_argument(arg, 99, 99);
	if(objName.empty() || throwTarget.empty()) {
		send_to_char("Il giusto formato per throw e': "
					 "throw <oggetto> [<dir> at] <target>.\n\r", ch);
		return;
	}

	if(ch->equipment[WIELD] && ch->equipment[WIELD]->obj_flags.weight >
			str_app[ STRENGTH_APPLY_INDEX(ch) ].wield_w) {
		send_to_char("Non puoi impugnare un'arma a due mani e lanciare "
					 "qualcosa.\n\r", ch);
	}
	else if(ch->equipment[ WIELD ] && ch->equipment[ HOLD ]) {
		send_to_char("Hai solo due mani.\n\r", ch);
	}
	else if(real_roomp(ch->in_room)->sector_type == SECT_UNDERWATER) {
		send_to_char("Non puoi lanciare nulla sott'acqua.\n\r", ch);
	}
	else if((pObjThrow = get_obj_in_list_vis(ch, objName.c_str(), ch->carrying))) {
		/* Check if second argument is a character or direction */
		targ = get_char_linear(ch, throwTarget.c_str(), &rng, &tdir);
		if(targ && targ == ch) {
			act("Non puoi lanciare $p verso te stess$b!", FALSE, ch, pObjThrow,
				NULL, TO_CHAR);
			return;
		}

		if(targ) {
			if(IS_SET(pObjThrow->obj_flags.wear_flags, ITEM_THROW)) {
				if(pObjThrow->obj_flags.type_flag != ITEM_WEAPON &&
						pObjThrow->obj_flags.type_flag != ITEM_MISSILE) {  // Gaia 2001
					/* Friendly throw */
					act("Lanci $p.", FALSE, ch, pObjThrow, NULL, TO_CHAR);
					obj_from_char(pObjThrow);
					act("$n lancia $p!", TRUE, ch, pObjThrow, 0, TO_ROOM);
					obj_to_room(pObjThrow, ch->in_room);
					throw_object(pObjThrow, tdir, ch->in_room);
				}
				else {
					if(check_peaceful(targ,
									  "Qualcuno ha cercato di disturbare la tua "
									  "pace.")) {
						send_to_char("Mi dispiace ma c'e' troppa pace li' per lanciarci "
									 "qualcosa.", ch);
						return;
					}
					act("Lanci $p verso $N.", FALSE, ch, pObjThrow, targ, TO_CHAR);
					obj_from_char(pObjThrow);
					act("$n lancia $p!", TRUE, ch, pObjThrow, NULL, TO_ROOM);
					throw_weapon(pObjThrow, tdir, targ, ch);
				}
			}
			else {
				act("Non puoi lanciare $p.", FALSE, ch, pObjThrow, NULL, TO_CHAR);
			}
		}
		else {
			send_to_char("Non c'e' nessuno qui intorno con quella descrizione.\n\r",
						 ch);
		}
	}
	else {
		send_to_char("Non hai niente del genere!\n\r",ch);
	}
}

ACTION_FUNC(do_stopfight) {
	if(ch == nullptr) {
		mudlog(LOG_SYSERR, "ch==nullptr in do_stopfight (act.off.cpp)");
		return;
	}

	if(ch->specials.fighting) {
		if(!IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
			stop_fighting(ch);
			WAIT_STATE(ch, PULSE_VIOLENCE);
			act("Hai smesso di combattere.", TRUE, ch, 0, 0, TO_CHAR);
			act("$n smette di combattere.", TRUE, ch, 0, 0, TO_ROOM);
		}
		else
			send_to_char("Tutto quello a cui pensi adesso, e' alla tua "
						 "battaglia\n\r", ch);
	}
	else {
		send_to_char("Ma se non stai combattendo!\n\r", ch);
	}

}
} // namespace Alarmud
