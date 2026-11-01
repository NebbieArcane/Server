/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  General include ************************************/
#include "config.hpp"
#include "flags.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "utils.hpp"
/***************************  Local    include ************************************/
#include "inventory_overflow.hpp"
#include "achievements.hpp"
#include "act.other.hpp"
#include "clan_symbol.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "object_instance.hpp"
#include "procarea.hpp"
#include "spell_parser.hpp"
#include "spells.hpp"
#include "Sql.hpp"
#include "toon_migration.hpp"
#include "utility.hpp"
#include "autoenums.hpp"

#if USE_MYSQL
#include <mysql/mysql.h>
#include <odb/database.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/transaction.hxx>
#endif

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Alarmud {

#if USE_MYSQL
#include "overflow_claim_listino.inc"
#endif

#if !USE_MYSQL

void inventory_plan_overflow(struct char_data* /*ch*/, int /*need*/,
							 std::vector<struct obj_data*>& excess_roots) {
	excess_roots.clear();
}

bool inventory_overflow_begin_persist(struct char_data* /*ch*/, InventoryOverflowHold& hold) {
	hold = {};
	return false;
}

void inventory_overflow_finish_persist(InventoryOverflowHold& hold, bool /*reattach*/) {
	hold = {};
}

void inventory_overflow_checkpoint_pulse(unsigned long /*now_pulse*/) {}

void inventory_overflow_expire_stale() {}

bool inventory_overflow_list(struct char_data* /*actor*/,
							 std::vector<InventoryOverflowListItem>& out,
							 const char* /*owner_name*/,
							 std::string* resolved_owner_out) {
	out.clear();
	if(resolved_owner_out) {
		resolved_owner_out->clear();
	}
	return false;
}

bool inventory_overflow_list_owners(struct char_data* /*actor*/,
								   std::vector<InventoryOverflowOwnerCount>& out) {
	out.clear();
	return false;
}

long inventory_overflow_claim_price(struct char_data* /*actor*/,
								   unsigned long long /*overflow_id*/,
								   const char* /*owner_name*/) {
	return 0;
}

bool inventory_overflow_claim(struct char_data* /*actor*/,
							  const std::vector<unsigned long long>& /*overflow_ids*/,
							  const char* /*owner_name*/) {
	return false;
}

#else /* USE_MYSQL */

namespace {

/** Fee minimo di ritiro per pezzo/lotto. */
constexpr long kOverflowClaimFeeMin = 500;
/**
 * Fee = max(min, valore_stimato / divisor).
 * Valore: cost proto (eq) oppure stima spell (pozioni/pergamene/bacchette).
 */
constexpr long kOverflowClaimFeeDivisor = 5;
/** Moltiplicatore fee se proto cost >= LIM_ITEM_COST_MIN (limited/rare). */
constexpr long kOverflowClaimLimitedMult = 3;

std::string ov_sql_escape(const char* s) {
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

std::string ov_sql_literal(const char* s) {
	if(!s || !*s) {
		return "''";
	}
	return "'" + ov_sql_escape(s) + "'";
}

bool ov_mysql_query(DB* db, const std::string& sql, MYSQL_RES*& out_res) {
	out_res = nullptr;
	if(!db) {
		return false;
	}
	const bool in_tx = odb::transaction::has_current();
	try {
		odb::connection_ptr owned;
		odb::connection& conn =
			in_tx ? odb::transaction::current().connection() : *(owned = db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(conn);
		MYSQL* h = mc.handle();
		if(mysql_query(h, sql.c_str()) != 0) {
			mudlog(LOG_SYSERR, "inventory_overflow query: %s", mysql_error(h));
			return false;
		}
		out_res = mysql_store_result(h);
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow query: %s", e.what());
		return false;
	}
}

unsigned long long ov_mysql_scalar(DB* db, const std::string& sql) {
	MYSQL_RES* res = nullptr;
	if(!ov_mysql_query(db, sql, res) || !res) {
		if(res) {
			mysql_free_result(res);
		}
		return 0;
	}
	unsigned long long n = 0;
	if(MYSQL_ROW row = mysql_fetch_row(res)) {
		if(row[0]) {
			n = std::strtoull(row[0], nullptr, 10);
		}
	}
	mysql_free_result(res);
	return n;
}

struct char_data* overflow_resolve_toon_pc(struct char_data* ch) {
	if(!ch) {
		return nullptr;
	}
	if(struct char_data* pc = save_char_resolve_pc(ch)) {
		return pc;
	}
	if(IS_POLY(ch)) {
		return find_poly_original_pc(ch);
	}
	return IS_PC(ch) ? ch : nullptr;
}

bool overflow_obj_is_edit(const struct obj_data* obj) {
	if(!obj) {
		return false;
	}
	if(obj->db_instance_id != 0) {
		return true;
	}
	return IS_OBJ_STAT2(obj, ITEM2_EDIT) != 0;
}

bool overflow_obj_is_quest(const struct obj_data* obj) {
	return obj && IS_OBJ_STAT2(obj, ITEM2_QUEST);
}

bool overflow_obj_is_personal(const struct obj_data* obj) {
	return obj && IS_OBJ_STAT2(obj, ITEM2_PERSONAL);
}

bool overflow_obj_is_has_gems(const struct obj_data* obj) {
	return obj && IS_OBJ_STAT2(obj, ITEM2_INSERT);
}

bool overflow_obj_is_procarea(const struct obj_data* obj) {
	return obj && procarea_obj_is_reward(obj);
}

/** Priorita' di tenuta nei 200: spillati dopo la roba generica (mai immuni). */
bool overflow_obj_is_protected(const struct obj_data* obj) {
	return overflow_obj_is_edit(obj) || overflow_obj_is_quest(obj) ||
		   overflow_obj_is_personal(obj) || overflow_obj_is_has_gems(obj) ||
		   overflow_obj_is_procarea(obj);
}

std::vector<struct obj_data*> overflow_list_siblings(struct obj_data* head) {
	std::vector<struct obj_data*> out;
	for(struct obj_data* o = head; o; o = o->next_content) {
		out.push_back(o);
	}
	return out;
}

int overflow_count_unmarked(struct obj_data* obj,
							const std::unordered_set<struct obj_data*>& marked) {
	if(!obj || marked.count(obj) != 0) {
		return 0;
	}
	int total = 1;
	for(struct obj_data* c = obj->contains; c; c = c->next_content) {
		total += overflow_count_unmarked(c, marked);
	}
	return total;
}

void overflow_mark_tree(struct obj_data* obj, std::unordered_set<struct obj_data*>& marked) {
	if(!obj || marked.count(obj) != 0) {
		return;
	}
	marked.insert(obj);
	for(struct obj_data* c = obj->contains; c; c = c->next_content) {
		overflow_mark_tree(c, marked);
	}
}

void overflow_collect_from_list(struct obj_data* head, int& need,
								std::vector<struct obj_data*>& roots,
								std::unordered_set<struct obj_data*>& marked,
								bool allow_protected) {
	if(need <= 0 || !head) {
		return;
	}
	const std::vector<struct obj_data*> items = overflow_list_siblings(head);
	for(auto it = items.rbegin(); it != items.rend() && need > 0; ++it) {
		struct obj_data* o = *it;
		if(!o || marked.count(o) != 0) {
			continue;
		}
		if(o->contains) {
			overflow_collect_from_list(o->contains, need, roots, marked, allow_protected);
		}
		if(need <= 0) {
			return;
		}
		if(marked.count(o) != 0) {
			continue;
		}
		if(!allow_protected && overflow_obj_is_protected(o)) {
			continue;
		}
		const int n = overflow_count_unmarked(o, marked);
		if(n <= 0) {
			continue;
		}
		roots.push_back(o);
		overflow_mark_tree(o, marked);
		need -= n;
	}
}

/**
 * Riprende i pezzi gia' in overflow (db_overflow_id != 0) cosi' il piano e'
 * stabile dopo finish(reattach): stesso set → sync incrementale skipped=N.
 * Prende l'intero sotto-albero solo se entra in `need` (niente spill extra).
 */
void overflow_collect_tagged(struct obj_data* head, int& need,
							 std::vector<struct obj_data*>& roots,
							 std::unordered_set<struct obj_data*>& marked,
							 bool allow_protected) {
	if(need <= 0 || !head) {
		return;
	}
	const std::vector<struct obj_data*> items = overflow_list_siblings(head);
	for(auto it = items.rbegin(); it != items.rend() && need > 0; ++it) {
		struct obj_data* o = *it;
		if(!o || marked.count(o) != 0) {
			continue;
		}
		if(o->db_overflow_id != 0 && (allow_protected || !overflow_obj_is_protected(o)) &&
		   !(o->in_obj && o->in_obj->db_overflow_id != 0)) {
			const int n = overflow_count_unmarked(o, marked);
			if(n > 0 && n <= need) {
				roots.push_back(o);
				overflow_mark_tree(o, marked);
				need -= n;
				continue;
			}
		}
		if(o->contains) {
			overflow_collect_tagged(o->contains, need, roots, marked, allow_protected);
		}
	}
}

} // namespace (anon)

void inventory_plan_overflow(struct char_data* ch, int need,
							 std::vector<struct obj_data*>& roots) {
	if(!ch || need <= 0) {
		return;
	}
	std::unordered_set<struct obj_data*> marked;
	/* 0a: gia' in overflow, non protetti (stabilita' tra giri). */
	overflow_collect_tagged(ch->carrying, need, roots, marked, false);
	/* 1: carrying non protetti, contenuti dal fondo. */
	overflow_collect_from_list(ch->carrying, need, roots, marked, false);
	/* 0b: gia' in overflow, protetti. */
	overflow_collect_tagged(ch->carrying, need, roots, marked, true);
	/* 2: carrying protetti. */
	overflow_collect_from_list(ch->carrying, need, roots, marked, true);
	/* 3: contenuti di container wearati; prima i gia' taggati, poi dal fondo. */
	for(int i = 0; i < MAX_WEAR && need > 0; ++i) {
		struct obj_data* worn = ch->equipment[i];
		if(!worn || !worn->contains) {
			continue;
		}
		overflow_collect_tagged(worn->contains, need, roots, marked, true);
		overflow_collect_from_list(worn->contains, need, roots, marked, true);
	}
	/* Ordine stabile: gia' in DB per id, nuovi in coda → list_index invariato. */
	std::stable_sort(roots.begin(), roots.end(),
					 [](const struct obj_data* a, const struct obj_data* b) {
						 const unsigned long long ka =
							 a->db_overflow_id != 0 ? a->db_overflow_id : ~0ULL;
						 const unsigned long long kb =
							 b->db_overflow_id != 0 ? b->db_overflow_id : ~0ULL;
						 return ka < kb;
					 });
	if(need > 0) {
		mudlog(LOG_SYSERR,
			   "inventory_overflow: still need %d after split for %s (worn-heavy?)",
			   need, GET_NAME(ch) ? GET_NAME(ch) : "?");
	}
}

namespace {

int overflow_item_vnum(struct obj_data* obj) {
	if(!obj) {
		return 0;
	}
	if(obj->db_instance_id != 0) {
		const int base = object_instance_resolve_base_vnum(obj);
		if(base > 0) {
			return base;
		}
	}
	if(obj->item_number >= 0) {
		return obj_index[obj->item_number].iVNum;
	}
	return 0;
}

struct OvFlatItem {
	struct obj_data* obj = nullptr;
	struct obj_data* parent = nullptr;
	int list_index = 0;
	int depth = 0;
};

struct OvDbSnap {
	int list_index = 0;
	int item_number = 0;
	int value0 = 0;
	int value1 = 0;
	int value2 = 0;
	int value3 = 0;
	int extra_flags = 0;
	int extra_flags2 = 0;
	int weight = 0;
	int timer = 0;
	unsigned int bitvector = 0;
	std::string obj_name;
	std::string short_desc;
	std::string description;
	int wear_pos = 0;
	int depth = 0;
	unsigned long long parent_id = 0;
	unsigned long long instance_id = 0;
};

void overflow_flatten_tree(struct obj_data* obj, struct obj_data* parent, int depth,
						   int& list_index,
						   const std::unordered_set<struct obj_data*>& separate_roots,
						   std::vector<OvFlatItem>& out) {
	if(!obj) {
		return;
	}
	out.push_back(OvFlatItem{obj, parent, list_index++, depth});
	for(struct obj_data* c = obj->contains; c; c = c->next_content) {
		if(separate_roots.count(c) != 0) {
			continue;
		}
		overflow_flatten_tree(c, obj, depth + 1, list_index, separate_roots, out);
	}
}

void overflow_write_affects_tx(DB* db, unsigned long long overflow_id, struct obj_data* obj) {
	db->execute(("DELETE FROM character_inventory_overflow_affect WHERE overflow_id = " +
				 std::to_string(overflow_id))
					.c_str());
	if(!obj) {
		return;
	}
	for(int a = 0; a < MAX_OBJ_AFFECT; ++a) {
		const obj_affected_type& oa = obj->affected[a];
		if(oa.location == 0 && oa.modifier == 0) {
			continue;
		}
		std::ostringstream aff;
		aff << "INSERT INTO character_inventory_overflow_affect "
			   "(overflow_id, affect_slot, location, modifier) VALUES ("
			<< overflow_id << ',' << a << ',' << static_cast<int>(oa.location) << ','
			<< static_cast<int>(oa.modifier) << ')';
		db->execute(aff.str().c_str());
	}
}

unsigned long long overflow_insert_one_tx(DB* db, const std::string& toon_id,
										  struct obj_data* obj,
										  unsigned long long parent_overflow_id,
										  int list_index, int depth) {
	if(!obj) {
		return 0;
	}
	const unsigned long long iid = obj->db_instance_id;
	const int wearpos = obj->equipped_by ? static_cast<int>(obj->eq_pos) + 1 : 0;
	const int vnum = overflow_item_vnum(obj);

	std::ostringstream ins;
	ins << "INSERT INTO character_inventory_overflow ("
		   "toon_id, list_index, item_number, value0, value1, value2, value3, "
		   "extra_flags, extra_flags2, weight, timer, bitvector, obj_name, short_desc, "
		   "description, wear_pos, depth, parent_overflow_id, instance_id, created_on, "
		   "deleted, deleted_on, deleted_for) VALUES ("
		<< toon_id << ',' << list_index << ',' << vnum << ','
		<< obj->obj_flags.value[0] << ',' << obj->obj_flags.value[1] << ','
		<< obj->obj_flags.value[2] << ',' << obj->obj_flags.value[3] << ','
		<< static_cast<int>(obj->obj_flags.extra_flags) << ','
		<< static_cast<int>(obj->obj_flags.extra_flags2) << ','
		<< obj->obj_flags.weight << ',' << obj->obj_flags.timer << ','
		<< static_cast<unsigned int>(obj->obj_flags.bitvector) << ','
		<< ov_sql_literal(obj->name) << ','
		<< ov_sql_literal(obj->short_description) << ','
		<< ov_sql_literal(obj->description) << ',' << wearpos << ',' << depth << ',';
	if(parent_overflow_id != 0) {
		ins << parent_overflow_id;
	}
	else {
		ins << "NULL";
	}
	ins << ',';
	if(iid != 0) {
		ins << iid;
	}
	else {
		ins << "NULL";
	}
	ins << ",NOW(),0,NULL,NULL)";
	db->execute(ins.str().c_str());

	const unsigned long long id = ov_mysql_scalar(db, "SELECT LAST_INSERT_ID()");
	if(id == 0) {
		mudlog(LOG_SYSERR, "inventory_overflow: LAST_INSERT_ID=0 for toon %s vnum %d",
			   toon_id.c_str(), vnum);
		return 0;
	}
	overflow_write_affects_tx(db, id, obj);
	obj->db_overflow_id = id;
	return id;
}

void overflow_update_one_tx(DB* db, const std::string& toon_id, unsigned long long id,
							struct obj_data* obj, unsigned long long parent_overflow_id,
							int list_index, int depth) {
	if(!obj || id == 0) {
		return;
	}
	const unsigned long long iid = obj->db_instance_id;
	const int wearpos = obj->equipped_by ? static_cast<int>(obj->eq_pos) + 1 : 0;
	const int vnum = overflow_item_vnum(obj);
	std::ostringstream sql;
	sql << "UPDATE character_inventory_overflow SET list_index=" << list_index
		<< ",item_number=" << vnum << ",value0=" << obj->obj_flags.value[0]
		<< ",value1=" << obj->obj_flags.value[1] << ",value2=" << obj->obj_flags.value[2]
		<< ",value3=" << obj->obj_flags.value[3]
		<< ",extra_flags=" << static_cast<int>(obj->obj_flags.extra_flags)
		<< ",extra_flags2=" << static_cast<int>(obj->obj_flags.extra_flags2)
		<< ",weight=" << obj->obj_flags.weight << ",timer=" << obj->obj_flags.timer
		<< ",bitvector=" << static_cast<unsigned int>(obj->obj_flags.bitvector)
		<< ",obj_name=" << ov_sql_literal(obj->name)
		<< ",short_desc=" << ov_sql_literal(obj->short_description)
		<< ",description=" << ov_sql_literal(obj->description) << ",wear_pos=" << wearpos
		<< ",depth=" << depth << ",parent_overflow_id=";
	if(parent_overflow_id != 0) {
		sql << parent_overflow_id;
	}
	else {
		sql << "NULL";
	}
	sql << ",instance_id=";
	if(iid != 0) {
		sql << iid;
	}
	else {
		sql << "NULL";
	}
	sql << " WHERE id=" << id << " AND toon_id=" << toon_id;
	db->execute(sql.str().c_str());
	overflow_write_affects_tx(db, id, obj);
}

bool overflow_snap_matches_obj(const OvDbSnap& snap, struct obj_data* obj, int list_index,
							   int depth, unsigned long long parent_id) {
	if(!obj) {
		return false;
	}
	const int wearpos = obj->equipped_by ? static_cast<int>(obj->eq_pos) + 1 : 0;
	if(snap.list_index != list_index || snap.depth != depth || snap.parent_id != parent_id ||
	   snap.item_number != overflow_item_vnum(obj) || snap.wear_pos != wearpos ||
	   snap.instance_id != obj->db_instance_id) {
		return false;
	}
	if(snap.value0 != obj->obj_flags.value[0] || snap.value1 != obj->obj_flags.value[1] ||
	   snap.value2 != obj->obj_flags.value[2] || snap.value3 != obj->obj_flags.value[3] ||
	   snap.extra_flags != static_cast<int>(obj->obj_flags.extra_flags) ||
	   snap.extra_flags2 != static_cast<int>(obj->obj_flags.extra_flags2) ||
	   snap.weight != obj->obj_flags.weight || snap.timer != obj->obj_flags.timer ||
	   snap.bitvector != static_cast<unsigned int>(obj->obj_flags.bitvector)) {
		return false;
	}
	const char* n = obj->name ? obj->name : "";
	const char* sd = obj->short_description ? obj->short_description : "";
	const char* d = obj->description ? obj->description : "";
	return snap.obj_name == n && snap.short_desc == sd && snap.description == d;
}

void overflow_delete_ids_tx(DB* db, const std::string& toon_id,
							const std::vector<unsigned long long>& ids) {
	if(ids.empty()) {
		return;
	}
	std::ostringstream in;
	for(size_t i = 0; i < ids.size(); ++i) {
		if(i) {
			in << ',';
		}
		in << ids[i];
	}
	db->execute(("DELETE FROM character_inventory_overflow WHERE toon_id = " + toon_id +
				 " AND id IN (" + in.str() + ")")
					.c_str());
}

void overflow_clear_ids_on_obj_tree(struct obj_data* obj) {
	if(!obj) {
		return;
	}
	obj->db_overflow_id = 0;
	for(struct obj_data* c = obj->contains; c; c = c->next_content) {
		overflow_clear_ids_on_obj_tree(c);
	}
}

void overflow_clear_ids_on_char(struct char_data* ch) {
	if(!ch) {
		return;
	}
	for(struct obj_data* o = ch->carrying; o; o = o->next_content) {
		overflow_clear_ids_on_obj_tree(o);
	}
	for(int i = 0; i < MAX_WEAR; ++i) {
		if(ch->equipment[i]) {
			overflow_clear_ids_on_obj_tree(ch->equipment[i]);
		}
	}
}

void overflow_collect_ids_on_obj_tree(struct obj_data* obj,
									  std::unordered_set<unsigned long long>& out) {
	if(!obj) {
		return;
	}
	if(obj->db_overflow_id != 0) {
		out.insert(obj->db_overflow_id);
	}
	for(struct obj_data* c = obj->contains; c; c = c->next_content) {
		overflow_collect_ids_on_obj_tree(c, out);
	}
}

/** Id overflow presenti sugli oggetti del PG (sessione live / reattach). */
void overflow_collect_ids_on_char(struct char_data* ch,
								  std::unordered_set<unsigned long long>& out) {
	out.clear();
	if(!ch) {
		return;
	}
	for(struct obj_data* o = ch->carrying; o; o = o->next_content) {
		overflow_collect_ids_on_obj_tree(o, out);
	}
	for(int i = 0; i < MAX_WEAR; ++i) {
		if(ch->equipment[i]) {
			overflow_collect_ids_on_obj_tree(ch->equipment[i], out);
		}
	}
}

bool overflow_load_active_snap_tx(DB* db, const std::string& toon_id,
								 std::unordered_map<unsigned long long, OvDbSnap>& out) {
	out.clear();
	MYSQL_RES* res = nullptr;
	const std::string sql =
		"SELECT id, list_index, item_number, value0, value1, value2, value3, extra_flags, "
		"extra_flags2, weight, timer, bitvector, obj_name, short_desc, description, wear_pos, "
		"depth, IFNULL(parent_overflow_id,0), IFNULL(instance_id,0) "
		"FROM character_inventory_overflow WHERE toon_id = " +
		toon_id + " AND (deleted = 0 OR deleted IS NULL)";
	if(!ov_mysql_query(db, sql, res) || !res) {
		if(res) {
			mysql_free_result(res);
		}
		return false;
	}
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		const unsigned long long id = row[0] ? std::strtoull(row[0], nullptr, 10) : 0;
		if(id == 0) {
			continue;
		}
		OvDbSnap s {};
		s.list_index = row[1] ? std::atoi(row[1]) : 0;
		s.item_number = row[2] ? std::atoi(row[2]) : 0;
		s.value0 = row[3] ? std::atoi(row[3]) : 0;
		s.value1 = row[4] ? std::atoi(row[4]) : 0;
		s.value2 = row[5] ? std::atoi(row[5]) : 0;
		s.value3 = row[6] ? std::atoi(row[6]) : 0;
		s.extra_flags = row[7] ? std::atoi(row[7]) : 0;
		s.extra_flags2 = row[8] ? std::atoi(row[8]) : 0;
		s.weight = row[9] ? std::atoi(row[9]) : 0;
		s.timer = row[10] ? std::atoi(row[10]) : 0;
		s.bitvector = row[11] ? static_cast<unsigned int>(std::strtoul(row[11], nullptr, 10)) : 0;
		s.obj_name = row[12] ? row[12] : "";
		s.short_desc = row[13] ? row[13] : "";
		s.description = row[14] ? row[14] : "";
		s.wear_pos = row[15] ? std::atoi(row[15]) : 0;
		s.depth = row[16] ? std::atoi(row[16]) : 0;
		s.parent_id = row[17] ? std::strtoull(row[17], nullptr, 10) : 0;
		s.instance_id = row[18] ? std::strtoull(row[18], nullptr, 10) : 0;
		out.emplace(id, std::move(s));
	}
	mysql_free_result(res);
	return true;
}

/**
 * Sync incrementale del set spill corrente.
 * live_ids: overflow_id presenti sul PG prima del clear (sessione live).
 * Righe DB attive non in live_ids = deposito claim (forcerent) → non cancellare.
 */
bool overflow_sync_active_tx(DB* db, const std::string& toon_id,
							 const std::vector<struct obj_data*>& roots,
							 const std::unordered_set<unsigned long long>& live_ids) {
	if(roots.empty()) {
		return true;
	}

	const std::unordered_set<struct obj_data*> separate_roots(roots.begin(), roots.end());
	std::vector<OvFlatItem> flat;
	int list_index = 0;
	for(struct obj_data* r : roots) {
		overflow_flatten_tree(r, nullptr, 0, list_index, separate_roots, flat);
	}

	std::unordered_map<unsigned long long, OvDbSnap> db_snap;
	if(!overflow_load_active_snap_tx(db, toon_id, db_snap)) {
		mudlog(LOG_SYSERR, "inventory_overflow: snapshot failed for toon %s", toon_id.c_str());
		return false;
	}

	std::unordered_set<unsigned long long> kept;
	int skipped = 0;
	int updated = 0;
	int inserted = 0;

	for(OvFlatItem& item : flat) {
		struct obj_data* obj = item.obj;
		if(!obj) {
			continue;
		}
		const unsigned long long parent_id =
			item.parent && item.parent->db_overflow_id != 0 ? item.parent->db_overflow_id : 0;
		unsigned long long id = obj->db_overflow_id;
		if(id != 0 && db_snap.find(id) == db_snap.end()) {
			id = 0;
			obj->db_overflow_id = 0;
		}
		if(id != 0) {
			const OvDbSnap& snap = db_snap[id];
			if(overflow_snap_matches_obj(snap, obj, item.list_index, item.depth, parent_id)) {
				++skipped;
				kept.insert(id);
				continue;
			}
			overflow_update_one_tx(db, toon_id, id, obj, parent_id, item.list_index, item.depth);
			++updated;
			kept.insert(id);
			continue;
		}
		const unsigned long long new_id =
			overflow_insert_one_tx(db, toon_id, obj, parent_id, item.list_index, item.depth);
		if(new_id == 0) {
			return false;
		}
		++inserted;
		kept.insert(new_id);
	}

	std::vector<unsigned long long> removed;
	size_t preserved = 0;
	for(const auto& kv : db_snap) {
		if(kept.count(kv.first) != 0) {
			continue;
		}
		if(live_ids.count(kv.first) == 0) {
			/* Deposito claim orfano (es. forcerent): lascia per Anxur. */
			++preserved;
			continue;
		}
		removed.push_back(kv.first);
	}
	overflow_delete_ids_tx(db, toon_id, removed);

	mudlog(LOG_SAVE,
		   "inventory_overflow incremental toon %s: skipped=%d updated=%d inserted=%d "
		   "removed=%zu preserved=%zu",
		   toon_id.c_str(), skipped, updated, inserted, removed.size(), preserved);
	return true;
}

void overflow_detach_to_hold(struct obj_data* obj, InventoryOverflowHold& hold) {
	if(!obj) {
		return;
	}
	struct obj_data* parent = obj->in_obj;
	hold.roots.push_back(obj);
	hold.parents.push_back(parent);
	if(obj->equipped_by) {
		const int pos = obj->eq_pos;
		if(pos >= 0 && pos < MAX_WEAR) {
			unequip_char(obj->equipped_by, pos);
		}
	}
	else if(obj->carried_by) {
		obj_from_char(obj);
	}
	else if(obj->in_obj) {
		obj_from_obj(obj);
	}
}

} // namespace

void inventory_overflow_finish_persist(InventoryOverflowHold& hold, bool reattach) {
	if(!hold.active) {
		hold = {};
		return;
	}
	struct char_data* ch = hold.ch;
	const size_t n = hold.roots.size();
	for(size_t i = 0; i < n; ++i) {
		struct obj_data* obj = hold.roots[i];
		struct obj_data* parent = (i < hold.parents.size()) ? hold.parents[i] : nullptr;
		if(!obj) {
			continue;
		}
		if(reattach && ch) {
			if(parent) {
				obj_to_obj(obj, parent);
			}
			else {
				obj_to_char(obj, ch);
			}
		}
		else {
			extract_obj(obj);
		}
	}
	hold = {};
}

bool inventory_overflow_begin_persist(struct char_data* ch, InventoryOverflowHold& hold) {
	hold = {};
	if(!ch || !IS_PC(ch)) {
		return false;
	}

	struct char_data* pc = overflow_resolve_toon_pc(ch);
	if(!pc || !GET_NAME(pc) || !toon_is_migrated_by_name(GET_NAME(pc))) {
		return false;
	}

	hold.ch = ch;
	hold.active = true;

	const int total = ContaOggettiChar(ch);
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(GET_NAME(pc)));
		if(!pg || !pg->id) {
			mudlog(LOG_SYSERR, "inventory_overflow_begin: missing toon for %s", GET_NAME(pc));
			hold = {};
			return false;
		}
		DB* db = Sql::getMysql();
		if(!db) {
			hold = {};
			return false;
		}
		const std::string toon_id = std::to_string(pg->id);

		/* Id spill live sul PG (reattach). Senza questi non toccare il deposito claim. */
		std::unordered_set<unsigned long long> live_ids;
		overflow_collect_ids_on_char(ch, live_ids);

		if(total <= MAX_OBJ_SAVE) {
			if(live_ids.empty()) {
				mudlog(LOG_CHECK,
					   "inventory_overflow: leave claim warehouse for %s (have %d, no live ids)",
					   GET_NAME(pc), total);
				return true;
			}
			std::vector<unsigned long long> absorbed(live_ids.begin(), live_ids.end());
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			overflow_delete_ids_tx(db, toon_id, absorbed);
			t.commit();
			overflow_clear_ids_on_char(ch);
			mudlog(LOG_CHECK,
				   "inventory_overflow: absorbed %zu live overflow id(s) for %s (have %d)",
				   absorbed.size(), GET_NAME(pc), total);
			return true;
		}

		int need = total - MAX_OBJ_SAVE;
		std::vector<struct obj_data*> roots;
		inventory_plan_overflow(ch, need, roots);
		if(roots.empty()) {
			mudlog(LOG_SYSERR, "inventory_overflow: no candidates for %s (have %d)",
				   GET_NAME(pc), total);
			hold = {};
			return false;
		}

		for(struct obj_data* r : roots) {
			overflow_detach_to_hold(r, hold);
		}
		/* I pezzi restati sul PG non sono piu' nello spill corrente. */
		overflow_clear_ids_on_char(ch);

		odb::transaction t(db->begin());
		t.tracer(logTracer);
		if(!overflow_sync_active_tx(db, toon_id, hold.roots, live_ids)) {
			mudlog(LOG_SYSERR, "inventory_overflow: sync failed for %s", GET_NAME(pc));
			t.rollback();
			/* Rimetti i pezzi: finish con reattach. */
			inventory_overflow_finish_persist(hold, true);
			return false;
		}
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_begin(%s): %s", GET_NAME(pc), e.what());
		if(hold.active && !hold.roots.empty()) {
			inventory_overflow_finish_persist(hold, true);
		}
		else {
			hold = {};
		}
		return false;
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_begin(%s): %s", GET_NAME(pc), e.what());
		if(hold.active && !hold.roots.empty()) {
			inventory_overflow_finish_persist(hold, true);
		}
		else {
			hold = {};
		}
		return false;
	}

	mudlog(LOG_PLAYERS,
		   "inventory_overflow: persist %zu root(s) for %s (was %d, cap %d) - save then finish",
		   hold.roots.size(), GET_NAME(pc), total, MAX_OBJ_SAVE);
	return true;
}

