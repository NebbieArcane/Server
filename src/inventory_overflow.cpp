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
#include "act.other.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "object_instance.hpp"
#include "procarea.hpp"
#include "Sql.hpp"
#include "toon_migration.hpp"
#include "utility.hpp"

#if USE_MYSQL
#include <mysql/mysql.h>
#include <odb/database.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/transaction.hxx>
#endif

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace Alarmud {

#if !USE_MYSQL

bool inventory_overflow_begin_persist(struct char_data* /*ch*/, InventoryOverflowHold& hold) {
	hold = {};
	return false;
}

void inventory_overflow_finish_persist(InventoryOverflowHold& hold, bool /*reattach*/) {
	hold = {};
}

void inventory_overflow_checkpoint_pulse(unsigned long /*now_pulse*/) {}

void inventory_overflow_expire_stale() {}

long inventory_overflow_claim_price(unsigned long long /*overflow_id*/) {
	return 0;
}

bool inventory_overflow_claim(struct char_data* /*ch*/, unsigned long long /*overflow_id*/) {
	return false;
}

#else /* USE_MYSQL */

namespace {

constexpr long kOverflowClaimPriceK = 100; /* potenza * K; tarare */

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

void overflow_collect_candidates(struct char_data* ch, int need,
								 std::vector<struct obj_data*>& roots) {
	std::unordered_set<struct obj_data*> marked;
	/* 1-2: carrying (prima non protetti, poi protetti), contenuti dal fondo. */
	overflow_collect_from_list(ch->carrying, need, roots, marked, false);
	overflow_collect_from_list(ch->carrying, need, roots, marked, true);
	/* 3: contenuti di container wearati dal fondo; mai il pezzo wearato. */
	for(int i = 0; i < MAX_WEAR && need > 0; ++i) {
		struct obj_data* worn = ch->equipment[i];
		if(!worn || !worn->contains) {
			continue;
		}
		overflow_collect_from_list(worn->contains, need, roots, marked, true);
	}
	if(need > 0) {
		mudlog(LOG_SYSERR,
			   "inventory_overflow: still need %d after split for %s (worn-heavy?)",
			   need, GET_NAME(ch) ? GET_NAME(ch) : "?");
	}
}

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

unsigned long long overflow_insert_obj_tx(
	DB* db, const std::string& toon_id, struct obj_data* obj,
	unsigned long long parent_overflow_id, int list_index, int depth,
	const std::unordered_set<struct obj_data*>& separate_roots) {
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

	for(int a = 0; a < MAX_OBJ_AFFECT; ++a) {
		const obj_affected_type& oa = obj->affected[a];
		if(oa.location == 0 && oa.modifier == 0) {
			continue;
		}
		std::ostringstream aff;
		aff << "INSERT INTO character_inventory_overflow_affect "
			   "(overflow_id, affect_slot, location, modifier) VALUES ("
			<< id << ',' << a << ',' << static_cast<int>(oa.location) << ','
			<< static_cast<int>(oa.modifier) << ')';
		db->execute(aff.str().c_str());
	}

	for(struct obj_data* c = obj->contains; c; c = c->next_content) {
		if(separate_roots.count(c) != 0) {
			continue;
		}
		overflow_insert_obj_tx(db, toon_id, c, id, 0, depth + 1, separate_roots);
	}
	return id;
}

/** Replace/clear snapshot: hard-delete set attivo (affect in CASCADE).
 * Soft-delete resta per OVERFLOW_EXPIRE / CLAIMED / NUKE. */
void overflow_hard_delete_active_tx(DB* db, const std::string& toon_id) {
	db->execute(("DELETE FROM character_inventory_overflow WHERE toon_id = " + toon_id +
				 " AND (deleted = 0 OR deleted IS NULL)")
					.c_str());
}

bool overflow_replace_active_tx(DB* db, const std::string& toon_id,
								const std::vector<struct obj_data*>& roots) {
	overflow_hard_delete_active_tx(db, toon_id);
	if(roots.empty()) {
		return true;
	}
	const std::unordered_set<struct obj_data*> separate_roots(roots.begin(), roots.end());
	int list_index = 0;
	for(struct obj_data* r : roots) {
		if(overflow_insert_obj_tx(db, toon_id, r, 0, list_index++, 0, separate_roots) == 0) {
			return false;
		}
	}
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

		if(total <= MAX_OBJ_SAVE) {
			odb::transaction t(db->begin());
			t.tracer(logTracer);
			overflow_hard_delete_active_tx(db, toon_id);
			t.commit();
			mudlog(LOG_CHECK, "inventory_overflow: cleared active overflow for %s (have %d)",
				   GET_NAME(pc), total);
			return true;
		}

		int need = total - MAX_OBJ_SAVE;
		std::vector<struct obj_data*> roots;
		overflow_collect_candidates(ch, need, roots);
		if(roots.empty()) {
			mudlog(LOG_SYSERR, "inventory_overflow: no candidates for %s (have %d)",
				   GET_NAME(pc), total);
			hold = {};
			return false;
		}

		for(struct obj_data* r : roots) {
			overflow_detach_to_hold(r, hold);
		}

		odb::transaction t(db->begin());
		t.tracer(logTracer);
		if(!overflow_replace_active_tx(db, toon_id, hold.roots)) {
			mudlog(LOG_SYSERR, "inventory_overflow: replace failed for %s", GET_NAME(pc));
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
		   "inventory_overflow: persist %zu root(s) for %s (was %d, cap %d) — save then finish",
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
		do_save_rent(ch);
		inventory_overflow_finish_persist(hold, true);
		done_this_round.insert(key);
		return;
	}
	drain = false;
	done_this_round.clear();
}

long inventory_overflow_claim_price(unsigned long long overflow_id) {
	if(overflow_id == 0) {
		return 0;
	}
	return std::max(1000L, kOverflowClaimPriceK);
}

bool inventory_overflow_claim(struct char_data* ch, unsigned long long overflow_id) {
	if(!ch || overflow_id == 0) {
		return false;
	}
	mudlog(LOG_CHECK, "inventory_overflow_claim: stub (id=%llu) for %s",
		   static_cast<unsigned long long>(overflow_id),
		   GET_NAME(ch) ? GET_NAME(ch) : "?");
	return false;
}

#endif /* USE_MYSQL */

} // namespace Alarmud
