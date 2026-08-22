/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "clan_symbol.hpp"

#if USE_MYSQL

#include "logging.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "utility.hpp"
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

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <strings.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "interpreter.hpp"
#include "multiclass.hpp"
#include "act.comm.hpp"
#include "cmdid.hpp"

namespace Alarmud {

namespace {

namespace fs = std::filesystem;

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

[[nodiscard]] bool objects_file_is_regular(int vnum) {
	std::error_code ec;
	const fs::path path = fs::path(OBJ_DIR) / std::to_string(vnum);
	return fs::is_regular_file(path, ec);
}

void set_personal_owner(struct obj_data* obj, std::string_view name) {
	if(obj == nullptr || name.empty()) {
		return;
	}
	const std::size_t n =
		std::min(name.size(), sizeof(obj->personal_owner) - 1u);
	name.copy(obj->personal_owner, n);
	obj->personal_owner[n] = '\0';
}

[[nodiscard]] bool parse_unsigned(std::string_view text, unsigned& out) {
	if(text.empty()) {
		return false;
	}
	unsigned value = 0;
	const char* const begin = text.data();
	const char* const end = text.data() + text.size();
	const auto [ptr, ec] = std::from_chars(begin, end, value);
	if(ec != std::errc{} || ptr != end) {
		return false;
	}
	out = value;
	return true;
}

[[nodiscard]] unsigned long long parse_ull(const char* text) {
	if(text == nullptr || !*text) {
		return 0;
	}
	unsigned long long value = 0;
	const char* end = text + std::strlen(text);
	const auto [ptr, ec] = std::from_chars(text, end, value);
	if(ec != std::errc{} || ptr != end) {
		return 0;
	}
	return value;
}

[[nodiscard]] unsigned parse_u(const char* text) {
	unsigned value = 0;
	if(text == nullptr || !parse_unsigned(text, value)) {
		return 0;
	}
	return value;
}

[[nodiscard]] long parse_long(const char* text) {
	if(text == nullptr || !*text) {
		return 0;
	}
	long value = 0;
	const char* end = text + std::strlen(text);
	const auto [ptr, ec] = std::from_chars(text, end, value);
	if(ec != std::errc{} || ptr != end) {
		return 0;
	}
	return value;
}

bool sql_escape(MYSQL* h, const std::string& in, std::string& out) {
	out.resize(in.size() * 2 + 1);
	const unsigned long n = mysql_real_escape_string(
		h, out.data(), in.c_str(), static_cast<unsigned long>(in.size()));
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
				id = parse_ull(row[0]);
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
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT name FROM toon WHERE id=" << id << " LIMIT 1";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "clan_symbol: name for id %llu: %s",
				   static_cast<unsigned long long>(id), mysql_error(h));
			return {};
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return {};
		}
		std::string out;
		if(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(row[0] && *row[0]) {
				out = row[0];
			}
		}
		mysql_free_result(res);
		return out;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol: name for id %llu: %s",
			   static_cast<unsigned long long>(id), e.what());
		return {};
	}
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
		const long n = (row && row[0]) ? parse_long(row[0]) : 0;
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
	std::error_code ec;
	for(const fs::directory_entry& entry : fs::directory_iterator(RENT_DIR, ec)) {
		if(!entry.is_regular_file(ec)) {
			continue;
		}
		const std::string pname = entry.path().filename().string();
		if(pname.empty() || pname.front() == '.' ||
		   pname.find(".aux") != std::string::npos) {
			continue;
		}
		const bool ok_name = std::all_of(pname.begin(), pname.end(), [](unsigned char c) {
			return std::isalnum(c) || c == '_' || c == '-';
		});
		if(!ok_name) {
			continue;
		}
		const std::string path = entry.path().string();
		obj_file_u st {};
		if(!legacy_load_rent_file_path(path.c_str(), st)) {
			continue;
		}
		for(int i = 0; i < st.number && i < MAX_OBJ_SAVE; ++i) {
			const unsigned v = st.objects[i].item_number;
			if(clan_symbol_is_listed_vnum(v)) {
				out[v].insert(pname);
			}
		}
	}
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
	const std::string path = (fs::path(RENT_DIR) / name).string();
	FILE* f = std::fopen(path.c_str(), "w+b");
	if(!f) {
		mudlog(LOG_SYSERR, "clan_symbol: cannot rewrite rent %s", path.c_str());
		return false;
	}
	const size_t n = std::fwrite(&st, sizeof(st), 1, f);
	std::fclose(f);
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

/* forward: definita piu' sotto insieme a clan assegna / strip */
void destroy_clan_symbol_obj(struct obj_data* obj,
							 unsigned long long template_instance_id,
							 struct char_data* actor);

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
		send_to_char("Questo simbolo non e' ancora associato a un clan.\n\r",
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
		send_to_char("Clan del simbolo non trovata.\n\r", ch);
		return false;
	}
	if(GET_NAME(ch) && strcasecmp(GET_NAME(ch), pname.c_str()) == 0) {
		return true;
	}
	if(HAS_PRINCE(ch) && GET_PRINCE(ch) &&
	   strcasecmp(GET_PRINCE(ch), pname.c_str()) == 0) {
		return true;
	}
	send_to_char("Solo i membri di quel clan possono indossarlo.\n\r", ch);
	return false;
}

bool clan_symbol_is_obj(const struct obj_data* obj) {
	if(!obj) {
		return false;
	}
	return obj->obj_flags.type_flag == ITEM_CLAN_SYMBOL ||
		   IS_SET(obj->obj_flags.wear_flags, ITEM_WEAR_CLAN_SYMBOL);
}

void clan_symbol_refresh_affects_from_instance(struct obj_data* obj) {
	if(!obj || !clan_symbol_is_obj(obj) || obj->db_instance_id == 0) {
		return;
	}
	struct char_data* wearer = obj->equipped_by;
	const int eq_pos = static_cast<int>(obj->eq_pos);
	const bool was_worn = (wearer != nullptr && eq_pos >= 0 && eq_pos < MAX_WEAR &&
						   wearer->equipment[eq_pos] == obj);
	if(was_worn) {
		obj_to_char(unequip_char(wearer, eq_pos), wearer);
	}
	if(!object_instance_apply(obj, obj->db_instance_id)) {
		mudlog(LOG_SYSERR,
			   "clan_symbol_refresh_affects: apply failed instance %llu",
			   static_cast<unsigned long long>(obj->db_instance_id));
		if(was_worn && wearer && obj->carried_by == wearer &&
		   !wearer->equipment[WEAR_CLAN_SYMBOL]) {
			obj_from_char(obj);
			equip_char(wearer, obj, WEAR_CLAN_SYMBOL);
		}
		return;
	}
	if(was_worn && wearer && obj->carried_by == wearer &&
	   !wearer->equipment[WEAR_CLAN_SYMBOL]) {
		obj_from_char(obj);
		equip_char(wearer, obj, WEAR_CLAN_SYMBOL);
	}
}

bool clan_symbol_char_holds_any(struct char_data* ch) {
	if(!ch) {
		return false;
	}
	for(int i = 0; i < MAX_WEAR; ++i) {
		if(clan_symbol_is_obj(ch->equipment[i])) {
			return true;
		}
	}
	for(struct obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(clan_symbol_is_obj(obj)) {
			return true;
		}
		for(struct obj_data* in = obj->contains; in; in = in->next_content) {
			if(clan_symbol_is_obj(in)) {
				return true;
			}
		}
	}
	return false;
}

bool clan_symbol_can_receive(struct char_data* ch, const struct obj_data* obj,
							 bool silent) {
	if(!ch || !obj) {
		return false;
	}
	if(!clan_symbol_is_obj(obj)) {
		return true;
	}
	if(IS_IMMORTAL(ch)) {
		return true;
	}
	/* Gia' in suo possesso (re-equip / move interno): ok. */
	if(obj->carried_by == ch || obj->equipped_by == ch) {
		return true;
	}
	if(clan_symbol_char_holds_any(ch)) {
		if(!silent) {
			send_to_char("Puoi possedere un solo simbolo del clan.\n\r", ch);
		}
		return false;
	}
	return true;
}

void clan_symbol_try_auto_wear(struct char_data* ch, struct obj_data* obj) {
	if(!ch || !obj || IS_NPC(ch) || !clan_symbol_is_obj(obj)) {
		return;
	}
	clan_symbol_refresh_affects_from_instance(obj);
	if(ch->equipment[WEAR_CLAN_SYMBOL] || obj->equipped_by == ch) {
		return;
	}
	if(!clan_symbol_can_wear(ch, obj)) {
		return;
	}
	if(obj->in_obj) {
		obj_from_obj(obj);
		obj_to_char(obj, ch);
	}
	if(obj->carried_by != ch) {
		return;
	}
	obj_from_char(obj);
	equip_char(ch, obj, WEAR_CLAN_SYMBOL);
	act("Indossi $p: il marchio del casato ti riconosce come su$b.", false, ch,
		obj, nullptr, TO_CHAR);
}