void inventory_overflow_expire_stale() {
	if(OVERFLOW_EXPIRE_HOURS <= 0) {
		return;
	}
	try {
		DB* db = Sql::getMysql();
		if(!db) {
			return;
		}
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		std::ostringstream sql;
		sql << "UPDATE character_inventory_overflow SET deleted = 1, deleted_on = NOW(), "
			   "deleted_for = 'OVERFLOW_EXPIRE' WHERE (deleted = 0 OR deleted IS NULL) "
			   "AND created_on < (NOW() - INTERVAL "
			<< OVERFLOW_EXPIRE_HOURS << " HOUR)";
		db->execute(sql.str().c_str());
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_expire_stale: %s", e.what());
	}
}

void inventory_overflow_checkpoint_pulse(unsigned long now_pulse) {
	const unsigned long interval =
		static_cast<unsigned long>(OVERFLOW_CHECKPOINT_MINUTES) * 60UL *
		static_cast<unsigned long>(PULSE_PER_SEC);
	static bool drain = false;
	/* Con finish(reattach) il PG resta >200: senza questo set drain
	 * ripersiste lo stesso PG ogni pulse e soft-delete a raffica. */
	static std::unordered_set<std::string> done_this_round;

	if(interval != 0 && (now_pulse % interval) == 0) {
		drain = true;
		done_this_round.clear();
	}
	if(!drain) {
		return;
	}

	for(struct char_data* ch = character_list; ch; ch = ch->next) {
		if(!IS_PC(ch)) {
			continue;
		}
		struct char_data* pc = overflow_resolve_toon_pc(ch);
		if(!pc || !GET_NAME(pc) || !toon_is_migrated_by_name(GET_NAME(pc))) {
			continue;
		}
		const std::string key(GET_NAME(pc));
		if(done_this_round.count(key)) {
			continue;
		}
		if(ContaOggettiChar(ch) <= MAX_OBJ_SAVE) {
			continue;
		}
		InventoryOverflowHold hold;
		if(!inventory_overflow_begin_persist(ch, hold)) {
			done_this_round.insert(key);
			return;
		}
		/* begin gia' fatto: niente secondo spill (cancellerebbe l'overflow). */
		do_save_rent(ch, false);
		inventory_overflow_finish_persist(hold, true);
		done_this_round.insert(key);
		return;
	}
	drain = false;
	done_this_round.clear();
}

