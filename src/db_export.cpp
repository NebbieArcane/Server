/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* db_export.cpp - comando wiz dbexport: MySQL -> CSV (no password).
 * */
/***************************  System  include ************************************/
#include <boost/filesystem.hpp>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>
#include <string>
#include <unordered_map>
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
#include "comm.hpp"
#include "Sql.hpp"
#include "interpreter.hpp"
#include "utility.hpp"
#include "multiclass.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "clan_symbol.hpp"
/***************************  Local    include ************************************/
#include "db_export.hpp"

#if USE_MYSQL
#include <odb/mysql/database.hxx>
#include <mysql/mysql.h>
#endif

namespace Alarmud {

namespace {

#if USE_MYSQL

namespace fs = boost::filesystem;

/** Directory sotto cwd (di solito lib/). */
constexpr const char* kDbExportDir = "exports";
constexpr unsigned long long kDbExportMaxRows = 200000ULL;

struct NamedExport {
	const char* name;
	const char* help;
	const char* sql; /* solo SELECT; colonne secret saltate in scrittura */
};

/** Tabelle ammesse per `dbexport table` / `dump`. */
constexpr const char* kAllowedTables[] = {
	"user",
	"toon",
	"registered",
	"toon_nuke_blacklist",
	"character_core",
	"character_classes",
	"character_stats",
	"character_skills",
	"character_affects",
	"character_resistance",
	"character_rent",
	"character_inventory",
	"character_inventory_affect",
	"character_prefs",
	"character_aliases",
	"character_achievements",
	"character_mercy",
	"character_quest_progress",
	"character_death_snapshot",
	"object_instance",
	"object_instance_affect",
	"object_instance_extradesc",
	"object_instance_event",
	"procarea_balance",
};

constexpr NamedExport kNamedExports[] = {
	{"toons", "toon (no password)",
	 "SELECT id, name, title, level, lastlogin, lasthost, owner_id, "
	 "migrated_at, schema_version FROM toon ORDER BY id"},
	{"users", "account user (no password; email solo con pii)",
	 "SELECT * FROM user ORDER BY id"},
	{"chars", "toon + character_core (scheda)",
	 "SELECT t.id, t.name, t.title, t.level, t.lastlogin, t.lasthost, "
	 "c.class_primary, c.sex, c.race, c.birth, c.played, c.last_logon, "
	 "c.hometown, c.alignment, c.act, c.load_room, c.start_room, c.updated_at "
	 "FROM toon t LEFT JOIN character_core c ON c.toon_id = t.id "
	 "ORDER BY t.id"},
	{"edits", "object_instance attivi + affect (A0-A4)",
	 nullptr}, /* export dedicato: write_edits_with_affects_csv */
	{"edits_deleted", "object_instance soft-deleted + affect (A0-A4)",
	 nullptr},
	{"edits_sheet",
	 "vista foglio DB Edit (grants + pool pre-strip ricostruito)",
	 nullptr}, /* write_edits_sheet_csv */
	{"edits_affect", "object_instance_affect (una riga per slot)",
	 "SELECT * FROM object_instance_affect ORDER BY key_instance_id, key_affect_slot"},
	{"edit_strips",
	 "storico strip listino (event edit_pool + oggetto)",
	 "SELECT e.id AS event_id, e.at, e.kind, e.note, e.detail, "
	 "e.actor_name, e.actor_toon_id, "
	 "oi.id AS instance_id, oi.base_vnum, oi.legacy_edit_vnum, "
	 "oi.short_desc, oi.obj_name, oi.owner_name, oi.source, oi.deleted "
	 "FROM object_instance_event e "
	 "INNER JOIN object_instance oi ON oi.id = e.instance_id "
	 "WHERE e.kind = 'edit_pool' "
	 "ORDER BY e.at DESC, e.id DESC"},
	{"dust",
	 "storico polvere (event player_dust + oggetto + dust_* attuali)",
	 "SELECT e.id AS event_id, e.at, e.kind, e.note, e.detail, "
	 "e.actor_name, e.actor_toon_id, "
	 "oi.id AS instance_id, oi.base_vnum, oi.legacy_edit_vnum, "
	 "oi.short_desc, oi.obj_name, oi.owner_name, oi.source, oi.deleted, "
	 "oi.dust_hp, oi.dust_mana, oi.dust_move, "
	 "oi.dust_hp_regen, oi.dust_mana_regen, oi.dust_move_regen, oi.dust_spellfail "
	 "FROM object_instance_event e "
	 "INNER JOIN object_instance oi ON oi.id = e.instance_id "
	 "WHERE e.kind = 'player_dust' "
	 "ORDER BY e.at DESC, e.id DESC"},
	{"inventory", "character_inventory (non deleted)",
	 "SELECT * FROM character_inventory "
	 "WHERE deleted = 0 OR deleted IS NULL ORDER BY toon_id, list_index"},
	{"stats", "character_stats + nome PG",
	 "SELECT t.name AS name, s.* "
	 "FROM character_stats s "
	 "LEFT JOIN toon t ON t.id = s.toon_id "
	 "ORDER BY s.toon_id"},
	{"skills", "character_skills",
	 "SELECT * FROM character_skills ORDER BY toon_id, skill_id"},
	{"procarea", "procarea_balance key/value",
	 "SELECT * FROM procarea_balance ORDER BY conf_key"},
};

[[nodiscard]] bool str_ieq(const char* a, const char* b) {
	if(a == nullptr || b == nullptr) {
		return a == b;
	}
	while(*a && *b) {
		if(std::tolower(static_cast<unsigned char>(*a)) !=
		   std::tolower(static_cast<unsigned char>(*b))) {
			return false;
		}
		++a;
		++b;
	}
	return *a == *b;
}

[[nodiscard]] bool table_name_ok(const char* name) {
	if(name == nullptr || *name == '\0') {
		return false;
	}
	for(const char* p = name; *p; ++p) {
		const unsigned char c = static_cast<unsigned char>(*p);
		if(!(std::isalnum(c) || *p == '_')) {
			return false;
		}
	}
	return true;
}

[[nodiscard]] bool table_allowed(const char* name) {
	if(!table_name_ok(name)) {
		return false;
	}
	for(const char* t : kAllowedTables) {
		if(str_ieq(t, name)) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] bool column_blocked(const char* col, bool include_pii) {
	if(col == nullptr || *col == '\0') {
		return true;
	}
	if(str_ieq(col, "password") || str_ieq(col, "authcode")) {
		return true;
	}
	if(!include_pii) {
		if(str_ieq(col, "email") || str_ieq(col, "backup_email") ||
		   str_ieq(col, "email1") || str_ieq(col, "email2") ||
		   str_ieq(col, "realname")) {
			return true;
		}
	}
	return false;
}

void csv_append_field(std::string& out, const char* raw) {
	const char* s = raw ? raw : "";
	bool need_quote = false;
	for(const char* p = s; *p; ++p) {
		if(*p == ',' || *p == '"' || *p == '\n' || *p == '\r') {
			need_quote = true;
			break;
		}
	}
	if(!need_quote) {
		out.append(s);
		return;
	}
	out.push_back('"');
	for(const char* p = s; *p; ++p) {
		if(*p == '"') {
			out.append("\"\"");
		}
		else {
			out.push_back(*p);
		}
	}
	out.push_back('"');
}

[[nodiscard]] std::string timestamp_stamp() {
	const time_t now = time(nullptr);
	struct tm tm_buf {};
#if defined(_POSIX_VERSION)
	localtime_r(&now, &tm_buf);
#else
	if(struct tm* tmp = localtime(&now); tmp != nullptr) {
		tm_buf = *tmp;
	}
#endif
	char buf[32];
	std::snprintf(buf, sizeof(buf), "%04d%02d%02d_%02d%02d%02d", tm_buf.tm_year + 1900,
				  tm_buf.tm_mon + 1, tm_buf.tm_mday, tm_buf.tm_hour, tm_buf.tm_min,
				  tm_buf.tm_sec);
	return buf;
}

[[nodiscard]] bool ensure_exports_root(std::string& err) {
	boost::system::error_code ec;
	fs::create_directories(kDbExportDir, ec);
	if(ec) {
		err = ec.message();
		return false;
	}
	return true;
}

struct ExportResult {
	bool ok = false;
	unsigned long long rows = 0;
	unsigned skipped_cols = 0;
	bool truncated = false;
	std::string path;
	std::string error;
};

[[nodiscard]] ExportResult write_query_csv(MYSQL* h, const char* sql, const fs::path& path,
										   bool include_pii) {
	ExportResult r;
	r.path = path.string();
	if(h == nullptr || sql == nullptr || *sql == '\0') {
		r.error = "parametri invalidi";
		return r;
	}
	if(mysql_query(h, sql) != 0) {
		r.error = mysql_error(h);
		return r;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(res == nullptr) {
		if(mysql_field_count(h) == 0) {
			r.error = "la query non ha prodotto un result set (usa solo SELECT)";
		}
		else {
			r.error = mysql_error(h);
		}
		return r;
	}

	FILE* fp = std::fopen(path.string().c_str(), "wt");
	if(fp == nullptr) {
		mysql_free_result(res);
		r.error = "impossibile creare il file";
		return r;
	}

	const unsigned nfields = mysql_num_fields(res);
	MYSQL_FIELD* fields = mysql_fetch_fields(res);
	std::vector<bool> keep(nfields, false);
	std::string header;
	for(unsigned i = 0; i < nfields; ++i) {
		const char* name = fields[i].name ? fields[i].name : "";
		if(column_blocked(name, include_pii)) {
			++r.skipped_cols;
			continue;
		}
		keep[i] = true;
		if(!header.empty()) {
			header.push_back(',');
		}
		csv_append_field(header, name);
	}
	header.push_back('\n');
	std::fputs(header.c_str(), fp);

	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(r.rows >= kDbExportMaxRows) {
			r.truncated = true;
			break;
		}
		unsigned long* lengths = mysql_fetch_lengths(res);
		std::string line;
		bool first = true;
		for(unsigned i = 0; i < nfields; ++i) {
			if(!keep[i]) {
				continue;
			}
			if(!first) {
				line.push_back(',');
			}
			first = false;
			if(row[i] == nullptr) {
				/* NULL -> campo vuoto */
			}
			else {
				csv_append_field(line, row[i]);
				(void)lengths;
			}
		}
		line.push_back('\n');
		std::fputs(line.c_str(), fp);
		++r.rows;
	}

	std::fclose(fp);
	mysql_free_result(res);
	r.ok = true;
	return r;
}

[[nodiscard]] const char* apply_loc_label(int loc) {
	if(loc >= 0 && loc <= APPLY_SKIP && apply_types[loc] != nullptr &&
	   apply_types[loc][0] != '\n') {
		return apply_types[loc];
	}
	return nullptr;
}

/** Come sprintbit su stringa: nomi dei bit settati in maschera. */
[[nodiscard]] std::string format_bit_flag_names(unsigned long bits, const char* names[]) {
	if(names == nullptr) {
		return "NONE";
	}
	std::string result;
	long nr = 0;
	for(; bits; bits >>= 1) {
		if(IS_SET(1, bits)) {
			if(!result.empty()) {
				result.push_back(' ');
			}
			if(*names[nr] != '\n') {
				result += names[nr];
			}
			else {
				result += "UNDEFINED";
			}
		}
		if(*names[nr] != '\n') {
			++nr;
		}
	}
	return result.empty() ? "NONE" : result;
}

[[nodiscard]] std::string format_affect_mod_text(int loc, int mod) {
	switch(loc) {
	case APPLY_M_IMMUNE:
	case APPLY_IMMUNE:
	case APPLY_SUSC:
		return format_bit_flag_names(static_cast<unsigned long>(mod), immunity_names);
	case APPLY_SPELL:
		return format_bit_flag_names(static_cast<unsigned long>(mod), affected_bits);
	case APPLY_AFF2:
		return format_bit_flag_names(static_cast<unsigned long>(mod), affected_bits2);
	default:
		return std::to_string(mod);
	}
}

[[nodiscard]] std::string format_affect_readable(int loc, int mod) {
	std::string out;
	if(const char* lab = apply_loc_label(loc); lab != nullptr) {
		out = lab;
	}
	else {
		out = "loc";
		out += std::to_string(loc);
	}
	out.push_back(' ');
	const bool bitflag = (loc == APPLY_M_IMMUNE || loc == APPLY_IMMUNE || loc == APPLY_SUSC ||
						  loc == APPLY_SPELL || loc == APPLY_AFF2);
	if(!bitflag && mod >= 0) {
		out.push_back('+');
	}
	out += format_affect_mod_text(loc, mod);
	return out;
}

struct AffectSlot {
	bool set = false;
	int location = 0;
	int modifier = 0;
};

using AffectRow = std::array<AffectSlot, MAX_OBJ_AFFECT>;

[[nodiscard]] std::unordered_map<unsigned long long, AffectRow>
load_all_instance_affects(MYSQL* h) {
	std::unordered_map<unsigned long long, AffectRow> out;
	if(h == nullptr) {
		return out;
	}
	const char* sql =
		"SELECT key_instance_id, key_affect_slot, location, modifier "
		"FROM object_instance_affect";
	if(mysql_query(h, sql) != 0) {
		mudlog(LOG_SYSERR, "dbexport load affects: %s", mysql_error(h));
		return out;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(res == nullptr) {
		return out;
	}
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(row[0] == nullptr || row[1] == nullptr) {
			continue;
		}
		const unsigned long long iid = strtoull(row[0], nullptr, 10);
		const int slot = atoi(row[1]);
		if(iid == 0 || slot < 0 || slot >= MAX_OBJ_AFFECT) {
			continue;
		}
		AffectSlot& s = out[iid][static_cast<size_t>(slot)];
		s.set = true;
		s.location = row[2] ? atoi(row[2]) : 0;
		s.modifier = row[3] ? atoi(row[3]) : 0;
	}
	mysql_free_result(res);
	return out;
}

void append_affect_columns(std::string& line, const AffectRow* aff) {
	std::string readable;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		line.push_back(',');
		if(aff == nullptr || !(*aff)[static_cast<size_t>(i)].set) {
			line.push_back(','); /* A_i_loc empty, A_i_mod empty */
			continue;
		}
		const AffectSlot& s = (*aff)[static_cast<size_t>(i)];
		line.append(std::to_string(s.location));
		line.push_back(',');
		line.append(std::to_string(s.modifier));
		if(s.location != 0 || s.modifier != 0) {
			if(!readable.empty()) {
				readable.push_back(';');
			}
			readable += format_affect_readable(s.location, s.modifier);
		}
	}
	line.push_back(',');
	csv_append_field(line, readable.c_str());
}

/**
 * object_instance (+ deleted filter) con affect affiancati:
 * A0_loc,A0_mod ... A4_loc,A4_mod,affects
 */
[[nodiscard]] ExportResult write_edits_with_affects_csv(MYSQL* h, bool deleted_list,
													   const fs::path& path,
													   bool include_pii) {
	ExportResult r;
	r.path = path.string();
	if(h == nullptr) {
		r.error = "parametri invalidi";
		return r;
	}

	const auto affects = load_all_instance_affects(h);

	const char* sql = deleted_list ?
						  "SELECT * FROM object_instance WHERE deleted = 1 ORDER BY id" :
						  "SELECT * FROM object_instance WHERE deleted = 0 ORDER BY id";
	if(mysql_query(h, sql) != 0) {
		r.error = mysql_error(h);
		return r;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(res == nullptr) {
		r.error = mysql_error(h);
		return r;
	}

	FILE* fp = std::fopen(path.string().c_str(), "wt");
	if(fp == nullptr) {
		mysql_free_result(res);
		r.error = "impossibile creare il file";
		return r;
	}

	const unsigned nfields = mysql_num_fields(res);
	MYSQL_FIELD* fields = mysql_fetch_fields(res);
	std::vector<bool> keep(nfields, false);
	int id_col = -1;
	std::string header;
	for(unsigned i = 0; i < nfields; ++i) {
		const char* name = fields[i].name ? fields[i].name : "";
		if(str_ieq(name, "id")) {
			id_col = static_cast<int>(i);
		}
		if(column_blocked(name, include_pii)) {
			++r.skipped_cols;
			continue;
		}
		keep[i] = true;
		if(!header.empty()) {
			header.push_back(',');
		}
		csv_append_field(header, name);
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		header.append(",A");
		header.append(std::to_string(i));
		header.append("_loc,A");
		header.append(std::to_string(i));
		header.append("_mod");
	}
	header.append(",affects\n");
	std::fputs(header.c_str(), fp);

	if(id_col < 0) {
		std::fclose(fp);
		mysql_free_result(res);
		r.error = "colonna id assente in object_instance";
		return r;
	}

	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(r.rows >= kDbExportMaxRows) {
			r.truncated = true;
			break;
		}
		std::string line;
		bool first = true;
		for(unsigned i = 0; i < nfields; ++i) {
			if(!keep[i]) {
				continue;
			}
			if(!first) {
				line.push_back(',');
			}
			first = false;
			if(row[i] != nullptr) {
				csv_append_field(line, row[i]);
			}
		}

		unsigned long long iid = 0;
		if(row[id_col] != nullptr) {
			iid = strtoull(row[id_col], nullptr, 10);
		}
		const AffectRow* aff = nullptr;
		if(iid != 0) {
			if(const auto it = affects.find(iid); it != affects.end()) {
				aff = &it->second;
			}
		}
		append_affect_columns(line, aff);
		line.push_back('\n');
		std::fputs(line.c_str(), fp);
		++r.rows;
	}

	std::fclose(fp);
	mysql_free_result(res);
	r.ok = true;
	return r;
}

/* --- edits_sheet: vista stile foglio DB Edit + pool pre-strip --- */

enum SheetStat : int {
	SS_DAM = 0,
	SS_MANA,
	SS_HIT,
	SS_MOVE,
	SS_HIT_REGEN,
	SS_MANA_REGEN,
	SS_MOVE_REGEN,
	SS_HITROLL,
	SS_HITNDAM,
	SS_COUNT
};

[[nodiscard]] int sheet_stat_for_apply(int loc) noexcept {
	switch(loc) {
	case APPLY_DAMROLL:
		return SS_DAM;
	case APPLY_MANA:
		return SS_MANA;
	case APPLY_HIT:
		return SS_HIT;
	case APPLY_MOVE:
		return SS_MOVE;
	case APPLY_HIT_REGEN:
		return SS_HIT_REGEN;
	case APPLY_MANA_REGEN:
		return SS_MANA_REGEN;
	case APPLY_MOVE_REGEN:
		return SS_MOVE_REGEN;
	case APPLY_HITROLL:
		return SS_HITROLL;
	case APPLY_HITNDAM:
		return SS_HITNDAM;
	default:
		return -1;
	}
}

[[nodiscard]] std::string strip_mud_color(const char* s) {
	std::string out;
	if(s == nullptr) {
		return out;
	}
	for(const char* p = s; *p;) {
		if(p[0] == '$' && (p[1] == 'c' || p[1] == 'C') &&
		   std::isdigit(static_cast<unsigned char>(p[2])) &&
		   std::isdigit(static_cast<unsigned char>(p[3])) &&
		   std::isdigit(static_cast<unsigned char>(p[4])) &&
		   std::isdigit(static_cast<unsigned char>(p[5]))) {
			p += 6;
			continue;
		}
		out.push_back(*p++);
	}
	return out;
}

[[nodiscard]] std::string format_grant_by(int loc, int mod) {
	std::string out;
	if(const char* lab = apply_loc_label(loc); lab != nullptr) {
		out = lab;
	}
	else {
		out = "loc";
		out += std::to_string(loc);
	}
	out += " by ";
	const bool bitflag = (loc == APPLY_M_IMMUNE || loc == APPLY_IMMUNE || loc == APPLY_SUSC ||
						  loc == APPLY_SPELL || loc == APPLY_AFF2);
	if(bitflag) {
		out += format_affect_mod_text(loc, mod);
	}
	else {
		out += std::to_string(mod);
	}
	return out;
}

[[nodiscard]] std::string format_extra_flags_text(unsigned long extra, unsigned long extra2) {
	std::string a = format_bit_flag_names(extra, extra_bits);
	std::string b = format_bit_flag_names(extra2, extra_bits2);
	if(a == "NONE") {
		a.clear();
	}
	if(b == "NONE") {
		b.clear();
	}
	if(a.empty()) {
		return b;
	}
	if(b.empty()) {
		return a;
	}
	return a + " " + b;
}

[[nodiscard]] const char* item_type_name(int type_flag) {
	if(type_flag >= 0 && item_types[type_flag] != nullptr &&
	   item_types[type_flag][0] != '\n') {
		return item_types[type_flag];
	}
	return "?";
}

[[nodiscard]] const char* weapon_type_name(int wtype) {
	if(wtype >= 0 && aszWeaponType[wtype] != nullptr &&
	   aszWeaponType[wtype][0] != '\n') {
		return aszWeaponType[wtype];
	}
	return "";
}

struct StripHist {
	bool has_boot = false;
	int abs_hit = 0;
	int abs_mana = 0;
	int abs_move = 0;
	int abs_hr = 0;
	int abs_mr = 0;
	int abs_vr = 0;
	int credit_hit = 0;
	int credit_mana = 0;
	int credit_move = 0;
	int credit_hr = 0;
	int credit_mr = 0;
	int credit_vr = 0;
};

[[nodiscard]] int parse_tagged_int(const char* hay, const char* tag) {
	if(hay == nullptr || tag == nullptr) {
		return 0;
	}
	const char* p = std::strstr(hay, tag);
	if(p == nullptr) {
		return 0;
	}
	p += std::strlen(tag);
	while(*p == ' ') {
		++p;
	}
	return static_cast<int>(std::strtol(p, nullptr, 10));
}

void parse_boot_strip_detail(const char* detail, StripHist& h) {
	if(detail == nullptr) {
		return;
	}
	const char* p = detail;
	while((p = std::strstr(p, "-")) != nullptr) {
		++p;
		int* dest = nullptr;
		size_t skip = 0;
		if(std::strncmp(p, "HIT_REGEN ", 10) == 0) {
			dest = &h.abs_hr;
			skip = 10;
		}
		else if(std::strncmp(p, "MANA_REGEN ", 11) == 0) {
			dest = &h.abs_mr;
			skip = 11;
		}
		else if(std::strncmp(p, "MOVE_REGEN ", 11) == 0) {
			dest = &h.abs_vr;
			skip = 11;
		}
		else if(std::strncmp(p, "HIT ", 4) == 0) {
			dest = &h.abs_hit;
			skip = 4;
		}
		else if(std::strncmp(p, "MANA ", 5) == 0) {
			dest = &h.abs_mana;
			skip = 5;
		}
		else if(std::strncmp(p, "MOVE ", 5) == 0) {
			dest = &h.abs_move;
			skip = 5;
		}
		if(dest != nullptr) {
			*dest += static_cast<int>(std::strtol(p + skip, nullptr, 10));
			p += skip;
		}
	}
	if(const char* dc = std::strstr(detail, "delta_credit "); dc != nullptr) {
		h.credit_hit = parse_tagged_int(dc, "hit=");
		h.credit_mana = parse_tagged_int(dc, "mana=");
		h.credit_move = parse_tagged_int(dc, "move=");
		h.credit_hr = parse_tagged_int(dc, "hr=");
		h.credit_mr = parse_tagged_int(dc, "mr=");
		h.credit_vr = parse_tagged_int(dc, "vr=");
	}
}

[[nodiscard]] std::unordered_map<unsigned long long, StripHist>
load_boot_strip_history(MYSQL* h) {
	std::unordered_map<unsigned long long, StripHist> out;
	if(h == nullptr) {
		return out;
	}
	const char* sql =
		"SELECT instance_id, note, detail FROM object_instance_event "
		"WHERE kind = 'edit_pool' ORDER BY at ASC, id ASC";
	if(mysql_query(h, sql) != 0) {
		mudlog(LOG_SYSERR, "dbexport load strip hist: %s", mysql_error(h));
		return out;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(res == nullptr) {
		return out;
	}
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(row[0] == nullptr) {
			continue;
		}
		const unsigned long long iid = strtoull(row[0], nullptr, 10);
		if(iid == 0 || out.count(iid) != 0) {
			continue;
		}
		const char* note = row[1] ? row[1] : "";
		const char* detail = row[2] ? row[2] : "";
		const bool is_boot =
			std::strncmp(note, "credit to ", 10) == 0 ||
			std::strstr(detail, "strip pool from instance") != nullptr;
		if(!is_boot) {
			continue;
		}
		StripHist hist;
		hist.has_boot = true;
		parse_boot_strip_detail(detail, hist);
		out.emplace(iid, hist);
	}
	mysql_free_result(res);
	return out;
}

struct ProtoPool {
	int hit = 0;
	int mana = 0;
	int move = 0;
	int hr = 0;
	int mr = 0;
	int vr = 0;
	int dam = 0;
	int hitroll = 0;
	int hitndam = 0;
};

[[nodiscard]] ProtoPool load_proto_pool(int base_vnum) {
	ProtoPool p;
	if(base_vnum <= 0) {
		return p;
	}
	struct obj_data* proto = read_object(base_vnum, VIRTUAL);
	if(proto == nullptr) {
		return p;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = proto->affected[i].location;
		const int mod = proto->affected[i].modifier;
		switch(loc) {
		case APPLY_HIT:
			p.hit += mod;
			break;
		case APPLY_MANA:
			p.mana += mod;
			break;
		case APPLY_MOVE:
			p.move += mod;
			break;
		case APPLY_HIT_REGEN:
			p.hr += mod;
			break;
		case APPLY_MANA_REGEN:
			p.mr += mod;
			break;
		case APPLY_MOVE_REGEN:
			p.vr += mod;
			break;
		case APPLY_DAMROLL:
			p.dam += mod;
			break;
		case APPLY_HITROLL:
			p.hitroll += mod;
			break;
		case APPLY_HITNDAM:
			p.hitndam += mod;
			break;
		default:
			break;
		}
	}
	extract_obj(proto);
	return p;
}

void csv_append_int(std::string& line, int v) {
	line.push_back(',');
	line.append(std::to_string(v));
}

void csv_append_empty(std::string& line) {
	line.push_back(',');
}

void append_stat_block(std::string& line, const int checks[MAX_OBJ_AFFECT], int totale,
					   int originale, int editato) {
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		csv_append_int(line, checks[i]);
	}
	csv_append_int(line, totale);
	csv_append_int(line, originale);
	csv_append_int(line, editato);
}

/**
 * Vista foglio DB Edit: grants leggibili; per hp/mana/move/regen i totali usano
 * i valori pre-strip dal primo evento boot edit_pool (se presente).
 */
[[nodiscard]] ExportResult write_edits_sheet_csv(MYSQL* h, const fs::path& path) {
	ExportResult r;
	r.path = path.string();
	if(h == nullptr) {
		r.error = "parametri invalidi";
		return r;
	}

	const auto affects = load_all_instance_affects(h);
	const auto strips = load_boot_strip_history(h);
	std::unordered_map<int, ProtoPool> proto_cache;

	const char* sql =
		"SELECT id, base_vnum, legacy_edit_vnum, type_flag, extra_flags, extra_flags2, "
		"weight, cost, cost_per_day, value0, value1, value2, value3, "
		"obj_name, short_desc, owner_name, created_at, source "
		"FROM object_instance WHERE deleted = 0 ORDER BY id";
	if(mysql_query(h, sql) != 0) {
		r.error = mysql_error(h);
		return r;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(res == nullptr) {
		r.error = mysql_error(h);
		return r;
	}

	FILE* fp = std::fopen(path.string().c_str(), "wt");
	if(fp == nullptr) {
		mysql_free_result(res);
		r.error = "impossibile creare il file";
		return r;
	}

	static const char* kHeader =
		"Nome Oggetto,Tipo Oggetto,L'Oggetto e',Peso,Valore,Costo di Rent,Raro,"
		"Dado di Danno,Tipo di Danno,ACApply,Cariche,Spells,Livello Spells,"
		"Grants1,Grants2,Grants3,Grants4,Grants5,"
		"VNum originale,VNum Attuale,instance_id,Proprietario,Data Creazione Edit,"
		"Costo dell'Edit,Simbolo del Clan,source,strip_ricostruito,"
		"dam check 1,dam check 2,dam check 3,dam check 4,dam check 5,"
		"DANNO TOTALE,danno originale,danno editato,"
		"mana chk 1,mana chk 2,mana chk 3,mana chk 4,mana chk 5,"
		"Mana totale,mana originale,mana editato,"
		"hp check 1,hp check 2,hp check 3,hp check 4,hp check 5,"
		"hp TOTALE,hp originale,hp editati,"
		"mov check 1,mov check 2,mov check 3,mov check 4,mov check 5,"
		"Mov TOTALE,Mov originale,Mov editato,"
		"hp reg check 1,hp reg check 2,hp reg check 3,hp reg check 4,hp reg check 5,"
		"HP REG TOTALE,HP REG ORIGINALE,HP REG EDITATO,"
		"manareg check 1,manareg check 2,manareg check 3,manareg check 4,manareg check 5,"
		"MANA REG TOTALE,MANAREG ORIGINALE,MANA REG EDITATO,"
		"Mov reg check 1,Mov reg check 2,Mov reg check 3,Mov reg check 4,Mov reg check 5,"
		"Mov REG TOTALE,MOV REG ORIGINALE,MOV REG EDITATO,"
		"hitroll check 1,hitroll check 2,hitroll check 3,hitroll check 4,hitroll check 5,"
		"HIroll totale,Hitroll originale,hitroll editato,"
		"hit e dam check 1,hit e dam check 2,hit e dam check 3,hit e dam check 4,hit e dam check 5,"
		"hit e dam totali,hitndam originale,hitndam editato,"
		"credit_hit,credit_mana,credit_move,credit_hr,credit_mr,credit_vr\n";
	std::fputs(kHeader, fp);

	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(r.rows >= kDbExportMaxRows) {
			r.truncated = true;
			break;
		}
		const unsigned long long iid = row[0] ? strtoull(row[0], nullptr, 10) : 0;
		const int base_vnum = row[1] ? atoi(row[1]) : 0;
		const int legacy = row[2] ? atoi(row[2]) : 0;
		const int type_flag = row[3] ? atoi(row[3]) : 0;
		const unsigned long extra = row[4] ? strtoul(row[4], nullptr, 10) : 0;
		const unsigned long extra2 = row[5] ? strtoul(row[5], nullptr, 10) : 0;
		const int weight = row[6] ? atoi(row[6]) : 0;
		const int cost = row[7] ? atoi(row[7]) : 0;
		const int rent = row[8] ? atoi(row[8]) : 0;
		const int v0 = row[9] ? atoi(row[9]) : 0;
		const int v1 = row[10] ? atoi(row[10]) : 0;
		const int v2 = row[11] ? atoi(row[11]) : 0;
		const int v3 = row[12] ? atoi(row[12]) : 0;
		const char* obj_name = row[13] ? row[13] : "";
		const char* short_desc = row[14] ? row[14] : "";
		const char* owner = row[15] ? row[15] : "";
		const char* created = row[16] ? row[16] : "";
		const char* source = row[17] ? row[17] : "";

		const AffectRow* aff = nullptr;
		if(iid != 0) {
			if(const auto it = affects.find(iid); it != affects.end()) {
				aff = &it->second;
			}
		}

		int checks[SS_COUNT][MAX_OBJ_AFFECT] = {};
		int cur_tot[SS_COUNT] = {};
		std::string grants[MAX_OBJ_AFFECT];
		if(aff != nullptr) {
			for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
				const AffectSlot& s = (*aff)[static_cast<size_t>(i)];
				if(!s.set || (s.location == 0 && s.modifier == 0)) {
					continue;
				}
				grants[i] = format_grant_by(s.location, s.modifier);
				const int ss = sheet_stat_for_apply(s.location);
				if(ss >= 0) {
					checks[ss][i] = s.modifier;
					cur_tot[ss] += s.modifier;
				}
			}
		}

		ProtoPool proto {};
		if(base_vnum > 0) {
			if(const auto it = proto_cache.find(base_vnum); it != proto_cache.end()) {
				proto = it->second;
			}
			else {
				proto = load_proto_pool(base_vnum);
				proto_cache.emplace(base_vnum, proto);
			}
		}
		const int proto_of[SS_COUNT] = {proto.dam,	   proto.mana,	  proto.hit,
										proto.move,	   proto.hr,		  proto.mr,
										proto.vr,	   proto.hitroll, proto.hitndam};

		const StripHist* sh = nullptr;
		if(iid != 0) {
			if(const auto it = strips.find(iid); it != strips.end()) {
				sh = &it->second;
			}
		}
		const bool reconstructed = sh != nullptr && sh->has_boot;

		int tot[SS_COUNT];
		for(int ss = 0; ss < SS_COUNT; ++ss) {
			tot[ss] = cur_tot[ss];
		}
		if(reconstructed) {
			/* Per-campo: preferisci assoluto dal boot; altrimenti attuale+credit. */
			tot[SS_HIT] = sh->abs_hit != 0 ? sh->abs_hit :
						  (cur_tot[SS_HIT] + sh->credit_hit);
			tot[SS_MANA] = sh->abs_mana != 0 ? sh->abs_mana :
						   (cur_tot[SS_MANA] + sh->credit_mana);
			tot[SS_MOVE] = sh->abs_move != 0 ? sh->abs_move :
						   (cur_tot[SS_MOVE] + sh->credit_move);
			tot[SS_HIT_REGEN] = sh->abs_hr != 0 ? sh->abs_hr :
								(cur_tot[SS_HIT_REGEN] + sh->credit_hr);
			tot[SS_MANA_REGEN] = sh->abs_mr != 0 ? sh->abs_mr :
								 (cur_tot[SS_MANA_REGEN] + sh->credit_mr);
			tot[SS_MOVE_REGEN] = sh->abs_vr != 0 ? sh->abs_vr :
								 (cur_tot[SS_MOVE_REGEN] + sh->credit_vr);
		}

		std::string nome = obj_name;
		if(nome.empty()) {
			nome = strip_mud_color(short_desc);
		}

		const bool clan = (type_flag == ITEM_CLAN_SYMBOL) ||
						  (source && std::strcmp(source, "clan_symbol") == 0) ||
						  (legacy > 0 && clan_symbol_is_listed_vnum(
											 static_cast<unsigned>(legacy))) ||
						  (base_vnum > 0 && clan_symbol_is_listed_vnum(
												static_cast<unsigned>(base_vnum)));

		std::string line;
		csv_append_field(line, nome.c_str());
		line.push_back(',');
		csv_append_field(line, item_type_name(type_flag));
		line.push_back(',');
		{
			const std::string ex = format_extra_flags_text(extra, extra2);
			csv_append_field(line, ex.c_str());
		}
		csv_append_int(line, weight);
		csv_append_int(line, cost);
		csv_append_int(line, rent);
		line.push_back(',');
		csv_append_field(line, (extra & ITEM_IMMUNE) ? "TRUE" : "FALSE");

		if(type_flag == ITEM_WEAPON) {
			std::ostringstream dice;
			dice << v1 << "d" << v2;
			line.push_back(',');
			csv_append_field(line, dice.str().c_str());
			line.push_back(',');
			csv_append_field(line, weapon_type_name(v3));
			csv_append_empty(line); /* ACApply */
			csv_append_empty(line); /* Cariche */
			csv_append_empty(line); /* Spells */
			csv_append_empty(line); /* Livello Spells */
		}
		else if(type_flag == ITEM_ARMOR || type_flag == ITEM_CLAN_SYMBOL) {
			csv_append_empty(line);
			csv_append_empty(line);
			csv_append_int(line, v0); /* ACApply */
			csv_append_empty(line);
			csv_append_empty(line);
			csv_append_empty(line);
		}
		else if(type_flag == ITEM_STAFF || type_flag == ITEM_WAND) {
			csv_append_empty(line);
			csv_append_empty(line);
			csv_append_empty(line);
			csv_append_int(line, v0); /* cariche (tipico) */
			csv_append_int(line, v3); /* spell id grezzo */
			csv_append_int(line, v2); /* livello grezzo */
		}
		else {
			csv_append_empty(line);
			csv_append_empty(line);
			csv_append_empty(line);
			csv_append_empty(line);
			csv_append_empty(line);
			csv_append_empty(line);
		}

		for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
			line.push_back(',');
			csv_append_field(line, grants[i].c_str());
		}

