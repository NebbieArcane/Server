/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "edit_listino.hpp"

#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "utils.hpp"
#include "configure.hpp"
#include "comm.hpp"
#include "obj_value.hpp"
#include "utility.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if USE_MYSQL
#include "Sql.hpp"
#include "odb/odb.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/exception.hxx>
#include <odb/mysql/connection.hxx>
#include <mysql/mysql.h>
#endif

namespace Alarmud {
namespace {

EditListinoRules g_rules{};
std::vector<EditListinoEntry> g_rows;
std::unordered_map<std::string, std::size_t> g_by_code;
/* apply_loc << 32 | apply_bit  -- apply_bit sta in 32 bit per i flag usati. */
std::unordered_map<unsigned long long, std::size_t> g_by_apply;

struct CapGroupEntry {
	std::string code;
	std::string label;
};
std::vector<CapGroupEntry> g_cap_groups;
std::unordered_set<std::string> g_unknown_logged;

void load_cap_group_defaults() {
	g_cap_groups.clear();
	for(std::size_t i = 0; i < kEditListinoCapGroupDefaultsCount; ++i) {
		const auto& d = kEditListinoCapGroupDefaults[i];
		CapGroupEntry g;
		g.code = d.code ? d.code : "";
		g.label = d.label ? d.label : "";
		if(!g.code.empty()) {
			g_cap_groups.push_back(std::move(g));
		}
	}
}

[[nodiscard]] bool is_mud_known_code(const std::string& code) {
	for(std::size_t i = 0; i < kEditListinoDefaultsCount; ++i) {
		const char* c = kEditListinoDefaults[i].code;
		if(c && code == c) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] bool usable_obj(const EditListinoEntry& e) noexcept {
	return e.mud_supported && e.enabled && e.on_obj;
}

[[nodiscard]] bool usable_pc(const EditListinoEntry& e) noexcept {
	return e.mud_supported && e.enabled && e.on_pc;
}

[[nodiscard]] std::string ascii_lower(std::string_view s) {
	std::string out;
	out.reserve(s.size());
	for(unsigned char c : s) {
		out.push_back(static_cast<char>(std::tolower(c)));
	}
	return out;
}

[[nodiscard]] unsigned long long apply_key(int loc, unsigned long bit) noexcept {
	return (static_cast<unsigned long long>(static_cast<unsigned int>(loc)) << 32) |
		   static_cast<unsigned long long>(bit);
}

[[nodiscard]] EditListinoEntry from_default(const EditListinoDefault& d) {
	EditListinoEntry e;
	e.code = d.code ? d.code : "";
	e.label = d.label ? d.label : "";
	e.category = d.category ? d.category : "";
	e.xp = d.xp;
	e.rune_num = d.rune_num;
	e.rune_den = d.rune_den > 0 ? d.rune_den : 1;
	e.sign = d.sign;
	e.cap_obj = d.cap_obj;
	e.step = d.step > 0 ? d.step : 1;
	e.on_obj = d.on_obj;
	e.on_pc = d.on_pc;
	e.cap_pc = d.cap_pc;
	e.cap_group = d.cap_group ? d.cap_group : "";
	e.over_cap = d.over_cap;
	e.over_step = d.over_step;
	e.over_rune = d.over_rune;
	e.over_xp = d.over_xp;
	e.over_gold = d.over_gold;
	e.over_inc_pct = d.over_inc_pct;
	e.apply_loc = d.apply_loc;
	e.apply_bit = d.apply_bit;
	e.resist_equiv = d.resist_equiv;
	e.resist_code = d.resist_code ? d.resist_code : "";
	e.buy_step = d.buy_step > 0 ? d.buy_step : 1;
	e.enabled = d.enabled ? 1 : 0;
	e.mud_supported = 1;
	return e;
}

void index_rows() {
	g_by_code.clear();
	g_by_apply.clear();
	for(std::size_t i = 0; i < g_rows.size(); ++i) {
		if(!g_rows[i].code.empty()) {
			g_by_code[g_rows[i].code] = i;
		}
		/* Righe solo-portale (!mud_supported) non entrano nel lookup apply:
		 * il MUD non le prezza e non le applica. */
		if(g_rows[i].mud_supported) {
			g_by_apply[apply_key(g_rows[i].apply_loc, g_rows[i].apply_bit)] = i;
		}
	}
}

void load_defaults() {
	g_rows.clear();
	g_rows.reserve(kEditListinoDefaultsCount);
	for(std::size_t i = 0; i < kEditListinoDefaultsCount; ++i) {
		g_rows.push_back(from_default(kEditListinoDefaults[i]));
	}
	index_rows();

	g_rules = EditListinoRules{};
	for(std::size_t i = 0; i < kEditListinoRulesDefaultsCount; ++i) {
		const auto& r = kEditListinoRulesDefaults[i];
		const std::string k = r.conf_key ? r.conf_key : "";
		if(k == "artifact_mult") {
			g_rules.artifact_mult_num = r.num;
			g_rules.artifact_mult_den = r.den > 0 ? r.den : 1;
		} else if(k == "class_mult_1") {
			g_rules.class_mult_1_num = r.num;
			g_rules.class_mult_1_den = r.den > 0 ? r.den : 1;
		} else if(k == "class_mult_2") {
			g_rules.class_mult_2_num = r.num;
			g_rules.class_mult_2_den = r.den > 0 ? r.den : 1;
		} else if(k == "class_mult_3") {
			g_rules.class_mult_3_num = r.num;
			g_rules.class_mult_3_den = r.den > 0 ? r.den : 1;
		} else if(k == "malus_mult") {
			g_rules.malus_mult_num = r.num;
			g_rules.malus_mult_den = r.den > 0 ? r.den : 1;
		} else if(k == "xp_split_by_classes") {
			g_rules.xp_split_by_classes = r.num;
		} else if(k == "rune_rounding") {
			g_rules.rune_rounding_ceil = r.num;
		} else if(k == "over_progression") {
			g_rules.over_progression = r.num;
		} else if(k == "broker_fee_percent") {
			g_rules.broker_fee_percent_num = r.num;
			g_rules.broker_fee_percent_den = r.den > 0 ? r.den : 1;
		} else if(k == "artifact_cost_xp") {
			g_rules.artifact_cost_xp_num = r.num;
			g_rules.artifact_cost_xp_den = r.den > 0 ? r.den : 1;
		}
	}
	load_cap_group_defaults();
}

void upsert_entry(EditListinoEntry e) {
	if(e.code.empty()) {
		return;
	}
	if(e.rune_den <= 0) {
		e.rune_den = 1;
	}
	const auto it = g_by_code.find(e.code);
	if(it != g_by_code.end()) {
		g_rows[it->second] = std::move(e);
	} else {
		g_rows.push_back(std::move(e));
	}
	index_rows();
}

void apply_rule_pair(const std::string& key, int num, int den) {
	if(den <= 0) {
		den = 1;
	}
	if(key == "artifact_mult") {
		g_rules.artifact_mult_num = num;
		g_rules.artifact_mult_den = den;
	} else if(key == "class_mult_1") {
		g_rules.class_mult_1_num = num;
		g_rules.class_mult_1_den = den;
	} else if(key == "class_mult_2") {
		g_rules.class_mult_2_num = num;
		g_rules.class_mult_2_den = den;
	} else if(key == "class_mult_3") {
		g_rules.class_mult_3_num = num;
		g_rules.class_mult_3_den = den;
	} else if(key == "malus_mult") {
		g_rules.malus_mult_num = num;
		g_rules.malus_mult_den = den;
	} else if(key == "xp_split_by_classes") {
		g_rules.xp_split_by_classes = num;
	} else if(key == "rune_rounding") {
		g_rules.rune_rounding_ceil = num;
	} else if(key == "over_progression") {
		g_rules.over_progression = num;
	} else if(key == "broker_fee_percent") {
		g_rules.broker_fee_percent_num = num;
		g_rules.broker_fee_percent_den = den;
	} else if(key == "artifact_cost_xp") {
		g_rules.artifact_cost_xp_num = num;
		g_rules.artifact_cost_xp_den = den;
	}
}

#if USE_MYSQL
[[nodiscard]] EditListinoEntry from_odb(const edit_listino& row) {
	EditListinoEntry e;
	e.code = row.code;
	e.label = row.label;
	e.category = row.category;
	e.xp = static_cast<long>(row.xp);
	e.rune_num = row.rune_num;
	e.rune_den = row.rune_den > 0 ? row.rune_den : 1;
	e.sign = row.sign;
	e.cap_obj = row.cap_obj;
	e.step = row.step > 0 ? row.step : 1;
	e.on_obj = row.on_obj;
	e.on_pc = row.on_pc;
	e.cap_pc = row.cap_pc;
	e.cap_group = row.cap_group;
	e.over_cap = row.over_cap;
	e.over_step = row.over_step;
	e.over_rune = row.over_rune;
	e.over_xp = static_cast<long>(row.over_xp);
	e.over_gold = static_cast<long>(row.over_gold);
	e.over_inc_pct = row.over_inc_pct;
	e.apply_loc = row.apply_loc;
	e.apply_bit = static_cast<unsigned long>(row.apply_bit);
	e.resist_equiv = row.resist_equiv;
	e.resist_code = row.resist_code;
	e.buy_step = row.buy_step > 0 ? row.buy_step : 1;
	e.enabled = row.enabled ? 1 : 0;
	e.mud_supported = row.mud_supported ? 1 : 0;
	return e;
}

void fill_odb(edit_listino& row, const EditListinoEntry& e,
			  const boost::posix_time::ptime& now) {
	row.code = e.code;
	row.label = e.label;
	row.category = e.category;
	row.xp = e.xp;
	row.rune_num = e.rune_num;
	row.rune_den = e.rune_den > 0 ? e.rune_den : 1;
	row.sign = static_cast<short>(e.sign);
	row.cap_obj = e.cap_obj;
	row.step = e.step > 0 ? e.step : 1;
	row.on_obj = static_cast<unsigned char>(e.on_obj ? 1 : 0);
	row.on_pc = static_cast<unsigned char>(e.on_pc ? 1 : 0);
	row.cap_pc = e.cap_pc;
	row.cap_group = e.cap_group;
	row.over_cap = e.over_cap;
	row.over_step = e.over_step;
	row.over_rune = e.over_rune;
	row.over_xp = e.over_xp;
	row.over_gold = e.over_gold;
	row.over_inc_pct = e.over_inc_pct;
	row.apply_loc = e.apply_loc;
	row.apply_bit = e.apply_bit;
	row.resist_equiv = static_cast<short>(e.resist_equiv);
	row.resist_code = e.resist_code;
	row.buy_step = static_cast<short>(e.buy_step > 0 ? e.buy_step : 1);
	row.enabled = static_cast<unsigned char>(e.enabled ? 1 : 0);
	row.mud_supported = static_cast<unsigned char>(e.mud_supported ? 1 : 0);
	row.updated_at = now;
}

void overlay_from_db() {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		int loaded = 0;
		for(const edit_listino& row : db->query<edit_listino>()) {
			upsert_entry(from_odb(row));
			++loaded;
		}
		int rules_n = 0;
		for(const edit_listino_rules& row : db->query<edit_listino_rules>()) {
			apply_rule_pair(row.conf_key, row.num, row.den);
			++rules_n;
		}
		t.commit();
		mudlog(LOG_CHECK, "edit_listino: loaded %d rows and %d rules from DB",
			   loaded, rules_n);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "edit_listino load (ODB): %s — uso i default", e.what());
	}

