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
#include "edit_pool_balance.hpp"
#include "configure.hpp"
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

EditPoolBalanceConfig g_cfg{};

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

void sanitize_price(EditPoolUnitPrice& p, bool allow_unset) {
	if(allow_unset && p.xp == -1 && p.rune_num == -1 && p.rune_den == -1) {
		return;
	}
	if(allow_unset) {
		/* Parziale: solo i pezzi settati (> -1) vengono clampati. */
		if(p.xp >= 0) {
			p.xp = std::clamp(p.xp, 0L, 100000000L);
		}
		if(p.rune_num >= 0) {
			p.rune_num = std::clamp(p.rune_num, 0, 1000);
		}
		if(p.rune_den >= 0) {
			p.rune_den = std::clamp(p.rune_den, 1, 1000);
		}
		return;
	}
	p.xp = std::clamp(p.xp, 0L, 100000000L);
	p.rune_num = std::clamp(p.rune_num, 0, 1000);
	p.rune_den = std::clamp(p.rune_den, 1, 1000);
}

void sanitize_over_max(int& v) {
	if(v == -1) {
		return;
	}
	v = std::clamp(v, 0, 500);
}

void sanitize(EditPoolBalanceConfig& c) {
	c.max_hp = std::clamp(c.max_hp, 1, 500);
	c.max_mana = std::clamp(c.max_mana, 1, 500);
	c.max_move = std::clamp(c.max_move, 1, 500);
	c.max_hp_regen = std::clamp(c.max_hp_regen, 1, 200);
	c.max_mana_regen = std::clamp(c.max_mana_regen, 1, 200);
	c.max_move_regen = std::clamp(c.max_move_regen, 1, 200);
	sanitize_price(c.hp, false);
	sanitize_price(c.mana, false);
	sanitize_price(c.move, false);
	sanitize_price(c.hp_regen, false);
	sanitize_price(c.mana_regen, false);
	sanitize_price(c.move_regen, false);
	sanitize_over_max(c.max_over_hp);
	sanitize_over_max(c.max_over_mana);
	sanitize_over_max(c.max_over_move);
	sanitize_over_max(c.max_over_hp_regen);
	sanitize_over_max(c.max_over_mana_regen);
	sanitize_over_max(c.max_over_move_regen);
	sanitize_price(c.over_hp, true);
	sanitize_price(c.over_mana, true);
	sanitize_price(c.over_move, true);
	sanitize_price(c.over_hp_regen, true);
	sanitize_price(c.over_mana_regen, true);
	sanitize_price(c.over_move_regen, true);
}

void export_price(KeyMap& map, const char* prefix, const EditPoolUnitPrice& p) {
	map[std::string(prefix) + "_xp"] = std::to_string(p.xp);
	map[std::string(prefix) + "_rune_num"] = std::to_string(p.rune_num);
	map[std::string(prefix) + "_rune_den"] = std::to_string(p.rune_den);
}

void export_map(KeyMap& map) {
	map.clear();
	map["max_hp"] = std::to_string(g_cfg.max_hp);
	map["max_mana"] = std::to_string(g_cfg.max_mana);
	map["max_move"] = std::to_string(g_cfg.max_move);
	map["max_hp_regen"] = std::to_string(g_cfg.max_hp_regen);
	map["max_mana_regen"] = std::to_string(g_cfg.max_mana_regen);
	map["max_move_regen"] = std::to_string(g_cfg.max_move_regen);
	export_price(map, "hp", g_cfg.hp);
	export_price(map, "mana", g_cfg.mana);
	export_price(map, "move", g_cfg.move);
	export_price(map, "hp_regen", g_cfg.hp_regen);
	export_price(map, "mana_regen", g_cfg.mana_regen);
	export_price(map, "move_regen", g_cfg.move_regen);
	map["max_over_hp"] = std::to_string(g_cfg.max_over_hp);
	map["max_over_mana"] = std::to_string(g_cfg.max_over_mana);
	map["max_over_move"] = std::to_string(g_cfg.max_over_move);
	map["max_over_hp_regen"] = std::to_string(g_cfg.max_over_hp_regen);
	map["max_over_mana_regen"] = std::to_string(g_cfg.max_over_mana_regen);
	map["max_over_move_regen"] = std::to_string(g_cfg.max_over_move_regen);
	export_price(map, "over_hp", g_cfg.over_hp);
	export_price(map, "over_mana", g_cfg.over_mana);
	export_price(map, "over_move", g_cfg.over_move);
	export_price(map, "over_hp_regen", g_cfg.over_hp_regen);
	export_price(map, "over_mana_regen", g_cfg.over_mana_regen);
	export_price(map, "over_move_regen", g_cfg.over_move_regen);
}

