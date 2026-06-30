/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "handler.hpp"
#include "procarea_internal.hpp"
#include "procarea_records.hpp"
#include "reception.hpp"
#include "logging.hpp"
#include "comm.hpp"
#include "procarea_fatigue.hpp"
#include "procarea_rune_fragments.hpp"
#if USE_MYSQL
#include "Sql.hpp"
#include <odb/mysql/database.hxx>
#include <mysql/mysql.h>
#endif
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
namespace Alarmud {

static std::unordered_set<std::string> g_procarea_records_deferred_persist;

namespace {

struct ProcareaRecordState {
	int best_eff_band = 0;
	int best_tmpl_band = 0;
	int best_power_centi = 0;
	int fast_day_id = 0;
	int fast_day_sec = 0;
	int fast_day_band = 0;
	int fast_day_power_centi = 0;
	int fast_week_id = 0;
	int fast_week_sec = 0;
	int fast_week_band = 0;
	int fast_week_power_centi = 0;
	int captain_clears = 0;
	int best_hoard = 0;
	int best_sigils = 0;
};

[[nodiscard]] int procarea_records_local_tm(struct tm& tm_buf) {
	const time_t now = time(nullptr);
#if defined(_POSIX_VERSION)
	return localtime_r(&now, &tm_buf) != nullptr ? 0 : -1;
#else
	struct tm* tmp = localtime(&now);
	if(tmp == nullptr) {
		return -1;
	}
	tm_buf = *tmp;
	return 0;
#endif
}

[[nodiscard]] int procarea_records_power_centi(float power) {
	return std::max(0, static_cast<int>(power + 0.5f));
}

void procarea_records_sync_from_char(const char_data* ch, ProcareaRecordState& state) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	state.best_eff_band = std::max(0, ch->specials.procarea_rec_best_eff_band);
	state.best_tmpl_band = std::max(0, ch->specials.procarea_rec_best_tmpl_band);
	state.best_power_centi = std::max(0, ch->specials.procarea_rec_best_power_centi);
	state.fast_day_id = ch->specials.procarea_rec_fast_day_id;
	state.fast_day_sec = std::max(0, ch->specials.procarea_rec_fast_day_sec);
	state.fast_day_band = std::max(0, ch->specials.procarea_rec_fast_day_band);
	state.fast_day_power_centi = std::max(0, ch->specials.procarea_rec_fast_day_power);
	state.fast_week_id = ch->specials.procarea_rec_fast_week_id;
	state.fast_week_sec = std::max(0, ch->specials.procarea_rec_fast_week_sec);
	state.fast_week_band = std::max(0, ch->specials.procarea_rec_fast_week_band);
	state.fast_week_power_centi = std::max(0, ch->specials.procarea_rec_fast_week_power);
	state.captain_clears = std::max(0, ch->specials.procarea_rec_captain_clears);
	state.best_hoard = std::max(0, ch->specials.procarea_rec_best_hoard);
	state.best_sigils = std::max(0, ch->specials.procarea_rec_best_sigils);
}

void procarea_records_sync_to_char(char_data* ch, const ProcareaRecordState& state) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}
	ch->specials.procarea_rec_best_eff_band = state.best_eff_band;
	ch->specials.procarea_rec_best_tmpl_band = state.best_tmpl_band;
	ch->specials.procarea_rec_best_power_centi = state.best_power_centi;
	ch->specials.procarea_rec_fast_day_id = state.fast_day_id;
	ch->specials.procarea_rec_fast_day_sec = state.fast_day_sec;
	ch->specials.procarea_rec_fast_day_band = state.fast_day_band;
	ch->specials.procarea_rec_fast_day_power = state.fast_day_power_centi;
	ch->specials.procarea_rec_fast_week_id = state.fast_week_id;
	ch->specials.procarea_rec_fast_week_sec = state.fast_week_sec;
	ch->specials.procarea_rec_fast_week_band = state.fast_week_band;
	ch->specials.procarea_rec_fast_week_power = state.fast_week_power_centi;
	ch->specials.procarea_rec_captain_clears = state.captain_clears;
	ch->specials.procarea_rec_best_hoard = state.best_hoard;
	ch->specials.procarea_rec_best_sigils = state.best_sigils;
}