	/* Etichette cap_group: tabella assente o errore -> restano i default, no crash. */
	try {
		DB* db = Sql::getMysql();
		if(db != nullptr) {
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			int n = 0;
			for(const edit_listino_cap_group& row : db->query<edit_listino_cap_group>()) {
				if(row.code.empty()) {
					continue;
				}
				bool found = false;
				for(auto& g : g_cap_groups) {
					if(g.code == row.code) {
						g.label = row.label;
						found = true;
						break;
					}
				}
				if(!found) {
					g_cap_groups.push_back(CapGroupEntry{row.code, row.label});
				}
				++n;
			}
			t.commit();
			if(n > 0) {
				mudlog(LOG_CHECK, "edit_listino: loaded %d cap_group from DB", n);
			}
		}
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "edit_listino cap_group load: %s — uso i default",
			   e.what());
	}
}

bool persist_entry(const EditListinoEntry& e) {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return false;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const boost::posix_time::ptime now =
			boost::posix_time::second_clock::universal_time();
		using Q = odb::query<edit_listino>;
		auto r = db->query<edit_listino>(Q::code == e.code);
		auto it = r.begin();
		if(it != r.end()) {
			edit_listino row = *it;
			fill_odb(row, e, now);
			db->update(row);
		} else {
			edit_listino row;
			fill_odb(row, e, now);
			db->persist(row);
		}
		t.commit();
		return true;
	}
	catch(const odb::exception& ex) {
		mudlog(LOG_SYSERR, "edit_listino persist %s: %s", e.code.c_str(), ex.what());
		return false;
	}
}