		csv_append_int(line, base_vnum);
		csv_append_int(line, legacy);
		line.push_back(',');
		line.append(std::to_string(iid));
		line.push_back(',');
		csv_append_field(line, owner);
		line.push_back(',');
		csv_append_field(line, created);
		csv_append_empty(line); /* Costo dell'Edit: non in DB */
		line.push_back(',');
		csv_append_field(line, clan ? "Y" : "");
		line.push_back(',');
		csv_append_field(line, source);
		line.push_back(',');
		csv_append_field(line, reconstructed ? "Y" : "N");

		for(int ss = 0; ss < SS_COUNT; ++ss) {
			append_stat_block(line, checks[ss], tot[ss], proto_of[ss],
							  tot[ss] - proto_of[ss]);
		}

		if(reconstructed) {
			csv_append_int(line, sh->credit_hit);
			csv_append_int(line, sh->credit_mana);
			csv_append_int(line, sh->credit_move);
			csv_append_int(line, sh->credit_hr);
			csv_append_int(line, sh->credit_mr);
			csv_append_int(line, sh->credit_vr);
		}
		else {
			csv_append_int(line, 0);
			csv_append_int(line, 0);
			csv_append_int(line, 0);
			csv_append_int(line, 0);
			csv_append_int(line, 0);
			csv_append_int(line, 0);
		}

