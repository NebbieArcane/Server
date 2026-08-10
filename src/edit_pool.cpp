/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "edit_pool.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <unordered_map>

#include "clan_symbol.hpp"
#include "config.hpp"
#include "constants.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "logging.hpp"
#include "object_instance.hpp"
#include "Sql.hpp"
#include "utils.hpp"

#if USE_MYSQL
#include "odb/account.hpp"
#include "odb/account-odb.hxx"
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>
#include <odb/transaction.hxx>
#endif

namespace Alarmud {

namespace {

struct PoolTotals {
	int hit{};
	int mana{};
	int move{};
	int hit_regen{};
	int mana_regen{};
	int move_regen{};

	void add(const PoolTotals& o) {
		hit += o.hit;
		mana += o.mana;
		move += o.move;
		hit_regen += o.hit_regen;
		mana_regen += o.mana_regen;
		move_regen += o.move_regen;
	}

	[[nodiscard]] bool empty() const noexcept {
		return hit == 0 && mana == 0 && move == 0 && hit_regen == 0 &&
			   mana_regen == 0 && move_regen == 0;
	}
};

[[nodiscard]] int sum_apply_arr(const struct obj_affected_type* aff, int location) {
	if(!aff) {
		return 0;
	}
	int tot = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(aff[i].location == location) {
			tot += aff[i].modifier;
		}
	}
	return tot;
}

[[nodiscard]] PoolTotals delta_affs(const struct obj_affected_type* edited,
									const struct obj_affected_type* proto) {
	PoolTotals d;
	d.hit = sum_apply_arr(edited, APPLY_HIT) - sum_apply_arr(proto, APPLY_HIT);
	d.mana = sum_apply_arr(edited, APPLY_MANA) - sum_apply_arr(proto, APPLY_MANA);
	d.move = sum_apply_arr(edited, APPLY_MOVE) - sum_apply_arr(proto, APPLY_MOVE);
	d.hit_regen =
		sum_apply_arr(edited, APPLY_HIT_REGEN) - sum_apply_arr(proto, APPLY_HIT_REGEN);
	d.mana_regen = sum_apply_arr(edited, APPLY_MANA_REGEN) -
				   sum_apply_arr(proto, APPLY_MANA_REGEN);
	d.move_regen = sum_apply_arr(edited, APPLY_MOVE_REGEN) -
				   sum_apply_arr(proto, APPLY_MOVE_REGEN);
	return d;
}

[[nodiscard]] PoolTotals delta_vs_proto(const struct obj_data* obj,
										const struct obj_data* proto) {
	static const struct obj_affected_type kEmpty[MAX_OBJ_AFFECT] = {};
	return delta_affs(obj ? obj->affected : kEmpty,
					  proto ? proto->affected : kEmpty);
}

void credit_one(sh_int* edit, sh_int* over, int amount, int cap) {
	if(amount <= 0 || !edit || !over) {
		return;
	}
	const int cur = static_cast<int>(*edit);
	const int room = std::max(0, cap - cur);
	const int to_edit = std::min(amount, room);
	*edit = static_cast<sh_int>(cur + to_edit);
	*over = static_cast<sh_int>(static_cast<int>(*over) + (amount - to_edit));
}

[[nodiscard]] std::string lower_copy(std::string s) {
	for(char& c : s) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return s;
}

[[nodiscard]] int resolve_base_vnum(const struct obj_data* obj) {
	if(!obj) {
		return 0;
	}
	int base = obj->char_vnum;
	if(base <= 0 ||
	   (base >= LOW_EDITED_ITEMS && base <= HIGH_EDITED_ITEMS)) {
		if(obj->item_number >= 0 && obj->item_number <= top_of_objt) {
			base = obj_index[obj->item_number].iVNum;
		}
	}
	if(base <= 0 ||
	   (base >= LOW_EDITED_ITEMS && base <= HIGH_EDITED_ITEMS)) {
		return 0;
	}
	return base;
}

[[nodiscard]] int current_obj_vnum(const struct obj_data* obj) {
	if(!obj || obj->item_number < 0 || obj->item_number > top_of_objt) {
		return 0;
	}
	return obj_index[obj->item_number].iVNum;
}

[[nodiscard]] bool vnum_in_edit_range(int vnum) noexcept {
	return vnum >= LOW_EDITED_ITEMS && vnum <= HIGH_EDITED_ITEMS;
}

/**
 * Solo pezzi edit: range 34k oppure istanza MySQL (edit gia' migrati, vnum=base).
 * Esclude toy/god gear e reward anche se flaggati ITEM2_EDIT per errore
 * (es. Ghost Sword 18020, focus DarkStar 65290).
 * I simboli di casata non entrano mai nel pool (type oppure vnum in lista).
 */
[[nodiscard]] bool is_edit_eligible_for_pool(const struct obj_data* obj) {
	if(!obj) {
		return false;
	}
	if(obj->obj_flags.type_flag == ITEM_CLAN_SYMBOL) {
		return false;
	}
	{
		const int cur = current_obj_vnum(obj);
		if(cur > 0 &&
		   clan_symbol_is_listed_vnum(static_cast<unsigned>(cur))) {
			return false;
		}
	}
	if(obj->db_instance_id != 0) {
		return true;
	}
	if(vnum_in_edit_range(current_obj_vnum(obj))) {
		return true;
	}
	if(vnum_in_edit_range(obj->char_vnum)) {
		return true;
	}
	return false;
}

/**
 * Edit eleggibile + PG proprietario (PERSONAL / ED*).
 */
[[nodiscard]] bool should_pool_migrate_for_holder(struct char_data* ch,
												  struct obj_data* obj) {
	if(!ch || !obj || !is_edit_eligible_for_pool(obj)) {
		return false;
	}
	return pers_on(ch, obj) != FALSE;
}

[[nodiscard]] struct obj_data* load_proto_tmp(const struct obj_data* obj) {
	const int base = resolve_base_vnum(obj);
	if(base <= 0) {
		return nullptr;
	}
	return read_object(base, VIRTUAL);
}

[[nodiscard]] const char* pool_loc_label(int location) noexcept {
	switch(location) {
	case APPLY_HIT:
		return "HIT";
	case APPLY_MANA:
		return "MANA";
	case APPLY_MOVE:
		return "MOVE";
	case APPLY_HIT_REGEN:
		return "HIT_REGEN";
	case APPLY_MANA_REGEN:
		return "MANA_REGEN";
	case APPLY_MOVE_REGEN:
		return "MOVE_REGEN";
	default:
		return "POOL";
	}
}

[[nodiscard]] std::string format_strip_detail(const struct obj_data* obj,
											  const PoolTotals& d) {
	std::ostringstream detail;
	detail << "strip pool";
	if(obj) {
		for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
			if(!edit_pool_is_pool_apply(obj->affected[i].location) ||
			   obj->affected[i].modifier == 0) {
				continue;
			}
			detail << "; -" << pool_loc_label(obj->affected[i].location) << " "
				   << obj->affected[i].modifier;
		}
	}
	detail << "; delta_credit hit=" << d.hit << " mana=" << d.mana
		   << " move=" << d.move << " hr=" << d.hit_regen << " mr=" << d.mana_regen
		   << " vr=" << d.move_regen;
	return detail.str();
}