bool persist_rule(const std::string& key, int num, int den) {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return false;
	}
	if(den <= 0) {
		den = 1;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const boost::posix_time::ptime now =
			boost::posix_time::second_clock::universal_time();
		using Q = odb::query<edit_listino_rules>;
		auto r = db->query<edit_listino_rules>(Q::conf_key == key);
		auto it = r.begin();
		if(it != r.end()) {
			edit_listino_rules row = *it;
			row.num = num;
			row.den = den;
			row.updated_at = now;
			db->update(row);
		} else {
			edit_listino_rules row;
			row.conf_key = key;
			row.num = num;
			row.den = den;
			row.updated_at = now;
			db->persist(row);
		}
		t.commit();
		return true;
	}
	catch(const odb::exception& ex) {
		mudlog(LOG_SYSERR, "edit_listino_rules persist %s: %s", key.c_str(),
			   ex.what());
		return false;
	}
}

bool persist_pc_defaults_and_rules() {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return false;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const boost::posix_time::ptime now =
			boost::posix_time::second_clock::universal_time();
		for(const auto& e : g_rows) {
			if(!e.on_pc) {
				continue;
			}
			using Q = odb::query<edit_listino>;
			auto r = db->query<edit_listino>(Q::code == e.code);
			auto it = r.begin();
			if(it != r.end()) {
				edit_listino row = *it;
				fill_odb(row, e, now);
				db->update(row);
			} else {
				edit_listino row;
				fill_odb(row, e, now);
				db->persist(row);
			}
		}
		auto persist_one_rule = [&](const char* key, int num, int den) {
			using Q = odb::query<edit_listino_rules>;
			auto r = db->query<edit_listino_rules>(Q::conf_key == std::string(key));
			auto it = r.begin();
			if(it != r.end()) {
				edit_listino_rules row = *it;
				row.num = num;
				row.den = den;
				row.updated_at = now;
				db->update(row);
			} else {
				edit_listino_rules row;
				row.conf_key = key;
				row.num = num;
				row.den = den;
				row.updated_at = now;
				db->persist(row);
			}
		};
		persist_one_rule("artifact_mult", g_rules.artifact_mult_num,
						 g_rules.artifact_mult_den);
		persist_one_rule("class_mult_1", g_rules.class_mult_1_num,
						 g_rules.class_mult_1_den);
		persist_one_rule("class_mult_2", g_rules.class_mult_2_num,
						 g_rules.class_mult_2_den);
		persist_one_rule("class_mult_3", g_rules.class_mult_3_num,
						 g_rules.class_mult_3_den);
		persist_one_rule("malus_mult", g_rules.malus_mult_num, g_rules.malus_mult_den);
		persist_one_rule("xp_split_by_classes", g_rules.xp_split_by_classes, 1);
		persist_one_rule("rune_rounding", g_rules.rune_rounding_ceil, 1);
		persist_one_rule("over_progression", g_rules.over_progression, 1);
		persist_one_rule("broker_fee_percent", g_rules.broker_fee_percent_num,
						 g_rules.broker_fee_percent_den);
		persist_one_rule("artifact_cost_xp", g_rules.artifact_cost_xp_num,
						 g_rules.artifact_cost_xp_den);
		t.commit();
		return true;
	}
	catch(const odb::exception& ex) {
		mudlog(LOG_SYSERR, "edit_listino reset persist: %s", ex.what());
		return false;
	}
}
#endif

[[nodiscard]] long long mul_div(long long v, int num, int den) noexcept {
	if(den <= 0) {
		den = 1;
	}
	return (v * static_cast<long long>(num)) / static_cast<long long>(den);
}

[[nodiscard]] int ceil_frac(long long num, long long den) noexcept {
	if(den <= 0) {
		den = 1;
	}
	if(num <= 0) {
		return 0;
	}
	const long long q = (num + den - 1) / den;
	if(q > 2147483647LL) {
		return 2147483647;
	}
	return static_cast<int>(q);
}

[[nodiscard]] const EditListinoEntry* find_code(const char* code) {
	if(code == nullptr || *code == '\0') {
		return nullptr;
	}
	const auto it = g_by_code.find(code);
	if(it == g_by_code.end()) {
		return nullptr;
	}
	return &g_rows[it->second];
}


#if USE_MYSQL
/* Allinea mud_supported=1 sui codici del seed, senza toccare i prezzi.
 * Solo righe ancora a 0 (voce creata dal web, poi implementata dal MUD). */
void mark_known_codes_supported_db() {
	DB* db = Sql::getMysql();
	if(db == nullptr || kEditListinoDefaultsCount == 0) {
		return;
	}
	std::ostringstream sql;
	sql << "UPDATE `edit_listino` SET `mud_supported`=1, "
		   "`updated_at`=CURRENT_TIMESTAMP "
		   "WHERE `mud_supported`=0 AND `code` IN (";
	bool first = true;
	for(std::size_t i = 0; i < kEditListinoDefaultsCount; ++i) {
		const char* c = kEditListinoDefaults[i].code;
		if(c == nullptr || *c == '\0') {
			continue;
		}
		bool ok = true;
		for(const char* p = c; *p; ++p) {
			const unsigned char ch = static_cast<unsigned char>(*p);
			if(!(std::isalnum(ch) || ch == '_')) {
				ok = false;
				break;
			}
		}
		if(!ok) {
			continue;
		}
		if(!first) {
			sql << ',';
		}
		first = false;
		sql << "'" << c << "'";
	}
	sql << ')';
	if(first) {
		return;
	}
	try {
		const unsigned long long n = db->execute(sql.str().c_str());
		if(n > 0) {
			mudlog(LOG_CHECK,
				   "edit_listino: mud_supported=1 su %llu righe gia' in DB "
				   "(prezzi invariati)",
				   n);
		}
	}
	catch(const std::exception& ex) {
		mudlog(LOG_SYSERR, "edit_listino mud_supported UPDATE: %s", ex.what());
	}
}
#endif

void log_unknown_rows() {
	for(const auto& e : g_rows) {
		if(e.mud_supported || e.code.empty()) {
			continue;
		}
		if(g_unknown_logged.insert(e.code).second) {
			mudlog(LOG_CHECK,
				   "edit_listino: riga non supportata dal MUD "
				   "(non applicata, non prezzata, lasciata disabilitata): %s",
				   e.code.c_str());
		}
	}
}

void sync_mud_supported() {
#if USE_MYSQL
	mark_known_codes_supported_db();
#endif
	for(auto& e : g_rows) {
		e.mud_supported = is_mud_known_code(e.code) ? 1 : 0;
	}
	index_rows();
	log_unknown_rows();
}

void finish_load() {
	sync_mud_supported();
}

} // namespace

void edit_listino_boot() {
	load_defaults();
#if USE_MYSQL
	overlay_from_db();
#endif
	finish_load();
	mudlog(LOG_CHECK, "edit_listino: ready (%zu rows)", g_rows.size());
}

void edit_listino_reload() {
	load_defaults();
#if USE_MYSQL
	overlay_from_db();
#endif
	finish_load();
	mudlog(LOG_CHECK, "edit_listino: reloaded (%zu rows)", g_rows.size());
}

