/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "logging.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "utility.hpp"
#include "multiclass.hpp"
#include "configure.hpp"
#include "dispel_balance.hpp"
#include "utility.hpp"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#if USE_MYSQL
#include "Sql.hpp"
#include "odb/odb.hpp"
#include <odb/mysql/database.hxx>
#include <mysql/mysql.h>
#endif

namespace Alarmud {

namespace {

DispelBalanceConfig g_cfg{};

using KeyMap = std::unordered_map<std::string, std::string>;

[[nodiscard]] bool iequals(std::string_view a, std::string_view b) {
	return a.size() == b.size() &&
		   std::equal(a.begin(), a.end(), b.begin(), [](char l, char r) {
			   return std::tolower(static_cast<unsigned char>(l)) ==
					  std::tolower(static_cast<unsigned char>(r));
		   });
}

/** Primo token e resto (C++17, niente buffer C / one_argument). */
struct ArgSplit {
	std::string_view first;
	std::string_view rest;
};

[[nodiscard]] ArgSplit split_arg(std::string_view s) {
	const auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
	while(!s.empty() && is_space(static_cast<unsigned char>(s.front()))) {
		s.remove_prefix(1);
	}
	if(s.empty()) {
		return {};
	}
	std::size_t i = 0;
	while(i < s.size() && !is_space(static_cast<unsigned char>(s[i]))) {
		++i;
	}
	const std::string_view first = s.substr(0, i);
	std::string_view rest = s.substr(i);
	while(!rest.empty() && is_space(static_cast<unsigned char>(rest.front()))) {
		rest.remove_prefix(1);
	}
	return {first, rest};
}

[[nodiscard]] std::string ascii_lower(std::string_view s) {
	std::string out;
	out.reserve(s.size());
	for(unsigned char c : s) {
		out.push_back(static_cast<char>(std::tolower(c)));
	}
	return out;
}

[[nodiscard]] bool is_balance_wiz(char_data* ch) {
	return ch != nullptr && IS_PC(ch) && GetMaxLevel(ch) >= QUESTMASTER;
}

[[nodiscard]] int clamp_int(int v, int lo, int hi) {
	return std::clamp(v, lo, hi);
}

void parse_bool_val(std::string_view val, bool& out, bool ok_int, int iv) {
	if(iequals(val, "1") || iequals(val, "true") || iequals(val, "on") || iequals(val, "yes")) {
		out = true;
		return;
	}
	if(iequals(val, "0") || iequals(val, "false") || iequals(val, "off") || iequals(val, "no")) {
		out = false;
		return;
	}
	if(ok_int) {
		out = iv != 0;
	}
}

void sanitize(DispelBalanceConfig& c) {
	c.sp_baseline = clamp_int(c.sp_baseline, 0, 200);
	c.pct_cap = clamp_int(c.pct_cap, 0, 100);
	c.level_bonus_cap = clamp_int(c.level_bonus_cap, 0, 50);
	c.high_level = clamp_int(c.high_level, 1, 70);
	c.high_level_pct = clamp_int(c.high_level_pct, 0, 100);
	c.mono_soft_pct = clamp_int(c.mono_soft_pct, 0, 100);
	c.mono_max_bonus = clamp_int(c.mono_max_bonus, 1, DISPEL_MONO_BAND_MAX);
	/* Soglie strettamente crescenti e >= 1. */
	int prev = 0;
	for(int i = 0; i < DISPEL_MONO_BAND_MAX; ++i) {
		c.mono_t[i] = clamp_int(c.mono_t[i], prev + 1, 300);
		prev = c.mono_t[i];
	}
}

void export_map(KeyMap& map) {
	map.clear();
	map["soft"] = g_cfg.soft ? "1" : "0";
	map["sp_baseline"] = std::to_string(g_cfg.sp_baseline);
	map["pct_cap"] = std::to_string(g_cfg.pct_cap);
	map["level_bonus_cap"] = std::to_string(g_cfg.level_bonus_cap);
	map["high_level"] = std::to_string(g_cfg.high_level);
	map["high_level_pct"] = std::to_string(g_cfg.high_level_pct);
	map["mono_band"] = g_cfg.mono_band ? "1" : "0";
	map["mono_soft_pct"] = std::to_string(g_cfg.mono_soft_pct);
	map["mono_max_bonus"] = std::to_string(g_cfg.mono_max_bonus);
	map["mono_t1"] = std::to_string(g_cfg.mono_t[0]);
	map["mono_t2"] = std::to_string(g_cfg.mono_t[1]);
	map["mono_t3"] = std::to_string(g_cfg.mono_t[2]);
	map["mono_t4"] = std::to_string(g_cfg.mono_t[3]);
}

[[nodiscard]] bool apply_key(std::string_view key_in, std::string_view val) {
	/* one_argument lowercasava i token: allineiamo le chiavi wiz. */
	const std::string key = ascii_lower(key_in);
	int iv = 0;
	const bool ok_int = configure_parse_int(val, iv);

	if(key == "soft") {
		parse_bool_val(val, g_cfg.soft, ok_int, iv);
		return true;
	}
	if(key == "mono_band") {
		parse_bool_val(val, g_cfg.mono_band, ok_int, iv);
		return true;
	}
	if(!ok_int) {
		return false;
	}
	if(key == "sp_baseline") {
		g_cfg.sp_baseline = iv;
	} else if(key == "pct_cap") {
		g_cfg.pct_cap = iv;
	} else if(key == "level_bonus_cap") {
		g_cfg.level_bonus_cap = iv;
	} else if(key == "high_level") {
		g_cfg.high_level = iv;
	} else if(key == "high_level_pct") {
		g_cfg.high_level_pct = iv;
	} else if(key == "mono_soft_pct") {
		g_cfg.mono_soft_pct = iv;
	} else if(key == "mono_max_bonus") {
		g_cfg.mono_max_bonus = iv;
	} else if(key == "mono_t1") {
		g_cfg.mono_t[0] = iv;
	} else if(key == "mono_t2") {
		g_cfg.mono_t[1] = iv;
	} else if(key == "mono_t3") {
		g_cfg.mono_t[2] = iv;
	} else if(key == "mono_t4") {
		g_cfg.mono_t[3] = iv;
	} else {
		return false;
	}
	return true;
}

#if USE_MYSQL
constexpr const char* kEnsureTableSql =
	"CREATE TABLE IF NOT EXISTS `spell_balance` ("
	"  `conf_key` varchar(64) NOT NULL PRIMARY KEY,"
	"  `conf_value` varchar(64) NOT NULL,"
	"  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP "
	"ON UPDATE CURRENT_TIMESTAMP"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";

void ensure_table(DB* db) {
	if(db == nullptr) {
		return;
	}
	try {
		odb::transaction t(db->begin());
		db->execute(kEnsureTableSql);
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "spell_balance ensure table: %s", e.what());
	}
}

[[nodiscard]] std::string sql_escape(MYSQL* h, std::string_view s) {
	std::string out(s.size() * 2 + 1, '\0');
	const unsigned long n =
		mysql_real_escape_string(h, out.data(), s.data(), static_cast<unsigned long>(s.size()));
	out.resize(n);
	return out;
}

void load_from_db() {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	ensure_table(db);
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		if(mysql_query(h, "SELECT conf_key, conf_value FROM spell_balance") != 0) {
			mudlog(LOG_SYSERR, "spell_balance load: %s", mysql_error(h));
			t.commit();
			return;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(res == nullptr) {
			t.commit();
			return;
		}
		int loaded = 0;
		MYSQL_ROW row;
		while((row = mysql_fetch_row(res)) != nullptr) {
			if(row[0] == nullptr || row[1] == nullptr) {
				continue;
			}
			if(apply_key(row[0], row[1])) {
				++loaded;
			}
		}
		mysql_free_result(res);
		t.commit();
		sanitize(g_cfg);
		if(loaded > 0) {
			mudlog(LOG_CHECK, "spell_balance: loaded %d keys from DB", loaded);
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "spell_balance load: %s", e.what());
	}
}

void save_map_to_db(const KeyMap& map) {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	ensure_table(db);
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		db->execute("DELETE FROM spell_balance");
		for(const auto& kv : map) {
			const std::string ek = sql_escape(h, kv.first);
			const std::string ev = sql_escape(h, kv.second);
			std::ostringstream ins;
			ins << "INSERT INTO spell_balance (conf_key, conf_value, updated_at) VALUES ('"
				<< ek << "','" << ev << "',UTC_TIMESTAMP())";
			db->execute(ins.str().c_str());
		}
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "spell_balance save: %s", e.what());
	}
}
#endif