		line.push_back('\n');
		std::fputs(line.c_str(), fp);
		++r.rows;
	}

	std::fclose(fp);
	mysql_free_result(res);
	r.ok = true;
	return r;
}

void send_export_result(char_data* ch, const ExportResult& r, const char* label) {
	if(ch == nullptr) {
		return;
	}
	if(!r.ok) {
		std::ostringstream os;
		os << "dbexport " << (label ? label : "") << " fallito: " << r.error << "\n\r";
		send_to_char(os.str().c_str(), ch);
		return;
	}
	std::ostringstream os;
	os << "dbexport " << (label ? label : "") << ": " << r.rows << " righe -> " << r.path;
	if(r.skipped_cols > 0) {
		os << " (saltate " << r.skipped_cols << " colonne secret/pii)";
	}
	if(r.truncated) {
		os << " [TRONCATO a " << kDbExportMaxRows << " righe]";
	}
	os << "\n\r";
	send_to_char(os.str().c_str(), ch);
	{
		std::ostringstream log;
		log << "dbexport by " << (GET_NAME(ch) ? GET_NAME(ch) : "?") << ": "
			<< (label ? label : "?") << " (" << r.rows << " rows) -> " << r.path;
		mudlog(LOG_CHECK, "%s", log.str().c_str());
	}
}