namespace {

bool overflow_resolve_toon_id(struct char_data* ch, std::string& toon_id_out,
							  struct char_data*& pc_out) {
	toon_id_out.clear();
	pc_out = overflow_resolve_toon_pc(ch);
	if(!pc_out || !GET_NAME(pc_out) || !toon_is_migrated_by_name(GET_NAME(pc_out))) {
		return false;
	}
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(GET_NAME(pc_out)));
		if(!pg || !pg->id) {
			return false;
		}
		toon_id_out = std::to_string(pg->id);
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "overflow_resolve_toon_id(%s): %s",
			   GET_NAME(pc_out) ? GET_NAME(pc_out) : "?", e.what());
		return false;
	}
}

/**
 * Deposito target: proprio, oppure (QUESTMASTER+) owner_name.
 * staff_other: true se staff agisce sul deposito di un altro PG.
 */
bool overflow_resolve_owner(struct char_data* actor, const char* owner_name,
							std::string& toon_id_out, std::string& owner_label_out,
							bool& staff_other_out) {
	toon_id_out.clear();
	owner_label_out.clear();
	staff_other_out = false;
	if(!actor || IS_NPC(actor)) {
		return false;
	}
	const bool named = owner_name && owner_name[0] != '\0';
	if(!named) {
		struct char_data* pc = nullptr;
		if(!overflow_resolve_toon_id(actor, toon_id_out, pc)) {
			return false;
		}
		owner_label_out = GET_NAME(pc) ? GET_NAME(pc) : "?";
		return true;
	}
	if(!IS_QUESTMASTER(actor)) {
		return false;
	}
	if(!toon_is_migrated_by_name(owner_name)) {
		return false;
	}
	try {
		const toonPtr pg = Sql::getOne<toon>(toonQuery::name == std::string(owner_name));
		if(!pg || !pg->id) {
			return false;
		}
		toon_id_out = std::to_string(pg->id);
		owner_label_out = pg->name.empty() ? std::string(owner_name) : pg->name;
		const char* self = GET_NAME(actor);
		staff_other_out =
			!(self && str_cmp(self, owner_label_out.c_str()) == 0);
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "overflow_resolve_owner(%s -> %s): %s",
			   GET_NAME(actor) ? GET_NAME(actor) : "?", owner_name, e.what());
		return false;
	}
}

