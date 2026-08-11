/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>
#include <filesystem>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
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
#include "db.hpp"
#include "auction.hpp"
#include "comm.hpp"
#include "act.other.hpp"
#include "mobact.hpp"
#include "modify.hpp"
#include "mail.hpp"
#include "fight.hpp"
#include "act.social.hpp"
#include "gilde.hpp"
#include "interpreter.hpp"
#include "spell_parser.hpp"
#include "reception.hpp"
#include "server_text.hpp"
#include "weather.hpp"
#include "handler.hpp"
#include "opinion.hpp"
#include "spell_list.hpp"

#include "Registered.hpp"
#include "regen.hpp"
#include "script.hpp"
#include "Sql.hpp"
#include "utility.hpp"
#include "odb/account-odb.hxx" //Sirio per gestione registrazione pg su db
#include "multiclass.hpp" //Sirio per gestione registrazione pg su db
#include "toon_migration.hpp"
#include "procarea.hpp"
#include "object_instance.hpp"
#include "clan_symbol.hpp"
#include "edit_pool.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <odb/exception.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/transaction.hxx>
#include <mysql/mysql.h>

namespace Alarmud {

// Helper function to get executable directory from argv[0] using C++17 filesystem
static std::string get_executable_dir(const char* argv0) {
    namespace fs = std::filesystem;
    
    try {
        // If argv0 is an absolute path, use it directly
        if (argv0 && argv0[0] == '/') {
            fs::path exec_path(argv0);
            return exec_path.parent_path().string();
        }
        
        // Try /proc/self/exe (Linux) for symlinks
        if (fs::exists("/proc/self/exe")) {
            fs::path exec_path = fs::read_symlink("/proc/self/exe");
            return exec_path.parent_path().string();
        }
        
        // Fallback: use current directory
        return fs::current_path().string();
    } catch (const fs::filesystem_error& e) {
        // Ultimate fallback: use current directory
        return ".";
    }
}

// Dynamic path variables
std::string PAGES_DIR;
std::string POSEMESS_FILE;
std::string MESS_FILE;
std::string SOCMESS_FILE;
std::string LOGIN_FILE;
std::string CREDITS_FILE;
std::string NEWS_FILE;
std::string WIZNEWS_FILE;
std::string MOTD_FILE;
std::string WIZ_MOTD_FILE;
std::string HELP_KWRD_FILE;
std::string HELP_PAGE_FILE;
std::string WIZ_HELP_FILE;
std::string INFO_FILE;

void init_paths(const char* argv0) {
    std::string exec_dir = get_executable_dir(argv0);
    PAGES_DIR = exec_dir + "/pages";
    
    POSEMESS_FILE = PAGES_DIR + "/myst.pos";
    MESS_FILE = PAGES_DIR + "/myst.dam";
    SOCMESS_FILE = PAGES_DIR + "/myst.act";
    LOGIN_FILE = PAGES_DIR + "/login";
    CREDITS_FILE = PAGES_DIR + "/credits";
    NEWS_FILE = PAGES_DIR + "/news";
    WIZNEWS_FILE = PAGES_DIR + "/wiznews";
    MOTD_FILE = PAGES_DIR + "/motd";
    WIZ_MOTD_FILE = PAGES_DIR + "/wizmotd";
    HELP_KWRD_FILE = PAGES_DIR + "/helptbl";
    HELP_PAGE_FILE = PAGES_DIR + "/help";
    WIZ_HELP_FILE = PAGES_DIR + "/wizhelptbl";
    INFO_FILE = PAGES_DIR + "/info";
}

long long sql_to_ll(const char* s, long long fallback = 0) {
	if(!s) {
		return fallback;
	}
	char* end = nullptr;
	const long long v = std::strtoll(s, &end, 10);
	return (end == s) ? fallback : v;
}

bool mysql_query_select(DB* db, const std::string& sql, MYSQL_RES*& out_res) {
	out_res = nullptr;
	if(!db) {
		return false;
	}
	/* Pool may hand back a socket killed by wait_timeout (MySQL 4031); ODB ping
	 * then throws. Catch + one retry so login/load does not abort the process. */
	for(int attempt = 0; attempt < 2; ++attempt) {
		try {
			odb::connection_ptr cp(db->connection());
			auto& mc = static_cast<odb::mysql::connection&>(*cp);
			MYSQL* h = mc.handle();
			if(mysql_query(h, sql.c_str()) != 0) {
				mudlog(LOG_SYSERR, "mysql_query_select: %s", mysql_error(h));
				return false;
			}
			out_res = mysql_store_result(h);
			return true;
		}
		catch(const odb::exception& e) {
			const char* phase = (attempt == 0) ? " (will retry)" : " (giving up)";
			mudlog(LOG_SYSERR, "mysql_query_select: odb%s: %s", phase, e.what());
			if(attempt != 0) {
				return false;
			}
		}
	}
	return false;
}

bool mysql_query_select_tx(DB* db, const std::string& sql, MYSQL_RES*& out_res) {
	out_res = nullptr;
	if(!db) {
		return false;
	}
	const bool in_tx = odb::transaction::has_current();
	const int max_attempts = in_tx ? 1 : 2;
	for(int attempt = 0; attempt < max_attempts; ++attempt) {
		try {
			odb::connection_ptr owned;
			odb::connection& conn = in_tx ? odb::transaction::current().connection()
										  : *(owned = db->connection());
			auto& mc = static_cast<odb::mysql::connection&>(conn);
			MYSQL* h = mc.handle();
			if(mysql_query(h, sql.c_str()) != 0) {
				mudlog(LOG_SYSERR, "mysql_query_select_tx: %s", mysql_error(h));
				return false;
			}
			out_res = mysql_store_result(h);
			return true;
		}
		catch(const odb::exception& e) {
			const char* phase =
				(!in_tx && attempt == 0) ? " (will retry)" : " (giving up)";
			mudlog(LOG_SYSERR, "mysql_query_select_tx: odb%s: %s", phase, e.what());
			if(in_tx || attempt != 0) {
				return false;
			}
		}
	}
	return false;
}

std::string db_sql_escape(const char* s) {
	if(!s) {
		return "";
	}
	std::string out;
	out.reserve(std::strlen(s) * 2 + 4);
	for(const char* p = s; *p; ++p) {
		if(*p == '\'' || *p == '\\') {
			out.push_back('\\');
		}
		out.push_back(*p);
	}
	return out;
}

std::string db_sql_literal(const char* s, bool allow_null = false) {
	if(!s || !*s) {
		return allow_null ? "NULL" : "''";
	}
	return "'" + db_sql_escape(s) + "'";
}

struct char_data* save_char_resolve_pc(struct char_data* ch) {
	if(!ch || !IS_PC(ch)) {
		return nullptr;
	}
	if(IS_POLY(ch)) {
		if(!ch->desc || !ch->desc->original) {
			return nullptr;
		}
		return ch->desc->original;
	}
	/* Ghost/LD without desc: save the real PC (pwd in st; empty lasthost if absent). */
	if(!ch->desc) {
		return ch;
	}
	return ch;
}

void db_write_character_snapshot_tx(DB* db, const std::string& toon_id,
									const struct char_file_u& st) {
	db->execute(("DELETE FROM character_affects WHERE toon_id = " + toon_id).c_str());
	db->execute(("DELETE FROM character_skills WHERE toon_id = " + toon_id).c_str());
	db->execute(("DELETE FROM character_classes WHERE toon_id = " + toon_id).c_str());
	db->execute(("DELETE FROM character_stats WHERE toon_id = " + toon_id).c_str());
	db->execute(("DELETE FROM character_core WHERE toon_id = " + toon_id).c_str());

	const int wimpy = std::atoi(st.WimpyLevel);
	std::ostringstream core;
	core << "INSERT INTO character_core (toon_id, description, class_primary, sex, race, "
			"birth, played, last_logon, weight, height, hometown, "
			"talks_0, talks_1, talks_2, speaks, user_flags, extra_flags, age_modifier, "
			"authcode, wimpy_level, load_room, start_room, spells_to_learn, alignment, act, "
			"affected_by, affected_by2, condition_drunk, condition_full, condition_thirst, "
			"save_throw_0, save_throw_1, save_throw_2, save_throw_3, save_throw_4, "
			"save_throw_5, save_throw_6, save_throw_7) VALUES ("
		 << toon_id << ','
		 << db_sql_literal(st.description, true) << ','
		 << st.iClass << ',' << static_cast<int>(st.sex) << ',' << st.race << ','
		 << st.birth << ',' << st.played << ',' << st.last_logon << ','
		 << st.weight << ',' << st.height << ',' << st.hometown << ','
		 << (st.talks[0] ? 1 : 0) << ',' << (st.talks[1] ? 1 : 0) << ','
		 << (st.talks[2] ? 1 : 0) << ',' << st.speaks << ',' << st.user_flags << ','
		 << st.extra_flags << ',' << st.agemod << ','
		 << db_sql_literal(st.authcode, false) << ',' << wimpy << ',' << st.load_room << ','
		 << st.startroom << ',' << static_cast<int>(st.spells_to_learn) << ','
		 << st.alignment << ',' << st.act << ',' << st.affected_by << ','
		 << st.affected_by2 << ',' << st.conditions[0] << ',' << st.conditions[1] << ','
		 << st.conditions[2];
	for(int i = 0; i < MAX_SAVES; ++i) {
		core << ',' << st.apply_saving_throw[i];
	}
	core << ')';
	db->execute(core.str().c_str());

	std::ostringstream stats;
	stats << "INSERT INTO character_stats (toon_id, str, str_add, intel, wis, dex, con, chr, "
			 "extra, extra2, mana, max_mana, mana_gain, hit, max_hit, hit_gain, move, "
			 "max_move, move_gain, p_rune_dei, points_extra1, points_extra2, points_extra3, "
			 "armor, gold, bank_gold, exp, true_exp, extra_dual, hitroll, damroll, libero, "
			 "edit_hp, edit_mana, edit_move, edit_hp_regen, edit_mana_regen, edit_move_regen, "
			 "overedit_hp, overedit_mana, overedit_move, overedit_hp_regen, "
			 "overedit_mana_regen, overedit_move_regen, edit_pool_migrated) "
			 "VALUES ("
		  << toon_id << ',' << static_cast<int>(st.abilities.str) << ','
		  << static_cast<int>(st.abilities.str_add) << ','
		  << static_cast<int>(st.abilities.intel) << ','
		  << static_cast<int>(st.abilities.wis) << ','
		  << static_cast<int>(st.abilities.dex) << ','
		  << static_cast<int>(st.abilities.con) << ','
		  << static_cast<int>(st.abilities.chr) << ','
		  << static_cast<int>(st.abilities.extra) << ','
		  << static_cast<int>(st.abilities.extra2) << ','
		  << st.points.mana << ',' << st.points.max_mana << ','
		  << static_cast<int>(st.points.mana_gain) << ',' << st.points.hit << ','
		  << st.points.max_hit << ',' << static_cast<int>(st.points.hit_gain) << ','
		  << st.points.move << ',' << st.points.max_move << ','
		  << static_cast<int>(st.points.move_gain) << ',' << st.points.pRuneDei << ','
		  << st.points.extra1 << ',' << st.points.extra2 << ','
		  << static_cast<int>(st.points.extra3) << ',' << st.points.armor << ','
		  << st.points.gold << ',' << st.points.bankgold << ',' << st.points.exp << ','
		  << st.points.true_exp << ',' << st.points.extra_dual << ','
		  << static_cast<int>(st.points.hitroll) << ','
		  << static_cast<int>(st.points.damroll) << ','
		  << static_cast<int>(st.points.libero) << ','
		  << st.edit_pool.edit_hp << ',' << st.edit_pool.edit_mana << ','
		  << st.edit_pool.edit_move << ',' << st.edit_pool.edit_hp_regen << ','
		  << st.edit_pool.edit_mana_regen << ',' << st.edit_pool.edit_move_regen << ','
		  << st.edit_pool.overedit_hp << ',' << st.edit_pool.overedit_mana << ','
		  << st.edit_pool.overedit_move << ',' << st.edit_pool.overedit_hp_regen << ','
		  << st.edit_pool.overedit_mana_regen << ','
		  << st.edit_pool.overedit_move_regen << ','
		  << static_cast<int>(st.edit_pool.migrated) << ')';
	db->execute(stats.str().c_str());

	for(int i = 0; i < MAX_CLASS; ++i) {
		if(st.level[i] == 0) {
			continue;
		}
		std::ostringstream classes;
		classes << "INSERT INTO character_classes (toon_id, class_index, level) VALUES ("
				<< toon_id << ',' << i << ',' << static_cast<int>(st.level[i]) << ')';
		db->execute(classes.str().c_str());
	}

	for(int i = 0; i < MAX_SKILLS; ++i) {
		const char_skill_data& sk = st.skills[i];
		if(sk.learned == 0 && sk.flags == 0 && sk.special == 0 && sk.nummem == 0) {
			continue;
		}
		std::ostringstream skills;
		skills << "INSERT INTO character_skills "
				  "(toon_id, skill_id, learned, flags, special, nummem) VALUES ("
			   << toon_id << ',' << i << ',' << static_cast<int>(sk.learned) << ','
			   << static_cast<int>(sk.flags) << ',' << static_cast<int>(sk.special) << ','
			   << static_cast<int>(sk.nummem) << ')';
		db->execute(skills.str().c_str());
	}

	for(int i = 0; i < MAX_AFFECT; ++i) {
		const affected_type_u& af = st.affected[i];
		if(af.type == 0 || af.duration <= 0) {
			continue;
		}
		std::ostringstream affects;
		affects << "INSERT INTO character_affects "
				   "(toon_id, slot, type, duration, modifier, location, bitvector) VALUES ("
				<< toon_id << ',' << i << ',' << af.type << ',' << af.duration << ','
				<< af.modifier << ',' << af.location << ',' << af.bitvector << ')';
		db->execute(affects.str().c_str());
	}
}

void db_update_toon_registry_tx(DB* db, const std::string& toon_id, struct char_data* ch,
								const struct char_file_u& st) {
	const int level = static_cast<int>(st.level[BestClassIND(ch)]);
	const char* title = st.title[0] ? st.title : "";
	const char* host = (ch->desc && ch->desc->host[0]) ? ch->desc->host : "";
	std::ostringstream sql;
	sql << "UPDATE toon SET password=" << db_sql_literal(st.pwd, false) << ",title="
		<< db_sql_literal(title, false) << ",level=" << level << ",lastlogin=NOW(),lasthost="
		<< db_sql_literal(host, false) << " WHERE id=" << toon_id;
	db->execute(sql.str().c_str());
}

namespace {

constexpr int kInventoryInsertBatch = 50;

std::string inventory_insert_values(const std::string& toon_id, int list_index,
									  const obj_file_elem& o, bool soft_delete,
									  bool parent_supported, int parent_list_index,
									  unsigned long long parent_id,
									  unsigned long long instance_id) {
	std::ostringstream row;
	row << '(' << toon_id << ',' << list_index << ',' << o.item_number << ',' << o.value[0] << ','
		<< o.value[1] << ',' << o.value[2] << ',' << o.value[3] << ',' << o.extra_flags << ','
		<< o.extra_flags2 << ',' << o.weight << ',' << o.timer << ',' << o.bitvector << ','
		<< db_sql_literal(o.name, false) << ',' << db_sql_literal(o.sd, false) << ','
		<< db_sql_literal(o.desc, false) << ',' << static_cast<int>(o.wearpos) << ','
		<< static_cast<int>(o.depth);
	if(parent_supported) {
		if(parent_id > 0) {
			row << ',' << parent_id;
		}
		else if(parent_list_index < 0) {
			row << ",NULL";
		}
		else {
			row << ",(SELECT ci_p.id FROM character_inventory ci_p WHERE ci_p.toon_id=" << toon_id
				<< " AND ci_p.list_index=" << parent_list_index;
			if(soft_delete) {
				row << " AND (ci_p.deleted=0 OR ci_p.deleted IS NULL)";
			}
			row << " LIMIT 1)";
		}
	}
	if(instance_id > 0) {
		row << ',' << instance_id;
	}
	else {
		row << ",NULL";
	}
	if(soft_delete) {
		row << ",0,NULL,NULL";
	}
	row << ')';
	return row.str();
}

const char* inventory_insert_columns(bool soft_delete, bool parent_supported) {
	if(parent_supported && soft_delete) {
		return "INSERT INTO character_inventory (toon_id, list_index, item_number, value0, "
			   "value1, value2, value3, extra_flags, extra_flags2, weight, timer, bitvector, "
			   "obj_name, short_desc, description, wear_pos, depth, parent_inventory_id, "
			   "instance_id, deleted, deleted_on, deleted_for) VALUES ";
	}
	if(parent_supported) {
		return "INSERT INTO character_inventory (toon_id, list_index, item_number, value0, "
			   "value1, value2, value3, extra_flags, extra_flags2, weight, timer, bitvector, "
			   "obj_name, short_desc, description, wear_pos, depth, parent_inventory_id, "
			   "instance_id) VALUES ";
	}
	if(soft_delete) {
		return "INSERT INTO character_inventory (toon_id, list_index, item_number, value0, "
			   "value1, value2, value3, extra_flags, extra_flags2, weight, timer, bitvector, "
			   "obj_name, short_desc, description, wear_pos, depth, instance_id, deleted, "
			   "deleted_on, deleted_for) VALUES ";
	}
	return "INSERT INTO character_inventory (toon_id, list_index, item_number, value0, "
		   "value1, value2, value3, extra_flags, extra_flags2, weight, timer, bitvector, "
		   "obj_name, short_desc, description, wear_pos, depth, instance_id) VALUES ";
}

void execute_values_batch(DB* db, const char* prefix, const std::vector<std::string>& rows,
						  int batch_size) {
	if(rows.empty()) {
		return;
	}
	for(size_t off = 0; off < rows.size(); off += static_cast<size_t>(batch_size)) {
		std::ostringstream sql;
		sql << prefix << rows[off];
		const size_t end = std::min(off + static_cast<size_t>(batch_size), rows.size());
		for(size_t i = off + 1; i < end; ++i) {
			sql << ',' << rows[i];
		}
		db->execute(sql.str().c_str());
	}
}

void execute_union_batch(DB* db, const char* prefix, const std::vector<std::string>& parts,
						 int batch_size) {
	if(parts.empty()) {
		return;
	}
	for(size_t off = 0; off < parts.size(); off += static_cast<size_t>(batch_size)) {
		std::ostringstream sql;
		sql << prefix << parts[off];
		const size_t end = std::min(off + static_cast<size_t>(batch_size), parts.size());
		for(size_t i = off + 1; i < end; ++i) {
			sql << " UNION ALL " << parts[i];
		}
		db->execute(sql.str().c_str());
	}
}

const char* inventory_affect_insert_select_prefix() {
	return "INSERT INTO character_inventory_affect (inventory_id, affect_slot, location, "
		   "modifier) ";
}

std::string inventory_affect_select_row(const std::string& toon_id, int list_index,
										int affect_slot, int location, int modifier,
										bool soft_delete_supported) {
	std::ostringstream row;
	row << "SELECT ci.id, " << affect_slot << ", " << location << ", " << modifier
		<< " FROM character_inventory ci WHERE ci.toon_id = " << toon_id
		<< " AND ci.list_index = " << list_index;
	if(soft_delete_supported) {
		row << " AND (ci.deleted = 0 OR ci.deleted IS NULL)";
	}
	return row.str();
}

void insert_inventory_affects_batch_tx(DB* db, const std::string& toon_id, int object_count,
									   const struct obj_file_u& rent,
									   bool soft_delete_supported) {
	std::vector<std::string> select_rows;
	for(int i = 0; i < object_count; ++i) {
		for(int a = 0; a < MAX_OBJ_AFFECT; ++a) {
			const obj_affected_type& oa = rent.objects[i].affected[a];
			if(oa.location == 0 && oa.modifier == 0) {
				continue;
			}
			select_rows.push_back(inventory_affect_select_row(
				toon_id, i, a, static_cast<int>(oa.location), static_cast<int>(oa.modifier),
				soft_delete_supported));
		}
	}
	execute_union_batch(db, inventory_affect_insert_select_prefix(), select_rows,
						kInventoryInsertBatch);
}

bool inventory_parent_column_exists_tx(DB* db) {
	MYSQL_RES* cols_res = nullptr;
	const std::string cols_sql =
		"SELECT COUNT(*) FROM information_schema.COLUMNS "
		"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'character_inventory' "
		"AND COLUMN_NAME = 'parent_inventory_id'";
	if(!mysql_query_select(db, cols_sql, cols_res) || !cols_res) {
		return false;
	}
	MYSQL_ROW cols_row = mysql_fetch_row(cols_res);
	const bool exists = cols_row && sql_to_ll(cols_row[0], 0) >= 1;
	mysql_free_result(cols_res);
	return exists;
}

bool inventory_parent_index_exists_tx(DB* db) {
	MYSQL_RES* idx_res = nullptr;
	const std::string idx_sql =
		"SELECT COUNT(*) FROM information_schema.STATISTICS "
		"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'character_inventory' "
		"AND INDEX_NAME = 'idx_inventory_parent'";
	if(!mysql_query_select(db, idx_sql, idx_res) || !idx_res) {
		return false;
	}
	MYSQL_ROW idx_row = mysql_fetch_row(idx_res);
	const bool exists = idx_row && sql_to_ll(idx_row[0], 0) >= 1;
	mysql_free_result(idx_res);
	return exists;
}

bool mysql_execute_ddl_db(DB* db, const char* sql) {
	if(!db || !sql || !*sql) {
		return false;
	}
	if(odb::transaction::has_current()) {
		mudlog(LOG_SYSERR,
			   "mysql_execute_ddl_db: refused DDL inside open transaction: %s", sql);
		return false;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		db->execute(sql);
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "mysql_execute_ddl_db: %s", e.what());
		return false;
	}
}

bool mysql_execute_sql_db(DB* db, const char* sql) {
	if(!db || !sql || !*sql) {
		return false;
	}
	try {
		if(odb::transaction::has_current()) {
			db->execute(sql);
			return true;
		}
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		db->execute(sql);
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "mysql_execute_sql_db: %s", e.what());
		return false;
	}
}

bool inventory_parent_id_supported_tx(DB* db) {
	static int parent_col_cache = -1;
	if(parent_col_cache == 1) {
		return true;
	}
	if(!db) {
		return false;
	}

	if(inventory_parent_column_exists_tx(db)) {
		if(!inventory_parent_index_exists_tx(db) && !odb::transaction::has_current()) {
			mudlog(LOG_SAVE,
				   "inventory_parent_id_supported_tx: adding idx_inventory_parent index");
			mysql_execute_ddl_db(db, "ALTER TABLE character_inventory "
									   "ADD INDEX idx_inventory_parent (parent_inventory_id)");
		}
		parent_col_cache = 1;
		return true;
	}

	if(odb::transaction::has_current()) {
		return false;
	}

	mudlog(LOG_SAVE, "inventory_parent_id_supported_tx: adding parent_inventory_id column");
	if(!mysql_execute_ddl_db(db,
							 "ALTER TABLE character_inventory "
							 "ADD COLUMN parent_inventory_id BIGINT UNSIGNED NULL DEFAULT NULL "
							 "COMMENT 'FK esplicita al contenitore padre; NULL = root (carry/equip)' "
							 "AFTER depth")) {
		return false;
	}
	if(!inventory_parent_column_exists_tx(db)) {
		mudlog(LOG_SYSERR,
			   "inventory_parent_id_supported_tx: parent_inventory_id column still missing");
		return false;
	}
	if(!inventory_parent_index_exists_tx(db)) {
		mudlog(LOG_SAVE,
			   "inventory_parent_id_supported_tx: adding idx_inventory_parent index");
		if(!mysql_execute_ddl_db(db, "ALTER TABLE character_inventory "
									   "ADD INDEX idx_inventory_parent (parent_inventory_id)")) {
			return false;
		}
	}
	parent_col_cache = 1;
	return true;
}

bool inventory_stored_vnum_is_container(ush_int vnum) {
	const int r = real_object(vnum);
	if(r < 0) {
		return false;
	}
	const struct obj_data* proto = static_cast<struct obj_data*>(obj_index[r].data);
	if(proto == nullptr) {
		return false;
	}
	return GET_ITEM_TYPE(proto) == ITEM_CONTAINER;
}

unsigned long long resolve_parent_id_from_flat(int parent_list_index,
											   const std::vector<inventory_flat_item>& flat) {
	if(parent_list_index < 0) {
		return 0;
	}
	for(const inventory_flat_item& item : flat) {
		if(item.list_index == parent_list_index && item.db_inventory_id != 0) {
			return item.db_inventory_id;
		}
	}
	return 0;
}

void backfill_inventory_parent_ids_for_toon_tx(DB* db, const std::string& toon_id,
											   bool soft_delete_supported);

unsigned long long flat_instance_id_at(const std::vector<inventory_flat_item>* flat,
									   int list_index) {
	if(!flat || list_index < 0) {
		return 0;
	}
	for(const inventory_flat_item& item : *flat) {
		if(item.list_index == list_index) {
			return item.db_instance_id;
		}
	}
	return 0;
}

void save_rent_mysql_tx(DB* db, const std::string& toon_id, const struct obj_file_u& rent,
						const std::vector<inventory_flat_item>* flat = nullptr) {
	static int soft_delete_cols = -1;
	if(soft_delete_cols < 0) {
		MYSQL_RES* cols_res = nullptr;
		const std::string cols_sql =
			"SELECT COUNT(*) FROM information_schema.COLUMNS "
			"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'character_inventory' "
			"AND COLUMN_NAME IN ('deleted','deleted_on','deleted_for')";
		if(mysql_query_select(db, cols_sql, cols_res) && cols_res) {
			MYSQL_ROW cols_row = mysql_fetch_row(cols_res);
			soft_delete_cols = static_cast<int>(cols_row ? sql_to_ll(cols_row[0], 0) : 0);
			mysql_free_result(cols_res);
		}
		else {
			soft_delete_cols = 0;
		}
	}
	const bool soft_delete_supported = (soft_delete_cols >= 3);
	const bool parent_supported = inventory_parent_id_supported_tx(db);

	if(soft_delete_supported) {
		db->execute(("DELETE cia FROM character_inventory_affect cia "
					 "INNER JOIN character_inventory ci ON ci.id = cia.inventory_id "
					 "WHERE ci.toon_id = " +
					 toon_id + " AND (ci.deleted = 0 OR ci.deleted IS NULL)")
						.c_str());
		db->execute(("DELETE FROM character_inventory WHERE toon_id = " + toon_id +
					 " AND (deleted = 0 OR deleted IS NULL)")
						.c_str());
	}
	else {
		db->execute(("DELETE cia FROM character_inventory_affect cia "
					 "INNER JOIN character_inventory ci ON ci.id = cia.inventory_id "
					 "WHERE ci.toon_id = " +
					 toon_id)
						.c_str());
		db->execute(("DELETE FROM character_inventory WHERE toon_id = " + toon_id).c_str());
	}
	db->execute(("DELETE FROM character_rent WHERE toon_id = " + toon_id).c_str());

	const int object_count = std::clamp(rent.number, 0, static_cast<int>(MAX_OBJ_SAVE));
	std::ostringstream rent_sql;
	rent_sql << "INSERT INTO character_rent (toon_id, gold_left, total_cost, last_update, "
				"minimum_stay, object_count) VALUES ("
			 << toon_id << ',' << rent.gold_left << ',' << rent.total_cost << ','
			 << rent.last_update << ',' << rent.minimum_stay << ',' << object_count << ')';
	db->execute(rent_sql.str().c_str());

	if(object_count > 0) {
		std::vector<std::string> inventory_rows;
		inventory_rows.reserve(static_cast<size_t>(object_count));
		int cur_depth = 0;
		int parent_at_depth[64] {};
		for(int i = 0; i < 64; ++i) {
			parent_at_depth[i] = -1;
		}
		for(int i = 0; i < object_count; ++i) {
			obj_file_elem o = rent.objects[i];
			unsigned long long iid = flat_instance_id_at(flat, i);
			{
				unsigned vnum = o.item_number;
				object_instance_normalize_stored(&vnum, &iid);
				o.item_number = static_cast<ush_int>(vnum);
			}
			if(o.depth > cur_depth) {
				if(cur_depth < 64) {
					parent_at_depth[cur_depth] = i - 1;
				}
				cur_depth = o.depth;
			}
			else if(o.depth < cur_depth) {
				cur_depth = o.depth;
			}
			int parent_list_index = -1;
			if(cur_depth > 0 && cur_depth <= 64) {
				parent_list_index = parent_at_depth[cur_depth - 1];
			}
			if(parent_list_index >= 0 && parent_list_index < object_count &&
			   !inventory_stored_vnum_is_container(rent.objects[parent_list_index].item_number)) {
				parent_list_index = -1;
			}
			inventory_rows.push_back(inventory_insert_values(
				toon_id, i, o, soft_delete_supported, parent_supported, parent_list_index, 0,
				iid));
		}
		execute_values_batch(db, inventory_insert_columns(soft_delete_supported, parent_supported),
							 inventory_rows, kInventoryInsertBatch);

		insert_inventory_affects_batch_tx(db, toon_id, object_count, rent,
										  soft_delete_supported);
	}
	if(parent_supported) {
		backfill_inventory_parent_ids_for_toon_tx(db, toon_id, soft_delete_supported);
	}
}

void backfill_inventory_parent_ids_for_toon_tx(DB* db, const std::string& toon_id,
											   bool soft_delete_supported) {
	MYSQL_RES* res = nullptr;
	std::ostringstream sql;
	sql << "SELECT id, list_index, item_number, depth FROM character_inventory WHERE toon_id = "
		<< toon_id;
	if(soft_delete_supported) {
		sql << " AND (deleted = 0 OR deleted IS NULL)";
	}
	sql << " ORDER BY list_index";
	if(!mysql_query_select_tx(db, sql.str(), res) || !res) {
		return;
	}

	struct row_meta {
		unsigned long long id;
		int list_index;
		ush_int vnum;
		int depth;
	};
	std::vector<row_meta> rows;
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		row_meta meta {};
		meta.id = static_cast<unsigned long long>(sql_to_ll(row[0], 0));
		meta.list_index = static_cast<int>(sql_to_ll(row[1], -1));
		meta.vnum = static_cast<ush_int>(sql_to_ll(row[2], 0));
		meta.depth = static_cast<int>(sql_to_ll(row[3], 0));
		if(meta.id > 0 && meta.list_index >= 0) {
			rows.push_back(meta);
		}
	}
	mysql_free_result(res);

	if(rows.empty()) {
		return;
	}

	int cur_depth = 0;
	unsigned long long in_obj[64] {};
	unsigned long long last_id = 0;
	for(int i = 0; i < 64; ++i) {
		in_obj[i] = 0;
	}

	for(const row_meta& meta : rows) {
		int depth = meta.depth;
		if(depth > 60) {
			depth = 0;
		}
		if(depth > cur_depth) {
			if(last_id != 0 && cur_depth < 64) {
				in_obj[cur_depth] = last_id;
			}
			cur_depth = depth;
		}
		else if(depth < cur_depth) {
			cur_depth = depth;
		}

		unsigned long long parent_id = 0;
		if(cur_depth > 0 && cur_depth <= 64) {
			parent_id = in_obj[cur_depth - 1];
		}
		if(parent_id != 0) {
			bool parent_is_container = false;
			for(const row_meta& candidate : rows) {
				if(candidate.id == parent_id) {
					parent_is_container = inventory_stored_vnum_is_container(candidate.vnum);
					break;
				}
			}
			if(!parent_is_container) {
				parent_id = 0;
			}
		}

		std::ostringstream upd;
		if(parent_id > 0) {
			upd << "UPDATE character_inventory SET parent_inventory_id=" << parent_id
				<< " WHERE id=" << meta.id << " AND toon_id=" << toon_id;
		}
		else {
			upd << "UPDATE character_inventory SET parent_inventory_id=NULL WHERE id=" << meta.id
				<< " AND toon_id=" << toon_id;
		}
		mysql_execute_sql_db(db, upd.str().c_str());

		last_id = meta.id;
	}
}

bool inventory_soft_delete_supported_tx(DB* db) {
	static int soft_delete_cols = -1;
	if(soft_delete_cols < 0) {
		MYSQL_RES* cols_res = nullptr;
		const std::string cols_sql =
			"SELECT COUNT(*) FROM information_schema.COLUMNS "
			"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'character_inventory' "
			"AND COLUMN_NAME IN ('deleted','deleted_on','deleted_for')";
		if(mysql_query_select(db, cols_sql, cols_res) && cols_res) {
			MYSQL_ROW cols_row = mysql_fetch_row(cols_res);
			soft_delete_cols = static_cast<int>(cols_row ? sql_to_ll(cols_row[0], 0) : 0);
			mysql_free_result(cols_res);
		}
		else {
			soft_delete_cols = 0;
		}
	}
	return soft_delete_cols >= 3;
}

bool obj_file_elem_snapshot_equal(const obj_file_elem& a, const obj_file_elem& b) {
	if(a.item_number != b.item_number) {
		return false;
	}
	for(int i = 0; i < 4; ++i) {
		if(a.value[i] != b.value[i]) {
			return false;
		}
	}
	if(a.extra_flags != b.extra_flags || a.extra_flags2 != b.extra_flags2 ||
	   a.weight != b.weight || a.timer != b.timer || a.bitvector != b.bitvector ||
	   a.wearpos != b.wearpos || a.depth != b.depth) {
		return false;
	}
	if(std::strcmp(a.name, b.name) != 0 || std::strcmp(a.sd, b.sd) != 0 ||
	   std::strcmp(a.desc, b.desc) != 0) {
		return false;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(a.affected[i].location != b.affected[i].location ||
		   a.affected[i].modifier != b.affected[i].modifier) {
			return false;
		}
	}
	return true;
}