void clan_symbol_enforce_single(struct char_data* ch) {
	if(!ch || IS_NPC(ch) || IS_IMMORTAL(ch)) {
		return;
	}
	struct obj_data* keep = nullptr;
	if(clan_symbol_is_obj(ch->equipment[WEAR_CLAN_SYMBOL])) {
		keep = ch->equipment[WEAR_CLAN_SYMBOL];
	}
	std::vector<struct obj_data*> extras;
	auto consider = [&](struct obj_data* obj) {
		if(!clan_symbol_is_obj(obj) || obj == keep) {
			return;
		}
		if(!keep) {
			keep = obj;
			return;
		}
		extras.push_back(obj);
	};
	for(int i = 0; i < MAX_WEAR; ++i) {
		consider(ch->equipment[i]);
	}
	for(struct obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		consider(obj);
		for(struct obj_data* in = obj->contains; in; in = in->next_content) {
			consider(in);
		}
	}
	if(!extras.empty()) {
		for(struct obj_data* obj : extras) {
			if(obj->equipped_by) {
				const int pos = obj->eq_pos;
				if(pos >= 0 && pos < MAX_WEAR) {
					obj_to_char(unequip_char(ch, pos), ch);
				}
			}
			if(obj->in_obj) {
				obj_from_obj(obj);
				obj_to_char(obj, ch);
			}
			destroy_clan_symbol_obj(obj, 0, ch);
		}
		send_to_char(
			"Puoi avere un solo simbolo del clan: i pezzi in eccesso sono stati "
			"ritirati.\n\r",
			ch);
		mudlog(LOG_CHECK, "clan_symbol_enforce_single: stripped extras for %s",
			   GET_NAME(ch));
	}
	if(keep) {
		clan_symbol_refresh_affects_from_instance(keep);
	}
	clan_symbol_try_auto_wear(ch, keep);
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
		set_personal_owner(obj, entry.prince_name);

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
		out.vnum = parse_u(row[0]);
		out.base_vnum = parse_u(row[1]);
		out.prince_toon_id = parse_ull(row[2]);
		out.template_instance_id = parse_ull(row[3]);
		out.slots_max = row[4] ? parse_u(row[4]) : kDefaultClanSymbolSlots;
		out.prince_name = row[5] ? row[5] : prince_name;
		mysql_free_result(res);
		return out.prince_toon_id != 0 || !out.prince_name.empty();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol load_registry: %s", e.what());
		return false;
	}
}