using OvAffectArr = std::array<obj_affected_type, MAX_OBJ_AFFECT>;

bool overflow_affects_equal(const OvAffectArr& a, const OvAffectArr& b) {
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(a[i].location != b[i].location || a[i].modifier != b[i].modifier) {
			return false;
		}
	}
	return true;
}

/** Identici per listino (ignora parent/depth/list_index/wear_pos). */
bool overflow_snap_stack_equal(const OvDbSnap& a, const OvDbSnap& b, const OvAffectArr& aa,
							   const OvAffectArr& ba) {
	if(a.item_number != b.item_number || a.instance_id != b.instance_id) {
		return false;
	}
	if(a.value0 != b.value0 || a.value1 != b.value1 || a.value2 != b.value2 ||
	   a.value3 != b.value3) {
		return false;
	}
	if(a.extra_flags != b.extra_flags || a.extra_flags2 != b.extra_flags2 ||
	   a.weight != b.weight || a.timer != b.timer || a.bitvector != b.bitvector) {
		return false;
	}
	if(a.obj_name != b.obj_name || a.short_desc != b.short_desc ||
	   a.description != b.description) {
		return false;
	}
	return overflow_affects_equal(aa, ba);
}

bool overflow_promote_orphans_tx(DB* db, const std::string& toon_id,
								 const std::vector<unsigned long long>& former_parents) {
	if(!db || former_parents.empty()) {
		return true;
	}
	std::ostringstream in;
	for(size_t i = 0; i < former_parents.size(); ++i) {
		if(i) {
			in << ',';
		}
		in << former_parents[i];
	}
	std::ostringstream sql;
	sql << "UPDATE character_inventory_overflow SET parent_overflow_id = NULL WHERE toon_id = "
		<< toon_id << " AND parent_overflow_id IN (" << in.str()
		<< ") AND (deleted = 0 OR deleted IS NULL)";
	db->execute(sql.str().c_str());
	return true;
}