#if USE_MYSQL
void emit_login_strip_event(struct obj_data* obj, struct char_data* ch,
							const PoolTotals& d) {
	if(!obj || !obj->db_instance_id) {
		return;
	}
	const std::string detail = format_strip_detail(obj, d);
	std::ostringstream note;
	note << "login strip";
	if(ch && GET_NAME(ch)) {
		note << " " << GET_NAME(ch);
	}
	object_instance_append_event(obj->db_instance_id, "edit_pool", note.str().c_str(),
								 detail.c_str(), "edit_pool_login", ch);
}

/**
 * true se questa instance ha gia' un event kind=edit_pool (convertita una volta).
 * In quel caso non riaccreditare: al massimo strip apply residue.
 */
[[nodiscard]] bool instance_has_edit_pool_event(unsigned long long instance_id) {
	if(instance_id == 0) {
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return false;
	}
	try {
		odb::transaction t(db->begin());
		using EvQ = odb::query<object_instance_event>;
		/* Non usare query_one: molte instance hanno piu' event edit_pool
		 * (boot + login) e ODB asserta se ne trova >1. */
		auto r = db->query<object_instance_event>(
			EvQ::instance_id == instance_id && EvQ::kind == "edit_pool");
		const bool found = !r.empty();
		t.commit();
		return found;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "edit_pool: instance_has_edit_pool_event(%llu): %s",
			   static_cast<unsigned long long>(instance_id), e.what());
		return false;
	}
}

