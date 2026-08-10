/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "clan_symbol.hpp"

#if USE_MYSQL

#include "logging.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "autoenums.hpp"
#include "db.hpp"
#include "comm.hpp"
#include "handler.hpp"
#include "Sql.hpp"
#include "odb/account-odb.hxx"
#include "legacy_import.hpp"
#include "legacy_loader.hpp"
#include "object_instance.hpp"
#include "toon_migration.hpp"

#include <odb/mysql/database.hxx>
#include <mysql/mysql.h>

#include <cctype>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <strings.h>
#include <sys/stat.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "interpreter.hpp"

namespace Alarmud {

namespace {

struct ClanSymbolEntry {
	unsigned vnum;
	const char* prince_name;
};

/*
 * Seed iniziale (una tantum se riga assente in clan_symbol).
 * Specchio di tools/clan_symbol_vnums.txt — dopo il seed la verita' e' MySQL.
 */
constexpr ClanSymbolEntry kClanSymbols[] = {
	{34002, "Montero"},
	{34006, "Fouler"},
	{34007, "Myst"},
	{34081, "Armageddon"},
	{34164, "Omega"},
	{34168, "Kamui"},
	{34171, "Aresius"},
	{34173, "Vilyasilma"},
	{34191, "Quigonstim"},
	{34193, "Quigonmeth"},
	{34291, "Drfeelgood"},
	{34307, "Acidus"},
	{34309, "Caos"},
	{34311, "Kyoko"},
	{34359, "Astaroth"},
	{34375, "Martinus"},
	{34534, "Ogun"},
	{34649, "Fratello"},
	{34694, "Chunli"},
	{34696, "Greenblade"},
};

constexpr unsigned kClanSymbolWearFlags =
	static_cast<unsigned>(ITEM_TAKE) | static_cast<unsigned>(ITEM_WEAR_CLAN_SYMBOL);

constexpr int kClanSymbolWearPos = static_cast<int>(WEAR_CLAN_SYMBOL) + 1;

constexpr const char* kSystemActor = "clan_symbol_boot";

bool objects_file_is_regular(int vnum) {
	char path[256];
	struct stat st;
	snprintf(path, sizeof(path), "%s/%d", OBJ_DIR, vnum);
	return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

bool sql_escape(MYSQL* h, const std::string& in, std::string& out) {
	out.resize(in.size() * 2 + 1);
	const unsigned long n = mysql_real_escape_string(
		h, &out[0], in.c_str(), static_cast<unsigned long>(in.size()));
	out.resize(n);
	return true;
}

constexpr unsigned kDefaultClanSymbolSlots = 5;
constexpr const char* kClanAssegnaActor = "clan_assegna";

bool ensure_clan_symbol_table(DB* db) {
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		db->execute(
			"CREATE TABLE IF NOT EXISTS `clan_symbol` ("
			"  `vnum` INT UNSIGNED NOT NULL,"
			"  `base_vnum` INT UNSIGNED NULL,"
			"  `prince_name` VARCHAR(64) NOT NULL,"
			"  `prince_toon_id` BIGINT UNSIGNED NULL,"
			"  `instance_id` BIGINT UNSIGNED NULL,"
			"  `slots_max` TINYINT UNSIGNED NOT NULL DEFAULT 5,"
			"  `active` TINYINT UNSIGNED NOT NULL DEFAULT 1,"
			"  `updated_at` DATETIME NOT NULL,"
			"  PRIMARY KEY (`vnum`),"
			"  KEY `idx_clan_symbol_prince` (`prince_toon_id`),"
			"  KEY `idx_clan_symbol_instance` (`instance_id`)"
			") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol: CREATE TABLE: %s", e.what());
		return false;
	}
	/* Migrazione tabelle create prima di slots_max. */
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		db->execute(
			"ALTER TABLE `clan_symbol` "
			"ADD COLUMN `slots_max` TINYINT UNSIGNED NOT NULL DEFAULT 5");
		t.commit();
	}
	catch(const odb::exception&) {
		/* colonna gia' presente */
	}
	return true;
}

unsigned long long lookup_toon_id_ci(DB* db, const char* name) {
	if(!db || !name || !*name) {
		return 0;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::string esc;
		sql_escape(h, name, esc);
		std::ostringstream sql;
		sql << "SELECT id FROM toon WHERE LOWER(name)=LOWER('" << esc << "') LIMIT 1";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "clan_symbol: lookup %s: %s", name, mysql_error(h));
			return 0;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return 0;
		}
		unsigned long long id = 0;
		if(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(row[0]) {
				id = strtoull(row[0], nullptr, 10);
			}
		}
		mysql_free_result(res);
		return id;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol: lookup %s: %s", name, e.what());
		return 0;
	}
}

std::string lookup_toon_name_by_id(DB* db, unsigned long long id) {
	if(!db || id == 0) {
		return {};
	}
	try {
		toon pg;
		if(db->query_one<toon>(odb::query<toon>::id == id, pg)) {
			return pg.name;
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol: name for id %llu: %s",
			   static_cast<unsigned long long>(id), e.what());
	}
	return {};
}