void send_dbexport_usage(char_data* ch) {
	send_to_char(
		"Uso:\n\r"
		"  dbexport list\n\r"
		"  dbexport dump [pii]\n\r"
		"  dbexport table <nome> [pii]\n\r"
		"  dbexport <query> [pii]\n\r"
		"\n\r"
		"CSV in exports/ (cwd lib/). Password/authcode mai esportati.\n\r"
		"Con pii: include email/realname.\n\r",
		ch);
}

void send_dbexport_list(char_data* ch) {
	send_to_char("Query nominate:\n\r", ch);
	for(const NamedExport& q : kNamedExports) {
		std::ostringstream os;
		os << "  " << q.name << " - " << q.help << "\n\r";
		send_to_char(os.str().c_str(), ch);
	}
	send_to_char("Tabelle (dbexport table / dump):\n\r", ch);
	std::ostringstream line;
	line << "  ";
	bool first = true;
	for(const char* t : kAllowedTables) {
		if(!first) {
			line << ", ";
		}
		first = false;
		line << t;
	}
	line << "\n\r";
	send_to_char(line.str().c_str(), ch);
}

[[nodiscard]] const NamedExport* find_named(const char* name) {
	for(const NamedExport& q : kNamedExports) {
		if(str_ieq(q.name, name)) {
			return &q;
		}
	}
	return nullptr;
}