[[nodiscard]] EditPoolUnitPrice* price_by_prefix(std::string_view prefix) {
	if(prefix == "hp") {
		return &g_cfg.hp;
	}
	if(prefix == "mana") {
		return &g_cfg.mana;
	}
	if(prefix == "move") {
		return &g_cfg.move;
	}
	if(prefix == "hp_regen") {
		return &g_cfg.hp_regen;
	}
	if(prefix == "mana_regen") {
		return &g_cfg.mana_regen;
	}
	if(prefix == "move_regen") {
		return &g_cfg.move_regen;
	}
	if(prefix == "over_hp") {
		return &g_cfg.over_hp;
	}
	if(prefix == "over_mana") {
		return &g_cfg.over_mana;
	}
	if(prefix == "over_move") {
		return &g_cfg.over_move;
	}
	if(prefix == "over_hp_regen") {
		return &g_cfg.over_hp_regen;
	}
	if(prefix == "over_mana_regen") {
		return &g_cfg.over_mana_regen;
	}
	if(prefix == "over_move_regen") {
		return &g_cfg.over_move_regen;
	}
	return nullptr;
}

[[nodiscard]] bool apply_key(std::string_view key_in, std::string_view val) {
	const std::string key = ascii_lower(key_in);
	long lv = 0;
	if(!configure_parse_long(val, lv)) {
		return false;
	}
	const int iv = static_cast<int>(lv);

	if(key == "max_hp") {
		g_cfg.max_hp = iv;
		return true;
	}
	if(key == "max_mana") {
		g_cfg.max_mana = iv;
		return true;
	}
	if(key == "max_move") {
		g_cfg.max_move = iv;
		return true;
	}
	if(key == "max_hp_regen") {
		g_cfg.max_hp_regen = iv;
		return true;
	}
	if(key == "max_mana_regen") {
		g_cfg.max_mana_regen = iv;
		return true;
	}
	if(key == "max_move_regen") {
		g_cfg.max_move_regen = iv;
		return true;
	}
	if(key == "max_over_hp") {
		g_cfg.max_over_hp = iv;
		return true;
	}
	if(key == "max_over_mana") {
		g_cfg.max_over_mana = iv;
		return true;
	}
	if(key == "max_over_move") {
		g_cfg.max_over_move = iv;
		return true;
	}
	if(key == "max_over_hp_regen") {
		g_cfg.max_over_hp_regen = iv;
		return true;
	}
	if(key == "max_over_mana_regen") {
		g_cfg.max_over_mana_regen = iv;
		return true;
	}
	if(key == "max_over_move_regen") {
		g_cfg.max_over_move_regen = iv;
		return true;
	}

	/* <campo>_xp | <campo>_rune_num | <campo>_rune_den (anche over_*) */
	const auto apply_price = [&](std::string_view prefix, std::string_view suffix) {
		EditPoolUnitPrice* p = price_by_prefix(prefix);
		if(p == nullptr) {
			return false;
		}
		if(suffix == "xp") {
			p->xp = lv;
			return true;
		}
		if(suffix == "rune_num") {
			p->rune_num = iv;
			return true;
		}
		if(suffix == "rune_den") {
			p->rune_den = iv;
			return true;
		}
		return false;
	};

	if(key.size() > 9 && key.compare(key.size() - 9, 9, "_rune_num") == 0) {
		return apply_price(key.substr(0, key.size() - 9), "rune_num");
	}
	if(key.size() > 9 && key.compare(key.size() - 9, 9, "_rune_den") == 0) {
		return apply_price(key.substr(0, key.size() - 9), "rune_den");
	}
	if(key.size() > 3 && key.compare(key.size() - 3, 3, "_xp") == 0) {
		return apply_price(key.substr(0, key.size() - 3), "xp");
	}
	return false;
}