const EditListinoRules& edit_listino_rules_cache() {
	if(g_rows.empty()) {
		load_defaults();
	}
	return g_rules;
}

std::pair<int, int> edit_listino_broker_fee_frac() noexcept {
	const auto& r = edit_listino_rules_cache();
	if(r.broker_fee_percent_den <= 0) {
		return {25, 1};
	}
	return {r.broker_fee_percent_num, r.broker_fee_percent_den};
}


std::pair<int, int> edit_listino_class_mult_frac(int n_classes) noexcept {
	const auto& r = edit_listino_rules_cache();
	if(n_classes >= 3) {
		return {r.class_mult_3_num, r.class_mult_3_den};
	}
	if(n_classes == 2) {
		return {r.class_mult_2_num, r.class_mult_2_den};
	}
	return {r.class_mult_1_num, r.class_mult_1_den};
}

double edit_listino_class_mult(int n_classes) noexcept {
	const auto [n, d] = edit_listino_class_mult_frac(n_classes);
	if(d <= 0) {
		return 1.0;
	}
	return static_cast<double>(n) / static_cast<double>(d);
}

const EditListinoEntry* edit_listino_find(const char* code) {
	if(g_rows.empty()) {
		load_defaults();
	}
	return find_code(code);
}

const EditListinoEntry* edit_listino_find_apply(int apply_loc, unsigned long apply_bit) {
	if(g_rows.empty()) {
		load_defaults();
	}
	const auto it = g_by_apply.find(apply_key(apply_loc, apply_bit));
	if(it == g_by_apply.end()) {
		return nullptr;
	}
	return &g_rows[it->second];
}

std::vector<const EditListinoEntry*> edit_listino_rows_for_apply(int apply_loc) {
	if(g_rows.empty()) {
		load_defaults();
	}
	std::vector<const EditListinoEntry*> out;
	for(const auto& e : g_rows) {
		if(e.apply_loc == apply_loc && e.apply_bit != 0 && usable_obj(e)) {
			out.push_back(&e);
		}
	}
	return out;
}

const std::vector<EditListinoEntry>& edit_listino_all_rows() {
	if(g_rows.empty()) {
		load_defaults();
	}
	return g_rows;
}

bool edit_listino_is_pc_code(const char* code) {
	const auto* e = edit_listino_find(code);
	return e != nullptr && usable_pc(*e);
}

int edit_listino_cap_pc(const char* code) noexcept {
	const auto* e = edit_listino_find(code);
	return e != nullptr ? e->cap_pc : 0;
}

int edit_listino_over_cap(const char* code) noexcept {
	const auto* e = edit_listino_find(code);
	return e != nullptr ? e->over_cap : 0;
}

long edit_listino_raw_unit_value(const EditListinoEntry& e) noexcept {
	return e.xp / kObjValueStorageScale;
}

int edit_listino_entry_step(const EditListinoEntry& e) noexcept {
	return e.step > 0 ? e.step : 1;
}

namespace {

[[nodiscard]] bool is_bitfield_loc(int loc) noexcept {
	return loc == APPLY_AFF2 || loc == APPLY_IMMUNE || loc == APPLY_M_IMMUNE ||
		   loc == APPLY_SPELL || loc == APPLY_WEAPON_SPELL;
}

[[nodiscard]] long total_apply_mod(const struct obj_data* obj, int loc, int skip_slot,
								   int replacement) {
	long total = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location != loc) {
			continue;
		}
		if(i == skip_slot) {
			total += replacement;
		} else {
			total += obj->affected[i].modifier;
		}
	}
	return total;
}

} // namespace

std::string edit_listino_obj_step_flags(const struct obj_data* obj) {
	if(obj == nullptr) {
		return {};
	}
	std::ostringstream os;
	/* Una riga per ogni apply numerico con step>1 o cap_obj>0 presente sul pezzo. */
	for(const auto& e : g_rows) {
		if(!usable_obj(e) || e.apply_loc == APPLY_NONE || is_bitfield_loc(e.apply_loc) ||
		   e.apply_bit != 0) {
			continue;
		}
		const int step = edit_listino_entry_step(e);
		if(step <= 1 && e.cap_obj <= 0) {
			continue;
		}
		bool present = false;
		long total = 0;
		for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
			if(obj->affected[i].location != e.apply_loc ||
			   obj->affected[i].modifier == 0) {
				continue;
			}
			present = true;
			const int mod = obj->affected[i].modifier;
			total += mod;
			if(step > 1 && (mod % step) != 0) {
				os << "  $c0011Listino$c0007: " << e.label << " " << mod
				   << " (slot) non e' multiplo dello step " << step;
				if(e.cap_obj > 0 && e.sign < 0) {
					os << " (cap -" << e.cap_obj << " a pezzo)";
				} else if(e.cap_obj > 0) {
					os << " (cap " << e.cap_obj << " a pezzo)";
				}
				os << ". Il pezzo resta valutato; le nuove modifiche devono "
					  "rispettare lo step.\n\r";
			}
		}
		if(!present) {
			continue;
		}
		if(step > 1 && (total % step) != 0) {
			os << "  $c0011Listino$c0007: " << e.label << " totale " << total
			   << " non e' multiplo dello step " << step
			   << ". Il pezzo resta valutato.\n\r";
		}
		if(e.cap_obj > 0 && e.sign < 0 && total < -e.cap_obj) {
			os << "  $c0011Listino$c0007: " << e.label << " totale " << total
			   << " oltre il cap di -" << e.cap_obj
			   << " a pezzo. Il pezzo resta valutato.\n\r";
		} else if(e.cap_obj > 0 && e.sign > 0 && total > e.cap_obj) {
			os << "  $c0011Listino$c0007: " << e.label << " totale " << total
			   << " oltre il cap di " << e.cap_obj
			   << " a pezzo. Il pezzo resta valutato.\n\r";
		}
	}
	return os.str();
}

