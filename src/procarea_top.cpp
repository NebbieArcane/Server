/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "handler.hpp"
#include "procarea.hpp"
#include "procarea_fatigue.hpp"
#include "procarea_internal.hpp"
#include "procarea_rune_fragments.hpp"
#include "procarea_records.hpp"
#include "procarea_top.hpp"
#include "reception.hpp"
#include "logging.hpp"
#if USE_MYSQL
#include "Sql.hpp"
#include <odb/mysql/database.hxx>
#include <mysql/mysql.h>
#endif
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
namespace Alarmud {

namespace {

[[nodiscard]] bool token_is(std::string_view token, std::initializer_list<std::string_view> aliases) {
	return procarea_internal::cmd_is(token, aliases);
}

[[nodiscard]] int procarea_top_clamp_limit(int limit) {
	if(limit <= 0) {
		return PROCAREA_TOP_LIMIT_DEFAULT;
	}
	return std::min(limit, PROCAREA_TOP_LIMIT_MAX);
}

[[nodiscard]] int procarea_today_solo_from_char(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	if(ch->specials.procarea_fatigue_day != procarea_fatigue_day_id()) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_fatigue_solo);
}

[[nodiscard]] int procarea_today_group_from_char(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	if(ch->specials.procarea_fatigue_day != procarea_fatigue_day_id()) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_fatigue_group);
}

[[nodiscard]] int procarea_month_solo_from_char(const char_data* ch) {
	return procarea_clears_solo_month_get(ch);
}

[[nodiscard]] int procarea_month_group_from_char(const char_data* ch) {
	return procarea_clears_group_month_get(ch);
}

[[nodiscard]] int procarea_week_solo_from_char(const char_data* ch) {
	return procarea_fatigue_solo_clears_week_get(ch);
}

[[nodiscard]] int procarea_week_group_from_char(const char_data* ch) {
	if(ch == nullptr || !IS_PC(ch)) {
		return 0;
	}
	if(ch->specials.procarea_fatigue_week != procarea_records_week_id()) {
		return 0;
	}
	return std::max(0, ch->specials.procarea_fatigue_group_week);
}

enum class ProcareaTopPeriod { Today, Monthly, Lifetime };

void procarea_top_sort_entries(std::vector<ProcareaTopEntry>& entries, bool higher_is_better) {
	std::sort(entries.begin(), entries.end(),
			  [higher_is_better](const ProcareaTopEntry& a, const ProcareaTopEntry& b) {
				  if(a.value != b.value) {
					  return higher_is_better ? a.value > b.value : a.value < b.value;
				  }
				  return a.name < b.name;
			  });
}

#if USE_MYSQL

bool procarea_top_mysql_select(const std::string& sql, MYSQL_RES*& out_res) {
	out_res = nullptr;
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return false;
	}
	odb::connection_ptr cp(db->connection());
	auto& mc = static_cast<odb::mysql::connection&>(*cp);
	MYSQL* h = mc.handle();
	if(mysql_query(h, sql.c_str()) != 0) {
		mudlog(LOG_SYSERR, "procarea_top_mysql_select: %s", mysql_error(h));
		return false;
	}
	out_res = mysql_store_result(h);
	return out_res != nullptr;
}

[[nodiscard]] std::string procarea_top_sql_pref_key(ProcareaTopKind kind) {
	switch(kind) {
	case ProcareaTopKind::TodaySolo:
	case ProcareaTopKind::TodayGroup:
	case ProcareaTopKind::TodayAll:
		return "procarea_ftg";
	case ProcareaTopKind::MonthlySolo:
	case ProcareaTopKind::MonthlyGroup:
	case ProcareaTopKind::MonthlyAll:
		return "procarea_clr_mo";
	case ProcareaTopKind::WeekSolo:
	case ProcareaTopKind::WeekGroup:
	case ProcareaTopKind::WeekAll:
		return "procarea_ftg_wk";
	case ProcareaTopKind::LifetimeSolo:
	case ProcareaTopKind::LifetimeGroup:
	case ProcareaTopKind::LifetimeAll:
		return "procarea_clr_tot";
	case ProcareaTopKind::Fragments:
		return "procarea_rune_frg";
	case ProcareaTopKind::BestEffBand:
		return "procarea_band_mx";
	case ProcareaTopKind::BestPower:
		return "procarea_pwr_mx";
	case ProcareaTopKind::FastDay:
		return "procarea_fast_dy";
	case ProcareaTopKind::FastWeek:
		return "procarea_fast_wk";
	case ProcareaTopKind::CaptainClears:
		return "procarea_captain";
	case ProcareaTopKind::BestHoard:
		return "procarea_hoard_mx";
	case ProcareaTopKind::BestSigils:
		return "procarea_sigil_mx";
	}
	return "procarea_ftg";
}