[[nodiscard]] bool instance_already_pool_converted(
	unsigned long long instance_id,
	std::unordered_map<unsigned long long, bool>& cache) {
	if(instance_id == 0) {
		return false;
	}
	const auto it = cache.find(instance_id);
	if(it != cache.end()) {
		return it->second;
	}
	const bool v = instance_has_edit_pool_event(instance_id);
	cache.emplace(instance_id, v);
	return v;
}

[[nodiscard]] bool instance_has_edit_pool_event_tx(DB* db,
												  unsigned long long instance_id) {
	if(!db || instance_id == 0) {
		return false;
	}
	using EvQ = odb::query<object_instance_event>;
	auto r = db->query<object_instance_event>(
		EvQ::instance_id == instance_id && EvQ::kind == "edit_pool");
	return !r.empty();
}
#endif

void walk_objs(struct obj_data* list, PoolTotals& tot, bool strip,
			   struct char_data* owner_for_sync,
			   std::unordered_map<unsigned long long, bool>* converted_cache) {
	for(struct obj_data* obj = list; obj; obj = obj->next_content) {
		if(should_pool_migrate_for_holder(owner_for_sync, obj)) {
			struct obj_data* proto = load_proto_tmp(obj);
			const PoolTotals d = delta_vs_proto(obj, proto);
			if(!d.empty()) {
				bool already = false;
#if USE_MYSQL
				if(converted_cache && obj->db_instance_id) {
					already = instance_already_pool_converted(obj->db_instance_id,
															  *converted_cache);
				}
#endif
				if(!already) {
					tot.add(d);
				}
				if(strip) {
#if USE_MYSQL
					if(!already) {
						emit_login_strip_event(obj, owner_for_sync, d);
					}
					else {
						mudlog(LOG_CHECK,
							   "edit_pool: instance %llu already converted, strip "
							   "only (no re-credit)",
							   static_cast<unsigned long long>(obj->db_instance_id));
					}
#endif
					edit_pool_strip_obj(obj);
#if USE_MYSQL
					if(owner_for_sync && obj->db_instance_id) {
						object_instance_sync(obj, owner_for_sync);
					}
#else
					(void)owner_for_sync;
#endif
				}
			}
			if(proto) {
				extract_obj(proto);
			}
		}
		if(obj->contains) {
			walk_objs(obj->contains, tot, strip, owner_for_sync, converted_cache);
		}
	}
}