/** Livello minimo di apprendimento spell (classi con min_level > 0). */
int overflow_spell_learn_level(int spell_num) {
	if(spell_num < 1 || spell_num >= MAX_SPL_LIST) {
		return 0;
	}
	const spell_info_type& si = spell_info[spell_num];
	int best = 0;
	const ubyte levels[] = {
		si.min_level_magic, si.min_level_cleric, si.min_level_druid,
		si.min_level_sorcerer, si.min_level_paladin, si.min_level_ranger,
		si.min_level_psi};
	for(ubyte lv : levels) {
		if(lv == 0) {
			continue;
		}
		if(best == 0 || static_cast<int>(lv) < best) {
			best = static_cast<int>(lv);
		}
	}
	return best;
}

long overflow_mega_to_coin(long mega) {
	if(mega <= 0) {
		return 0;
	}
	return mega * OvListino::kCoinPerMega;
}

/** Mega listino per uno spell; fallback da learn level. */
int overflow_spell_mega(int spell_num) {
	if(spell_num < 1 || spell_num >= MAX_SPL_LIST) {
		return 0;
	}
	for(const OvListino::SpellMega& row : OvListino::kSpellRates) {
		if(row.spell == spell_num) {
			return row.mega;
		}
	}
	const int learn = overflow_spell_learn_level(spell_num);
	int mega = learn * OvListino::kSpellUnknownMegaPerLearnLevel;
	if(mega < OvListino::kSpellUnknownMegaMin) {
		mega = OvListino::kSpellUnknownMegaMin;
	}
	if(mega > OvListino::kSpellUnknownMegaMax) {
		mega = OvListino::kSpellUnknownMegaMax;
	}
	return mega;
}