void elem_from_db_inventory_row(const MYSQL_ROW row, obj_file_elem& o) {
	o = obj_file_elem {};
	o.item_number = static_cast<ush_int>(sql_to_ll(row[2]));
	o.value[0] = static_cast<int>(sql_to_ll(row[3]));
	o.value[1] = static_cast<int>(sql_to_ll(row[4]));
	o.value[2] = static_cast<int>(sql_to_ll(row[5]));
	o.value[3] = static_cast<int>(sql_to_ll(row[6]));
	o.extra_flags = static_cast<int>(sql_to_ll(row[7]));
	o.extra_flags2 = static_cast<int>(sql_to_ll(row[8]));
	o.weight = static_cast<int>(sql_to_ll(row[9]));
	o.timer = static_cast<int>(sql_to_ll(row[10]));
	o.bitvector = static_cast<unsigned int>(sql_to_ll(row[11]));
	std::snprintf(o.name, sizeof(o.name), "%s", row[12] ? row[12] : "");
	std::snprintf(o.sd, sizeof(o.sd), "%s", row[13] ? row[13] : "");
	std::snprintf(o.desc, sizeof(o.desc), "%s", row[14] ? row[14] : "");
	o.wearpos = static_cast<ubyte>(sql_to_ll(row[15]));
	o.depth = static_cast<ubyte>(sql_to_ll(row[16]));
}

std::string sql_id_in_list(const std::vector<unsigned long long>& ids) {
	std::ostringstream sql;
	for(size_t i = 0; i < ids.size(); ++i) {
		if(i) {
			sql << ',';
		}
		sql << ids[i];
	}
	return sql.str();
}

void delete_inventory_ids_tx(DB* db, const std::string& toon_id,
							 const std::vector<unsigned long long>& ids) {
	if(ids.empty()) {
		return;
	}
	const std::string id_list = sql_id_in_list(ids);
	db->execute(("DELETE cia FROM character_inventory_affect cia "
				 "INNER JOIN character_inventory ci ON ci.id = cia.inventory_id "
				 "WHERE ci.toon_id = " +
				 toon_id + " AND ci.id IN (" + id_list + ")")
					.c_str());
	db->execute(("DELETE FROM character_inventory WHERE toon_id = " + toon_id + " AND id IN (" +
				 id_list + ")")
					.c_str());
}

void update_inventory_row_tx(DB* db, const std::string& toon_id, unsigned long long id,
							 int list_index, const obj_file_elem& o, bool parent_supported,
							 int parent_list_index, unsigned long long parent_id,
							 bool soft_delete_supported, unsigned long long instance_id) {
	std::ostringstream sql;
	sql << "UPDATE character_inventory SET list_index=" << list_index << ",item_number="
		<< o.item_number << ",value0=" << o.value[0] << ",value1=" << o.value[1]
		<< ",value2=" << o.value[2] << ",value3=" << o.value[3] << ",extra_flags=" << o.extra_flags
		<< ",extra_flags2=" << o.extra_flags2 << ",weight=" << o.weight << ",timer=" << o.timer
		<< ",bitvector=" << o.bitvector << ",obj_name=" << db_sql_literal(o.name, false)
		<< ",short_desc=" << db_sql_literal(o.sd, false) << ",description="
		<< db_sql_literal(o.desc, false) << ",wear_pos=" << static_cast<int>(o.wearpos)
		<< ",depth=" << static_cast<int>(o.depth);
	if(instance_id > 0) {
		sql << ",instance_id=" << instance_id;
	}
	else {
		sql << ",instance_id=NULL";
	}
	if(parent_supported) {
		if(parent_id > 0) {
			sql << ",parent_inventory_id=" << parent_id;
		}
		else if(parent_list_index < 0) {
			sql << ",parent_inventory_id=NULL";
		}
		else {
			sql << ",parent_inventory_id=(SELECT ci_p.id FROM character_inventory ci_p "
				   "WHERE ci_p.toon_id="
				<< toon_id << " AND ci_p.list_index=" << parent_list_index;
			if(soft_delete_supported) {
				sql << " AND (ci_p.deleted=0 OR ci_p.deleted IS NULL)";
			}
			sql << " LIMIT 1)";
		}
	}
	sql << " WHERE id=" << id << " AND toon_id=" << toon_id;
	db->execute(sql.str().c_str());
}

void delete_inventory_affects_for_id_tx(DB* db, unsigned long long inventory_id) {
	std::ostringstream sql;
	sql << "DELETE FROM character_inventory_affect WHERE inventory_id=" << inventory_id;
	db->execute(sql.str().c_str());
}

void upsert_character_rent_tx(DB* db, const std::string& toon_id, const struct obj_file_u& rent,
							  int object_count) {
	std::ostringstream sql;
	sql << "INSERT INTO character_rent (toon_id, gold_left, total_cost, last_update, "
		   "minimum_stay, object_count) VALUES ("
		<< toon_id << ',' << rent.gold_left << ',' << rent.total_cost << ',' << rent.last_update
		<< ',' << rent.minimum_stay << ',' << object_count
		<< ") ON DUPLICATE KEY UPDATE gold_left=VALUES(gold_left), "
		   "total_cost=VALUES(total_cost), last_update=VALUES(last_update), "
		   "minimum_stay=VALUES(minimum_stay), object_count=VALUES(object_count)";
	db->execute(sql.str().c_str());
}

bool flat_has_any_db_inventory_id(const std::vector<inventory_flat_item>& flat) {
	for(const inventory_flat_item& item : flat) {
		if(item.db_inventory_id != 0) {
			return true;
		}
	}
	return false;
}

unsigned long long count_active_inventory_rows_tx(DB* db, const std::string& toon_id,
												  bool soft_delete_supported) {
	std::ostringstream sql;
	sql << "SELECT COUNT(*) FROM character_inventory WHERE toon_id = " << toon_id;
	if(soft_delete_supported) {
		sql << " AND (deleted = 0 OR deleted IS NULL)";
	}
	MYSQL_RES* res = nullptr;
	if(!mysql_query_select_tx(db, sql.str(), res) || !res) {
		return 0;
	}
	unsigned long long count = 0;
	if(MYSQL_ROW row = mysql_fetch_row(res)) {
		count = static_cast<unsigned long long>(sql_to_ll(row[0], 0));
	}
	mysql_free_result(res);
	return count;
}

void assign_db_inventory_ids_tx(DB* db, const std::string& toon_id,
								std::vector<inventory_flat_item>& flat, int object_count,
								bool soft_delete_supported) {
	if(flat.empty() || object_count <= 0) {
		return;
	}
	std::ostringstream sql;
	sql << "SELECT list_index, id FROM character_inventory WHERE toon_id = " << toon_id;
	if(soft_delete_supported) {
		sql << " AND (deleted = 0 OR deleted IS NULL)";
	}
	sql << " ORDER BY list_index";
	MYSQL_RES* res = nullptr;
	if(!mysql_query_select_tx(db, sql.str(), res) || !res) {
		return;
	}
	std::unordered_map<int, unsigned long long> id_by_index;
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		const int idx = static_cast<int>(sql_to_ll(row[0], -1));
		const unsigned long long id = static_cast<unsigned long long>(sql_to_ll(row[1], 0));
		if(idx >= 0 && idx < object_count && id > 0) {
			id_by_index[static_cast<size_t>(idx)] = id;
		}
	}
	mysql_free_result(res);
	for(inventory_flat_item& item : flat) {
		if(item.list_index < 0 || item.list_index >= object_count) {
			continue;
		}
		const auto it = id_by_index.find(static_cast<size_t>(item.list_index));
		if(it != id_by_index.end()) {
			item.db_inventory_id = it->second;
		}
	}
}

void save_rent_mysql_incremental_tx(DB* db, const std::string& toon_id,
									const struct obj_file_u& rent,
									std::vector<inventory_flat_item>& flat) {
	const bool soft_delete_supported = inventory_soft_delete_supported_tx(db);
	const bool parent_supported = inventory_parent_id_supported_tx(db);
	const int object_count = std::clamp(rent.number, 0, static_cast<int>(MAX_OBJ_SAVE));

	if(!flat_has_any_db_inventory_id(flat) &&
	   count_active_inventory_rows_tx(db, toon_id, soft_delete_supported) > 0) {
		mudlog(LOG_SAVE,
			   "save_rent_mysql_incremental_tx: no db ids in memory for toon_id %s, full replace",
			   toon_id.c_str());
		save_rent_mysql_tx(db, toon_id, rent, &flat);
		assign_db_inventory_ids_tx(db, toon_id, flat, object_count, soft_delete_supported);
		return;
	}

	std::unordered_map<unsigned long long, obj_file_elem> db_elems;
	std::unordered_map<unsigned long long, int> db_list_index;
	std::unordered_map<unsigned long long, unsigned long long> db_parent_id;
	std::unordered_map<unsigned long long, unsigned long long> db_instance_id;
	std::unordered_set<unsigned long long> kept_ids;
	MYSQL_RES* res = nullptr;
	std::ostringstream snap_sql;
	snap_sql << "SELECT id, list_index, item_number, value0, value1, value2, value3, extra_flags, "
				"extra_flags2, weight, timer, bitvector, obj_name, short_desc, description, "
				"wear_pos, depth";
	if(parent_supported) {
		snap_sql << ", parent_inventory_id";
	}
	snap_sql << ", instance_id FROM character_inventory WHERE toon_id = " << toon_id;
	if(soft_delete_supported) {
		snap_sql << " AND (deleted = 0 OR deleted IS NULL)";
	}
	if(!mysql_query_select_tx(db, snap_sql.str(), res) || !res) {
		mudlog(LOG_SYSERR,
			   "save_rent_mysql_incremental_tx: snapshot failed for toon_id %s, full replace",
			   toon_id.c_str());
		save_rent_mysql_tx(db, toon_id, rent, &flat);
		assign_db_inventory_ids_tx(db, toon_id, flat, object_count, soft_delete_supported);
		return;
	}
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		const unsigned long long id = static_cast<unsigned long long>(sql_to_ll(row[0], 0));
		const int list_index = static_cast<int>(sql_to_ll(row[1], -1));
		if(id == 0) {
			continue;
		}
		obj_file_elem elem {};
		elem_from_db_inventory_row(row, elem);
		db_elems[id] = elem;
		db_list_index[id] = list_index;
		const int instance_col = parent_supported ? 18 : 17;
		db_instance_id[id] = static_cast<unsigned long long>(sql_to_ll(row[instance_col], 0));
		if(parent_supported) {
			db_parent_id[id] = static_cast<unsigned long long>(sql_to_ll(row[17], 0));
		}
	}
	mysql_free_result(res);

	res = nullptr;
	std::ostringstream aff_sql;
	aff_sql << "SELECT ci.id, cia.affect_slot, cia.location, cia.modifier "
			   "FROM character_inventory_affect cia "
			   "INNER JOIN character_inventory ci ON ci.id = cia.inventory_id "
			   "WHERE ci.toon_id = "
			<< toon_id;
	if(soft_delete_supported) {
		aff_sql << " AND (ci.deleted = 0 OR ci.deleted IS NULL)";
	}
	if(mysql_query_select_tx(db, aff_sql.str(), res) && res) {
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			const unsigned long long id = static_cast<unsigned long long>(sql_to_ll(row[0], 0));
			const int slot = static_cast<int>(sql_to_ll(row[1], -1));
			auto it = db_elems.find(id);
			if(it == db_elems.end() || slot < 0 || slot >= MAX_OBJ_AFFECT) {
				continue;
			}
			it->second.affected[slot].location = static_cast<short>(sql_to_ll(row[2]));
			it->second.affected[slot].modifier = static_cast<int>(sql_to_ll(row[3]));
		}
		mysql_free_result(res);
	}

	std::vector<unsigned long long> removed_ids;
	for(const auto& entry : db_elems) {
		kept_ids.insert(entry.first);
	}
	for(const inventory_flat_item& item : flat) {
		if(item.db_inventory_id != 0) {
			kept_ids.erase(item.db_inventory_id);
		}
	}
	removed_ids.assign(kept_ids.begin(), kept_ids.end());
	delete_inventory_ids_tx(db, toon_id, removed_ids);

	upsert_character_rent_tx(db, toon_id, rent, object_count);

	std::vector<std::string> insert_rows;
	std::unordered_set<int> affect_refresh_indices;
	int skipped = 0;
	int updated = 0;
	int inserted = 0;

	for(inventory_flat_item& item : flat) {
		if(item.list_index < 0 || item.list_index >= object_count) {
			continue;
		}
		const obj_file_elem& elem = rent.objects[item.list_index];
		const unsigned long long id = item.db_inventory_id;
		const unsigned long long new_parent_id =
			resolve_parent_id_from_flat(item.parent_list_index, flat);

		if(id != 0) {
			const auto db_it = db_elems.find(id);
			if(db_it == db_elems.end()) {
				delete_inventory_ids_tx(db, toon_id, {id});
				item.db_inventory_id = 0;
				insert_rows.push_back(inventory_insert_values(
					toon_id, item.list_index, elem, soft_delete_supported, parent_supported,
					item.parent_list_index, new_parent_id, item.db_instance_id));
				++inserted;
				affect_refresh_indices.insert(item.list_index);
				continue;
			}
			const bool same_elem = obj_file_elem_snapshot_equal(elem, db_it->second);
			const int old_index = db_list_index[id];
			const unsigned long long old_parent_id =
				parent_supported ? db_parent_id[id] : new_parent_id;
			const unsigned long long old_instance_id = db_instance_id[id];
			if(same_elem && old_index == item.list_index && old_parent_id == new_parent_id &&
			   old_instance_id == item.db_instance_id) {
				++skipped;
				continue;
			}
			if(same_elem && (old_index != item.list_index || old_parent_id != new_parent_id) &&
			   old_instance_id == item.db_instance_id) {
				std::ostringstream sql;
				sql << "UPDATE character_inventory SET list_index=" << item.list_index;
				if(parent_supported) {
					if(new_parent_id > 0) {
						sql << ",parent_inventory_id=" << new_parent_id;
					}
					else if(item.parent_list_index < 0) {
						sql << ",parent_inventory_id=NULL";
					}
					else {
						sql << ",parent_inventory_id=(SELECT ci_p.id FROM character_inventory ci_p "
							   "WHERE ci_p.toon_id="
							<< toon_id << " AND ci_p.list_index=" << item.parent_list_index;
						if(soft_delete_supported) {
							sql << " AND (ci_p.deleted=0 OR ci_p.deleted IS NULL)";
						}
						sql << " LIMIT 1)";
					}
				}
				sql << " WHERE id=" << id << " AND toon_id=" << toon_id;
				db->execute(sql.str().c_str());
				++updated;
				continue;
			}
			update_inventory_row_tx(db, toon_id, id, item.list_index, elem, parent_supported,
									item.parent_list_index, new_parent_id, soft_delete_supported,
									item.db_instance_id);
			delete_inventory_affects_for_id_tx(db, id);
			affect_refresh_indices.insert(item.list_index);
			++updated;
			continue;
		}

		insert_rows.push_back(inventory_insert_values(
			toon_id, item.list_index, elem, soft_delete_supported, parent_supported,
			item.parent_list_index, new_parent_id, item.db_instance_id));
		++inserted;
		affect_refresh_indices.insert(item.list_index);
	}

	if(!insert_rows.empty()) {
		execute_values_batch(db,
							 inventory_insert_columns(soft_delete_supported, parent_supported),
							 insert_rows, kInventoryInsertBatch);
	}

	assign_db_inventory_ids_tx(db, toon_id, flat, object_count, soft_delete_supported);

	if(!affect_refresh_indices.empty()) {
		std::vector<std::string> affect_rows;
		for(int idx : affect_refresh_indices) {
			if(idx < 0 || idx >= object_count) {
				continue;
			}
			for(int a = 0; a < MAX_OBJ_AFFECT; ++a) {
				const obj_affected_type& oa = rent.objects[idx].affected[a];
				if(oa.location == 0 && oa.modifier == 0) {
					continue;
				}
				affect_rows.push_back(inventory_affect_select_row(
					toon_id, idx, a, static_cast<int>(oa.location),
					static_cast<int>(oa.modifier), soft_delete_supported));
			}
		}
		execute_union_batch(db, inventory_affect_insert_select_prefix(), affect_rows,
							kInventoryInsertBatch);
	}

	mudlog(LOG_SAVE,
		   "save_rent_mysql_incremental_tx: toon_id %s skipped=%d updated=%d inserted=%d removed=%zu",
		   toon_id.c_str(), skipped, updated, inserted, removed_ids.size());
}

} // namespace

bool inventory_parent_column_supported() {
#if !USE_MYSQL
	return false;
#else
	try {
		DB* db = Sql::getMysql();
		return inventory_parent_id_supported_tx(db);
	}
	catch(...) {
		return false;
	}
#endif
}

void assign_db_inventory_ids_after_rent_save(DB* db, const std::string& toon_id,
											 std::vector<inventory_flat_item>& flat,
											 int object_count) {
	static int soft_delete_cols = -1;
	if(soft_delete_cols < 0) {
		MYSQL_RES* cols_res = nullptr;
		const std::string cols_sql =
			"SELECT COUNT(*) FROM information_schema.COLUMNS "
			"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'character_inventory' "
			"AND COLUMN_NAME IN ('deleted','deleted_on','deleted_for')";
		if(mysql_query_select(db, cols_sql, cols_res) && cols_res) {
			MYSQL_ROW cols_row = mysql_fetch_row(cols_res);
			soft_delete_cols = static_cast<int>(cols_row ? sql_to_ll(cols_row[0], 0) : 0);
			mysql_free_result(cols_res);
		}
		else {
			soft_delete_cols = 0;
		}
	}
	const int count = std::clamp(object_count, 0, static_cast<int>(MAX_OBJ_SAVE));
	assign_db_inventory_ids_tx(db, toon_id, flat, count, soft_delete_cols >= 3);
	/* Dopo full replace senza flat a save-time, o refresh post-save: allinea instance_id. */
	for(const inventory_flat_item& item : flat) {
		if(item.db_inventory_id == 0) {
			continue;
		}
		std::ostringstream sql;
		sql << "UPDATE character_inventory SET instance_id=";
		if(item.db_instance_id > 0) {
			sql << item.db_instance_id;
		}
		else {
			sql << "NULL";
		}
		sql << " WHERE id=" << item.db_inventory_id << " AND toon_id=" << toon_id;
		db->execute(sql.str().c_str());
	}
}

bool save_character_rent_incremental(struct char_data* ch, const struct obj_file_u* rent,
									 std::vector<inventory_flat_item>& flat) {
#if !USE_MYSQL
	(void)ch;
	(void)rent;
	(void)flat;
	return false;
#else
	struct char_data* pc = save_char_resolve_pc(ch);
	if(!pc || !rent) {
		return false;
	}
	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(GET_NAME(pc)));
	if(!pg || !pg->id) {
		mudlog(LOG_SYSERR, "save_character_rent_incremental: missing toon for %s", GET_NAME(pc));
		return false;
	}
	try {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string toon_id = std::to_string(pg->id);
		save_rent_mysql_incremental_tx(db, toon_id, *rent, flat);
		save_char_extra_mysql_tx(db, pg->id, pc);
		t.commit();
		mudlog(LOG_SAVE, "save_character_rent_incremental: OK %s", GET_NAME(pc));
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "save_character_rent_incremental: %s", e.what());
		return false;
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "save_character_rent_incremental: %s", e.what());
		return false;
	}
#endif
}

bool save_character_to_db(struct char_data* ch, const struct char_file_u* st,
						  const struct obj_file_u* rent, unsigned save_flags,
						  const std::vector<inventory_flat_item>* rent_flat) {
#if !USE_MYSQL
	(void)ch;
	(void)st;
	(void)rent;
	(void)save_flags;
	(void)rent_flat;
	return false;
#else
	struct char_data* pc = save_char_resolve_pc(ch);
	if(!pc) {
		const char* who = "?";
		if(ch && IS_PC(ch) && GET_NAME(ch)) {
			who = GET_NAME(ch);
		}
		mudlog(LOG_SYSERR, "save_character_to_db: cannot resolve PC for %s", who);
		return false;
	}

	const bool want_body = (save_flags & CHAR_DB_SAVE_BODY) != 0;
	const bool want_toon = (save_flags & CHAR_DB_SAVE_TOON) != 0;
	const bool want_extra = (save_flags & CHAR_DB_SAVE_EXTRA) != 0;
	const bool want_rent = (save_flags & CHAR_DB_SAVE_RENT) != 0;

	if(want_body && !st) {
		return false;
	}
	if(want_toon && !st) {
		return false;
	}
	if(want_rent && !rent) {
		return false;
	}
	if(!want_body && !want_toon && !want_extra && !want_rent) {
		return false;
	}

	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(GET_NAME(pc)));
	if(!pg || !pg->id) {
		mudlog(LOG_SYSERR, "save_character_to_db: missing toon for %s", GET_NAME(pc));
		return false;
	}

	try {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string toon_id = std::to_string(pg->id);

		if(want_body && st) {
			db_write_character_snapshot_tx(db, toon_id, *st);
			toon_mark_migrated_tx(db, pg->id, CHARACTER_MIGRATION_SCHEMA_VERSION);
		}
		if(want_toon && st) {
			db_update_toon_registry_tx(db, toon_id, pc, *st);
		}
		if(want_extra) {
			save_char_extra_mysql_tx(db, pg->id, pc);
		}
		if(want_rent && rent) {
			save_rent_mysql_tx(db, toon_id, *rent, rent_flat);
#if INVENTORY_SAVE_INCREMENTAL
			refresh_inventory_db_ids_after_rent_save(pc, db, toon_id);
#endif
		}

		t.commit();
		mudlog(LOG_SAVE, "save_character_to_db: OK %s flags=0x%x", GET_NAME(pc), save_flags);
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "save_character_to_db: %s", e.what());
		return false;
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "save_character_to_db: %s", e.what());
		return false;
	}
#endif
}

bool save_char_mysql_snapshot(struct char_data* ch, const struct char_file_u& st) {
	return save_character_to_db(ch, &st, nullptr, CHAR_DB_SAVE_BODY_TOON);
}

/**************************************************************************
 *  declarations of most of the 'global' variables                         *
 ************************************************************************ */
const long beginning_of_time = BEG_OF_TIME;
int no_mail = 0;
int top_of_scripts = 0;
int top_of_world = 0; /* ref to the top element of world */
#if HASH
struct hash_header room_db;
#else
struct room_data* room_db[WORLD_SIZE];
#endif

struct obj_data* object_list = 0; /* the global linked list of obj's */
struct char_data* character_list = 0; /* global l-list of chars          */

struct zone_data* zone_table = NULL; /* table of reset data             */
int top_of_zone_table = 0;
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
struct player_index_element* player_table = 0; /* index to player file   */
int top_of_p_table = 0; /* ref to top of table             */
int top_of_p_file = 0;
long total_bc = 0;
long room_count = 0;
long mob_count = 0;
long obj_count = 0;
long total_mbc = 0;
long total_obc = 0;


/*
 **  distributed monster stuff
 */
int mob_tick_count = 0;
char wmotd[MAX_STRING_LENGTH];
char credits[MAX_STRING_LENGTH]; /* the Credits List                */
char news[MAX_STRING_LENGTH]; /* the news                        */
char wiznews[MAX_STRING_LENGTH]; /* wiz news                        */
char motd[MAX_STRING_LENGTH]; /* the messages of today           */
char help[MAX_STRING_LENGTH]; /* the main help page              */
char info[MAX_STRING_LENGTH]; /* the info text                   */
char wizlist[MAX_STRING_LENGTH * 2]; /* the wizlist                     */
char princelist[MAX_STRING_LENGTH * 2]; /* the princelist                  */
char immlist[MAX_STRING_LENGTH * 2]; /* the princelist                  */
char rarelist[MAX_STRING_LENGTH * 2]; /*Acidus 2004-show rare*/
char login[MAX_STRING_LENGTH];

FILE* mob_f; /* file containing mob prototypes  */
FILE* obj_f; /* obj prototypes                  */
FILE* help_fl; /* file for help texts (HELP <kwd>)*/
FILE* wizhelp_fl; /* file for wizhelp */

struct index_data* mob_index; /* index table for mobile file     */
struct index_data* obj_index; /* index table for object file     */
struct help_index_element* help_index = 0;
struct help_index_element* wizhelp_index = 0;
int top_of_mobt = 0; /* top of mobile index table       */
int top_of_objt = 0; /* top of object index table       */
int top_of_sort_mobt = 0;
int top_of_sort_objt = 0;
int top_of_alloc_mobt = 0;
int top_of_alloc_objt = 0;
int top_of_helpt; /* top of help index table         */
int top_of_wizhelpt; /* top of wiz help index table         */

struct NebbieQuest KnownObjQuest[MAX_QUEST_ACHIE];

struct time_info_data time_info; /* the infomation about the time   */
struct weather_data weather_info; /* the infomation about the weather */
long saved_rooms[WORLD_SIZE];
long number_of_saved_rooms = 0;
struct script_com* gpComp = NULL;
struct scripts* gpScript_data = NULL;
struct reset_q_type gReset_q = { NULL, NULL };
char curfile[256]; /* Informazioni sul file in lettura */

static unsigned g_fread_quiet_depth = 0;
static int g_fread_error_count = 0;

static void fread_note_error() {
	g_fread_error_count++;
}

void fread_quiet_begin() {
	if(g_fread_quiet_depth == 0) {
		g_fread_error_count = 0;
	}
	g_fread_quiet_depth++;
}

int fread_quiet_end() {
	const int errs = g_fread_error_count;
	if(g_fread_quiet_depth > 0) {
		g_fread_quiet_depth--;
	}
	if(g_fread_quiet_depth == 0) {
		g_fread_error_count = 0;
	}
	return errs;
}

int fread_is_quiet() {
	return g_fread_quiet_depth > 0 ? 1 : 0;
}

/*************************************************************************
 *  routines for booting the system                                       *
 *********************************************************************** */
void FrozeHim_old(struct char_data* ch, int hp) {
	char buf[500];
	snprintf(buf, 499,
			 "$c0115Non ti sembra che %d hp siano davvero un po' troppi?$c0007\n\r",
			 hp);
	send_to_char(buf, ch);
	SET_BIT(ch->specials.act, PLR_FREEZE);
	buglog(LOG_PLAYERS, "%s congelato per HP", GET_NAME(ch));
	do_title(ch, "has been frozen by Alar! [CDB]", CMD_TITLE);
}

void FrozeHim(struct char_data* ch, int hp) {
	char buf[500];

	if(GetMaxLevel(ch) < IMMORTAL) {
		snprintf(buf, 499,
				 "$c0115Non ti sembra che %d hp siano davvero un po' troppi?$c0007\n\r",
				 hp);
		send_to_char(buf, ch);
		SET_BIT(ch->specials.act, PLR_FREEZE);
		buglog(LOG_PLAYERS, "%s congelato per HP", GET_NAME(ch));
		do_title(ch, "has been frozen by Alar! [CDB]", CMD_TITLE);
	}
	else {
		snprintf(buf, 499,
				 "$c0115Non ti sembra che %d hp siano troppi? Segnala la cosa ai capoccia$c0007\n\r",
				 hp);
		send_to_char(buf, ch);
		buglog(LOG_PLAYERS, "%s e' immortale ma gli HP sono strani...",
			   GET_NAME(ch));
	}
}