#if USE_MYSQL
constexpr const char* kEnsureTableSql =
	"CREATE TABLE IF NOT EXISTS `edit_pool_balance` ("
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
		mudlog(LOG_SYSERR, "edit_pool_balance ensure table: %s", e.what());
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
		if(mysql_query(h, "SELECT conf_key, conf_value FROM edit_pool_balance") != 0) {
			mudlog(LOG_SYSERR, "edit_pool_balance load: %s", mysql_error(h));
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
			mudlog(LOG_CHECK, "edit_pool_balance: loaded %d keys from DB", loaded);
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "edit_pool_balance load: %s", e.what());
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
		db->execute("DELETE FROM edit_pool_balance");
		for(const auto& kv : map) {
			const std::string ek = sql_escape(h, kv.first);
			const std::string ev = sql_escape(h, kv.second);
			std::ostringstream ins;
			ins << "INSERT INTO edit_pool_balance (conf_key, conf_value, updated_at) VALUES ('"
				<< ek << "','" << ev << "',UTC_TIMESTAMP())";
			db->execute(ins.str().c_str());
		}
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "edit_pool_balance save: %s", e.what());
	}
}
#endif

void dump_price_line(std::ostringstream& os, const char* label,
					 const EditPoolUnitPrice& p) {
	os << cfg_k(label) << ": " << cfg_kv("xp/u", p.xp) << cfg_sep << cfg_k("rune")
	   << "=" << cfg_v(std::to_string(p.rune_num) + "/" + std::to_string(p.rune_den))
	   << "\n\r";
}

void dump_config(char_data* ch) {
	const EditPoolBalanceConfig& c = g_cfg;
	std::ostringstream os;
	os << "$c0014=== Editpool — cap ===$c0007\n\r"
	   << cfg_kv("max_hp", c.max_hp) << cfg_sep << cfg_kv("max_mana", c.max_mana)
	   << cfg_sep << cfg_kv("max_move", c.max_move) << "\n\r"
	   << cfg_kv("max_hp_regen", c.max_hp_regen) << cfg_sep
	   << cfg_kv("max_mana_regen", c.max_mana_regen) << cfg_sep
	   << cfg_kv("max_move_regen", c.max_move_regen) << "\n\r"
	   << "$c0014=== Editpool — prezzo per unita' ===$c0007\n\r";
	dump_price_line(os, "hp", c.hp);
	dump_price_line(os, "mana", c.mana);
	dump_price_line(os, "move", c.move);
	dump_price_line(os, "hp_regen", c.hp_regen);
	dump_price_line(os, "mana_regen", c.mana_regen);
	dump_price_line(os, "move_regen", c.move_regen);
	os << "$c0014=== Editpool — over (cap/costo; -1 = non configurato) ===$c0007\n\r"
	   << cfg_kv("max_over_hp", c.max_over_hp) << cfg_sep
	   << cfg_kv("max_over_mana", c.max_over_mana) << cfg_sep
	   << cfg_kv("max_over_move", c.max_over_move) << "\n\r"
	   << cfg_kv("max_over_hp_regen", c.max_over_hp_regen) << cfg_sep
	   << cfg_kv("max_over_mana_regen", c.max_over_mana_regen) << cfg_sep
	   << cfg_kv("max_over_move_regen", c.max_over_move_regen) << "\n\r";
	dump_price_line(os, "over_hp", c.over_hp);
	dump_price_line(os, "over_mana", c.over_mana);
	dump_price_line(os, "over_move", c.over_move);
	dump_price_line(os, "over_hp_regen", c.over_hp_regen);
	dump_price_line(os, "over_mana_regen", c.over_mana_regen);
	dump_price_line(os, "over_move_regen", c.over_move_regen);
	os << "Uso: $c0014configure editpool set <chiave> <val>$c0007 | "
		  "$c0014configure editpool reset$c0007 | $c0014configure editpool help$c0007\n\r";
	send_to_char(os.str().c_str(), ch);
}