bool edit_listino_accept_obj_mod(const struct obj_data* obj, int slot, int new_mod,
								 std::string& err) {
	err.clear();
	if(obj == nullptr || slot < 0 || slot >= MAX_OBJ_AFFECT) {
		err = "Slot affect non valido.\n\r";
		return false;
	}
	if(g_rows.empty()) {
		load_defaults();
	}
	const int loc = obj->affected[slot].location;
	if(loc == APPLY_NONE || loc == APPLY_SKIP || is_bitfield_loc(loc)) {
		return true;
	}
	const EditListinoEntry* e = edit_listino_find_apply(loc, 0UL);
	if(e == nullptr || !usable_obj(*e)) {
		return true;
	}
	const int step = edit_listino_entry_step(*e);
	const int old_mod = obj->affected[slot].modifier;
	const long old_total = total_apply_mod(obj, loc, -1, 0);
	const long new_total = total_apply_mod(obj, loc, slot, new_mod);

	if(step > 1 && new_mod != 0 && (new_mod % step) != 0) {
		err = "Il " + e->label + " si vende a slot di " +
			  (e->sign < 0 ? "-" : "") + std::to_string(step);
		if(e->cap_obj > 0 && e->sign < 0) {
			err += " (cap -" + std::to_string(e->cap_obj) + " a pezzo)";
		} else if(e->cap_obj > 0) {
			err += " (cap " + std::to_string(e->cap_obj) + " a pezzo)";
		}
		err += ". Il modificatore deve essere un multiplo di " +
			   std::to_string(step) + ".\n\r";
		return false;
	}
	if(e->cap_obj > 0 && e->sign < 0 && new_total < -e->cap_obj) {
		err = "Il " + e->label + " ha cap -" + std::to_string(e->cap_obj) +
			  " a pezzo (totale sul pezzo sarebbe " + std::to_string(new_total) +
			  ").\n\r";
		return false;
	}
	if(e->cap_obj > 0 && e->sign > 0 && new_total > e->cap_obj) {
		err = "Il " + e->label + " ha cap " + std::to_string(e->cap_obj) +
			  " a pezzo (totale sul pezzo sarebbe " + std::to_string(new_total) +
			  ").\n\r";
		return false;
	}
	if(step > 1 && new_total != old_total && new_total != 0 &&
	   (new_total % step) != 0) {
		err = "Il totale di " + e->label + " sul pezzo deve essere un multiplo di " +
			  std::to_string(step) + " (ora " + std::to_string(old_total) +
			  ", diventerebbe " + std::to_string(new_total) + ").\n\r";
		return false;
	}
	(void)old_mod;
	return true;
}

EditListinoCost edit_listino_normal_cost(const char* code, int units, int n_classes) {
	EditListinoCost c;
	units = std::abs(units);
	const auto* e = edit_listino_find(code);
	if(e == nullptr || !usable_pc(*e) || units <= 0) {
		return c;
	}
	n_classes = std::max(1, n_classes);
	const auto& r = edit_listino_rules_cache();
	const auto [cm_n, cm_d] = edit_listino_class_mult_frac(n_classes);

	const int step = e->step > 0 ? e->step : 1;
	long long xp = (static_cast<long long>(e->xp) * static_cast<long long>(units)) /
				   static_cast<long long>(step);
	xp = mul_div(xp, r.artifact_mult_num, r.artifact_mult_den);
	xp = mul_div(xp, cm_n, cm_d);
	c.xp_total = std::max(0LL, xp);
	if(r.xp_split_by_classes) {
		xp /= static_cast<long long>(n_classes);
	}
	c.xp = std::max(0LL, xp);

	const long long rune_num = static_cast<long long>(e->rune_num) *
							   static_cast<long long>(units) *
							   static_cast<long long>(r.artifact_mult_num) *
							   static_cast<long long>(cm_n);
	const long long rune_den = static_cast<long long>(std::max(1, e->rune_den)) *
							   static_cast<long long>(std::max(1, r.artifact_mult_den)) *
							   static_cast<long long>(std::max(1, cm_d)) *
							   static_cast<long long>(step);
	if(r.rune_rounding_ceil) {
		c.rune = ceil_frac(rune_num, rune_den);
	} else {
		c.rune = static_cast<int>(std::max(0LL, rune_num / rune_den));
	}
	return c;
}

EditListinoCost edit_listino_over_cost(const char* code, int from_units, int to_units,
									   int n_classes) {
	EditListinoCost c;
	const auto* e = edit_listino_find(code);
	if(e == nullptr || !usable_pc(*e) || to_units <= from_units) {
		return c;
	}
	const int step = e->over_step > 0 ? e->over_step : 1;
	const int from_slots = from_units / step;
	const int to_slots = to_units / step;
	if(to_slots <= from_slots) {
		return c;
	}
	n_classes = std::max(1, n_classes);
	const auto& r = edit_listino_rules_cache();
	const auto [cm_n, cm_d] = edit_listino_class_mult_frac(n_classes);
	const int art_n = r.artifact_mult_num;
	const int art_d = r.artifact_mult_den > 0 ? r.artifact_mult_den : 1;

	long long base_xp = 0;
	long long base_gold = 0;
	int rune = 0;
	for(int s = from_slots + 1; s <= to_slots; ++s) {
		/* Slot n: base_n = over_* + (n-1)*over_* * inc/100. */
		const long long inc_xp =
			(static_cast<long long>(e->over_xp) * static_cast<long long>(e->over_inc_pct) *
			 static_cast<long long>(s - 1)) /
			100LL;
		const long long inc_gold =
			(static_cast<long long>(e->over_gold) * static_cast<long long>(e->over_inc_pct) *
			 static_cast<long long>(s - 1)) /
			100LL;
		base_xp += static_cast<long long>(e->over_xp) + inc_xp;
		base_gold += static_cast<long long>(e->over_gold) + inc_gold;
		/* Rune: ceil(over_rune * artifact) a slot, senza class_mult. */
		rune += ceil_frac(static_cast<long long>(e->over_rune) * art_n, art_d);
	}
	/* xp/oro: base * artifact * class_mult. */
	long long xp_total = mul_div(base_xp, art_n, art_d);
	xp_total = mul_div(xp_total, cm_n, cm_d);
	long long gold = mul_div(base_gold, art_n, art_d);
	gold = mul_div(gold, cm_n, cm_d);

	long long xp_pay = xp_total;
	if(r.xp_split_by_classes) {
		xp_pay = xp_total / static_cast<long long>(n_classes);
	}
	c.xp = std::max(0LL, xp_pay);
	c.xp_total = std::max(0LL, xp_total);
	c.gold = std::max(0LL, gold);
	c.rune = std::max(0, rune);
	return c;
}