[[nodiscard]] bool parse_pii_flag(const char* arg) {
	return arg != nullptr && *arg &&
		   (str_ieq(arg, "pii") || str_ieq(arg, "private") || str_ieq(arg, "emails"));
}

void do_export_named(char_data* ch, const NamedExport& q, bool include_pii) {
	std::string err;
	if(!ensure_exports_root(err)) {
		send_to_char(("dbexport: non posso creare exports/: " + err + "\n\r").c_str(), ch);
		return;
	}
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		send_to_char("Nessuna connessione MySQL.\n\r", ch);
		return;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const fs::path path =
			fs::path(kDbExportDir) / (std::string(q.name) + "_" + timestamp_stamp() + ".csv");
		ExportResult r;
		if(str_ieq(q.name, "edits")) {
			r = write_edits_with_affects_csv(h, false, path, include_pii);
		}
		else if(str_ieq(q.name, "edits_deleted")) {
			r = write_edits_with_affects_csv(h, true, path, include_pii);
		}
		else if(str_ieq(q.name, "edits_sheet")) {
			r = write_edits_sheet_csv(h, path);
		}
		else if(q.sql != nullptr) {
			r = write_query_csv(h, q.sql, path, include_pii);
		}
		else {
			r.error = "query non configurata";
		}
		send_export_result(ch, r, q.name);
	}
	catch(const odb::exception& e) {
		send_to_char("dbexport: errore ODB.\n\r", ch);
		mudlog(LOG_SYSERR, "dbexport named: %s", e.what());
	}
}