[[nodiscard]] std::string procarea_top_sql_value_expr(ProcareaTopKind kind) {
	const int today = procarea_fatigue_day_id();
	const int month = procarea_clears_month_id();
	switch(kind) {
	case ProcareaTopKind::TodaySolo:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(today) +
			   " THEN CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 2), ':', -1) AS "
			   "SIGNED) ELSE 0 END)";
	case ProcareaTopKind::TodayGroup:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(today) +
			   " THEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED) ELSE 0 END)";
	case ProcareaTopKind::TodayAll:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(today) +
			   " THEN CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 2), ':', -1) AS "
			   "SIGNED)"
			   " + CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED) ELSE 0 END)";
	case ProcareaTopKind::MonthlySolo:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(month) +
			   " THEN CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 2), ':', -1) AS "
			   "SIGNED) ELSE 0 END)";
	case ProcareaTopKind::MonthlyGroup:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(month) +
			   " THEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED) ELSE 0 END)";
	case ProcareaTopKind::MonthlyAll:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(month) +
			   " THEN CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 2), ':', -1) AS "
			   "SIGNED)"
			   " + CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED) ELSE 0 END)";
	case ProcareaTopKind::LifetimeSolo:
		return "CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS SIGNED)";
	case ProcareaTopKind::LifetimeGroup:
		return "CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED)";
	case ProcareaTopKind::LifetimeAll:
		return "(CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS SIGNED)"
			   " + CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED))";
	case ProcareaTopKind::Fragments:
		return "CAST(cp.pref_value AS SIGNED)";
	case ProcareaTopKind::BestEffBand:
		return "CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS SIGNED)";
	case ProcareaTopKind::BestPower:
		return "CAST(cp.pref_value AS SIGNED)";
	case ProcareaTopKind::FastDay:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(procarea_records_day_id()) +
			   " THEN CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 2), ':', -1) AS "
			   "SIGNED) ELSE 0 END)";
	case ProcareaTopKind::FastWeek:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(procarea_records_week_id()) +
			   " THEN CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 2), ':', -1) AS "
			   "SIGNED) ELSE 0 END)";
	case ProcareaTopKind::WeekSolo:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(procarea_records_week_id()) +
			   " THEN CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 2), ':', -1) AS "
			   "SIGNED) ELSE 0 END)";
	case ProcareaTopKind::WeekGroup:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(procarea_records_week_id()) +
			   " THEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED) ELSE 0 END)";
	case ProcareaTopKind::WeekAll:
		return "(CASE WHEN CAST(SUBSTRING_INDEX(cp.pref_value, ':', 1) AS UNSIGNED) = " +
			   std::to_string(procarea_records_week_id()) +
			   " THEN CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 2), ':', -1) AS "
			   "SIGNED)"
			   " + CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED) ELSE 0 END)";
	case ProcareaTopKind::CaptainClears:
	case ProcareaTopKind::BestHoard:
	case ProcareaTopKind::BestSigils:
		return "CAST(cp.pref_value AS SIGNED)";
	}
	return "0";
}

[[nodiscard]] std::string procarea_top_sql_filter_clause(ProcareaTopKind kind,
														 const ProcareaTopFilter& filter) {
	if(kind != ProcareaTopKind::FastDay && kind != ProcareaTopKind::FastWeek) {
		return {};
	}
	std::ostringstream sql;
	if(filter.min_band_display > 0) {
		sql << " AND CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(cp.pref_value, ':', 3), ':', -1) AS SIGNED) >= "
			<< (filter.min_band_display - 1);
	}
	if(filter.min_power_centi > 0) {
		sql << " AND CAST(SUBSTRING_INDEX(cp.pref_value, ':', -1) AS SIGNED) >= "
			<< filter.min_power_centi;
	}
	return sql.str();
}

void procarea_top_fetch_mysql(ProcareaTopKind kind, int limit, std::vector<ProcareaTopEntry>& out,
							  const ProcareaTopFilter& filter) {
	out.clear();
	const std::string pref_key = procarea_top_sql_pref_key(kind);
	const std::string value_expr = procarea_top_sql_value_expr(kind);
	const bool higher = procarea_top_higher_is_better(kind);
	std::ostringstream sql;
	sql << "SELECT t.name, " << value_expr << " AS val "
		<< "FROM character_prefs cp "
		<< "INNER JOIN toon t ON t.id = cp.toon_id "
		<< "WHERE cp.pref_key = '" << pref_key << "' "
		<< "AND t.level < " << LOW_IMMORTAL << procarea_top_sql_filter_clause(kind, filter) << ' '
		<< "HAVING val > 0 "
		<< "ORDER BY val " << (higher ? "DESC" : "ASC") << ", t.name ASC "
		<< "LIMIT " << limit;

	MYSQL_RES* res = nullptr;
	if(!procarea_top_mysql_select(sql.str(), res) || res == nullptr) {
		return;
	}

	MYSQL_ROW row;
	while((row = mysql_fetch_row(res)) != nullptr) {
		if(row[0] == nullptr || row[1] == nullptr) {
			continue;
		}
		const int val = std::max(0, std::atoi(row[1]));
		if(val <= 0) {
			continue;
		}
		out.push_back(ProcareaTopEntry{row[0], val});
	}
	mysql_free_result(res);
}