[[nodiscard]] bool load_registry_by_prince_toon_id(DB* db,
												  unsigned long long prince_toon_id,
												  ClanRegistry& out) {
	if(!db || prince_toon_id == 0) {
		return false;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT vnum, IFNULL(base_vnum,0), IFNULL(prince_toon_id,0), "
			   "IFNULL(instance_id,0), IFNULL(slots_max,"
			<< kDefaultClanSymbolSlots
			<< "), prince_name FROM clan_symbol WHERE prince_toon_id="
			<< prince_toon_id << " AND active=1 LIMIT 1";
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
		out.vnum = parse_u(row[0]);
		out.base_vnum = parse_u(row[1]);
		out.prince_toon_id = parse_ull(row[2]);
		out.template_instance_id = parse_ull(row[3]);
		out.slots_max = row[4] ? parse_u(row[4]) : kDefaultClanSymbolSlots;
		out.prince_name = row[5] ? row[5] : "";
		mysql_free_result(res);
		return out.prince_toon_id != 0;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan_symbol load_registry by id: %s", e.what());
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
			<< " OR (oi.id IS NULL AND EXISTS ("
			   "SELECT 1 FROM clan_symbol cs WHERE cs.active=1 "
			   "AND cs.prince_toon_id="
			<< prince_toon_id
			<< " AND (ci.item_number=cs.base_vnum OR ci.item_number=cs.vnum)"
			   ")))";
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

int clan_symbol_slots_used(DB* db, unsigned long long prince_toon_id);

[[nodiscard]] std::string ascii_lower_copy(std::string s) {
	for(char& c : s) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return s;
}

[[nodiscard]] std::string file_canonical_pc_name(const char* name) {
	if(!name || !*name) {
		return {};
	}
	char_file_u st {};
	if(legacy_load_char_file(name, st) && st.name[0]) {
		return st.name;
	}
	return name;
}

[[nodiscard]] bool rent_elem_is_clan_symbol_of(const obj_file_elem& o,
												 const ClanRegistry& reg) {
	if(o.item_number == 0) {
		return false;
	}
	if(reg.vnum != 0 && o.item_number == static_cast<ush_int>(reg.vnum)) {
		return true;
	}
	if(reg.base_vnum != 0 &&
	   o.item_number == static_cast<ush_int>(reg.base_vnum)) {
		return true;
	}
	/* Simboli migrati: value0 = prince_toon_id. */
	if(reg.prince_toon_id != 0 &&
	   static_cast<unsigned long long>(o.value[0]) == reg.prince_toon_id) {
		return true;
	}
	return false;
}

[[nodiscard]] bool rent_holds_clan_symbol_of(const obj_file_u& st,
											   const ClanRegistry& reg) {
	for(int i = 0; i < st.number && i < MAX_OBJ_SAVE; ++i) {
		if(rent_elem_is_clan_symbol_of(st.objects[i], reg)) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] bool aux_read_principe(const char* name, std::string& prince_out) {
	prince_out.clear();
	LegacyCharAux aux;
	if(!legacy_load_char_aux(name, aux)) {
		return false;
	}
	for(const LegacyAuxEntry& e : aux.entries) {
		if(strcasecmp(e.tag.c_str(), "principe") == 0 && !e.value.empty()) {
			prince_out = e.value;
			return true;
		}
	}
	return false;
}

[[nodiscard]] bool aux_is_vassal_of(const char* vassal_name,
									 const char* prince_name) {
	if(!vassal_name || !*vassal_name || !prince_name || !*prince_name) {
		return false;
	}
	std::string prince;
	if(!aux_read_principe(vassal_name, prince)) {
		return false;
	}
	return strcasecmp(prince.c_str(), prince_name) == 0;
}

bool aux_clear_principe_if_matches(const char* vassal_name,
								   const char* prince_name) {
	if(!vassal_name || !*vassal_name || !prince_name || !*prince_name) {
		return false;
	}
	LegacyCharAux aux;
	if(!legacy_load_char_aux(vassal_name, aux)) {
		return false;
	}
	bool found = false;
	std::vector<LegacyAuxEntry> keep;
	keep.reserve(aux.entries.size());
	for(const LegacyAuxEntry& e : aux.entries) {
		if(strcasecmp(e.tag.c_str(), "principe") == 0 &&
		   strcasecmp(e.value.c_str(), prince_name) == 0) {
			found = true;
			continue;
		}
		keep.push_back(e);
	}
	if(!found) {
		return false;
	}

	const std::string lowered = lower(vassal_name);
	char path[256];
	std::snprintf(path, sizeof(path), "%s/%s.aux", RENT_DIR, lowered.c_str());
	FILE* fp = std::fopen(path, "w");
	if(!fp) {
		std::snprintf(path, sizeof(path), "%s/%s.aux", RENT_DIR, vassal_name);
		fp = std::fopen(path, "w");
	}
	if(!fp) {
		mudlog(LOG_SYSERR, "clan aux_clear: cannot write %s", path);
		return false;
	}
	for(const LegacyAuxEntry& e : keep) {
		std::fprintf(fp, "%s:%s\n", e.tag.c_str(), e.value.c_str());
	}
	std::fclose(fp);
	return true;
}

bool strip_clan_symbols_from_rent_file(const char* name, const ClanRegistry& reg) {
	if(!name || !*name) {
		return false;
	}
	obj_file_u st {};
	if(!legacy_load_rent_file(name, st)) {
		return false;
	}
	int w = 0;
	bool changed = false;
	for(int i = 0; i < st.number && i < MAX_OBJ_SAVE; ++i) {
		if(rent_elem_is_clan_symbol_of(st.objects[i], reg)) {
			changed = true;
			continue;
		}
		if(w != i) {
			st.objects[w] = st.objects[i];
		}
		++w;
	}
	if(!changed) {
		return false;
	}
	for(int i = w; i < st.number && i < MAX_OBJ_SAVE; ++i) {
		st.objects[i] = obj_file_elem {};
	}
	st.number = w;

	const std::string lowered = lower(name);
	const std::string path = (fs::path(RENT_DIR) / lowered).string();
	FILE* f = std::fopen(path.c_str(), "w+b");
	if(!f) {
		mudlog(LOG_SYSERR, "clan strip rent: cannot write %s", path.c_str());
		return false;
	}
	const size_t n = std::fwrite(&st, sizeof(st), 1, f);
	std::fclose(f);
	return n == 1;
}

void collect_vassals_from_aux(const char* prince_name,
							  std::map<std::string, std::string>& by_lower) {
	if(!prince_name || !*prince_name) {
		return;
	}
	std::error_code ec;
	for(const fs::directory_entry& entry : fs::directory_iterator(RENT_DIR, ec)) {
		if(!entry.is_regular_file(ec)) {
			continue;
		}
		const std::string fname = entry.path().filename().string();
		if(fname.size() < 5 || fname.compare(fname.size() - 4, 4, ".aux") != 0) {
			continue;
		}
		const std::string stem = fname.substr(0, fname.size() - 4);
		if(stem.empty() || stem.front() == '.') {
			continue;
		}
		if(toon_is_migrated_by_name(stem.c_str())) {
			continue;
		}
		std::string prince;
		if(!aux_read_principe(stem.c_str(), prince) ||
		   strcasecmp(prince.c_str(), prince_name) != 0) {
			continue;
		}
		const std::string low = ascii_lower_copy(stem);
		if(by_lower.count(low)) {
			continue;
		}
		by_lower.emplace(low, file_canonical_pc_name(stem.c_str()));
	}
}

void collect_symbol_holders_from_rent(const ClanRegistry& reg,
									  std::map<std::string, std::string>& by_lower) {
	std::error_code ec;
	for(const fs::directory_entry& entry : fs::directory_iterator(RENT_DIR, ec)) {
		if(!entry.is_regular_file(ec)) {
			continue;
		}
		const std::string pname = entry.path().filename().string();
		if(pname.empty() || pname.front() == '.' ||
		   pname.find(".aux") != std::string::npos) {
			continue;
		}
		const bool ok_name =
			std::all_of(pname.begin(), pname.end(), [](unsigned char c) {
				return std::isalnum(c) || c == '_' || c == '-';
			});
		if(!ok_name) {
			continue;
		}
		if(toon_is_migrated_by_name(pname.c_str())) {
			continue;
		}
		const std::string low = ascii_lower_copy(pname);
		if(by_lower.count(low)) {
			continue;
		}
		obj_file_u st {};
		if(!legacy_load_rent_file_path(entry.path().string().c_str(), st)) {
			continue;
		}
		if(!rent_holds_clan_symbol_of(st, reg)) {
			continue;
		}
		by_lower.emplace(low, file_canonical_pc_name(pname.c_str()));
	}
}

int count_rent_symbol_holders(const ClanRegistry& reg,
							  const std::unordered_set<std::string>& skip_lower) {
	std::map<std::string, std::string> found;
	collect_symbol_holders_from_rent(reg, found);
	int n = 0;
	for(const auto& kv : found) {
		if(skip_lower.count(kv.first)) {
			continue;
		}
		++n;
	}
	return n;
}

int clan_symbol_slots_used(DB* db, unsigned long long prince_toon_id) {
	std::unordered_set<std::string> online;
	const int on = count_online_symbol_holders(prince_toon_id, online);
	const int off = count_db_symbol_holders(db, prince_toon_id, online);
	int off_rent = 0;
	if(db && prince_toon_id != 0) {
		const std::string pname = lookup_toon_name_by_id(db, prince_toon_id);
		ClanRegistry reg;
		if(!pname.empty() && load_registry_by_prince(db, pname.c_str(), reg)) {
			off_rent = count_rent_symbol_holders(reg, online);
		}
	}
	return on + off + off_rent;
}

/** PC reale: in poly usa desc->original (IS_PRINCE/IS_IMMORTALE falliscono sul mob). */
[[nodiscard]] struct char_data* clan_pc_identity(struct char_data* ch) {
	if(ch != nullptr && IS_POLY(ch) && ch->desc != nullptr &&
	   ch->desc->original != nullptr) {
		return ch->desc->original;
	}
	return ch;
}

[[nodiscard]] bool clan_is_prince(struct char_data* ch) {
	return ch != nullptr && IS_PRINCE(clan_pc_identity(ch));
}

[[nodiscard]] bool clan_is_immortale(struct char_data* ch) {
	return ch != nullptr && IS_IMMORTALE(clan_pc_identity(ch));
}

[[nodiscard]] struct char_data* find_pc_by_name_ci(const char* name) {
	if(name == nullptr || !*name) {
		return nullptr;
	}
	for(struct char_data* i = character_list; i; i = i->next) {
		if(IS_PC(i) && GET_NAME(i) && strcasecmp(GET_NAME(i), name) == 0) {
			return i;
		}
	}
	return nullptr;
}

void append_presence_line(std::ostringstream& out, const char* name,
						  struct char_data* pc) {
	out << "  - " << (name ? name : "?") << " (";
	if(pc == nullptr) {
		out << "$c0009assente$c0007";
	}
	else if(IS_LINKDEAD(pc)) {
		const bool female = (GET_SEX(pc) == SEX_FEMALE);
		out << "$c0011" << (female ? "disconnessa" : "disconnesso") << "$c0007";
	}
	else {
		const bool female = (GET_SEX(pc) == SEX_FEMALE);
		out << "$c0010" << (female ? "connessa" : "connesso") << "$c0007";
	}
	out << ")\n\r";
}

void list_vassals(struct char_data* ch, const ClanRegistry& reg) {
	std::map<std::string, std::string> by_lower;

	for(struct char_data* i = character_list; i; i = i->next) {
		if(!IS_PC(i) || !GET_NAME(i) ||
		   !IS_VASSALLOOF(i, reg.prince_name.c_str())) {
			continue;
		}
		by_lower.emplace(ascii_lower_copy(GET_NAME(i)), GET_NAME(i));
	}

	DB* db = Sql::getMysql();
	if(db) {
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
			if(mysql_query(h, sql.str().c_str()) == 0) {
				MYSQL_RES* res = mysql_store_result(h);
				if(res) {
					while(MYSQL_ROW row = mysql_fetch_row(res)) {
						if(!row[0]) {
							continue;
						}
						by_lower.emplace(ascii_lower_copy(row[0]), row[0]);
					}
					mysql_free_result(res);
				}
			}
		}
		catch(const odb::exception& e) {
			mudlog(LOG_SYSERR, "clan vassalli: %s", e.what());
		}
	}

	collect_vassals_from_aux(reg.prince_name.c_str(), by_lower);

	std::ostringstream out;
	out << "Vassalli di " << reg.prince_name << ":\n\r";
	if(by_lower.empty()) {
		out << "  (nessuno)\n\r";
	}
	else {
		for(const auto& kv : by_lower) {
			append_presence_line(out, kv.second.c_str(),
								 find_pc_by_name_ci(kv.second.c_str()));
		}
		out << "Totale: " << by_lower.size() << "\n\r";
	}
	send_to_char(out.str().c_str(), ch);
}

void list_symbol_holders(struct char_data* ch, const ClanRegistry& reg) {
	DB* db = Sql::getMysql();
	std::map<std::string, std::string> by_lower;
	std::ostringstream out;
	const int used =
		clan_symbol_slots_used(db, reg.prince_toon_id);
	out << "Simboli del clan di " << reg.prince_name << " (usati " << used << "/"
		<< reg.slots_max << "):\n\r";

	for(struct char_data* i = character_list; i; i = i->next) {
		if(!IS_PC(i) || !GET_NAME(i)) {
			continue;
		}
		if(!char_holds_clan_symbol(i, reg.prince_toon_id)) {
			continue;
		}
		by_lower.emplace(ascii_lower_copy(GET_NAME(i)), GET_NAME(i));
	}

	if(db) {
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
				<< " OR (oi.id IS NULL AND EXISTS ("
				   "SELECT 1 FROM clan_symbol cs WHERE cs.active=1 "
				   "AND cs.prince_toon_id="
				<< reg.prince_toon_id
				<< " AND (ci.item_number=cs.base_vnum OR ci.item_number=cs.vnum)"
				   "))) "
				   "ORDER BY t.name";
			if(mysql_query(h, sql.str().c_str()) == 0) {
				MYSQL_RES* res = mysql_store_result(h);
				if(res) {
					while(MYSQL_ROW row = mysql_fetch_row(res)) {
						if(!row[0]) {
							continue;
						}
						by_lower.emplace(ascii_lower_copy(row[0]), row[0]);
					}
					mysql_free_result(res);
				}
			}
		}
		catch(const odb::exception& e) {
			mudlog(LOG_SYSERR, "clan simboli: %s", e.what());
		}
	}

	collect_symbol_holders_from_rent(reg, by_lower);

	if(by_lower.empty()) {
		out << "  (nessuno)\n\r";
	}
	else {
		for(const auto& kv : by_lower) {
			append_presence_line(out, kv.second.c_str(),
								 find_pc_by_name_ci(kv.second.c_str()));
		}
	}
	send_to_char(out.str().c_str(), ch);
}

bool clan_assegna_to_vassal(struct char_data* prince, struct char_data* vassal) {
	if(!prince || !vassal) {
		return false;
	}
	struct char_data* const prince_pc = clan_pc_identity(prince);
	struct char_data* const vassal_pc = clan_pc_identity(vassal);
	if(!prince_pc || !vassal_pc) {
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", prince);
		return false;
	}
	ClanRegistry reg;
	if(!load_registry_by_prince(db, GET_NAME(prince_pc), reg) ||
	   reg.template_instance_id == 0 || reg.base_vnum == 0) {
		send_to_char("Il tuo clan non ha un simbolo registrato.\n\r", prince);
		return false;
	}
	if(!IS_VASSALLOOF(vassal, GET_NAME(prince_pc)) &&
	   !IS_VASSALLOOF(vassal_pc, GET_NAME(prince_pc))) {
		act("$N non e' tu$b vassall$b.", true, prince, nullptr, vassal, TO_CHAR);
		return false;
	}
	if(char_holds_clan_symbol(vassal, reg.prince_toon_id) ||
	   (vassal != vassal_pc &&
		char_holds_clan_symbol(vassal_pc, reg.prince_toon_id))) {
		act("$N ha gia' il simbolo del clan.", true, prince, nullptr, vassal,
			TO_CHAR);
		return false;
	}
	if(clan_symbol_char_holds_any(vassal) ||
	   (vassal != vassal_pc && clan_symbol_char_holds_any(vassal_pc))) {
		act("$N possiede gia' un altro simbolo del clan.", true, prince, nullptr,
			vassal, TO_CHAR);
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
	if(GET_NAME(vassal_pc)) {
		set_personal_owner(obj, GET_NAME(vassal_pc));
	}
	const unsigned long long nid = object_instance_persist(
		obj, static_cast<int>(reg.base_vnum), 0, prince_pc, true, kClanAssegnaActor);
	if(nid == 0) {
		extract_obj(obj);
		send_to_char("Salvataggio simbolo fallito.\n\r", prince);
		return false;
	}
	/* Il pezzo va sul corpo in gioco (anche poly). */
	obj_to_char(obj, vassal);
	clan_symbol_try_auto_wear(vassal, obj);
	act("Consegni a $N il simbolo del casato, segno della su$B fedelta'.",
		true, prince, nullptr, vassal, TO_CHAR);
	act("$n ti consegna il simbolo del casato: ormai ne sei degn$b.", true,
		prince, nullptr, vassal, TO_VICT);
	act("$n consegna a $N il simbolo del casato.", true, prince, nullptr,
		vassal, TO_NOTVICT);
	save_char(vassal_pc, AUTO_RENT, 0);
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
		send_to_char("Principe/clan non trovata in clan_symbol.\n\r", ch);
		return false;
	}
	const std::string msg =
		"Quota simboli di " + reg.prince_name + " impostata a " +
		std::to_string(reg.slots_max) + " (ora usati " +
		std::to_string(clan_symbol_slots_used(db, reg.prince_toon_id)) +
		").\n\r";
	send_to_char(msg.c_str(), ch);
	return true;
}

[[nodiscard]] int strip_held_clan_symbols(struct char_data* ch,
										  unsigned long long prince_id,
										  unsigned long long template_id,
										  struct char_data* actor) {
	if(!ch || prince_id == 0) {
		return 0;
	}
	int stripped = 0;
	for(int i = 0; i < MAX_WEAR; ++i) {
		struct obj_data* obj = ch->equipment[i];
		if(!is_clan_symbol_obj(obj, prince_id)) {
			continue;
		}
		destroy_clan_symbol_obj(obj, template_id, actor);
		++stripped;
	}
	strip_clan_symbols_recursive(ch->carrying, prince_id, template_id, actor,
								 &stripped);
	return stripped;
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

	const int stripped =
		strip_held_clan_symbols(ch, prince_id, template_id, ch);
	if(stripped > 0) {
		send_to_char(
			"Il simbolo del casato si dissolve tra le tue mani, ormai senza "
			"diritto di portarlo.\n\r",
			ch);
		struct char_data* prince = get_char_room_vis(ch, prince_name);
		if(prince != nullptr && prince != ch) {
			act("$n perde il diritto di portare il simbolo del tuo casato.",
				true, ch, nullptr, prince, TO_VICT);
			act("$n viene privat$b del simbolo del casato.", true, ch, nullptr,
				prince, TO_NOTVICT);
		}
		else {
			act("$n viene privat$b del simbolo del casato.", true, ch, nullptr,
				nullptr, TO_ROOM);
		}
		std::ostringstream msg;
		msg << "clan_symbol_strip: "
			<< (GET_NAME(ch) ? GET_NAME(ch) : "?") << " lost " << stripped
			<< " symbols of " << prince_name;
		mudlog(LOG_CHECK, "%s", msg.str().c_str());
	}
}

void show_not_in_clan(struct char_data* ch) {
	send_to_char("Non fai parte di nessun clan.\n\r", ch);
	if(ch != nullptr && clan_is_prince(ch)) {
		send_to_char(
			"Se vuoi creare un tuo clan digita clan associa <nome> "
			"con il tuo futuro vassallo presente in stanza.\n\r",
			ch);
	}
	if(ch != nullptr && clan_is_immortale(ch)) {
		send_to_char(
			"Uso (god):\n\r"
			"  clan vassalli <principe>   - lista vassalli\n\r"
			"  clan simboli <principe>    - lista simboli\n\r"
			"  clan quota <principe> [n]  - mostra/imposta quota (default 5)\n\r"
			"  clan togli <pg>            - toglie/distrugge il simbolo\n\r",
			ch);
	}
}

[[nodiscard]] bool prince_has_vassals(struct char_data* ch) {
	if(ch == nullptr || !GET_NAME(ch)) {
		return false;
	}
	const char* pname = GET_NAME(clan_pc_identity(ch));
	if(pname == nullptr) {
		return false;
	}
	for(struct char_data* i = character_list; i; i = i->next) {
		if(IS_PC(i) && IS_VASSALLOOF(i, pname)) {
			return true;
		}
	}
	DB* db = Sql::getMysql();
	if(db) {
		try {
			odb::connection_ptr cp(db->connection());
			auto& mc = static_cast<odb::mysql::connection&>(*cp);
			MYSQL* h = mc.handle();
			std::string esc;
			sql_escape(h, pname, esc);
			std::ostringstream sql;
			sql << "SELECT 1 FROM character_prefs cp "
				   "WHERE cp.pref_key='principe' AND LOWER(cp.pref_value)=LOWER('"
				<< esc << "') LIMIT 1";
			if(mysql_query(h, sql.str().c_str()) == 0) {
				MYSQL_RES* res = mysql_store_result(h);
				if(res) {
					const bool found = (mysql_fetch_row(res) != nullptr);
					mysql_free_result(res);
					if(found) {
						return true;
					}
				}
			}
		}
		catch(const odb::exception& e) {
			mudlog(LOG_SYSERR, "clan prince_has_vassals: %s", e.what());
		}
	}

	std::map<std::string, std::string> from_aux;
	collect_vassals_from_aux(pname, from_aux);
	return !from_aux.empty();
}

void show_clan_usage(struct char_data* ch) {
	struct char_data* const id = clan_pc_identity(ch);
	const bool leads = prince_has_vassals(ch) ||
					   (id != nullptr && id != ch && prince_has_vassals(id));
	const char* const princeOf =
		HAS_PRINCE(ch) && GET_PRINCE(ch)
			? GET_PRINCE(ch)
			: (id != nullptr && HAS_PRINCE(id) ? GET_PRINCE(id) : nullptr);

	if(princeOf != nullptr) {
		const std::string msg =
			std::string("Fai parte del clan di ") + princeOf + ".\n\r";
		send_to_char(msg.c_str(), ch);
	}
	else if(leads && GET_NAME(id ? id : ch)) {
		struct char_data* const who = id != nullptr ? id : ch;
		const bool female = (GET_SEX(who) == SEX_FEMALE);
		const std::string msg =
			std::string(female ? "Sei la principessa del clan di "
							   : "Sei il principe del clan di ") +
			GET_NAME(who) + ".\n\r";
		send_to_char(msg.c_str(), ch);
	}

	if(leads) {
		send_to_char(
			"Uso:\n\r"
			"  clan vassalli              - lista i tuoi vassalli\n\r"
			"  clan simboli               - chi ha i simboli del clan\n\r"
			"  clan assegna <nome>        - assegna un simbolo (stessa stanza)\n\r"
			"  clan ritira <nome>         - ritira/distrugge un simbolo (anche assente)\n\r"
			"  clan associa <nome>        - nomina un vassallo (stessa stanza)\n\r"
			"  clan ripudia <nome>        - bandisci (stanza / mondo / offline)\n\r"
			"                               o rinuncia al tuo principe\n\r"
			"  clan tell <messaggio>      - parla al clan (anche da polato)\n\r"
			"  ctell <messaggio>          - alias di clan tell\n\r",
			ch);
	}
	else if(princeOf != nullptr) {
		send_to_char(
			"Uso:\n\r"
			"  clan ripudia [nome]        - rinuncia al tuo principe\n\r"
			"  clan tell <messaggio>      - parla al clan (anche da polato)\n\r"
			"  ctell <messaggio>          - alias di clan tell\n\r",
			ch);
	}
	if(clan_is_immortale(ch)) {
		send_to_char(
			"  clan vassalli <principe>   - (god) lista vassalli\n\r"
			"  clan simboli <principe>    - (god) lista simboli\n\r"
			"  clan quota <principe> [n]  - (god) mostra/imposta quota (default 5)\n\r"
			"  clan togli <pg>            - (god) toglie/distrugge il simbolo\n\r",
			ch);
	}
}

[[nodiscard]] bool char_in_clan(struct char_data* ch) {
	if(ch == nullptr) {
		return false;
	}
	/* Immortali/dei possono far parte dei clan come chiunque: vassallo o capo. */
	if(HAS_PRINCE(ch) || prince_has_vassals(ch)) {
		return true;
	}
	struct char_data* const id = clan_pc_identity(ch);
	return id != nullptr && id != ch &&
		   (HAS_PRINCE(id) || prince_has_vassals(id));
}

[[nodiscard]] bool resolve_prince_target(struct char_data* ch,
										 std::string_view arg,
										 ClanRegistry& reg) {
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", ch);
		return false;
	}
	std::string name{arg};
	while(!name.empty() &&
		  (name.back() == ' ' || name.back() == '\r' || name.back() == '\n')) {
		name.pop_back();
	}
	if(name.empty()) {
		if(!clan_is_prince(ch) || !GET_NAME(clan_pc_identity(ch))) {
			send_to_char("Specifica il nome del principe.\n\r", ch);
			return false;
		}
		name = GET_NAME(clan_pc_identity(ch));
	}
	else if(!clan_is_immortale(ch) &&
			(!clan_is_prince(ch) || !GET_NAME(clan_pc_identity(ch)) ||
			 strcasecmp(GET_NAME(clan_pc_identity(ch)), name.c_str()) != 0)) {
		send_to_char("Puoi gestire solo il tuo clan.\n\r", ch);
		return false;
	}
	if(!load_registry_by_prince(db, name.c_str(), reg)) {
		send_to_char("Clan/principe non trovato in clan_symbol.\n\r", ch);
		return false;
	}
	return true;
}