/* body of the booting system */
void boot_db() {
	int i;

	mudlog(LOG_CHECK, "Boot db --- BEGIN.");

	mudlog(LOG_CHECK, "Resetting the game time:");
	reset_time();

	mudlog(LOG_CHECK, "Reading newsfile, credits, help-page, info and motd.");
#if USE_MYSQL
	server_text_boot();
#else
	file_to_string(NEWS_FILE.c_str(), news);
	file_to_string(WIZNEWS_FILE.c_str(), wiznews);
	file_to_string(MOTD_FILE.c_str(), motd);
	file_to_string(WIZ_MOTD_FILE.c_str(), wmotd);
#endif
	file_to_string(CREDITS_FILE.c_str(), credits);
	file_to_string(HELP_PAGE_FILE.c_str(), help);
	file_to_string(INFO_FILE.c_str(), info);
	file_to_string(WIZLIST_FILE, wizlist);
	file_to_string(LOGIN_FILE.c_str(), login);
    /* achievement stuff
     file_to_string(LVL1_ACHIE_DONE, achie_lvl1_done);
     file_to_string(LVL2_ACHIE_DONE, achie_lvl2_done);
     file_to_string(LVL3_ACHIE_DONE, achie_lvl3_done);
     file_to_string(LVL4_ACHIE_DONE, achie_lvl4_done);
     file_to_string(LVL5_ACHIE_DONE, achie_lvl5_done); */

	mudlog(LOG_CHECK, "Initializing Script Files.");

	/* some machines are pre-allocation specific when dealing with realloc */
	gpScript_data = (struct scripts*) malloc(sizeof(struct scripts));
	if(gpScript_data == NULL) {
		mudlog(LOG_SYSERR, "Cannot allocate memory for gpScript_data");
		abort();
	}
	CommandSetup();
	InitScripts();
	mudlog(LOG_CHECK, "Opening mobile, object and help files.");
	if(!(mob_f = fopen(MOB_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","Opening mob file",strerror(errno));
		abort();
	}

	if(!(obj_f = fopen(OBJ_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","Opening obj file",strerror(errno));
		abort();
	}
	if(!(help_fl = fopen(HELP_KWRD_FILE.c_str(), "r"))) {
		mudlog(LOG_ERROR, "   Could not open help file.");
	}
	else {
		help_index = build_help_index(help_fl, &top_of_helpt);
	}
	if(!(wizhelp_fl = fopen(WIZ_HELP_FILE.c_str(), "r"))) {
		mudlog(LOG_ERROR, "   Could not open wizhelp file.");
	}
	else {
		wizhelp_index = build_help_index(wizhelp_fl, &top_of_wizhelpt);
	}
#if CLEAN_AT_BOOT
	mudlog(LOG_CHECK, "Clearing inactive players");
	clean_playerfile();
#else
	mudlog(LOG_CHECK, "Skipping inactive players check");
#endif

	mudlog(LOG_CHECK, "Booting mail system.");
	if(!scan_mail_file()) {
		mudlog(LOG_ERROR, "   Mail system error -- mail system disabled!");
		no_mail = 1;
	}
#if ENABLE_AUCTION
	Start_Auction();
#endif
	mudlog(LOG_CHECK, "Loading zone table.");
	boot_zones();

	mudlog(LOG_CHECK, "Boot procarea zone table.");
	procarea_boot_zone();

	mudlog(LOG_CHECK, "Loading saved zone table.");
	boot_saved_zones();

	mudlog(LOG_CHECK, "Loading rooms.");
	boot_world();

	mudlog(LOG_CHECK, "Boot procarea static rooms.");
	procarea_boot_darkstar_temple();

	mudlog(LOG_CHECK, "Boot procarea reward object prototypes.");
	procarea_boot_reward_shields();
	procarea_boot_reward_gear();

	mudlog(LOG_CHECK, "Loading saved rooms.");
	boot_saved_rooms();

	mudlog(LOG_CHECK, "Generating index tables for mobile and object files.");
	mob_index = generate_indices(mob_f, &top_of_mobt, &top_of_sort_mobt,
								 &top_of_alloc_mobt, MOB_DIR);
	obj_index = generate_indices(obj_f, &top_of_objt, &top_of_sort_objt,
								 &top_of_alloc_objt, OBJ_DIR);

	mudlog(LOG_CHECK, "Renumbering zone table.");
	renum_zone_table(0);

	mudlog(LOG_CHECK, "Generating player index.");
	build_player_index();

	mudlog(LOG_CHECK, "Loading fight messages.");
	load_messages();

	mudlog(LOG_CHECK, "Loading social messages.");
	boot_social_messages();

	mudlog(LOG_CHECK, "Loading pose messages.");
	boot_pose_messages();

	mudlog(LOG_CHECK, "Assigning function pointers:");
	if(!no_specials) {
		mudlog(LOG_CHECK, "   Mobiles, Objects, Rooms.");
		assign_speciales();
		mudlog(LOG_CHECK, "   Guilds.");
		BootGuilds();
	}

	mudlog(LOG_CHECK, "   Commands.");
	assign_command_pointers();

	mudlog(LOG_CHECK, "   Nannies.");
	assign_nannies_pointers();

	mudlog(LOG_CHECK, "   Spells.");
	assign_spell_pointers();
	boot_spells();

#if USE_MYSQL
	mudlog(LOG_CHECK, "Migrating OK edit objects (34k) into object_instance:");
	object_instance_boot_migrate();

	mudlog(LOG_CHECK,
		   "Migrating clan symbols (ITEM_CLAN_SYMBOL + free 34k -> instance):");
	clan_symbol_boot_migrate();

	mudlog(LOG_CHECK, "Migrating edit hp/mana/move/regen from eq to character_stats:");
	edit_pool_boot_migrate();

	mudlog(LOG_CHECK, "Archiving legacy files for migrated characters:");
	cleanup_migrated_legacy_files();
#endif

	mudlog(LOG_CHECK, "Updating characters with saved items:");
	update_obj_file();

#if LIMITED_ITEMS
#if USE_MYSQL
	mudlog(LOG_CHECK, "Counting rare items in MySQL inventories:");
	CountLimitedItemsMysql();
#endif
	PrintLimitedItems();
#endif

	mudlog(LOG_CHECK, "Loading objects for saved rooms.");
	ReloadRooms();

	for(i = 0; i <= top_of_zone_table; i++) {
		char* s;
		int d, e;
		s = zone_table[i].name;
		d = (i ? (zone_table[i - 1].top + 1) : 0);
		zone_table[i].bottom = d;
		e = zone_table[i].top;
		mudlog(LOG_WORLD, "Performing boot-time init of %d:%s (rooms %d-%d).",
			   zone_table[i].num, s, d, e);
		zone_table[i].start = 0;

		if(i == 0) {
			mudlog(LOG_WORLD, "Performing boot-time reload of static mobs.");
			reset_zone(0);
		}

		if(i == 1) {
			mudlog(LOG_WORLD, "Reset of %s", s);
			reset_zone(1);
		}
	}

	gReset_q.head = gReset_q.tail = 0;

	mudlog(LOG_CHECK, "Boot db -- DONE.");
}

/* reset the time in the game from file */
void reset_time() {

	//struct time_info_data mud_time_passed(time_t t2, time_t t1);

	time_info = mud_time_passed(time(0), beginning_of_time);

	moontype = time_info.day;

	switch(time_info.hours) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4: {
		weather_info.sunlight = SUN_DARK;
		switch_light(MOON_SET);
		break;
	}
	case 5:
	case 6: {
		weather_info.sunlight = SUN_RISE;
		switch_light(SUN_RISE);
		break;
	}
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18: {
		weather_info.sunlight = SUN_LIGHT;
		switch_light(SUN_LIGHT);
		break;
	}
	case 19:
	case 20: {
		weather_info.sunlight = SUN_SET;
		switch_light(SUN_SET);
		break;
	}
	case 21:
	case 22:
	case 23:
	default: {
		switch_light(SUN_DARK);
		weather_info.sunlight = SUN_DARK;
		break;
	}
	}

	mudlog(LOG_CHECK, "   Current Gametime: %dH %dD %dM %dY.",
		   static_cast<int>(time_info.hours), static_cast<int>(time_info.day),
		   static_cast<int>(time_info.month), static_cast<int>(time_info.year));

	weather_info.pressure = 960;
	if((time_info.month >= 7) && (time_info.month <= 12)) {
		weather_info.pressure += dice(1, 50);
	}
	else {
		weather_info.pressure += dice(1, 80);
	}

	weather_info.change = 0;

	if(weather_info.pressure <= 980) {
		if((time_info.month >= 3) && (time_info.month <= 14)) {
			weather_info.sky = SKY_LIGHTNING;
		}
		else {
			weather_info.sky = SKY_LIGHTNING;
		}
	}
	else if(weather_info.pressure <= 1000) {
		if((time_info.month >= 3) && (time_info.month <= 14)) {
			weather_info.sky = SKY_RAINING;
		}
		else {
			weather_info.sky = SKY_RAINING;
		}
	}
	else if(weather_info.pressure <= 1020) {
		weather_info.sky = SKY_CLOUDY;
	}
	else {
		weather_info.sky = SKY_CLOUDLESS;
	}
}

/* update the time file */
void update_time() {
	return;
}

struct wizs {
	char name[20];
	int level;
};

int intcomp(struct wizs* j, struct wizs* k) {
	return (k->level - j->level);
}

char* GeneraSezione(int livello, struct wizlistgen* list_wiz) {
	//FIXME: Use c string instead og static char buffer
#define SBB 20480
	char buf[512];
	static char bigbuf[SBB+1];
	int center, i, j, ciclo;
	bigbuf[0] = '\0';
	//bigbuf[SBB] = '\0';
	switch(livello) {
	case IMMENSO:
		sprintf(buf, "$c0011-* Immenso *-$c0007\n\r");
		if(list_wiz->number[livello] > 1) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case MAESTRO_DEI_CREATORI:
		sprintf(buf, "$c0011-* Maestro dei Creatori *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case MAESTRO_DEL_CREATO:
		sprintf(buf, "$c0011-* Maestro del Creato *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case QUESTMASTER:
		sprintf(buf, "$c0011-* Maestro del Fato *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case CREATORE:
		sprintf(buf, "$c0011-* Creatore *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case MAESTRO_DEGLI_DEI:
		sprintf(buf, "$c0011-* Maestro degli Dei *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case DIO:
		sprintf(buf, "$c0011-* Dio *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case DIO_MINORE:
		sprintf(buf, "$c0011 -* Dio Minore *-$c0007\n\r");
		if(list_wiz->number[livello] > 0) {
			ciclo = list_wiz->number[livello];
		}
		else {
			ciclo = 1;
		}
		break;
	case IMMORTALE:
		sprintf(buf, "$c0011-* Immortale *-$c0007\n\r");
		break;
	case PRINCIPE:
		sprintf(buf, "$c0011-* Principi *-$c0007\n\r");
		break;
	}
	/*   if (list_wiz->number[livello]==0)
	 return("\0"); */
	center = 38 - (int)(Ansi_len(buf) / 2);
	for(i = 0; i <= center; i++) {
		strncat(bigbuf, " ",SBB -strlen(buf));
	}
	strncat(bigbuf, buf,SBB -strlen(buf));
	for(i = 0; i < list_wiz->number[livello]; i++) {
		sprintf(buf, "%s %s$c0007\n\r", list_wiz->lookup[livello].stuff[i].name,
				list_wiz->lookup[livello].stuff[i].title);

		center = 38 - (int)(Ansi_len(buf) / 2);
		for(j = 0; j <= center; j++) {
			strncat(bigbuf, " ", SBB -strlen(buf));
		}
		strncat(bigbuf, buf, SBB -strlen(buf));
	}
	for(; livello > DIO_MINORE && i < ciclo; i++) {
		sprintf(buf, "%s %s$c0007\n\r", " ", " ");

		center = 38 - (int)(Ansi_len(buf) / 2);
		for(j = 0; j <= center; j++) {
			strncat(bigbuf, " ", SBB -strlen(buf));
		}
		strncat(bigbuf, buf, SBB -strlen(buf));
	}
	return (bigbuf);
}
/**
 * Search a toon name in the
 */
bool getFromDb(const char* cname,const char* pwd, const char* title) {
	string name(cname);
	DB* db = Sql::getMysql();
	//odb::session s;
	odb::transaction t(db->begin());
	t.tracer(logTracer);
	toon pg("","");
	if(!db->query_one<toon>(toonQuery::name==name,pg)) {
		pg.name=name;
		pg.password.assign(pwd);
		pg.title.assign(title);
		mudlog(LOG_CONNECT, "Creating record for %s", pg.name.c_str());
		try {
			db->persist<toon>(pg);
			t.commit();
			return true;
		}
		catch(odb::exception &e) {
			mudlog(LOG_ERROR, "Error creating record for %s %s", pg.name.c_str(),e.what());
		}
	}
	t.commit();
	return false;
}

/* generate index table for the player file */
void build_player_index() {
	using namespace boost::filesystem;
	struct wizlistgen list_wiz;
	int j, i;
	char buf[512];

	/* might use ABS_MAX_CLASS here some time */
	for(j = 0; j < MAX_CLASS; j++) {
		list_wiz.number[j] = 0;
	}

	top_of_p_table = 0;
	path p(current_path());
	p/=PLAYERS_DIR; // Overloaded operator: concats adding path separator
	vector<path> todelete;
	if(exists(p) and is_directory(p)) {
		for(auto &entry : boost::make_iterator_range(directory_iterator(p), directory_iterator())) {
			const path &file=entry.path();
			if(is_regular_file(file) and file.extension()==".dat") {
				FILE* pFile;
				struct char_file_u Player;
				if(!(pFile = fopen(file.c_str(), "r"))) {
					continue;
				}
				if(fread(&Player, 1, sizeof(Player), pFile)
						== sizeof(Player)) {
					int max;
					if(strcasecmp(file.stem().c_str(),Player.name)) {
						mudlog(LOG_SYSERR,"Strangeness: %s contains wrong name %s",file.filename().c_str(),Player.name);
						todelete.push_back(file);
						fclose(pFile);
						continue;
					}
					if(forceDbInit and not getFromDb(Player.name,Player.pwd,Player.title)) {
						mudlog(LOG_ERROR,"Updated: %s for %s",file.filename().c_str(),Player.name);

					}
					top_of_p_table++;

					for(j = 0, max = 0; j < MAX_CLASS; j++) {
						if(Player.level[j] > MAX_IMMORT) {
							Player.level[j] = 0;
							max = ABS_MAX_LVL + 1;
						}

						if(Player.level[j] > max) {
							max = Player.level[j];
						}
					}
					if(max >= (ABS_MAX_LVL + 1)) {
						mudlog(LOG_ERROR,
							   "ERR: %s, Levels [%d][%d][%d][%d][%d][%d][%d][%d]",
							   Player.name,
							   static_cast<unsigned int>(Player.level[0]),
							   static_cast<unsigned int>(Player.level[1]),
							   static_cast<unsigned int>(Player.level[2]),
							   static_cast<unsigned int>(Player.level[3]),
							   static_cast<unsigned int>(Player.level[4]),
							   static_cast<unsigned int>(Player.level[5]),
							   static_cast<unsigned int>(Player.level[6]),
							   static_cast<unsigned int>(Player.level[7]));
						mudlog(LOG_CHECK, "ERR: %s", file.c_str());
					}
                    else if(max >= PRINCIPE) { // Montero 10-Sep-2018 db.cpp: cambiato MAESTRO_DEL_CREATO in PRINCIPE per generare le liste principi e immortali
						/*		       (max==PRINCIPE && Player.points.exp>=PRINCEEXP) */
						/**Modifica Urhar sull' esperienza dei principi: con il nuovo livello
						 il check sui px non e' piu' necessario */
                        /* Montero 11-Sep-18 db.ccp: se il livello è IMM o GOD scrivo i livelli sul LOG_CHECK */
                        if ( max > PRINCIPE)
                        {
                            sprintf(buf, "%s: %s, Levels [%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d]", max > IMMORTALE ? "GOD" : "IMM",
							   Player.name,
							   static_cast<unsigned int>(Player.level[0]),
							   static_cast<unsigned int>(Player.level[1]),
							   static_cast<unsigned int>(Player.level[2]),
							   static_cast<unsigned int>(Player.level[3]),
							   static_cast<unsigned int>(Player.level[4]),
							   static_cast<unsigned int>(Player.level[5]),
							   static_cast<unsigned int>(Player.level[6]),
                               static_cast<unsigned int>(Player.level[7]),
                               /* Aggiunte tutte le classi */
                               static_cast<unsigned int>(Player.level[8]),
                               static_cast<unsigned int>(Player.level[9]),
                               static_cast<unsigned int>(Player.level[10]));
                            mudlog(LOG_CHECK, buf);
                        } /* fine Montero 11-Sep-18 db.ccp */

						list_wiz.lookup[max].stuff[list_wiz.number[max]].name =
							(char*) strdup(Player.name);
						list_wiz.lookup[max].stuff[list_wiz.number[max]].title =
							(char*) strdup(Player.title);
                        list_wiz.number[max]++;
                    }
				}
				fclose(pFile);
                }
		}
	}
	for(auto &file : todelete) {
		mudlog(LOG_SYSERR,"Removed invalid file %s",file.c_str());
		remove(file.string());
	}

	mudlog(LOG_CHECK, "Began Wizlist Generation.");

	sprintf(wizlist, "\033[2J\033[0;0H\n\r\n\r");
	for(i = IMMENSO; i > IMMORTALE; i--) {
		strncat(wizlist, GeneraSezione(i, &list_wiz),
				sizeof(wizlist) - strlen(wizlist) - 1);
	}
	strncat(wizlist, "\n\r", sizeof(wizlist) - strlen(wizlist) - 1);
	j = 0;
	for(i = DIO_MINORE; i <= IMMENSO; i++) {
		j += list_wiz.number[i];
	}
	sprintf(buf, "$c0007Totale Dei: %d\n\r", j);
	strncat(wizlist, buf, sizeof(wizlist) - strlen(wizlist) - 1);

	/* Immortali */
	sprintf(immlist, "\033[2J\033[0;0H\n\r\n\r");
	strncat(immlist, GeneraSezione(IMMORTALE, &list_wiz),
			sizeof(immlist) - strlen(immlist) - 1);
	/* Principi */
	sprintf(princelist, "\033[2J\033[0;0H\n\r\n\r");
	strncat(princelist, GeneraSezione(PRINCIPE, &list_wiz),
			sizeof(princelist) - strlen(princelist) - 1);

	return;
}

void ReplaceInIndex(struct index_data* pIndex, char* szName, int nRNum,
					int nVNum, int nTop) {

	if(nRNum < 0 || nRNum >= nTop) {
		mudlog(LOG_SYSERR, "Invalid RNum in ReplaceInIndex (db.c).");
		return;
	}

	pIndex[nRNum].iVNum = nVNum;
	pIndex[nRNum].pos = -1;
	pIndex[nRNum].name = strdup(szName);
	pIndex[nRNum].data = NULL;
}

struct index_data* InsertInIndex(struct index_data* pIndex, char* szName,
								 int nVNum, int* alloc_top, int* top) {
	if(*top >= *alloc_top) {
		if(!(pIndex = (struct index_data*) realloc(pIndex,
					  (*top + 50) * sizeof(struct index_data)))) {
			mudlog(LOG_ERROR,"%s:%s","load indices",strerror(errno));
			assert(0);
		}
		*alloc_top += 50;
	}
	pIndex[*top].iVNum = nVNum;
	pIndex[*top].pos = -1;
	pIndex[*top].name = strdup(szName);
	pIndex[*top].number = 0;
	pIndex[*top].func = 0;
	pIndex[*top].data = NULL;
	(*top)++;
	return pIndex;
}

void InsertObject(struct obj_data* pObj, int nVNum) {
	int nRNum = real_object(nVNum);
	if(nRNum < 0) {
		obj_index = InsertInIndex(obj_index, pObj->name, nVNum,
								  &top_of_alloc_objt, &top_of_objt);
	}
	else {
		ReplaceInIndex(obj_index, pObj->name, nRNum, nVNum, top_of_objt);
	}
}

void InsertMobile(struct char_data* pMob, int nVNum) {
	int nRNum = real_mobile(nVNum);
	if(nRNum < 0) {
		obj_index = InsertInIndex(mob_index, GET_NAME(pMob), nVNum,
								  &top_of_alloc_mobt, &top_of_mobt);
	}
	else {
		ReplaceInIndex(mob_index, GET_NAME(pMob), nRNum, nVNum, top_of_mobt);
	}
}

void read_object_to_memory(int nVNum) {
	int i = real_object(nVNum);
	if(i >= 0) {
		obj_index[i].data = (void*) read_object(i, REAL);
	}
}

int compare_index(const void* p1, const void* p2) {
	const struct index_data* s1, *s2;
	s1 = static_cast<const struct index_data*>(p1);
	s2 = static_cast<const struct index_data*>(p2);
	return (s1->iVNum - s2->iVNum);
}

/* generate index table for object or monster file */
struct index_data* generate_indices(FILE* fl, int* top, int* sort_top,
									int* alloc_top, const char* dirname) {
	FILE* f;
	DIR* dir;
	struct index_data* index;
	struct dirent* ent;
	long i = 0, di = 0, vnum, j;
	long bc = 2000;
	long dvnums[2000]; /* guess 2000 stored objects is enuff */
	int mobvnum = 0;
	char buf[300], tbuf[128];
	char loaded[100000];
	for(i = 0; i < 100000; i++) {
		loaded[i] = 0;
	}
	i = 0;
	/* scan main obj file */
	rewind(fl);
	for(;;) {
		if(fgets(buf, sizeof(buf), fl)) {
			if(*buf == '#') {
				if(!i) {  /* first cell */
					CREATE(index, struct index_data, bc);
				}
				else if(i >= bc) {
					if(!(index = (struct index_data*) realloc(index,
								 (i + 50) * sizeof(struct index_data)))) {
						mudlog(LOG_ERROR,"%s:%s","load indices",strerror(errno));
						assert(0);
					}
					bc += 50;
				}
				sscanf(buf, "#%d", &index[i].iVNum);
				mobvnum = index[i].iVNum;
				if(!loaded[mobvnum]) {
					loaded[mobvnum] = 1;
					sprintf(tbuf, "%s/%d", dirname, index[i].iVNum);
					/* Se non esiste nella dir obj/mob salvati */
					if((f = fopen(tbuf, "rt")) == NULL) {
						index[i].pos = ftell(fl);
						index[i].name =
							(index[i].iVNum < 99999) ?
							fread_string(fl) : strdup("omega");
					}
					else {
						index[i].pos = -1;
						fscanf(f, "#%*d\n");
						index[i].name =
							(index[i].iVNum < 99999) ?
							fread_string(f) : strdup("omega");
						dvnums[di++] = index[i].iVNum;
						fclose(f);
					}
					index[i].number = 0;
					index[i].func = 0;
					index[i].data = NULL;
					index[i].specname = NULL;
					index[i].specparms = NULL;
					i++;
				}
			}
			else {
				if(*buf == '%' && buf[1] == '%') {  /* EOF */
					break;
				}
			}
		}
		else {
			fprintf(stderr, "generate indices");
			assert(0);
		}
	}
	*sort_top = i;
	*alloc_top = bc;
	*top = *sort_top;
	mudlog(LOG_CHECK, "Sorting %s....", dirname);
	qsort(index, i, sizeof(index_data), compare_index);
	mudlog(LOG_CHECK, "Done!");
	/* scan for directory entrys */
	if((dir = opendir(dirname)) == NULL) {
		mudlog(LOG_CHECK, "Ignored missing directory %s", dirname);
		return (index);
	}
	while((ent = readdir(dir)) != NULL) {
		if(*ent->d_name == '.') {
			continue;
		}
		/* Solo file il cui nome e' interamente numerico (vnum).
		 * Altrimenti objects/34030.bak verrebbe ripreso al reboot (atoi ferma a .). */
		{
			const char* p = ent->d_name;
			bool all_digits = *p != '\0';
			for(; *p; ++p) {
				if(!isdigit(static_cast<unsigned char>(*p))) {
					all_digits = false;
					break;
				}
			}
			if(!all_digits) {
				continue;
			}
		}
		vnum = atoi(ent->d_name);
		if(vnum == 0) {
			continue;
		}
		/* search if vnum was already sorted in main database */
		for(j = 0; j < di; j++)
			if(dvnums[j] == vnum) {
				break;
			}
		if(dvnums[j] == vnum) {
			continue;
		}
		snprintf(buf, sizeof(buf)-1,"%s/%s", dirname, ent->d_name);
		if((f = fopen(buf, "rt")) == NULL) {
			mudlog(LOG_ERROR, "Can't open file %s for reading\n", buf);
			continue;
		}
		if(!i) {
			CREATE(index, struct index_data, bc);
		}
		else if(i >= bc) {
			if(!(index = (struct index_data*) realloc(index,
						 (i + 50) * sizeof(struct index_data)))) {
				mudlog(LOG_ERROR,"%s:%s","load indices",strerror(errno));
				assert(0);
			}
			bc += 50;
		}
		fscanf(f, "#%*d\n");
		index[i].iVNum = vnum;
		index[i].pos = -1;
		index[i].name =
			(index[i].iVNum < 99999) ? fread_string(f) : strdup("omega");
		index[i].number = 0;
		index[i].func = 0;
		index[i].data = NULL;
		fclose(f);
		i++;
	}
	*alloc_top = bc;
	*top = i;
	*sort_top = *top;
	mudlog(LOG_CHECK, "Extra Sorting %s ....", dirname);
	qsort(index, i, sizeof(index_data), compare_index);
	mudlog(LOG_CHECK, "Done!");
	return (index);
}

void cleanout_room(struct room_data* rp) {
	int i;
	struct extra_descr_data* exptr, *nptr;

	free(rp->name);
	rp->name = NULL;
	free(rp->description);
	rp->description = NULL;
	for(i = 0; i < 6; i++) {
		if(rp->dir_option[i]) {
			free(rp->dir_option[i]->general_description);
			free(rp->dir_option[i]->keyword);
			free(rp->dir_option[i]);
			rp->dir_option[i] = NULL;
		}
	}

	for(exptr = rp->ex_description; exptr; exptr = nptr) {
		nptr = exptr->next;
		free(exptr->keyword);
		free(exptr->description);
		free(exptr);
	}
	rp->ex_description = NULL;
}

void completely_cleanout_room(struct room_data* rp) {
	struct char_data* ch;
	struct obj_data* obj;

	while(rp->people) {
		ch = rp->people;
		act(
			"The hand of god sweeps across the land and you are swept into the Void.",
			FALSE, NULL, NULL, NULL, TO_VICT);
		char_from_room(ch);
		char_to_room(ch, 0); /* send character to the void */
	}

	while(rp->contents) {
		obj = rp->contents;
		obj_from_room(obj);
		obj_to_room(obj, 0); /* send item to the void */
	}

	cleanout_room(rp);
}

void load_one_room(FILE* fl, struct room_data* rp) {
	char chk[161];
	int bc = 0;
	long int tmp;

	struct extra_descr_data* new_descr;

	bc = sizeof(struct room_data);

	rp->name = fread_string(fl);
	if(IsTest()) {
		mudlog(LOG_WORLD, "LR: vnum %d name %s", rp->number, rp->name);
	}

	if(rp->name && *rp->name) {
		bc += strlen(rp->name);
	}
	rp->description = fread_string(fl);
	if(rp->description && *rp->description) {
		bc += strlen(rp->description);
	}

	if(top_of_zone_table >= 0) {
		int zone;
		fscanf(fl, " %*d ");

		/* OBS: Assumes ordering of input rooms */

		for(zone = 0;
				rp->number > zone_table[zone].top && zone <= top_of_zone_table;
				zone++)
			;
		if(zone > top_of_zone_table) {
			mudlog(LOG_SYSERR, "Room %ld (%s) is outside of any zone (%d/%d).\n",
				   rp->number, rp->name,zone,top_of_zone_table);
			exit(1);
		}
		rp->zone = zone;
	}
	sprintf(curfile, "Caricando stanza %ld .\n", rp->number);
	tmp = fread_number(fl);
	rp->room_flags = tmp;
	tmp = fread_number(fl);
	rp->sector_type = tmp;

	sprintf(curfile, "Letto flags= %ld , sector= %ld \n", rp->room_flags,
			rp->sector_type);
	if(tmp == -1) {
		sprintf(curfile, "Stanza con settore -1 (teleport) room: %ld %s\n",
				rp->number, rp->name);
		tmp = fread_number(fl);
		sprintf(curfile,
				"Stanza con settore -1 (teleport) room: %ld %s teletime %ld\n",
				rp->number, rp->name, tmp);
		rp->tele_time = tmp;
		tmp = fread_number(fl);
		sprintf(curfile,
				"Stanza con settore -1 (teleport) room: %ld %s teletarget %ld\n",
				rp->number, rp->name, tmp);
		rp->tele_targ = tmp;
		tmp = fread_number(fl);
		sprintf(curfile,
				"Stanza con settore -1 (teleport) room: %ld %s telemask %ld\n",
				rp->number, rp->name, tmp);
		rp->tele_mask = tmp;
		if(IS_SET(TELE_COUNT, rp->tele_mask)) {
			tmp = fread_number(fl);
			sprintf(curfile,
					"Stanza con settore -1 (teleport) room: %ld %s telecount %ld\n",
					rp->number, rp->name, tmp);
			rp->tele_cnt = tmp;
		}
		else {
			rp->tele_cnt = 0;
		}
		tmp = fread_number(fl);
		sprintf(curfile,
				"Stanza con settore -1 (teleport) room: %ld %s true sector %ld\n",
				rp->number, rp->name, tmp);
		rp->sector_type = tmp;
	}
	else {
		rp->tele_time = 0;
		rp->tele_targ = 0;
		rp->tele_mask = 0;
		rp->tele_cnt = 0;
	}

	if(tmp == SECT_WATER_NOSWIM || tmp == SECT_UNDERWATER) {
		/* river
		 * read direction and rate of flow
		 * su myst non tutte le stanze subaquee hanno la corrente, per cui
		 * uso fread_if_number che non da' errore se non trova un numero
		 * */
		tmp = fread_if_number(fl);
		rp->river_speed = tmp;
		tmp = fread_if_number(fl);
		rp->river_dir = tmp;
	}

	if(rp->room_flags & TUNNEL) {
		/* read in mobile limit on tunnel */
		tmp = fread_number(fl);
		/* corretto per evitare stanze in cui non si puo' entrare Gaia 2001 */
		rp->moblim = MAX(tmp, 1);
	}

	rp->funct = 0;
	rp->light = 0; /* Zero light sources */

	for(tmp = 0; tmp <= 5; tmp++) {
		rp->dir_option[tmp] = 0;
	}

	rp->ex_description = 0;

	while(fscanf(fl, " %160s \n", chk) == 1) {
		switch(*chk) {
		case 'D':
			setup_dir(fl, rp->number, atoi(chk + 1));
			bc += sizeof(struct room_direction_data);
			break;
		case 'E': /* extra description field */

			CREATE(new_descr, struct extra_descr_data, 1);
			bc += sizeof(struct extra_descr_data);

			new_descr->keyword = fread_string(fl);
			if(new_descr->keyword && *new_descr->keyword) {
				bc += strlen(new_descr->keyword);
			}
			else {
				sprintf(curfile, "No keyword in room %ld\n", rp->number);
			}

			new_descr->description = fread_string(fl);
			if(new_descr->description && *new_descr->description) {
				bc += strlen(new_descr->description);
			}
			else {
				sprintf(curfile, "No desc in room %ld\n", rp->number);
			}

			new_descr->next = rp->ex_description;
			rp->ex_description = new_descr;
			break;
		case 'L':
			rp->szWhenBrightAtNight = fread_string(fl);
			rp->szWhenBrightAtDay = fread_string(fl);
			break;
		case 'S': /* end of current room */

			total_bc += bc;
			room_count++;

			if(IS_SET(rp->room_flags, SAVE_ROOM)) {
				saved_rooms[number_of_saved_rooms] = rp->number;
				number_of_saved_rooms++;
			}
			else {
				FILE* fp;
				char buf[255];

				sprintf(buf, "world/%ld", rp->number);
				fp = fopen(buf, "r");
				if(fp) {
					saved_rooms[number_of_saved_rooms] = rp->number;
					number_of_saved_rooms++;
					fclose(fp);
				}
			}
			return;
		case 'C':
			/* Commento, non deve fare nulla. Il tutto deve stare su una sola
			 * linea. */
			break;
		default:
			mudlog(LOG_ERROR,
				   "unknown auxiliary code '%s' in room load of #%ld", chk,
				   rp->number)
			;
			break;
		}
	}
}

/* load the rooms */
void boot_world() {
	FILE* fl;
	long lVNum, last;
	struct room_data* rp;

#if HASH
	init_hash_table(&room_db, sizeof(struct room_data), 2048);
#else
	init_world(room_db);
#endif
	character_list = 0;
	object_list = 0;

	if(!(fl = fopen(WORLD_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","fopen",strerror(errno));
		mudlog(LOG_ERROR, "boot_world: could not open world file.");
		assert(0);
	}
	/* GG Sto meditando di dividere il file delle stanze in un file per ogni zona
	 * Questo e' un pezzo di codice che scorre una directory leggendo tutti i
	 * files presenti
	 *   DIR *dir;
	 *  struct wizlistgen list_wiz;
	 *  int j, i, center;
	 *  char buf[ 256 ];
	 *
	 *   might use ABS_MAX_CLASS here some time
	 *  for(j = 0; j < MAX_CLASS; j++)
	 *    list_wiz.number[j] = 0;
	 *
	 * top_of_p_table = 0;
	 *
	 * if( ( dir = opendir( PLAYERS_DIR ) ) != NULL )
	 * {
	 *   struct dirent *ent;
	 *   while( ( ent = readdir( dir ) ) != NULL )
	 *   {
	 *     FILE *pFile;
	 *     char szFileName[ 40 ];
	 *
	 *     if( *ent->d_name == '.' )
	 *       continue;
	 *
	 *     sprintf( szFileName, "%s/%s", PLAYERS_DIR, ent->d_name );
	 *
	 *     if( ( pFile = fopen( szFileName, "r+" ) ) != NULL )
	 */

	last = 0;
	while(1 == fscanf(fl, " #%ld\n", &lVNum)) {
		allocate_room(lVNum);
		rp = real_roomp(lVNum);
		if(rp)
#ifdef CYGWIN
			bzero((char*) rp, sizeof(*rp));
#else
			bzero(rp, sizeof(*rp));
#endif
		else {
			fprintf(stderr, "Error, room %ld not in database!(%ld)\n", lVNum,
					last);
			assert(0);
		}

		rp->number = lVNum;
		load_one_room(fl, rp);
		last = lVNum;
	}

	fclose(fl);
}

void allocate_room(long room_number) {
	if(room_number > top_of_world) {
		top_of_world = room_number;
	}
#if HASH
	hash_find_or_create(&room_db, room_number);
#else
	room_find_or_create(room_db, room_number);
#endif
}

/* read direction data */
void setup_dir(FILE* fl, long room, int dir) {
	long tmp;
	int trashroom = 0;
	struct room_data* rp, dummy;

	rp = real_roomp(room);

	if(!rp) {
		mudlog(LOG_CHECK, "Trashing room: %d", room);
		trashroom = 1;
		rp = &dummy; /* this is a quick fix to make the game */
		dummy.number = room; /* stop crashing   */
	}

	CREATE(rp->dir_option[dir], struct room_direction_data, 1);

	rp->dir_option[dir]->general_description = fread_string(fl);
	rp->dir_option[dir]->keyword = fread_string(fl);

	rp->dir_option[dir]->exit_info = fread_number(fl);

	rp->dir_option[dir]->key = fread_number(fl);

	rp->dir_option[dir]->to_room = fread_number(fl);

	tmp = -1;
	fscanf(fl, " %ld ", &tmp);
	rp->dir_option[dir]->open_cmd = tmp;
	if(trashroom) {

		/* Ho fatto lo stesso tutta la routine per posizionare correttamente
		 * il file, pero adesso rimuovo e libero la memoria
		 * */
		free(rp->dir_option[dir]->general_description);
		free(rp->dir_option[dir]->keyword);
		free(rp->dir_option[dir]);
	}

}

void boot_saved_zones() {
	DIR* dir;
	FILE* fp;
	struct dirent* ent;
	char buf[300];
	long zone;

	if((dir = opendir("zones")) == NULL) {
		mudlog(LOG_ERROR, "Unable to open zones directory.\n");
		return;
	}

	while((ent = readdir(dir)) != NULL) {
		if(*ent->d_name == '.') {
			continue;
		}
		zone = atoi(ent->d_name);
		if(!zone || zone > top_of_zone_table) {
			continue;
		}
		snprintf(buf, sizeof(buf)-1 ,"zones/%s", ent->d_name);
		if((fp = fopen(buf, "rt")) == NULL) {
			mudlog(LOG_ERROR, "Can't open file %s for reading\n", buf);
			continue;
		}
		mudlog(LOG_CHECK, "Loading saved zone %ld:%s", zone,
			   zone_table[zone].name);
		LoadZoneFile(fp, zone);
		fclose(fp);
	}
}

void boot_saved_rooms() {
	DIR* dir;
	FILE* fp;
	long oldnum = 0;
	struct dirent* ent;
	char buf[300];
	struct room_data* rp;
	long rooms = 0, vnum;

	if((dir = opendir("rooms")) == NULL) {
		mudlog(LOG_ERROR, "Unable to open rooms directory.\n");
		return;
	}

	while((ent = readdir(dir)) != NULL) {
		if(*ent->d_name == '.') {
			continue;
		}
		vnum = atoi(ent->d_name);
		if(!vnum || vnum > top_of_world) {
			continue;
		}
		snprintf(buf, sizeof(buf)-1, "rooms/%s", ent->d_name);
		if((fp = fopen(buf, "rt")) == NULL) {
			mudlog(LOG_ERROR, "Can't open file %s for reading\n", buf);
			continue;
		}
		while(!feof(fp)) {
			fscanf(fp, "#%ld\n", &oldnum);
			mudlog(LOG_CHECK, "Extra room %s (old number %d)", buf, oldnum);
			if((rp = real_roomp(vnum)) == NULL) {  /* empty room */
				rp = (struct room_data*) malloc(sizeof(struct room_data));
				if(rp)
#ifdef CYGWIN
					bzero((char*) rp, sizeof(struct room_data));
#else
					bzero(rp, sizeof(struct room_data));
#endif
#if HASH
				room_enter(&room_db, vnum, rp);
#else
				room_enter(room_db, vnum, rp);
#endif
			}
			else {
				cleanout_room(rp);
			}
			rp->number = vnum;
			load_one_room(fp, rp);
		}
		fclose(fp);
		rooms++;
	}
	if(rooms) {
		mudlog(LOG_CHECK, "Loaded %ld rooms", rooms);
	}
}

#define LOG_ZONE_ERROR(ch, type, zone, cmd) { mudlog( LOG_ERROR,"error in zone %s cmd %ld (%c) resolving %s number",zone_table[zone].name, cmd, ch, type);}
#define CKNUM(num,s) /*if (num<zone_table[zone].bottom || num>zone_table[zone].top) mudlog(LOG_ERROR,"NUMERR in %d %s\n%s %d",zone_table[zone].num,zone_table[zone].name,s,num);*/

void renum_zone_table(int spec_zone) {
	long zone, comm, start, end;
	struct reset_com* cmd;

	if(spec_zone) {
		start = end = spec_zone;
	}
	else {
		start = 0;
		end = top_of_zone_table;
	}

	for(zone = start; zone <= end; zone++) {
		for(comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++) {
			switch((cmd = zone_table[zone].cmd + comm)->command) {
			case 'M':
				CKNUM(cmd->arg1, "mob");
				cmd->arg1 = real_mobile(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('M', "mobile", zone, comm);
				}
				if(cmd->arg3 < 0 || real_roomp(cmd->arg3) == NULL) {
					LOG_ZONE_ERROR('M', "room", zone, comm);
				}
				break;
			case 'C':
				CKNUM(cmd->arg1, "mob");
				cmd->arg1 = real_mobile(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('C', "mobile", zone, comm);
				}
				break;
			case 'O':
				CKNUM(cmd->arg1, "obj");
				cmd->arg1 = real_object(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('O', "object", zone, comm);
				}
				if(cmd->arg3 != NOWHERE) {
					/*cmd->arg3 = real_room(cmd->arg3);*/
					if(cmd->arg3 < 0 || real_roomp(cmd->arg3) == NULL) {
						LOG_ZONE_ERROR('O', "room", zone, comm);
					}
				}
				break;
			case 'G':
				CKNUM(cmd->arg1, "obj");
				cmd->arg1 = real_object(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('G', "object", zone, comm);
				}
				break;
			case 'E':
				CKNUM(cmd->arg1, "obj");
				cmd->arg1 = real_object(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('E', "object", zone, comm);
				}
				break;
			case 'P':
				CKNUM(cmd->arg1, "obj");
				CKNUM(cmd->arg3, "obj");
				cmd->arg1 = real_object(cmd->arg1);
				if(cmd->arg1 < 0) {
					LOG_ZONE_ERROR('P', "object", zone, comm);
				}
				cmd->arg3 = real_object(cmd->arg3);
				if(cmd->arg3 < 0) {
					LOG_ZONE_ERROR('P', "object", zone, comm);
				}
				break;
			case 'D':
				/*cmd->arg1 = real_room(cmd->arg1);*/
				if(cmd->arg1 < 0 || real_roomp(cmd->arg1) == NULL) {
					LOG_ZONE_ERROR('D', "room", zone, comm);
				}
				break;
			}
		}
	}
}

/* load the zone table and command tables */
void boot_zones() {

	FILE* fl;
	int zon = 0, cmd_no = 0, expand, tmp, bc = 100, cc = 22, znumber;
	char* check, buf[256];

	if(!(fl = fopen(ZONE_FILE, "r"))) {
		mudlog(LOG_ERROR,"%s:%s","boot_zones",strerror(errno));
		assert(0);
	}

	for(;;) {
		/*read Riga1 = #nnn */
		fscanf(fl, " #%d\n", &znumber);
		/*read riga2 Nome della zona~ */
		check = fread_string(fl);
		if(*check == '$') {
			mudlog(LOG_WORLD,"Letto $");
			break;
		} /* end of file */
		SetStatus(check, NULL);
		/* alloc a new zone */

		if(!zon) {
			CREATE(zone_table, struct zone_data, bc);
		}
		else if(zon >= bc) {
			if(!(zone_table = (struct zone_data*) realloc(zone_table,
							  (zon + 10) * sizeof(struct zone_data)))) {
				mudlog(LOG_ERROR,"%s:%s","boot_zones realloc",strerror(errno));
				assert(0);
			}
			bc += 10;
		}
		zone_table[zon].num = znumber;
		zone_table[zon].name = check;
		zone_table[zon].bottom = -1;
		zone_table[zon].top = -1;
		zone_table[zon].lifespan = -1;
		zone_table[zon].reset_mode = -1;
		fscanf(fl, "%d", &zone_table[zon].top);
		fscanf(fl, "%d", &zone_table[zon].lifespan);
		fscanf(fl, "%d", &zone_table[zon].reset_mode);
		zone_table[zon].bottom = zon ? zone_table[zon - 1].top + 1 : 0;
		strncpy(buf, zone_table[zon].name, 20);
		buf[20] = '\0';
		mudlog(LOG_WORLD, "#%d(%d): %s %d-%d %dm %d", zon, zone_table[zon].num,
			   buf, zone_table[zon].bottom, zone_table[zon].top,
			   zone_table[zon].lifespan, zone_table[zon].reset_mode);

		/* read the command table */

		/*
		 * new code to allow the game to be 'static' i.e. all the mobs are saved
		 * in one big zone file, and restored later.
		 */

		cmd_no = 0;

		if(zon == 0) {
			cc = 20;
		}

		for(expand = 1;;) {
			if(expand) {
				if(!cmd_no) {
					CREATE(zone_table[zon].cmd, struct reset_com, cc);
				}
				else if(cmd_no >= cc) {
					cc += 5;
					if(!(zone_table[zon].cmd = (struct reset_com*) realloc(
												   zone_table[zon].cmd,
												   (cc * sizeof(struct reset_com))))) {
						mudlog(LOG_ERROR,"%s:%s","reset command load",strerror(errno));
						assert(0);
					}
				}
			}

			expand = 1;
			fscanf(fl, " "); /* skip blanks */
			fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);
			if(!strchr(" HFMCOGEPD*;SR",
					   zone_table[zon].cmd[cmd_no].command)) {
				fgets(buf, 250, fl); /* skip command */
				mudlog(LOG_CHECK, "'%c' strangeness in zon %s: %s",
					   zone_table[zon].cmd[cmd_no].command, check, buf);
				continue;
				/* Codice strano nel file zon */
			}

			if(zone_table[zon].cmd[cmd_no].command == 'S') {
				break;
			}

			if(zone_table[zon].cmd[cmd_no].command == '#') {
				mudlog(LOG_ERROR, "!!!!!!!!!!!!!!!!!!!!!!");
				fgets(buf, 80, fl); /* skip command */
				mudlog(LOG_ERROR, "-->%s", buf); /* Sta saltando una zona...*/
			}
			if(zone_table[zon].cmd[cmd_no].command == 'R') {
				fgets(buf, 80, fl); /* skip command */
				mudlog(LOG_ERROR, "R not implemented in %s", buf); /* Sta saltando una zona...*/
			}

			if(zone_table[zon].cmd[cmd_no].command == '*') {
				expand = 0;
				fgets(buf, 250, fl); /* skip command */
				continue;
			}

			if(zone_table[zon].cmd[cmd_no].command == ';') {
				expand = 0;
				fgets(buf, 250, fl); /* skip command */
				continue;
			}
			fgets(buf, 255, fl);
			zone_table[zon].cmd[cmd_no].if_flag = 0;
			zone_table[zon].cmd[cmd_no].arg1 = -1;
			zone_table[zon].cmd[cmd_no].arg2 = 0;
			zone_table[zon].cmd[cmd_no].arg3 = -1;
			zone_table[zon].cmd[cmd_no].arg4 = 0;
			sscanf(buf, " %d %d %d %d %d", &tmp,
				   &zone_table[zon].cmd[cmd_no].arg1,
				   &zone_table[zon].cmd[cmd_no].arg2,
				   &zone_table[zon].cmd[cmd_no].arg3,
				   &zone_table[zon].cmd[cmd_no].arg4);
			zone_table[zon].cmd[cmd_no].if_flag = tmp;
			/*         mudlog(LOG_CHECK,"Skipping comment: %s",buf);*/
			cmd_no++;
		}
		zon++;
		if(zon == 1) {
			/* fix the cheat */
			/*      if (fl != tmp_fl && fl != 0)
			 fclose(fl);
			 mudlog(LOG_CHECK,"File chiuso???");
			 fl = tmp_fl;*/
		}

	}
	top_of_zone_table = --zon;
	free(check);
	fclose(fl);
}

/*************************************************************************
 *  procedures for resetting, both play-time and boot-time                *
 *********************************************************************** */

/* read a mobile from MOB_FILE */
struct char_data* read_mobile(int nr, int type) {
	int i;
	long tmp, tmp2, tmp3, bc = 0;
	struct char_data* mob;
	char letter;

	i = nr;
	if(type == VIRTUAL) {
		if((nr = real_mobile(nr)) < 0) {
			mudlog(LOG_ERROR, "Mobile (V) %d does not exist in database.", i);
			return NULL;
		}
	}

	fseek(mob_f, mob_index[nr].pos, 0);

	CREATE(mob, struct char_data, 1);

	if(!mob) {
		mudlog(LOG_SYSERR, "Cannot create mob?! db.c read_mobile");
		return nullptr;
	}

	bc = sizeof(struct char_data);
	clear_char(mob);

	mob->specials.last_direction = -1; /* this is a fix for wander */

	/***** String data *** */
	mob->player.name = fread_string(mob_f);
	if(mob->player.name) {
		bc += strlen(mob->player.name);
	}
	mob->player.short_descr = fread_string(mob_f);
	if(mob->player.short_descr) {
		bc += strlen(mob->player.short_descr);
	}
	mob->player.long_descr = fread_string(mob_f);
	if(mob->player.long_descr) {
		bc += strlen(mob->player.long_descr);
	}
	mob->player.description = fread_string(mob_f);
	if(mob->player.description) {
		bc += strlen(mob->player.description);
	}
	mob->player.title = 0;

	/* *** Numeric data *** */

	mob->mult_att = 1.0;
	mob->specials.spellfail = 101;

	mob->specials.act = fread_number(mob_f);
	SET_BIT(mob->specials.act, ACT_ISNPC);
	if(IS_SET(mob->specials.act, ACT_POLYSELF)) {
		mudlog(LOG_ERROR, "ACT_POLYSELF bit set in mob #%d.",
			   mob_index[nr].iVNum);
		REMOVE_BIT(mob->specials.act, ACT_POLYSELF);
	}

	mob->specials.affected_by = fread_number(mob_f);

	mob->specials.alignment = fread_number(mob_f);

	mob->player.iClass = CLASS_WARRIOR;

	mob->player.oggetti = 0;

	fscanf(mob_f, " %c ", &letter);
    mob->specials.mobtype = letter;
	if(letter == 'S') {
		/* SIMPLE MOB */
		fscanf(mob_f, "\n");

		tmp = fread_number(mob_f);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		mob->abilities.str = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.intel = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.wis = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.dex = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.con = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.chr = MIN(10 + number(0, MAX(1, tmp / 5)), 18);

		mob->points.hitroll = 20 - fread_number(mob_f);

		tmp = fread_number(mob_f);

		if(tmp > 10 || tmp < -10) {
			tmp /= 10;
		}

		mob->points.armor = 10 * tmp;

		fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
		mob->points.max_hit = dice(tmp, tmp2) + tmp3;
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
		mob->points.damroll = tmp3;
		mob->specials.damnodice = tmp;
		mob->specials.damsizedice = tmp2;

		mob->points.max_mana = 10;
		mob->points.max_move = 50;

		tmp = fread_number(mob_f);
		if(tmp == -1) {
			mob->points.gold = fread_number(mob_f);
			GET_EXP(mob) = fread_number(mob_f);
			GET_RACE(mob) = fread_number(mob_f);
			if(IsGiant(mob)) {
				mob->abilities.str += number(1, 4);
			}
			if(IsSmall(mob)) {
				mob->abilities.str -= 1;
			}
		}
		else {
			mob->points.gold = tmp;
			GET_EXP(mob) = fread_number(mob_f);
		}
		mob->specials.position = fread_number(mob_f);

		mob->specials.default_pos = fread_number(mob_f);

		tmp = fread_number(mob_f);
		if(tmp < 3) {
			mob->player.sex = tmp;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}
		else if(tmp < 6) {
			mob->player.sex = tmp - 3;
			mob->immune = fread_number(mob_f);
			mob->M_immune = fread_number(mob_f);
			mob->susc = fread_number(mob_f);
		}
		else {
			mob->player.sex = 0;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}

		fscanf(mob_f, "\n");

		mob->player.iClass = 0;

		mob->player.time.birth = time(0);
		mob->player.time.played = 0;
		mob->player.time.logon = time(0);
		mob->player.weight = 200;
		mob->player.height = 198;

		for(i = 0; i < 3; i++) {
			GET_COND(mob, i) = -1;
		}

		for(i = 0; i < 5; i++)
			mob->specials.apply_saving_throw[i] = MAX(
					20 - GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
	}/* FINE SIMPLE */
	else if(letter == 'A' || letter == 'N' || letter == 'B' || letter == 'L') {
		if(letter == 'A' || letter == 'B' || letter == 'L') {
			mob->mult_att = (float) fread_number(mob_f);
			/*Fine prima riga con dati numerici */
#if 0
			/*  read in types: */
			for(i = 0; i < mob->mult_att && i < 10; i++) {
				mob->att_type[i] = fread_number(mob_f);
			}
#endif
		}

		fscanf(mob_f, "\n");
		/* Righe numeriche */
		/* LIVELLO THAC0 AC HITPBONUS DAMROLL
		 * -1 GOLD XPBONUS RACE or GOLD XPBONUS
		 * POSITON DEFAULTPOS SEX (se sex >3 allora IMMUNE METAIMM SUSC
		 */
		tmp = fread_number(mob_f);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		mob->abilities.str = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.intel = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.wis = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.dex = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.con = MIN(10 + number(0, MAX(1, tmp / 5)), 18);
		mob->abilities.chr = MIN(10 + number(0, MAX(1, tmp / 5)), 18);

		mob->points.hitroll = 20 - fread_number(mob_f);

		mob->points.armor = 10 * fread_number(mob_f);

		tmp = fread_number(mob_f);
		mob->points.max_hit = dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8) + tmp;
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
		mob->points.damroll = tmp3;
		mob->specials.damnodice = tmp;
		mob->specials.damsizedice = tmp2;

		mob->points.max_mana = 100;
		mob->points.max_move = 50;

		tmp = fread_number(mob_f);
		if(tmp == -1) {
			mob->points.gold = fread_number(mob_f);
			tmp = fread_number(mob_f);
			if(tmp >= 0) {
				GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);
			}
			else {
				GET_EXP(mob) = -tmp;
			}
			GET_RACE(mob) = fread_number(mob_f);
			if(IsGiant(mob)) {
				mob->abilities.str += number(1, 4);
			}
			if(IsSmall(mob)) {
				mob->abilities.str -= 1;
			}
		}
		else {
			mob->points.gold = tmp;

			/* this is where the new exp will come into play */
			tmp = fread_number(mob_f);
			if(tmp >= 0) {
				GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);
			}
			else {
				GET_EXP(mob) = -tmp;
			}
		}

		mob->specials.position = fread_number(mob_f);

		mob->specials.default_pos = fread_number(mob_f);

		tmp = fread_number(mob_f);
		if(tmp < 3) {
			mob->player.sex = tmp;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}
		else if(tmp < 6) {
			mob->player.sex = tmp - 3;
			mob->immune = fread_number(mob_f);
			mob->M_immune = fread_number(mob_f);
			mob->susc = fread_number(mob_f);
		}
		else {
			mob->player.sex = 0;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}

		/* read in the sound string for a mobile */
		if(letter == 'L') {
			mob->player.sounds = fread_string(mob_f);
			if(mob->player.sounds && *mob->player.sounds) {
				bc += strlen(mob->player.sounds);
			}

			mob->player.distant_snds = fread_string(mob_f);
			if(mob->player.distant_snds && *mob->player.distant_snds) {
				bc += strlen(mob->player.distant_snds);
			}
		}
		else {
			mob->player.sounds = 0;
			mob->player.distant_snds = 0;
		}

		if(letter == 'B') {
			SET_BIT(mob->specials.act, ACT_HUGE);
		}

		mob->player.iClass = 0;

		mob->player.time.birth = time(0);
		mob->player.time.played = 0;
		mob->player.time.logon = time(0);
		mob->player.weight = 200;
		mob->player.height = 198;

		for(i = 0; i < 3; i++) {
			GET_COND(mob, i) = -1;
		}

		for(i = 0; i < 5; i++)
			mob->specials.apply_saving_throw[i] = MAX(
					20 - GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
	}
	else {   /* The old monsters are down below here */
		fscanf(mob_f, "\n");

		fscanf(mob_f, " %ld ", &tmp);
		mob->abilities.str = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->abilities.intel = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->abilities.wis = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->abilities.dex = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		mob->abilities.con = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		fscanf(mob_f, " %ld ", &tmp2);

		mob->points.max_hit = number(tmp, tmp2);
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.armor = 10 * tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.mana = tmp;
		mob->points.max_mana = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.move = tmp;
		mob->points.max_move = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->points.gold = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		GET_EXP(mob) = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->specials.position = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->specials.default_pos = tmp;

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.sex = tmp;

		mob->player.iClass = fread_number(mob_f);

		fscanf(mob_f, " %ld ", &tmp);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		mob->abilities.chr = MIN(10 + number(0, MAX(1, tmp / 5)), 18);

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.time.birth = time(0);
		mob->player.time.played = 0;
		mob->player.time.logon = time(0);

		fscanf(mob_f, " %ld ", &tmp);
		mob->player.weight = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		mob->player.height = tmp;

		for(i = 0; i < 3; i++) {
			fscanf(mob_f, " %ld ", &tmp);
			GET_COND(mob, i) = tmp;
		}
		fscanf(mob_f, " \n ");

		for(i = 0; i < 5; i++) {
			fscanf(mob_f, " %ld ", &tmp);
			mob->specials.apply_saving_throw[i] = tmp;
		}

		fscanf(mob_f, " \n ");

		/* Set the damage as some standard 1d4 */
		mob->points.damroll = 0;
		mob->specials.damnodice = 1;
		mob->specials.damsizedice = 6;

		/* Calculate THAC0 as a formular of Level */
		mob->points.hitroll = MAX(1, GET_LEVEL(mob, WARRIOR_LEVEL_IND) - 3);
	}

	mob->tmpabilities = mob->abilities;

	for(i = 0; i < MAX_WEAR; i++) {  /* Initialisering Ok */
		mob->equipment[i] = 0;
	}

	mob->nr = nr;

	mob->desc = 0;

	if(!IS_SET(mob->specials.act, ACT_ISNPC)) {
		SET_BIT(mob->specials.act, ACT_ISNPC);
	}

	mob->generic = 0;
	mob->commandp = 0;
	mob->commandp2 = 0;
	mob->waitp = 0;

    mob->lastpkill = NULL;
    mob->lastmkill = NULL;

    mob->points.max_move = NewMobMov(mob);
    mob->points.move = mob->points.max_move;

	/* Check to see if associated with a script, if so, set it up */
	if(IS_SET(mob->specials.act, ACT_SCRIPT)) {
		REMOVE_BIT(mob->specials.act, ACT_SCRIPT);
	}

	for(i = 0; i < top_of_scripts; i++) {
		if(gpScript_data[i].iVNum == mob_index[nr].iVNum) {
			SET_BIT(mob->specials.act, ACT_SCRIPT);
			mob->script = i;
			break;
		}
	}

	/* insert in list */

	mob->next = character_list;
	character_list = mob;

#if LOW_GOLD
	if(mob->points.gold >= 10) {
		mob->points.gold /= 5;
	}
	else if(mob->points.gold > 0) {
		mob->points.gold = 1;
	}
#endif

	/* set up things that all members of the race have */
	SetRacialStuff(mob);

	/* change exp for wimpy mobs (lower) */
	if(IS_SET(mob->specials.act, ACT_WIMPY)) {
		GET_EXP(mob) -= GET_EXP(mob) / 10;
	}

	/* change exp for agressive mobs (higher) */
	if(IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		GET_EXP(mob) += GET_EXP(mob) / 10;
		/* big bonus for fully aggressive mobs for now */
		if(!IS_SET(mob->specials.act, ACT_WIMPY) ||
				IS_SET(mob->specials.act, ACT_META_AGG)) {
			GET_EXP(mob) += (GET_EXP(mob) / 2);
		}
	}

	mob->points.mana = mana_limit(mob);
	mob->points.move = move_limit(mob);

	/* set up distributed movement system */

	mob->specials.tick = mob_tick_count++;

	if(mob_tick_count == TICK_WRAP_COUNT) {
		mob_tick_count = 0;
	}

	mob_index[nr].number++;

	total_mbc += bc;
	mob_count++;

	return (mob);
}

void clone_obj_to_obj(struct obj_data* obj, struct obj_data* osrc) {
	struct extra_descr_data* new_descr, *tmp_descr;
	int i;

	if(osrc->name) {
		obj->name = strdup(osrc->name);
	}
	if(osrc->short_description) {
		obj->short_description = strdup(osrc->short_description);
	}
	if(osrc->description) {
		obj->description = strdup(osrc->description);
	}
	if(osrc->action_description) {
		obj->action_description = strdup(osrc->action_description);
	}

	/* *** numeric data *** */

	obj->obj_flags.type_flag = osrc->obj_flags.type_flag;
	obj->obj_flags.extra_flags = osrc->obj_flags.extra_flags;
    obj->obj_flags.extra_flags2 = osrc->obj_flags.extra_flags2;
	obj->obj_flags.wear_flags = osrc->obj_flags.wear_flags;
	obj->obj_flags.value[0] = osrc->obj_flags.value[0];
	obj->obj_flags.value[1] = osrc->obj_flags.value[1];
	obj->obj_flags.value[2] = osrc->obj_flags.value[2];
	obj->obj_flags.value[3] = osrc->obj_flags.value[3];
	obj->obj_flags.weight = osrc->obj_flags.weight;
	obj->obj_flags.cost = osrc->obj_flags.cost;
	obj->obj_flags.cost_per_day = osrc->obj_flags.cost_per_day;

	/* *** extra descriptions *** */

	obj->ex_description = 0;

	if(osrc->ex_description) {
		for(tmp_descr = osrc->ex_description; tmp_descr;
				tmp_descr = tmp_descr->next) {
			CREATE(new_descr, struct extra_descr_data, 1);
			new_descr->nMagicNumber = EXDESC_VALID_MAGIC;
			if(tmp_descr->keyword) {
				new_descr->keyword = strdup(tmp_descr->keyword);
			}
			if(tmp_descr->description) {
				new_descr->description = strdup(tmp_descr->description);
			}
			new_descr->next = obj->ex_description;
			obj->ex_description = new_descr;
		}
	}

	for(i = 0; i < MAX_OBJ_AFFECT; i++) {
		obj->affected[i].location = osrc->affected[i].location;
		obj->affected[i].modifier = osrc->affected[i].modifier;
	}

	if(osrc->szForbiddenWearToChar) {
		obj->szForbiddenWearToChar = strdup(osrc->szForbiddenWearToChar);
	}
	if(osrc->szForbiddenWearToRoom) {
		obj->szForbiddenWearToRoom = strdup(osrc->szForbiddenWearToRoom);
	}
}

int read_obj_from_file(struct obj_data* obj, FILE* f) {
	int i, tmp;
	long bc = 0L;
	char chk[161];
	struct extra_descr_data* new_descr;

	obj->name = fread_string(f);

	if(obj->name) {
		bc += strlen(obj->name);
	}
	obj->short_description = fread_string(f);
	if(obj->short_description) {
		bc += strlen(obj->short_description);
	}
	obj->description = fread_string(f);
	if(obj->description) {
		bc += strlen(obj->description);
	}
	obj->action_description = fread_string(f);
	if(obj->action_description) {
		bc += strlen(obj->action_description);
	}

	/* *** numeric data *** */

	obj->obj_flags.type_flag = fread_number(f);
	obj->obj_flags.extra_flags = fread_number(f);
	obj->obj_flags.wear_flags = fread_number(f);
	obj->obj_flags.value[0] = fread_number(f);
	obj->obj_flags.value[1] = fread_number(f);
	obj->obj_flags.value[2] = fread_number(f);
	obj->obj_flags.value[3] = fread_number(f);
	obj->obj_flags.weight = fread_number(f);
	obj->obj_flags.cost = fread_number(f);
	obj->obj_flags.cost_per_day = fread_number(f);

	/* *** extra descriptions *** */

	obj->ex_description = 0;

	while(fscanf(f, " %160s \n", chk) == 1 && *chk == 'E') {
		CREATE(new_descr, struct extra_descr_data, 1);
		new_descr->nMagicNumber = EXDESC_VALID_MAGIC;
		bc += sizeof(struct extra_descr_data);
		new_descr->keyword = fread_string(f);
		if(new_descr->keyword) {
			bc += strlen(new_descr->keyword);
		}
		new_descr->description = fread_string(f);
		if(new_descr->description) {
			bc += strlen(new_descr->description);
		}

		new_descr->next = obj->ex_description;
		obj->ex_description = new_descr;
	}

	for(i = 0; (i < MAX_OBJ_AFFECT) && (*chk == 'A'); i++) {
		fscanf(f, " %d ", &tmp);
		obj->affected[i].location = tmp;
		fscanf(f, " %d \n", &tmp);
		obj->affected[i].modifier = tmp;
		if(fscanf(f, " %160s \n", chk) != 1) {
			i++;
			break;
		}
	}

	for(; (i < MAX_OBJ_AFFECT); i++) {
		obj->affected[i].location = APPLY_NONE;
		obj->affected[i].modifier = 0;
	}

    if(*chk == 'F')
    {
        obj->obj_flags.extra_flags2 = fread_number(f);
    }

	SetStatus("Reading forbidden string in read_obj_from_file", NULL);

	if(*chk == 'P') {
		obj->szForbiddenWearToChar = fread_string(f);
		obj->szForbiddenWearToRoom = fread_string(f);
		fscanf(f, " %160s \n", chk);
	}
	else {
		obj->szForbiddenWearToChar = NULL;
		obj->szForbiddenWearToRoom = NULL;
	}

	SetStatus("Returning from read_obj_from_file", "None");

	return bc;
}

void write_obj_to_file(struct obj_data* obj, FILE* f, long vnumber)
{
	int i;
	struct extra_descr_data* descr;

//	fprintf(f, "#%d\n", obj->item_number >= 0 ? obj_index[obj->item_number].iVNum : 0);
	fprintf(f, "#%ld\n", vnumber);
	fwrite_string(f, obj->name);
	fwrite_string(f, obj->short_description);
	fwrite_string(f, obj->description);
	fwrite_string(f, obj->action_description);

	fprintf(f, "%d %d %d\n", obj->obj_flags.type_flag, obj->obj_flags.extra_flags, obj->obj_flags.wear_flags);
	fprintf(f, "%d %d %d %d\n", obj->obj_flags.value[0], obj->obj_flags.value[1], obj->obj_flags.value[2], obj->obj_flags.value[3]);
	fprintf(f, "%d %d %d\n", obj->obj_flags.weight, obj->obj_flags.cost, obj->obj_flags.cost_per_day);

	/* *** extra descriptions *** */
	if(obj->ex_description)
	{
		for(descr = obj->ex_description; descr; descr = descr->next)
		{
			fprintf(f, "E\n");
			fwrite_string(f, descr->keyword);
			fwrite_string(f, descr->description);
		}
	}

	for(i = 0; i < MAX_OBJ_AFFECT; i++)
	{
		if(obj->affected[i].location != APPLY_NONE)
		{
			fprintf(f, "A\n%d %d\n", obj->affected[i].location, obj->affected[i].modifier);
		}
	}

	if(obj->obj_flags.extra_flags2)
	{
		fprintf(f, "F\n");
		fprintf(f, "%d\n", obj->obj_flags.extra_flags2);
	}

	if(obj->szForbiddenWearToChar) {
		fprintf(f, "P\n");
		fwrite_string(f, obj->szForbiddenWearToChar);
		fwrite_string(f, obj->szForbiddenWearToRoom);
	}

}

/* read an object from OBJ_FILE */
struct obj_data* read_object(int nr, int type) {
	FILE* f;
	struct obj_data* obj;
	int i, tmp;
	long bc;
	char buf[300];

	SetStatus("read_object start", NULL);
	i = nr;
	if(type == VIRTUAL) {
		SetStatus("before real_object", NULL);
		nr = real_object(nr);
	}
	if(nr < 0 || nr >= top_of_objt) {
		fread_note_error();
		if(!fread_is_quiet()) {
			mudlog(LOG_ERROR, "Object (V) %d does not exist in database.", i);
		}
		return NULL;
	}

	SetStatus("before CREATE object", NULL);

	CREATE(obj, struct obj_data, 1);

	bc = sizeof(struct obj_data);

	SetStatus("before clear_object", NULL);
	clear_object(obj);

	if(obj_index[nr].data == NULL) {
		/* object haven't data structure */
		if(obj_index[nr].pos == -1) {
			/* object in external file */
			snprintf(buf, sizeof(buf)-1, "%s/%d", OBJ_DIR, obj_index[nr].iVNum);
			if((f = fopen(buf, "rt")) == NULL) {
				fread_note_error();
				if(!fread_is_quiet()) {
					mudlog(LOG_ERROR, "can't open object file for object %d",
						   obj_index[nr].iVNum);
				}
				free(obj);
				return (0);
			}
			{
				char stbuf[128];
				snprintf(stbuf, sizeof(stbuf), "read_obj file %d", obj_index[nr].iVNum);
				SetStatus(stbuf, NULL);
			}
			fscanf(f, "#%d \n", &tmp);
			obj->char_vnum = tmp;
//			fscanf(f, "#%*d \n");	formato vecchio
			SetStatus("before read_obj_from_file 1", NULL);
			read_obj_from_file(obj, f);
			fclose(f);
		}
		else {
			if(fseek(obj_f, obj_index[nr].pos, 0) == 0) {
				SetStatus("before read_obj_from_file 2", NULL);
				read_obj_from_file(obj, obj_f);
			}
			else {
				fread_note_error();
				if(!fread_is_quiet()) {
					mudlog(LOG_ERROR,
						   "Cannot seek obj file at %l for obj n. %d(%d) in "
						   "read_object (%s).", obj_index[nr].pos, nr,
						   obj_index[nr].iVNum, __FILE__);
				}
				free(obj);
				return NULL;
			}
		}
	}
	else {
		SetStatus("before clone_obj_to_obj", NULL);
		/* data for object present */
		clone_obj_to_obj(obj, (struct obj_data*) obj_index[nr].data);
	}

	SetStatus("before inzializing object", NULL);

	obj->in_room = NOWHERE;
	obj->next_content = 0;
	obj->carried_by = 0;
	obj->equipped_by = 0;
	obj->eq_pos = -1;
	obj->in_obj = 0;
	obj->contains = 0;
	obj->item_number = nr;
	obj->in_obj = 0;

	obj->next = object_list;
	object_list = obj;

	obj_index[nr].number++;

	obj_count++;

	total_obc += bc;

	// se l'oggetto e' un premio di una quest setto il bit solo se il caricamento e' 'REAL'
	if(type == REAL)
	{
		if(IsQuestItem(obj))
		{
			if(!IS_SET(obj->obj_flags.extra_flags2, ITEM2_QUEST))
			{
				SET_BIT(obj->obj_flags.extra_flags2, ITEM2_QUEST);
			}
		}
	}

	SetStatus("ending read_object", NULL);

	return (obj);
}

#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update() {
	int i;
	struct reset_q_element* update_u, *temp, *tmp2;

	/* enqueue zones */

	for(i = 0; i <= top_of_zone_table; i++) {
		if(zone_table[i].start) {
			if(zone_table[i].age < zone_table[i].lifespan
					&& zone_table[i].reset_mode) {

				(zone_table[i].age)++;
			}
			else if(zone_table[i].age < ZO_DEAD
					&& zone_table[i].reset_mode) {

				/* enqueue zone */

				CREATE(update_u, struct reset_q_element, 1);

				update_u->zone_to_reset = i;
				update_u->next = 0;

				if(!gReset_q.head) {
					gReset_q.head = gReset_q.tail = update_u;
				}
				else {
					gReset_q.tail->next = update_u;
					gReset_q.tail = update_u;
				}

				zone_table[i].age = ZO_DEAD;
			}
		}
	}

	/* dequeue zones (if possible) and reset */

	for(update_u = gReset_q.head; update_u; update_u = tmp2) {
		if(update_u->zone_to_reset > top_of_zone_table) {

			/*  this may or may not work */
			/*  may result in some lost memory, but the loss is not signifigant
			 *   over the short run
			 */
			update_u->zone_to_reset = 0;
			update_u->next = 0;
		}
		tmp2 = update_u->next;

		if(IS_SET(zone_table[update_u->zone_to_reset].reset_mode, ZONE_ALWAYS)
				|| (IS_SET(zone_table[update_u->zone_to_reset].reset_mode,
						   ZONE_EMPTY)
					&& is_empty(update_u->zone_to_reset))) {
			SetStatus("Before reset_zone", NULL);
			reset_zone(update_u->zone_to_reset);
			SetStatus("After reset_zone", NULL);
			/* dequeue */

			if(update_u == gReset_q.head) {
				gReset_q.head = gReset_q.head->next;
			}
			else {
				for(temp = gReset_q.head; temp->next != update_u;
						temp = temp->next)
					;

				if(!update_u->next) {
					gReset_q.tail = temp;
				}

				temp->next = update_u->next;
			}
			free(update_u);
		}
	}
}

#if 0

typedef struct tagZoneCommand {
	int nCmdNo;

} ZoneCommand;

void ExecuteZoneCommand(ZoneCommand* pZC, NumberType NT) {

}
#endif

#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */

/* I have gotten a memory out of bounds on this function, not sure where the */
/* problem came from... need to look for it, could possibly be a pointer */

/* going out of range or a variable not getting assigned. msw */

void reset_zone(int zone) {
	int cmd_no, nLastCmd = TRUE;
	char buf[256];
	char rbuf[256];
	struct char_data* pMob = NULL;
	struct char_data* pMaster = NULL;
	struct obj_data* pObj, *pCont;
	struct room_data* rp;
	//static int done = FALSE;
	struct char_data* pLastMob = 0;
	// Qui veniva messo il puntatore all'ultimo container utilizzato, dato che poi non veniva mai utilizzato
	// Lascio commentato nel caso scopra invece che mi era sfuggito l'utilizzo
	//struct obj_data* pLastCont = 0;
	char* s;
	int d, e, valore_max = 0;

	s = zone_table[zone].name;
	d = (zone ? (zone_table[zone - 1].top + 1) : 0);
	zone_table[zone].bottom = d;
	e = zone_table[zone].top;
	if(zone_table[zone].start == 0)
		snprintf(buf, sizeof(buf)-1, "Run time initialization of zone %s (%d), rooms (%d-%d)",
				s, zone, d, e);
	else
		snprintf(buf, sizeof(buf)-1, "Run time reset of zone %s (%d), rooms (%d-%d)", s, zone,
				d, e);

	mudlog(LOG_CHECK, buf);

	if(!zone_table[zone].cmd) {
		return;
	}

	for(cmd_no = 0;; cmd_no++) {
		if(ZCMD.command == 'S') {
			break;
		}

		if(nLastCmd || ZCMD.if_flag <= 0) {
			snprintf(rbuf,sizeof(rbuf)-1, "<%d %d %d %d %d>",
					ZCMD.if_flag, ZCMD.arg1, ZCMD.arg2, ZCMD.arg3, ZCMD.arg4);
			switch(ZCMD.command) {
			case 'M': /* read a mobile */
				SetStatus("Command M", rbuf);
				rp = real_roomp(ZCMD.arg3);
				if((ZCMD.arg2 == 0 || mob_index[ ZCMD.arg1 ].number < ZCMD.arg2)  // World cap
						&& (ZCMD.arg4 == 0 || MobRoomCount(real_mobile(ZCMD.arg1), rp) < ZCMD.arg4)// Room cap
						&& !fighting_in_room(ZCMD.arg3)//Combattimento in corso
						&& !CheckKillFile(mob_index[ZCMD.arg1].iVNum)//Disabilitato dal kill file
						&& (pMob = read_mobile(ZCMD.arg1, REAL)) != NULL// Mob esiste
						&& rp != NULL) { // stanza esiste
					pLastMob = pMaster = pMob;
					pMob->specials.zone = zone;
					char_to_room(pMob, ZCMD.arg3);
					if(IS_SET(pMob->specials.act, ACT_SENTINEL)) {
						pMob->lStartRoom = ZCMD.arg3;
					}

					if(GET_RACE(pMob) > RACE_GNOME &&
							!strchr(zone_table[ zone ].races, GET_RACE(pMob))) {
						zone_table[ zone ].races[ strlen(zone_table[ zone ].races) ] =
							GET_RACE(pMob);
					}

					nLastCmd = TRUE;
				}
				else {
					pLastMob = pMaster = pMob = NULL;
					nLastCmd = FALSE;
				}
				if(rp == NULL)
					mudlog(LOG_ERROR, "M Cannot find room #%d in zone %s",
						   ZCMD.arg3, s);
				break;

			case 'C': /* read a mobile.  Charm them to follow prev. */
				SetStatus("Command C", rbuf);
				if((ZCMD.arg2 == 0 || mob_index[ ZCMD.arg1 ].number < ZCMD.arg2)
						&& (ZCMD.arg4 == 0 || MobRoomCount(real_mobile(ZCMD.arg1), rp) < ZCMD.arg4)
						&& !CheckKillFile(mob_index[ ZCMD.arg1 ].iVNum)
						&& pMaster
						&& (pMob = read_mobile(ZCMD.arg1, REAL)) != NULL) {
					pLastMob = pMob;
					pMob->specials.zone = zone;
					if(GET_RACE(pMob) > RACE_GNOME &&
							!strchr(zone_table[ zone ].races, GET_RACE(pMob)))
						zone_table[ zone ].races[ strlen(zone_table[ zone ].races) ] =
							GET_RACE(pMob);

					char_to_room(pMob, pMaster->in_room);
					/* add the charm bit to the dude.  */
					add_follower(pMob, pMaster);
					SET_BIT(pMob->specials.affected_by, AFF_CHARM);
					SET_BIT(pMob->specials.act, ZCMD.arg3);
					nLastCmd = TRUE;
				}
				else {
					pLastMob = pMob = NULL;
					nLastCmd = FALSE;
				}
				break;

			case 'Z': /* set the last mobile to this zone */
				SetStatus("Command Z", rbuf);
				if(pLastMob) {
					pLastMob->specials.zone = zone;

					if(GET_RACE(pLastMob) > RACE_GNOME &&
							!strchr(zone_table[ zone].races,
									GET_RACE(pLastMob)))
						zone_table[zone].races[strlen(zone_table[zone].races)] =
							GET_RACE(pLastMob);
				}
				break;

			case 'O': /* read an object */
				SetStatus("Command O", rbuf);
                valore_max = ZCMD.arg2;
#if NICE_LIMITED
                if(valore_max > 0)
                {
                    valore_max *= 2;
                }
#endif
				pObj = NULL;
				nLastCmd = FALSE;
				if(ZCMD.arg1 >= 0 && (ZCMD.arg2 == 0 || obj_index[ ZCMD.arg1 ].number < valore_max)
				  ) {
					if((ZCMD.arg3 >= 0 && (rp = real_roomp(ZCMD.arg3)) != NULL)) {
						if((ZCMD.arg4 == 0 || ObjRoomCount(ZCMD.arg1, rp) < ZCMD.arg4) && (ZCMD.arg2 == 0 ||  ObjRoomCount(ZCMD.arg1, rp) < ZCMD.arg2)) {
							if((pObj = read_object(ZCMD.arg1, REAL)) != NULL) {
                                if(IS_SET(pObj->obj_flags.type_flag, ITEM_KEY) && ZCMD.arg2 == 1 && obj_index[ ZCMD.arg1 ].number > 1)
                                {
                                    extract_obj(pObj);
                                }
                                else
                                {
                                    obj_to_room(pObj, ZCMD.arg3);
                                }
								nLastCmd = TRUE;
								//if (ITEM_TYPE(pObj) == ITEM_CONTAINER)
								//{ pLastCont = pObj; }
							}
						}
					}
					else {
						mudlog(LOG_ERROR, "O Cannot find room #%d in zone %s",
							   ZCMD.arg3, s);
					}
				}
				break;

			case 'P': /* object to object */
				SetStatus("Command P", rbuf);
                valore_max = ZCMD.arg2;
#if NICE_LIMITED
                if(valore_max > 0)
                {
                    valore_max *= 2;
                }
#endif
				if(ZCMD.arg1 >= 0 &&
						(ZCMD.arg2 == 0 ||
						 obj_index[ ZCMD.arg1 ].number < valore_max) &&
						(pCont = get_obj_num(ZCMD.arg3)) != NULL &&
						(pObj = read_object(ZCMD.arg1, REAL)) != NULL) {
                    if(IS_SET(pObj->obj_flags.type_flag, ITEM_KEY) && ZCMD.arg2 == 1 && obj_index[ ZCMD.arg1 ].number > 1)
                    {
                        extract_obj(pObj);
                    }
                    else
                    {
                        obj_to_obj(pObj, pCont);
                    }
					nLastCmd = TRUE;
				}
				else {
					pObj = pCont = NULL;
					// nLastCmd = FALSE;    commentando questo viene caricato tutto nel contenitore, escluso gli oggetti maxxati
				}
				break;

			case 'G': /* obj_to_char */
				SetStatus("Command G", rbuf);
                valore_max = ZCMD.arg2;
#if NICE_LIMITED
                if(valore_max > 0)
                {
                    valore_max *= 2;
                }
#endif
				if(ZCMD.arg1 >= 0 &&
						(ZCMD.arg2 == 0 ||
						 obj_index[ ZCMD.arg1 ].number < valore_max) &&
						pLastMob && (pObj = read_object(ZCMD.arg1, REAL)) != NULL) {
                    if(IS_SET(pObj->obj_flags.type_flag, ITEM_KEY) && ZCMD.arg2 == 1 && obj_index[ ZCMD.arg1 ].number > 1)
                    {
                        extract_obj(pObj);
                    }
                    else
                    {
                        obj_to_char(pObj, pLastMob);
                    }
					//if (ITEM_TYPE(pObj) == ITEM_CONTAINER)
					//{ pLastCont = pObj; }
				}
				break;

			case 'H': /* hatred to char */
				SetStatus("Command H", rbuf);
				if(pLastMob) {
					AddHatred(pLastMob, ZCMD.arg1, ZCMD.arg2);
				}
				break;

			case 'F': /* fear to char */
				SetStatus("Command F", rbuf);
				if(pLastMob) {
					AddFears(pLastMob, ZCMD.arg1, ZCMD.arg2);
				}
				break;

			case 'E': /* object to equipment list */
				SetStatus("Command E", rbuf);
                valore_max = ZCMD.arg2;
#if NICE_LIMITED
                if(valore_max > 0)
                {
                    valore_max *= 2;
                }
#endif
				if(ZCMD.arg1 >= 0 && (ZCMD.arg2 == 0 ||
									  obj_index[ZCMD.arg1].number < valore_max) &&
						pLastMob && (pObj = read_object(ZCMD.arg1, REAL)) != NULL) {
					if(!pLastMob->equipment[ ZCMD.arg3 ]) {
                        if(IS_SET(pObj->obj_flags.type_flag, ITEM_KEY) && ZCMD.arg2 == 1 && obj_index[ ZCMD.arg1 ].number > 1)
                        {
                            extract_obj(pObj);
                        }
                        else
                        {
                            equip_char(pLastMob, pObj, ZCMD.arg3);
                        }
						//if (ITEM_TYPE(pObj) == ITEM_CONTAINER)
						//{ pLastCont = pObj; }
					}
					else {
						mudlog(LOG_ERROR, "eq error - zone %d, cmd %d, "
							   "item %d, mob %d, "
							   "loc %d in zone %s\n", zone, cmd_no,
							   obj_index[ ZCMD.arg1 ].iVNum,
							   mob_index[ pLastMob->nr ].iVNum,
							   ZCMD.arg3, s);
					}
				}
				break;

			case 'D': /* set state of door */
				SetStatus("Command D", rbuf);
				rp = real_roomp(ZCMD.arg1);
				if(rp && rp->dir_option[ZCMD.arg2]) {
					if(!IS_SET(rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR)) {
						mudlog(LOG_ERROR,
							   "Door error - zone %d, cmd %d, loc %d (fixed) in %s",
							   zone, cmd_no, ZCMD.arg1, s);
						SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR);
					}
					switch(ZCMD.arg3) {
					case 0:
						REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
						REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
						break;
					case 1:
						SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
						REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
						break;
					case 2:
						SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
						SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
						break;
					}
				}
				else {
					/* that exit doesn't exist anymore */
					mudlog(LOG_ERROR, "Exit error - zone %d, cmd %d, loc %d",
						   zone, cmd_no, ZCMD.arg1);
				}
				break;

			default:
				mudlog(LOG_ERROR, "Undef cmd '%c' in reset: zone %s cmd# %d\n\r",
					   ZCMD.command, s, cmd_no);
				break;
			}
		}
		else {
			nLastCmd = FALSE;
		}
	}

	zone_table[zone].age = 0;
	zone_table[zone].start = 1;
	SetStatus("Out of loop", rbuf);
}

#undef ZCMD

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr) {
	struct descriptor_data* i;

	for(i = descriptor_list; i; i = i->next)
		if(!i->connected)
			if(real_roomp(i->character->in_room)->zone == zone_nr) {
				return (0);
			}

	return (1);
}

/*************************************************************************
 *  stuff related to the save/load player system                                                           *
 *********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int load_char(const char* name, struct char_file_u* char_element) {
	FILE* fl;
	struct stat fileinfo;
	char szFileName[41];
	long filesize = 0;

	snprintf(szFileName, sizeof(szFileName)-1,"%s/%s.dat", PLAYERS_DIR, lower(name));
	if((fl = fopen(szFileName, "r")) != NULL) {
		if(stat(szFileName, &fileinfo)) {
			filesize = fileinfo.st_size;
		}
		else {
			filesize = 999999;
		}
		char_element->agemod = 0;
		fread(char_element, MIN(filesize, sizeof(struct char_file_u)), 1, fl);
		fclose(fl);
		/* Campi append-only assenti nei .dat vecchi. */
		if(filesize < static_cast<long>(sizeof(struct char_file_u))) {
			std::memset(&char_element->edit_pool, 0, sizeof(char_element->edit_pool));
		}
		/*
		 **  Kludge for ressurection
		 */
		char_element->talks[2] = FALSE; /* they are not dead */
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/* Load character from MySQL character_* into char_file_u. */
int load_char_mysql(const char* name, struct char_file_u* char_element) {
#if !USE_MYSQL
	(void)name;
	(void)char_element;
	return FALSE;
#else
	if(!name || !*name || !char_element) {
		return FALSE;
	}

	*char_element = char_file_u {};

	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
	if(!pg || !pg->id) {
		return FALSE;
	}

	DB* db = Sql::getMysql();
	odb::transaction t(db->begin());
	t.tracer(logTracer);

	const std::string toon_id = std::to_string(pg->id);
	MYSQL_RES* res = nullptr;

	const std::string core_stats_sql =
		"SELECT "
		"cc.class_primary, cc.sex, cc.race, cc.birth, cc.played, cc.weight, cc.height, "
		"cc.hometown, cc.description, cc.talks_0, cc.talks_1, cc.talks_2, "
		"cc.speaks, cc.user_flags, cc.extra_flags, cc.age_modifier, cc.authcode, "
		"cc.wimpy_level, cc.load_room, cc.start_room, cc.spells_to_learn, cc.alignment, "
		"cc.act, cc.affected_by, cc.affected_by2, cc.last_logon, "
		"cc.condition_drunk, cc.condition_full, cc.condition_thirst, "
		"cc.save_throw_0, cc.save_throw_1, cc.save_throw_2, cc.save_throw_3, "
		"cc.save_throw_4, cc.save_throw_5, cc.save_throw_6, cc.save_throw_7, "
		"cs.str, cs.str_add, cs.intel, cs.wis, cs.dex, cs.con, cs.chr, cs.extra, cs.extra2, "
		"cs.mana, cs.max_mana, cs.mana_gain, cs.hit, cs.max_hit, cs.hit_gain, "
		"cs.move, cs.max_move, cs.move_gain, cs.p_rune_dei, cs.points_extra1, cs.points_extra2, "
		"cs.points_extra3, cs.armor, cs.gold, cs.bank_gold, cs.exp, cs.true_exp, "
		"cs.extra_dual, cs.hitroll, cs.damroll, cs.libero, "
		"cs.edit_hp, cs.edit_mana, cs.edit_move, cs.edit_hp_regen, cs.edit_mana_regen, "
		"cs.edit_move_regen, cs.overedit_hp, cs.overedit_mana, cs.overedit_move, "
		"cs.overedit_hp_regen, cs.overedit_mana_regen, cs.overedit_move_regen, "
		"cs.edit_pool_migrated "
		"FROM character_core cc "
		"INNER JOIN character_stats cs ON cs.toon_id = cc.toon_id "
		"WHERE cc.toon_id = " + toon_id + " LIMIT 1";

	if(!mysql_query_select(db, core_stats_sql, res) || !res) {
		t.commit();
		return FALSE;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if(!row) {
		mysql_free_result(res);
		t.commit();
		return FALSE;
	}

	int c = 0;
	char_file_u& st = *char_element;

	st.iClass = static_cast<int>(sql_to_ll(row[c++]));
	st.sex = static_cast<ubyte>(sql_to_ll(row[c++]));
	st.race = static_cast<int>(sql_to_ll(row[c++]));
	st.birth = static_cast<unsigned int>(sql_to_ll(row[c++]));
	st.played = static_cast<int>(sql_to_ll(row[c++]));
	st.weight = static_cast<unsigned int>(sql_to_ll(row[c++]));
	st.height = static_cast<unsigned int>(sql_to_ll(row[c++]));
	st.hometown = static_cast<sh_int>(sql_to_ll(row[c++]));
	std::snprintf(st.description, sizeof(st.description), "%s", row[c++] ? row[c - 1] : "");
	st.talks[0] = (sql_to_ll(row[c++]) != 0);
	st.talks[1] = (sql_to_ll(row[c++]) != 0);
	st.talks[2] = (sql_to_ll(row[c++]) != 0);
	st.speaks = static_cast<int>(sql_to_ll(row[c++]));
	st.user_flags = static_cast<int>(sql_to_ll(row[c++]));
	st.extra_flags = static_cast<int>(sql_to_ll(row[c++]));
	st.agemod = static_cast<int>(sql_to_ll(row[c++]));
	std::snprintf(st.authcode, sizeof(st.authcode), "%s", row[c++] ? row[c - 1] : "");
	const int wimpy = static_cast<int>(sql_to_ll(row[c++]));
	std::snprintf(st.WimpyLevel, sizeof(st.WimpyLevel), "%03d", std::clamp(wimpy, 0, 999));
	st.load_room = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.startroom = static_cast<int>(sql_to_ll(row[c++]));
	st.spells_to_learn = static_cast<byte>(sql_to_ll(row[c++]));
	st.alignment = static_cast<int>(sql_to_ll(row[c++]));
	st.act = static_cast<unsigned int>(sql_to_ll(row[c++]));
	st.affected_by = static_cast<unsigned int>(sql_to_ll(row[c++]));
	st.affected_by2 = static_cast<unsigned int>(sql_to_ll(row[c++]));
	st.last_logon = static_cast<unsigned int>(sql_to_ll(row[c++]));
	st.conditions[0] = static_cast<int>(sql_to_ll(row[c++]));
	st.conditions[1] = static_cast<int>(sql_to_ll(row[c++]));
	st.conditions[2] = static_cast<int>(sql_to_ll(row[c++]));
	for(int i = 0; i < MAX_SAVES; ++i) {
		st.apply_saving_throw[i] = static_cast<sh_int>(sql_to_ll(row[c++]));
	}
	st.abilities.str = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.abilities.str_add = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.abilities.intel = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.abilities.wis = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.abilities.dex = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.abilities.con = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.abilities.chr = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.abilities.extra = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.abilities.extra2 = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.points.mana = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.max_mana = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.mana_gain = static_cast<ubyte>(sql_to_ll(row[c++]));
	st.points.hit = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.max_hit = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.hit_gain = static_cast<ubyte>(sql_to_ll(row[c++]));
	st.points.move = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.max_move = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.move_gain = static_cast<ubyte>(sql_to_ll(row[c++]));
	st.points.pRuneDei = static_cast<ush_int>(sql_to_ll(row[c++]));
	st.points.extra1 = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.extra2 = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.extra3 = static_cast<ubyte>(sql_to_ll(row[c++]));
	st.points.armor = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.points.gold = static_cast<int>(sql_to_ll(row[c++]));
	st.points.bankgold = static_cast<int>(sql_to_ll(row[c++]));
	st.points.exp = static_cast<int>(sql_to_ll(row[c++]));
	st.points.true_exp = static_cast<int>(sql_to_ll(row[c++]));
	st.points.extra_dual = static_cast<int>(sql_to_ll(row[c++]));
	st.points.hitroll = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.points.damroll = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.points.libero = static_cast<sbyte>(sql_to_ll(row[c++]));
	st.edit_pool.edit_hp = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.edit_mana = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.edit_move = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.edit_hp_regen = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.edit_mana_regen = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.edit_move_regen = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.overedit_hp = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.overedit_mana = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.overedit_move = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.overedit_hp_regen = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.overedit_mana_regen = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.overedit_move_regen = static_cast<sh_int>(sql_to_ll(row[c++]));
	st.edit_pool.migrated = static_cast<ubyte>(sql_to_ll(row[c++]));

	mysql_free_result(res);
	res = nullptr;

	std::snprintf(st.name, sizeof(st.name), "%s", pg->name.c_str());
	std::snprintf(st.title, sizeof(st.title), "%s", pg->title.c_str());
	std::snprintf(st.pwd, sizeof(st.pwd), "%s", pg->password.c_str());

	const std::string classes_sql =
		"SELECT class_index, level FROM character_classes WHERE toon_id = " + toon_id;
	if(mysql_query_select(db, classes_sql, res) && res) {
		while((row = mysql_fetch_row(res)) != nullptr) {
			const int idx = static_cast<int>(sql_to_ll(row[0], -1));
			const int lvl = static_cast<int>(sql_to_ll(row[1]));
			if(idx >= 0 && idx < ABS_MAX_CLASS) {
				st.level[idx] = static_cast<ubyte>(std::clamp(lvl, 0, 255));
			}
		}
		mysql_free_result(res);
		res = nullptr;
	}

	const std::string skills_sql =
		"SELECT skill_id, learned, flags, special, nummem FROM character_skills WHERE toon_id = " +
		toon_id;
	if(mysql_query_select(db, skills_sql, res) && res) {
		while((row = mysql_fetch_row(res)) != nullptr) {
			const int skill_id = static_cast<int>(sql_to_ll(row[0], -1));
			if(skill_id >= 0 && skill_id < MAX_SKILLS) {
				st.skills[skill_id].learned = static_cast<ubyte>(sql_to_ll(row[1]));
				st.skills[skill_id].flags = static_cast<byte>(sql_to_ll(row[2]));
				st.skills[skill_id].special = static_cast<byte>(sql_to_ll(row[3]));
				st.skills[skill_id].nummem = static_cast<byte>(sql_to_ll(row[4]));
			}
		}
		mysql_free_result(res);
		res = nullptr;
	}

	const std::string affects_sql =
		"SELECT slot, type, duration, modifier, location, bitvector "
		"FROM character_affects WHERE toon_id = " +
		toon_id + " AND duration > 0";
	if(mysql_query_select(db, affects_sql, res) && res) {
		while((row = mysql_fetch_row(res)) != nullptr) {
			const int slot = static_cast<int>(sql_to_ll(row[0], -1));
			if(slot >= 0 && slot < MAX_AFFECT) {
				st.affected[slot].type = static_cast<short>(sql_to_ll(row[1]));
				st.affected[slot].duration = static_cast<sh_int>(sql_to_ll(row[2]));
				st.affected[slot].modifier = static_cast<int>(sql_to_ll(row[3]));
				st.affected[slot].location = static_cast<int>(sql_to_ll(row[4]));
				st.affected[slot].bitvector = static_cast<int>(sql_to_ll(row[5]));
				st.affected[slot].next = 0;
			}
		}
		mysql_free_result(res);
		res = nullptr;
	}

	t.commit();
	st.talks[2] = FALSE; // compat load_char file-path
	return TRUE;
#endif
}

#if USE_MYSQL
static std::string refund_inventory_time_filter(bool has_time_window, long long from_epoch,
												long long to_epoch) {
	if(!has_time_window) {
		return {};
	}
	std::ostringstream filter;
	filter << " AND deleted_on BETWEEN FROM_UNIXTIME(" << from_epoch << ")"
		   << " AND FROM_UNIXTIME(" << to_epoch << ")";
	return filter.str();
}


static void dedupe_inventory_wear_pos_mysql(DB* db, const std::string& toon_id) {
	// Dopo refund SQL piu' snapshot possono riattivare lo stesso wear_pos:
	// tieni la riga piu' vecchia (id minimo) equipaggiata ed elimina le copie.
	const std::string dup_ids =
		"SELECT c1.id "
		"FROM character_inventory c1 "
		"INNER JOIN character_inventory c2 "
		"  ON c1.toon_id = c2.toon_id "
		" AND c1.wear_pos = c2.wear_pos "
		" AND c1.wear_pos > 0 "
		" AND c1.id > c2.id "
		" AND (c1.deleted = 0 OR c1.deleted IS NULL) "
		" AND (c2.deleted = 0 OR c2.deleted IS NULL) "
		"WHERE c1.toon_id = " +
		toon_id;
	db->execute(("DELETE cia FROM character_inventory_affect cia "
				 "INNER JOIN character_inventory ci ON ci.id = cia.inventory_id "
				 "INNER JOIN (" +
				 dup_ids + ") dup ON ci.id = dup.id")
					.c_str());
	db->execute(("DELETE ci FROM character_inventory ci "
				 "INNER JOIN (" +
				 dup_ids + ") dup ON ci.id = dup.id")
					.c_str());
}

static void refund_finalize_inventory_tx(DB* db, const std::string& toon_id) {
	db->execute("SET @rent_idx := -1");
	db->execute(("UPDATE character_inventory ci "
				 "INNER JOIN ("
				 "SELECT id, (@rent_idx := @rent_idx + 1) AS new_list_index "
				 "FROM character_inventory "
				 "WHERE toon_id = " + toon_id +
				 " AND (deleted = 0 OR deleted IS NULL) "
				 "ORDER BY id"
				 ") ord ON ord.id = ci.id "
				 "SET ci.list_index = ord.new_list_index")
					.c_str());
	db->execute(("UPDATE character_rent SET object_count = ("
				 "SELECT COUNT(*) FROM character_inventory "
				 "WHERE toon_id = " + toon_id +
				 " AND (deleted = 0 OR deleted IS NULL)) "
				 "WHERE toon_id = " + toon_id)
					.c_str());
	dedupe_inventory_wear_pos_mysql(db, toon_id);
}

static bool refund_detect_inventory_event(DB* db, const std::string& toon_id,
										  bool has_time_window, long long from_epoch,
										  long long to_epoch, std::string& cause_out,
										  std::string& event_time_out, bool& partial_out) {
	const std::string time_filter =
		refund_inventory_time_filter(has_time_window, from_epoch, to_epoch);
	MYSQL_RES* res = nullptr;
	const std::string full_sql =
		"SELECT deleted_for, deleted_on, COUNT(*) AS cnt "
		"FROM character_inventory "
		"WHERE toon_id = " +
		toon_id +
		" AND deleted = 1 "
		"AND deleted_for IN ('DEATH','RENT_EXPIRED','NUKE','TRAP','MANUAL')" +
		time_filter + " GROUP BY deleted_for, deleted_on "
					  "ORDER BY deleted_on DESC LIMIT 1";
	if(mysql_query_select(db, full_sql, res) && res) {
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row && row[0] && row[1]) {
			cause_out = row[0];
			event_time_out = row[1];
			/* DEATH: merge dei pezzi mancanti; altri cause full-replace. */
			partial_out = (cause_out == "DEATH");
			mysql_free_result(res);
			return true;
		}
		mysql_free_result(res);
	}

	res = nullptr;
	const std::string scrap_sql =
		"SELECT deleted_for, deleted_on, COUNT(*) AS cnt "
		"FROM character_inventory "
		"WHERE toon_id = " +
		toon_id + " AND deleted = 1 AND deleted_for = 'SCRAP'" + time_filter +
		" GROUP BY deleted_for, deleted_on "
		"ORDER BY deleted_on DESC LIMIT 1";
	if(!mysql_query_select(db, scrap_sql, res) || !res) {
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if(!row || !row[0]) {
		mysql_free_result(res);
		return false;
	}
	cause_out = row[0];
	event_time_out = row[1] ? row[1] : "";
	partial_out = true;
	mysql_free_result(res);
	return true;
}

static std::string refund_inventory_snapshot_where(const std::string& toon_id, const char* cause,
												   bool has_time_window, long long from_epoch,
												   long long to_epoch) {
	std::ostringstream where;
	where << "toon_id = " << toon_id << " AND deleted = 1 AND deleted_for = "
		  << db_sql_literal(cause, false);
	where << refund_inventory_time_filter(has_time_window, from_epoch, to_epoch);
	return where.str();
}

static std::string refund_inventory_event_filter(const std::string& event_time) {
	/* Finestra di qualche secondo: NOW() e client TZ possono non combaciare
	 * al secondo esatto su tutto il batch soft-deleted. */
	std::ostringstream filter;
	filter << " AND deleted_on >= (" << db_sql_literal(event_time.c_str(), false)
		   << " - INTERVAL 5 SECOND)"
		   << " AND deleted_on < (" << db_sql_literal(event_time.c_str(), false)
		   << " + INTERVAL 5 SECOND)";
	return filter.str();
}

static bool refund_fetch_latest_event_time(DB* db, const std::string& toon_id, const char* cause,
										   bool has_time_window, long long from_epoch,
										   long long to_epoch, std::string& event_time_out) {
	std::ostringstream sql;
	/* Preferisci il batch piu' grande (es. morte completa), poi il piu' recente. */
	sql << "SELECT deleted_on FROM character_inventory WHERE toon_id = " << toon_id
		<< " AND deleted = 1 AND deleted_for = " << db_sql_literal(cause, false);
	sql << refund_inventory_time_filter(has_time_window, from_epoch, to_epoch);
	sql << " GROUP BY deleted_on ORDER BY COUNT(*) DESC, deleted_on DESC LIMIT 1";
	MYSQL_RES* res = nullptr;
	if(!mysql_query_select(db, sql.str().c_str(), res) || !res) {
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if(!row || !row[0]) {
		mysql_free_result(res);
		return false;
	}
	event_time_out = row[0];
	mysql_free_result(res);
	return true;
}

static bool refund_merge_missing_from_snapshot_tx(DB* db, const std::string& toon_id,
												  const std::string& restore_where,
												  long* restored_count_out) {
	/* Conta pezzi attivi: per instance_id e per fingerprint senza instance. */
	std::unordered_map<unsigned long long, int> active_instances;
	std::unordered_map<std::string, int> active_fingerprints;
	{
		MYSQL_RES* res = nullptr;
		const std::string sql =
			"SELECT instance_id, item_number, value0, value1, value2, value3, "
			"wear_pos, IFNULL(obj_name,'') FROM character_inventory "
			"WHERE toon_id = " +
			toon_id + " AND (deleted = 0 OR deleted IS NULL)";
		if(!mysql_query_select(db, sql, res) || !res) {
			return false;
		}
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			const unsigned long long iid =
				static_cast<unsigned long long>(sql_to_ll(row[0], 0));
			if(iid > 0) {
				active_instances[iid] += 1;
				continue;
			}
			std::ostringstream fp;
			fp << sql_to_ll(row[1], 0) << '|' << sql_to_ll(row[2], 0) << '|'
			   << sql_to_ll(row[3], 0) << '|' << sql_to_ll(row[4], 0) << '|'
			   << sql_to_ll(row[5], 0) << '|' << sql_to_ll(row[6], 0) << '|'
			   << (row[7] ? row[7] : "");
			active_fingerprints[fp.str()] += 1;
		}
		mysql_free_result(res);
	}

	std::vector<unsigned long long> restore_ids;
	{
		MYSQL_RES* res = nullptr;
		const std::string sql =
			"SELECT id, instance_id, item_number, value0, value1, value2, value3, "
			"wear_pos, IFNULL(obj_name,'') FROM character_inventory WHERE " +
			restore_where + " ORDER BY id";
		if(!mysql_query_select(db, sql, res) || !res) {
			return false;
		}
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			const unsigned long long id =
				static_cast<unsigned long long>(sql_to_ll(row[0], 0));
			const unsigned long long iid =
				static_cast<unsigned long long>(sql_to_ll(row[1], 0));
			if(iid > 0) {
				if(active_instances[iid] > 0) {
					continue;
				}
				active_instances[iid] += 1;
				restore_ids.push_back(id);
				continue;
			}
			std::ostringstream fp;
			fp << sql_to_ll(row[2], 0) << '|' << sql_to_ll(row[3], 0) << '|'
			   << sql_to_ll(row[4], 0) << '|' << sql_to_ll(row[5], 0) << '|'
			   << sql_to_ll(row[6], 0) << '|' << sql_to_ll(row[7], 0) << '|'
			   << (row[8] ? row[8] : "");
			const std::string key = fp.str();
			if(active_fingerprints[key] > 0) {
				active_fingerprints[key] -= 1;
				continue;
			}
			restore_ids.push_back(id);
		}
		mysql_free_result(res);
	}

	if(restore_ids.empty()) {
		if(restored_count_out) {
			*restored_count_out = 0;
		}
		return true;
	}

	std::ostringstream ids;
	for(size_t i = 0; i < restore_ids.size(); ++i) {
		if(i) {
			ids << ',';
		}
		ids << restore_ids[i];
	}
	db->execute(("UPDATE character_inventory "
				 "SET deleted = 0, deleted_on = NULL, deleted_for = NULL "
				 "WHERE toon_id = " +
				 toon_id + " AND id IN (" + ids.str() + ")")
					.c_str());
	if(restored_count_out) {
		*restored_count_out = static_cast<long>(restore_ids.size());
	}
	return true;
}

static bool refund_apply_inventory_restore_tx(DB* db, const std::string& toon_id, const char* name,
											  const std::string& restore_where, bool partial_restore,
											  const char* cause_label, long* restored_count_out) {
	MYSQL_RES* res = nullptr;
	const std::string count_sql =
		"SELECT COUNT(*) FROM character_inventory WHERE " + restore_where;
	if(!mysql_query_select(db, count_sql, res) || !res) {
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	const long matching =
		(row && row[0]) ? sql_to_ll(row[0], 0) : 0;
	mysql_free_result(res);

	if(matching <= 0) {
		return false;
	}

	long restored_count = 0;
	const bool merge_death =
		partial_restore && cause_label && strcmp(cause_label, "DEATH") == 0;

	if(merge_death) {
		if(!refund_merge_missing_from_snapshot_tx(db, toon_id, restore_where,
												  &restored_count)) {
			return false;
		}
	}
	else {
		if(!partial_restore) {
			db->execute(("UPDATE character_inventory "
						 "SET deleted = 1, deleted_on = NOW(), deleted_for = 'MANUAL' "
						 "WHERE toon_id = " +
						 toon_id + " AND (deleted = 0 OR deleted IS NULL)")
							.c_str());
		}

		std::ostringstream upd;
		upd << "UPDATE character_inventory "
			   "SET deleted = 0, deleted_on = NULL, deleted_for = NULL "
			   "WHERE "
			<< restore_where;
		db->execute(upd.str().c_str());
		restored_count = matching;
	}

	refund_finalize_inventory_tx(db, toon_id);
	if(restored_count_out) {
		*restored_count_out = restored_count;
	}
	const char* restore_mode =
		merge_death ? "merge" : (partial_restore ? "partial" : "replace");
	mudlog(LOG_PLAYERS,
		   "refund_apply_inventory_restore: %s restored %d items (cause=%s, mode=%s, "
		   "snapshot=%d)",
		   name, static_cast<int>(restored_count), cause_label, restore_mode,
		   static_cast<int>(matching));
	return true;
}

static void dedupe_rent_wear_pos(struct obj_file_u* rent) {
	bool used_wear[MAX_WEAR + 1] {};

	for(int i = 0; i < MAX_OBJ_SAVE; i++) {
		obj_file_elem& o = rent->objects[i];
		if(o.item_number <= 0 || o.wearpos == 0) {
			continue;
		}
		if(o.wearpos > MAX_WEAR || used_wear[o.wearpos]) {
			o.wearpos = 0;
			continue;
		}
		used_wear[o.wearpos] = true;
	}
}

bool try_load_rent_mysql_by_parent(const char* name, struct obj_file_u* rent,
								   unsigned long long* db_inventory_ids,
								   std::vector<inventory_mysql_row>& rows) {
	rows.clear();
	if(!name || !*name || !rent) {
		return false;
	}

	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
	if(!pg || !pg->id) {
		return false;
	}

	DB* db = Sql::getMysql();
	if(!inventory_parent_id_supported_tx(db)) {
		return false;
	}

	*rent = obj_file_u {};
	const std::string toon_id = std::to_string(pg->id);
	const bool soft_delete_supported = inventory_soft_delete_supported_tx(db);
	MYSQL_RES* res = nullptr;

	const std::string rent_sql =
		"SELECT gold_left, total_cost, last_update, minimum_stay, object_count "
		"FROM character_rent WHERE toon_id = " +
		toon_id + " LIMIT 1";
	if(!mysql_query_select(db, rent_sql, res) || !res) {
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if(!row) {
		mysql_free_result(res);
		return false;
	}

	rent->gold_left = static_cast<int>(sql_to_ll(row[0]));
	rent->total_cost = static_cast<int>(sql_to_ll(row[1]));
	rent->last_update = static_cast<int>(sql_to_ll(row[2]));
	rent->minimum_stay = static_cast<int>(sql_to_ll(row[3]));
	rent->number = static_cast<int>(sql_to_ll(row[4]));
	if(rent->number < 0) {
		rent->number = 0;
	}
	if(rent->number > MAX_OBJ_SAVE) {
		rent->number = MAX_OBJ_SAVE;
	}
	if(db_inventory_ids) {
		for(int i = 0; i < MAX_OBJ_SAVE; ++i) {
			db_inventory_ids[i] = 0;
		}
	}
	mysql_free_result(res);
	res = nullptr;

	{
		std::ostringstream backfill_check;
		backfill_check << "SELECT COUNT(*) FROM character_inventory WHERE toon_id = " << toon_id
					   << " AND depth > 0 AND parent_inventory_id IS NULL";
		if(soft_delete_supported) {
			backfill_check << " AND (deleted = 0 OR deleted IS NULL)";
		}
		if(mysql_query_select(db, backfill_check.str(), res) && res) {
			if(MYSQL_ROW bf_row = mysql_fetch_row(res)) {
				if(sql_to_ll(bf_row[0], 0) > 0) {
					backfill_inventory_parent_ids_for_toon_tx(db, toon_id,
															  soft_delete_supported);
				}
			}
			mysql_free_result(res);
			res = nullptr;
		}
	}

	std::ostringstream inv_sql;
	inv_sql << "SELECT id, list_index, item_number, value0, value1, value2, value3, extra_flags, "
			   "extra_flags2, weight, timer, bitvector, obj_name, short_desc, description, "
			   "wear_pos, depth, parent_inventory_id, instance_id FROM character_inventory "
			   "WHERE toon_id = "
			<< toon_id;
	if(soft_delete_supported) {
		inv_sql << " AND (deleted = 0 OR deleted IS NULL)";
	}
	inv_sql << " ORDER BY list_index";
	if(!mysql_query_select(db, inv_sql.str(), res) || !res) {
		return false;
	}

	rows.reserve(static_cast<size_t>(rent->number));
	while((row = mysql_fetch_row(res)) != nullptr) {
		const int idx = static_cast<int>(sql_to_ll(row[1], -1));
		if(idx < 0 || idx >= MAX_OBJ_SAVE) {
			continue;
		}
		inventory_mysql_row inv_row {};
		inv_row.id = static_cast<unsigned long long>(sql_to_ll(row[0], 0));
		inv_row.list_index = idx;
		inv_row.parent_inventory_id =
			static_cast<unsigned long long>(sql_to_ll(row[17], 0));
		inv_row.instance_id = static_cast<unsigned long long>(sql_to_ll(row[18], 0));
		elem_from_db_inventory_row(row, inv_row.elem);
		rows.push_back(inv_row);

		if(db_inventory_ids) {
			db_inventory_ids[idx] = inv_row.id;
		}
		rent->objects[idx] = inv_row.elem;
	}
	mysql_free_result(res);
	res = nullptr;

	std::ostringstream aff_sql;
	aff_sql << "SELECT ci.id, cia.affect_slot, cia.location, cia.modifier "
			   "FROM character_inventory_affect cia "
			   "INNER JOIN character_inventory ci ON ci.id = cia.inventory_id "
			   "WHERE ci.toon_id = "
			<< toon_id;
	if(soft_delete_supported) {
		aff_sql << " AND (ci.deleted = 0 OR ci.deleted IS NULL)";
	}
	aff_sql << " ORDER BY ci.id, cia.affect_slot";
	if(mysql_query_select(db, aff_sql.str(), res) && res) {
		std::unordered_map<unsigned long long, size_t> row_by_id;
		row_by_id.reserve(rows.size());
		for(size_t i = 0; i < rows.size(); ++i) {
			if(rows[i].id != 0) {
				row_by_id[rows[i].id] = i;
			}
		}
		while((row = mysql_fetch_row(res)) != nullptr) {
			const unsigned long long id = static_cast<unsigned long long>(sql_to_ll(row[0], 0));
			const int slot = static_cast<int>(sql_to_ll(row[1], -1));
			const auto it = row_by_id.find(id);
			if(it == row_by_id.end() || slot < 0 || slot >= MAX_OBJ_AFFECT) {
				continue;
			}
			rows[it->second].elem.affected[slot].location =
				static_cast<short>(sql_to_ll(row[2]));
			rows[it->second].elem.affected[slot].modifier = static_cast<int>(sql_to_ll(row[3]));
			rent->objects[rows[it->second].list_index].affected[slot].location =
				rows[it->second].elem.affected[slot].location;
			rent->objects[rows[it->second].list_index].affected[slot].modifier =
				rows[it->second].elem.affected[slot].modifier;
		}
		mysql_free_result(res);
	}

	{
		bool used_wear[MAX_WEAR + 1] {};
		for(inventory_mysql_row& inv_row : rows) {
			if(inv_row.elem.item_number <= 0 || inv_row.elem.wearpos == 0) {
				continue;
			}
			if(inv_row.elem.wearpos > MAX_WEAR || used_wear[inv_row.elem.wearpos]) {
				inv_row.elem.wearpos = 0;
				if(inv_row.list_index >= 0 && inv_row.list_index < MAX_OBJ_SAVE) {
					rent->objects[inv_row.list_index].wearpos = 0;
				}
				continue;
			}
			used_wear[inv_row.elem.wearpos] = true;
		}
	}

	std::snprintf(rent->owner, sizeof(rent->owner), "%s", pg->name.c_str());
	mudlog(LOG_CHECK, "try_load_rent_mysql_by_parent: %s loaded %zu items", name, rows.size());
	return true;
}
#endif

bool load_rent_mysql(const char* name, struct obj_file_u* rent,
					 unsigned long long* db_inventory_ids) {
#if !USE_MYSQL
	(void)name;
	(void)rent;
	(void)db_inventory_ids;
	return false;
#else
	static int soft_delete_cols_cache = -1; // -1 unknown, 0 missing, 1 available.
	auto inventory_soft_delete_supported = [&](DB* db) -> bool {
		if(soft_delete_cols_cache != -1) {
			return soft_delete_cols_cache == 1;
		}
		MYSQL_RES* cols_res = nullptr;
		const std::string cols_sql =
			"SELECT COUNT(*) FROM information_schema.COLUMNS "
			"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'character_inventory' "
			"AND COLUMN_NAME IN ('deleted','deleted_on','deleted_for')";
		if(!mysql_query_select(db, cols_sql, cols_res) || !cols_res) {
			soft_delete_cols_cache = 0;
			return false;
		}
		MYSQL_ROW cols_row = mysql_fetch_row(cols_res);
		const long count = cols_row ? sql_to_ll(cols_row[0], 0) : 0;
		mysql_free_result(cols_res);
		soft_delete_cols_cache = (count >= 3) ? 1 : 0;
		return soft_delete_cols_cache == 1;
	};

	if(!name || !*name || !rent) {
		return false;
	}

	*rent = obj_file_u {};

	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
	if(!pg || !pg->id) {
		return false;
	}

	DB* db = Sql::getMysql();
	const std::string toon_id = std::to_string(pg->id);
	const bool soft_delete_supported = inventory_soft_delete_supported(db);
	MYSQL_RES* res = nullptr;

	const std::string rent_sql =
		"SELECT gold_left, total_cost, last_update, minimum_stay, object_count "
		"FROM character_rent WHERE toon_id = " +
		toon_id + " LIMIT 1";
	if(!mysql_query_select(db, rent_sql, res) || !res) {
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if(!row) {
		mysql_free_result(res);
		return false;
	}

	rent->gold_left = static_cast<int>(sql_to_ll(row[0]));
	rent->total_cost = static_cast<int>(sql_to_ll(row[1]));
	rent->last_update = static_cast<int>(sql_to_ll(row[2]));
	rent->minimum_stay = static_cast<int>(sql_to_ll(row[3]));
	rent->number = static_cast<int>(sql_to_ll(row[4]));
	if(rent->number < 0) {
		rent->number = 0;
	}
	if(rent->number > MAX_OBJ_SAVE) {
		rent->number = MAX_OBJ_SAVE;
	}
	if(db_inventory_ids) {
		for(int i = 0; i < MAX_OBJ_SAVE; ++i) {
			db_inventory_ids[i] = 0;
		}
	}
	mysql_free_result(res);
	res = nullptr;

	std::string inv_sql =
		"SELECT id, list_index, item_number, value0, value1, value2, value3, extra_flags, "
		"extra_flags2, weight, timer, bitvector, obj_name, short_desc, description, "
		"wear_pos, depth FROM character_inventory WHERE toon_id = " +
		toon_id;
	if(soft_delete_supported) {
		inv_sql += " AND deleted = 0";
	}
	inv_sql += " ORDER BY list_index";
	if(!mysql_query_select(db, inv_sql, res) || !res) {
		return false;
	}
	while((row = mysql_fetch_row(res)) != nullptr) {
		const int idx = static_cast<int>(sql_to_ll(row[1], -1));
		if(idx < 0 || idx >= MAX_OBJ_SAVE) {
			continue;
		}
		if(db_inventory_ids) {
			db_inventory_ids[idx] = static_cast<unsigned long long>(sql_to_ll(row[0], 0));
		}
		obj_file_elem& o = rent->objects[idx];
		o = obj_file_elem {};
		o.item_number = static_cast<ush_int>(sql_to_ll(row[2]));
		o.value[0] = static_cast<int>(sql_to_ll(row[3]));
		o.value[1] = static_cast<int>(sql_to_ll(row[4]));
		o.value[2] = static_cast<int>(sql_to_ll(row[5]));
		o.value[3] = static_cast<int>(sql_to_ll(row[6]));
		o.extra_flags = static_cast<int>(sql_to_ll(row[7]));
		o.extra_flags2 = static_cast<int>(sql_to_ll(row[8]));
		o.weight = static_cast<int>(sql_to_ll(row[9]));
		o.timer = static_cast<int>(sql_to_ll(row[10]));
		o.bitvector = static_cast<unsigned int>(sql_to_ll(row[11]));
		std::snprintf(o.name, sizeof(o.name), "%s", row[12] ? row[12] : "");
		std::snprintf(o.sd, sizeof(o.sd), "%s", row[13] ? row[13] : "");
		std::snprintf(o.desc, sizeof(o.desc), "%s", row[14] ? row[14] : "");
		o.wearpos = static_cast<ubyte>(sql_to_ll(row[15]));
		o.depth = static_cast<ubyte>(sql_to_ll(row[16]));
	}
	mysql_free_result(res);
	res = nullptr;

	std::string aff_sql =
		"SELECT ci.list_index, cia.affect_slot, cia.location, cia.modifier "
		"FROM character_inventory_affect cia "
		"INNER JOIN character_inventory ci ON ci.id = cia.inventory_id "
		"WHERE ci.toon_id = " + toon_id;
	if(soft_delete_supported) {
		aff_sql += " AND ci.deleted = 0";
	}
	aff_sql += " ORDER BY ci.list_index, cia.affect_slot";
	if(mysql_query_select(db, aff_sql, res) && res) {
		while((row = mysql_fetch_row(res)) != nullptr) {
			const int idx = static_cast<int>(sql_to_ll(row[0], -1));
			const int slot = static_cast<int>(sql_to_ll(row[1], -1));
			if(idx < 0 || idx >= MAX_OBJ_SAVE || slot < 0 || slot >= MAX_OBJ_AFFECT) {
				continue;
			}
			rent->objects[idx].affected[slot].location =
				static_cast<short>(sql_to_ll(row[2]));
			rent->objects[idx].affected[slot].modifier = static_cast<int>(sql_to_ll(row[3]));
		}
		mysql_free_result(res);
	}

	dedupe_rent_wear_pos(rent);

	std::snprintf(rent->owner, sizeof(rent->owner), "%s", pg->name.c_str());
	return true;
#endif
}

bool mark_scrapped_item_mysql(const char* name, const struct obj_data* obj) {
#if !USE_MYSQL
	(void)name;
	(void)obj;
	return false;
#else
	if(!name || !*name || !obj || obj->item_number < 0) {
		return false;
	}
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
		if(!pg || !pg->id) {
			return false;
		}
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string toon_id = std::to_string(pg->id);
		unsigned vnum = static_cast<unsigned>(obj_index[obj->item_number].iVNum);
		unsigned long long iid = obj->db_instance_id;
		object_instance_normalize_stored(&vnum, &iid);
		if(iid != 0) {
			const int base = object_instance_resolve_base_vnum(obj);
			if(base > 0) {
				vnum = static_cast<unsigned>(base);
			}
		}
		const int wearpos = obj->equipped_by ? static_cast<int>(obj->eq_pos) + 1 : 0;

		std::ostringstream ins;
		ins << "INSERT INTO character_inventory (toon_id, list_index, item_number, value0, "
			   "value1, value2, value3, extra_flags, extra_flags2, weight, timer, bitvector, "
			   "obj_name, short_desc, description, wear_pos, depth, instance_id, deleted, "
			   "deleted_on, deleted_for) VALUES ("
			<< toon_id << ",0," << vnum << ',' << obj->obj_flags.value[0] << ','
			<< obj->obj_flags.value[1] << ',' << obj->obj_flags.value[2] << ','
			<< obj->obj_flags.value[3] << ','
			<< static_cast<int>(obj->obj_flags.extra_flags) << ','
			<< static_cast<int>(obj->obj_flags.extra_flags2) << ','
			<< obj->obj_flags.weight << ','
			<< obj->obj_flags.timer << ','
			<< static_cast<int>(obj->obj_flags.bitvector) << ','
			<< db_sql_literal(obj->name ? obj->name : "", false) << ','
			<< db_sql_literal(obj->short_description ? obj->short_description : "", false) << ','
			<< db_sql_literal(obj->description ? obj->description : "", false) << ','
			<< wearpos << ",0,";
		if(iid > 0) {
			ins << iid;
		}
		else {
			ins << "NULL";
		}
		ins << ",1,NOW()," << db_sql_literal("SCRAP", false) << ')';
		db->execute(ins.str().c_str());

		for(int a = 0; a < MAX_OBJ_AFFECT; ++a) {
			const obj_affected_type& oa = obj->affected[a];
			if(oa.location == 0 && oa.modifier == 0) {
				continue;
			}
			std::ostringstream aff;
			aff << "INSERT INTO character_inventory_affect (inventory_id, affect_slot, "
				   "location, modifier) SELECT id, "
				<< a << ',' << static_cast<int>(oa.location) << ','
				<< static_cast<int>(oa.modifier)
				<< " FROM character_inventory WHERE id = LAST_INSERT_ID()";
			db->execute(aff.str().c_str());
		}

		t.commit();
		mudlog(LOG_PLAYERS,
			   "mark_scrapped_item_mysql: SCRAP snapshot for %s vnum %u instance %llu",
			   name, vnum, static_cast<unsigned long long>(iid));
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "mark_scrapped_item_mysql(%s): %s", name, e.what());
		return false;
	}
#endif
}

bool mark_inventory_deleted_mysql(const char* name, const char* cause) {
#if !USE_MYSQL
	(void)name;
	(void)cause;
	return false;
#else
	if(!name || !*name || !cause || !*cause) {
		return false;
	}
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
		if(!pg || !pg->id) {
			return false;
		}
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string toon_id = std::to_string(pg->id);
		std::ostringstream upd;
		upd << "UPDATE character_inventory "
			   "SET deleted = 1, deleted_on = NOW(), deleted_for = "
			<< db_sql_literal(cause, false)
			<< " WHERE toon_id = " << toon_id
			<< " AND (deleted = 0 OR deleted IS NULL)"
			/* Simbolo del clan: resta sul PG alla morte, non entra nello snapshot DEATH. */
			<< " AND wear_pos <> " << (static_cast<int>(WEAR_CLAN_SYMBOL) + 1);
		db->execute(upd.str().c_str());
		// Allinea il contatore rent al numero di righe attive ripristinate,
		// altrimenti load_rent_mysql vede number=0 e non ricarica alcun oggetto.
		db->execute(("UPDATE character_rent SET object_count = ("
					 "SELECT COUNT(*) FROM character_inventory "
					 "WHERE toon_id = " + toon_id +
					 " AND (deleted = 0 OR deleted IS NULL)) "
					 "WHERE toon_id = " + toon_id)
						.c_str());
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "mark_inventory_deleted_mysql(%s): %s", name, e.what());
		return false;
	}
#endif
}

bool refund_restore_inventory_by_cause_mysql(const char* name, const char* cause,
											 std::string* matched_cause) {
#if !USE_MYSQL
	(void)name;
	(void)cause;
	(void)matched_cause;
	return false;
#else
	if(!name || !*name || !cause || !*cause) {
		return false;
	}
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
		if(!pg || !pg->id) {
			return false;
		}
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string toon_id = std::to_string(pg->id);
		const bool partial_restore =
			strcmp(cause, "SCRAP") == 0 || strcmp(cause, "DEATH") == 0;

		std::ostringstream restore_where;
		restore_where << "toon_id = " << toon_id << " AND deleted = 1 AND deleted_for = "
					  << db_sql_literal(cause, false);
		/* SCRAP: tutte le righe scrap; DEATH/altri: batch evento (il piu' grande). */
		if(strcmp(cause, "SCRAP") != 0) {
			std::string event_time;
			if(!refund_fetch_latest_event_time(db, toon_id, cause, false, 0, 0, event_time)) {
				t.commit();
				return false;
			}
			restore_where << refund_inventory_event_filter(event_time);
		}

		long restored_count = 0;
		if(!refund_apply_inventory_restore_tx(db, toon_id, name, restore_where.str(),
											 partial_restore, cause, &restored_count)) {
			t.commit();
			return false;
		}
		t.commit();
		if(matched_cause) {
			*matched_cause = cause;
		}
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "refund_restore_inventory_by_cause_mysql(%s): %s", name, e.what());
		return false;
	}
#endif
}

bool refund_restore_inventory_mysql(const char* name, long long from_epoch, long long to_epoch,
									std::string* matched_cause) {
#if !USE_MYSQL
	(void)name;
	(void)from_epoch;
	(void)to_epoch;
	(void)matched_cause;
	return false;
#else
	if(!name || !*name) {
		return false;
	}
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
		if(!pg || !pg->id) {
			return false;
		}
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string toon_id = std::to_string(pg->id);
		const bool has_time_window = from_epoch > 0 && to_epoch > 0 && from_epoch <= to_epoch;

		std::string detected_cause;
		std::string event_time;
		bool partial_restore = false;
		if(!refund_detect_inventory_event(db, toon_id, has_time_window, from_epoch, to_epoch,
										  detected_cause, event_time, partial_restore)) {
			t.commit();
			return false;
		}

		std::string restore_where =
			refund_inventory_snapshot_where(toon_id, detected_cause.c_str(), has_time_window,
											from_epoch, to_epoch);
		/* SCRAP: tutte le righe; altrimenti filtra sul batch evento. */
		if(detected_cause != "SCRAP") {
			if(event_time.empty()) {
				t.commit();
				return false;
			}
			restore_where += refund_inventory_event_filter(event_time);
		}

		long restored_count = 0;
		if(!refund_apply_inventory_restore_tx(db, toon_id, name, restore_where, partial_restore,
											  detected_cause.c_str(), &restored_count)) {
			t.commit();
			return false;
		}
		t.commit();
		if(matched_cause) {
			*matched_cause = detected_cause;
		}
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "refund_restore_inventory_mysql(%s): %s", name, e.what());
		return false;
	}
#endif
}

bool save_rent_mysql(const char* name, const struct obj_file_u& rent) {
#if !USE_MYSQL
	(void)name;
	(void)rent;
	return false;
#else
	if(!name || !*name) {
		return false;
	}

	const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
	if(!pg || !pg->id) {
		mudlog(LOG_SYSERR, "save_rent_mysql: missing toon for %s", name);
		return false;
	}

	try {
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string toon_id = std::to_string(pg->id);
		save_rent_mysql_tx(db, toon_id, rent);
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "save_rent_mysql: %s", e.what());
		return false;
	}
#endif
}

