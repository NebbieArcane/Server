/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "handler.hpp"
#include "procarea_fatigue.hpp"
#include "procarea_internal.hpp"
#include "reception.hpp"
#include "utility.hpp"
#include "procarea.hpp"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <string>
#include <unordered_map>
#include <unordered_set>
namespace Alarmud {

namespace {

struct ProcareaFatigueState {
	int day_id = 0;
	int solo_clears = 0;
	int group_clears = 0;
};

std::unordered_map<std::string, ProcareaFatigueState> g_procarea_fatigue;

/*
 * tier 0 = run 1-3 piene; tier 1..5 = run 4..8+.
 * Gear: 75 → 60 → 45 → 30 → 10 (−15/run, floor 10% dall'8ª).
 * Decay hoard extra: ~25% del base.
 * Oro: stessa pendenza (−14%/run), floor 30% dall'8ª.
 */
static constexpr int kProcFatigueGearBase[PROCAREA_FATIGUE_TIER_COUNT] = {
	100, 75, 60, 45, 30, 10,
};
static constexpr int kProcFatigueGearDecay[PROCAREA_FATIGUE_TIER_COUNT] = {
	25, 19, 15, 11, 8, 10,
};
static constexpr int kProcFatigueGoldPct[PROCAREA_FATIGUE_TIER_COUNT] = {
	100, 86, 72, 58, 44, 30,
};

[[nodiscard]] std::string procarea_fatigue_key(const char* name) {
	if(name == nullptr || *name == '\0') {
		return {};
	}
	std::string key(name);
	for(char& c : key) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return key;
}

[[nodiscard]] int procarea_fatigue_today_id() {
	const time_t now = time(nullptr);
	struct tm tm_buf {};
#if defined(_POSIX_VERSION)
	localtime_r(&now, &tm_buf);
#else
	struct tm* tmp = localtime(&now);
	if(tmp != nullptr) {
		tm_buf = *tmp;
	}
#endif
	return (tm_buf.tm_year + 1900) * 10000 + (tm_buf.tm_mon + 1) * 100 + tm_buf.tm_mday;
}

void procarea_fatigue_refresh_state(ProcareaFatigueState& state) {
	const int today = procarea_fatigue_today_id();
	if(state.day_id != today) {
		state.day_id = today;
		state.solo_clears = 0;
		state.group_clears = 0;
	}
}

void procarea_fatigue_refresh_char(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	const int today = procarea_fatigue_today_id();
	if(ch->specials.procarea_fatigue_day != today) {
		ch->specials.procarea_fatigue_day = today;
		ch->specials.procarea_fatigue_solo = 0;
		ch->specials.procarea_fatigue_group = 0;
	}
}

void procarea_fatigue_sync_char_to_state(char_data* ch, ProcareaFatigueState& state) {
	procarea_fatigue_refresh_char(ch);
	state.day_id = ch->specials.procarea_fatigue_day;
	state.solo_clears = ch->specials.procarea_fatigue_solo;
	state.group_clears = ch->specials.procarea_fatigue_group;
}

void procarea_fatigue_sync_state_to_char(char_data* ch, const ProcareaFatigueState& state) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	ch->specials.procarea_fatigue_day = state.day_id;
	ch->specials.procarea_fatigue_solo = state.solo_clears;
	ch->specials.procarea_fatigue_group = state.group_clears;
}

ProcareaFatigueState procarea_fatigue_load_state(const char* name) {
	ProcareaFatigueState state{};
	const std::string key = procarea_fatigue_key(name);
	if(key.empty()) {
		state.day_id = procarea_fatigue_today_id();
		return state;
	}

	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		procarea_fatigue_sync_char_to_state(ch, state);
		g_procarea_fatigue[key] = state;
		return state;
	}

	const auto cached = g_procarea_fatigue.find(key);
	if(cached != g_procarea_fatigue.end()) {
		state = cached->second;
		procarea_fatigue_refresh_state(state);
		g_procarea_fatigue[key] = state;
		return state;
	}

#if USE_MYSQL
	if(procarea_fatigue_load_mysql(name, state.day_id, state.solo_clears, state.group_clears)) {
		procarea_fatigue_refresh_state(state);
	} else {
		state.day_id = procarea_fatigue_today_id();
	}
#else
	state.day_id = procarea_fatigue_today_id();
#endif

	g_procarea_fatigue[key] = state;
	return state;
}

void procarea_fatigue_store_state(const char* name, const ProcareaFatigueState& state) {
	if(name == nullptr || *name == '\0') {
		return;
	}
	const std::string key = procarea_fatigue_key(name);
	g_procarea_fatigue[key] = state;

	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		procarea_fatigue_sync_state_to_char(ch, state);
	}

#if USE_MYSQL
	procarea_fatigue_save_mysql(name, state.day_id, state.solo_clears, state.group_clears);
#endif
}