void strip_equipped_pool(struct char_data* ch, struct obj_data* obj,
						 PoolTotals& tot,
						 std::unordered_map<unsigned long long, bool>* converted_cache) {
	if(!ch || !obj) {
		return;
	}
	if(should_pool_migrate_for_holder(ch, obj)) {
		struct obj_data* proto = load_proto_tmp(obj);
		const PoolTotals d = delta_vs_proto(obj, proto);
		if(!d.empty()) {
			bool already = false;
#if USE_MYSQL
			if(converted_cache && obj->db_instance_id) {
				already = instance_already_pool_converted(obj->db_instance_id,
														  *converted_cache);
			}
#endif
			if(!already) {
				tot.add(d);
			}
			for(int a = 0; a < MAX_OBJ_AFFECT; ++a) {
				if(edit_pool_is_pool_apply(obj->affected[a].location) &&
				   obj->affected[a].modifier) {
					affect_modify(ch, obj->affected[a].location,
								  obj->affected[a].modifier, obj->obj_flags.bitvector,
								  FALSE);
				}
			}
#if USE_MYSQL
			if(!already) {
				emit_login_strip_event(obj, ch, d);
			}
			else {
				mudlog(LOG_CHECK,
					   "edit_pool: instance %llu already converted, strip only "
					   "(no re-credit)",
					   static_cast<unsigned long long>(obj->db_instance_id));
			}
#endif
			edit_pool_strip_obj(obj);
#if USE_MYSQL
			if(obj->db_instance_id) {
				object_instance_sync(obj, ch);
			}
#endif
		}
		if(proto) {
			extract_obj(proto);
		}
	}
	if(obj->contains) {
		walk_objs(obj->contains, tot, true, ch, converted_cache);
	}
}