/** Scala il mega spell rispetto al livello di lancio (ref = kSpellRefCastLevel). */
long overflow_spell_market_one(int spell_num, int cast_level) {
	const int mega = overflow_spell_mega(spell_num);
	if(mega <= 0) {
		return 0;
	}
	int lv = cast_level;
	if(lv < 1) {
		lv = 1;
	}
	if(lv > 60) {
		lv = 60;
	}
	const int ref = OvListino::kSpellRefCastLevel;
	/* ~0.6 + level/50 rispetto al ref (ref → 1.0). */
	const long numer = 30L + static_cast<long>(lv);
	const long denom = 30L + static_cast<long>(ref);
	return (overflow_mega_to_coin(mega) * numer) / denom;
}

long overflow_estimate_spell_consumable_value(int cast_level, int sp1, int sp2, int sp3,
											 int charges) {
	if(charges < 1) {
		charges = 1;
	}
	long value = 0;
	const int spells[3] = {sp1, sp2, sp3};
	for(int i = 0; i < 3; ++i) {
		value += overflow_spell_market_one(spells[i], cast_level);
	}
	if(value <= 0) {
		value = 500;
	}
	return value * static_cast<long>(charges);
}

long overflow_affect_location_coins(int location, int modifier) {
	const int units = modifier < 0 ? -modifier : modifier;
	if(units == 0) {
		return 0;
	}
	for(const OvListino::AffectRate& row : OvListino::kAffectRates) {
		if(row.location != location || row.batch <= 0 || row.mega_x10 <= 0) {
			continue;
		}
		/* mega = units * (mega_x10/10) / batch → coin = mega * kCoinPerMega */
		const long coin =
			(static_cast<long>(units) * static_cast<long>(row.mega_x10) *
			 OvListino::kCoinPerMega) /
			(static_cast<long>(row.batch) * 10L);
		return coin;
	}
	return 0;
}

long overflow_imm_bits_coins(unsigned long bits, bool full_immune) {
	if(bits == 0) {
		return 0;
	}
	long mega = 0;
	unsigned long seen = 0;
	for(const OvListino::ImmBitMega& row : OvListino::kImmBitRates) {
		if((bits & row.bit) == 0) {
			continue;
		}
		seen |= row.bit;
		mega += full_immune ? row.immune_mega : row.resist_mega;
	}
	unsigned long rest = bits & ~seen;
	while(rest != 0) {
		rest &= rest - 1; /* pop one bit */
		mega += full_immune ? OvListino::kImmBitImmuneDefaultMega
							: OvListino::kImmBitResistDefaultMega;
	}
	return overflow_mega_to_coin(mega);
}

long overflow_affects_market(const OvAffectArr* aff) {
	if(!aff) {
		return 0;
	}
	long total = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = (*aff)[i].location;
		const int mod = (*aff)[i].modifier;
		if(loc == APPLY_NONE || mod == 0) {
			continue;
		}
		if(loc == APPLY_IMMUNE || loc == APPLY_SUSC) {
			total += overflow_imm_bits_coins(static_cast<unsigned long>(mod), false);
			continue;
		}
		if(loc == APPLY_M_IMMUNE) {
			total += overflow_imm_bits_coins(static_cast<unsigned long>(mod), true);
			continue;
		}
		if(loc == APPLY_SPELL || loc == APPLY_WEAPON_SPELL || loc == APPLY_EAT_SPELL) {
			/* modifier = spell id; potenza ~ ref level. */
			total += overflow_spell_market_one(mod, OvListino::kSpellRefCastLevel);
			continue;
		}
		total += overflow_affect_location_coins(loc, mod);
	}
	return total;
}

long overflow_claim_fee_from_value(long market_value) {
	if(market_value <= 0) {
		return kOverflowClaimFeeMin;
	}
	long fee = market_value / kOverflowClaimFeeDivisor;
	if(fee < kOverflowClaimFeeMin) {
		fee = kOverflowClaimFeeMin;
	}
	return fee;
}

bool overflow_snap_is_limited(const OvDbSnap& s) {
	const int rnum = real_object(s.item_number);
	if(rnum < 0 || obj_index[rnum].data == nullptr) {
		return false;
	}
	const struct obj_data* proto =
		static_cast<const struct obj_data*>(obj_index[rnum].data);
	return proto->obj_flags.cost >= LIM_ITEM_COST_MIN;
}

long overflow_mine_table_lookup(const OvListino::MineMarket* table, size_t n, int vnum) {
	for(size_t i = 0; i < n; ++i) {
		if(table[i].vnum == vnum) {
			return table[i].market_coin;
		}
	}
	return 0;
}

/** Mercato claim per pepite / pietre / lingotti (0 = non mining / sconosciuto). */
long overflow_mining_market(int vnum, int type_flag, long proto_cost) {
	if(type_flag == ITEM_M_MINERAL) {
		const long hit = overflow_mine_table_lookup(
			OvListino::kMineMinerals,
			sizeof(OvListino::kMineMinerals) / sizeof(OvListino::kMineMinerals[0]), vnum);
		if(hit > 0) {
			return hit;
		}
		return std::max(10000L, proto_cost * 400L);
	}
	if(type_flag == ITEM_M_GEM) {
		const long hit = overflow_mine_table_lookup(
			OvListino::kMineGems, sizeof(OvListino::kMineGems) / sizeof(OvListino::kMineGems[0]),
			vnum);
		if(hit > 0) {
			return hit;
		}
		if(proto_cost > 0) {
			return proto_cost * OvListino::kMineGemCostMult;
		}
		return 120000;
	}
	if(type_flag == ITEM_BAR) {
		const long hit = overflow_mine_table_lookup(
			OvListino::kMineBars, sizeof(OvListino::kMineBars) / sizeof(OvListino::kMineBars[0]),
			vnum);
		if(hit > 0) {
			return hit;
		}
		return std::max(30000L, proto_cost * 600L);
	}
	return 0;
}

/**
 * Mercato stimato: max(proto.cost, affect+spell+ac+artifact [, mining]).
 * Fee = mercato / divisor (min), × limited.
 */