[[nodiscard]] int procarea_fatigue_clears_for_name(const char* name, bool solo_mode) {
	const ProcareaFatigueState state = procarea_fatigue_load_state(name);
	return solo_mode ? state.solo_clears : state.group_clears;
}

void procarea_fatigue_increment_name(const char* name, bool solo_mode) {
	ProcareaFatigueState state = procarea_fatigue_load_state(name);
	if(solo_mode) {
		++state.solo_clears;
	} else {
		++state.group_clears;
	}
	procarea_fatigue_store_state(name, state);
}

static void procarea_clears_totals_persist_char(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	const char* name = GET_NAME(ch);
	if(name == nullptr || *name == '\0') {
		return;
	}
#if USE_MYSQL
	procarea_clears_totals_save_mysql(
		name, ch->specials.procarea_clears_solo_total, ch->specials.procarea_clears_group_total);
#else
	(void)name;
#endif
}

static void procarea_clears_totals_increment_name(const char* name, bool solo_mode) {
	if(name == nullptr || *name == '\0') {
		return;
	}

	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		if(solo_mode) {
			++ch->specials.procarea_clears_solo_total;
		} else {
			++ch->specials.procarea_clears_group_total;
		}
		procarea_clears_totals_persist_char(ch);
		procarea_clears_sync_achievements(ch);
		return;
	}

	int solo_total = 0;
	int group_total = 0;
#if USE_MYSQL
	procarea_clears_totals_load_mysql(name, solo_total, group_total);
#endif
	if(solo_mode) {
		++solo_total;
	} else {
		++group_total;
	}
#if USE_MYSQL
	procarea_clears_totals_save_mysql(name, solo_total, group_total);
#endif
}

[[nodiscard]] static float procarea_fatigue_group_effective_clears(
	const procarea_internal::ProcAreaInstance& inst) {
	std::unordered_set<std::string> names;
	auto track = [&](const char* name) {
		if(name != nullptr && *name != '\0') {
			names.insert(name);
		}
	};

	track(inst.owner_name.c_str());
	for(const std::string& member : inst.member_names) {
		track(member.c_str());
	}
	if(names.empty()) {
		return 0.0f;
	}

	float sum = 0.0f;
	int peak = 0;
	for(const std::string& name : names) {
		const int clears = procarea_fatigue_clears_for_name(name.c_str(), false);
		sum += static_cast<float>(clears);
		peak = std::max(peak, clears);
	}
	const float avg = sum / static_cast<float>(names.size());
	return PROCAREA_GROUP_FATIGUE_AVG_WEIGHT * avg +
		   PROCAREA_GROUP_FATIGUE_MAX_WEIGHT * static_cast<float>(peak);
}

[[nodiscard]] static int procarea_fatigue_group_tier_for_instance(
	const procarea_internal::ProcAreaInstance& inst) {
	return procarea_fatigue_tier_from_effective_clears(
		procarea_fatigue_group_effective_clears(inst));
}

} // namespace

int procarea_fatigue_day_id() {
	return procarea_fatigue_today_id();
}

int procarea_fatigue_tier_from_clears(int clears_before) {
	return procarea_fatigue_tier_from_effective_clears(static_cast<float>(clears_before));
}

int procarea_fatigue_tier_from_effective_clears(float effective_clears) {
	if(effective_clears < static_cast<float>(PROCAREA_FATIGUE_FULL_CLEARS)) {
		return 0;
	}
	const int tier =
		static_cast<int>(effective_clears - static_cast<float>(PROCAREA_FATIGUE_FULL_CLEARS - 1) +
						 0.5f);
	return std::clamp(tier, 1, PROCAREA_FATIGUE_TIER_COUNT - 1);
}

int procarea_fatigue_tier_for_name(const std::string& name, bool solo_mode) {
	return procarea_fatigue_tier_from_clears(procarea_fatigue_clears_for_name(name.c_str(), solo_mode));
}

int procarea_fatigue_solo_clears_for_name(const char* name) {
	return procarea_fatigue_clears_for_name(name, true);
}

int procarea_fatigue_group_clears_for_name(const char* name) {
	return procarea_fatigue_clears_for_name(name, false);
}

int procarea_fatigue_gear_drop_pct(int hoard_index, int fatigue_tier) {
	const int hoard = std::max(1, hoard_index);
	const int tier = std::clamp(fatigue_tier, 0, PROCAREA_FATIGUE_TIER_COUNT - 1);
	const int base = kProcFatigueGearBase[tier];
	const int decay = kProcFatigueGearDecay[tier];
	return std::max(0, base - decay * (hoard - 1));
}

int procarea_fatigue_gold_drop_pct(int fatigue_tier) {
	const int tier = std::clamp(fatigue_tier, 0, PROCAREA_FATIGUE_TIER_COUNT - 1);
	return kProcFatigueGoldPct[tier];
}