void apply_fields(struct obj_data* obj, int prince_id) {
	if(!obj) {
		return;
	}
	obj->obj_flags.type_flag = ITEM_CLAN_SYMBOL;
	obj->obj_flags.wear_flags = kClanSymbolWearFlags;
	obj->obj_flags.value[0] = prince_id;
}

bool seed_row(DB* db, unsigned vnum, const char* prince) {
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::string esc;
		sql_escape(h, prince, esc);
		std::ostringstream sql;
		/* Non sovrascrive prince_name gia' presente. */
		sql << "INSERT INTO clan_symbol (vnum, prince_name, active, updated_at) "
			   "VALUES ("
			<< vnum << ",'" << esc << "',1,NOW()) "
			<< "ON DUPLICATE KEY UPDATE vnum=vnum";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "clan_symbol seed %u: %s", vnum, mysql_error(h));
			t.rollback();
			return false;
		}
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol seed %u: %s", vnum, e.what());
		return false;
	}
}

void update_registry_row(DB* db, unsigned vnum, unsigned base_vnum,
						 unsigned long long prince_id, const char* prince,
						 unsigned long long instance_id) {
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::string esc;
		sql_escape(h, prince ? prince : "", esc);
		std::ostringstream sql;
		sql << "UPDATE clan_symbol SET base_vnum=" << base_vnum
			<< ", prince_toon_id=" << prince_id << ", prince_name='" << esc << "'"
			<< ", instance_id=" << instance_id << ", active=1, updated_at=NOW() "
			<< "WHERE vnum=" << vnum;
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "clan_symbol update %u: %s", vnum, mysql_error(h));
			t.rollback();
			return;
		}
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol update %u: %s", vnum, e.what());
	}
}

bool relink_inventory(DB* db, unsigned edit_vnum, unsigned base_vnum,
					  unsigned long long instance_id, int prince_id) {
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		std::ostringstream sql;
		sql << "UPDATE character_inventory SET item_number=" << base_vnum
			<< ", instance_id=" << instance_id << ", value0=" << prince_id
			<< ", wear_pos=IF(wear_pos>0," << kClanSymbolWearPos << ",0) "
			<< "WHERE item_number=" << edit_vnum
			<< " AND (deleted=0 OR deleted IS NULL)";
		db->execute(sql.str().c_str());
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol relink %u: %s", edit_vnum, e.what());
		return false;
	}
}

long count_inv_item_number(DB* db, unsigned edit_vnum) {
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT COUNT(*) FROM character_inventory WHERE item_number="
			<< edit_vnum << " AND (deleted=0 OR deleted IS NULL)";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			return -1;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return -1;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		const long n = (row && row[0]) ? strtol(row[0], nullptr, 10) : 0;
		mysql_free_result(res);
		return n;
	}
	catch(const odb::exception&) {
		return -1;
	}
}

void collect_mysql_holders(DB* db, unsigned edit_vnum,
						   std::unordered_set<std::string>& out) {
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT DISTINCT t.name FROM character_inventory ci "
			   "INNER JOIN toon t ON t.id = ci.toon_id "
			   "WHERE ci.item_number = "
			<< edit_vnum
			<< " AND (ci.deleted = 0 OR ci.deleted IS NULL)";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "clan_symbol holders: %s", mysql_error(h));
			return;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return;
		}
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(row[0] && *row[0]) {
				out.insert(row[0]);
			}
		}
		mysql_free_result(res);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol holders: %s", e.what());
	}
}

void index_rent_holders(
	std::unordered_map<unsigned, std::unordered_set<std::string>>& out) {
	DIR* dir = opendir(RENT_DIR);
	if(!dir) {
		return;
	}
	struct dirent* ent;
	while((ent = readdir(dir)) != nullptr) {
		if(*ent->d_name == '.' || strstr(ent->d_name, ".aux") != nullptr) {
			continue;
		}
		bool ok_name = true;
		for(const char* p = ent->d_name; *p; ++p) {
			const unsigned char c = static_cast<unsigned char>(*p);
			if(!(std::isalnum(c) || c == '_' || c == '-')) {
				ok_name = false;
				break;
			}
		}
		if(!ok_name) {
			continue;
		}
		char path[512];
		snprintf(path, sizeof(path), "%s/%s", RENT_DIR, ent->d_name);
		struct stat stbuf;
		if(stat(path, &stbuf) != 0 || !S_ISREG(stbuf.st_mode)) {
			continue;
		}
		obj_file_u st {};
		if(!legacy_load_rent_file_path(path, st)) {
			continue;
		}
		const std::string pname = ent->d_name;
		for(int i = 0; i < st.number && i < MAX_OBJ_SAVE; ++i) {
			const unsigned v = st.objects[i].item_number;
			if(clan_symbol_is_listed_vnum(v)) {
				out[v].insert(pname);
			}
		}
	}
	closedir(dir);
}