[[nodiscard]] int procarea_top_rank_mysql(ProcareaTopKind kind, int value,
										  const ProcareaTopFilter& filter) {
	if(value <= 0) {
		return 0;
	}
	const std::string pref_key = procarea_top_sql_pref_key(kind);
	const std::string value_expr = procarea_top_sql_value_expr(kind);
	const bool higher = procarea_top_higher_is_better(kind);
	std::ostringstream sql;
	sql << "SELECT COUNT(*) + 1 "
		<< "FROM character_prefs cp "
		<< "INNER JOIN toon t ON t.id = cp.toon_id "
		<< "WHERE cp.pref_key = '" << pref_key << "' "
		<< "AND t.level < " << LOW_IMMORTAL << procarea_top_sql_filter_clause(kind, filter) << ' '
		<< "AND (" << value_expr << ") " << (higher ? ">" : "<") << ' ' << value;

	MYSQL_RES* res = nullptr;
	if(!procarea_top_mysql_select(sql.str(), res) || res == nullptr) {
		return 0;
	}
	int rank = 0;
	if(MYSQL_ROW row = mysql_fetch_row(res); row != nullptr && row[0] != nullptr) {
		rank = std::max(0, std::atoi(row[0]));
	}
	mysql_free_result(res);
	return rank;
}

#endif /* USE_MYSQL */

void procarea_top_fetch_online(ProcareaTopKind kind, int limit, std::vector<ProcareaTopEntry>& out,
							   const ProcareaTopFilter& filter) {
	out.clear();
	for(struct descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		if(d->character == nullptr || !IS_PC(d->character) ||
		   GetMaxLevel(d->character) >= LOW_IMMORTAL) {
			continue;
		}
		const char* name = GET_NAME(d->character);
		if(name == nullptr || *name == '\0') {
			continue;
		}
		int value = procarea_top_value_for_char(d->character, kind);
		if(value <= 0) {
			continue;
		}
		if(filter.min_band_display > 0 || filter.min_power_centi > 0) {
			if(kind != ProcareaTopKind::FastDay && kind != ProcareaTopKind::FastWeek) {
				continue;
			}
			const int band = kind == ProcareaTopKind::FastDay
								? d->character->specials.procarea_rec_fast_day_band
								: d->character->specials.procarea_rec_fast_week_band;
			const int power = kind == ProcareaTopKind::FastDay
								  ? d->character->specials.procarea_rec_fast_day_power
								  : d->character->specials.procarea_rec_fast_week_power;
			if(filter.min_band_display > 0 && band < filter.min_band_display - 1) {
				continue;
			}
			if(filter.min_power_centi > 0 && power < filter.min_power_centi) {
				continue;
			}
		}
		out.push_back(ProcareaTopEntry{name, value});
	}
	procarea_top_sort_entries(out, procarea_top_higher_is_better(kind));
	if(static_cast<int>(out.size()) > limit) {
		out.resize(static_cast<std::size_t>(limit));
	}
}

[[nodiscard]] int procarea_top_rank_online(const char* name, ProcareaTopKind kind, int value,
										   const ProcareaTopFilter& filter) {
	if(name == nullptr || *name == '\0' || value <= 0) {
		return 0;
	}
	std::vector<ProcareaTopEntry> all;
	for(struct descriptor_data* d = descriptor_list; d != nullptr; d = d->next) {
		if(d->character == nullptr || !IS_PC(d->character) ||
		   GetMaxLevel(d->character) >= LOW_IMMORTAL) {
			continue;
		}
		const char* entry_name = GET_NAME(d->character);
		if(entry_name == nullptr || *entry_name == '\0') {
			continue;
		}
		const int entry_value = procarea_top_value_for_char(d->character, kind);
		if(entry_value <= 0) {
			continue;
		}
		if(filter.min_band_display > 0 || filter.min_power_centi > 0) {
			if(kind != ProcareaTopKind::FastDay && kind != ProcareaTopKind::FastWeek) {
				continue;
			}
			const int band = kind == ProcareaTopKind::FastDay
								? d->character->specials.procarea_rec_fast_day_band
								: d->character->specials.procarea_rec_fast_week_band;
			const int power = kind == ProcareaTopKind::FastDay
								  ? d->character->specials.procarea_rec_fast_day_power
								  : d->character->specials.procarea_rec_fast_week_power;
			if(filter.min_band_display > 0 && band < filter.min_band_display - 1) {
				continue;
			}
			if(filter.min_power_centi > 0 && power < filter.min_power_centi) {
				continue;
			}
		}
		all.push_back(ProcareaTopEntry{entry_name, entry_value});
	}
	procarea_top_sort_entries(all, procarea_top_higher_is_better(kind));
	int rank = 1;
	const bool higher = procarea_top_higher_is_better(kind);
	for(const ProcareaTopEntry& entry : all) {
		if(higher) {
			if(entry.value > value) {
				++rank;
				continue;
			}
		} else if(entry.value < value) {
			++rank;
			continue;
		}
		if(entry.value == value && entry.name < name) {
			++rank;
		}
	}
	return rank;
}