#if USE_MYSQL

bool procarea_records_mysql_select(const std::string& sql, MYSQL_RES*& out_res) {
	out_res = nullptr;
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return false;
	}
	odb::connection_ptr cp(db->connection());
	auto& mc = static_cast<odb::mysql::connection&>(*cp);
	MYSQL* h = mc.handle();
	if(mysql_query(h, sql.c_str()) != 0) {
		mudlog(LOG_SYSERR, "procarea_records_mysql_select: %s", mysql_error(h));
		return false;
	}
	out_res = mysql_store_result(h);
	return out_res != nullptr;
}

void procarea_records_insert_pref(odb::database* db, unsigned long long toon_id, const char* key,
								  const char* value) {
	if(db == nullptr || key == nullptr || value == nullptr) {
		return;
	}
	std::ostringstream sql;
	sql << "INSERT INTO character_prefs (toon_id, pref_key, pref_value) VALUES (" << toon_id
		<< ",'" << key << "','" << value
		<< "') ON DUPLICATE KEY UPDATE pref_value = VALUES(pref_value)";
	db->execute(sql.str().c_str());
}

void procarea_records_save_mysql(const char* name, const ProcareaRecordState& state) {
	if(name == nullptr || *name == '\0') {
		return;
	}
	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
	if(!pg || !pg->id) {
		return;
	}

	try {
		DB* db = Sql::getMysql();
		if(db == nullptr) {
			return;
		}
		odb::transaction t(db->begin());
		t.tracer(logTracer);

		char vbuf[64];
		if(state.best_eff_band > 0) {
			std::snprintf(vbuf, sizeof(vbuf), "%d:%d", state.best_eff_band, state.best_tmpl_band);
			procarea_records_insert_pref(db, pg->id, "procarea_band_mx", vbuf);
		}
		if(state.best_power_centi > 0) {
			std::snprintf(vbuf, sizeof(vbuf), "%d", state.best_power_centi);
			procarea_records_insert_pref(db, pg->id, "procarea_pwr_mx", vbuf);
		}
		if(state.fast_day_sec > 0) {
			std::snprintf(vbuf, sizeof(vbuf), "%d:%d:%d:%d", state.fast_day_id, state.fast_day_sec,
						   state.fast_day_band, state.fast_day_power_centi);
			procarea_records_insert_pref(db, pg->id, "procarea_fast_dy", vbuf);
		}
		if(state.fast_week_sec > 0) {
			std::snprintf(vbuf, sizeof(vbuf), "%d:%d:%d:%d", state.fast_week_id, state.fast_week_sec,
						   state.fast_week_band, state.fast_week_power_centi);
			procarea_records_insert_pref(db, pg->id, "procarea_fast_wk", vbuf);
		}
		if(state.captain_clears > 0) {
			std::snprintf(vbuf, sizeof(vbuf), "%d", state.captain_clears);
			procarea_records_insert_pref(db, pg->id, "procarea_captain", vbuf);
		}
		if(state.best_hoard > 0) {
			std::snprintf(vbuf, sizeof(vbuf), "%d", state.best_hoard);
			procarea_records_insert_pref(db, pg->id, "procarea_hoard_mx", vbuf);
		}
		if(state.best_sigils > 0) {
			std::snprintf(vbuf, sizeof(vbuf), "%d", state.best_sigils);
			procarea_records_insert_pref(db, pg->id, "procarea_sigil_mx", vbuf);
		}

		t.commit();
	} catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "procarea_records_save_mysql(%s): %s", name, e.what());
	}
}