bool rewrite_rent_file(const std::string& name, unsigned edit_vnum,
					   unsigned base_vnum) {
	obj_file_u st {};
	if(!legacy_load_rent_file(name.c_str(), st)) {
		return false;
	}
	bool changed = false;
	for(int i = 0; i < st.number && i < MAX_OBJ_SAVE; ++i) {
		if(st.objects[i].item_number == static_cast<ush_int>(edit_vnum)) {
			st.objects[i].item_number = static_cast<ush_int>(base_vnum);
			if(st.objects[i].wearpos > 0) {
				st.objects[i].wearpos = static_cast<ubyte>(kClanSymbolWearPos);
			}
			changed = true;
		}
	}
	if(!changed) {
		return true;
	}
	char path[512];
	snprintf(path, sizeof(path), "%s/%s", RENT_DIR, name.c_str());
	FILE* f = fopen(path, "w+b");
	if(!f) {
		mudlog(LOG_SYSERR, "clan_symbol: cannot rewrite rent %s", path);
		return false;
	}
	const size_t n = fwrite(&st, sizeof(st), 1, f);
	fclose(f);
	return n == 1;
}

void patch_online_by_instance(unsigned long long instance_id, int prince_id) {
	if(instance_id == 0) {
		return;
	}
	for(struct obj_data* obj = object_list; obj; obj = obj->next) {
		if(obj->db_instance_id != instance_id) {
			continue;
		}
		apply_fields(obj, prince_id);
	}
}

} // namespace

bool clan_symbol_is_listed_vnum(unsigned vnum) {
	for(const ClanSymbolEntry& e : kClanSymbols) {
		if(e.vnum == vnum) {
			return true;
		}
	}
	return false;
}

bool clan_symbol_can_wear(struct char_data* ch, const struct obj_data* obj) {
	if(!ch || !obj) {
		return false;
	}
	if(obj->obj_flags.type_flag != ITEM_CLAN_SYMBOL &&
	   !IS_SET(obj->obj_flags.wear_flags, ITEM_WEAR_CLAN_SYMBOL)) {
		return true;
	}
	if(IS_IMMORTAL(ch)) {
		return true;
	}
	const int prince_id = obj->obj_flags.value[0];
	if(prince_id <= 0) {
		send_to_char("Questo simbolo non e' ancora associato a una casata.\n\r",
					 ch);
		return false;
	}

	DB* db = Sql::getMysql();
	std::string pname;
	if(db) {
		try {
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			pname = lookup_toon_name_by_id(db, static_cast<unsigned long long>(prince_id));
			t.commit();
		}
		catch(const odb::exception& e) {
			mudlog(LOG_SYSERR, "clan_symbol_can_wear: %s", e.what());
		}
	}
	if(pname.empty()) {
		send_to_char("Casata del simbolo non trovata.\n\r", ch);
		return false;
	}
	if(GET_NAME(ch) && strcasecmp(GET_NAME(ch), pname.c_str()) == 0) {
		return true;
	}
	if(HAS_PRINCE(ch) && GET_PRINCE(ch) &&
	   strcasecmp(GET_PRINCE(ch), pname.c_str()) == 0) {
		return true;
	}
	send_to_char("Solo i membri di quella casata possono indossarlo.\n\r", ch);
	return false;
}