#if USE_MYSQL
void apply_totals_to_stats_sql(DB* db, unsigned long long toon_id,
							   const PoolTotals& t) {
	odb::connection_ptr cp(db->connection());
	auto& mc = static_cast<odb::mysql::connection&>(*cp);
	MYSQL* h = mc.handle();

	std::ostringstream sel;
	sel << "SELECT edit_hp, edit_mana, edit_move, edit_hp_regen, edit_mana_regen, "
		   "edit_move_regen, overedit_hp, overedit_mana, overedit_move, "
		   "overedit_hp_regen, overedit_mana_regen, overedit_move_regen, "
		   "edit_pool_migrated FROM character_stats WHERE toon_id = "
		<< toon_id << " LIMIT 1";
	if(mysql_query(h, sel.str().c_str()) != 0) {
		mudlog(LOG_SYSERR, "edit_pool: SELECT failed for toon %llu: %s",
			   static_cast<unsigned long long>(toon_id), mysql_error(h));
		return;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(!res) {
		return;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	if(!row) {
		mysql_free_result(res);
		return;
	}

	char_edit_pool_data pool {};
	pool.edit_hp = static_cast<sh_int>(row[0] ? std::atoi(row[0]) : 0);
	pool.edit_mana = static_cast<sh_int>(row[1] ? std::atoi(row[1]) : 0);
	pool.edit_move = static_cast<sh_int>(row[2] ? std::atoi(row[2]) : 0);
	pool.edit_hp_regen = static_cast<sh_int>(row[3] ? std::atoi(row[3]) : 0);
	pool.edit_mana_regen = static_cast<sh_int>(row[4] ? std::atoi(row[4]) : 0);
	pool.edit_move_regen = static_cast<sh_int>(row[5] ? std::atoi(row[5]) : 0);
	pool.overedit_hp = static_cast<sh_int>(row[6] ? std::atoi(row[6]) : 0);
	pool.overedit_mana = static_cast<sh_int>(row[7] ? std::atoi(row[7]) : 0);
	pool.overedit_move = static_cast<sh_int>(row[8] ? std::atoi(row[8]) : 0);
	pool.overedit_hp_regen = static_cast<sh_int>(row[9] ? std::atoi(row[9]) : 0);
	pool.overedit_mana_regen =
		static_cast<sh_int>(row[10] ? std::atoi(row[10]) : 0);
	pool.overedit_move_regen =
		static_cast<sh_int>(row[11] ? std::atoi(row[11]) : 0);
	mysql_free_result(res);

	/* Credit anche se gia' migrated (strip boot puo' arrivare dopo un flag spurio). */
	edit_pool_credit_raw(&pool, t.hit, t.mana, t.move, t.hit_regen, t.mana_regen,
						 t.move_regen);
	pool.migrated = 1;

	std::ostringstream upd;
	upd << "UPDATE character_stats SET "
		<< "edit_hp=" << pool.edit_hp << ", edit_mana=" << pool.edit_mana
		<< ", edit_move=" << pool.edit_move
		<< ", edit_hp_regen=" << pool.edit_hp_regen
		<< ", edit_mana_regen=" << pool.edit_mana_regen
		<< ", edit_move_regen=" << pool.edit_move_regen
		<< ", overedit_hp=" << pool.overedit_hp
		<< ", overedit_mana=" << pool.overedit_mana
		<< ", overedit_move=" << pool.overedit_move
		<< ", overedit_hp_regen=" << pool.overedit_hp_regen
		<< ", overedit_mana_regen=" << pool.overedit_mana_regen
		<< ", overedit_move_regen=" << pool.overedit_move_regen
		<< ", edit_pool_migrated=1 WHERE toon_id=" << toon_id;
	db->execute(upd.str().c_str());

	{
		std::ostringstream msg;
		msg << "edit_pool: toon " << toon_id << " credited hit=" << pool.edit_hp
			<< "/" << pool.overedit_hp << " mana=" << pool.edit_mana << "/"
			<< pool.overedit_mana << " move=" << pool.edit_move << "/"
			<< pool.overedit_move << " hr=" << pool.edit_hp_regen << "/"
			<< pool.overedit_hp_regen << " mr=" << pool.edit_mana_regen << "/"
			<< pool.overedit_mana_regen << " vr=" << pool.edit_move_regen << "/"
			<< pool.overedit_move_regen << " (edit/over)";
		mudlog(LOG_CHECK, "%s", msg.str().c_str());
	}
}

[[nodiscard]] unsigned long long resolve_toon_id_ci(DB* db,
												   const std::string& name) {
	if(name.empty()) {
		return 0;
	}
	odb::connection_ptr cp(db->connection());
	auto& mc = static_cast<odb::mysql::connection&>(*cp);
	MYSQL* h = mc.handle();
	std::string esc = name;
	/* Escape minimale: solo nomi PG [A-Za-z0-9]. */
	std::ostringstream sql;
	sql << "SELECT id FROM toon WHERE LOWER(name) = LOWER('" << name
		<< "') LIMIT 1";
	(void)esc;
	if(mysql_query(h, sql.str().c_str()) != 0) {
		return 0;
	}
	MYSQL_RES* res = mysql_store_result(h);
	if(!res) {
		return 0;
	}
	MYSQL_ROW row = mysql_fetch_row(res);
	const unsigned long long id =
		(row && row[0]) ? strtoull(row[0], nullptr, 10) : 0ULL;
	mysql_free_result(res);
	return id;
}
#endif

} // namespace

bool edit_pool_is_pool_apply(int location) noexcept {
	switch(location) {
	case APPLY_HIT:
	case APPLY_MANA:
	case APPLY_MOVE:
	case APPLY_HIT_REGEN:
	case APPLY_MANA_REGEN:
	case APPLY_MOVE_REGEN:
		return true;
	default:
		return false;
	}
}

bool edit_pool_location_blocked_on_eq(int location) noexcept {
	return edit_pool_is_pool_apply(location);
}

void edit_pool_accumulate_obj_delta(const struct obj_data* obj,
									const struct obj_data* proto,
									struct char_edit_pool_data* add) {
	if(!obj || !add) {
		return;
	}
	const PoolTotals d = delta_vs_proto(obj, proto);
	add->edit_hp = static_cast<sh_int>(add->edit_hp + d.hit);
	add->edit_mana = static_cast<sh_int>(add->edit_mana + d.mana);
	add->edit_move = static_cast<sh_int>(add->edit_move + d.move);
	add->edit_hp_regen = static_cast<sh_int>(add->edit_hp_regen + d.hit_regen);
	add->edit_mana_regen =
		static_cast<sh_int>(add->edit_mana_regen + d.mana_regen);
	add->edit_move_regen =
		static_cast<sh_int>(add->edit_move_regen + d.move_regen);
}

bool edit_pool_strip_obj(struct obj_data* obj) {
	if(!obj) {
		return false;
	}
	bool changed = false;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(edit_pool_is_pool_apply(obj->affected[i].location)) {
			obj->affected[i].location = APPLY_NONE;
			obj->affected[i].modifier = 0;
			changed = true;
		}
	}
	return changed;
}