[[nodiscard]] bool parse_period(std::string_view token, ProcareaTopKind& kind_out,
								ProcareaTopPeriod& period_out) {
	if(token_is(token, { "today" })) {
		kind_out = ProcareaTopKind::TodayAll;
		period_out = ProcareaTopPeriod::Today;
		return true;
	}
	if(token_is(token, { "monthly", "month" })) {
		kind_out = ProcareaTopKind::MonthlyAll;
		period_out = ProcareaTopPeriod::Monthly;
		return true;
	}
	if(token_is(token, { "lifetime", "total", "alltime" })) {
		kind_out = ProcareaTopKind::LifetimeAll;
		period_out = ProcareaTopPeriod::Lifetime;
		return true;
	}
	return false;
}

[[nodiscard]] bool parse_mode(std::string_view token, ProcareaTopPeriod period,
							  ProcareaTopKind& kind_out) {
	if(token_is(token, { "solo", "single" })) {
		switch(period) {
		case ProcareaTopPeriod::Today:
			kind_out = ProcareaTopKind::TodaySolo;
			break;
		case ProcareaTopPeriod::Monthly:
			kind_out = ProcareaTopKind::MonthlySolo;
			break;
		case ProcareaTopPeriod::Lifetime:
			kind_out = ProcareaTopKind::LifetimeSolo;
			break;
		}
		return true;
	}
	if(token_is(token, { "group" })) {
		switch(period) {
		case ProcareaTopPeriod::Today:
			kind_out = ProcareaTopKind::TodayGroup;
			break;
		case ProcareaTopPeriod::Monthly:
			kind_out = ProcareaTopKind::MonthlyGroup;
			break;
		case ProcareaTopPeriod::Lifetime:
			kind_out = ProcareaTopKind::LifetimeGroup;
			break;
		}
		return true;
	}
	if(token_is(token, { "all", "both", "combined" })) {
		switch(period) {
		case ProcareaTopPeriod::Today:
			kind_out = ProcareaTopKind::TodayAll;
			break;
		case ProcareaTopPeriod::Monthly:
			kind_out = ProcareaTopKind::MonthlyAll;
			break;
		case ProcareaTopPeriod::Lifetime:
			kind_out = ProcareaTopKind::LifetimeAll;
			break;
		}
		return true;
	}
	return false;
}

void procarea_top_send_help(char_data* ch) {
	std::ostringstream help;
	help << "$c0014=== Classifica Dimensioni Effimere ===$c0007\n\r"
		 << "Uso: $c0014topinstances$c0007 [categoria] [modalita] [limite]\n\r"
		 << "  $c0010today$c0007 [solo|group|all] - clear di oggi (default: all)\n\r"
		 << "  $c0010monthly$c0007 [solo|group|all] - clear del mese (default: all)\n\r"
		 << "  $c0010lifetime$c0007 [solo|group|all] - clear totali (default: all)\n\r"
		 << "  $c0010fragments$c0007 [limite] - frammenti di runa accumulati\n\r"
		 << "Record e nicchia:\n\r"
		 << "  $c0010band$c0007 - miglior fascia effettiva completata\n\r"
		 << "  $c0010power$c0007 - miglior potenza al kill del custode\n\r"
		 << "  $c0010speed today$c0007 | $c0010speed week$c0007 - clear piu' rapida\n\r"
		 << "    (opzionale: $c0010band N$c0007 o $c0010power N$c0007 dopo today/week)\n\r"
		 << "  $c0010storm$c0007 | $c0010storm week$c0007 - piu' clear solitarie oggi/settimana\n\r"
		 << "  $c0010captain$c0007 - clear di gruppo come owner\n\r"
		 << "  $c0010hoard$c0007 - piu' stanze tesoro saccheggiate in una run\n\r"
		 << "  $c0010seals$c0007 - piu' trappole/add uccisi in una run\n\r"
		 << "Limite righe: 1-" << PROCAREA_TOP_LIMIT_MAX << " (default "
		 << PROCAREA_TOP_LIMIT_DEFAULT << ").\n\r"
		 << "Esempi:\n\r"
		 << "  $c0014topinstances today solo$c0007\n\r"
		 << "  $c0014topinstances monthly group$c0007\n\r"
		 << "  $c0014topinstances lifetime group 15$c0007\n\r"
		 << "  $c0014topinstances fragments$c0007\n\r"
		 << "  $c0014topinstances band$c0007 | $c0014power$c0007 | "
		 << "$c0014speed week$c0007\n\r"
		 << "  $c0014dimensione record$c0007 - i tuoi record personali\n\r"
		 << "  $c0014topinstances storm week$c0007\n\r";
	send_to_char(help.str().c_str(), ch);
}

