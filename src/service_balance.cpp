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
#include "configure.hpp"
#include "service_balance.hpp"
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

DerentBalanceConfig g_derent{};
DisincantaBalanceConfig g_disincanta{};

using KeyMap = std::unordered_map<std::string, std::string>;

[[nodiscard]] bool iequals(std::string_view a, std::string_view b) {
	return a.size() == b.size() &&
		   std::equal(a.begin(), a.end(), b.begin(), [](char l, char r) {
			   return std::tolower(static_cast<unsigned char>(l)) ==
					  std::tolower(static_cast<unsigned char>(r));
		   });
}

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

void sanitize_derent(DerentBalanceConfig& c) {
	c.rune = std::clamp(c.rune, 0, 100);
	c.exp = std::clamp(c.exp, 0L, 100000000L);
	c.gold = std::clamp(c.gold, 0L, 100000000L);
}

void sanitize_disincanta(DisincantaBalanceConfig& c) {
	c.rune = std::clamp(c.rune, 0, 100);
	c.exp = std::clamp(c.exp, 0L, 100000000L);
	c.coins = std::clamp(c.coins, 0L, 100000000L);
}

void export_map(KeyMap& map) {
	map.clear();
	map["derent_rune"] = std::to_string(g_derent.rune);
	map["derent_exp"] = std::to_string(g_derent.exp);
	map["derent_gold"] = std::to_string(g_derent.gold);
	map["disincanta_rune"] = std::to_string(g_disincanta.rune);
	map["disincanta_exp"] = std::to_string(g_disincanta.exp);
	map["disincanta_coins"] = std::to_string(g_disincanta.coins);
}

void apply_key_long(std::string_view key_in, long lv) {
	const std::string key = ascii_lower(key_in);
	if(key == "derent_rune") {
		g_derent.rune = static_cast<int>(lv);
	} else if(key == "derent_exp") {
		g_derent.exp = lv;
	} else if(key == "derent_gold") {
		g_derent.gold = lv;
	} else if(key == "disincanta_rune") {
		g_disincanta.rune = static_cast<int>(lv);
	} else if(key == "disincanta_exp") {
		g_disincanta.exp = lv;
	} else if(key == "disincanta_coins") {
		g_disincanta.coins = lv;
	}
}

#if USE_MYSQL
constexpr const char* kEnsureTableSql =
	"CREATE TABLE IF NOT EXISTS `service_balance` ("
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
		mudlog(LOG_SYSERR, "service_balance ensure table: %s", e.what());
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
		if(mysql_query(h, "SELECT conf_key, conf_value FROM service_balance") != 0) {
			mudlog(LOG_SYSERR, "service_balance load: %s", mysql_error(h));
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
			/* Solo chiavi canoniche da DB (niente alias ambigui). */
			const std::string_view k = row[0];
			const std::string_view v = row[1];
			long lv = 0;
			const auto [p, ec] = std::from_chars(v.data(), v.data() + v.size(), lv);
			if(ec != std::errc() || p != v.data() + v.size()) {
				continue;
			}
			if(k == "derent_rune") {
				g_derent.rune = static_cast<int>(lv);
			} else if(k == "derent_exp") {
				g_derent.exp = lv;
			} else if(k == "derent_gold") {
				g_derent.gold = lv;
			} else if(k == "disincanta_rune") {
				g_disincanta.rune = static_cast<int>(lv);
			} else if(k == "disincanta_exp") {
				g_disincanta.exp = lv;
			} else if(k == "disincanta_coins") {
				g_disincanta.coins = lv;
			} else {
				continue;
			}
			++loaded;
		}
		mysql_free_result(res);
		t.commit();
		sanitize_derent(g_derent);
		sanitize_disincanta(g_disincanta);
		if(loaded > 0) {
			mudlog(LOG_CHECK, "service_balance: loaded %d keys from DB", loaded);
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "service_balance load: %s", e.what());
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
		db->execute("DELETE FROM service_balance");
		for(const auto& kv : map) {
			const std::string ek = sql_escape(h, kv.first);
			const std::string ev = sql_escape(h, kv.second);
			std::ostringstream ins;
			ins << "INSERT INTO service_balance (conf_key, conf_value, updated_at) VALUES ('"
				<< ek << "','" << ev << "',UTC_TIMESTAMP())";
			db->execute(ins.str().c_str());
		}
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "service_balance save: %s", e.what());
	}
}
#endif