/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u* st, struct char_data* ch) {
	int i;
	int max;

	GET_SEX(ch) = st->sex;
	ch->player.iClass = st->iClass;

	for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
		ch->player.level[i] = st->level[i];
	}

	/* to make sure all levels above the normal are 0 */
	for(i = MAX_CLASS; i < ABS_MAX_CLASS; i++) {
		ch->player.level[i] = 0;
	}
	ch->points.exp = st->points.exp;

	/* azzero i contatori delle posizioni */
	for(i = 0; i < MAX_POSITION; i++) {
		GET_TEMPO_IN(ch, i) = 0;
	}

	GET_POS_PREV(ch) = POSITION_STANDING;

	GET_RACE(ch) = st->race;

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;

	if(*st->title) {
		CREATE(ch->player.title, char, strlen(st->title) + 1);
		strcpy(ch->player.title, st->title);
	}
	else {
		GET_TITLE(ch) = 0;
	}

	if(*st->description) {
		CREATE(ch->player.description, char, strlen(st->description) + 1);
		strcpy(ch->player.description, st->description);
	}
	else {
		ch->player.description = 0;
	}

	ch->player.hometown = st->hometown;

	ch->player.time.birth = st->birth;

	ch->player.time.played = st->played;

	ch->player.time.logon = time(0);

	for(i = 0; i <= MAX_TOUNGE - 1; i++) {
		ch->player.talks[i] = st->talks[i];
	}

	ch->player.weight = st->weight;
	ch->player.height = st->height;

	ch->player.oggetti = 0;

	ch->abilities = st->abilities;
	ch->tmpabilities = st->abilities;
	mudlog(LOG_SAVE, "<-Mana/Hits prima di reload: %d/%d", GET_MAX_MANA(ch),
		   GET_MAX_HIT(ch));
	ch->points = st->points;
	ch->edit_pool = st->edit_pool;
	/* Snapshot SUBITO: affect_remove/affect_to_char chiamano affect_total →
	 * alter_* e clampano al max nudo prima di arrivare a fine funzione. */
	const sh_int load_hit = ch->points.hit;
	const sh_int load_mana = ch->points.mana;
	const sh_int load_move = ch->points.move;
	mudlog(LOG_SAVE, "<-Mana/Hits dopo reload    : %d/%d", GET_MAX_MANA(ch),
		   GET_MAX_HIT(ch));
	mudlog(LOG_SAVE, "<-MMana/MHits from points: %d/%d", ch->points.max_mana,
		   ch->points.max_hit);
	mudlog(LOG_SAVE, "<- Mana/ Hits from points: %d/%d", ch->points.mana,
		   ch->points.hit);

	SpaceForSkills(ch);

	if(IS_IMMORTAL(ch)) {
		max = 100;
	}
	else if(HowManyClasses(ch) >= 3) {
		max = 81;
	}
	else if(HowManyClasses(ch) == 2) {
		max = 86;
	}
	else {
		max = 95;
	}

	for(i = 0; i <= MAX_SKILLS - 1; i++) {
		ch->skills[i].flags = st->skills[i].flags;
		ch->skills[i].special = st->skills[i].special;
		ch->skills[i].nummem = st->skills[i].nummem;
		ch->skills[i].learned = MIN(st->skills[i].learned, max);
	}

	/* Specials */
	ch->specials.spells_to_learn = st->spells_to_learn;
	ch->specials.alignment = st->alignment;

	ch->specials.act = st->act;
	ch->specials.WimpyLevel = atoi(st->WimpyLevel);
	if(IS_SET(ch->specials.act, PLR_WIMPY)
			&& (ch->specials.WimpyLevel < 1
				|| ch->specials.WimpyLevel > GET_MAX_HIT(ch) / 3 * 2)) {
		ch->specials.WimpyLevel = GET_MAX_HIT(ch) / 5;
	}
	if(!IS_SET(ch->specials.act, PLR_WIMPY)) {
		ch->specials.WimpyLevel = 0;
	}

	ch->specials.carry_weight = 0;
	ch->specials.carry_items = 0;
	ch->specials.pmask = 0;
	ch->specials.poofin = 0;
	ch->specials.poofout = 0;
	ch->specials.group_name = 0;
	ch->points.armor = 100;
	ch->points.hitroll = 0;
	ch->points.damroll = 0;
	GET_EQ_SPELLPOWER(ch) = 0;
	ch->specials.affected_by = st->affected_by;
	ch->specials.affected_by2 = st->affected_by2;
	ch->specials.start_room = st->startroom;
	ch->player.vassallodi = 0;
	ch->specials.email = 0;
	ch->specials.realname = 0;
	ch->specials.authcode = 0;
	ch->specials.AuthorizedBy = 0;
	ch->specials.supporting = 0;
	ch->specials.bodyguarding = 0;
	ch->specials.bodyguard = 0;
	ch->specials.lastversion = 0;
	/* Reset posizioni */
	for(i = 0; i < MAX_POSITION; i++) {
		GET_TEMPO_IN(ch, i) = 0;
	}

	GET_POS_PREV(ch) = POSITION_STANDING;

	ch->player.speaks = st->speaks;
	ch->player.user_flags = st->user_flags;
	ch->player.extra_flags = st->extra_flags;
	ch->AgeModifier = st->agemod;
	CREATE(GET_NAME(ch), char, strlen(st->name) + 1);

	strcpy(GET_NAME(ch), st->name);

	/*GGPATCH*/
	if(*st->authcode) {

		CREATE(GET_AUTHCODE(ch), char, Registered::REG_CODELEN + 1);
		strncpy(GET_AUTHCODE(ch), st->authcode, Registered::REG_CODELEN);
	}
	else {
		GET_AUTHCODE(ch) = 0;
	}
	mudlog(LOG_PLAYERS, "Loading %s registrato come %s", GET_NAME(ch),
		   GET_AUTHCODE(ch));

	for(i = 0; i <= 4; i++) {
		ch->specials.apply_saving_throw[i] = 0;
	}

	for(i = 0; i <= 2; i++) {
		GET_COND(ch, i) = st->conditions[i];
	}

	/* Replace persisted affects (avoid stacking on repeated store_to_char). */
	while(ch->affected) {
		affect_remove(ch, ch->affected);
	}

	/* Add all spell effects */
	for(i = 0; i < MAX_AFFECT; i++) {
		if(st->affected[i].type) {
			if(IsInnateAffectType(st->affected[i].type)) {
				continue;
			}
			if(affected_by_spell(ch, st->affected[i].type)) {
				continue;
			}
			/* Inside file, we had to save a fake structure because reserving space for the pointer was architecture dependend
			 * Now, we copy the data in a temporary structure.
			 * Fortunately, the passed value will be copied so we dont need to allocate memory
			 */

			struct affected_type temp_affect;
			temp_affect.bitvector = st->affected[i].bitvector;
			temp_affect.duration = st->affected[i].duration;
			temp_affect.location = st->affected[i].location;
			temp_affect.modifier = st->affected[i].modifier;
			temp_affect.type = st->affected[i].type;
			temp_affect.next = (struct affected_type*) NULL;
			affect_to_char(ch, &temp_affect);
		}
	}
	mudlog(LOG_SAVE, "<-Mana/Hits dopo affect   : %d/%d", GET_MAX_MANA(ch),
		   GET_MAX_HIT(ch));

	//Acidus 2003 - poiche la load_room e definita sul file come sh_int, uso i valori
	//negativi per le reception con vnum compreso tra 32768 e 65535 (oltre non si puo)

	if(st->load_room < -2) {
		ch->in_room = st->load_room + 65536;
	}
	else {
		ch->in_room = st->load_room;
	}

	ch->term = 0;

	/* set default screen size */
	ch->size = 25;

	/* affect_total (e gli affect_* sopra) clampano al max nudo senza eq.
	 * Ripristina i valori del file: restano validi fino a dopo load_char_objs. */
	affect_total(ch);

	GET_HIT(ch) = load_hit;
	GET_MANA(ch) = load_mana;
	GET_MOVE(ch) = load_move;

	mudlog(LOG_SAVE, "<-Mana/Hits dopo affecttot : %d/%d", GET_MAX_MANA(ch),
		   GET_MAX_HIT(ch));
	ch->nMagicNumber = CHAR_VALID_MAGIC;
	/* Rimuove il FREEZE */
	if(IS_SET(ch->specials.act, PLR_FREEZE)) {
		REMOVE_BIT(ch->specials.act, PLR_FREEZE);
		mudlog(LOG_PLAYERS, "FREEZE removed from %s", GET_NAME(ch));
	}

} /* store_to_char */