void clan_clear_prince_link(struct char_data* ch) {
	if(ch == nullptr || GET_PRINCE(ch) == nullptr) {
		return;
	}
	const std::string princeName = GET_PRINCE(ch);
	clan_symbol_strip_from_char(ch, princeName.c_str());
	free(GET_PRINCE(ch));
	GET_PRINCE(ch) = nullptr;
}

void clan_set_prince_link(struct char_data* vassal, std::string_view princeName) {
	if(vassal == nullptr || princeName.empty()) {
		return;
	}
	clan_clear_prince_link(vassal);
	const std::string copy{princeName};
	GET_PRINCE(vassal) = strdup(copy.c_str());
}

[[nodiscard]] bool clan_combat_blocks_act(struct char_data* ch,
										  struct char_data* victim) {
	if(ch == nullptr) {
		return true;
	}
	if(victim != nullptr &&
	   (ch->specials.fighting != nullptr || victim->specials.fighting != nullptr)) {
		send_to_char("Pensate a combattere!\n\r", ch);
		return true;
	}
	return false;
}

void clan_ripudia_renounce_absent(struct char_data* ch,
								  const std::string& princeName) {
	if(ch == nullptr || princeName.empty() ||
	   !IS_VASSALLOOF(ch, princeName.c_str())) {
		act("Capisco la concitazione... ma non ne sei vassall$b!", true, ch,
			nullptr, nullptr, TO_CHAR);
		return;
	}

	clan_clear_prince_link(ch);
	{
		const std::string msg =
			"Rinunci a " + princeName + " (non in gioco).\n\r";
		send_to_char(msg.c_str(), ch);
	}
	act("Adesso sei liber$b.", true, ch, nullptr, nullptr, TO_CHAR);
	GET_EXP(ch) -= static_cast<int>(GET_EXP(ch) / 100 * 5);
	save_char(ch, AUTO_RENT, 0);
}