void dump_derent(char_data* ch) {
	const DerentBalanceConfig& c = g_derent;
	std::ostringstream os;
	os << "$c0014=== Derent (RentEditor) — prezzi ===$c0007\n\r"
	   << cfg_kv("rune", c.rune) << cfg_sep << cfg_kv("exp", c.exp) << cfg_sep
	   << cfg_kv("gold", c.gold) << "\n\r"
	   << "Uso: $c0014configure derent set <chiave> <val>$c0007 | "
		  "$c0014configure derent reset$c0007 | $c0014configure derent help$c0007\n\r";
	send_to_char(os.str().c_str(), ch);
}

void dump_disincanta(char_data* ch) {
	const DisincantaBalanceConfig& c = g_disincanta;
	std::ostringstream os;
	os << "$c0014=== Disincanta (AffectSlotRemover) — prezzi ===$c0007\n\r"
	   << cfg_kv("rune", c.rune) << cfg_sep << cfg_kv("exp", c.exp) << cfg_sep
	   << cfg_kv("coins", c.coins) << "\n\r"
	   << "Uso: $c0014configure disincanta set <chiave> <val>$c0007 | "
		  "$c0014configure disincanta reset$c0007 | "
		  "$c0014configure disincanta help$c0007\n\r";
	send_to_char(os.str().c_str(), ch);
}

void help_derent(char_data* ch) {
	send_to_char(
		"$c0014configure derent$c0007 - mostra prezzi RentEditor\n\r"
		"$c0014configure derent set <chiave> <val>$c0007\n\r"
		"$c0014configure derent reset$c0007 - default (1 runa / 2M exp / 300k gold)\n\r"
		"Chiavi: rune exp gold\n\r"
		"Valori: cifre, punti migliaia o k/m (es. 300k, 2m).\n\r"
		"Il tipo di pagamento (rune/exp/gold) resta sullo special del mob.\n\r",
		ch);
}

void help_disincanta(char_data* ch) {
	send_to_char(
		"$c0014configure disincanta$c0007 - mostra prezzi AffectSlotRemover\n\r"
		"$c0014configure disincanta set <chiave> <val>$c0007\n\r"
		"$c0014configure disincanta reset$c0007 - default (1 runa / 0 exp / 300k coins)\n\r"
		"Chiavi: rune exp coins\n\r"
		"Valori: cifre, punti migliaia o k/m (es. 300k, 2m).\n\r",
		ch);
}

void persist_service_balance() {
	sanitize_derent(g_derent);
	sanitize_disincanta(g_disincanta);
#if USE_MYSQL
	KeyMap map;
	export_map(map);
	save_map_to_db(map);
#endif
}

} // namespace

void service_balance_reset_derent() {
	g_derent = DerentBalanceConfig{};
	sanitize_derent(g_derent);
}

void service_balance_reset_disincanta() {
	g_disincanta = DisincantaBalanceConfig{};
	sanitize_disincanta(g_disincanta);
}

void service_balance_save() {
	persist_service_balance();
}

void service_balance_boot() {
	service_balance_reset_derent();
	service_balance_reset_disincanta();
#if USE_MYSQL
	load_from_db();
#endif
	mudlog(LOG_CHECK,
		   "service_balance: ready (derent rune=%d exp=%ld gold=%ld; "
		   "disincanta rune=%d exp=%ld coins=%ld)",
		   g_derent.rune, g_derent.exp, g_derent.gold, g_disincanta.rune,
		   g_disincanta.exp, g_disincanta.coins);
}

const DerentBalanceConfig& derent_balance_config() {
	return g_derent;
}

const DisincantaBalanceConfig& disincanta_balance_config() {
	return g_disincanta;
}