/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data* ch, struct char_file_u* st) {
	int i;
	struct affected_type* af;
	struct obj_data* char_eq[MAX_WEAR];
	char buf[300];
	sh_int hit, mana, move;

	hit = GET_HIT(ch);
	mana = GET_MANA(ch);
	move = GET_MOVE(ch);

	/* Unaffect everything a character can be affected by */

	mudlog(LOG_SAVE, "Saving %s.dat", GET_NAME(ch));
	/* inizializzo area dummy */
	strcpy(st->dummy, "123456789012345678"); // SALVO la dummy e un array di 19
	for(i = 0; i < MAX_WEAR; i++) {
		if(ch->equipment[i]) {
			char_eq[i] = unequip_char(ch, i);
		}
		else {
			char_eq[i] = 0;
		}
	}
	mudlog(LOG_CHECK, "Removing all affects from %s", GET_NAME(ch));
	for(af = ch->affected, i = 0; af && i < MAX_AFFECT; af = af->next) {
		if(IsInnateAffectType(af->type)) {
			continue;
		}
		/* Inside file, we had to save a fake structure because reserving space for the pointer was architecture dependend
		 * Now, we need to assign item per item
		 */
		st->affected[i].bitvector = af->bitvector;
		st->affected[i].duration = af->duration;
		st->affected[i].location = af->location;
		st->affected[i].modifier = af->modifier;
		st->affected[i].type = af->type;
		st->affected[i].next = 0;
		/* subtract effect of the spell or the effect will be doubled */
		affect_modify(ch, st->affected[i].location,
					  st->affected[i].modifier, st->affected[i].bitvector, FALSE);
		snprintf(buf,sizeof(buf)-1, "Saving %s modifies %s by %d points", GET_NAME(ch),
				apply_types[st->affected[i].location],
				st->affected[i].modifier);
		i++;
	}
	for(; i < MAX_AFFECT; i++) {
		st->affected[i].type = 0; /* Zero signifies not used */
		st->affected[i].duration = 0;
		st->affected[i].modifier = 0;
		st->affected[i].location = 0;
		st->affected[i].bitvector = 0;
		st->affected[i].next = 0;
	}

	if(af != nullptr) {
		mudlog(LOG_CHECK, "WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");
	}

	ch->tmpabilities = ch->abilities;

	st->birth = ch->player.time.birth;
	st->played = ch->player.time.played;
	st->played += (long)(time(0) - ch->player.time.logon);
	st->last_logon = time(0);

	ch->player.time.played = st->played;
	ch->player.time.logon = time(0);

	st->hometown = ch->player.hometown;

	st->weight = GET_WEIGHT(ch);
	st->height = GET_HEIGHT(ch);
	st->sex = GET_SEX(ch);
	st->iClass = ch->player.iClass;

	for(i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
		st->level[i] = ch->player.level[i];
	}

	st->race = GET_RACE(ch);

	ch->specials.charging = 0; /* null it out to be sure. */
	ch->specials.charge_dir = -1; /* null it out */

    if(!affected_by_spell(ch,STATUS_QUEST)) {
        ch->specials.quest_ref = 0;
    }

	st->abilities = ch->abilities;

	st->points = ch->points;
	st->edit_pool = ch->edit_pool;

	st->alignment = ch->specials.alignment;
	st->spells_to_learn = ch->specials.spells_to_learn;
    if(!IS_SET(ch->specials.act, PLR_NEW_EQ))
    {
        SET_BIT(ch->specials.act, PLR_NEW_EQ);
    }
	st->act = ch->specials.act;
	st->affected_by = ch->specials.affected_by;
	st->affected_by2 = ch->specials.affected_by2;
	const int wimpy = std::clamp<int>(ch->specials.WimpyLevel, 0, 999);
	std::snprintf(st->WimpyLevel, sizeof(st->WimpyLevel), "%03d", wimpy);
	/* do not store group_name */
	st->startroom = ch->specials.start_room;
	st->extra_flags = ch->player.extra_flags;
	st->agemod = ch->AgeModifier;

	st->speaks = ch->player.speaks;
	st->user_flags = ch->player.user_flags;

	st->points.armor = 100;
	st->points.hitroll = 0;
	st->points.damroll = 0;
	/* Dopo unequip, ch->points.hit/mana/move sono gia' clampati al max nudo.
	 * Persisti i valori pre-unequip (altrimenti al login resti "nudo"). */
	st->points.hit = hit;
	st->points.mana = mana;
	st->points.move = move;

	if(GET_TITLE(ch)) {
		strcpy(st->title, GET_TITLE(ch));
	}
	else {
		*st->title = '\0';
	}

	/*GGPATCH*/

	if(GET_AUTHBY(ch) && GET_AUTHCODE(ch)) {
		strncpy(st->authcode, GET_AUTHCODE(ch), Registered::REG_CODELEN);
	}
	else {
		*st->authcode = '\0';
	}

	if(ch->player.description) {
		strcpy(st->description, ch->player.description);
	}
	else {
		*st->description = '\0';
	}

	for(i = 0; i <= MAX_TOUNGE - 1; i++) {
		st->talks[i] = ch->player.talks[i];
	}

	for(i = 0; i <= MAX_SKILLS - 1; i++) {
		st->skills[i] = ch->skills[i];
		st->skills[i].flags = ch->skills[i].flags;
		st->skills[i].special = ch->skills[i].special;
		st->skills[i].nummem = ch->skills[i].nummem;
	}

	strcpy(st->name, GET_NAME(ch));

	for(i = 0; i <= 4; i++) {
		st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];
	}

	for(i = 0; i <= 2; i++) {
		st->conditions[i] = GET_COND(ch, i);
	}

	for(af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
		if(af) {
			/* Add effect of the spell or it will be lost */
			/* When saving without quitting               */
			affect_modify(ch, st->affected[i].location,
						  st->affected[i].modifier, st->affected[i].bitvector, TRUE);
			af = af->next;
		}
	}

	{
		/* Stesso motivo del load rent: non rifare ego/align mentre si
		 * rimette l'eq appena tolta per il save. */
		const long room_bak = ch->in_room;
		ch->in_room = NOWHERE;
		for(i = 0; i < MAX_WEAR; i++) {
			if(char_eq[i]) {
				equip_char(ch, char_eq[i], i);
			}
		}
		ch->in_room = room_bak;
	}

	affect_total(ch);

	//FLYP 2003 Perdono
	/*if (strcmp(ch->has_killed,"perdonato")==0)
	 {
	 for(af = st->affected, i = 0; i<MAX_AFFECT; i++)
	 {
	 if (af && st->affected[i].bitvector==AFF2_PKILLER)
	 {
	 affect_remove (ch, af);
	 af = af->next;
	 }
	 }
	 }*/

    // rimuovo il realname se presente
    if(ch->specials.realname) {
        free(ch->specials.realname);
        ch->specials.realname = NULL;
    }

	/* Unequip ha clampato i current al max nudo; dopo re-equip ripristinali. */
	restore_char_points_after_equip(ch, hit, mana, move);
} /* Char to store */