void do_export_table(char_data* ch, const char* table, bool include_pii) {
	if(!table_allowed(table)) {
		send_to_char(
			"Tabella non ammessa. Usa dbexport list per la whitelist.\n\r", ch);
		return;
	}
	std::string err;
	if(!ensure_exports_root(err)) {
		send_to_char(("dbexport: non posso creare exports/: " + err + "\n\r").c_str(), ch);
		return;
	}
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		send_to_char("Nessuna connessione MySQL.\n\r", ch);
		return;
	}
	/* table_allowed ha gia filtrato charset -> safe in backtick. */
	std::string sql = "SELECT * FROM `";
	sql += table;
	sql += "`";
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const fs::path path =
			fs::path(kDbExportDir) / (std::string(table) + "_" + timestamp_stamp() + ".csv");
		const ExportResult r = write_query_csv(h, sql.c_str(), path, include_pii);
		send_export_result(ch, r, table);
	}
	catch(const odb::exception& e) {
		send_to_char("dbexport: errore ODB.\n\r", ch);
		mudlog(LOG_SYSERR, "dbexport table: %s", e.what());
	}
}

void do_export_dump(char_data* ch, bool include_pii) {
	std::string err;
	if(!ensure_exports_root(err)) {
		send_to_char(("dbexport: non posso creare exports/: " + err + "\n\r").c_str(), ch);
		return;
	}
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		send_to_char("Nessuna connessione MySQL.\n\r", ch);
		return;
	}

	const std::string stamp = timestamp_stamp();
	const fs::path dir = fs::path(kDbExportDir) / ("dump_" + stamp);
	boost::system::error_code ec;
	fs::create_directories(dir, ec);
	if(ec) {
		send_to_char(("dbexport dump: mkdir fallita: " + ec.message() + "\n\r").c_str(), ch);
		return;
	}

	unsigned ok_n = 0;
	unsigned fail_n = 0;
	unsigned long long total_rows = 0;
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		for(const char* table : kAllowedTables) {
			std::string sql = "SELECT * FROM `";
			sql += table;
			sql += "`";
			const fs::path path = dir / (std::string(table) + ".csv");
			const ExportResult r = write_query_csv(h, sql.c_str(), path, include_pii);
			if(r.ok) {
				++ok_n;
				total_rows += r.rows;
			}
			else {
				++fail_n;
				std::ostringstream os;
				os << "  skip " << table << ": " << r.error << "\n\r";
				send_to_char(os.str().c_str(), ch);
			}
		}
	}
	catch(const odb::exception& e) {
		send_to_char("dbexport dump: errore ODB.\n\r", ch);
		mudlog(LOG_SYSERR, "dbexport dump: %s", e.what());
		return;
	}

	std::ostringstream os;
	os << "dbexport dump: " << ok_n << " tabelle ok";
	if(fail_n > 0) {
		os << ", " << fail_n << " fallite/assenti";
	}
	os << ", " << total_rows << " righe totali -> " << dir.string() << "\n\r";
	send_to_char(os.str().c_str(), ch);
	{
		std::ostringstream log;
		log << "dbexport dump by " << (GET_NAME(ch) ? GET_NAME(ch) : "?") << ": "
			<< ok_n << " tables, " << total_rows << " rows -> " << dir.string();
		mudlog(LOG_CHECK, "%s", log.str().c_str());
	}
}