[[nodiscard]] bool parse_record_category(std::string_view token, ProcareaTopKind& kind_out) {
	if(token_is(token, { "band" })) {
		kind_out = ProcareaTopKind::BestEffBand;
		return true;
	}
	if(token_is(token, { "power", "pwr" })) {
		kind_out = ProcareaTopKind::BestPower;
		return true;
	}
	if(token_is(token, { "captain", "captaincy" })) {
		kind_out = ProcareaTopKind::CaptainClears;
		return true;
	}
	if(token_is(token, { "hoard" })) {
		kind_out = ProcareaTopKind::BestHoard;
		return true;
	}
	if(token_is(token, { "seals", "seal" })) {
		kind_out = ProcareaTopKind::BestSigils;
		return true;
	}
	return false;
}

[[nodiscard]] bool parse_speed_category(std::string_view token, ProcareaTopKind& kind_out) {
	if(!token_is(token, { "speed", "fast", "time" })) {
		return false;
	}
	kind_out = ProcareaTopKind::FastDay;
	return true;
}

[[nodiscard]] bool parse_speed_filter_token(std::string_view token, std::string_view arg,
											ProcareaTopFilter& filter) {
	if(token_is(token, { "band" }) && !arg.empty()) {
		const int band = std::atoi(std::string(arg).c_str());
		if(band > 0) {
			filter.min_band_display = band;
			return true;
		}
	}
	if(token_is(token, { "power", "pwr" }) && !arg.empty()) {
		const int power = std::atoi(std::string(arg).c_str());
		if(power > 0) {
			filter.min_power_centi = power;
			return true;
		}
	}
	return false;
}

} // namespace

bool procarea_top_higher_is_better(ProcareaTopKind kind) {
	switch(kind) {
	case ProcareaTopKind::FastDay:
	case ProcareaTopKind::FastWeek:
		return false;
	default:
		return true;
	}
}

std::string procarea_top_format_value(ProcareaTopKind kind, int value) {
	if(kind == ProcareaTopKind::FastDay || kind == ProcareaTopKind::FastWeek) {
		const int mins = value / 60;
		const int secs = value % 60;
		char buf[16];
		std::snprintf(buf, sizeof(buf), "%d:%02d", mins, secs);
		return buf;
	}
	if(kind == ProcareaTopKind::BestEffBand) {
		return std::to_string(value + 1);
	}
	if(kind == ProcareaTopKind::BestPower) {
		return std::to_string(value);
	}
	return std::to_string(value);
}

const char* procarea_top_kind_title(ProcareaTopKind kind) {
	switch(kind) {
	case ProcareaTopKind::TodaySolo:
		return "Clear di oggi - solitaria";
	case ProcareaTopKind::TodayGroup:
		return "Clear di oggi - gruppo";
	case ProcareaTopKind::TodayAll:
		return "Clear di oggi - totale";
	case ProcareaTopKind::MonthlySolo:
		return "Clear del mese - solitaria";
	case ProcareaTopKind::MonthlyGroup:
		return "Clear del mese - gruppo";
	case ProcareaTopKind::MonthlyAll:
		return "Clear del mese - totale";
	case ProcareaTopKind::WeekSolo:
		return "Tempesta solitaria - settimana";
	case ProcareaTopKind::WeekGroup:
		return "Clear della settimana - gruppo";
	case ProcareaTopKind::WeekAll:
		return "Clear della settimana - totale";
	case ProcareaTopKind::LifetimeSolo:
		return "Clear totali - solitaria";
	case ProcareaTopKind::LifetimeGroup:
		return "Clear totali - gruppo";
	case ProcareaTopKind::LifetimeAll:
		return "Clear totali - totale";
	case ProcareaTopKind::Fragments:
		return "Frammenti di runa degli Dei";
	case ProcareaTopKind::BestEffBand:
		return "Miglior fascia effettiva completata";
	case ProcareaTopKind::BestPower:
		return "Miglior potenza al kill del custode";
	case ProcareaTopKind::FastDay:
		return "Clear piu' rapida di oggi";
	case ProcareaTopKind::FastWeek:
		return "Clear piu' rapida della settimana";
	case ProcareaTopKind::CaptainClears:
		return "Capitaneria - clear di gruppo come owner";
	case ProcareaTopKind::BestHoard:
		return "Stanze tesoro saccheggiate in una run";
	case ProcareaTopKind::BestSigils:
		return "Cacciatore di sigilli - trappole/add in una run";
	}
	return "Classifica";
}