void clan_symbol_boot_migrate() {
	DB* db = Sql::getMysql();
	if(!db) {
		mudlog(LOG_CHECK, "clan_symbol_boot_migrate: no MySQL, skip");
		return;
	}
	if(!ensure_clan_symbol_table(db)) {
		return;
	}

	std::unordered_map<unsigned, std::unordered_set<std::string>> rent_holders;
	index_rent_holders(rent_holders);

	int converted = 0;
	int skipped = 0;
	int missing_prince = 0;
	int missing_obj = 0;
	int bad_base = 0;
	int archived = 0;
	int deferred = 0;
	int failed = 0;

	for(const ClanSymbolEntry& entry : kClanSymbols) {
		seed_row(db, entry.vnum, entry.prince_name);

		unsigned long long prince_id = 0;
		try {
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			prince_id = lookup_toon_id_ci(db, entry.prince_name);
			t.commit();
		}
		catch(const odb::exception& e) {
			mudlog(LOG_SYSERR, "clan_symbol: toon lookup %s: %s", entry.prince_name,
				   e.what());
		}
		if(prince_id == 0) {
			mudlog(LOG_CHECK,
				   "clan_symbol_boot_migrate: skip %u (prince '%s' not in toon)",
				   entry.vnum, entry.prince_name);
			++missing_prince;
			continue;
		}
		const int prince_id_i = static_cast<int>(prince_id);

		unsigned existing_base = 0;
		unsigned long long existing_id =
			object_instance_find_by_legacy_edit(entry.vnum, &existing_base);
		const int rnum = real_object(static_cast<int>(entry.vnum));
		const bool file_gone =
			(rnum < 0) || !objects_file_is_regular(static_cast<int>(entry.vnum));

		/* Gia' migrato e file 34k assente: sync registry / inventori. */
		if(existing_id != 0 && file_gone) {
			update_registry_row(db, entry.vnum, existing_base, prince_id,
								entry.prince_name, existing_id);
			relink_inventory(db, entry.vnum, existing_base, existing_id, prince_id_i);
			patch_online_by_instance(existing_id, prince_id_i);
			++skipped;
			continue;
		}

		if(rnum < 0) {
			mudlog(LOG_CHECK, "clan_symbol_boot_migrate: skip %u (no object index)",
				   entry.vnum);
			++missing_obj;
			continue;
		}

		struct obj_data* obj = read_object(static_cast<int>(entry.vnum), VIRTUAL);
		if(!obj) {
			mudlog(LOG_SYSERR, "clan_symbol_boot_migrate: read_object %u failed",
				   entry.vnum);
			++failed;
			continue;
		}

		const int base_vnum = obj->char_vnum;
		if(base_vnum <= 0 ||
		   (base_vnum >= LOW_EDITED_ITEMS && base_vnum <= HIGH_EDITED_ITEMS) ||
		   real_object(base_vnum) < 0) {
			mudlog(LOG_CHECK,
				   "clan_symbol_boot_migrate: skip %u (header/base %d not OK_HEADER)",
				   entry.vnum, base_vnum);
			extract_obj(obj);
			++bad_base;
			continue;
		}

		apply_fields(obj, prince_id_i);
		strncpy(obj->personal_owner, entry.prince_name, sizeof(obj->personal_owner) - 1);
		obj->personal_owner[sizeof(obj->personal_owner) - 1] = '\0';

		unsigned long long instance_id = existing_id;
		if(instance_id == 0) {
			instance_id = object_instance_persist(obj, base_vnum, 0, nullptr, true,
												  kSystemActor);
		}
		else {
			instance_id =
				object_instance_persist(obj, base_vnum, instance_id, nullptr, true,
										kSystemActor);
		}
		if(instance_id == 0) {
			mudlog(LOG_SYSERR, "clan_symbol_boot_migrate: persist failed for %u",
				   entry.vnum);
			extract_obj(obj);
			++failed;
			continue;
		}

		update_registry_row(db, entry.vnum, static_cast<unsigned>(base_vnum), prince_id,
							entry.prince_name, instance_id);

		std::unordered_set<std::string> holders = rent_holders[entry.vnum];
		collect_mysql_holders(db, entry.vnum, holders);

		bool can_archive = true;
		for(const std::string& name : holders) {
			if(toon_is_migrated_by_name(name.c_str())) {
				continue;
			}
			LegacyImportReport rep;
			if(!legacy_import_character_mysql(name.c_str(), rep)) {
				mudlog(LOG_ERROR,
					   "clan_symbol_boot_migrate: legacyimport %s for %u failed: %s",
					   name.c_str(), entry.vnum, rep.message.c_str());
				can_archive = false;
			}
			else {
				mudlog(LOG_CHECK,
					   "clan_symbol_boot_migrate: legacyimport %s for %u OK (%s)",
					   name.c_str(), entry.vnum, rep.message.c_str());
			}
		}

		if(!relink_inventory(db, entry.vnum, static_cast<unsigned>(base_vnum),
							 instance_id, prince_id_i)) {
			can_archive = false;
		}

		for(const std::string& name : rent_holders[entry.vnum]) {
			if(!toon_is_migrated_by_name(name.c_str())) {
				can_archive = false;
				continue;
			}
			if(!rewrite_rent_file(name, entry.vnum, static_cast<unsigned>(base_vnum))) {
				can_archive = false;
			}
		}

		const long left = count_inv_item_number(db, entry.vnum);
		if(left < 0 || left > 0) {
			if(left > 0) {
				mudlog(LOG_CHECK,
					   "clan_symbol_boot_migrate: %u still has %ld inventory rows",
					   entry.vnum, left);
			}
			can_archive = false;
		}

		extract_obj(obj);
		obj = nullptr;
		patch_online_by_instance(instance_id, prince_id_i);

		if(can_archive) {
			std::string err;
			if(!archive_object_file(static_cast<int>(entry.vnum), err)) {
				mudlog(LOG_ERROR, "clan_symbol_boot_migrate: archive %u failed: %s",
					   entry.vnum, err.c_str());
				++failed;
			}
			else {
				mudlog(LOG_CHECK,
					   "clan_symbol_boot_migrate: %u -> instance %llu base %d "
					   "prince=%s archived",
					   entry.vnum, static_cast<unsigned long long>(instance_id),
					   base_vnum, entry.prince_name);
				++archived;
				++converted;
			}
		}
		else {
			mudlog(LOG_CHECK,
				   "clan_symbol_boot_migrate: %u instance %llu ok, archive deferred",
				   entry.vnum, static_cast<unsigned long long>(instance_id));
			++deferred;
			++converted;
		}
	}

	mudlog(LOG_CHECK,
		   "clan_symbol_boot_migrate: done converted=%d archived=%d deferred=%d "
		   "skipped=%d missing_prince=%d missing_obj=%d bad_base=%d failed=%d",
		   converted, archived, deferred, skipped, missing_prince, missing_obj,
		   bad_base, failed);
}

struct ClanRegistry {
	unsigned vnum = 0;
	unsigned base_vnum = 0;
	unsigned long long prince_toon_id = 0;
	unsigned long long template_instance_id = 0;
	unsigned slots_max = kDefaultClanSymbolSlots;
	std::string prince_name;
};