void dump_config(char_data* ch) {
	const DispelBalanceConfig& c = g_cfg;
	std::ostringstream os;
	const std::string mono_t = format_thousands(c.mono_t[0]) + "/" +
							   format_thousands(c.mono_t[1]) + "/" +
							   format_thousands(c.mono_t[2]) + "/" +
							   format_thousands(c.mono_t[3]);
	os << "$c0014=== Dispel / spellpower ===$c0007\n\r"
	   << cfg_kv("soft", c.soft ? "1" : "0") << cfg_sep
	   << cfg_kv("sp_baseline", c.sp_baseline) << cfg_sep
	   << cfg_kv("pct_cap", c.pct_cap) << cfg_sep
	   << cfg_kv("level_bonus_cap", c.level_bonus_cap) << "\n\r"
	   << cfg_kv("high_level", c.high_level) << cfg_sep
	   << cfg_kv("high_level_pct", c.high_level_pct) << "\n\r"
	   << cfg_kv("mono_band", c.mono_band ? "1" : "0") << cfg_sep
	   << cfg_kv("mono_soft_pct", c.mono_soft_pct) << cfg_sep
	   << cfg_kv("mono_max_bonus", c.mono_max_bonus) << "\n\r"
	   << cfg_kv("mono_t", mono_t)
	   << " (100% auto a +1..+4; a T-1 = soft_pct)\n\r"
	   << "Uso: $c0014configure dispel set <chiave> <val>$c0007 | "
		  "$c0014configure dispel reset$c0007 | $c0014configure dispel help$c0007\n\r"
		  "(alias: dispelconf …)\n\r";
	send_to_char(os.str().c_str(), ch);
}