/* write the vital data of a player to the player file */

void save_char(struct char_data* ch, sh_int load_room, int bonus) {
	struct char_file_u st;
	FILE* fl = NULL;
	char szFileName[200];
	struct char_data* tmp = NULL;

	if(!IS_PC(ch)) {
		return;
	}

	if(IS_POLY(ch)) {
		if(!ch->desc) {
			return;
		}
		tmp = ch->desc->original;
		if(!tmp) {
			return;
		}
	}
	else {
		if(!ch->desc) {
			return;
		}
	}
	if(!tmp) {
		tmp = ch;
	}

	char_to_store(tmp, &st);

	st.load_room = load_room;
	st.last_logon += bonus * 60 * 60 * 24;
	strcpy(st.pwd, ch->desc->pwd);

	bool skip_dat_file = false;
#if USE_MYSQL
	skip_dat_file = toon_is_migrated_by_name(GET_NAME(tmp));
	if(skip_dat_file) {
		mudlog(LOG_SAVE, "save_char: skip .dat file for migrated %s", GET_NAME(tmp));
	}
#endif

	sprintf(szFileName, "%s/%s.dat", PLAYERS_DIR, lower(tmp->player.name));
	if(!skip_dat_file) {
		if((fl = fopen(szFileName, "r+b")) == NULL) {
			if((fl = fopen(szFileName, "wb")) == NULL) {
				mudlog(LOG_ERROR, "Cannot create file %s for saving player.",
					   szFileName);
				return;
			}
		}

		rewind(fl);
		fwrite(&st, sizeof(struct char_file_u), 1, fl);
	}
#if USE_MYSQL
	{
		char_data* ch_to_sync = save_char_resolve_pc(ch);
		if(ch_to_sync) {
			if(!save_character_to_db(ch_to_sync, &st, nullptr, CHAR_DB_SAVE_BODY_TOON)) {
				mudlog(LOG_SYSERR, "save_char: save_character_to_db failed for %s",
					   GET_NAME(ch_to_sync));
			}
		}
	}
#endif
	if(fl) {
		fclose(fl);
	}

}
/* void save_char(struct char_data *ch, sh_int load_room)
 {
 save_char(ch,load_room,0);
 } */