void derent_balance_wiz(char_data* ch, const char* arg) {
	if(ch == nullptr) {
		return;
	}
	if(!is_balance_wiz(ch)) {
		send_to_char("Solo gli immortali possono modificare i prezzi derent.\n\r", ch);
		return;
	}
	const ArgSplit a = split_arg(arg != nullptr ? arg : "");
	if(a.first.empty()) {
		dump_derent(ch);
		return;
	}
	if(iequals(a.first, "help") || iequals(a.first, "?")) {
		help_derent(ch);
		return;
	}
	if(iequals(a.first, "reset")) {
		service_balance_reset_derent();
		persist_service_balance();
		send_to_char("derent: reset ai default e salvato.\n\r", ch);
		mudlog(LOG_CHECK, "service_balance: %s reset derent", GET_NAME(ch));
		dump_derent(ch);
		return;
	}
	if(iequals(a.first, "set")) {
		const ArgSplit b = split_arg(a.rest);
		const ArgSplit c = split_arg(b.rest);
		if(b.first.empty() || c.first.empty()) {
			send_to_char("Uso: configure derent set <chiave> <valore>\n\r", ch);
			return;
		}
		const std::string key = ascii_lower(b.first);
		std::string canon;
		if(key == "rune") {
			canon = "derent_rune";
		} else if(key == "exp") {
			canon = "derent_exp";
		} else if(key == "gold" || key == "monete") {
			canon = "derent_gold";
		} else if(key == "derent_rune" || key == "derent_exp" || key == "derent_gold") {
			canon = key;
		}
		if(canon.empty()) {
			send_to_char("Chiave sconosciuta (rune|exp|gold). Vedi: configure derent help\n\r",
						 ch);
			return;
		}
		long lv = 0;
		if(!configure_parse_long(c.first, lv) || lv < 0) {
			configure_tell_bad_amount(ch);
			return;
		}
		const std::string key_before(b.first);
		const std::string val_before(c.first);
		apply_key_long(canon, lv);
		sanitize_derent(g_derent);
		persist_service_balance();
		send_to_char("derent: aggiornato e salvato.\n\r", ch);
		mudlog(LOG_CHECK, "service_balance: %s derent set %s=%s", GET_NAME(ch),
			   key_before.c_str(), val_before.c_str());
		dump_derent(ch);
		return;
	}
	help_derent(ch);
}

void disincanta_balance_wiz(char_data* ch, const char* arg) {
	if(ch == nullptr) {
		return;
	}
	if(!is_balance_wiz(ch)) {
		send_to_char("Solo gli immortali possono modificare i prezzi disincanta.\n\r", ch);
		return;
	}
	const ArgSplit a = split_arg(arg != nullptr ? arg : "");
	if(a.first.empty()) {
		dump_disincanta(ch);
		return;
	}
	if(iequals(a.first, "help") || iequals(a.first, "?")) {
		help_disincanta(ch);
		return;
	}
	if(iequals(a.first, "reset")) {
		service_balance_reset_disincanta();
		persist_service_balance();
		send_to_char("disincanta: reset ai default e salvato.\n\r", ch);
		mudlog(LOG_CHECK, "service_balance: %s reset disincanta", GET_NAME(ch));
		dump_disincanta(ch);
		return;
	}
	if(iequals(a.first, "set")) {
		const ArgSplit b = split_arg(a.rest);
		const ArgSplit c = split_arg(b.rest);
		if(b.first.empty() || c.first.empty()) {
			send_to_char("Uso: configure disincanta set <chiave> <valore>\n\r", ch);
			return;
		}
		const std::string key = ascii_lower(b.first);
		std::string canon;
		if(key == "rune") {
			canon = "disincanta_rune";
		} else if(key == "exp") {
			canon = "disincanta_exp";
		} else if(key == "coins" || key == "coin" || key == "gold" || key == "monete") {
			canon = "disincanta_coins";
		} else if(key == "disincanta_rune" || key == "disincanta_exp" ||
				  key == "disincanta_coins") {
			canon = key;
		}
		if(canon.empty()) {
			send_to_char(
				"Chiave sconosciuta (rune|exp|coins). Vedi: configure disincanta help\n\r",
				ch);
			return;
		}
		long lv = 0;
		if(!configure_parse_long(c.first, lv) || lv < 0) {
			configure_tell_bad_amount(ch);
			return;
		}
		const std::string key_before(b.first);
		const std::string val_before(c.first);
		apply_key_long(canon, lv);
		sanitize_disincanta(g_disincanta);
		persist_service_balance();
		send_to_char("disincanta: aggiornato e salvato.\n\r", ch);
		mudlog(LOG_CHECK, "service_balance: %s disincanta set %s=%s", GET_NAME(ch),
			   key_before.c_str(), val_before.c_str());
		dump_disincanta(ch);
		return;
	}
	help_disincanta(ch);
}

} // namespace Alarmud