void help_text(char_data* ch) {
	send_to_char(
		"$c0014configure dispel$c0007 - mostra config\n\r"
		"$c0014configure dispel set <chiave> <val>$c0007 - ritocca un parametro\n\r"
		"$c0014configure dispel reset$c0007 - default\n\r"
		"(alias: dispelconf …)\n\r"
		"Valori: cifre, punti migliaia o k/m.\n\r"
		"Seconda chance: soft, sp_baseline, pct_cap, level_bonus_cap, "
		"high_level, high_level_pct\n\r"
		"Mono fasce: mono_band, mono_soft_pct, mono_max_bonus, "
		"mono_t1..mono_t4\n\r"
		"Dettaglio: $c0014wizhelp configure dispel$c0007 | "
		"esempi: $c0014wizhelp configure dispel esempi$c0007\n\r",
		ch);
}

[[nodiscard]] int mono_threshold(int k) {
	/* k = 0 → 0; k = 1..4 → mono_t[k-1] */
	if(k <= 0) {
		return 0;
	}
	const int idx = std::clamp(k, 1, DISPEL_MONO_BAND_MAX) - 1;
	return g_cfg.mono_t[idx];
}

} // namespace

void dispel_balance_reset() {
	g_cfg = DispelBalanceConfig{};
	sanitize(g_cfg);
}

void dispel_balance_save() {
	sanitize(g_cfg);
#if USE_MYSQL
	KeyMap map;
	export_map(map);
	save_map_to_db(map);
#endif
}

void dispel_balance_boot() {
	dispel_balance_reset();
#if USE_MYSQL
	load_from_db();
#endif
	mudlog(LOG_CHECK,
		   "spell_balance: ready (soft=%d mono_band=%d t=%d/%d/%d/%d soft_pct=%d)",
		   g_cfg.soft ? 1 : 0, g_cfg.mono_band ? 1 : 0, g_cfg.mono_t[0], g_cfg.mono_t[1],
		   g_cfg.mono_t[2], g_cfg.mono_t[3], g_cfg.mono_soft_pct);
}