long overflow_claim_fee_one(const OvDbSnap& s, const OvAffectArr* aff) {
	const int rnum = real_object(s.item_number);
	int type_flag = -1;
	long proto_cost = 0;
	int extra_flags = s.extra_flags;
	int vnum = s.item_number;
	if(rnum >= 0 && obj_index[rnum].data != nullptr) {
		const struct obj_data* proto =
			static_cast<const struct obj_data*>(obj_index[rnum].data);
		type_flag = static_cast<int>(proto->obj_flags.type_flag);
		proto_cost = proto->obj_flags.cost;
		if(obj_index[rnum].iVNum != 0) {
			vnum = obj_index[rnum].iVNum;
		}
	}

	const long mining = overflow_mining_market(vnum, type_flag, proto_cost);
	if(mining > 0) {
		/* Derivati dig: tabella rarita'/utilita' (ignora proto basso). */
		long fee = overflow_claim_fee_from_value(mining);
		if(proto_cost >= LIM_ITEM_COST_MIN) {
			fee *= kOverflowClaimLimitedMult;
		}
		return fee;
	}

	long built = overflow_affects_market(aff);

	if(type_flag == ITEM_POTION || type_flag == ITEM_SCROLL) {
		built += overflow_estimate_spell_consumable_value(s.value0, s.value1, s.value2, s.value3,
														  1);
	}
	else if(type_flag == ITEM_WAND || type_flag == ITEM_STAFF) {
		built += overflow_estimate_spell_consumable_value(s.value0, s.value3, 0, 0,
														  std::max(1, s.value2));
	}
	else if(type_flag == ITEM_ARMOR) {
		const int ac = s.value0 > 0 ? s.value0 : 0;
		built += overflow_mega_to_coin(static_cast<long>(ac) *
									   OvListino::kArmorAcMegaPerPoint);
	}

	/* ITEM_IMMUNE = flag ARTIFACT in game (immune to scrap). */
	if(IS_SET(extra_flags, ITEM_IMMUNE)) {
		built += overflow_mega_to_coin(OvListino::kArtifactFlagMega);
	}

	long market = built;
	if(proto_cost > market) {
		market = proto_cost;
	}
	if(market <= 0) {
		market = 1000;
	}

	long fee = overflow_claim_fee_from_value(market);
	if(proto_cost >= LIM_ITEM_COST_MIN) {
		fee *= kOverflowClaimLimitedMult;
	}
	return fee;
}

void overflow_load_affects_tx(DB* db, const std::vector<unsigned long long>& ids,
							  std::unordered_map<unsigned long long,
												 std::array<obj_affected_type, MAX_OBJ_AFFECT>>&
								  out) {
	out.clear();
	if(!db || ids.empty()) {
		return;
	}
	std::ostringstream in;
	for(size_t i = 0; i < ids.size(); ++i) {
		if(i) {
			in << ',';
		}
		in << ids[i];
	}
	MYSQL_RES* res = nullptr;
	const std::string sql =
		"SELECT overflow_id, affect_slot, location, modifier FROM "
		"character_inventory_overflow_affect WHERE overflow_id IN (" +
		in.str() + ")";
	if(!ov_mysql_query(db, sql, res) || !res) {
		if(res) {
			mysql_free_result(res);
		}
		return;
	}
	while(MYSQL_ROW row = mysql_fetch_row(res)) {
		const unsigned long long oid = row[0] ? std::strtoull(row[0], nullptr, 10) : 0;
		const int slot = row[1] ? std::atoi(row[1]) : -1;
		if(oid == 0 || slot < 0 || slot >= MAX_OBJ_AFFECT) {
			continue;
		}
		auto& arr = out[oid];
		arr[slot].location = row[2] ? static_cast<short>(std::atoi(row[2])) : 0;
		arr[slot].modifier = row[3] ? std::atoi(row[3]) : 0;
	}
	mysql_free_result(res);
}

struct obj_data* overflow_materialize_one(const OvDbSnap& snap, unsigned long long overflow_id,
										  unsigned long long instance_id,
										  const obj_affected_type* affects) {
	struct obj_data* obj =
		object_instance_load_stored(snap.item_number, instance_id != 0 ? instance_id : snap.instance_id);
	if(!obj) {
		return nullptr;
	}
#if USE_MYSQL
	if(!(obj->db_instance_id != 0 && clan_symbol_is_obj(obj)))
#endif
	{
		obj->obj_flags.value[0] = snap.value0;
		obj->obj_flags.value[1] = snap.value1;
		obj->obj_flags.value[2] = snap.value2;
		obj->obj_flags.value[3] = snap.value3;
		obj->obj_flags.extra_flags = snap.extra_flags;
		obj->obj_flags.extra_flags2 = snap.extra_flags2;
		obj->obj_flags.weight = snap.weight;
		obj->obj_flags.timer = snap.timer;
		obj->obj_flags.bitvector = snap.bitvector;
		if(obj->name) {
			free(obj->name);
		}
		if(obj->short_description) {
			free(obj->short_description);
		}
		if(obj->description) {
			free(obj->description);
		}
		obj->name = strdup(snap.obj_name.c_str());
		obj->short_description = strdup(snap.short_desc.c_str());
		obj->description = strdup(snap.description.c_str());
		hydrate_personal_owner_from_ed(obj, true);
		if(affects) {
			for(int j = 0; j < MAX_OBJ_AFFECT; ++j) {
				obj->affected[j] = affects[j];
			}
		}
	}
	(void)achie_reward_remap_obj(obj);
	obj->db_overflow_id = 0;
	obj->db_inventory_id = 0;
	(void)overflow_id;
	return obj;
}

void overflow_extract_tree(struct obj_data* obj) {
	if(!obj) {
		return;
	}
	while(obj->contains) {
		overflow_extract_tree(obj->contains);
	}
	extract_obj(obj);
}

bool overflow_soft_delete_claimed_tx(DB* db, const std::string& toon_id,
									 const std::vector<unsigned long long>& ids) {
	if(!db || ids.empty()) {
		return false;
	}
	std::ostringstream in;
	for(size_t i = 0; i < ids.size(); ++i) {
		if(i) {
			in << ',';
		}
		in << ids[i];
	}
	std::ostringstream sql;
	sql << "UPDATE character_inventory_overflow SET deleted = 1, deleted_on = NOW(), "
		   "deleted_for = 'CLAIMED' WHERE toon_id = "
		<< toon_id << " AND id IN (" << in.str()
		<< ") AND (deleted = 0 OR deleted IS NULL)";
	db->execute(sql.str().c_str());
	return true;
}

} // namespace

bool inventory_overflow_list_owners(struct char_data* actor,
								   std::vector<InventoryOverflowOwnerCount>& out) {
	out.clear();
	if(!actor || IS_NPC(actor) || !IS_QUESTMASTER(actor)) {
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return false;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		MYSQL_RES* res = nullptr;
		const std::string sql =
			"SELECT t.name, COUNT(*) AS pezzi "
			"FROM character_inventory_overflow o "
			"INNER JOIN toon t ON t.id = o.toon_id "
			"WHERE o.deleted = 0 OR o.deleted IS NULL "
			"GROUP BY t.name "
			"ORDER BY pezzi DESC, t.name ASC";
		if(!ov_mysql_query(db, sql, res) || !res) {
			if(res) {
				mysql_free_result(res);
			}
			t.rollback();
			return false;
		}
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(!row[0] || row[0][0] == '\0') {
				continue;
			}
			InventoryOverflowOwnerCount row_out;
			row_out.name = row[0];
			row_out.item_count = row[1] ? std::atoi(row[1]) : 0;
			if(row_out.item_count > 0) {
				out.push_back(std::move(row_out));
			}
		}
		mysql_free_result(res);
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_list_owners(%s): %s",
			   GET_NAME(actor) ? GET_NAME(actor) : "?", e.what());
		return false;
	}
}

