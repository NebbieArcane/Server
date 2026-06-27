/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "handler.hpp"
#include "comm.hpp"
#include "procarea_fatigue.hpp"
#include "procarea_internal.hpp"
#include "reception.hpp"
#include "utility.hpp"
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

static constexpr int kProcFatigueGearBase[PROCAREA_FATIGUE_TIER_COUNT] = {
	100, 75, 60, 48, 40, 35,
};
static constexpr int kProcFatigueGearDecay[PROCAREA_FATIGUE_TIER_COUNT] = {
	25, 30, 35, 40, 42, 45,
};
static constexpr int kProcFatigueGoldPct[PROCAREA_FATIGUE_TIER_COUNT] = {
	100, 85, 75, 65, 58, 55,
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

void procarea_fatigue_notify(char_data* ch, int treasure_tier) {
	if(ch == nullptr || !IS_PC(ch) || treasure_tier <= 0) {
		return;
	}
	send_to_char(
		"$c0010La bruma riconosce chi l'ha gia' attraversata oggi:$c0007 "
		"le reliquie effimere si formano piu' lentamente.\n\r",
		ch);
}

} // namespace

int procarea_fatigue_day_id() {
	return procarea_fatigue_today_id();
}

int procarea_fatigue_tier_from_clears(int clears_before) {
	if(clears_before < PROCAREA_FATIGUE_FULL_CLEARS) {
		return 0;
	}
	return std::clamp(clears_before - (PROCAREA_FATIGUE_FULL_CLEARS - 1), 1,
					  PROCAREA_FATIGUE_TIER_COUNT - 1);
}

int procarea_fatigue_tier_for_name(const std::string& name, bool solo_mode) {
	return procarea_fatigue_tier_from_clears(procarea_fatigue_clears_for_name(name.c_str(), solo_mode));
}

int procarea_fatigue_gear_drop_pct(int hoard_index, int fatigue_tier) {
	const int hoard = std::max(1, hoard_index);
	const int tier = std::clamp(fatigue_tier, 0, PROCAREA_FATIGUE_TIER_COUNT - 1);
	const int base = kProcFatigueGearBase[tier];
	const int decay = kProcFatigueGearDecay[tier];
	return std::max(0, base - decay * (hoard - 1));
}

bool procarea_fatigue_roll_gold(int fatigue_tier) {
	const int tier = std::clamp(fatigue_tier, 0, PROCAREA_FATIGUE_TIER_COUNT - 1);
	const int pct = kProcFatigueGoldPct[tier];
	return pct >= 100 || number(0, 99) < pct;
}

int procarea_fatigue_treasure_tier_for_instance(const procarea_internal::ProcAreaInstance& inst) {
	if(!inst.owner_name.empty()) {
		return procarea_fatigue_tier_for_name(inst.owner_name, inst.solo_mode);
	}
	for(const std::string& member : inst.member_names) {
		return procarea_fatigue_tier_for_name(member, inst.solo_mode);
	}
	return 0;
}

void procarea_fatigue_on_boss_killed(procarea_internal::ProcAreaInstance& inst, int treasure_tier) {
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
	}

	if(treasure_tier <= 0) {
		return;
	}

	std::unordered_set<char_data*> notified;
	auto notify = [&](char_data* ch) {
		if(ch == nullptr || !IS_PC(ch)) {
			return;
		}
		if(notified.insert(ch).second) {
			procarea_fatigue_notify(ch, treasure_tier);
		}
	};

	for(long vnum : inst.room_vnums) {
		struct room_data* rp = real_roomp(vnum);
		if(rp == nullptr) {
			continue;
		}
		for(char_data* ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
			notify(ch);
		}
	}
	for(const std::string& member : inst.member_names) {
		notify(get_char(member.c_str()));
	}
	if(!inst.owner_name.empty()) {
		notify(get_char(inst.owner_name.c_str()));
	}
}

} // namespace Alarmud