[[nodiscard]] bool load_registry_by_prince(DB* db, const char* prince_name,
										   ClanRegistry& out) {
	if(!db || !prince_name || !*prince_name) {
		return false;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::string esc;
		sql_escape(h, prince_name, esc);
		std::ostringstream sql;
		sql << "SELECT vnum, IFNULL(base_vnum,0), IFNULL(prince_toon_id,0), "
			   "IFNULL(instance_id,0), IFNULL(slots_max,"
			<< kDefaultClanSymbolSlots
			<< "), prince_name FROM clan_symbol WHERE LOWER(prince_name)=LOWER('"
			<< esc << "') AND active=1 LIMIT 1";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			return false;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return false;
		}
		MYSQL_ROW row = mysql_fetch_row(res);
		if(!row) {
			mysql_free_result(res);
			return false;
		}
		out.vnum = row[0] ? static_cast<unsigned>(strtoul(row[0], nullptr, 10)) : 0;
		out.base_vnum =
			row[1] ? static_cast<unsigned>(strtoul(row[1], nullptr, 10)) : 0;
		out.prince_toon_id = row[2] ? strtoull(row[2], nullptr, 10) : 0;
		out.template_instance_id = row[3] ? strtoull(row[3], nullptr, 10) : 0;
		out.slots_max = row[4] ? static_cast<unsigned>(strtoul(row[4], nullptr, 10))
							   : kDefaultClanSymbolSlots;
		out.prince_name = row[5] ? row[5] : prince_name;
		mysql_free_result(res);
		return out.prince_toon_id != 0 || !out.prince_name.empty();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol load_registry: %s", e.what());
		return false;
	}
}

[[nodiscard]] bool is_clan_symbol_obj(const struct obj_data* obj,
									  unsigned long long prince_toon_id) {
	if(!obj) {
		return false;
	}
	if(obj->obj_flags.type_flag != ITEM_CLAN_SYMBOL &&
	   !IS_SET(obj->obj_flags.wear_flags, ITEM_WEAR_CLAN_SYMBOL)) {
		return false;
	}
	if(prince_toon_id == 0) {
		return true;
	}
	return static_cast<unsigned long long>(obj->obj_flags.value[0]) ==
		   prince_toon_id;
}

[[nodiscard]] bool char_holds_clan_symbol(struct char_data* ch,
										  unsigned long long prince_toon_id) {
	if(!ch) {
		return false;
	}
	for(int i = 0; i < MAX_WEAR; ++i) {
		if(is_clan_symbol_obj(ch->equipment[i], prince_toon_id)) {
			return true;
		}
	}
	for(struct obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(is_clan_symbol_obj(obj, prince_toon_id)) {
			return true;
		}
		/* un livello di contenitori basta per eq tipico */
		for(struct obj_data* in = obj->contains; in; in = in->next_content) {
			if(is_clan_symbol_obj(in, prince_toon_id)) {
				return true;
			}
		}
	}
	return false;
}

void destroy_clan_symbol_obj(struct obj_data* obj,
							 unsigned long long template_instance_id,
							 struct char_data* actor) {
	if(!obj) {
		return;
	}
	const unsigned long long iid = obj->db_instance_id;
	if(obj->equipped_by) {
		const int pos = obj->eq_pos;
		struct char_data* wearer = obj->equipped_by;
		if(pos >= 0) {
			obj_to_char(unequip_char(wearer, pos), wearer);
		}
	}
	if(iid != 0 && iid != template_instance_id) {
		object_instance_delete(iid, actor);
	}
	else if(iid != 0) {
		/* Template condiviso del migrate: non soft-delete, solo scollega. */
		obj->db_instance_id = 0;
	}
	extract_obj(obj);
}

void strip_clan_symbols_recursive(struct obj_data* list,
								  unsigned long long prince_toon_id,
								  unsigned long long template_instance_id,
								  struct char_data* actor, int* stripped) {
	struct obj_data* next = nullptr;
	for(struct obj_data* obj = list; obj; obj = next) {
		next = obj->next_content;
		if(obj->contains) {
			strip_clan_symbols_recursive(obj->contains, prince_toon_id,
										 template_instance_id, actor, stripped);
		}
		if(is_clan_symbol_obj(obj, prince_toon_id)) {
			destroy_clan_symbol_obj(obj, template_instance_id, actor);
			if(stripped) {
				++(*stripped);
			}
		}
	}
}

int count_online_symbol_holders(unsigned long long prince_toon_id,
								std::unordered_set<std::string>& names_lower) {
	int n = 0;
	for(struct char_data* ch = character_list; ch; ch = ch->next) {
		if(IS_NPC(ch) || !GET_NAME(ch)) {
			continue;
		}
		if(!char_holds_clan_symbol(ch, prince_toon_id)) {
			continue;
		}
		std::string low = GET_NAME(ch);
		for(char& c : low) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
		if(names_lower.insert(low).second) {
			++n;
		}
	}
	return n;
}