namespace {

[[nodiscard]] const char* alias_to_code(std::string_view key) {
	const std::string k = ascii_lower(key);
	if(k == "hp" || k == "hit" || k == "hits") {
		return "hit";
	}
	if(k == "mana") {
		return "mana";
	}
	if(k == "move" || k == "mov") {
		return "move";
	}
	if(k == "hpregen" || k == "hitregen" || k == "hp_regen" || k == "hit_regen" ||
	   k == "hpr") {
		return "hit_regen";
	}
	if(k == "manaregen" || k == "mana_regen" || k == "manar") {
		return "mana_regen";
	}
	if(k == "moveregen" || k == "movregen" || k == "move_regen" || k == "mover") {
		return "move_regen";
	}
	if(k == "spellfail" || k == "sf" || k == "fail") {
		return "spellfail";
	}
	return nullptr;
}

void send_wiz_help(char_data* ch) {
	send_to_char(
		"$c0014configure editpool$c0007 — listino unico (solo campi on_pc=1).\n\r"
		"  $c0015configure editpool$c0007\n\r"
		"  $c0015configure editpool set <chiave> <valore>$c0007\n\r"
		"  $c0015configure editpool reload$c0007\n\r"
		"  $c0015configure editpool reset$c0007\n\r"
		"Campi PG: hit (alias hp), mana, move, hit_regen, mana_regen, move_regen, "
		"spellfail.\n\r"
		"Chiavi riga: max_<campo>, <campo>_xp, <campo>_rune_num, <campo>_rune_den,\n\r"
		"  max_over_<campo>, over_<campo>_xp, over_<campo>_gold, over_<campo>_rune,\n\r"
		"  over_<campo>_step, over_<campo>_inc_pct.\n\r"
		"Step listino normale (colonna step, default 1): AC oggetto a slot di -10,\n\r"
		"  cap -40 a pezzo, 10M xp / 15 rune a slot.\n\r"
		"Scala resistenze -100..+100 (righe resist_<elem>, disabilitate):\n\r"
		"  APPLY_IMMUNE=+50, APPLY_M_IMMUNE=+100, APPLY_SUSC=-50\n\r"
		"  (resist_equiv / resist_code; i bit in gioco non cambiano).\n\r"
		"Regole: artifact_mult_num/den, class_mult_1|2|3_num/den, malus_mult_num/den,\n\r"
		"  xp_split_by_classes, rune_rounding, over_progression,\n\r"
		"  broker_fee_percent_num/den (default 25/1),\n\r"
		"  artifact_cost_xp_num/den (flag artifact, default 100M/1; x class_mult).\n\r"
		"Chiavi extra riga: <campo>_buy_step (pool: hit/mana/move 10, regen 5),\n\r"
		"  <campo>_enabled (1=vendibile se anche on_obj/on_pc).\n\r"
		"Valori: cifre, punti migliaia o k/m (es. 1.600k, 2m, -1).\n\r",
		ch);
}

void dump_listino(char_data* ch) {
	const auto& r = edit_listino_rules_cache();
	std::ostringstream os;
	os << "$c0014=== edit_listino (campi PG) ===$c0007\n\r"
	   << "Regole: artifact " << r.artifact_mult_num << "/" << r.artifact_mult_den
	   << "  class 1:" << r.class_mult_1_num << "/" << r.class_mult_1_den
	   << " 2:" << r.class_mult_2_num << "/" << r.class_mult_2_den
	   << " 3:" << r.class_mult_3_num << "/" << r.class_mult_3_den
	   << "  malus " << r.malus_mult_num << "/" << r.malus_mult_den << "\n\r"
	   << "  xp_split_by_classes=" << r.xp_split_by_classes
	   << "  rune_rounding=" << (r.rune_rounding_ceil ? "ceil" : "trunc")
	   << "  over_progression=" << r.over_progression << " (1=lineare)\n\r"
	   << "  broker_fee_percent=" << r.broker_fee_percent_num << "/"
	   << (r.broker_fee_percent_den > 0 ? r.broker_fee_percent_den : 1)
	   << "  artifact_cost_xp=" << r.artifact_cost_xp_num << "/"
	   << (r.artifact_cost_xp_den > 0 ? r.artifact_cost_xp_den : 1) << "\n\r";
	for(const auto& e : g_rows) {
		if(!e.on_pc) {
			continue;
		}
		os << "  $c0015" << e.code << "$c0007 " << e.label
		   << "  xp=" << format_thousands(e.xp)
		   << "  rune=" << e.rune_num << "/" << e.rune_den
		   << "  sign=" << e.sign
		   << "  step=" << (e.step > 0 ? e.step : 1)
		   << "  buy_step=" << (e.buy_step > 0 ? e.buy_step : 1)
		   << "  enabled=" << (e.enabled ? 1 : 0)
		   << "  cap_pc=" << e.cap_pc << "\n\r";
		if(e.over_cap > 0 || e.over_xp != 0 || e.over_rune != 0) {
			os << "    over cap=" << e.over_cap << " step=" << e.over_step
			   << " rune/slot=" << e.over_rune
			   << " xp=" << format_thousands(e.over_xp)
			   << " gold=" << format_thousands(e.over_gold)
			   << " inc_pct=" << e.over_inc_pct << "\n\r";
		}
	}
	os << "$c0014=== edit_listino (oggetti con step/cap) ===$c0007\n\r";
	bool any_obj = false;
	for(const auto& e : g_rows) {
		if(!e.on_obj || e.xp == 0) {
			continue;
		}
		if((e.step > 1 || e.cap_obj > 0)) {
			any_obj = true;
			os << "  $c0015" << e.code << "$c0007 " << e.label
			   << "  xp=" << format_thousands(e.xp)
			   << "  rune=" << e.rune_num << "/" << e.rune_den
			   << "  step=" << (e.step > 0 ? e.step : 1)
			   << "  cap_obj=" << e.cap_obj << "\n\r";
		}
	}
	if(!any_obj) {
		os << "  (nessuna riga oggetto con step>1 o cap_obj)\n\r";
	}
	os << "$c0014=== cap group ===$c0007\n\r";
	if(g_cap_groups.empty()) {
		os << "  (nessun gruppo)\n\r";
	}
	for(const auto& g : g_cap_groups) {
		os << "  $c0015" << g.code << "$c0007 " << g.label << "\n\r";
	}
	bool any_unsupported = false;
	for(const auto& e : g_rows) {
		if(!e.mud_supported) {
			if(!any_unsupported) {
				os << "$c0014=== righe non supportate dal MUD ===$c0007\n\r";
				any_unsupported = true;
			}
			os << "  $c0015" << e.code << "$c0007 " << e.label
			   << "  (ignorata: no apply, no prezzo)\n\r";
		}
	}
	os << "$c0014=== scala resistenze numeriche (disabilitata) ===$c0007\n\r"
	   << "Equivalenza bit: APPLY_IMMUNE = +50, APPLY_M_IMMUNE = +100, "
		  "APPLY_SUSC = -50\n\r"
	   << "(colonne resist_equiv / resist_code; on_obj=on_pc=0 finche' non "
		  "c'e' il motore)\n\r";
	for(const auto& e : g_rows) {
		if(e.category != "resist_num") {
			continue;
		}
		os << "  $c0015" << e.code << "$c0007 " << e.label
		   << "  xp=" << format_thousands(e.xp)
		   << "  rune=" << e.rune_num << "/" << e.rune_den
		   << "  step=" << (e.step > 0 ? e.step : 1)
		   << "  cap=-" << e.cap_obj << "..+" << e.cap_obj << "\n\r";
	}
	send_to_char(os.str().c_str(), ch);
}

[[nodiscard]] bool starts_with(const std::string& s, const char* pfx) {
	const std::size_t n = std::strlen(pfx);
	return s.size() >= n && s.compare(0, n, pfx) == 0;
}

[[nodiscard]] bool set_rule_key(const std::string& key, long long val, std::string& err) {
	auto set_num_den = [&](int& num, int& den, bool is_num) {
		if(is_num) {
			num = static_cast<int>(val);
		} else {
			if(val <= 0) {
				err = "Il denominatore deve essere >= 1.\n\r";
				return false;
			}
			den = static_cast<int>(val);
		}
		return true;
	};
	bool ok = false;
	if(key == "artifact_mult_num") {
		ok = set_num_den(g_rules.artifact_mult_num, g_rules.artifact_mult_den, true);
	} else if(key == "artifact_mult_den") {
		ok = set_num_den(g_rules.artifact_mult_num, g_rules.artifact_mult_den, false);
	} else if(key == "class_mult_1_num") {
		ok = set_num_den(g_rules.class_mult_1_num, g_rules.class_mult_1_den, true);
	} else if(key == "class_mult_1_den") {
		ok = set_num_den(g_rules.class_mult_1_num, g_rules.class_mult_1_den, false);
	} else if(key == "class_mult_2_num") {
		ok = set_num_den(g_rules.class_mult_2_num, g_rules.class_mult_2_den, true);
	} else if(key == "class_mult_2_den") {
		ok = set_num_den(g_rules.class_mult_2_num, g_rules.class_mult_2_den, false);
	} else if(key == "class_mult_3_num") {
		ok = set_num_den(g_rules.class_mult_3_num, g_rules.class_mult_3_den, true);
	} else if(key == "class_mult_3_den") {
		ok = set_num_den(g_rules.class_mult_3_num, g_rules.class_mult_3_den, false);
	} else if(key == "malus_mult_num") {
		ok = set_num_den(g_rules.malus_mult_num, g_rules.malus_mult_den, true);
	} else if(key == "malus_mult_den") {
		ok = set_num_den(g_rules.malus_mult_num, g_rules.malus_mult_den, false);
	} else if(key == "xp_split_by_classes") {
		g_rules.xp_split_by_classes = val != 0 ? 1 : 0;
		ok = true;
	} else if(key == "rune_rounding") {
		g_rules.rune_rounding_ceil = val != 0 ? 1 : 0;
		ok = true;
	} else if(key == "over_progression") {
		g_rules.over_progression = static_cast<int>(val);
		ok = true;
	} else if(key == "broker_fee_percent_num") {
		ok = set_num_den(g_rules.broker_fee_percent_num, g_rules.broker_fee_percent_den,
						 true);
	} else if(key == "broker_fee_percent_den") {
		ok = set_num_den(g_rules.broker_fee_percent_num, g_rules.broker_fee_percent_den,
						 false);
	} else if(key == "artifact_cost_xp_num") {
		ok = set_num_den(g_rules.artifact_cost_xp_num, g_rules.artifact_cost_xp_den, true);
	} else if(key == "artifact_cost_xp_den") {
		ok = set_num_den(g_rules.artifact_cost_xp_num, g_rules.artifact_cost_xp_den, false);
	} else {
		return false;
	}
	if(!ok) {
		return false;
	}
#if USE_MYSQL
	auto persist_frac = [](const char* k, int n, int d) {
		persist_rule(k, n, d);
	};
	if(key.rfind("artifact_mult", 0) == 0) {
		persist_frac("artifact_mult", g_rules.artifact_mult_num, g_rules.artifact_mult_den);
	} else if(key.rfind("class_mult_1", 0) == 0) {
		persist_frac("class_mult_1", g_rules.class_mult_1_num, g_rules.class_mult_1_den);
	} else if(key.rfind("class_mult_2", 0) == 0) {
		persist_frac("class_mult_2", g_rules.class_mult_2_num, g_rules.class_mult_2_den);
	} else if(key.rfind("class_mult_3", 0) == 0) {
		persist_frac("class_mult_3", g_rules.class_mult_3_num, g_rules.class_mult_3_den);
	} else if(key.rfind("malus_mult", 0) == 0) {
		persist_frac("malus_mult", g_rules.malus_mult_num, g_rules.malus_mult_den);
	} else if(key == "xp_split_by_classes") {
		persist_rule("xp_split_by_classes", g_rules.xp_split_by_classes, 1);
	} else if(key == "rune_rounding") {
		persist_rule("rune_rounding", g_rules.rune_rounding_ceil, 1);
	} else if(key == "over_progression") {
		persist_rule("over_progression", g_rules.over_progression, 1);
	} else if(key.rfind("broker_fee_percent", 0) == 0) {
		persist_frac("broker_fee_percent", g_rules.broker_fee_percent_num,
					 g_rules.broker_fee_percent_den);
	} else if(key.rfind("artifact_cost_xp", 0) == 0) {
		persist_frac("artifact_cost_xp", g_rules.artifact_cost_xp_num,
					 g_rules.artifact_cost_xp_den);
	}
#endif
	return true;
}

[[nodiscard]] bool parse_field_key(const std::string& key, std::string& code,
								   std::string& field, std::string& err) {
	/* max_<code>, max_over_<code>, over_<code>_<attr>, <code>_<attr> */
	if(starts_with(key, "max_over_")) {
		const char* c = alias_to_code(key.substr(9));
		if(c == nullptr) {
			err = "Campo sconosciuto (max_over_*).\n\r";
			return false;
		}
		code = c;
		field = "over_cap";
		return true;
	}
	if(starts_with(key, "max_")) {
		const char* c = alias_to_code(key.substr(4));
		if(c == nullptr) {
			err = "Campo sconosciuto (max_*).\n\r";
			return false;
		}
		code = c;
		field = "cap_pc";
		return true;
	}
	if(starts_with(key, "over_")) {
		/* over_<code>_xp|gold|rune|step|inc_pct */
		const std::string rest = key.substr(5);
		const auto pos = rest.rfind('_');
		if(pos == std::string::npos || pos == 0) {
			err = "Chiave over non valida.\n\r";
			return false;
		}
		const char* c = alias_to_code(rest.substr(0, pos));
		if(c == nullptr) {
			err = "Campo sconosciuto (over_*).\n\r";
			return false;
		}
		const std::string attr = rest.substr(pos + 1);
		code = c;
		if(attr == "xp") {
			field = "over_xp";
		} else if(attr == "gold") {
			field = "over_gold";
		} else if(attr == "rune") {
			field = "over_rune";
		} else if(attr == "step") {
			field = "over_step";
		} else if(attr == "inc_pct" || attr == "inc") {
			field = "over_inc_pct";
		} else if(attr == "cap") {
			field = "over_cap";
		} else {
			err = "Attributo over sconosciuto (xp|gold|rune|step|inc_pct|cap).\n\r";
			return false;
		}
		return true;
	}
	const auto pos = key.rfind('_');
	if(pos == std::string::npos || pos == 0) {
		err = "Chiave sconosciuta.\n\r";
		return false;
	}
	const char* c = alias_to_code(key.substr(0, pos));
	if(c == nullptr) {
		err = "Campo sconosciuto.\n\r";
		return false;
	}
	const std::string attr = key.substr(pos + 1);
	code = c;
	if(attr == "xp") {
		field = "xp";
	} else if(attr == "rune" || attr == "rune_num") {
		field = "rune_num";
	} else if(attr == "den" || attr == "rune_den") {
		field = "rune_den";
	} else if(attr == "buy_step" || attr == "buystep") {
		field = "buy_step";
	} else if(attr == "enabled") {
		field = "enabled";
	} else {
		err = "Attributo sconosciuto (xp|rune_num|rune_den|buy_step|enabled).\n\r";
		return false;
	}
	return true;
}

} // namespace