bool procarea_fatigue_roll_gold(int fatigue_tier) {
	const int tier = std::clamp(fatigue_tier, 0, PROCAREA_FATIGUE_TIER_COUNT - 1);
	const int pct = kProcFatigueGoldPct[tier];
	return pct >= 100 || number(0, 99) < pct;
}

float procarea_fatigue_group_effective_clears_for_instance(
	const procarea_internal::ProcAreaInstance& inst) {
	return procarea_fatigue_group_effective_clears(inst);
}

int procarea_fatigue_treasure_tier_for_instance(const procarea_internal::ProcAreaInstance& inst) {
	if(inst.solo_mode) {
		if(!inst.owner_name.empty()) {
			return procarea_fatigue_tier_for_name(inst.owner_name, true);
		}
		for(const std::string& member : inst.member_names) {
			return procarea_fatigue_tier_for_name(member, true);
		}
		return 0;
	}
	return procarea_fatigue_group_tier_for_instance(inst);
}

void procarea_fatigue_on_boss_killed(procarea_internal::ProcAreaInstance& inst, int treasure_tier) {
	(void)treasure_tier;
	std::unordered_set<std::string> names;
	auto track = [&](const char* name) {
		if(name != nullptr && *name != '\0') {
			names.insert(name);
		}
	};

	if(!inst.owner_name.empty()) {
		track(inst.owner_name.c_str());
	}
	for(const std::string& member : inst.member_names) {
		track(member.c_str());
	}

	for(const std::string& name : names) {
		procarea_fatigue_increment_name(name.c_str(), inst.solo_mode);
		procarea_clears_totals_increment_name(name.c_str(), inst.solo_mode);
	}
}

int procarea_clears_solo_total_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_clears_solo_total);
}

int procarea_clears_group_total_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_clears_group_total);
}

int procarea_clears_total_get(const char_data* ch) {
	return procarea_clears_solo_total_get(ch) + procarea_clears_group_total_get(ch);
}

static void procarea_clears_sync_achievement(char_data* ch, int achie_type, int target) {
	if(ch == nullptr || !IS_PC(ch) || target <= 0) {
		return;
	}

	char_data* tch = ch;
	if(IS_POLY(ch) && ch->desc != nullptr && ch->desc->original != nullptr) {
		tch = ch->desc->original;
	}

	int& val = tch->specials.achievements[OTHER_ACHIE][achie_type];
	while(val < target) {
		++val;
		if(!IS_SET(tch->specials.act, PLR_ACHIE)) {
			SET_BIT(tch->specials.act, PLR_ACHIE);
		}
		CheckAchie(ch, achie_type, OTHER_ACHIE);
	}
}

void procarea_clears_sync_achievements(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	const int solo = procarea_clears_solo_total_get(ch);
	const int group = procarea_clears_group_total_get(ch);
	procarea_clears_sync_achievement(ch, ACHIE_PROCAREA_TOTAL, solo + group);
	procarea_clears_sync_achievement(ch, ACHIE_PROCAREA_SOLO, solo);
	procarea_clears_sync_achievement(ch, ACHIE_PROCAREA_GROUP, group);
}

static void procarea_grant_other_achievement(char_data* ch, int achie_type) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	char_data* tch = ch;
	if(IS_POLY(ch) && ch->desc != nullptr && ch->desc->original != nullptr) {
		tch = ch->desc->original;
	}

	tch->specials.achievements[OTHER_ACHIE][achie_type] += 1;
	if(!IS_SET(tch->specials.act, PLR_ACHIE)) {
		SET_BIT(tch->specials.act, PLR_ACHIE);
	}
	CheckAchie(ch, achie_type, OTHER_ACHIE);
}

void procarea_achievement_on_pc_death(char_data* victim) {
	if(victim == nullptr || !IS_PC(victim)) {
		return;
	}
	if(victim->in_room < 0 || !procarea_is_generated_room(victim->in_room)) {
		return;
	}
	procarea_grant_other_achievement(victim, ACHIE_PROCAREA_DEATH);
}

void procarea_achievement_on_mob_kill(char_data* killer, int proc_mob_kind) {
	if(killer == nullptr || !IS_PC(killer)) {
		return;
	}

	int achie_type = -1;
	switch(static_cast<procarea_internal::ProcMobKind>(proc_mob_kind)) {
	case procarea_internal::ProcMobKind::Boss:
		achie_type = ACHIE_PROCAREA_KILL_BOSS;
		break;
	case procarea_internal::ProcMobKind::Trap:
		achie_type = ACHIE_PROCAREA_KILL_TRAP;
		break;
	case procarea_internal::ProcMobKind::Normal:
		achie_type = ACHIE_PROCAREA_KILL_MOB;
		break;
	default:
		return;
	}

	procarea_grant_other_achievement(killer, achie_type);
}

} // namespace Alarmud