void procarea_records_load_mysql(const char* name, ProcareaRecordState& state) {
	if(name == nullptr || *name == '\0') {
		return;
	}
	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
	if(!pg || !pg->id) {
		return;
	}

	auto load_one = [&](const char* key, auto apply_fn) {
		std::ostringstream sql;
		sql << "SELECT pref_value FROM character_prefs WHERE toon_id = " << pg->id
			<< " AND pref_key = '" << key << "' LIMIT 1";
		MYSQL_RES* res = nullptr;
		if(!procarea_records_mysql_select(sql.str(), res) || res == nullptr) {
			return;
		}
		if(MYSQL_ROW row = mysql_fetch_row(res); row != nullptr && row[0] != nullptr) {
			apply_fn(row[0]);
		}
		mysql_free_result(res);
	};

	load_one("procarea_band_mx", [&](const char* value) {
		int eff = 0;
		int tmpl = 0;
		if(std::sscanf(value, "%d:%d", &eff, &tmpl) == 2) {
			state.best_eff_band = std::max(0, eff);
			state.best_tmpl_band = std::max(0, tmpl);
		}
	});
	load_one("procarea_pwr_mx", [&](const char* value) {
		state.best_power_centi = std::max(0, std::atoi(value));
	});
	load_one("procarea_fast_dy", [&](const char* value) {
		int day = 0;
		int sec = 0;
		int band = 0;
		int power = 0;
		if(std::sscanf(value, "%d:%d:%d:%d", &day, &sec, &band, &power) == 4) {
			state.fast_day_id = day;
			state.fast_day_sec = std::max(0, sec);
			state.fast_day_band = std::max(0, band);
			state.fast_day_power_centi = std::max(0, power);
		} else if(std::sscanf(value, "%d:%d", &day, &sec) == 2) {
			state.fast_day_id = day;
			state.fast_day_sec = std::max(0, sec);
		}
	});
	load_one("procarea_fast_wk", [&](const char* value) {
		int week = 0;
		int sec = 0;
		int band = 0;
		int power = 0;
		if(std::sscanf(value, "%d:%d:%d:%d", &week, &sec, &band, &power) == 4) {
			state.fast_week_id = week;
			state.fast_week_sec = std::max(0, sec);
			state.fast_week_band = std::max(0, band);
			state.fast_week_power_centi = std::max(0, power);
		} else if(std::sscanf(value, "%d:%d", &week, &sec) == 2) {
			state.fast_week_id = week;
			state.fast_week_sec = std::max(0, sec);
		}
	});
	load_one("procarea_captain", [&](const char* value) {
		state.captain_clears = std::max(0, std::atoi(value));
	});
	load_one("procarea_hoard_mx", [&](const char* value) {
		state.best_hoard = std::max(0, std::atoi(value));
	});
	load_one("procarea_sigil_mx", [&](const char* value) {
		state.best_sigils = std::max(0, std::atoi(value));
	});
}

#endif /* USE_MYSQL */

void procarea_records_persist(const char* name, const ProcareaRecordState& state) {
	if(name == nullptr || *name == '\0') {
		return;
	}
#if USE_MYSQL
	procarea_records_save_mysql(name, state);
#else
	(void)state;
#endif
}

ProcareaRecordState procarea_records_load(const char* name) {
	ProcareaRecordState state{};
	if(name == nullptr || *name == '\0') {
		return state;
	}

	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		procarea_records_sync_from_char(ch, state);
		return state;
	}

#if USE_MYSQL
	procarea_records_load_mysql(name, state);
#endif
	return state;
}

void procarea_records_store(const char* name, const ProcareaRecordState& state) {
	if(name == nullptr || *name == '\0') {
		return;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		procarea_records_sync_to_char(ch, state);
		g_procarea_records_deferred_persist.insert(name);
		return;
	}
	procarea_records_persist(name, state);
}

void procarea_records_collect_names(const procarea_internal::ProcAreaInstance& inst,
									std::vector<std::string>& out) {
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
	out.assign(names.begin(), names.end());
}

void procarea_records_update_band(ProcareaRecordState& state, int eff_band, int tmpl_band) {
	if(eff_band > state.best_eff_band) {
		state.best_eff_band = eff_band;
		state.best_tmpl_band = tmpl_band;
	} else if(eff_band == state.best_eff_band && tmpl_band > state.best_tmpl_band) {
		state.best_tmpl_band = tmpl_band;
	}
}