void edit_listino_wiz(struct char_data* ch, const char* arg) {
	if(ch == nullptr || !IS_PC(ch) || GetMaxLevel(ch) < QUESTMASTER) {
		send_to_char("Non hai il livello per usare configure editpool.\n\r", ch);
		return;
	}
	if(g_rows.empty()) {
		load_defaults();
	}

	std::string_view av = arg ? arg : "";
	while(!av.empty() && std::isspace(static_cast<unsigned char>(av.front()))) {
		av.remove_prefix(1);
	}
	std::string first;
	std::string_view rest = av;
	if(!av.empty()) {
		std::size_t i = 0;
		while(i < av.size() && !std::isspace(static_cast<unsigned char>(av[i]))) {
			++i;
		}
		first = ascii_lower(av.substr(0, i));
		rest = av.substr(i);
		while(!rest.empty() && std::isspace(static_cast<unsigned char>(rest.front()))) {
			rest.remove_prefix(1);
		}
	}

	if(first.empty() || first == "dump" || first == "show" || first == "list") {
		dump_listino(ch);
		return;
	}
	if(first == "help" || first == "?") {
		send_wiz_help(ch);
		return;
	}
	if(first == "reload") {
		edit_listino_reload();
		send_to_char(
			"edit_listino: ricaricato da DB (se il DB non risponde restano i valori "
			"in cache/default).\n\r",
			ch);
		mudlog(LOG_CHECK, "edit_listino: %s reload", GET_NAME(ch));
		return;
	}
	if(first == "reset") {
		load_defaults();
#if USE_MYSQL
		if(!persist_pc_defaults_and_rules()) {
			send_to_char("Reset in memoria ok, ma salvataggio MySQL fallito.\n\r", ch);
			return;
		}
#endif
		send_to_char(
			"edit_listino: campi PG e regole riportati ai default e salvati.\n\r",
			ch);
		mudlog(LOG_CHECK, "edit_listino: %s reset", GET_NAME(ch));
		return;
	}
	if(first != "set") {
		send_wiz_help(ch);
		return;
	}

	std::string key;
	std::string_view valpart = rest;
	{
		std::size_t i = 0;
		while(i < rest.size() && !std::isspace(static_cast<unsigned char>(rest[i]))) {
			++i;
		}
		key = ascii_lower(rest.substr(0, i));
		valpart = rest.substr(i);
		while(!valpart.empty() &&
			  std::isspace(static_cast<unsigned char>(valpart.front()))) {
			valpart.remove_prefix(1);
		}
	}
	if(key.empty() || valpart.empty()) {
		send_wiz_help(ch);
		return;
	}

	long long val = 0;
	if(!configure_parse_ll(valpart, val)) {
		send_to_char(
			"Valore non valido (cifre, punti migliaia, suffisso k/m, segno opzionale).\n\r",
			ch);
		return;
	}

	std::string err;
	if(set_rule_key(key, val, err)) {
		if(!err.empty()) {
			send_to_char(err.c_str(), ch);
			return;
		}
		send_to_char("edit_listino: regola aggiornata e salvata.\n\r", ch);
		mudlog(LOG_CHECK, "edit_listino: %s set %s=%lld", GET_NAME(ch), key.c_str(),
			   val);
		return;
	}

	std::string code;
	std::string field;
	if(!parse_field_key(key, code, field, err)) {
		send_to_char(err.c_str(), ch);
		return;
	}
	auto* e = const_cast<EditListinoEntry*>(find_code(code.c_str()));
	if(e == nullptr) {
		send_to_char("Riga listino non trovata.\n\r", ch);
		return;
	}
	if(!e->on_pc) {
		send_to_char(
			"configure editpool agisce solo sui campi on_pc=1.\n\r", ch);
		return;
	}

	if(field == "xp") {
		e->xp = static_cast<long>(std::max(0LL, val));
	} else if(field == "rune_num") {
		e->rune_num = static_cast<int>(std::max(0LL, val));
	} else if(field == "rune_den") {
		if(val <= 0) {
			send_to_char("rune_den deve essere >= 1.\n\r", ch);
			return;
		}
		e->rune_den = static_cast<int>(val);
	} else if(field == "cap_pc") {
		e->cap_pc = static_cast<int>(std::max(0LL, val));
	} else if(field == "over_cap") {
		e->over_cap = static_cast<int>(val);
	} else if(field == "over_xp") {
		e->over_xp = static_cast<long>(std::max(0LL, val));
	} else if(field == "over_gold") {
		e->over_gold = static_cast<long>(std::max(0LL, val));
	} else if(field == "over_rune") {
		e->over_rune = static_cast<int>(std::max(0LL, val));
	} else if(field == "over_step") {
		e->over_step = static_cast<int>(std::max(0LL, val));
	} else if(field == "over_inc_pct") {
		e->over_inc_pct = static_cast<int>(std::max(0LL, val));
	} else if(field == "buy_step") {
		if(val <= 0) {
			send_to_char("buy_step deve essere >= 1.\n\r", ch);
			return;
		}
		e->buy_step = static_cast<int>(val);
	} else if(field == "enabled") {
		e->enabled = val != 0 ? 1 : 0;
	} else {
		send_to_char("Campo riga sconosciuto.\n\r", ch);
		return;
	}

#if USE_MYSQL
	if(!persist_entry(*e)) {
		send_to_char("Aggiornato in memoria, ma salvataggio MySQL fallito.\n\r", ch);
		return;
	}
#endif
	send_to_char("edit_listino: aggiornato e salvato.\n\r", ch);
	mudlog(LOG_CHECK, "edit_listino: %s set %s=%lld (code=%s field=%s)", GET_NAME(ch),
		   key.c_str(), val, code.c_str(), field.c_str());
}

} // namespace Alarmud
