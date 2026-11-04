/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
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
#include "phase_boss.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "fight.hpp"
#include "handler.hpp"
#include "spec_mount_olymp.hpp"
#include "spell_parser.hpp"
#include "spells.hpp"
#include "utility.hpp"

namespace Alarmud {
namespace {

/*--------------------------------------------------------------------------
 * PhaseBoss — engine (fasi + telegraph / interrupt).
 * Dati / profili: phase_boss_config.{hpp,cpp}
 * Special: M <vnum> PhaseBoss [id]  (profilo da token .spe o da vnum).
 *--------------------------------------------------------------------------*/

enum class Phase : std::uint8_t { Idle = 0, Rage = 1, Wrath = 2 };

enum class Resolve : std::uint8_t { Full = 0, Soft, Hard };

using Interrupt = PhaseBossInterrupt;
using AbilityDef = PhaseBossAbility;
using PhaseDef = PhaseBossPhase;
using BossProfile = PhaseBossProfile;

[[nodiscard]] constexpr int dmg_attack_type(PhaseBossDmg k) noexcept {
	switch(k) {
	case PhaseBossDmg::Bludgeon:
		return TYPE_BLUDGEON;
	case PhaseBossDmg::Lightning:
	default:
		return SPELL_LIGHTNING_BOLT;
	}
}

[[nodiscard]] std::string_view first_spe_token(const char* parms) {
	if(!parms) {
		return {};
	}
	while(*parms && std::isspace(static_cast<unsigned char>(*parms))) {
		++parms;
	}
	if(!*parms) {
		return {};
	}
	const char* start = parms;
	while(*parms && !std::isspace(static_cast<unsigned char>(*parms))) {
		++parms;
	}
	return std::string_view(start, static_cast<std::size_t>(parms - start));
}

[[nodiscard]] const BossProfile* resolve_profile(char_data* mob) {
	const auto token = first_spe_token(GET_SPEC_PARM(mob));
	if(!token.empty()) {
		if(const BossProfile* by_name = phase_boss_find_by_id(token)) {
			return by_name;
		}
		mudlog(LOG_ERROR, "PhaseBoss: profilo '%.*s' sconosciuto (mob vnum %d)",
			   static_cast<int>(token.size()), token.data(),
			   (mob->nr >= 0) ? mob_index[mob->nr].iVNum : -1);
		return nullptr;
	}
	if(mob->nr >= 0) {
		return phase_boss_find_by_vnum(mob_index[mob->nr].iVNum);
	}
	return nullptr;
}

struct BossState {
	const BossProfile* profile = nullptr;
	Phase phase = Phase::Idle;
	int ability_index = -1;
	int windup_left = 0;
	int cast_hp = 0;
	Interrupt interrupt_mask = Interrupt::None;
	Resolve resolve = Resolve::Full;
	int skill_cd = 0;
	int fight_ticks = 0;
	int add_count = 0;

	[[nodiscard]] bool casting() const noexcept {
		return ability_index >= 0 && windup_left > 0;
	}

	[[nodiscard]] bool busy() const noexcept {
		return ability_index >= 0;
	}

	void clear_cast() noexcept {
		ability_index = -1;
		windup_left = 0;
		cast_hp = 0;
		interrupt_mask = Interrupt::None;
		resolve = Resolve::Full;
	}

	void reset() noexcept {
		const BossProfile* keep = profile;
		*this = BossState{};
		profile = keep;
	}