void clan_ripudia_vassal_breaks(struct char_data* ch, struct char_data* prince,
								bool same_room) {
	if(ch == nullptr || prince == nullptr) {
		return;
	}
	if(same_room) {
		act("Guardi negli occhi $N e rompi il tuo giuramento di fedelta'!", true,
			ch, nullptr, prince, TO_CHAR);
		act("$n rompe il suo giuramento di fedelta'!", true, ch, nullptr, prince,
			TO_VICT);
		act("$n rompe il suo giuramento di fedelta' a $N!", true, ch, nullptr,
			prince, TO_NOTVICT);
	}
	else {
		act("Rompi il tuo giuramento di fedelta' verso $N, ovunque si trovi.",
			true, ch, nullptr, prince, TO_CHAR);
		act("$n rompe il su$b giuramento di fedelta' verso di te.", true, ch,
			nullptr, prince, TO_VICT);
	}
	clan_clear_prince_link(ch);
	if(IS_POLY(ch) && ch->desc != nullptr && ch->desc->original != nullptr) {
		clan_clear_prince_link(ch->desc->original);
		save_char(ch->desc->original, AUTO_RENT, 0);
	}
	else {
		save_char(ch, AUTO_RENT, 0);
	}
}

void clan_ripudia_prince_expels(struct char_data* ch, struct char_data* vassal,
								bool same_room) {
	if(ch == nullptr || vassal == nullptr) {
		return;
	}
	if(same_room) {
		act("Fissi $N con uno sguardo severo e l$b bandisci dal tuo casato!",
			true, ch, nullptr, vassal, TO_CHAR);
		act("$n ti bandisce dal su$b casato!", true, ch, nullptr, vassal,
			TO_VICT);
		act("$n bandisce $N dal su$b casato!", true, ch, nullptr, vassal,
			TO_NOTVICT);
	}
	else {
		act("Bandisci $N dal tuo casato, ovunque si trovi.", true, ch, nullptr,
			vassal, TO_CHAR);
		act("$n ti bandisce dal su$b casato.", true, ch, nullptr, vassal,
			TO_VICT);
	}
	clan_clear_prince_link(vassal);
	if(IS_POLY(vassal) && vassal->desc != nullptr &&
	   vassal->desc->original != nullptr) {
		clan_clear_prince_link(vassal->desc->original);
		save_char(vassal->desc->original, AUTO_RENT, 0);
	}
	else {
		save_char(vassal, AUTO_RENT, 0);
	}
}

[[nodiscard]] bool db_is_vassal_of_prince(DB* db, const char* vassal_name,
										   const char* prince_name) {
	if(!db || !vassal_name || !*vassal_name || !prince_name || !*prince_name) {
		return false;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::string esc_v;
		std::string esc_p;
		sql_escape(h, vassal_name, esc_v);
		sql_escape(h, prince_name, esc_p);
		std::ostringstream sql;
		sql << "SELECT 1 FROM character_prefs cp "
			   "INNER JOIN toon t ON t.id = cp.toon_id "
			   "WHERE cp.pref_key='principe' AND LOWER(t.name)=LOWER('"
			<< esc_v << "') AND LOWER(cp.pref_value)=LOWER('" << esc_p
			<< "') LIMIT 1";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "clan ripudia vassal check: %s", mysql_error(h));
			return false;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return false;
		}
		const bool found = (mysql_fetch_row(res) != nullptr);
		mysql_free_result(res);
		return found;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan ripudia vassal check: %s", e.what());
		return false;
	}
}

[[nodiscard]] std::string db_canonical_toon_name(DB* db, const char* name) {
	if(!db || !name || !*name) {
		return name ? name : "";
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::string esc;
		sql_escape(h, name, esc);
		std::ostringstream sql;
		sql << "SELECT name FROM toon WHERE LOWER(name)=LOWER('" << esc
			<< "') LIMIT 1";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			return name;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return name;
		}
		std::string out = name;
		if(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(row[0] && *row[0]) {
				out = row[0];
			}
		}
		mysql_free_result(res);
		return out;
	}
	catch(const odb::exception&) {
		return name;
	}
}

/** Ritira simboli del clan dal rent MySQL di un PG offline.
 *  Ritorna quante righe inventory soft-delete. */
int clan_symbol_strip_from_offline(DB* db, unsigned long long vassal_toon_id,
								   unsigned long long prince_toon_id,
								   unsigned long long template_instance_id,
								   struct char_data* actor) {
	if(!db || vassal_toon_id == 0 || prince_toon_id == 0) {
		return 0;
	}
	std::vector<unsigned long long> inv_ids;
	std::vector<unsigned long long> instance_ids;
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT ci.id, ci.instance_id FROM character_inventory ci "
			   "LEFT JOIN object_instance oi ON oi.id = ci.instance_id "
			   "WHERE ci.toon_id = "
			<< vassal_toon_id
			<< " AND (ci.deleted = 0 OR ci.deleted IS NULL) "
			   "AND ci.value0 = "
			<< prince_toon_id
			<< " AND (oi.type_flag = " << static_cast<int>(ITEM_CLAN_SYMBOL)
			<< " OR (oi.id IS NULL AND EXISTS ("
			   "SELECT 1 FROM clan_symbol cs WHERE cs.active=1 "
			   "AND cs.prince_toon_id="
			<< prince_toon_id
			<< " AND (ci.item_number=cs.base_vnum OR ci.item_number=cs.vnum)"
			   ")))";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "clan strip offline select: %s", mysql_error(h));
			return 0;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return 0;
		}
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(row[0]) {
				inv_ids.push_back(parse_ull(row[0]));
			}
			if(row[1] && *row[1]) {
				const unsigned long long iid = parse_ull(row[1]);
				if(iid != 0) {
					instance_ids.push_back(iid);
				}
			}
		}
		mysql_free_result(res);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan strip offline select: %s", e.what());
		return 0;
	}

	for(const unsigned long long iid : instance_ids) {
		if(iid != template_instance_id) {
			object_instance_delete(iid, actor);
		}
	}

	if(inv_ids.empty()) {
		return 0;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		std::ostringstream ids;
		for(size_t i = 0; i < inv_ids.size(); ++i) {
			if(i) {
				ids << ',';
			}
			ids << inv_ids[i];
		}
		/* deleted_for e' ENUM: DEATH|RENT_EXPIRED|NUKE|TRAP|MANUAL|SCRAP */
		db->execute(("UPDATE character_inventory SET deleted = 1, "
					 "deleted_on = NOW(), deleted_for = 'MANUAL', "
					 "instance_id = NULL WHERE id IN (" +
					 ids.str() + ")")
						.c_str());
		db->execute(("UPDATE character_rent SET object_count = ("
					 "SELECT COUNT(*) FROM character_inventory WHERE toon_id = " +
					 std::to_string(vassal_toon_id) +
					 " AND (deleted = 0 OR deleted IS NULL)) WHERE toon_id = " +
					 std::to_string(vassal_toon_id))
						.c_str());
		t.commit();
		mudlog(LOG_CHECK, "clan_symbol_strip_offline: toon %llu lost %zu symbols",
			   static_cast<unsigned long long>(vassal_toon_id), inv_ids.size());
		return static_cast<int>(inv_ids.size());
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "clan strip offline update: %s", e.what());
		return 0;
	}
}