const char* procarea_top_kind_unit(ProcareaTopKind kind) {
	switch(kind) {
	case ProcareaTopKind::Fragments:
		return "frammenti";
	case ProcareaTopKind::BestEffBand:
		return "fascia";
	case ProcareaTopKind::BestPower:
		return "potenza";
	case ProcareaTopKind::FastDay:
	case ProcareaTopKind::FastWeek:
		return "min";
	case ProcareaTopKind::BestHoard:
		return "stanze";
	case ProcareaTopKind::BestSigils:
		return "sigilli";
	default:
		return "clear";
	}
}

int procarea_top_value_for_char(const char_data* ch, ProcareaTopKind kind) {
	switch(kind) {
	case ProcareaTopKind::TodaySolo:
		return procarea_today_solo_from_char(ch);
	case ProcareaTopKind::TodayGroup:
		return procarea_today_group_from_char(ch);
	case ProcareaTopKind::TodayAll:
		return procarea_today_solo_from_char(ch) + procarea_today_group_from_char(ch);
	case ProcareaTopKind::MonthlySolo:
		return procarea_month_solo_from_char(ch);
	case ProcareaTopKind::MonthlyGroup:
		return procarea_month_group_from_char(ch);
	case ProcareaTopKind::MonthlyAll:
		return procarea_clears_month_total_get(ch);
	case ProcareaTopKind::WeekSolo:
		return procarea_week_solo_from_char(ch);
	case ProcareaTopKind::WeekGroup:
		return procarea_week_group_from_char(ch);
	case ProcareaTopKind::WeekAll:
		return procarea_week_solo_from_char(ch) + procarea_week_group_from_char(ch);
	case ProcareaTopKind::LifetimeSolo:
		return procarea_clears_solo_total_get(ch);
	case ProcareaTopKind::LifetimeGroup:
		return procarea_clears_group_total_get(ch);
	case ProcareaTopKind::LifetimeAll:
		return procarea_clears_total_get(ch);
	case ProcareaTopKind::Fragments:
		return procarea_rune_fragments_get(ch);
	case ProcareaTopKind::BestEffBand:
		return procarea_records_best_eff_band_get(ch);
	case ProcareaTopKind::BestPower:
		return procarea_records_best_power_centi_get(ch);
	case ProcareaTopKind::FastDay:
		return procarea_records_fast_day_sec_get(ch);
	case ProcareaTopKind::FastWeek:
		return procarea_records_fast_week_sec_get(ch);
	case ProcareaTopKind::CaptainClears:
		return procarea_records_captain_clears_get(ch);
	case ProcareaTopKind::BestHoard:
		return procarea_records_best_hoard_get(ch);
	case ProcareaTopKind::BestSigils:
		return procarea_records_best_sigils_get(ch);
	}
	return 0;
}

int procarea_top_value_for_name(const char* name, ProcareaTopKind kind) {
	if(name == nullptr || *name == '\0') {
		return 0;
	}
	if(char_data* ch = get_char(name); ch != nullptr && IS_PC(ch)) {
		return procarea_top_value_for_char(ch, kind);
	}

	switch(kind) {
	case ProcareaTopKind::TodaySolo:
		return procarea_fatigue_solo_clears_for_name(name);
	case ProcareaTopKind::TodayGroup:
		return procarea_fatigue_group_clears_for_name(name);
	case ProcareaTopKind::TodayAll:
		return procarea_fatigue_solo_clears_for_name(name) +
			   procarea_fatigue_group_clears_for_name(name);
	case ProcareaTopKind::MonthlySolo:
		return procarea_clears_solo_month_for_name(name);
	case ProcareaTopKind::MonthlyGroup:
		return procarea_clears_group_month_for_name(name);
	case ProcareaTopKind::MonthlyAll:
		return procarea_clears_solo_month_for_name(name) +
			   procarea_clears_group_month_for_name(name);
	case ProcareaTopKind::WeekSolo:
		return procarea_fatigue_solo_clears_week_for_name(name);
	case ProcareaTopKind::WeekGroup:
		return procarea_fatigue_group_clears_week_for_name(name);
	case ProcareaTopKind::WeekAll:
		return procarea_fatigue_solo_clears_week_for_name(name) +
			   procarea_fatigue_group_clears_week_for_name(name);
	case ProcareaTopKind::LifetimeSolo: {
#if USE_MYSQL
		int solo = 0;
		int group = 0;
		if(procarea_clears_totals_load_mysql(name, solo, group)) {
			return std::max(0, solo);
		}
#endif
		return 0;
	}
	case ProcareaTopKind::LifetimeGroup: {
#if USE_MYSQL
		int solo = 0;
		int group = 0;
		if(procarea_clears_totals_load_mysql(name, solo, group)) {
			return std::max(0, group);
		}
#endif
		return 0;
	}
	case ProcareaTopKind::LifetimeAll: {
#if USE_MYSQL
		int solo = 0;
		int group = 0;
		if(procarea_clears_totals_load_mysql(name, solo, group)) {
			return std::max(0, solo) + std::max(0, group);
		}
#endif
		return 0;
	}
	case ProcareaTopKind::Fragments:
		return procarea_rune_fragments_get_for_name(name);
	case ProcareaTopKind::BestEffBand:
		return procarea_records_best_eff_band_for_name(name);
	case ProcareaTopKind::BestPower:
		return procarea_records_best_power_centi_for_name(name);
	case ProcareaTopKind::FastDay:
		return procarea_records_fast_day_sec_for_name(name);
	case ProcareaTopKind::FastWeek:
		return procarea_records_fast_week_sec_for_name(name);
	case ProcareaTopKind::CaptainClears:
		return procarea_records_captain_clears_for_name(name);
	case ProcareaTopKind::BestHoard:
		return procarea_records_best_hoard_for_name(name);
	case ProcareaTopKind::BestSigils:
		return procarea_records_best_sigils_for_name(name);
	}
	return 0;
}

