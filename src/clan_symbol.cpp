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
			"  `active` TINYINT UNSIGNED NOT NULL DEFAULT 1,"
			"  `updated_at` DATETIME NOT NULL,"
			"  PRIMARY KEY (`vnum`),"
			"  KEY `idx_clan_symbol_prince` (`prince_toon_id`),"
			"  KEY `idx_clan_symbol_instance` (`instance_id`)"
			") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol: CREATE TABLE: %s", e.what());
		return false;
	}
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
	/* Revert 2026-08: migrate disabilitata finche' non sistemiamo
	 * header OK_HEADER, principi mancanti e idempotenza (legacy_edit /
	 * doppio Montero). Seed tabella resta via ensure + seed_row se
	 * riabilitata. */
	mudlog(LOG_CHECK, "clan_symbol_boot_migrate: DISABLED (revert in corso), skip");
	return;

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

} // namespace Alarmud

#endif /* USE_MYSQL */