/* for possible later use with qsort */
int compare(struct player_index_element* arg1,
			struct player_index_element* arg2) {
	return (str_cmp(arg1->name, arg2->name));
}

/************************************************************************
 *  procs of a (more or less) general utility nature
 *
 ********************************************************************** */

int fwrite_string(FILE* fl, char* buf) {
	if(buf) {
		return (fprintf(fl, "%s~\n", buf));
	}
	else {
		return (fprintf(fl, "~\n"));
	}
}

char* fread_string(FILE* f1) {
	char buf[ MAX_STRING_LENGTH];
	int i = 0, tmp;
	char* pReturnString = NULL;

	buf[0] = '\0';

	while(i < MAX_STRING_LENGTH - 3) {
		if((tmp = fgetc(f1)) == EOF) {
			fread_note_error();
			if(!fread_is_quiet()) {
				mudlog(LOG_ERROR, "Error '%s' reading file in fread_string",
					   strerror(errno));
			}
			break;
		}

		if(tmp == '~') {
			break;
		}

		buf[i++] = (char) tmp;
		if(buf[i - 1] == '\n') {
			buf[i++] = '\r';
		}
	}

	if(i >= MAX_STRING_LENGTH - 3) {
		/* We filled the buffer */
		fread_note_error();
		if(!fread_is_quiet()) {
			mudlog(LOG_ERROR, "Line too long (fread_string). Flushing");
		}
		while((tmp = fgetc(f1)) != EOF)
			if(tmp == '~') {
				break;
			}
	}

	buf[i] = '\0';

	fgetc(f1);

	if(strlen(buf)) {

		/*     if (!malloc(strlen(buf)+1))
		 mudlog(LOG_ERROR,"Malloc ha ritornato un puntatore nullo"); */

		PushStatus("fread_string malloc");
		pReturnString = (char*) malloc(strlen(buf) + 1);
		if(pReturnString) {
			PushStatus("fread_string strcpy");
			strcpy(pReturnString, buf);
			PopStatus();
		}
		PopStatus();

		if(pReturnString == NULL) {
			mudlog(LOG_ERROR, "Fread_string:Errore nel ritornare la stringa %s",
				   buf);
		}
		fflush(NULL);

	}

	return pReturnString;
}

/****************************************************************************
 * Legge un numero dal file puntato da pFIle. Se il numero contiene il
 * carattere | le due porzioni di numero vengono addizionate. Ad esempio
 * 4|128 diventa 132. Molto utile per i flags.
 ****************************************************************************/
long fread_number_int(FILE* pFile, const char* cmdfile, int cmdline,
					  const char* infofile) {
	long number;
	bool sign;
	char c;
	char memo[1024];
	long l;
	l = 0;
	SetStatus(infofile);
	SetLine(cmdfile, cmdline);
	do {
		c = getc(pFile);
		if(l < 1023) {
			memo[l++] = c;
		}
	}
	while(isspace(c));

	number = 0;

	sign = FALSE;
	if(c == '+') {
		c = getc(pFile);
	}
	else if(c == '-') {
		sign = TRUE;
		c = getc(pFile);
		if(l < 1023) {
			memo[l++] = c;
		}

	}

	if(!isdigit(c)) {
		memo[l] = 0;
		fread_note_error();
		if(!fread_is_quiet()) {
			char errbuf[1200];
			snprintf(errbuf, sizeof(errbuf),
					 "Fread_number: bad char %c line %s Info: %s",
					 c, memo, infofile ? infofile : "");
			mudlog(LOG_ERROR, "%s", errbuf);
			PrintStatus(1);
		}
		ungetc(c, pFile);
		return 0;
	}

	while(isdigit(c)) {
		number = number * 10 + c - '0';
		c = getc(pFile);
	}

	if(sign) {
		number = 0 - number;
	}

	if(c == '|') {
		number += fread_number(pFile);
	}
	else if(c != ' ') {
		ungetc(c, pFile);
	}

	return number;
}

long fread_if_number(FILE* pFile) {
	long number;
	bool sign;
	char c;

	do {
		c = getc(pFile);
	}
	while(isspace(c));

	number = 0;

	sign = FALSE;
	if(c == '+') {
		c = getc(pFile);
	}
	else if(c == '-') {
		sign = TRUE;
		c = getc(pFile);
	}

	if(!isdigit(c)) {
		ungetc(c, pFile);
		return 0;
	}

	while(isdigit(c)) {
		number = number * 10 + c - '0';
		c = getc(pFile);
	}

	if(sign) {
		number = 0 - number;
	}

	if(c == '|') {
		number += fread_number(pFile);
	}
	else if(c != ' ') {
		ungetc(c, pFile);
	}

	return number;
}

void fwrite_flag(FILE* pFile, unsigned long ulFlags) {
	unsigned long ulBit = 1;
	short bPrimaVolta = TRUE;

	while(ulFlags) {
		if(ulFlags & 1) {
			if(!bPrimaVolta) {
				fprintf(pFile, "|");
			}
			else {
				bPrimaVolta = FALSE;
			}

			fprintf(pFile, "%lu", ulBit);
		}
		ulBit *= 2;
		ulFlags >>= 1;
	}
}

/* release memory allocated for a char struct */
void free_char(struct char_data* ch) {
	struct affected_type* af, *pNext = NULL;
	int i;

	if(auction->item) {  // SALVO pulisco in auction
		if(auction->seller == ch) {
			auction->seller = NULL;
		}
		if(auction->buyer == ch) {
			auction->buyer = NULL;
		}
	}
#ifndef NOEVENTS
	/* cancel point updates */
	for(i = 0; i < 3; i++)
		if(GET_POINTS_EVENT(ch, i)) {
			GET_POINTS_EVENT(ch, i) = NULL;
		}
#endif

	if(ch->nMagicNumber != CHAR_VALID_MAGIC) {
		mudlog(LOG_SYSERR,
			   "Characters char %s with uncorrect magic number in free_char!",
			   GET_NAME_DESC(ch));
		return;
	}

	mudlog(LOG_CHECK, "Freeing char %s (ADDR: %p, magic %d)", GET_NAME_DESC(ch),
		   ch, ch->nMagicNumber);

	if(GET_NAME(ch)) {
		free(GET_NAME(ch));
		GET_NAME(ch) = NULL;
	}
	if(GET_PRINCE(ch)) {
		free(GET_PRINCE(ch));
		GET_PRINCE(ch) = NULL;
	}

	if(ch->specials.poofin) {
		free(ch->specials.poofin);
		ch->specials.poofin = NULL;
	}
	if(ch->specials.poofout) {
		free(ch->specials.poofout);
		ch->specials.poofout = NULL;
	}
	if(ch->specials.prompt) {
		free(ch->specials.prompt);
		ch->specials.prompt = NULL;
	}
	if(ch->specials.lastversion) {
		free(ch->specials.lastversion);
		ch->specials.lastversion = NULL;
	}
	if(ch->specials.email) {
		free(ch->specials.email);
		ch->specials.email = NULL;
	}
	if(ch->specials.realname) {
		free(ch->specials.realname);
		ch->specials.realname = NULL;
	}
	if(ch->specials.authcode) {
		free(ch->specials.authcode);
		ch->specials.authcode = NULL;
	}
	if(ch->specials.AuthorizedBy) {
		free(ch->specials.AuthorizedBy);
		ch->specials.AuthorizedBy = NULL;
	}
	if(ch->specials.supporting) {
		free(ch->specials.supporting);
		ch->specials.supporting = NULL;
	}
	if(ch->specials.bodyguarding) {
		free(ch->specials.bodyguarding);
		ch->specials.bodyguarding = NULL;
	}
	if(ch->specials.bodyguard) {
		free(ch->specials.bodyguard);
		ch->specials.bodyguard = NULL;
	}

	if(ch->player.title) {
		free(ch->player.title);
		ch->player.title = NULL;
	}
	if(ch->player.short_descr) {
		free(ch->player.short_descr);
		ch->player.short_descr = NULL;
	}
	if(ch->player.long_descr) {
		free(ch->player.long_descr);
		ch->player.long_descr = NULL;
	}
	if(ch->player.description) {
		free(ch->player.description);
		ch->player.description = NULL;
	}
	if(ch->player.sounds) {
		free(ch->player.sounds);
		ch->player.sounds = NULL;
	}
	if(ch->player.distant_snds) {
		free(ch->player.distant_snds);
		ch->player.distant_snds = NULL;
	}
	if(ch->specials.A_list) {
		for(i = 0; i < MAX_ALIAS; i++) {
			if(GET_ALIAS(ch, i)) {
				free(GET_ALIAS(ch, i));
				GET_ALIAS(ch, i) = NULL;
			}
		}
		free(ch->specials.A_list);
		ch->specials.A_list = NULL;
	}

	for(af = ch->affected; af; af = pNext) {
		pNext = af->next;
		affect_remove(ch, af);
	}

    if(ch->lastpkill)       // destroy
    {
        free(ch->lastpkill);
        ch->lastpkill = NULL;
    }

    if(ch->lastmkill)       // quests
    {
        free(ch->lastmkill);
        ch->lastmkill = NULL;
    }

	if(ch->skills) {
		free(ch->skills);
		ch->skills = NULL;
	}
	if(ch->desc) {
		ch->desc->character = nullptr;
		ch->desc = nullptr;
	}
	if(ch->nMagicNumber != CHAR_FREEDED_MAGIC) {
		ch->nMagicNumber = CHAR_FREEDED_MAGIC;
		free(ch);
	}

}

/* Sposta objects/<vnum> sotto deleted/objects/ e invalida obj_index. */
bool archive_object_file(int vnum, std::string& err) {
	char src[256];
	char dest[320];
	struct stat st;

	if(vnum < 1) {
		err = "vnum non valido";
		return false;
	}

	snprintf(src, sizeof(src), "%s/%d", OBJ_DIR, vnum);
	if(stat(src, &st) != 0 || !S_ISREG(st.st_mode)) {
		err = "file non trovato in objects/";
		return false;
	}

	auto ensure_dir = [](const char* path) -> bool {
		struct stat dst {};
		if(stat(path, &dst) == 0) {
			return S_ISDIR(dst.st_mode);
		}
		return mkdir(path, 0755) == 0 || errno == EEXIST;
	};

	if(!ensure_dir(DELETED_DIR) || !ensure_dir(DELETED_OBJ_DIR)) {
		err = "impossibile creare deleted/objects/";
		return false;
	}

	snprintf(dest, sizeof(dest), "%s/%d", DELETED_OBJ_DIR, vnum);
	if(stat(dest, &st) == 0) {
		snprintf(dest, sizeof(dest), "%s/%d.%ld", DELETED_OBJ_DIR, vnum,
				 static_cast<long>(time(nullptr)));
	}

	if(rename(src, dest) != 0) {
		err = "rename verso deleted/objects/ fallito";
		return false;
	}

	const int rnum = real_object(vnum);
	if(rnum >= 0) {
		if(obj_index[rnum].data) {
			free_obj(static_cast<struct obj_data*>(obj_index[rnum].data));
			obj_index[rnum].data = nullptr;
		}
		if(obj_index[rnum].name) {
			free(obj_index[rnum].name);
		}
		obj_index[rnum].name = strdup("(deleted)");
		obj_index[rnum].pos = -1;
	}

	mudlog(LOG_CHECK, "archive_object_file: %s -> %s", src, dest);
	return true;
}

/* release memory allocated for an obj struct */
void free_obj(struct obj_data* obj) {
	struct extra_descr_data* pExDescr, *next_one;

	if(!obj) {
		/* bug fix, msw */
		mudlog(LOG_SYSERR, "!obj in free_obj, db.c");
		return;
	}
	free(obj->name);
	obj->name = NULL;
	free(obj->description);
	obj->description = NULL;
	free(obj->short_description);
	obj->short_description = NULL;
	free(obj->action_description);
	obj->action_description = NULL;

	for(pExDescr = obj->ex_description; pExDescr; pExDescr = next_one) {
		if(pExDescr->nMagicNumber == EXDESC_VALID_MAGIC) {
			next_one = pExDescr->next;
			pExDescr->nMagicNumber = EXDESC_FREED_MAGIC;
			free(pExDescr->keyword);
			pExDescr->keyword = NULL;
			free(pExDescr->description);
			pExDescr->description = NULL;
			free(pExDescr);
		}
		else {
			next_one = NULL;
			mudlog(LOG_SYSERR,
				   "Invalid extra description freeing object in free_obj (db.c)");
		}
	}
	obj->ex_description = NULL;

	free(obj->szForbiddenWearToChar);
	obj->szForbiddenWearToChar = NULL;
	free(obj->szForbiddenWearToRoom);
	obj->szForbiddenWearToRoom = NULL;

	free(obj);
}

/** Legge un file di testo in buf (max MAX_STRING_LENGTH-1). Ritorna 0 ok, -1 errore. */
int file_to_string(const char* name, char* buf) {
	if(!name || !buf) {
		return -1;
	}
	buf[0] = '\0';

	std::ifstream file(name);
	if(!file.is_open()) {
		mudlog(LOG_ERROR, "Unable to open %s, continuing", name);
		return -1;
	}

	std::ostringstream oss;
	oss << file.rdbuf();
	std::string content = oss.str();

	const std::size_t max_len = MAX_STRING_LENGTH - 1;
	int rc = 0;
	if(content.size() > max_len) {
		mudlog(LOG_ERROR, "File %s too big for buffer (len: %zu)!", name,
			content.size());
		content.resize(max_len);
		rc = -1;
	}

	std::memcpy(buf, content.data(), content.size());
	buf[content.size()] = '\0';
	return rc;
}

void ClearDeadBit(struct char_data* ch) {
}

/* clear some of the the working variables of a char */
void restore_char_points_after_equip(struct char_data* ch, int hit, int mana,
									int move) {
	if(!ch) {
		return;
	}

	/* std::min: evita Alarmud::MIN e lascia i current fino al max con eq. */
	GET_HIT(ch) = std::min(hit, GET_MAX_HIT(ch));
	GET_MANA(ch) = std::min(mana, GET_MAX_MANA(ch));
	GET_MOVE(ch) = std::min(move, GET_MAX_MOVE(ch));

	if(GET_HIT(ch) <= 0) {
		GET_HIT(ch) = 1;
	}
	if(GET_MANA(ch) <= 0) {
		GET_MANA(ch) = 1;
	}
	if(GET_MOVE(ch) <= 0) {
		GET_MOVE(ch) = 1;
	}

	alter_hit(ch, 0);
	alter_mana(ch, 0);
	alter_move(ch, 0);
}

void reset_char_and_load_objs(struct char_data* ch, bool ghost) {
	if(!ch) {
		return;
	}

	const int hit = GET_HIT(ch);
	const int mana = GET_MANA(ch);
	const int move = GET_MOVE(ch);

	reset_char(ch);
	load_char_objs(ch, ghost);
	affect_total(ch);
	restore_char_points_after_equip(ch, hit, mana, move);
	mudlog(LOG_SAVE, "reset_char_and_load_objs %s: hit %d (max %d, saved %d)",
		   GET_NAME(ch), GET_HIT(ch), GET_MAX_HIT(ch), hit);
}

void reset_char(struct char_data* ch) {
	double ratio = 0.0;
	int i;
	double absmaxhp;
	mudlog(LOG_SAVE, "Resetting char %s", GET_NAME(ch));
	for(i = 0; i < MAX_WEAR; i++) {  /* Initializing */
		ch->equipment[i] = 0;
	}

	ch->player.oggetti = 0;
	ch->followers = 0;
	ch->master = 0;
	ch->carrying = 0;
	ch->next = 0;

	ch->immune = 0;
	ch->M_immune = 0;
	ch->susc = 0;
	ch->mult_att = 1.0;

	if(!GET_RACE(ch)) {
		GET_RACE(ch) = RACE_HUMAN;
	}

	for(i = 0; i < MAX_CLASS; i++) {
		if(GET_LEVEL(ch, i) > IMMENSO) {
			GET_LEVEL(ch, i) = 51;
		}
	}

	SET_BIT(ch->specials.act, PLR_ECHO);

	ch->hunt_dist = 0;
	ch->hatefield = 0;
	ch->fearfield = 0;
	ch->hates.clist = 0;
	ch->fears.clist = 0;

	/* AC adjustment */
	GET_AC(ch) = 100;

	GET_HITROLL(ch) = 0;
	GET_DAMROLL(ch) = 0;
	GET_EQ_SPELLPOWER(ch) = 0;

	ch->next_fighting = 0;
	ch->next_in_room = 0;
	ch->specials.fighting = 0;
    ch->specials.quest_ref = 0;
    ch->specials.eq_val_idx = 0.0;
	ch->specials.PosPrev = POSITION_STANDING;
	for(i = 0; i < MAX_POSITION; i++) {
		ch->specials.TempoPassatoIn[i] = 0;
	}
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
	ch->specials.carry_weight = 0;
	ch->specials.carry_items = 0;
	ch->specials.spellfail = 101;


	/* Achievemets */
	for( i = 0; i < MAX_RACE_ACHIE; i++)
	{
		ch->specials.achievements[RACESLAYER_ACHIE][i] = 0;
	}
	for( i = 0; i < MAX_BOSS_ACHIE; i++)
	{
		ch->specials.achievements[BOSSKILL_ACHIE][i] = 0;
	}
	for( i = 0; i < MAX_CLASS_ACHIE; i++)
	{
		ch->specials.achievements[CLASS_ACHIE][i] = 0;
	}
	for( i = 0; i < MAX_QUEST_ACHIE; i++)
	{
		ch->specials.achievements[QUEST_ACHIE][i] = 0;
		ch->specials.quest_mob[QUEST_ACHIE][i] = 0;
		ch->specials.mercy[i] = 0;
	}
	for( i = 0; i < MAX_OTHER_ACHIE; i++)
	{
		ch->specials.achievements[OTHER_ACHIE][i] = 0;
	}

	if(GET_HIT(ch) <= 0) {
		GET_HIT(ch) = 1;
	} /*** SALVO il regen parte prima di uscire ***/
	if(GET_MOVE(ch) <= 0) {
		GET_MOVE(ch) = 1;
	}
	if(GET_MANA(ch) <= 0) {
		GET_MANA(ch) = 1;
	}
	ch->points.max_mana = 0;
	ch->points.max_move = GET_CON(ch) + number(0, 6) - 3;
	ch->points.mana_gain = 0;
	ch->points.move_gain = 0;
	ch->points.hit_gain = 0;

	if(IS_IMMORTAL(ch)) {
		GET_BANK(ch) = 0;
		GET_GOLD(ch) = 1000000;
	}

	if(GET_BANK(ch) > GetMaxLevel(ch) * 10000) {
		mudlog(LOG_PLAYERS, "%s has %d coins in bank.", GET_NAME(ch),
			   GET_BANK(ch));
	}
	if(GET_GOLD(ch) > GetMaxLevel(ch) * 10000) {
		mudlog(LOG_PLAYERS, "%s has %d coins.", GET_NAME(ch), GET_GOLD(ch));
	}

	/* rimettiamo a posto le condizioni di affamato od assetato in modo che
	 * qualche bug non tolga la necessita di bere o di mangiare al PC >:) */
	if(!IS_IMMORTALE(ch)) {
		if(GET_COND(ch, DRUNK) < 0) {
			GET_COND(ch, DRUNK) = 0;
		}
		if(GET_COND(ch, FULL) < 0) {
			GET_COND(ch, FULL) = 0;
		}
		if(GET_COND(ch, THIRST) < 0) {
			GET_COND(ch, THIRST) = 0;
		}
	}

	/*
	 * Class specific Stuff
	 */

	ClassSpecificStuff(ch);

	if(HasClass(ch, CLASS_MONK)) {
		GET_AC(ch) -= MIN(150, (GET_LEVEL(ch, MONK_LEVEL_IND) * 5));
		ch->points.max_move += GET_LEVEL(ch, MONK_LEVEL_IND);
	}

	/*
	 * racial stuff
	 */
	SetRacialStuff(ch);

	/*
	 * update the affects on the character.
	 */

	ch->specials.sev = LOG_SYSERR | LOG_ERROR | LOG_CONNECT;
#if 0
	for(af = ch->affected; af; af = af->next) {
		affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
	}
#endif
	if(!HasClass(ch, CLASS_MONK)) {
		GET_AC(ch) += dex_app[(int) GET_DEX(ch)].defensive;
	}

	/* could add barbarian double dex bonus here.... ... Nah! */

	if(GET_AC(ch) > 100) {
		GET_AC(ch) = 100;
	}

	/*
	 * clear out the 'dead' bit on characters
	 */
	if(ch->desc) {
		ClearDeadBit(ch);
	}
	/*
	 * Clear out berserk flags case there was a crash in a fight
	 */
	if(IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
		REMOVE_BIT(ch->specials.affected_by2, AFF2_BERSERK);
	}
	/*
	 * Clear out Parry flags case there was a crash in a fight
	 */
	if(IS_SET(ch->specials.affected_by2, AFF2_PARRY)) {
		REMOVE_BIT(ch->specials.affected_by2, AFF2_PARRY);
	}
	/*
	 * Clear out MAILING flags case there was a crash
	 */
	if(IS_SET(ch->specials.act, PLR_MAILING)) {
		REMOVE_BIT(ch->specials.act, PLR_MAILING);
	}

	/*
	 * Clear out objedit flags
	 */
	if(IS_SET(ch->player.user_flags, CAN_OBJ_EDIT)) {
		REMOVE_BIT(ch->player.user_flags, CAN_OBJ_EDIT);
	}
	/*
	 * Clear out group/order/AFK flags
	 */

	REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
	if(IS_SET(ch->specials.affected_by2, AFF2_CON_ORDER)) {
		REMOVE_BIT(ch->specials.affected_by2, AFF2_CON_ORDER);
	}
	if(IS_AFFECTED2(ch, AFF2_AFK)) {
		REMOVE_BIT(ch->specials.affected_by2, AFF2_AFK);
	}

	/*
	 * Remove bogus flags on mortals
	 */

	if(IS_SET(ch->specials.act, PLR_NOHASSLE) && GetMaxLevel(ch) < IMMORTALE) {
		REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
	}

	/* check spells and if lower than 95 remove special flag */
	if(!IS_IMMORTALE(ch)) {
		for(i = 0; i < MAX_SKILLS - 1; i++) {
			if(ch->skills[i].learned < 95||
					!IS_SET(ch->skills[i].flags, SKILL_KNOWN)) {
				ch->skills[i].special = 0;
			}
		}
	}

	SetDefaultLang(ch);

	/* Imposta i livelli di default obbligatori per gli dei */
	//if( !strcmp(GET_NAME(ch),"Isildur"))
	//{
	//GET_LEVEL(ch,0) = 60;
	//}
	//if( !strcmp(GET_NAME(ch),"Flyp"))
	//{
	//GET_LEVEL(ch,0) = 60;
	//}

	if(!strcmp(GET_NAME(ch), "Alar")) {         //Giovanni
		GET_LEVEL(ch, 0) = 60;
	}
	if(!strcmp(GET_NAME(ch), "Isildur")) {      //Nicola
		GET_LEVEL(ch, 0) = 59;
	}
	if(!strcmp(GET_NAME(ch), "Requiem")) {      //Francesco
		GET_LEVEL(ch, 0) = 59;
	}
	if(!strcmp(GET_NAME(ch), "Flyp")) {         //Enrico
		GET_LEVEL(ch, 0) = 59;
	}
	if(!strcmp(GET_NAME(ch), "Nihil")) {        //Marco
		GET_LEVEL(ch, 0) = 58;

        if(PORT == DEVEL_PORT)                  //Marco su DEVEL_PORT
        {
            GET_LEVEL(ch, 0) = 59;
        }
	}
	if(!strcmp(GET_NAME(ch), "LadyOfPain")) {   //Giuseppe
		GET_LEVEL(ch, 0) = 58;

        if(PORT == DEVEL_PORT)                  //Giuseppe su DEVEL_PORT
        {
            GET_LEVEL(ch, 0) = 59;
        }
	}
    if(!strcmp(GET_NAME(ch), "Croneh"))  {       //Corrado
        GET_LEVEL(ch, 0) = 59;
    }

    /* Montero 10-Sep-2018 db.cpp: controllo se il livello del toon è >= 58 */
    if ( GET_LEVEL(ch, 0) >= 58 )
    {
        /* assegno i livelli se >= 58 */
        for(i = 0; i < MAX_CLASS; i++)
        {
            if(GET_LEVEL(ch, i) < GetMaxLevel(ch)) {
                GET_LEVEL(ch, i) = GetMaxLevel(ch);
            }
        }

        /* le classi */
        for(i = 1; i <= CLASS_PSI; i *= 2)
        {
            if(!HasClass(ch, i)) {
                ch->player.iClass += i;
            }
        }

        /* le skill */
        for(i = 0; i <= MAX_SKILLS - 1; i++)
        {
            ch->skills[i].learned = 100;
            ch->skills[i].flags = 1;        // con 0 non vengono mostrate con prac classe, con 1 si
            ch->skills[i].special = 1;
            ch->skills[i].nummem = 0;
        }


    } /* fine Montero 10-Sep-2018 db.cpp */

	/* this is to clear up bogus levels on people that where here before */
	/* these classes where made... */

	if(!HasClass(ch, CLASS_MAGIC_USER)) {
		ch->player.level[0] = 0;
	}
	if(!HasClass(ch, CLASS_CLERIC)) {
		ch->player.level[1] = 0;
	}
	if(!HasClass(ch, CLASS_WARRIOR)) {
		ch->player.level[2] = 0;
	}
	if(!HasClass(ch, CLASS_THIEF)) {
		ch->player.level[3] = 0;
	}
	if(!HasClass(ch, CLASS_WARRIOR)) {
		ch->player.level[2] = 0;
	}
	if(!HasClass(ch, CLASS_DRUID)) {
		ch->player.level[4] = 0;
	}
	if(!HasClass(ch, CLASS_MONK)) {
		ch->player.level[5] = 0;
	}
	if(!HasClass(ch, CLASS_BARBARIAN)) {
		ch->player.level[6] = 0;
	}
	if(!HasClass(ch, CLASS_SORCERER)) {
		ch->player.level[7] = 0;
	}
	if(!HasClass(ch, CLASS_PALADIN)) {
		ch->player.level[8] = 0;
	}
	if(!HasClass(ch, CLASS_RANGER)) {
		ch->player.level[9] = 0;
	}
	if(!HasClass(ch, CLASS_PSI)) {
		ch->player.level[10] = 0;
	}
	for(i = MAX_CLASS; i < ABS_MAX_CLASS; i++) {
		ch->player.level[i] = 0;
	}
	/* Qui viene impostato il flag PKILL
	 * per i personaggi dal 31esimo livello in su e rimosso per gli immortali
	 * */
	if(GetMaxLevel(ch) >= INIZIATO) {
		if(IS_PC(
					ch) && !IS_IMMORTALE(ch) && !IS_SET(ch->player.user_flags, RACE_WAR)) {
			send_to_char(
				"$c0115      ATTENTO! ADESSO SEI PKILL                $c0007\n\r",
				ch);
			SET_BIT(ch->player.user_flags, RACE_WAR);
		}
		if(IS_PC(ch) && IS_IMMORTAL(ch)) {
			REMOVE_BIT(ch->player.user_flags, RACE_WAR);
		}

	}
	/* Controlli per giocatori scorretti */
	if(GetMaxLevel(ch) >= DIO_MINORE) {
		ratio = 0.0;
	}
	else if(GetMaxLevel(ch) >= BARONE) {
		ratio = 0.8;
	}
	else if(GetMaxLevel(ch) >= MAESTRO) {
		ratio = 0.7;
	}
	else if(GetMaxLevel(ch) >= ESPERTO) {
		ratio = 0.6;
	}
	else if(GetMaxLevel(ch) >= INIZIATO) {
		ratio = 0.4;
	}
	else if(GetMaxLevel(ch) >= ALLIEVO) {
		ratio = 0.2;
	}
	else if(GetMaxLevel(ch) >= NOVIZIO) {
		ratio = 0.1;
	}

	{
		if((ch->player.time.played / SECS_PER_REAL_HOUR)
				< (ratio * GetTotLevel(ch))) {
			int minplayed = (ch->player.time.played % SECS_PER_REAL_HOUR) / 60;
			buglog(LOG_PLAYERS,
				   "%s ha fatto %d livelli in %5d ore e %2d minuti",
				   GET_NAME(ch), GetTotLevel(ch),
				   (ch->player.time.played / SECS_PER_REAL_HOUR), minplayed);
		}
	}

	/* Controllo sugli hp, per bug o trucchi*/
	absmaxhp = (float) GetExtimatedHp(ch);
	ratio = (float) GET_MHIT(ch) / absmaxhp;
	if(ratio > 1.0) {
		buglog(LOG_SYSERR, "%s ha %d hp (stimati: %f) ratio %f", GET_NAME(ch),
			   hit_limit(ch), absmaxhp, ratio);
	}

	/* start regening new points */
	alter_hit(ch, 0);
	alter_mana(ch, 0);
	alter_move(ch, 0);

	if(ratio > 1.19) {
		if(GetMaxLevel(ch) > CHUMP) {
			FrozeHim(ch, GET_MHIT(ch));
		}
	}
}