int count_db_symbol_holders(DB* db, unsigned long long prince_toon_id,
							const std::unordered_set<std::string>& skip_lower) {
	if(!db || prince_toon_id == 0) {
		return 0;
	}
	int n = 0;
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT DISTINCT t.name FROM character_inventory ci "
			   "INNER JOIN toon t ON t.id = ci.toon_id "
			   "LEFT JOIN object_instance oi ON oi.id = ci.instance_id "
			   "WHERE (ci.deleted = 0 OR ci.deleted IS NULL) "
			   "AND ci.value0 = "
			<< prince_toon_id
			<< " AND (oi.type_flag = " << static_cast<int>(ITEM_CLAN_SYMBOL)
			<< " OR oi.id IS NULL)";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "clan_symbol count_db: %s", mysql_error(h));
			return 0;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return 0;
		}
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(!row[0] || !*row[0]) {
				continue;
			}
			std::string low = row[0];
			for(char& c : low) {
				c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
			}
			if(skip_lower.count(low)) {
				continue;
			}
			++n;
		}
		mysql_free_result(res);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol count_db: %s", e.what());
	}
	return n;
}

int clan_symbol_slots_used(DB* db, unsigned long long prince_toon_id) {
	std::unordered_set<std::string> online;
	const int on = count_online_symbol_holders(prince_toon_id, online);
	const int off = count_db_symbol_holders(db, prince_toon_id, online);
	return on + off;
}

void list_vassals(struct char_data* ch, const ClanRegistry& reg) {
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", ch);
		return;
	}
	std::ostringstream out;
	out << "Vassalli di " << reg.prince_name << ":\n\r";
	int count = 0;
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::string esc;
		sql_escape(h, reg.prince_name, esc);
		std::ostringstream sql;
		sql << "SELECT t.name FROM character_prefs cp "
			   "INNER JOIN toon t ON t.id = cp.toon_id "
			   "WHERE cp.pref_key='principe' AND LOWER(cp.pref_value)=LOWER('"
			<< esc << "') ORDER BY t.name";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			send_to_char("Errore lettura vassalli.\n\r", ch);
			return;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			send_to_char("Nessun vassallo.\n\r", ch);
			return;
		}
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(!row[0]) {
				continue;
			}
			++count;
			const char* name = row[0];
			bool online = false;
			for(struct char_data* i = character_list; i; i = i->next) {
				if(!IS_NPC(i) && GET_NAME(i) &&
				   strcasecmp(GET_NAME(i), name) == 0) {
					online = true;
					break;
				}
			}
			out << "  - " << name << (online ? " (online)" : " (offline)")
				<< "\n\r";
		}
		mysql_free_result(res);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan vassalli: %s", e.what());
		send_to_char("Errore lettura vassalli.\n\r", ch);
		return;
	}
	if(count == 0) {
		out << "  (nessuno)\n\r";
	}
	else {
		out << "Totale: " << count << "\n\r";
	}
	send_to_char(out.str().c_str(), ch);
}

void list_symbol_holders(struct char_data* ch, const ClanRegistry& reg) {
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", ch);
		return;
	}
	std::unordered_set<std::string> seen;
	std::ostringstream out;
	out << "Simboli di casata di " << reg.prince_name << " (usati "
		<< clan_symbol_slots_used(db, reg.prince_toon_id) << "/" << reg.slots_max
		<< "):\n\r";

	for(struct char_data* i = character_list; i; i = i->next) {
		if(IS_NPC(i) || !GET_NAME(i)) {
			continue;
		}
		if(!char_holds_clan_symbol(i, reg.prince_toon_id)) {
			continue;
		}
		std::string low = GET_NAME(i);
		for(char& c : low) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
		seen.insert(low);
		out << "  - " << GET_NAME(i) << " (online)\n\r";
	}

	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT DISTINCT t.name FROM character_inventory ci "
			   "INNER JOIN toon t ON t.id = ci.toon_id "
			   "LEFT JOIN object_instance oi ON oi.id = ci.instance_id "
			   "WHERE (ci.deleted = 0 OR ci.deleted IS NULL) "
			   "AND ci.value0 = "
			<< reg.prince_toon_id
			<< " AND (oi.type_flag = " << static_cast<int>(ITEM_CLAN_SYMBOL)
			<< " OR oi.id IS NULL) "
			<< "ORDER BY t.name";
		if(mysql_query(h, sql.str().c_str()) == 0) {
			MYSQL_RES* res = mysql_store_result(h);
			if(res) {
				while(MYSQL_ROW row = mysql_fetch_row(res)) {
					if(!row[0]) {
						continue;
					}
					std::string low = row[0];
					for(char& c : low) {
						c = static_cast<char>(
							std::tolower(static_cast<unsigned char>(c)));
					}
					if(!seen.insert(low).second) {
						continue;
					}
					out << "  - " << row[0] << " (offline)\n\r";
				}
				mysql_free_result(res);
			}
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan simboli: %s", e.what());
	}

	if(seen.empty()) {
		out << "  (nessuno)\n\r";
	}
	send_to_char(out.str().c_str(), ch);
}