void help_text(char_data* ch) {
	send_to_char(
		"$c0014configure editpool$c0007 - mostra cap, prezzi e over\n\r"
		"$c0014configure editpool set <chiave> <val>$c0007\n\r"
		"$c0014configure editpool reset$c0007 - default ufficiali "
		"(over a -1)\n\r"
		"Valori: cifre, punti migliaia o k/m (es. 1.600k, 2m, -1).\n\r"
		"Cap: max_hp max_mana max_move max_hp_regen max_mana_regen "
		"max_move_regen\n\r"
		"(alias: hp mana move hp_regen mana_regen move_regen)\n\r"
		"Prezzo: <campo>_xp <campo>_rune_num <campo>_rune_den\n\r"
		"Over cap: max_over_hp … max_over_move_regen (-1 = non configurato)\n\r"
		"Over prezzo: over_<campo>_xp|rune_num|rune_den (-1 = non "
		"configurato)\n\r"
		"  campo = hp|mana|move|hp_regen|mana_regen|move_regen\n\r"
		"Esempio: $c0014configure editpool set mana_xp 1600k$c0007\n\r"
		"         $c0014configure editpool set max_over_hp 50$c0007\n\r"
		"Dettaglio: $c0014wizhelp configure editpool$c0007\n\r",
		ch);
}

/** Alias corti cap → chiave canonica. */
[[nodiscard]] std::string canon_key(std::string_view key) {
	if(key == "hp") {
		return "max_hp";
	}
	if(key == "mana") {
		return "max_mana";
	}
	if(key == "move") {
		return "max_move";
	}
	if(key == "hp_regen" || key == "hpregen") {
		return "max_hp_regen";
	}
	if(key == "mana_regen" || key == "manaregen") {
		return "max_mana_regen";
	}
	if(key == "move_regen" || key == "moveregen") {
		return "max_move_regen";
	}
	return std::string(key);
}

} // namespace

void edit_pool_balance_reset() {
	g_cfg = EditPoolBalanceConfig{};
	sanitize(g_cfg);
}

void edit_pool_balance_save() {
	sanitize(g_cfg);
#if USE_MYSQL
	KeyMap map;
	export_map(map);
	save_map_to_db(map);
#endif
}

void edit_pool_balance_boot() {
	edit_pool_balance_reset();
#if USE_MYSQL
	load_from_db();
#endif
	mudlog(LOG_CHECK,
		   "edit_pool_balance: ready (hp=%d mana=%d move=%d regen=%d/%d/%d)",
		   g_cfg.max_hp, g_cfg.max_mana, g_cfg.max_move, g_cfg.max_hp_regen,
		   g_cfg.max_mana_regen, g_cfg.max_move_regen);
}

const EditPoolBalanceConfig& edit_pool_balance_config() {
	return g_cfg;
}