const DispelBalanceConfig& dispel_balance_config() {
	return g_cfg;
}

DispelBalanceConfig& dispel_balance_config_mut() {
	return g_cfg;
}

bool DispelMonoBandPath(struct char_data* ch) {
	if(ch == nullptr || !g_cfg.mono_band) {
		return false;
	}
	if(!IS_CASTER_N(ch) || HowManyClasses(ch) > 1) {
		return false;
	}
	return true;
}

int DispelMonoBandUnlocked(struct char_data* ch) {
	if(!DispelMonoBandPath(ch)) {
		return 0;
	}
	const int sp = SpellpowerTotal(ch);
	const int max_b = g_cfg.mono_max_bonus;
	int unlocked = 0;
	for(int k = 1; k <= max_b; ++k) {
		if(sp >= mono_threshold(k)) {
			unlocked = k;
		} else {
			break;
		}
	}
	return unlocked;
}

int DispelMonoBandAutoChance(struct char_data* ch, struct char_data* victim) {
	if(!DispelMonoBandPath(ch) || victim == nullptr) {
		return 0;
	}
	const int need = GetMaxLevel(victim) - GetMaxLevel(ch);
	if(need <= 0) {
		return 100;
	}
	if(need > g_cfg.mono_max_bonus) {
		return 0;
	}
	const int sp = SpellpowerTotal(ch);
	const int t_hi = mono_threshold(need);
	const int t_lo = mono_threshold(need - 1);
	if(sp >= t_hi) {
		return 100;
	}
	if(sp <= t_lo) {
		return 0;
	}
	/* Rampa fino a soft_pct a (t_hi - 1). */
	const int soft_at = t_hi - 1;
	int denom = soft_at - t_lo;
	if(denom < 1) {
		denom = 1;
	}
	int chance = (g_cfg.mono_soft_pct * (sp - t_lo)) / denom;
	if(chance < 0) {
		chance = 0;
	}
	if(chance > g_cfg.mono_soft_pct) {
		chance = g_cfg.mono_soft_pct;
	}
	return chance;
}

bool DispelMonoBandAutoRoll(struct char_data* ch, struct char_data* victim) {
	const int chance = DispelMonoBandAutoChance(ch, victim);
	if(chance <= 0) {
		return false;
	}
	if(chance >= 100) {
		return true;
	}
	return number(1, 100) <= chance;
}

ACTION_FUNC(do_dispelconf) {
	if(ch == nullptr) {
		return;
	}
	if(!is_balance_wiz(ch)) {
		send_to_char("Solo gli immortali possono modificare il balance del dispel.\n\r", ch);
		return;
	}
	const ArgSplit a = split_arg(arg != nullptr ? arg : "");
	if(a.first.empty()) {
		dump_config(ch);
		return;
	}
	if(iequals(a.first, "help") || iequals(a.first, "?")) {
		help_text(ch);
		return;
	}
	if(iequals(a.first, "reset")) {
		dispel_balance_reset();
		dispel_balance_save();
		send_to_char("spell_balance: reset ai default e salvato.\n\r", ch);
		mudlog(LOG_CHECK, "spell_balance: %s reset", GET_NAME(ch));
		return;
	}
	if(iequals(a.first, "set")) {
		const ArgSplit b = split_arg(a.rest);
		const ArgSplit c = split_arg(b.rest);
		if(b.first.empty() || c.first.empty()) {
			send_to_char("Uso: configure dispel set <chiave> <valore>\n\r", ch);
			return;
		}
		const std::string key_before(b.first);
		const std::string val_before(c.first);
		if(!apply_key(b.first, c.first)) {
			configure_tell_bad_amount(ch);
			return;
		}
		sanitize(g_cfg);
		dispel_balance_save();
		send_to_char("spell_balance: aggiornato e salvato.\n\r", ch);
		mudlog(LOG_CHECK, "spell_balance: %s set %s=%s", GET_NAME(ch), key_before.c_str(),
			   val_before.c_str());
		dump_config(ch);
		return;
	}
	help_text(ch);
}

} // namespace Alarmud