/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data* ch) {
	memset(ch, '\0', sizeof(struct char_data));

	ch->in_room = NOWHERE;
	ch->specials.was_in_room = NOWHERE;
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
	GET_AC(ch) = 100; /* Basic Armor */
	ch->size = 25;
	ch->nMagicNumber = CHAR_VALID_MAGIC;
}

void clear_object(struct obj_data* obj) {
	memset(obj, '\0', sizeof(struct obj_data));

	obj->item_number = -1;
	obj->in_room = NOWHERE;
	obj->eq_pos = -1;
}

/* initialize a new character only if class is set */
void init_char(struct char_data* ch) {
	int i;

	/* *** if this is our first player --- he be God *** */
	mudlog(LOG_CHECK, "Creazione di %s in corso", GET_NAME(ch));

	if((top_of_p_table < 0) || !strcmp(GET_NAME(ch), "Alar")) {

		mudlog(LOG_CHECK, "Building FIRST CHAR, setting up IMPLEMENTOR STATUS!");

		GET_EXP(ch) = 500000000;
		GET_LEVEL(ch, 0) = IMMENSO;
		ch->points.max_hit = 1000;

		/* set all levels */

		for(i = 0; i < MAX_CLASS; i++) {
			if(GET_LEVEL(ch, i) < GetMaxLevel(ch)) {
				GET_LEVEL(ch, i) = GetMaxLevel(ch);
			}
		}/* for */

		/* set all classes */
		for(i = 1; i <= CLASS_PSI; i *= 2) {
			if(!HasClass(ch, i)) {
				ch->player.iClass += i;
			}
		} /* for */

	} /* end implmentor setup */

	set_title(ch);

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;
	ch->player.description = 0;

	ch->player.hometown = number(1, 4);

	ch->player.time.birth = time(0);
	ch->player.time.played = 0;
	ch->player.time.logon = time(0);

	SET_BIT(ch->player.user_flags, USE_PAGING);

	for(i = 0; i < MAX_TOUNGE; i++) {
		ch->player.talks[i] = 0;
	}

	ch->abilities.str = 9;
	ch->abilities.intel = 9;
	ch->abilities.wis = 9;
	ch->abilities.dex = 9;
	ch->abilities.con = 9;
	ch->abilities.chr = 9;
	ch->tmpabilities = ch->abilities;

	/* make favors for sex */
	switch(GET_RACE(ch)) {
	case RACE_HUMAN:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 180);
			ch->player.height = number(160, 200);
		}
		else {
			ch->player.weight = number(100, 160);
			ch->player.height = number(150, 180);
		}
		break;

	case RACE_DWARF:
	case RACE_GNOME:
	case RACE_DARK_DWARF:
	case RACE_DEEP_GNOME:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 180);
			ch->player.height = number(100, 150);
		}
		else {
			ch->player.weight = number(100, 160);
			ch->player.height = number(100, 150);
		}
		break;

	case RACE_HALFLING:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(70, 120);
			ch->player.height = number(80, 120);
		}
		else {
			ch->player.weight = number(60, 110);
			ch->player.height = number(70, 115);
		}
		break;

	case RACE_ELVEN:
	case RACE_DARK_ELF:
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(100, 150);
			ch->player.height = number(160, 200);
		}
		else {
			ch->player.weight = number(80, 230);
			ch->player.height = number(150, 180);
		}
		break;

	case RACE_HALF_ELVEN:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(110, 160);
			ch->player.height = number(140, 180);
		}
		else {
			ch->player.weight = number(90, 150);
			ch->player.height = number(140, 170);
		}
		break;

	case RACE_HALF_OGRE:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(200, 400);
			ch->player.height = number(200, 230);
		}
		else {
			ch->player.weight = number(180, 350);
			ch->player.height = number(190, 220);
		}
		break;

	case RACE_HALF_ORC:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 180);
			ch->player.height = number(160, 200);
		}
		else {
			ch->player.weight = number(100, 160);
			ch->player.height = number(150, 180);
		}
		break;

	case RACE_HALF_GIANT:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(300, 900);
			ch->player.height = number(300, 400);
		}
		else {
			ch->player.weight = number(250, 800);
			ch->player.height = number(290, 350);
		}
		break;

	case RACE_ORC:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(140, 200);
			ch->player.height = number(150, 190);
		}
		else {
			ch->player.weight = number(120, 180);
			ch->player.height = number(140, 170);
		}
		break;

	case RACE_GOBLIN:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(60, 90);
			ch->player.height = number(130, 160);
		}
		else {
			ch->player.weight = number(60, 90);
			ch->player.height = number(120, 150);
		}
		break;

	case RACE_DEMON:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 150);
			ch->player.height = number(150, 190);
		}
		{
			ch->player.weight = number(135, 170);
			ch->player.height = number(160, 200);
		}
		break;

	case RACE_TROLL:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(220, 430);
			ch->player.height = number(210, 250);
		}
		else {
			ch->player.weight = number(200, 400);
			ch->player.height = number(200, 240);
		}
		break;

	default:
		if(ch->player.sex == SEX_MALE) {
			ch->player.weight = number(120, 180);
			ch->player.height = number(160, 200);
		}
		else {
			ch->player.weight = number(100, 160);
			ch->player.height = number(150, 180);
		}
	}

	ch->points.mana = GET_MAX_MANA(ch);
	ch->points.hit = GET_MAX_HIT(ch);
	ch->points.move = GET_MAX_MOVE(ch);

	ch->points.armor = 100;

	if(!ch->skills) {
		SpaceForSkills(ch);
	}

	for(i = 0; i <= MAX_SKILLS - 1; i++) {
		if(GetMaxLevel(ch) < MAESTRO_DEI_CREATORI) {
			ch->skills[i].learned = 0;
			ch->skills[i].flags = 0;
			ch->skills[i].special = 0;
			ch->skills[i].nummem = 0;
		}
		else {
			ch->skills[i].learned = 100;
			ch->skills[i].flags = 0;
			ch->skills[i].special = 1;
			ch->skills[i].nummem = 0;
		}
	}

	ch->specials.affected_by = 0;
	ch->specials.spells_to_learn = 0;

	for(i = 0; i < 5; i++) {
		ch->specials.apply_saving_throw[i] = 0;
	}

	if(GetMaxLevel(ch) > CREATORE) {
		for(i = 0; i < 3; i++) {
			GET_COND(ch, i) = -1;
		}
	}
	else {
		GET_COND(ch, FULL) = 24;
		GET_COND(ch, THIRST) = 24;
		GET_COND(ch, DRUNK) = 0;
	}
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int iVNum) {
	int bot, top, mid;

	bot = 0;
	top = top_of_sort_mobt - 1;

	/* perform binary search on mob-table */
	for(;;) {
		mid = (bot + top) / 2;

		if((mob_index + mid)->iVNum == iVNum) {
			return (mid);
		}
		if(bot >= top) {
			/* start unsorted search now */
			for(mid = top_of_sort_mobt; mid < top_of_mobt; mid++)
				if((mob_index + mid)->iVNum == iVNum) {
					return (mid);
				}
			return (-1);
		}
		if((mob_index + mid)->iVNum > iVNum) {
			top = mid - 1;
		}
		else {
			bot = mid + 1;
		}
	}
	return -1;
}

/* returns the real number of the object with given virtual number */
int real_object(int nVNum) {
	long bot, top, mid;

	bot = 0;
	top = top_of_sort_objt - 1;

	/* perform binary search on obj-table */
	for(;;) {
		mid = (bot + top) / 2;

		if((obj_index + mid)->iVNum == nVNum) {
			return (mid);
		}
		if(bot >= top) {
			/* start unsorted search now */
			for(mid = top_of_sort_objt; mid < top_of_objt; mid++)
				if((obj_index + mid)->iVNum == nVNum) {
					return (mid);
				}
			return (-1);
		}
		if((obj_index + mid)->iVNum > nVNum) {
			top = mid - 1;
		}
		else {
			bot = mid + 1;
		}
	}
	return -1;
}

int ObjRoomCount(int nr, struct room_data* rp) {
	struct obj_data* o;
	int count = 0;

	for(o = rp->contents; o; o = o->next_content) {
		if(o->item_number == nr) {
			count++;
		}
	}
	return (count);
}

int MobRoomCount(int nr, struct room_data* rp) {
	struct char_data* o;
	int count = 0;

	for(o = rp->people; o; o = o->next_in_room) {
		if(o->nr == nr) {
			count++;
		}
	}
	return (count);
}

int str_len(char* buf) {
	int i = 0;
	while(buf[i] != '\0') {
		i++;
	}
	return (i);
}
void reload_files_and_scripts() {

	mudlog(LOG_CHECK, "Rebooting Essential Text Files.");

#if USE_MYSQL
	server_text_reload();
#else
	file_to_string(NEWS_FILE.c_str(), news);
	file_to_string(WIZNEWS_FILE.c_str(), wiznews);
	file_to_string(MOTD_FILE.c_str(), motd);
	file_to_string(WIZ_MOTD_FILE.c_str(), wmotd);
#endif
	file_to_string(CREDITS_FILE.c_str(), credits);
	file_to_string(HELP_PAGE_FILE.c_str(), help);
	mudlog(LOG_CHECK, "Initializing Scripts.");
	InitScripts();

	/* jdb -- you don't appear to re-install the scripts after you
	 * reset the script db
	 */

	for(struct char_data* p = character_list; p; p = p->next) {
		if(!IS_MOB(p)) {
			continue;
		}
		for(int i = 0; i < top_of_scripts; i++) {
			if(gpScript_data[i].iVNum == mob_index[p->nr].iVNum) {
				SET_BIT(p->specials.act, ACT_SCRIPT);
				mudlog(LOG_CHECK, "Setting SCRIPT bit for mobile %s, file %s.",
					   GET_NAME(p), gpScript_data[i].filename);
				p->script = i;
				break;
			}
		}
	}
	return;

}
ACTION_FUNC(reboot_text) {
	reload_files_and_scripts();
}
void InitScripts() {
	char buf[255], buf2[255];
	FILE* f1, *f2;
	int count;
	struct char_data* mob;

	if(!gpScript_data) {
		top_of_scripts = 0;
	}

	/* what is ths for?  turn off all the scripts ??? */
	/* -yes, just in case the script file was removed, saves pointer probs */

	for(mob = character_list; mob; mob = mob->next) {
		if(IS_MOB(mob) && IS_SET(mob->specials.act, ACT_SCRIPT)) {
			mob->commandp = 0;
			REMOVE_BIT(mob->specials.act, ACT_SCRIPT);
		}
	}

	if(!(f1 = fopen("scripts.dat", "r"))) {
		mudlog(LOG_ERROR, "Unable to open file \"scripts.dat\".");
		return;
	}

	if(gpScript_data) {
		for(int i = 0; i < top_of_scripts; i++) {
			free(gpScript_data[i].script);
			free(gpScript_data[i].filename);
		}
		free(gpScript_data);
		top_of_scripts = 0;
	}

	gpScript_data = NULL;
	gpScript_data = (struct scripts*) malloc(sizeof(struct scripts));

	while(1) {
		int i;
		if(fgets(buf, 254, f1) == NULL) {
			break;
		}

		if(buf[strlen(buf) - 1] == '\n') {
			buf[strlen(buf) - 1] = '\0';
		}

		sscanf(buf, "%s %d", buf2, &i);

		std::snprintf(buf, sizeof(buf), "scripts/%.240s", buf2);
		if(!(f2 = fopen(buf, "r"))) {
			mudlog(LOG_ERROR, "Unable to open script \"%s\" for reading.", buf2);
		}
		else {

			gpScript_data = (struct scripts*) realloc(gpScript_data,
							(top_of_scripts + 1) * sizeof(struct scripts));

			count = 0;
			while(!feof(f2)) {
				fgets(buf, 254, f2);
				if(buf[strlen(buf) - 1] == '\n') {
					buf[strlen(buf) - 1] = '\0';
				}
				/* you really don't want to do a lot of reallocs all at once */
				if(count == 0) {
					gpScript_data[top_of_scripts].script =
						(struct foo_data*) malloc(sizeof(struct foo_data));
				}
				else {
					gpScript_data[top_of_scripts].script =
						(struct foo_data*) realloc(
							gpScript_data[top_of_scripts].script,
							sizeof(struct foo_data) * (count + 1));
				}
				gpScript_data[top_of_scripts].script[count].line =
					(char*) malloc(sizeof(char) * (strlen(buf) + 1));

				strcpy(gpScript_data[top_of_scripts].script[count].line, buf);

				count++;
			}

			gpScript_data[top_of_scripts].iVNum = i;
			gpScript_data[top_of_scripts].filename = (char*) malloc(
						(strlen(buf2) + 1) * sizeof(char));
			strcpy(gpScript_data[top_of_scripts].filename, buf2);
			mudlog(LOG_CHECK, "Script %s assigned to mobile %d.", buf2, i);
			top_of_scripts++;
			fclose(f2);
		}
	}

	if(top_of_scripts) {
		mudlog(LOG_CHECK, "%d scripts assigned.", top_of_scripts);
	}
	else {
		mudlog(LOG_CHECK, "No scripts found to assign.");
	}
	fclose(f1);
}

int CheckKillFile(int iVNum) {
	FILE* f1;
	char buf[255];
	int i;

	if(!(f1 = fopen(killfile, "r"))) {
		mudlog(LOG_ERROR, "Unable to find killfile.");
		exit(0);
	}

	while(fgets(buf, 254, f1) != NULL) {
		sscanf(buf, "%d", &i);
		if(i == iVNum) {
			fclose(f1);
			return (1);
		}
	}

	fclose(f1);
	return (0);
}

void ReloadRooms() {
	int i;

	for(i = 0; i < number_of_saved_rooms; i++) {
		load_room_objs(saved_rooms[i]);
	}
}

void SaveTheWorld() {
#if SAVEWORLD

	static int ctl = 0;
	char cmd, buf[80];
	int i, j, arg1, arg2, arg3;
	struct char_data* p;
	struct obj_data* o;
	struct room_data* room;
	FILE* fp;

	if(ctl == WORLD_SIZE) {
		ctl = 0;
	}

	sprintf(buf, "world/mobs.%d", ctl);
	fp = (FILE*) fopen(buf, "w"); /* append */

	if(!fp) {
		mudlog(LOG_ERROR, "Unable to open zone writing file.");
		return;
	}

	i = ctl;
	ctl += 1000;

	for(; i < ctl; i++) {
		room = real_roomp(i);
		if(room && !IS_SET(room->room_flags, DEATH)) {
			/*
			 *  first write out monsters
			 */
			for(p = room->people; p; p = p->next_in_room) {
				if(IS_MOB(p)) {
					cmd = 'M';
					arg1 = MobVnum(p);
					arg2 = mob_index[p->nr].number;
					arg3 = i;
					Zwrite(fp, cmd, 0, arg1, arg2, arg3, 0, p->player.short_descr);
					fprintf(fp, "Z 1 %d 1\n", p->specials.zone);

					/* save hatreds && fears */
					if(IS_SET(p->hatefield, HATE_SEX)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_SEX, p->hates.sex);
					}
					if(IS_SET(p->hatefield, HATE_RACE)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_RACE, p->hates.race);
					}
					if(IS_SET(p->hatefield, HATE_GOOD)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_GOOD, p->hates.good);
					}
					if(IS_SET(p->hatefield, HATE_EVIL)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_EVIL, p->hates.evil);
					}
					if(IS_SET(p->hatefield, HATE_CLASS)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_CLASS, p->hates.iClass);
					}
					if(IS_SET(p->hatefield, HATE_VNUM)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_VNUM, p->hates.vnum);
					}

					if(IS_SET(p->fearfield, FEAR_SEX)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_SEX, p->fears.sex);
					}
					if(IS_SET(p->fearfield, FEAR_RACE)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_RACE, p->fears.race);
					}
					if(IS_SET(p->fearfield, FEAR_GOOD)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_GOOD, p->fears.good);
					}
					if(IS_SET(p->fearfield, FEAR_EVIL)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_EVIL, p->fears.evil);
					}
					if(IS_SET(p->fearfield, FEAR_CLASS)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_CLASS, p->fears.iClass);
					}
					if(IS_SET(p->fearfield, FEAR_VNUM)) {
						fprintf(fp, "H 1 %d %d -1\n", OP_VNUM, p->fears.vnum);
					}

					for(j = 0; j < MAX_WEAR; j++) {
						if(p->equipment[j]) {
							if(p->equipment[j]->item_number >= 0) {
								cmd = 'E';
								arg1 = ObjVnum(p->equipment[j]);
								arg2 = obj_index[p->equipment[j]->item_number].number;
								arg3 = j;
								strcpy(buf, p->equipment[j]->short_description);
								Zwrite(fp, cmd, 1, arg1, arg2, arg3, 0, buf);
								RecZwriteObj(fp, p->equipment[j]);
							}
						}
					}
					for(o = p->carrying; o; o = o->next_content) {
						if(o->item_number >= 0) {
							cmd = 'G';
							arg1 = ObjVnum(o);
							arg2 = obj_index[o->item_number].number;
							arg3 = 0;
							strcpy(buf, o->short_description);
							Zwrite(fp, cmd, 1, arg1, arg2, arg3, 0, buf);
							RecZwriteObj(fp, o);
						}
					}
				}
			}
		}
	}
	fprintf(fp, "S\n");
	fclose(fp);

#endif
}

int ENomeValido(char* pchNome) {
	if(pchNome) {
		while(strlen(pchNome) && !isalpha(pchNome[strlen(pchNome) - 1])) {
			pchNome[strlen(pchNome) - 1] = 0;
		}

		if(strlen(pchNome)) {
			while(pchNome) {
				if(!isalpha(*pchNome)) {
					return FALSE;
				}
				pchNome++;
			}
			return TRUE;
		}
	}
	return FALSE;
}
void clean_playerfile() {

	struct junk {
		struct char_file_u dummy;
		bool AXE;
	};
	struct junk grunt;
	struct dirent* ent;

	time_t timeH;
	int j, max, num_warned, num_processed, num_deleted, num_demoted, ones;
	long age;
	long life;
	DIR* dir;
	long lSize;

	num_warned = num_processed = num_deleted = num_demoted = ones = 0;
	timeH = time(0);

	mudlog(LOG_SYSERR, "time now %i", timeH);

	if((dir = opendir(PLAYERS_DIR)) != NULL) {
		while((ent = readdir(dir)) != NULL) {
			FILE* pFile;
			char szFileName[300];

			if(*ent->d_name == '.') {
				continue;
			}

			snprintf(szFileName, sizeof(szFileName)-1, "%s/%s", PLAYERS_DIR, ent->d_name);

			if(strstr(ent->d_name, ".dat")) {
				if((pFile = fopen(szFileName, "r+")) != NULL) {
					grunt.AXE = FALSE;

					fseek(pFile, 0, SEEK_END);
					lSize = ftell(pFile);
					rewind(pFile);

					mudlog(LOG_SYSERR, "%i -- %i", sizeof(grunt.dummy), lSize);
					if(fread(&grunt.dummy, 1, sizeof(grunt.dummy), pFile) != sizeof(grunt.dummy)) {
						mudlog(LOG_SYSERR, "Nome: %s", grunt.dummy.name);
						num_processed++;

						/* Fa la lista dei personaggi attivi.. a bit tedious */
						const char* classname[] = { "Mu", "Cl", "Wa", "Th",
													"Dr", "Mo", "Ba", "So", "Pa", "Ra", "Ps", "?",
													"??"
												  };
						char classes[100];
						classes[0] = '\0';
						int i;

						for(i = max = 0; i < MAX_CLASS; i++) {
							/*calcola il livello piu' alto in max*/
							if(grunt.dummy.level[i] > max) {
								max = grunt.dummy.level[i];
							}

							if(grunt.dummy.level[i] > 0) {
								if(strlen(classes) != 0) {
									strcat(classes, "/");
								}

								sprintf(classes + strlen(classes), "%s",
										classname[i]);
							}
						}

						if(grunt.dummy.points.max_hit > 500) {
							buglog(LOG_PLAYERS,
								   "%s (%s) ha piu' di 500 hp (%d)",
								   grunt.dummy.name, classes,
								   grunt.dummy.points.max_hit);
						}

						if(max < IMMORTALE) {
							j = (int) max;
							if(j < 5) {
								j = 5;
							}

							age = timeH - grunt.dummy.last_logon;

							mudlog(LOG_SYSERR,
								   "*****%s****Last logon: %i***age-->%i",
								   grunt.dummy.name, grunt.dummy.last_logon,
								   age);
							mudlog(LOG_SYSERR, "*****%s",
								   grunt.dummy.description);

							/* BUG BUG BUG */
							/* Cancella il player dopo j settimane
							 mudlog(LOG_PLAYERS, "%---Trascorso da Last Log %i---Time to delete %i", age, (long) j * (SECS_PER_REAL_DAY * 7));
							 if (!grunt.AXE && age > (long) j * (SECS_PER_REAL_DAY * 7) && !IS_SET(grunt.dummy.user_flags, NO_DELETE)) {
							 num_deleted++;
							 regdelete(grunt.dummy.name);
							 grunt.AXE = TRUE;
							 mudlog(LOG_PLAYERS, "%s marked for deletion after more than %d weeks of inactivity.", grunt.dummy.name, j);
							 } */

							/* even the no_deletes get deleted after a time
							 if (IS_SET(grunt.dummy.user_flags, NO_DELETE)) {
							 j *= 3;
							 if (!grunt.AXE && age > (long) (j) * (SECS_PER_REAL_DAY * 7)) {
							 num_deleted++;
							 // regdelete(lower(grunt.dummy.name));
							 grunt.AXE = TRUE;
							 mudlog(LOG_PLAYERS, "%s marked for deletion after %d weeks of inactivity (NO_DELETE).", grunt.dummy.name, j);
							 }
							 } */

							/* Avviso di cancellazione imminente*/

							if(!grunt.AXE
									&& age
									> (long)(j - 1)
									* (SECS_PER_REAL_DAY * 7)&& !IS_SET(grunt.dummy.user_flags, NO_DELETE)) {
								num_warned++;
								life = (long)(j * 7)
									 - (age / SECS_PER_REAL_DAY);
								if(life < 2) {
									mudlog(LOG_PLAYERS,
										   "XXX %s to be deleted in %d day",
										   grunt.dummy.name, (int) life);
								}
							}
						}
						else if(max >= IMMORTALE) {
							if(max > IMMENSO) {
								num_deleted++;
								grunt.AXE = TRUE;
								mudlog(LOG_PLAYERS,
									   "%s marked for deletion (TOOHIGHLEVEL).",
									   grunt.dummy.name);
							}
						}
						else {
							mudlog(LOG_ERROR, "Error %s reading file %s.",
								   strerror(errno), szFileName);
						}

						fclose(pFile);

						if(grunt.AXE) {
							mudlog(LOG_CHECK, "%s doveva essere cancellato",
								   grunt.dummy.name);
						}
					}
					else {
						mudlog(LOG_ERROR, "Error opening file %s.", szFileName);
					}
				}
			}
		} /* while */
	}
	else {
		mudlog(LOG_ERROR, "Error opening dir %s.", PLAYERS_DIR);
	}
	mudlog(LOG_CHECK, "-- %d characters processed.", num_processed);
	mudlog(LOG_CHECK, "-- %d characters warned.", num_warned);
	mudlog(LOG_CHECK, "-- %d characters deleted.  ", num_deleted);
	mudlog(LOG_CHECK, "-- %d gods demoted due to inactivity.", num_demoted);
	mudlog(LOG_CHECK, "Cleaning done.");
}

#if ENABLE_AUCTION

void Start_Auction() {
	auction = (AUCTION_DATA*) calloc(1, sizeof(AUCTION_DATA)); /* DOH!!! */
	if(auction == NULL) {
		mudlog(LOG_ERROR, "malloc'ing AUCTION_DATA didn't give %d bytes",
			   sizeof(AUCTION_DATA));
		exit(1);
	}

	auction->item = NULL; /* nothing is being sold */
}
#endif

ACTION_FUNC(do_WorldSave) {
	if(!ch->desc) {
		return;
	}

	send_to_char("Comando disabilitato\r\n", ch);
	return;
}

/* Mob related handy functions */

    int NewMobMov (struct char_data* mob) {
        int extra_mov = 0;

    /* Nuova assegnazione punti movimento mob */
        if(GET_LEVEL(mob, WARRIOR_LEVEL_IND) > ALLIEVO && GET_LEVEL(mob, WARRIOR_LEVEL_IND) < INIZIATO )
        {
            extra_mov += GET_LEVEL(mob, WARRIOR_LEVEL_IND);
        }
        else if(GET_LEVEL(mob, WARRIOR_LEVEL_IND) >= INIZIATO && GET_LEVEL(mob, WARRIOR_LEVEL_IND) < MAESTRO)
        {
            extra_mov += (50 + GET_LEVEL(mob, WARRIOR_LEVEL_IND));
        }
        else if(GET_LEVEL(mob, WARRIOR_LEVEL_IND) >= MAESTRO && GET_LEVEL(mob, WARRIOR_LEVEL_IND) < PRINCIPE)
        {
            extra_mov += (100 + GET_LEVEL(mob, WARRIOR_LEVEL_IND));
        }
        else if(GET_LEVEL(mob, WARRIOR_LEVEL_IND) >= PRINCIPE)
        {
            extra_mov += (250 + GET_LEVEL(mob, WARRIOR_LEVEL_IND));
        }

    return(mob->points.max_move+extra_mov);

    }

#if DEATH_FIX

static void death_snapshot_write_file(const char* name, long saved_exp, long saved_at_epoch) {
	char nomefile[256];
	std::snprintf(nomefile, sizeof(nomefile), "%s/%s.dead", PLAYERS_DIR, lower(name));
	mudlog(LOG_PLAYERS, "death_snapshot: opening %s", nomefile);
	FILE* fdeath = std::fopen(nomefile, "w+");
	if(!fdeath) {
		mudlog(LOG_PLAYERS, "death_snapshot: cannot save xp for %s", name);
		return;
	}
	std::fprintf(fdeath, "%ld : %ld", saved_exp, saved_at_epoch);
	std::fclose(fdeath);
	mudlog(LOG_PLAYERS, "death_snapshot: file %s exp=%ld at=%ld", name, saved_exp, saved_at_epoch);
}

void death_snapshot_save(const char* name, long saved_exp, long saved_at_epoch) {
	if(!name || !*name) {
		return;
	}
	death_snapshot_write_file(name, saved_exp, saved_at_epoch);

#if USE_MYSQL
	try {
		toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
		if(!pg || !pg->id) {
			return;
		}
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string toon_id = std::to_string(pg->id);
		const std::string sql =
			"INSERT INTO character_death_snapshot (toon_id, saved_exp, saved_at) VALUES (" +
			toon_id + ',' + std::to_string(saved_exp) + ',' +
			std::to_string(static_cast<unsigned long>(saved_at_epoch)) +
			") ON DUPLICATE KEY UPDATE saved_exp = VALUES(saved_exp), saved_at = VALUES(saved_at)";
		db->execute(sql.c_str());
		t.commit();
		mudlog(LOG_SAVE, "death_snapshot: DB for %s exp=%ld at=%ld", name, saved_exp,
			   saved_at_epoch);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "death_snapshot_save DB(%s): %s", name, e.what());
	}
#endif
}

static bool death_snapshot_read_file(const char* name, long& saved_exp, long& saved_at_epoch) {
	char nomefile[256];
	std::snprintf(nomefile, sizeof(nomefile), "%s/%s.dead", PLAYERS_DIR, lower(name));
	FILE* fdeath = std::fopen(nomefile, "r");
	if(!fdeath) {
		return false;
	}
	long xp = 0;
	long ora = 0;
	if(std::fscanf(fdeath, "%ld : %ld", &xp, &ora) != 2) {
		std::fclose(fdeath);
		return false;
	}
	std::fclose(fdeath);
	saved_exp = xp;
	saved_at_epoch = ora;
	return true;
}

bool death_snapshot_load(const char* name, long& saved_exp, long& saved_at_epoch) {
	if(!name || !*name) {
		return false;
	}

#if USE_MYSQL
	if(toon_is_migrated_by_name(name)) {
		try {
			toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
			if(pg && pg->id) {
				DB* db = Sql::getMysql();
				const std::string sql =
					"SELECT saved_exp, saved_at FROM character_death_snapshot WHERE toon_id = " +
					std::to_string(pg->id) + " LIMIT 1";
				MYSQL_RES* res = nullptr;
				if(mysql_query_select(db, sql, res) && res) {
					MYSQL_ROW row = mysql_fetch_row(res);
					if(row && row[0] && row[1]) {
						saved_exp = std::atol(row[0]);
						saved_at_epoch = std::atol(row[1]);
						mysql_free_result(res);
						mudlog(LOG_PLAYERS, "death_snapshot_load: DB %s exp=%ld", name, saved_exp);
						return true;
					}
					mysql_free_result(res);
				}
			}
		}
		catch(const odb::exception& e) {
			mudlog(LOG_SYSERR, "death_snapshot_load DB(%s): %s", name, e.what());
		}
	}
#endif

	if(death_snapshot_read_file(name, saved_exp, saved_at_epoch)) {
		mudlog(LOG_PLAYERS, "death_snapshot_load: file %s exp=%ld", name, saved_exp);
		return true;
	}
	return false;
}

bool death_snapshot_sync_exp_mysql(const char* name, long saved_exp) {
#if !USE_MYSQL
	(void)name;
	(void)saved_exp;
	return false;
#else
	if(!name || !*name || !toon_is_migrated_by_name(name)) {
		return false;
	}
	try {
		toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(name));
		if(!pg || !pg->id) {
			return false;
		}
		DB* db = Sql::getMysql();
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string sql = "UPDATE character_stats SET exp = " +
								std::to_string(saved_exp) + " WHERE toon_id = " +
								std::to_string(pg->id);
		db->execute(sql.c_str());
		t.commit();
		mudlog(LOG_SAVE, "death_snapshot_sync_exp_mysql: %s exp=%ld", name, saved_exp);
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "death_snapshot_sync_exp_mysql(%s): %s", name, e.what());
		return false;
	}
#endif
}

#endif /* DEATH_FIX */

}