	void begin_cast(int idx, const AbilityDef& ab) noexcept {
		ability_index = idx;
		windup_left = ab.windup;
		cast_hp = ab.cast_hp;
		interrupt_mask = ab.mask;
		resolve = Resolve::Full;
	}
};

BossState& state_of(char_data* mob) {
	if(!mob->act_ptr) {
		mob->act_ptr = new BossState{};
	}
	return *static_cast<BossState*>(mob->act_ptr);
}

void destroy_state(char_data* mob) noexcept {
	delete static_cast<BossState*>(mob->act_ptr);
	mob->act_ptr = nullptr;
}

[[nodiscard]] int hp_pct(char_data* mob) noexcept {
	const int mx = GET_MAX_HIT(mob);
	if(mx <= 0) {
		return 100;
	}
	return (100 * GET_HIT(mob)) / mx;
}

[[nodiscard]] Phase phase_for_hp(const BossProfile& profile, int pct) noexcept {
	if(pct <= profile.phases[2].enter_hp) {
		return Phase::Wrath;
	}
	if(pct <= profile.phases[1].enter_hp) {
		return Phase::Rage;
	}
	return Phase::Idle;
}

[[nodiscard]] bool is_fight_pc(char_data* t, char_data* mob) noexcept {
	return t
		&& t != mob
		&& !IS_NPC(t)
		&& !IS_IMMORTAL(t)
		&& t->in_room == mob->in_room
		&& GET_POS(t) > POSITION_DEAD;
}

[[nodiscard]] std::vector<char_data*> pcs_in_room(char_data* mob) {
	std::vector<char_data*> out;
	auto* room = real_roomp(mob->in_room);
	if(!room) {
		return out;
	}
	for(auto* t = room->people; t; t = t->next_in_room) {
		if(is_fight_pc(t, mob)) {
			out.push_back(t);
		}
	}
	return out;
}

void extract_adds(char_data* mob, const BossProfile& profile) {
	const int want = profile.phases[2].add_vnum;
	if(want <= 0) {
		return;
	}
	for(char_data* t = character_list; t;) {
		char_data* next = t->next;
		if(IS_NPC(t) && t->master == mob && t->nr >= 0
		   && mob_index[t->nr].iVNum == want) {
			extract_char(t);
		}
		t = next;
	}
}

void aoe_damage(char_data* mob, int dam, int atype) {
	for(char_data* vict : pcs_in_room(mob)) {
		damage(mob, vict, dam, atype, 5);
	}
}

void act_room(char_data* mob, const char* msg) {
	if(msg && msg[0]) {
		act(msg, FALSE, mob, nullptr, nullptr, TO_ROOM);
	}
}

void summon_adds(char_data* mob, BossState& st, const PhaseDef& phase) {
	if(phase.add_vnum <= 0 || phase.add_max <= 0) {
		return;
	}
	bool any_spawned = false;
	for(; st.add_count < phase.add_max; ++st.add_count) {
		const int rnum = real_mobile(phase.add_vnum);
		if(rnum < 0) {
			act("$n invoca i suoi campioni... ma il cielo resta vuoto.",
				FALSE, mob, nullptr, nullptr, TO_ROOM);
			return;
		}
		char_data* add = read_mobile(rnum, REAL);
		if(!add) {
			return;
		}
		char_to_room(add, mob->in_room);
		add_follower(add, mob);
		SET_BIT(add->specials.affected_by, AFF_CHARM);
		if(mob->specials.fighting) {
			hit(add, mob->specials.fighting, TYPE_UNDEFINED);
		}
		any_spawned = true;
	}
	if(any_spawned) {
		act_room(mob, phase.add_emote);
	}
}

void on_enter_phase(char_data* mob, BossState& st, Phase phase) {
	const BossProfile& profile = *st.profile;
	const auto& def = profile.phases[static_cast<std::size_t>(phase)];

	mudlog(LOG_CHECK, "PhaseBoss[%s] phase -> %d (hp%%=%d) room=%d",
		   std::string(profile.id), static_cast<int>(phase), hp_pct(mob),
		   mob->in_room);

	act_room(mob, def.enter_emote);
	act_room(mob, def.enter_emote2);
	if(phase == Phase::Wrath) {
		summon_adds(mob, st, def);
	}
	st.skill_cd = 0;
}

void update_phase(char_data* mob, BossState& st) {
	const Phase wanted = phase_for_hp(*st.profile, hp_pct(mob));
	if(static_cast<int>(wanted) > static_cast<int>(st.phase)) {
		st.phase = wanted;
		on_enter_phase(mob, st, wanted);
	}
}

[[nodiscard]] int roll_damage(const AbilityDef& ab) noexcept {
	int dam = ab.dam_base;
	if(ab.dice_n > 0 && ab.dice_s > 0) {
		dam += dice(ab.dice_n, ab.dice_s);
	}
	return dam;
}

[[nodiscard]] int soft_damage(const AbilityDef& ab, int full) noexcept {
	if(ab.soft_den <= 0) {
		return full / 3;
	}
	return (full * ab.soft_num) / ab.soft_den;
}

void resolve_cast(char_data* mob, BossState& st) {
	const BossProfile& profile = *st.profile;
	if(st.ability_index < 0 || st.ability_index >= profile.ability_count) {
		st.clear_cast();
		return;
	}
	const AbilityDef& ab =
		profile.abilities[static_cast<std::size_t>(st.ability_index)];

	Resolve how = st.resolve;
	if(st.cast_hp <= 0 && how != Resolve::Hard) {
		how = Resolve::Hard;
	}

	if(how == Resolve::Hard) {
		act("$c0010L'incantesimo di $n si spezza in una pioggia di scintille!$c0007",
			FALSE, mob, nullptr, nullptr, TO_ROOM);
		WAIT_STATE(mob, PULSE_VIOLENCE);
		st.clear_cast();
		st.skill_cd = 2;
		return;
	}

	const bool soft = (how == Resolve::Soft);
	const int full = roll_damage(ab);
	const int dam = soft ? soft_damage(ab, full) : full;
	const int atype = dmg_attack_type(ab.dmg);

	act_room(mob, soft ? ab.emote_soft : ab.emote_full);

	if(ab.target == PhaseBossTarget::Tank) {
		char_data* tank = mob->specials.fighting;
		if(tank && is_fight_pc(tank, mob)) {
			damage(mob, tank, dam, atype, 5);
		}
	}
	else {
		aoe_damage(mob, dam, atype);
	}

	st.clear_cast();
	st.skill_cd = profile.phases[static_cast<std::size_t>(st.phase)].cd_after_cast;
}

void tick_cast(char_data* mob, BossState& st) {
	if(st.ability_index < 0) {
		return;
	}
	if(st.windup_left > 1) {
		st.windup_left -= 1;
		const AbilityDef& ab =
			st.profile->abilities[static_cast<std::size_t>(st.ability_index)];
		act_room(mob, ab.emote_tick);
		return;
	}
	st.windup_left = 0;
	resolve_cast(mob, st);
}

void choose_cast(char_data* mob, BossState& st) {
	if(st.skill_cd > 0) {
		st.skill_cd -= 1;
		return;
	}
	if(st.busy() || pcs_in_room(mob).empty()) {
		return;
	}

	const BossProfile& profile = *st.profile;
	const int pi = static_cast<int>(st.phase);
	const int n = profile.weight_count[static_cast<std::size_t>(pi)];
	if(n <= 0) {
		return;
	}

	const int roll = number(1, 100);
	const AbilityDef* picked = nullptr;
	int picked_idx = -1;

	for(int i = 0; i < n; ++i) {
		const auto& w = profile.weights[static_cast<std::size_t>(pi)]
									 [static_cast<std::size_t>(i)];
		if(roll > w.chance) {
			continue;
		}
		const AbilityDef* ab = phase_boss_find_ability(profile, w.ability_key);
		if(!ab) {
			continue;
		}
		if(static_cast<int>(st.phase) < ab->min_phase) {
			continue;
		}
		if(st.fight_ticks < ab->min_fight_ticks) {
			continue;
		}
		picked = ab;
		for(int ai = 0; ai < profile.ability_count; ++ai) {
			if(profile.abilities[static_cast<std::size_t>(ai)].key == ab->key) {
				picked_idx = ai;
				break;
			}
		}
		break;
	}

	if(!picked || picked_idx < 0) {
		return;
	}

	st.begin_cast(picked_idx, *picked);
	act_room(mob, picked->emote_start);
	act_room(mob, picked->emote_hint);
}

void reset_if_idle(char_data* mob, BossState& st) {
	if(mob->specials.fighting || !pcs_in_room(mob).empty()) {
		return;
	}
	if(st.phase != Phase::Idle || st.busy() || st.fight_ticks || st.add_count) {
		if(st.profile) {
			extract_adds(mob, *st.profile);
		}
		st.reset();
	}
}

int phase_boss_engine(char_data* ch, int /*cmd*/, const char* /*arg*/, char_data* mob, int type,
					  const BossProfile* forced) {
	if(!mob) {
		return FALSE;
	}

	/* Interrupt gestito dalle skill di classe (class_interrupt.cpp), non da EVENT_COMMAND. */
	if(type == EVENT_COMMAND) {
		return FALSE;
	}

	BossState& st = state_of(mob);
	if(!st.profile) {
		st.profile = forced ? forced : resolve_profile(mob);
		/* Evita la fuga teleport dell'AI mage sotto 25% HP. */
		if(st.profile) {
			SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
		}
	}
	if(!st.profile) {
		return FALSE;
	}
	const BossProfile& profile = *st.profile;

	if(type == EVENT_DEATH) {
		extract_adds(mob, profile);
		act_room(mob, profile.death_emote);
		destroy_state(mob);
		return FALSE;
	}

	if(type != EVENT_TICK || !AWAKE(mob)) {
		return FALSE;
	}

	reset_if_idle(mob, st);
	if(!mob->specials.fighting && pcs_in_room(mob).empty()) {
		return FALSE;
	}

	++st.fight_ticks;

	if(st.fight_ticks >= profile.soft_enrage_ticks && st.phase != Phase::Wrath) {
		st.phase = Phase::Wrath;
		on_enter_phase(mob, st, Phase::Wrath);
	}

	update_phase(mob, st);

	if(st.busy()) {
		tick_cast(mob, st);
		return TRUE;
	}

	choose_cast(mob, st);
	return st.busy() ? TRUE : FALSE;
}

PhaseBossInterruptResult phase_boss_try_interrupt_impl(char_data* ch, char_data* victim,
													  int power) {
	if(!ch || !victim || !IS_NPC(victim) || victim->nr < 0) {
		return PhaseBossInterruptResult::NotBoss;
	}

	const BossProfile* prof = nullptr;
	if(victim->act_ptr) {
		prof = static_cast<const BossState*>(victim->act_ptr)->profile;
	}
	if(!prof) {
		prof = resolve_profile(victim);
	}
	if(!prof) {
		return PhaseBossInterruptResult::NotBoss;
	}

	auto& st = state_of(victim);
	st.profile = prof;

	if(!st.casting()) {
		return PhaseBossInterruptResult::NotCasting;
	}
	if(st.resolve == Resolve::Hard) {
		send_to_char(st.profile->already_broken, ch);
		return PhaseBossInterruptResult::AlreadyBroken;
	}
	if(!phase_boss_any(st.interrupt_mask, PhaseBossInterrupt::ClassBreak)) {
		act("Questa azione di $N non puo' essere interrotta!", FALSE, ch, nullptr, victim,
			TO_CHAR);
		return PhaseBossInterruptResult::NotCasting;
	}

	st.cast_hp -= (power > 0 ? power : 40);
	if(st.cast_hp <= 0) {
		st.resolve = Resolve::Hard;
		act("$c0010$n spezza l'incantesimo di $N!$c0007", FALSE, ch, nullptr, victim, TO_ROOM);
		act("$c0010Spezzi l'incantesimo di $N!$c0007", FALSE, ch, nullptr, victim, TO_CHAR);
		st.windup_left = 0;
		resolve_cast(victim, st);
		return PhaseBossInterruptResult::Hard;
	}

	st.resolve = Resolve::Soft;
	act("$c0014$n indebolisce l'incantesimo di $N!$c0007", FALSE, ch, nullptr, victim, TO_ROOM);
	act("$c0014Il potere di $N vacilla!$c0007", FALSE, ch, nullptr, victim, TO_CHAR);
	return PhaseBossInterruptResult::Soft;
}

} // namespace

PhaseBossInterruptResult phase_boss_try_interrupt(char_data* ch, char_data* victim, int power) {
	return phase_boss_try_interrupt_impl(ch, victim, power);
}

bool phase_boss_is_boss(const char_data* ch) noexcept {
	if(!ch || !IS_NPC(ch) || ch->nr < 0) {
		return false;
	}
	if(mob_index[ch->nr].func == reinterpret_cast<genericspecial_func>(PhaseBoss)) {
		return true;
	}
	return phase_boss_find_by_vnum(mob_index[ch->nr].iVNum) != nullptr;
}

bool phase_boss_resists_crowd_control(const char_data* ch) noexcept {
	return phase_boss_is_boss(ch);
}

bool phase_boss_try_set_pos(char_data* victim, int pos) noexcept {
	if(!victim || phase_boss_resists_crowd_control(victim)) {
		return false;
	}
	GET_POS(victim) = pos;
	return true;
}

MOBSPECIAL_FUNC(PhaseBoss) {
	/* Corte Olimpo (Zeus): allineata a OlympianKeyDrop (tick + death clear). */
	if(mob != nullptr && mob->nr >= 0
			&& is_olympian_god_vnum(mob_index[mob->nr].iVNum)) {
		if(type == EVENT_TICK) {
			olympian_maintain_court_room(real_roomp(mob->in_room));
		}
		else if(type == EVENT_DEATH) {
			olympian_clear_court_on_god_death(mob);
		}
	}
	return phase_boss_engine(ch, cmd, arg, mob, type, nullptr);
}

} // namespace Alarmud