[[nodiscard]] const EditPoolUnitPrice* price_for_field(EditPoolField field,
													   bool over) noexcept {
	switch(field) {
	case EditPoolField::Hp:
		return over ? &g_cfg.over_hp : &g_cfg.hp;
	case EditPoolField::Mana:
		return over ? &g_cfg.over_mana : &g_cfg.mana;
	case EditPoolField::Move:
		return over ? &g_cfg.over_move : &g_cfg.move;
	case EditPoolField::HpRegen:
		return over ? &g_cfg.over_hp_regen : &g_cfg.hp_regen;
	case EditPoolField::ManaRegen:
		return over ? &g_cfg.over_mana_regen : &g_cfg.mana_regen;
	case EditPoolField::MoveRegen:
		return over ? &g_cfg.over_move_regen : &g_cfg.move_regen;
	}
	return nullptr;
}

bool edit_pool_balance_listino_unit(EditPoolField field, long& xp_per_unit,
									int& rune_num, int& rune_den) noexcept {
	const EditPoolUnitPrice* p = price_for_field(field, false);
	if(p == nullptr || p->rune_den <= 0) {
		return false;
	}
	xp_per_unit = p->xp;
	rune_num = p->rune_num;
	rune_den = p->rune_den;
	return true;
}

bool edit_pool_balance_over_listino_unit(EditPoolField field, long& xp_per_unit,
										 int& rune_num, int& rune_den) noexcept {
	const EditPoolUnitPrice* p = price_for_field(field, true);
	if(p == nullptr || p->xp < 0 || p->rune_num < 0 || p->rune_den <= 0) {
		return false;
	}
	xp_per_unit = p->xp;
	rune_num = p->rune_num;
	rune_den = p->rune_den;
	return true;
}

int edit_pool_balance_over_cap(EditPoolField field) noexcept {
	switch(field) {
	case EditPoolField::Hp:
		return g_cfg.max_over_hp;
	case EditPoolField::Mana:
		return g_cfg.max_over_mana;
	case EditPoolField::Move:
		return g_cfg.max_over_move;
	case EditPoolField::HpRegen:
		return g_cfg.max_over_hp_regen;
	case EditPoolField::ManaRegen:
		return g_cfg.max_over_mana_regen;
	case EditPoolField::MoveRegen:
		return g_cfg.max_over_move_regen;
	}
	return -1;
}

void edit_pool_balance_wiz(char_data* ch, const char* arg) {
	if(ch == nullptr) {
		return;
	}
	if(!is_balance_wiz(ch)) {
		send_to_char("Solo gli immortali possono modificare il balance editpool.\n\r",
					 ch);
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
		edit_pool_balance_reset();
		edit_pool_balance_save();
		send_to_char("edit_pool_balance: reset ai default e salvato.\n\r", ch);
		mudlog(LOG_CHECK, "edit_pool_balance: %s reset", GET_NAME(ch));
		dump_config(ch);
		return;
	}
	if(iequals(a.first, "set")) {
		const ArgSplit b = split_arg(a.rest);
		const ArgSplit c = split_arg(b.rest);
		if(b.first.empty() || c.first.empty()) {
			send_to_char("Uso: configure editpool set <chiave> <valore>\n\r", ch);
			return;
		}
		KeyMap probe;
		export_map(probe);
		const std::string canon = canon_key(ascii_lower(b.first));
		if(probe.find(canon) == probe.end()) {
			send_to_char("Chiave sconosciuta. Vedi: configure editpool help\n\r", ch);
			return;
		}
		const std::string key_before(b.first);
		const std::string val_before(c.first);
		if(!apply_key(canon, c.first)) {
			configure_tell_bad_amount(ch);
			return;
		}
		sanitize(g_cfg);
		edit_pool_balance_save();
		send_to_char("edit_pool_balance: aggiornato e salvato.\n\r", ch);
		mudlog(LOG_CHECK, "edit_pool_balance: %s set %s=%s", GET_NAME(ch),
			   key_before.c_str(), val_before.c_str());
		dump_config(ch);
		return;
	}
	help_text(ch);
}

} // namespace Alarmud