void edit_pool_credit_raw(struct char_edit_pool_data* pool, int hit, int mana,
						  int move, int hit_regen, int mana_regen,
						  int move_regen) {
	if(!pool) {
		return;
	}
	credit_one(&pool->edit_hp, &pool->overedit_hp, hit, kEditPoolMaxHit);
	credit_one(&pool->edit_mana, &pool->overedit_mana, mana, kEditPoolMaxMana);
	credit_one(&pool->edit_move, &pool->overedit_move, move, kEditPoolMaxMove);
	credit_one(&pool->edit_hp_regen, &pool->overedit_hp_regen, hit_regen,
			   kEditPoolMaxHitRegen);
	credit_one(&pool->edit_mana_regen, &pool->overedit_mana_regen, mana_regen,
			   kEditPoolMaxManaRegen);
	credit_one(&pool->edit_move_regen, &pool->overedit_move_regen, move_regen,
			   kEditPoolMaxMoveRegen);
}

void edit_pool_migrate_char(struct char_data* ch) {
	if(!ch || IS_NPC(ch)) {
		return;
	}

	PoolTotals tot;
	std::unordered_map<unsigned long long, bool> converted_cache;
	for(int i = 0; i < MAX_WEAR; ++i) {
		if(ch->equipment[i]) {
			strip_equipped_pool(ch, ch->equipment[i], tot, &converted_cache);
		}
	}
	walk_objs(ch->carrying, tot, true, ch, &converted_cache);

	/*
	 * Strip sempre (idempotente). Credit solo per instance senza event
	 * edit_pool (gia' convertite: strip-only, no riaccredito / no doppio credit
	 * su refund di eq con apply residue).
	 */
	if(!tot.empty()) {
		const bool first = !ch->edit_pool.migrated;
		edit_pool_credit_raw(&ch->edit_pool, tot.hit, tot.mana, tot.move,
							 tot.hit_regen, tot.mana_regen, tot.move_regen);
		ch->edit_pool.migrated = 1;
		std::ostringstream msg;
		msg << "edit_pool: " << GET_NAME(ch)
			<< (first ? " migrated from eq" : " credited leftover eq")
			<< " hit=" << tot.hit << " mana=" << tot.mana << " move=" << tot.move
			<< " hr=" << tot.hit_regen << " mr=" << tot.mana_regen
			<< " vr=" << tot.move_regen << " → edit " << ch->edit_pool.edit_hp
			<< "/" << ch->edit_pool.edit_mana << "/" << ch->edit_pool.edit_move
			<< " regen " << ch->edit_pool.edit_hp_regen << "/"
			<< ch->edit_pool.edit_mana_regen << "/"
			<< ch->edit_pool.edit_move_regen << " over "
			<< ch->edit_pool.overedit_hp << "/" << ch->edit_pool.overedit_mana
			<< "/" << ch->edit_pool.overedit_move << " regen "
			<< ch->edit_pool.overedit_hp_regen << "/"
			<< ch->edit_pool.overedit_mana_regen << "/"
			<< ch->edit_pool.overedit_move_regen;
		mudlog(LOG_CHECK, "%s", msg.str().c_str());
	}

	affect_total(ch);

#if USE_MYSQL
	/* PG migrati saltano save_char post-load: persisti solo il pool. */
	{
		DB* db = Sql::getMysql();
		if(db && GET_NAME(ch)) {
			const unsigned long long tid = resolve_toon_id_ci(db, GET_NAME(ch));
			if(tid) {
				std::ostringstream upd;
				upd << "UPDATE character_stats SET "
					<< "edit_hp=" << ch->edit_pool.edit_hp
					<< ", edit_mana=" << ch->edit_pool.edit_mana
					<< ", edit_move=" << ch->edit_pool.edit_move
					<< ", edit_hp_regen=" << ch->edit_pool.edit_hp_regen
					<< ", edit_mana_regen=" << ch->edit_pool.edit_mana_regen
					<< ", edit_move_regen=" << ch->edit_pool.edit_move_regen
					<< ", overedit_hp=" << ch->edit_pool.overedit_hp
					<< ", overedit_mana=" << ch->edit_pool.overedit_mana
					<< ", overedit_move=" << ch->edit_pool.overedit_move
					<< ", overedit_hp_regen=" << ch->edit_pool.overedit_hp_regen
					<< ", overedit_mana_regen=" << ch->edit_pool.overedit_mana_regen
					<< ", overedit_move_regen=" << ch->edit_pool.overedit_move_regen
					<< ", edit_pool_migrated="
					<< static_cast<int>(ch->edit_pool.migrated)
					<< " WHERE toon_id=" << tid;
				try {
					odb::transaction t(db->begin());
					db->execute(upd.str().c_str());
					t.commit();
				}
				catch(const odb::exception& e) {
					mudlog(LOG_SYSERR, "edit_pool: persist %s failed: %s",
						   GET_NAME(ch), e.what());
				}
			}
		}
	}
#endif
}