void clan_ripudia_prince_expels_absent(struct char_data* ch,
									   const std::string& vassal_name) {
	struct char_data* const prince_pc = clan_pc_identity(ch);
	if(ch == nullptr || vassal_name.empty() || prince_pc == nullptr ||
	   !GET_NAME(prince_pc)) {
		return;
	}
	DB* db = Sql::getMysql();
	const char* prince = GET_NAME(prince_pc);

	bool cleared_db = false;
	bool cleared_file = false;
	std::string canon = file_canonical_pc_name(vassal_name.c_str());
	if(canon.empty()) {
		canon = vassal_name;
	}

	ClanRegistry reg;
	bool have_reg = false;
	if(db) {
		have_reg = load_registry_by_prince(db, prince, reg);
		const std::string db_name = db_canonical_toon_name(db, vassal_name.c_str());
		if(!db_name.empty()) {
			canon = db_name;
		}
		if(db_is_vassal_of_prince(db, canon.c_str(), prince)) {
			const unsigned long long vassal_id = lookup_toon_id_ci(db, canon.c_str());
			if(vassal_id != 0) {
				try {
					odb::transaction t(db->begin());
					t.tracer(logTracer);
					odb::connection_ptr cp(db->connection());
					auto& mc = static_cast<odb::mysql::connection&>(*cp);
					MYSQL* h = mc.handle();
					std::string esc_p;
					sql_escape(h, prince, esc_p);
					std::ostringstream sql;
					sql << "DELETE FROM character_prefs WHERE toon_id = "
						<< vassal_id
						<< " AND pref_key = 'principe' AND LOWER(pref_value)=LOWER('"
						<< esc_p << "')";
					if(mysql_query(h, sql.str().c_str()) != 0) {
						mudlog(LOG_SYSERR, "clan ripudia offline prefs: %s",
							   mysql_error(h));
						t.rollback();
					}
					else {
						t.commit();
						cleared_db = true;
						unsigned long long prince_id = reg.prince_toon_id;
						unsigned long long template_id = reg.template_instance_id;
						if(!have_reg) {
							prince_id = lookup_toon_id_ci(db, prince);
						}
						clan_symbol_strip_from_offline(db, vassal_id, prince_id,
													   template_id, ch);
					}
				}
				catch(const odb::exception& e) {
					mudlog(LOG_SYSERR, "clan ripudia offline prefs: %s", e.what());
				}
			}
		}
	}

	if(!toon_is_migrated_by_name(canon.c_str()) &&
	   aux_is_vassal_of(canon.c_str(), prince)) {
		if(aux_clear_principe_if_matches(canon.c_str(), prince)) {
			cleared_file = true;
		}
		if(have_reg) {
			strip_clan_symbols_from_rent_file(canon.c_str(), reg);
		}
		else if(db) {
			/* Registry assente: prova comunque a caricare per strip rent. */
			if(load_registry_by_prince(db, prince, reg)) {
				strip_clan_symbols_from_rent_file(canon.c_str(), reg);
			}
		}
	}

	if(!cleared_db && !cleared_file) {
		act("Quell$b non e' tu$b vassall$b.", true, ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	std::string msg =
		"Bandisci " + canon + " dal tuo casato (non in gioco).\n\r";
	send_to_char(msg.c_str(), ch);
	const std::string logmsg =
		std::string("clan ripudia: ") + prince + " expels absent vassal " + canon +
		" (db=" + (cleared_db ? "yes" : "no") +
		" file=" + (cleared_file ? "yes" : "no") + ")";
	mudlog(LOG_PLAYERS, "%s", logmsg.c_str());
}

[[nodiscard]] long clan_associa_nomination_cost(struct char_data* ch,
												struct char_data* victim) {
	constexpr long kCostBase = 500000L;
	if(ch == nullptr || victim == nullptr) {
		return kCostBase;
	}
	return (17 - GET_RCHR(ch)) * 50000L + (GET_RCHR(victim) - 12) * 25000L +
		   kCostBase;
}

void clan_associa(struct char_data* ch, const char* arg) {
	if(ch == nullptr) {
		return;
	}
	struct char_data* const prince_pc = clan_pc_identity(ch);
	if(!clan_is_prince(ch) || prince_pc == nullptr || !GET_NAME(prince_pc)) {
		send_to_char("Presuntuosetto, eh?\n\r", ch);
		return;
	}

	const std::string target =
		chop_argument(arg, MAX_INPUT_LENGTH - 1, 0).first;
	struct char_data* victim =
		!target.empty() ? get_char_room_vis(ch, target.c_str()) : nullptr;

	if(victim == nullptr || (!IS_PC(victim))) {
		send_to_char("Ottima idea nominare dei vassalli..."
					 "ma almeno cerca di scrivere bene il loro nome!\n\r",
					 ch);
		return;
	}
	struct char_data* const vassal_pc = clan_pc_identity(victim);
	if(vassal_pc == nullptr) {
		send_to_char("Ottima idea nominare dei vassalli..."
					 "ma almeno cerca di scrivere bene il loro nome!\n\r",
					 ch);
		return;
	}
	if(clan_combat_blocks_act(ch, victim)) {
		return;
	}
	if(GetMaxLevel(vassal_pc) < VASSALLO) {
		act("$N e' troppo giovane per giurarti fedelta'.", true, ch, nullptr,
			victim, TO_CHAR);
		act("Sei troppo giovane per giurare fedelta' a $n.", true, ch, nullptr,
			victim, TO_VICT);
		return;
	}
	if(GET_PRINCE(vassal_pc) != nullptr || GET_PRINCE(victim) != nullptr) {
		struct char_data* linked =
			GET_PRINCE(vassal_pc) != nullptr ? vassal_pc : victim;
		if(IS_VASSALLOOF(linked, GET_NAME(prince_pc))) {
			act("$N e' gia' tu$b vassall$b!", true, ch, nullptr, victim, TO_CHAR);
			act("$n ha cercato di nominarti ANCORA su$b vassall$b!!", true, ch,
				nullptr, victim, TO_VICT);
		}
		else {
			act("$N ha' gia' giurato fedelta' a $T!", true, ch, nullptr,
				GET_PRINCE(linked), TO_CHAR);
		}
		return;
	}

	const long cost = clan_associa_nomination_cost(prince_pc, vassal_pc);
	if(GET_GOLD(prince_pc) < cost) {
		act("Ti costerebbe troppo...", true, ch, nullptr, nullptr, TO_CHAR);
		return;
	}

	GET_GOLD(prince_pc) -= cost;
	clan_set_prince_link(vassal_pc, GET_NAME(prince_pc));
	if(victim != vassal_pc) {
		clan_set_prince_link(victim, GET_NAME(prince_pc));
	}

	const std::string costMsg =
		"Il che ti costa " + std::to_string(cost) + " monete d'oro!";
	act("Nomini $N tu$B vassall$B.", true, ch, nullptr, victim, TO_CHAR);
	act(costMsg.c_str(), true, ch, nullptr, victim, TO_CHAR);
	act("Ti inginocchi e giuri fedelta' a $n.", true, ch, nullptr, victim,
		TO_VICT);
	act("$N si inginocchia e $n l$B nomina su$B vassall$B!", true, ch, nullptr,
		victim, TO_NOTVICT);
	save_char(prince_pc, AUTO_RENT, 0);
	save_char(vassal_pc, AUTO_RENT, 0);
}

void clan_ripudia(struct char_data* ch, const char* arg) {
	if(ch == nullptr) {
		return;
	}

	const std::string target =
		chop_argument(arg, MAX_INPUT_LENGTH - 1, 0).first;
	if(target.empty()) {
		send_to_char("Chi vuoi ripudiare?\n\r", ch);
		return;
	}

	/* Prima stessa stanza, poi ovunque online (messaggi diversi). */
	struct char_data* victim = get_char_room_vis(ch, target.c_str());
	const bool same_room = (victim != nullptr);
	if(victim == nullptr) {
		victim = find_pc_by_name_ci(target.c_str());
	}

	if(victim != nullptr) {
		if(clan_combat_blocks_act(ch, victim)) {
			return;
		}
		struct char_data* const self_pc = clan_pc_identity(ch);
		struct char_data* const vict_pc = clan_pc_identity(victim);
		const char* const self_name =
			self_pc != nullptr ? GET_NAME(self_pc) : GET_NAME(ch);
		const char* const vict_name =
			vict_pc != nullptr ? GET_NAME(vict_pc) : GET_NAME(victim);
		if(vict_name != nullptr && IS_VASSALLOOF(ch, vict_name)) {
			clan_ripudia_vassal_breaks(ch, victim, same_room);
			return;
		}
		if(self_name != nullptr &&
		   (IS_PRINCEOF(self_name, victim) ||
			(vict_pc != nullptr && vict_pc != victim &&
			 IS_PRINCEOF(self_name, vict_pc)))) {
			clan_ripudia_prince_expels(ch, victim, same_room);
			return;
		}
		act("Non e' un tu$b vassall$b, ne' il tuo principe.", true, ch, nullptr,
			nullptr, TO_CHAR);
		return;
	}

	/* Target offline: principe bandisce (MySQL e/o .aux), oppure vassallo
	 * rinuncia al principe. */
	DB* db = Sql::getMysql();
	struct char_data* const self_pc = clan_pc_identity(ch);
	const char* const self_name =
		self_pc != nullptr ? GET_NAME(self_pc) : nullptr;
	if(clan_is_prince(ch) && self_name &&
	   ((db && db_is_vassal_of_prince(db, target.c_str(), self_name)) ||
		aux_is_vassal_of(target.c_str(), self_name))) {
		clan_ripudia_prince_expels_absent(ch, target);
		return;
	}
	clan_ripudia_renounce_absent(ch, target);
}

[[nodiscard]] unsigned long long first_held_clan_symbol_prince_id(
	struct char_data* ch) {
	if(!ch) {
		return 0;
	}
	auto from_obj = [](struct obj_data* obj) -> unsigned long long {
		if(!is_clan_symbol_obj(obj, 0)) {
			return 0;
		}
		const int v = obj->obj_flags.value[0];
		return v > 0 ? static_cast<unsigned long long>(v) : 0;
	};
	for(int i = 0; i < MAX_WEAR; ++i) {
		if(const unsigned long long pid = from_obj(ch->equipment[i])) {
			return pid;
		}
	}
	for(struct obj_data* obj = ch->carrying; obj; obj = obj->next_content) {
		if(const unsigned long long pid = from_obj(obj)) {
			return pid;
		}
		for(struct obj_data* in = obj->contains; in; in = in->next_content) {
			if(const unsigned long long pid = from_obj(in)) {
				return pid;
			}
		}
	}
	return 0;
}

[[nodiscard]] unsigned long long db_first_clan_symbol_prince_id(
	DB* db, unsigned long long toon_id) {
	if(!db || toon_id == 0) {
		return 0;
	}
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT ci.value0 FROM character_inventory ci "
			   "LEFT JOIN object_instance oi ON oi.id = ci.instance_id "
			   "WHERE ci.toon_id = "
			<< toon_id
			<< " AND (ci.deleted = 0 OR ci.deleted IS NULL) "
			   "AND ci.value0 > 0 "
			   "AND (oi.type_flag = " << static_cast<int>(ITEM_CLAN_SYMBOL)
			<< " OR (oi.id IS NULL AND EXISTS ("
			   "SELECT 1 FROM clan_symbol cs WHERE cs.active=1 "
			   "AND cs.prince_toon_id=ci.value0 "
			   "AND (ci.item_number=cs.base_vnum OR ci.item_number=cs.vnum)"
			   "))) "
			   "ORDER BY ci.id LIMIT 1";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			return 0;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			return 0;
		}
		unsigned long long pid = 0;
		if(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(row[0]) {
				pid = parse_ull(row[0]);
			}
		}
		mysql_free_result(res);
		return pid;
	}
	catch(const odb::exception&) {
		return 0;
	}
}