bool inventory_overflow_list(struct char_data* actor,
							 std::vector<InventoryOverflowListItem>& out,
							 const char* owner_name,
							 std::string* resolved_owner_out) {
	out.clear();
	if(resolved_owner_out) {
		resolved_owner_out->clear();
	}
	if(!actor) {
		return false;
	}
	std::string toon_id;
	std::string owner_label;
	bool staff_other = false;
	if(!overflow_resolve_owner(actor, owner_name, toon_id, owner_label, staff_other)) {
		return false;
	}
	(void)staff_other;
	if(resolved_owner_out) {
		*resolved_owner_out = owner_label;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return false;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		std::unordered_map<unsigned long long, OvDbSnap> snap;
		if(!overflow_load_active_snap_tx(db, toon_id, snap)) {
			t.rollback();
			return false;
		}
		std::vector<unsigned long long> all_ids;
		all_ids.reserve(snap.size());
		for(const auto& kv : snap) {
			all_ids.push_back(kv.first);
		}
		std::unordered_map<unsigned long long, OvAffectArr> aff;
		overflow_load_affects_tx(db, all_ids, aff);
		t.commit();

		std::sort(all_ids.begin(), all_ids.end());
		std::unordered_set<unsigned long long> used;
		for(unsigned long long id : all_ids) {
			if(used.count(id) != 0) {
				continue;
			}
			const OvDbSnap& base = snap[id];
			OvAffectArr base_aff {};
			const auto ait = aff.find(id);
			if(ait != aff.end()) {
				base_aff = ait->second;
			}
			InventoryOverflowListItem item;
			item.overflow_ids.push_back(id);
			used.insert(id);
			for(unsigned long long other : all_ids) {
				if(used.count(other) != 0) {
					continue;
				}
				OvAffectArr other_aff {};
				const auto oit = aff.find(other);
				if(oit != aff.end()) {
					other_aff = oit->second;
				}
				if(overflow_snap_stack_equal(base, snap[other], base_aff, other_aff)) {
					item.overflow_ids.push_back(other);
					used.insert(other);
				}
			}
			item.short_desc = base.short_desc.empty() ? base.obj_name : base.short_desc;
			item.item_count = static_cast<int>(item.overflow_ids.size());
			item.price = overflow_claim_fee_one(base, &base_aff);
			item.is_limited = overflow_snap_is_limited(base);
			out.push_back(std::move(item));
		}
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_list(%s): %s", owner_label.c_str(), e.what());
		return false;
	}
}

long inventory_overflow_claim_price(struct char_data* actor, unsigned long long overflow_id,
									const char* owner_name) {
	if(!actor || overflow_id == 0) {
		return 0;
	}
	std::string toon_id;
	std::string owner_label;
	bool staff_other = false;
	if(!overflow_resolve_owner(actor, owner_name, toon_id, owner_label, staff_other)) {
		return 0;
	}
	(void)staff_other;
	DB* db = Sql::getMysql();
	if(!db) {
		return 0;
	}
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		std::unordered_map<unsigned long long, OvDbSnap> snap;
		if(!overflow_load_active_snap_tx(db, toon_id, snap)) {
			t.rollback();
			return 0;
		}
		std::vector<unsigned long long> ids{overflow_id};
		std::unordered_map<unsigned long long, OvAffectArr> aff;
		overflow_load_affects_tx(db, ids, aff);
		t.commit();
		const auto it = snap.find(overflow_id);
		if(it == snap.end()) {
			return 0;
		}
		OvAffectArr empty {};
		const OvAffectArr* ap = &empty;
		const auto ait = aff.find(overflow_id);
		if(ait != aff.end()) {
			ap = &ait->second;
		}
		return overflow_claim_fee_one(it->second, ap);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_claim_price(%s): %s", owner_label.c_str(),
			   e.what());
		return 0;
	}
}

bool inventory_overflow_claim(struct char_data* actor,
							  const std::vector<unsigned long long>& overflow_ids,
							  const char* owner_name) {
	if(!actor || IS_NPC(actor) || overflow_ids.empty()) {
		return false;
	}
	for(unsigned long long id : overflow_ids) {
		if(id == 0) {
			return false;
		}
	}
	std::string toon_id;
	std::string owner_label;
	bool staff_other = false;
	if(!overflow_resolve_owner(actor, owner_name, toon_id, owner_label, staff_other)) {
		send_to_char("Non ho nulla in deposito a quel nome.\n\r", actor);
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return false;
	}

	std::unordered_map<unsigned long long, OvDbSnap> snap;
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		if(!overflow_load_active_snap_tx(db, toon_id, snap)) {
			t.rollback();
			send_to_char("Problema nel deposito. Avvisa un Dio.\n\r", actor);
			return false;
		}
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_claim load(%s): %s", owner_label.c_str(),
			   e.what());
		send_to_char("Problema nel deposito. Avvisa un Dio.\n\r", actor);
		return false;
	}

	for(unsigned long long id : overflow_ids) {
		if(snap.find(id) == snap.end()) {
			send_to_char("Quel pezzo non e' in deposito.\n\r", actor);
			return false;
		}
	}

	std::unordered_map<unsigned long long, OvAffectArr> aff;
	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		overflow_load_affects_tx(db, overflow_ids, aff);
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_claim affects(%s): %s", owner_label.c_str(),
			   e.what());
	}

	long unit_price = 0;
	for(unsigned long long id : overflow_ids) {
		OvAffectArr empty {};
		const OvAffectArr* ap = &empty;
		const auto ait = aff.find(id);
		if(ait != aff.end()) {
			ap = &ait->second;
		}
		unit_price += overflow_claim_fee_one(snap[id], ap);
	}
	const long price = staff_other ? 0 : unit_price;
	if(!staff_other && GET_GOLD(actor) < price) {
		send_to_char("Non hai abbastanza monete d'oro.\n\r", actor);
		return false;
	}

	std::vector<struct obj_data*> built;
	built.reserve(overflow_ids.size());
	for(unsigned long long id : overflow_ids) {
		const OvDbSnap& s = snap[id];
		const obj_affected_type* ap = nullptr;
		const auto ait = aff.find(id);
		if(ait != aff.end()) {
			ap = ait->second.data();
		}
		struct obj_data* obj = overflow_materialize_one(s, id, s.instance_id, ap);
		if(!obj) {
			mudlog(LOG_SYSERR, "inventory_overflow_claim: materialize failed id=%llu for %s",
				   static_cast<unsigned long long>(id), owner_label.c_str());
			for(struct obj_data* o : built) {
				if(o && !o->in_obj && !o->carried_by) {
					overflow_extract_tree(o);
				}
			}
			send_to_char("Non riesco a recuperare quel pezzo. Avvisa un Dio.\n\r", actor);
			return false;
		}
		built.push_back(obj);
	}

	SaveQuotaBlock blocked = SaveQuotaBlock::None;
	for(struct obj_data* obj : built) {
		blocked = char_save_quota_check(actor, obj);
		if(blocked != SaveQuotaBlock::None) {
			break;
		}
		/* Conta i pezzi gia' accettati nello stesso ritiro. */
		obj_to_char(obj, actor);
	}
	if(blocked != SaveQuotaBlock::None) {
		for(struct obj_data* o : built) {
			if(!o) {
				continue;
			}
			if(o->carried_by) {
				obj_from_char(o);
			}
			overflow_extract_tree(o);
		}
		if(blocked == SaveQuotaBlock::ObjContents) {
			send_to_char("Non puoi ritirarlo: contiene troppa roba per il tuo limite.\n\r", actor);
		}
		else {
			send_to_char("Porti gia' troppa roba da salvare; liberane un po' prima.\n\r", actor);
		}
		return false;
	}
	for(struct obj_data* obj : built) {
		obj_from_char(obj);
	}

	try {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		if(!overflow_soft_delete_claimed_tx(db, toon_id, overflow_ids)) {
			t.rollback();
			for(struct obj_data* o : built) {
				overflow_extract_tree(o);
			}
			send_to_char("Problema nel deposito. Avvisa un Dio.\n\r", actor);
			return false;
		}
		overflow_promote_orphans_tx(db, toon_id, overflow_ids);
		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "inventory_overflow_claim delete(%s): %s", owner_label.c_str(),
			   e.what());
		for(struct obj_data* o : built) {
			overflow_extract_tree(o);
		}
		send_to_char("Problema nel deposito. Avvisa un Dio.\n\r", actor);
		return false;
	}

	if(price > 0) {
		GET_GOLD(actor) -= price;
	}
	for(struct obj_data* obj : built) {
		obj_to_char(obj, actor);
	}
	schedule_inventory_save(actor);

	mudlog(LOG_PLAYERS,
		   "inventory_overflow_claim: %s claimed %zu item(s) from %s price=%ld%s",
		   GET_NAME(actor) ? GET_NAME(actor) : "?", overflow_ids.size(), owner_label.c_str(),
		   price, staff_other ? " (staff)" : "");
	return true;
}

#endif /* USE_MYSQL */

} // namespace Alarmud