void edit_pool_boot_migrate() {
#if !USE_MYSQL
	return;
#else
	DB* db = Sql::getMysql();
	if(!db) {
		mudlog(LOG_CHECK, "edit_pool_boot_migrate: no MySQL, skip");
		return;
	}

	/* Cintura: se ODB e' ancora a schema 1.7 senza regen, crea comunque le colonne. */
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		const char* probe =
			"SELECT COUNT(*) FROM information_schema.COLUMNS "
			"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'character_stats' "
			"AND COLUMN_NAME = 'edit_hp'";
		if(mysql_query(h, probe) == 0) {
			MYSQL_RES* res = mysql_store_result(h);
			MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
			const long have = (row && row[0]) ? std::strtol(row[0], nullptr, 10) : 0;
			if(res) {
				mysql_free_result(res);
			}
			if(have == 0) {
				mudlog(LOG_CHECK,
					   "edit_pool_boot_migrate: adding character_stats edit_* columns "
					   "(schema fallback)");
				db->execute(
					"ALTER TABLE character_stats "
					"ADD COLUMN edit_hp SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN edit_mana SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN edit_move SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN edit_hp_regen SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN edit_mana_regen SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN edit_move_regen SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN overedit_hp SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN overedit_mana SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN overedit_move SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN overedit_hp_regen SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN overedit_mana_regen SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN overedit_move_regen SMALLINT NOT NULL DEFAULT 0, "
					"ADD COLUMN edit_pool_migrated TINYINT UNSIGNED NOT NULL DEFAULT 0");
			}
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "edit_pool_boot_migrate: column ensure failed: %s",
			   e.what());
		return;
	}

	std::unordered_map<std::string, PoolTotals> by_owner;
	int stripped = 0;
	int scanned = 0;
	int events = 0;

	try {
		odb::transaction t(db->begin());
		using Q = odb::query<object_instance>;
		using AffQ = odb::query<object_instance_affect>;

		for(const auto& row : db->query<object_instance>(Q::deleted == false)) {
			++scanned;
			if(row.type_flag == ITEM_CLAN_SYMBOL) {
				continue;
			}
			if(!row.legacy_edit_vnum.null() &&
			   clan_symbol_is_listed_vnum(row.legacy_edit_vnum.get())) {
				continue;
			}
			struct obj_affected_type affs[MAX_OBJ_AFFECT];
			std::memset(affs, 0, sizeof(affs));
			bool has_pool = false;
			for(const auto& af : db->query<object_instance_affect>(
					AffQ::key.instance_id == row.id)) {
				if(af.key.affect_slot >= MAX_OBJ_AFFECT) {
					continue;
				}
				affs[af.key.affect_slot].location = af.location;
				affs[af.key.affect_slot].modifier = af.modifier;
				if(edit_pool_is_pool_apply(af.location)) {
					has_pool = true;
				}
			}
			if(!has_pool) {
				continue;
			}

			struct obj_affected_type proto_affs[MAX_OBJ_AFFECT];
			std::memset(proto_affs, 0, sizeof(proto_affs));
			struct obj_data* proto =
				read_object(static_cast<int>(row.base_vnum), VIRTUAL);
			if(proto) {
				std::memcpy(proto_affs, proto->affected, sizeof(proto_affs));
				extract_obj(proto);
			}

			const PoolTotals d = delta_affs(affs, proto_affs);
			if(d.empty()) {
				continue;
			}

			const bool already = instance_has_edit_pool_event_tx(db, row.id);

			std::string owner;
			if(!row.owner_name.null()) {
				owner = row.owner_name.get();
			}
			if(!already) {
				if(owner.empty()) {
					mudlog(LOG_CHECK,
						   "edit_pool_boot: instance %llu pool delta, no owner "
						   "(strip only)",
						   static_cast<unsigned long long>(row.id));
				}
				else {
					by_owner[lower_copy(owner)].add(d);
				}
			}
			else {
				mudlog(LOG_CHECK,
					   "edit_pool_boot: instance %llu already converted, strip "
					   "only (no re-credit)",
					   static_cast<unsigned long long>(row.id));
			}

			std::ostringstream detail;
			detail << (already ? "strip only already converted" : "strip pool from instance");
			if(!owner.empty()) {
				detail << " owner=" << owner;
			}
			detail << " base=" << row.base_vnum;
			if(!row.legacy_edit_vnum.null()) {
				detail << " legacy=" << row.legacy_edit_vnum.get();
			}
			for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
				if(!edit_pool_is_pool_apply(affs[i].location) ||
				   affs[i].modifier == 0) {
					continue;
				}
				detail << "; -" << pool_loc_label(affs[i].location) << " "
					   << affs[i].modifier;
			}
			if(!already) {
				detail << "; delta_credit hit=" << d.hit << " mana=" << d.mana
					   << " move=" << d.move << " hr=" << d.hit_regen
					   << " mr=" << d.mana_regen << " vr=" << d.move_regen;
			}

			for(unsigned char slot = 0; slot < MAX_OBJ_AFFECT; ++slot) {
				if(!edit_pool_is_pool_apply(affs[slot].location)) {
					continue;
				}
				try {
					object_instance_affect_key key;
					key.instance_id = row.id;
					key.affect_slot = slot;
					object_instance_affect af;
					db->load<object_instance_affect>(key, af);
					af.location = APPLY_NONE;
					af.modifier = 0;
					db->update(af);
					++stripped;
				}
				catch(const odb::exception&) {
				}
			}

			if(!already) {
				std::ostringstream note;
				note << "credit to " << (owner.empty() ? "?" : owner);
				object_instance_append_event_tx(db, row.id, "edit_pool",
												note.str().c_str(),
												detail.str().c_str(),
												"edit_pool_boot", nullptr);
				++events;
			}
		}

		for(const auto& kv : by_owner) {
			const unsigned long long tid = resolve_toon_id_ci(db, kv.first);
			if(tid == 0) {
				mudlog(LOG_CHECK,
					   "edit_pool_boot: owner '%s' missing in toon, credits "
					   "skipped (affects stripped)",
					   kv.first.c_str());
				continue;
			}
			apply_totals_to_stats_sql(db, tid, kv.second);
		}

		t.commit();
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "edit_pool_boot_migrate: %s", e.what());
		return;
	}

	{
		std::ostringstream msg;
		msg << "edit_pool_boot_migrate: scanned " << scanned << " instances, stripped "
			<< stripped << " affect slots, owners " << by_owner.size() << ", events "
			<< events;
		mudlog(LOG_CHECK, "%s", msg.str().c_str());
	}
#endif
}

} // namespace Alarmud