void procarea_top_fetch(ProcareaTopKind kind, int limit, std::vector<ProcareaTopEntry>& out,
						const ProcareaTopFilter& filter) {
	limit = procarea_top_clamp_limit(limit);
#if USE_MYSQL
	if(Sql::getMysql() != nullptr) {
		procarea_top_fetch_mysql(kind, limit, out, filter);
		return;
	}
#endif
	procarea_top_fetch_online(kind, limit, out, filter);
}

int procarea_top_rank_for_name(const char* name, ProcareaTopKind kind, int value,
							   const ProcareaTopFilter& filter) {
	if(value <= 0) {
		return 0;
	}
#if USE_MYSQL
	if(Sql::getMysql() != nullptr) {
		return procarea_top_rank_mysql(kind, value, filter);
	}
#endif
	return procarea_top_rank_online(name, kind, value, filter);
}

ACTION_FUNC(do_topinstances) {
	if(!IS_PC(ch)) {
		return;
	}

	std::array<char, MAX_INPUT_LENGTH> buf{};
	const char* rest = arg;
	ProcareaTopKind kind = ProcareaTopKind::TodayAll;
	ProcareaTopPeriod period = ProcareaTopPeriod::Today;
	ProcareaTopFilter filter{};
	int limit = PROCAREA_TOP_LIMIT_DEFAULT;
	bool storm_leaderboard = false;

	rest = one_argument(rest, buf.data());
	std::string_view token = buf.data();
	if(token_is(token, { "help", "?" })) {
		procarea_top_send_help(ch);
		return;
	}

	if(token_is(token, { "fragments", "fragment", "rune" })) {
		kind = ProcareaTopKind::Fragments;
		rest = one_argument(rest, buf.data());
		if(buf[0] != '\0') {
			limit = procarea_top_clamp_limit(std::atoi(buf.data()));
		}
	} else if(parse_record_category(token, kind)) {
		rest = one_argument(rest, buf.data());
		if(buf[0] != '\0' &&
		   (std::isdigit(static_cast<unsigned char>(buf[0])) ||
			(buf[0] == '-' && std::isdigit(static_cast<unsigned char>(buf[1]))))) {
			limit = procarea_top_clamp_limit(std::atoi(buf.data()));
		}
	} else if(parse_speed_category(token, kind)) {
		kind = ProcareaTopKind::FastDay;
		rest = one_argument(rest, buf.data());
		if(buf[0] != '\0') {
			if(token_is(buf.data(), { "week", "weekly" })) {
				kind = ProcareaTopKind::FastWeek;
				rest = one_argument(rest, buf.data());
			} else if(token_is(buf.data(), { "today", "day", "daily" })) {
				kind = ProcareaTopKind::FastDay;
				rest = one_argument(rest, buf.data());
			}
		}
		while(buf[0] != '\0') {
			if(std::isdigit(static_cast<unsigned char>(buf[0])) ||
			   (buf[0] == '-' && std::isdigit(static_cast<unsigned char>(buf[1])))) {
				limit = procarea_top_clamp_limit(std::atoi(buf.data()));
				rest = one_argument(rest, buf.data());
				continue;
			}
			std::array<char, MAX_INPUT_LENGTH> arg{};
			const char* next = one_argument(rest, arg.data());
			if(parse_speed_filter_token(buf.data(), arg.data(), filter)) {
				rest = next;
				rest = one_argument(rest, buf.data());
				continue;
			}
			break;
		}
	} else if(token_is(token, { "storm", "tempesta" })) {
		storm_leaderboard = true;
		kind = ProcareaTopKind::TodaySolo;
		rest = one_argument(rest, buf.data());
		if(buf[0] != '\0') {
			if(token_is(buf.data(), { "week", "weekly" })) {
				kind = ProcareaTopKind::WeekSolo;
				rest = one_argument(rest, buf.data());
			}
		}
		if(buf[0] != '\0' &&
		   (std::isdigit(static_cast<unsigned char>(buf[0])) ||
			(buf[0] == '-' && std::isdigit(static_cast<unsigned char>(buf[1]))))) {
			limit = procarea_top_clamp_limit(std::atoi(buf.data()));
		}
	} else if(token.empty()) {
		kind = ProcareaTopKind::TodayAll;
		period = ProcareaTopPeriod::Today;
	} else {
		if(!parse_period(token, kind, period)) {
			send_to_char(
				"Categoria non riconosciuta. Prova $c0010today$c0007, $c0010monthly$c0007, "
				"$c0010lifetime$c0007, $c0010fragments$c0007, $c0010band$c0007, $c0010power$c0007, "
				"$c0010speed$c0007, $c0010storm$c0007, $c0010storm week$c0007, $c0010captain$c0007, "
				"$c0010hoard$c0007, $c0010seals$c0007 o $c0014topinstances help$c0007.\n\r",
				ch);
			return;
		}

		rest = one_argument(rest, buf.data());
		if(buf[0] != '\0') {
			if(parse_mode(buf.data(), period, kind)) {
				rest = one_argument(rest, buf.data());
			} else if(std::isdigit(static_cast<unsigned char>(buf[0])) ||
					  (buf[0] == '-' && std::isdigit(static_cast<unsigned char>(buf[1])))) {
				limit = procarea_top_clamp_limit(std::atoi(buf.data()));
				buf[0] = '\0';
			} else {
				send_to_char("Modalita' non riconosciuta: usa solo, group o all.\n\r", ch);
				return;
			}
		}

		if(buf[0] != '\0' &&
		   (std::isdigit(static_cast<unsigned char>(buf[0])) ||
			(buf[0] == '-' && std::isdigit(static_cast<unsigned char>(buf[1]))))) {
			limit = procarea_top_clamp_limit(std::atoi(buf.data()));
		}
	}

	limit = procarea_top_clamp_limit(limit);
	std::vector<ProcareaTopEntry> entries;
	procarea_top_fetch(kind, limit, entries, filter);

	std::ostringstream out;
	const char* title = procarea_top_kind_title(kind);
	if(storm_leaderboard && kind == ProcareaTopKind::TodaySolo) {
		title = "Tempesta solitaria - oggi";
	}
	out << "$c0014=== " << title;
	if(filter.min_band_display > 0 || filter.min_power_centi > 0) {
		out << " (";
		bool first = true;
		if(filter.min_band_display > 0) {
			out << "fascia >= " << filter.min_band_display;
			first = false;
		}
		if(filter.min_power_centi > 0) {
			if(!first) {
				out << ", ";
			}
			out << "potenza >= " << filter.min_power_centi;
		}
		out << ')';
	}
	out << " ===$c0007 (top " << limit << ")\n\r";
	if(entries.empty()) {
		out << "Nessun risultato.\n\r";
	} else {
		int place = 1;
		for(const ProcareaTopEntry& entry : entries) {
			const bool self = GET_NAME(ch) != nullptr && entry.name == GET_NAME(ch);
			if(self) {
				out << "$c0011";
			}
			out << " " << place << ". " << entry.name << " - "
				<< procarea_top_format_value(kind, entry.value) << ' '
				<< procarea_top_kind_unit(kind);
			if(self) {
				out << "$c0007";
			}
			out << "\n\r";
			++place;
		}
	}

	if(GetMaxLevel(ch) < LOW_IMMORTAL) {
		const int my_value = procarea_top_value_for_char(ch, kind);
		if(my_value <= 0) {
			out << "Tu: non in classifica (0 " << procarea_top_kind_unit(kind) << ").\n\r";
		} else {
			bool listed = false;
			for(const ProcareaTopEntry& entry : entries) {
				if(entry.name == GET_NAME(ch)) {
					listed = true;
					break;
				}
			}
			if(!listed) {
				const int rank = procarea_top_rank_for_name(GET_NAME(ch), kind, my_value, filter);
				if(rank > 0) {
					out << "Tu: " << rank << "° con "
						<< procarea_top_format_value(kind, my_value) << ' '
						<< procarea_top_kind_unit(kind) << ".\n\r";
				} else {
					out << "Tu: " << procarea_top_format_value(kind, my_value) << ' '
						<< procarea_top_kind_unit(kind) << ".\n\r";
				}
			}
		}
	}

#if !USE_MYSQL
	out << "$c0013(Nota: classifica parziale, solo PG collegati.)$c0007\n\r";
#endif

	send_to_char(out.str().c_str(), ch);
}

} // namespace Alarmud