void procarea_records_update_power(ProcareaRecordState& state, int power_centi, int eff_band) {
	if(power_centi > state.best_power_centi) {
		state.best_power_centi = power_centi;
		(void)eff_band;
	}
}

void procarea_records_update_fast_day(ProcareaRecordState& state, int day_id, int clear_sec,
									  int eff_band, int power_centi) {
	if(clear_sec <= 0) {
		return;
	}
	if(state.fast_day_id != day_id || state.fast_day_sec <= 0 || clear_sec < state.fast_day_sec) {
		state.fast_day_id = day_id;
		state.fast_day_sec = clear_sec;
		state.fast_day_band = std::max(0, eff_band);
		state.fast_day_power_centi = std::max(0, power_centi);
	}
}

void procarea_records_update_fast_week(ProcareaRecordState& state, int week_id, int clear_sec,
									   int eff_band, int power_centi) {
	if(clear_sec <= 0) {
		return;
	}
	if(state.fast_week_id != week_id || state.fast_week_sec <= 0 ||
	   clear_sec < state.fast_week_sec) {
		state.fast_week_id = week_id;
		state.fast_week_sec = clear_sec;
		state.fast_week_band = std::max(0, eff_band);
		state.fast_week_power_centi = std::max(0, power_centi);
	}
}

} // namespace

void procarea_records_flush_deferred() {
	if(g_procarea_records_deferred_persist.empty()) {
		return;
	}
	std::vector<std::string> names(g_procarea_records_deferred_persist.begin(),
								   g_procarea_records_deferred_persist.end());
	g_procarea_records_deferred_persist.clear();
	for(const std::string& name : names) {
		ProcareaRecordState state = procarea_records_load(name.c_str());
		procarea_records_persist(name.c_str(), state);
	}
}

void procarea_records_flush_deferred_for(const char* name) {
	if(name == nullptr || *name == '\0') {
		return;
	}
	if(g_procarea_records_deferred_persist.erase(name) == 0) {
		return;
	}
	ProcareaRecordState state = procarea_records_load(name);
	procarea_records_persist(name, state);
}

int procarea_records_day_id() {
	struct tm tm_buf {};
	if(procarea_records_local_tm(tm_buf) != 0) {
		return 0;
	}
	return (tm_buf.tm_year + 1900) * 10000 + (tm_buf.tm_mon + 1) * 100 + tm_buf.tm_mday;
}

int procarea_records_week_id() {
	struct tm tm_buf {};
	if(procarea_records_local_tm(tm_buf) != 0) {
		return 0;
	}
	const int year = tm_buf.tm_year + 1900;
	const int month = tm_buf.tm_mon + 1;
	const int day = tm_buf.tm_mday;
	int week = (tm_buf.tm_yday + 7 - ((tm_buf.tm_wday + 6) % 7)) / 7 + 1;
	if(week <= 0) {
		week = 1;
	}
	if(month == 1 && day <= 7 && week > 50) {
		return (year - 1) * 100 + week;
	}
	if(month == 12 && day >= 28 && week == 1) {
		return (year + 1) * 100 + week;
	}
	return year * 100 + week;
}

void procarea_records_on_mob_death(char_data* victim, procarea_internal::ProcAreaInstance& inst) {
	if(victim == nullptr || !IS_NPC(victim)) {
		return;
	}
	if(victim->commandp == static_cast<int>(procarea_internal::ProcMobKind::Boss)) {
		return;
	}

	if(victim->commandp == static_cast<int>(procarea_internal::ProcMobKind::Trap)) {
		++inst.run_kill_sigils;
		return;
	}

	if(victim->commandp == static_cast<int>(procarea_internal::ProcMobKind::Normal) &&
	   victim->master != nullptr && IS_NPC(victim->master) &&
	   victim->master->commandp == static_cast<int>(procarea_internal::ProcMobKind::Boss)) {
		++inst.run_kill_sigils;
	}
}