[[nodiscard]] bool strip_symbol_offline_for_reg(DB* db, const std::string& canon,
												const ClanRegistry& reg,
												struct char_data* actor) {
	bool did = false;
	if(db) {
		const unsigned long long tid = lookup_toon_id_ci(db, canon.c_str());
		if(tid != 0 &&
		   clan_symbol_strip_from_offline(db, tid, reg.prince_toon_id,
										  reg.template_instance_id, actor) > 0) {
			did = true;
		}
	}
	if(!toon_is_migrated_by_name(canon.c_str()) &&
	   strip_clan_symbols_from_rent_file(canon.c_str(), reg)) {
		did = true;
	}
	return did;
}

void clan_ritira_announce(struct char_data* prince, struct char_data* target,
						  bool same_room) {
	if(same_room && target) {
		act("Ritiri il simbolo del clan a $N (viene distrutto).", true, prince,
			nullptr, target, TO_CHAR);
		act("$n ti ritira il simbolo del clan.", true, prince, nullptr, target,
			TO_VICT);
		act("$n ritira il simbolo del clan a $N.", true, prince, nullptr, target,
			TO_NOTVICT);
	}
	else if(target) {
		act("Ritiri il simbolo del clan a $N, ovunque si trovi (viene distrutto).",
			true, prince, nullptr, target, TO_CHAR);
		act("$n ti ritira il simbolo del clan.", true, prince, nullptr, target,
			TO_VICT);
	}
}

[[nodiscard]] bool is_member_of_prince_clan(DB* db, struct char_data* online,
											const char* name,
											const char* prince) {
	if(!prince || !*prince) {
		return false;
	}
	if(online != nullptr && GET_NAME(online)) {
		if(strcasecmp(GET_NAME(online), prince) == 0) {
			return true;
		}
		return IS_VASSALLOOF(online, prince);
	}
	if(!name || !*name) {
		return false;
	}
	if(strcasecmp(name, prince) == 0) {
		return true;
	}
	return (db && db_is_vassal_of_prince(db, name, prince)) ||
		   aux_is_vassal_of(name, prince);
}

void clan_ritira(struct char_data* ch, const char* arg) {
	if(ch == nullptr) {
		return;
	}
	struct char_data* const prince_pc = clan_pc_identity(ch);
	if(!clan_is_prince(ch) || prince_pc == nullptr || !GET_NAME(prince_pc)) {
		send_to_char("Solo un principe puo' ritirare il simbolo del clan.\n\r",
					 ch);
		return;
	}
	if(!char_in_clan(ch)) {
		show_not_in_clan(ch);
		return;
	}
	const std::string target =
		chop_argument(arg, MAX_INPUT_LENGTH - 1, 0).first;
	if(target.empty()) {
		send_to_char("A chi vuoi ritirare il simbolo?\n\r", ch);
		return;
	}

	DB* db = Sql::getMysql();
	ClanRegistry reg;
	if(!db || !load_registry_by_prince(db, GET_NAME(prince_pc), reg) ||
	   reg.prince_toon_id == 0) {
		send_to_char("Il tuo clan non ha un simbolo registrato.\n\r", ch);
		return;
	}

	struct char_data* victim = get_char_room_vis(ch, target.c_str());
	const bool same_room = (victim != nullptr);
	if(victim == nullptr) {
		victim = find_pc_by_name_ci(target.c_str());
	}

	if(victim != nullptr) {
		if(IS_NPC(victim) && !IS_POLY(victim)) {
			send_to_char("Non puoi ritirare il simbolo a un mob.\n\r", ch);
			return;
		}
		if(clan_combat_blocks_act(ch, victim)) {
			return;
		}
		struct char_data* const vict_pc = clan_pc_identity(victim);
		const char* const vict_name =
			vict_pc != nullptr ? GET_NAME(vict_pc) : GET_NAME(victim);
		if(!is_member_of_prince_clan(db, victim, vict_name,
									 GET_NAME(prince_pc)) &&
		   !(vict_pc != nullptr && vict_pc != victim &&
			 is_member_of_prince_clan(db, vict_pc, vict_name,
									  GET_NAME(prince_pc)))) {
			act("$N non fa parte del tuo clan.", true, ch, nullptr, victim,
				TO_CHAR);
			return;
		}
		if(!char_holds_clan_symbol(victim, reg.prince_toon_id) &&
		   !(vict_pc != nullptr && vict_pc != victim &&
			 char_holds_clan_symbol(vict_pc, reg.prince_toon_id))) {
			act("$N non ha il simbolo del tuo clan.", true, ch, nullptr, victim,
				TO_CHAR);
			return;
		}
		int n = strip_held_clan_symbols(victim, reg.prince_toon_id,
										reg.template_instance_id, ch);
		if(n <= 0 && vict_pc != nullptr && vict_pc != victim) {
			n = strip_held_clan_symbols(vict_pc, reg.prince_toon_id,
										reg.template_instance_id, ch);
		}
		if(n <= 0) {
			act("$N non ha il simbolo del tuo clan.", true, ch, nullptr, victim,
				TO_CHAR);
			return;
		}
		clan_ritira_announce(ch, victim, same_room);
		save_char(vict_pc != nullptr ? vict_pc : victim, AUTO_RENT, 0);
		{
			const std::string logmsg =
				std::string("clan ritira: ") +
				(GET_NAME(prince_pc) ? GET_NAME(prince_pc) : "?") +
				" strips symbol from " +
				(vict_name ? vict_name : "?") +
				(same_room ? " (room)" : " (world)");
			mudlog(LOG_PLAYERS, "%s", logmsg.c_str());
		}
		return;
	}

	std::string canon = db_canonical_toon_name(db, target.c_str());
	if(canon.empty()) {
		canon = file_canonical_pc_name(target.c_str());
	}
	if(canon.empty()) {
		canon = target;
	}

	if(!is_member_of_prince_clan(db, nullptr, canon.c_str(),
								 GET_NAME(prince_pc))) {
		send_to_char("Non fa parte del tuo clan.\n\r", ch);
		return;
	}
	if(!strip_symbol_offline_for_reg(db, canon, reg, ch)) {
		send_to_char("Non ha il simbolo del tuo clan.\n\r", ch);
		return;
	}
	{
		const std::string msg =
			"Ritiri il simbolo del clan a " + canon +
			" (non in gioco; viene distrutto).\n\r";
		send_to_char(msg.c_str(), ch);
	}
	{
		const std::string logmsg = std::string("clan ritira: ") +
								   (GET_NAME(prince_pc) ? GET_NAME(prince_pc) : "?") +
								   " strips symbol from absent " + canon;
		mudlog(LOG_PLAYERS, "%s", logmsg.c_str());
	}
}

void clan_togli_announce(struct char_data* god, struct char_data* target,
						 bool same_room) {
	if(same_room && target) {
		act("Togli e distruggi il simbolo del clan di $N.", true, god, nullptr,
			target, TO_CHAR);
		act("$n ti toglie il simbolo del clan.", true, god, nullptr, target,
			TO_VICT);
		act("$n toglie il simbolo del clan a $N.", true, god, nullptr, target,
			TO_NOTVICT);
	}
	else if(target) {
		act("Togli e distruggi il simbolo del clan di $N, ovunque si trovi.",
			true, god, nullptr, target, TO_CHAR);
		act("$n ti toglie il simbolo del clan.", true, god, nullptr, target,
			TO_VICT);
	}
}