#endif /* USE_MYSQL */

} // namespace

ACTION_FUNC(do_dbexport) {
	(void)cmd;
	if(ch == nullptr || IS_NPC(ch)) {
		return;
	}
#if !USE_MYSQL
	send_to_char("MySQL non abilitato.\n\r", ch);
	return;
#else
	if(GetMaxLevel(ch) < QUESTMASTER) {
		send_to_char("Non hai il livello per usare dbexport.\n\r", ch);
		return;
	}

	char tok1[MAX_INPUT_LENGTH];
	char tok2[MAX_INPUT_LENGTH];
	char tok3[MAX_INPUT_LENGTH];
	const char* rest = one_argument(arg, tok1);
	rest = one_argument(rest, tok2);
	one_argument(rest, tok3);

	if(!*tok1 || str_ieq(tok1, "help") || str_ieq(tok1, "?")) {
		send_dbexport_usage(ch);
		return;
	}
	if(str_ieq(tok1, "list")) {
		send_dbexport_list(ch);
		return;
	}

	bool include_pii = parse_pii_flag(tok2) || parse_pii_flag(tok3);

	if(str_ieq(tok1, "dump")) {
		do_export_dump(ch, include_pii || parse_pii_flag(tok2));
		return;
	}
	if(str_ieq(tok1, "table")) {
		if(!*tok2) {
			send_to_char("Uso: dbexport table <nome> [pii]\n\r", ch);
			return;
		}
		include_pii = parse_pii_flag(tok3);
		do_export_table(ch, tok2, include_pii);
		return;
	}

	if(const NamedExport* q = find_named(tok1); q != nullptr) {
		do_export_named(ch, *q, include_pii);
		return;
	}

	send_dbexport_usage(ch);
	send_to_char("Sconosciuto. Prova: dbexport list\n\r", ch);
#endif
}

} // namespace Alarmud