void procarea_records_on_boss_killed(procarea_internal::ProcAreaInstance& inst, int clear_sec) {
	const int day_id = procarea_records_day_id();
	const int week_id = procarea_records_week_id();
	const int clear_time = std::max(1, clear_sec);
	const int power_centi = procarea_records_power_centi(inst.group_eq_index);
	const int eff_band = std::max(0, inst.effective_band);
	const int tmpl_band = std::max(0, inst.template_band);
	const int sigil_count = std::max(0, inst.run_kill_sigils);

	std::vector<std::string> names;
	procarea_records_collect_names(inst, names);
	for(const std::string& name : names) {
		ProcareaRecordState state = procarea_records_load(name.c_str());
		procarea_records_update_band(state, eff_band, tmpl_band);
		procarea_records_update_power(state, power_centi, eff_band);
		procarea_records_update_fast_day(state, day_id, clear_time, eff_band, power_centi);
		procarea_records_update_fast_week(state, week_id, clear_time, eff_band, power_centi);
		if(sigil_count > state.best_sigils) {
			state.best_sigils = sigil_count;
		}
		procarea_records_store(name.c_str(), state);
	}

	if(!inst.solo_mode && !inst.owner_name.empty()) {
		ProcareaRecordState owner_state = procarea_records_load(inst.owner_name.c_str());
		++owner_state.captain_clears;
		procarea_records_store(inst.owner_name.c_str(), owner_state);
	}
}

void procarea_records_on_instance_end(const procarea_internal::ProcAreaInstance& inst) {
	if(!inst.exit_portal_open) {
		return;
	}
	const int hoard_count =
		std::max(inst.run_hoard_peak, static_cast<int>(inst.treasure_claimed.size()));
	if(hoard_count <= 0) {
		return;
	}
	std::vector<std::string> names;
	procarea_records_collect_names(inst, names);
	for(const std::string& name : names) {
		ProcareaRecordState state = procarea_records_load(name.c_str());
		if(hoard_count > state.best_hoard) {
			state.best_hoard = hoard_count;
			procarea_records_store(name.c_str(), state);
		}
	}
}

int procarea_records_best_eff_band_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_rec_best_eff_band);
}

int procarea_records_best_power_centi_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_rec_best_power_centi);
}

int procarea_records_fast_day_sec_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	if(ch->specials.procarea_rec_fast_day_id != procarea_records_day_id()) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_rec_fast_day_sec);
}

int procarea_records_fast_week_sec_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	if(ch->specials.procarea_rec_fast_week_id != procarea_records_week_id()) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_rec_fast_week_sec);
}

int procarea_records_captain_clears_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_rec_captain_clears);
}

int procarea_records_best_hoard_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_rec_best_hoard);
}

int procarea_records_best_sigils_get(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_rec_best_sigils);
}

int procarea_records_best_eff_band_for_name(const char* name) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_records_best_eff_band_get(ch);
	}
	return std::max(0, procarea_records_load(name).best_eff_band);
}

int procarea_records_best_power_centi_for_name(const char* name) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_records_best_power_centi_get(ch);
	}
	return std::max(0, procarea_records_load(name).best_power_centi);
}

int procarea_records_captain_clears_for_name(const char* name) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_records_captain_clears_get(ch);
	}
	return std::max(0, procarea_records_load(name).captain_clears);
}

int procarea_records_best_hoard_for_name(const char* name) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_records_best_hoard_get(ch);
	}
	return std::max(0, procarea_records_load(name).best_hoard);
}

int procarea_records_best_sigils_for_name(const char* name) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_records_best_sigils_get(ch);
	}
	return std::max(0, procarea_records_load(name).best_sigils);
}

int procarea_records_fast_day_sec_for_name(const char* name) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_records_fast_day_sec_get(ch);
	}
	ProcareaRecordState state = procarea_records_load(name);
	if(state.fast_day_id != procarea_records_day_id()) {
		return 0;
	}
	return std::max(0, state.fast_day_sec);
}