void clan_togli(struct char_data* ch, const char* arg) {
	if(ch == nullptr) {
		return;
	}
	if(!clan_is_immortale(ch)) {
		send_to_char("Solo gli immortali possono usare questo comando.\n\r", ch);
		return;
	}
	const std::string target =
		chop_argument(arg, MAX_INPUT_LENGTH - 1, 0).first;
	if(target.empty()) {
		send_to_char("Uso: clan togli <personaggio>\n\r", ch);
		return;
	}

	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", ch);
		return;
	}

	struct char_data* victim = get_char_room_vis(ch, target.c_str());
	const bool same_room = (victim != nullptr);
	if(victim == nullptr) {
		victim = find_pc_by_name_ci(target.c_str());
	}

	if(victim != nullptr) {
		if(IS_NPC(victim) && !IS_POLY(victim)) {
			send_to_char("Non e' un personaggio giocante.\n\r", ch);
			return;
		}
		struct char_data* const vict_pc = clan_pc_identity(victim);
		unsigned long long pid = first_held_clan_symbol_prince_id(victim);
		if(pid == 0 && vict_pc != nullptr && vict_pc != victim) {
			pid = first_held_clan_symbol_prince_id(vict_pc);
		}
		if(pid == 0) {
			act("$N non ha un simbolo del clan.", true, ch, nullptr, victim,
				TO_CHAR);
			return;
		}
		ClanRegistry reg;
		if(!load_registry_by_prince_toon_id(db, pid, reg)) {
			reg.prince_toon_id = pid;
			reg.prince_name = lookup_toon_name_by_id(db, pid);
			if(reg.prince_name.empty()) {
				reg.prince_name = "?";
			}
		}
		int n = strip_held_clan_symbols(victim, reg.prince_toon_id,
										reg.template_instance_id, ch);
		if(n <= 0 && vict_pc != nullptr && vict_pc != victim) {
			n = strip_held_clan_symbols(vict_pc, reg.prince_toon_id,
										reg.template_instance_id, ch);
		}
		if(n <= 0) {
			act("$N non ha un simbolo del clan.", true, ch, nullptr, victim,
				TO_CHAR);
			return;
		}
		clan_togli_announce(ch, victim, same_room);
		save_char(vict_pc != nullptr ? vict_pc : victim, AUTO_RENT, 0);
		{
			const std::string logmsg =
				std::string("clan togli: ") +
				(GET_NAME(ch) ? GET_NAME(ch) : "?") + " destroys symbol on " +
				(GET_NAME(victim) ? GET_NAME(victim) : "?") + " (clan " +
				reg.prince_name + (same_room ? ", room)" : ", world)");
			mudlog(LOG_PLAYERS, "%s", logmsg.c_str());
		}
		return;
	}

	std::string canon = db_canonical_toon_name(db, target.c_str());
	if(canon.empty()) {
		canon = file_canonical_pc_name(target.c_str());
	}
	if(canon.empty()) {
		canon = target;
	}
	const unsigned long long toon_id = lookup_toon_id_ci(db, canon.c_str());
	const unsigned long long pid =
		toon_id != 0 ? db_first_clan_symbol_prince_id(db, toon_id) : 0;
	if(pid == 0) {
		send_to_char("Non ha un simbolo del clan (o non esiste).\n\r", ch);
		return;
	}

	ClanRegistry reg;
	if(!load_registry_by_prince_toon_id(db, pid, reg)) {
		reg.prince_toon_id = pid;
		reg.prince_name = lookup_toon_name_by_id(db, pid);
		if(reg.prince_name.empty()) {
			reg.prince_name = "?";
		}
	}
	if(!strip_symbol_offline_for_reg(db, canon, reg, ch)) {
		send_to_char("Non ha un simbolo del clan (o non esiste).\n\r", ch);
		return;
	}
	{
		const std::string msg =
			"Togli e distruggi il simbolo del clan di " + canon +
			" (non in gioco).\n\r";
		send_to_char(msg.c_str(), ch);
	}
	{
		const std::string logmsg =
			std::string("clan togli: ") +
			(GET_NAME(ch) ? GET_NAME(ch) : "?") + " destroys symbol on absent " +
			canon + " (clan " + reg.prince_name + ")";
		mudlog(LOG_PLAYERS, "%s", logmsg.c_str());
	}
}

ACTION_FUNC(do_clan) {
	if(ch == nullptr) {
		return;
	}
	/* Poly OK: si opera via clan_pc_identity (IS_PRINCE/IS_IMMORTALE sul PC). */
	if(IS_NPC(ch) && !IS_POLY(ch)) {
		return;
	}
	(void)cmd;

	const auto [cmdtok, rest] =
		chop_argument(arg, MAX_INPUT_LENGTH - 1, MAX_INPUT_LENGTH - 1);

	if(cmdtok.empty()) {
		if(!char_in_clan(ch) && !char_in_clan(clan_pc_identity(ch))) {
			show_not_in_clan(ch);
			return;
		}
		show_clan_usage(ch);
		return;
	}

	if(is_abbrev(cmdtok.c_str(), "tell") ||
	   is_abbrev(cmdtok.c_str(), "ctell")) {
		do_ctell(ch, rest.c_str(), CMD_CTELL);
		return;
	}
	if(is_abbrev(cmdtok.c_str(), "associa") ||
	   is_abbrev(cmdtok.c_str(), "associate")) {
		clan_associa(ch, rest.c_str());
		return;
	}
	if(is_abbrev(cmdtok.c_str(), "ripudia")) {
		clan_ripudia(ch, rest.c_str());
		return;
	}
	if(is_abbrev(cmdtok.c_str(), "ritira")) {
		clan_ritira(ch, rest.c_str());
		return;
	}
	if(is_abbrev(cmdtok.c_str(), "togli")) {
		clan_togli(ch, rest.c_str());
		return;
	}

	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("MySQL non disponibile.\n\r", ch);
		return;
	}

	const std::string argtok =
		chop_argument(rest.c_str(), MAX_INPUT_LENGTH - 1, 0).first;

	if(is_abbrev(cmdtok.c_str(), "vassalli")) {
		if(!clan_is_prince(ch) && !clan_is_immortale(ch)) {
			send_to_char("Solo i principi possono usare questo comando.\n\r", ch);
			return;
		}
		if(!clan_is_immortale(ch) && !char_in_clan(ch) &&
		   !char_in_clan(clan_pc_identity(ch))) {
			show_not_in_clan(ch);
			return;
		}
		ClanRegistry reg;
		if(!resolve_prince_target(ch, argtok, reg)) {
			return;
		}
		list_vassals(ch, reg);
		return;
	}
	if(is_abbrev(cmdtok.c_str(), "simboli")) {
		if(!clan_is_prince(ch) && !clan_is_immortale(ch)) {
			send_to_char("Solo i principi possono usare questo comando.\n\r", ch);
			return;
		}
		if(!clan_is_immortale(ch) && !char_in_clan(ch) &&
		   !char_in_clan(clan_pc_identity(ch))) {
			show_not_in_clan(ch);
			return;
		}
		ClanRegistry reg;
		if(!resolve_prince_target(ch, argtok, reg)) {
			return;
		}
		list_symbol_holders(ch, reg);
		return;
	}
	if(is_abbrev(cmdtok.c_str(), "assegna")) {
		if(!clan_is_prince(ch)) {
			send_to_char("Solo un principe puo' assegnare il simbolo.\n\r", ch);
			return;
		}
		if(!char_in_clan(ch) && !char_in_clan(clan_pc_identity(ch))) {
			show_not_in_clan(ch);
			return;
		}
		if(argtok.empty()) {
			send_to_char("A chi vuoi assegnare il simbolo?\n\r", ch);
			return;
		}
		struct char_data* vict = get_char_room_vis(ch, argtok.c_str());
		if(!vict || (!IS_PC(vict))) {
			send_to_char("Non e' qui.\n\r", ch);
			return;
		}
		clan_assegna_to_vassal(ch, vict);
		return;
	}
	if(is_abbrev(cmdtok.c_str(), "quota")) {
		if(!clan_is_immortale(ch)) {
			send_to_char("Solo gli immortali possono gestire il numero dei simboli.\n\r", ch);
			return;
		}
		const auto [prince, numtok] =
			chop_argument(rest.c_str(), MAX_INPUT_LENGTH - 1, MAX_INPUT_LENGTH - 1);
		if(prince.empty()) {
			send_to_char("Uso: clan quota <principe> [n]\n\r", ch);
			return;
		}
		ClanRegistry reg;
		if(!load_registry_by_prince(db, prince.c_str(), reg)) {
			send_to_char("Clan/principe non trovato in clan_symbol.\n\r", ch);
			return;
		}
		if(numtok.empty()) {
			const std::string msg =
				"Quota di " + reg.prince_name + ": " +
				std::to_string(reg.slots_max) + " (usati " +
				std::to_string(clan_symbol_slots_used(db, reg.prince_toon_id)) +
				").\n\r";
			send_to_char(msg.c_str(), ch);
			return;
		}
		unsigned slots = 0;
		if(!parse_unsigned(numtok, slots)) {
			send_to_char("Quota non valida (1-50).\n\r", ch);
			return;
		}
		set_clan_quota(ch, prince.c_str(), slots);
		return;
	}

	if(!char_in_clan(ch) && !char_in_clan(clan_pc_identity(ch))) {
		show_not_in_clan(ch);
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