bool clan_assegna_to_vassal(struct char_data* prince, struct char_data* vassal) {
	if(!prince || !vassal) {
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", prince);
		return false;
	}
	ClanRegistry reg;
	if(!load_registry_by_prince(db, GET_NAME(prince), reg) ||
	   reg.template_instance_id == 0 || reg.base_vnum == 0) {
		send_to_char("La tua casata non ha un simbolo registrato.\n\r", prince);
		return false;
	}
	if(!IS_VASSALLOOF(vassal, GET_NAME(prince))) {
		act("$N non e' tu$b vassall$b.", TRUE, prince, nullptr, vassal, TO_CHAR);
		return false;
	}
	if(char_holds_clan_symbol(vassal, reg.prince_toon_id)) {
		act("$N ha gia' il simbolo della casata.", TRUE, prince, nullptr, vassal,
			TO_CHAR);
		return false;
	}
	const int used = clan_symbol_slots_used(db, reg.prince_toon_id);
	if(used >= static_cast<int>(reg.slots_max)) {
		send_to_char("Hai raggiunto il limite di simboli assegnabili.\n\r", prince);
		return false;
	}

	struct obj_data* obj = object_instance_materialize(reg.template_instance_id);
	if(!obj) {
		send_to_char("Impossibile creare una copia del simbolo.\n\r", prince);
		return false;
	}
	apply_fields(obj, static_cast<int>(reg.prince_toon_id));
	obj->db_instance_id = 0;
	if(GET_NAME(vassal)) {
		strncpy(obj->personal_owner, GET_NAME(vassal),
				sizeof(obj->personal_owner) - 1);
		obj->personal_owner[sizeof(obj->personal_owner) - 1] = '\0';
	}
	const unsigned long long nid = object_instance_persist(
		obj, static_cast<int>(reg.base_vnum), 0, prince, true, kClanAssegnaActor);
	if(nid == 0) {
		extract_obj(obj);
		send_to_char("Salvataggio simbolo fallito.\n\r", prince);
		return false;
	}
	obj_to_char(obj, vassal);
	act("Assegni il simbolo di casata a $N.", TRUE, prince, nullptr, vassal,
		TO_CHAR);
	act("$n ti assegna il simbolo della casata.", TRUE, prince, nullptr, vassal,
		TO_VICT);
	act("$n assegna il simbolo di casata a $N.", TRUE, prince, nullptr, vassal,
		TO_NOTVICT);
	return true;
}

bool set_clan_quota(struct char_data* ch, const char* prince_name, unsigned slots) {
	DB* db = Sql::getMysql();
	if(!db || !prince_name || !*prince_name) {
		return false;
	}
	if(slots < 1 || slots > 50) {
		send_to_char("Quota non valida (1-50).\n\r", ch);
		return false;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::string esc;
		sql_escape(h, prince_name, esc);
		std::ostringstream sql;
		sql << "UPDATE clan_symbol SET slots_max=" << slots
			<< ", updated_at=NOW() WHERE LOWER(prince_name)=LOWER('" << esc << "')";
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		db->execute(sql.str().c_str());
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan quota: %s", e.what());
		send_to_char("Aggiornamento quota fallito.\n\r", ch);
		return false;
	}
	ClanRegistry reg;
	if(!load_registry_by_prince(db, prince_name, reg)) {
		send_to_char("Principe/casata non trovata in clan_symbol.\n\r", ch);
		return false;
	}
	char buf[128];
	snprintf(buf, sizeof(buf), "Quota simboli di %s impostata a %u (ora usati %d).\n\r",
			 reg.prince_name.c_str(), reg.slots_max,
			 clan_symbol_slots_used(db, reg.prince_toon_id));
	send_to_char(buf, ch);
	return true;
}

void clan_symbol_strip_from_char(struct char_data* ch, const char* prince_name) {
	if(!ch || !prince_name || !*prince_name) {
		return;
	}
	DB* db = Sql::getMysql();
	ClanRegistry reg;
	unsigned long long prince_id = 0;
	unsigned long long template_id = 0;
	if(db && load_registry_by_prince(db, prince_name, reg)) {
		prince_id = reg.prince_toon_id;
		template_id = reg.template_instance_id;
	}
	else if(db) {
		try {
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			prince_id = lookup_toon_id_ci(db, prince_name);
			t.commit();
		}
		catch(const odb::exception&) {
		}
	}
	if(prince_id == 0) {
		return;
	}

	int stripped = 0;
	for(int i = 0; i < MAX_WEAR; ++i) {
		struct obj_data* obj = ch->equipment[i];
		if(!is_clan_symbol_obj(obj, prince_id)) {
			continue;
		}
		destroy_clan_symbol_obj(obj, template_id, ch);
		++stripped;
	}
	strip_clan_symbols_recursive(ch->carrying, prince_id, template_id, ch,
								 &stripped);
	if(stripped > 0) {
		send_to_char("Il simbolo di casata ti viene ritirato.\n\r", ch);
		std::ostringstream msg;
		msg << "clan_symbol_strip: "
			<< (GET_NAME(ch) ? GET_NAME(ch) : "?") << " lost " << stripped
			<< " symbols of " << prince_name;
		mudlog(LOG_CHECK, "%s", msg.str().c_str());
	}
}