int procarea_records_fast_week_sec_for_name(const char* name) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_records_fast_week_sec_get(ch);
	}
	ProcareaRecordState state = procarea_records_load(name);
	if(state.fast_week_id != procarea_records_week_id()) {
		return 0;
	}
	return std::max(0, state.fast_week_sec);
}

namespace {

[[nodiscard]] std::string procarea_format_duration(int seconds) {
	const int mins = std::max(0, seconds) / 60;
	const int secs = std::max(0, seconds) % 60;
	char buf[16];
	std::snprintf(buf, sizeof(buf), "%d:%02d", mins, secs);
	return buf;
}

} // namespace

void procarea_send_personal_records(char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return;
	}

	std::ostringstream out;
	out << "$c0014=== I tuoi record - Dimensione Effimera ===$c0007\n\r";
	out << "Clear oggi: " << procarea_fatigue_solo_clears_for_name(GET_NAME(ch)) << " solo, "
		<< procarea_fatigue_group_clears_for_name(GET_NAME(ch)) << " gruppo\n\r";
	out << "Clear settimana: " << procarea_fatigue_solo_clears_week_for_name(GET_NAME(ch))
		<< " solo, " << procarea_fatigue_group_clears_week_for_name(GET_NAME(ch)) << " gruppo\n\r";
	out << "Clear totali: " << procarea_clears_solo_total_get(ch) << " solo, "
		<< procarea_clears_group_total_get(ch) << " gruppo\n\r";
	out << "Frammenti runa: " << procarea_rune_fragments_get(ch) << "\n\r";

	const int best_band = procarea_records_best_eff_band_get(ch);
	if(best_band > 0) {
		out << "Miglior fascia effettiva: " << (best_band + 1);
		if(ch->specials.procarea_rec_best_tmpl_band > 0) {
			out << " (template " << (ch->specials.procarea_rec_best_tmpl_band + 1) << ")";
		}
		out << "\n\r";
	} else {
		out << "Miglior fascia effettiva: -\n\r";
	}

	const int best_power = procarea_records_best_power_centi_get(ch);
	out << "Miglior potenza al custode: " << (best_power > 0 ? std::to_string(best_power) : "-")
		<< "\n\r";

	const int fast_day = procarea_records_fast_day_sec_get(ch);
	if(fast_day > 0) {
		out << "Velocita' oggi: " << procarea_format_duration(fast_day);
		if(ch->specials.procarea_rec_fast_day_band > 0) {
			out << " (fascia " << (ch->specials.procarea_rec_fast_day_band + 1);
			if(ch->specials.procarea_rec_fast_day_power > 0) {
				out << ", potenza " << ch->specials.procarea_rec_fast_day_power;
			}
			out << ')';
		}
		out << "\n\r";
	} else {
		out << "Velocita' oggi: -\n\r";
	}

	const int fast_week = procarea_records_fast_week_sec_get(ch);
	if(fast_week > 0) {
		out << "Velocita' settimana: " << procarea_format_duration(fast_week);
		if(ch->specials.procarea_rec_fast_week_band > 0) {
			out << " (fascia " << (ch->specials.procarea_rec_fast_week_band + 1);
			if(ch->specials.procarea_rec_fast_week_power > 0) {
				out << ", potenza " << ch->specials.procarea_rec_fast_week_power;
			}
			out << ')';
		}
		out << "\n\r";
	} else {
		out << "Velocita' settimana: -\n\r";
	}

	out << "Capitaneria (clear come owner): " << procarea_records_captain_clears_get(ch) << "\n\r";
	out << "Record stanze tesoro in una run: " << procarea_records_best_hoard_get(ch) << "\n\r";
	out << "Record trappole/add in una run: " << procarea_records_best_sigils_get(ch) << "\n\r";
	out << "Classifiche globali: $c0014topinstances help$c0007\n\r";

	send_to_char(out.str().c_str(), ch);
}

} // namespace Alarmud