void show_clan_usage(struct char_data* ch) {
	send_to_char(
		"Uso:\n\r"
		"  clan vassalli              - lista i tuoi vassalli\n\r"
		"  clan simboli               - chi ha i simboli della casata\n\r"
		"  clan assegna <nome>        - assegna un simbolo (stessa stanza)\n\r",
		ch);
	if(IS_IMMORTALE(ch)) {
		send_to_char(
			"  clan vassalli <principe>   - (god) lista vassalli\n\r"
			"  clan simboli <principe>    - (god) lista simboli\n\r"
			"  clan quota <principe> [n]  - (god) mostra/imposta quota (default 5)\n\r",
			ch);
	}
}

[[nodiscard]] bool resolve_prince_target(struct char_data* ch, const char* arg,
										 ClanRegistry& reg) {
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", ch);
		return false;
	}
	std::string name;
	if(arg && *arg) {
		name = arg;
		while(!name.empty() && (name.back() == ' ' || name.back() == '\r' ||
								name.back() == '\n')) {
			name.pop_back();
		}
	}
	if(name.empty()) {
		if(!IS_PRINCE(ch) || !GET_NAME(ch)) {
			send_to_char("Specifica il nome del principe.\n\r", ch);
			return false;
		}
		name = GET_NAME(ch);
	}
	else if(!IS_IMMORTALE(ch) &&
			(!IS_PRINCE(ch) || !GET_NAME(ch) ||
			 strcasecmp(GET_NAME(ch), name.c_str()) != 0)) {
		send_to_char("Puoi gestire solo la tua casata.\n\r", ch);
		return false;
	}
	if(!load_registry_by_prince(db, name.c_str(), reg)) {
		send_to_char("Casata/principe non trovato in clan_symbol.\n\r", ch);
		return false;
	}
	return true;
}

ACTION_FUNC(do_clan) {
	if(!ch || IS_NPC(ch)) {
		return;
	}

	char cmdbuf[MAX_INPUT_LENGTH];
	char argbuf[MAX_INPUT_LENGTH];
	arg = one_argument(arg, cmdbuf);
	one_argument(arg, argbuf);

	if(!*cmdbuf) {
		show_clan_usage(ch);
		return;
	}

	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", ch);
		return;
	}

	if(is_abbrev(cmdbuf, "vassalli")) {
		if(!IS_PRINCE(ch) && !IS_IMMORTALE(ch)) {
			send_to_char("Solo i principi possono usare questo comando.\n\r", ch);
			return;
		}
		ClanRegistry reg;
		if(!resolve_prince_target(ch, argbuf, reg)) {
			return;
		}
		list_vassals(ch, reg);
		return;
	}
	if(is_abbrev(cmdbuf, "simboli")) {
		if(!IS_PRINCE(ch) && !IS_IMMORTALE(ch)) {
			send_to_char("Solo i principi possono usare questo comando.\n\r", ch);
			return;
		}
		ClanRegistry reg;
		if(!resolve_prince_target(ch, argbuf, reg)) {
			return;
		}
		list_symbol_holders(ch, reg);
		return;
	}
	if(is_abbrev(cmdbuf, "assegna")) {
		if(!IS_PRINCE(ch)) {
			send_to_char("Solo un principe puo' assegnare il simbolo.\n\r", ch);
			return;
		}
		if(!*argbuf) {
			send_to_char("A chi vuoi assegnare il simbolo?\n\r", ch);
			return;
		}
		struct char_data* vict = get_char_room_vis(ch, argbuf);
		if(!vict || IS_NPC(vict)) {
			send_to_char("Non e' qui.\n\r", ch);
			return;
		}
		clan_assegna_to_vassal(ch, vict);
		return;
	}
	if(is_abbrev(cmdbuf, "quota")) {
		if(!IS_IMMORTALE(ch)) {
			send_to_char("Solo gli immortali possono gestire la quota.\n\r", ch);
			return;
		}
		if(!*argbuf) {
			send_to_char("Uso: clan quota <principe> [n]\n\r", ch);
			return;
		}
		char prince_buf[MAX_INPUT_LENGTH];
		char num_buf[MAX_INPUT_LENGTH];
		arg = one_argument(arg, prince_buf);
		one_argument(arg, num_buf);
		ClanRegistry reg;
		if(!load_registry_by_prince(db, prince_buf, reg)) {
			send_to_char("Casata/principe non trovato in clan_symbol.\n\r", ch);
			return;
		}
		if(!*num_buf) {
			char buf[160];
			snprintf(buf, sizeof(buf),
					 "Quota di %s: %u (usati %d).\n\r", reg.prince_name.c_str(),
					 reg.slots_max,
					 clan_symbol_slots_used(db, reg.prince_toon_id));
			send_to_char(buf, ch);
			return;
		}
		set_clan_quota(ch, prince_buf, static_cast<unsigned>(atoi(num_buf)));
		return;
	}

	show_clan_usage(ch);
}

} // namespace Alarmud

#endif /* USE_MYSQL */

#if !USE_MYSQL
namespace Alarmud {
ACTION_FUNC(do_clan) {
	(void)arg;
	(void)cmd;
	send_to_char("Comando non disponibile.\n\r", ch);
}
} // namespace Alarmud
#endif

