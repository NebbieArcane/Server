/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "typedefs.hpp"
#include "flags.hpp"
#include "autoenums.hpp"
#include "structs.hpp"
#include "logging.hpp"
#include "edit_portal_listino.hpp"
#include "edit_listino.hpp"
#include "edit_pool.hpp"
#include "obj_value.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#if USE_MYSQL
#include "Sql.hpp"
#include <odb/mysql/connection.hxx>
#include <mysql/mysql.h>
#endif

namespace Alarmud {
namespace {

std::string g_revision;

[[nodiscard]] bool row_usable_obj(const EditListinoEntry& e) noexcept {
	return e.mud_supported != 0 && e.enabled != 0 && e.on_obj != 0;
}

[[nodiscard]] bool row_usable_pc(const EditListinoEntry& e) noexcept {
	return e.mud_supported != 0 && e.enabled != 0 && e.on_pc != 0;
}

[[nodiscard]] bool is_resist_bit_loc(int loc) noexcept {
	return loc == APPLY_IMMUNE || loc == APPLY_M_IMMUNE || loc == APPLY_SUSC;
}

[[nodiscard]] bool is_pool_apply_loc(int loc) noexcept {
	return loc == APPLY_HIT || loc == APPLY_MANA || loc == APPLY_MOVE ||
		   loc == APPLY_HIT_REGEN || loc == APPLY_MANA_REGEN ||
		   loc == APPLY_MOVE_REGEN || loc == APPLY_SPELLFAIL;
}

[[nodiscard]] long gcd_long(long a, long b) noexcept {
	a = a < 0 ? -a : a;
	b = b < 0 ? -b : b;
	while(b != 0) {
		const long t = a % b;
		a = b;
		b = t;
	}
	return a == 0 ? 1 : a;
}

/** Id storico del portale (app.js usa 'armor' e 'int'). Puntatore stabile. */
[[nodiscard]] const char* portal_id_for(const EditListinoEntry& e) noexcept {
	if(e.code == "ac") {
		return "armor";
	}
	if(e.code == "intel") {
		return "int";
	}
	return e.code.c_str();
}

/** Riga numerica su oggetto (APPLY senza bit, non pool PG). */
[[nodiscard]] bool is_obj_scalar_row(const EditListinoEntry& e) noexcept {
	return row_usable_obj(e) && e.apply_loc > 0 && e.apply_bit == 0UL &&
		   !is_pool_apply_loc(e.apply_loc);
}

[[nodiscard]] long malus_raw(long pos_raw) noexcept {
	const auto& r = edit_listino_rules_cache();
	const int den = r.malus_mult_den > 0 ? r.malus_mult_den : 1;
	return static_cast<long>((static_cast<long long>(pos_raw) * r.malus_mult_num) / den);
}

/** Raw CheckValueObj per UNA unita' (xp della riga e' per `step` unita'). */
[[nodiscard]] long unit_raw(const EditListinoEntry& e) noexcept {
	const int step = edit_listino_entry_step(e);
	const double xp_unit = static_cast<double>(e.xp) / static_cast<double>(step);
	return static_cast<long>(
		std::llround(xp_unit / static_cast<double>(kObjValueStorageScale)));
}

void fill_scalar_spec(const EditListinoEntry& e, ObjEditListinoSpec& out) noexcept {
	const int step = edit_listino_entry_step(e);
	const int cap = std::abs(e.cap_obj);
	out.location = e.apply_loc;
	out.id = portal_id_for(e);
	out.label = e.label.c_str();
	out.step = e.sign < 0 ? -step : step;
	out.min_total = e.sign < 0 ? -cap : 0;
	out.max_total = e.sign < 0 ? 0 : cap;
	out.positive_unit_raw = unit_raw(e);
	out.negative_unit_raw = malus_raw(out.positive_unit_raw);
}

void fill_bit_spec(const EditListinoEntry& e, ObjEditListinoSpec& out) noexcept {
	out.location = e.apply_loc;
	out.id = portal_id_for(e);
	out.label = e.label.c_str();
	out.step = 1;
	out.min_total = 0;
	out.max_total = e.cap_obj > 0 ? e.cap_obj : 1;
	out.positive_unit_raw = unit_raw(e);
	out.negative_unit_raw = malus_raw(out.positive_unit_raw);
}

void unit_rune(const EditListinoEntry& e, int& rune_num, int& rune_den) noexcept {
	long num = e.rune_num;
	long den = static_cast<long>(e.rune_den > 0 ? e.rune_den : 1) *
			   static_cast<long>(edit_listino_entry_step(e));
	const long g = gcd_long(num, den);
	num /= g;
	den /= g;
	rune_num = static_cast<int>(num);
	rune_den = static_cast<int>(den > 0 ? den : 1);
}

[[nodiscard]] const EditListinoEntry* find_scalar(int location) {
	if(location <= 0) {
		return nullptr;
	}
	const EditListinoEntry* e = edit_listino_find_apply(location, 0UL);
	if(e == nullptr || !is_obj_scalar_row(*e)) {
		return nullptr;
	}
	return e;
}

[[nodiscard]] const EditListinoEntry* find_bit(int location, unsigned long bit) {
	if(bit == 0UL || !is_resist_bit_loc(location)) {
		return nullptr;
	}
	const EditListinoEntry* e = edit_listino_find_apply(location, bit);
	if(e == nullptr || !row_usable_obj(*e)) {
		return nullptr;
	}
	return e;
}

[[nodiscard]] std::vector<const EditListinoEntry*> scalar_rows() {
	std::vector<const EditListinoEntry*> out;
	for(const auto& e : edit_listino_all_rows()) {
		if(!is_obj_scalar_row(e)) {
			continue;
		}
		/* Una sola voce per APPLY (vince quella indicizzata dal MUD). */
		if(edit_listino_find_apply(e.apply_loc, 0UL) != &e) {
			continue;
		}
		out.push_back(&e);
	}
	return out;
}

[[nodiscard]] double frac(int num, int den, double fallback) noexcept {
	if(den <= 0) {
		return fallback;
	}
	return static_cast<double>(num) / static_cast<double>(den);
}

#if USE_MYSQL
[[nodiscard]] std::string read_revision() {
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return {};
	}
	const char* sql =
		"SELECT IFNULL(UNIX_TIMESTAMP(MAX(updated_at)),0), CONCAT("
		"(SELECT CONCAT(IFNULL(UNIX_TIMESTAMP(MAX(updated_at)),0),'.',COUNT(*)) FROM edit_listino),"
		"'/',"
		"(SELECT CONCAT(IFNULL(UNIX_TIMESTAMP(MAX(updated_at)),0),'.',COUNT(*)) FROM edit_listino_rules),"
		"'/',"
		"(SELECT CONCAT(IFNULL(UNIX_TIMESTAMP(MAX(updated_at)),0),'.',COUNT(*)) FROM edit_listino_cap_group)) "
		"FROM edit_listino";
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		if(mysql_query(h, sql) != 0) {
			mudlog(LOG_SYSERR, "edit_portal_listino: revisione: %s", mysql_error(h));
			return {};
		}
		std::string out;
		if(MYSQL_RES* res = mysql_store_result(h)) {
			if(MYSQL_ROW row = mysql_fetch_row(res)) {
				out = std::string("edit_listino@") + (row[0] ? row[0] : "0") + "#" +
					  (row[1] ? row[1] : "");
			}
			mysql_free_result(res);
		}
		return out;
	}
	catch(const std::exception& e) {
		mudlog(LOG_SYSERR, "edit_portal_listino: revisione: %s", e.what());
		return {};
	}
}
#endif

} // namespace

bool obj_edit_listino_spec(int location, ObjEditListinoSpec& out) noexcept {
	return portal_listino_affect_by_location(location, out);
}

int obj_edit_listino_scalar_count() noexcept {
	return portal_listino_affect_count();
}

bool obj_edit_listino_scalar_at(int index, ObjEditListinoSpec& out) noexcept {
	return portal_listino_affect_at(index, out);
}

bool portal_listino_ready() noexcept {
	try {
		return !edit_listino_all_rows().empty();
	}
	catch(...) {
		return false;
	}
}

int portal_listino_affect_count() noexcept {
	try {
		return static_cast<int>(scalar_rows().size());
	}
	catch(...) {
		return 0;
	}
}

bool portal_listino_affect_at(int index, ObjEditListinoSpec& out) noexcept {
	try {
		const auto rows = scalar_rows();
		if(index < 0 || index >= static_cast<int>(rows.size())) {
			return false;
		}
		fill_scalar_spec(*rows[static_cast<std::size_t>(index)], out);
		return true;
	}
	catch(...) {
		return false;
	}
}

bool portal_listino_affect_by_location(int location, ObjEditListinoSpec& out) noexcept {
	try {
		const EditListinoEntry* e = find_scalar(location);
		if(e == nullptr) {
			return false;
		}
		fill_scalar_spec(*e, out);
		return true;
	}
	catch(...) {
		return false;
	}
}

bool portal_listino_affect_rune(int location, int& rune_num, int& rune_den) noexcept {
	try {
		const EditListinoEntry* e = find_scalar(location);
		if(e == nullptr) {
			return false;
		}
		unit_rune(*e, rune_num, rune_den);
		return true;
	}
	catch(...) {
		return false;
	}
}

bool portal_listino_affect_by_bit(int location, unsigned long apply_bit,
								  ObjEditListinoSpec& out) noexcept {
	try {
		const EditListinoEntry* e = find_bit(location, apply_bit);
		if(e == nullptr) {
			return false;
		}
		fill_bit_spec(*e, out);
		return true;
	}
	catch(...) {
		return false;
	}
}

bool portal_listino_affect_rune_bit(int location, unsigned long apply_bit, int& rune_num,
									int& rune_den) noexcept {
	try {
		const EditListinoEntry* e = find_bit(location, apply_bit);
		if(e == nullptr) {
			return false;
		}
		unit_rune(*e, rune_num, rune_den);
		return true;
	}
	catch(...) {
		return false;
	}
}

std::string portal_listino_affect_cap_group(int location) {
	const EditListinoEntry* e = find_scalar(location);
	return e != nullptr ? e->cap_group : std::string();
}

double portal_listino_param(const char* key, double fallback) noexcept {
	if(key == nullptr) {
		return fallback;
	}
	const auto& r = edit_listino_rules_cache();
	const double scale = static_cast<double>(kObjValueStorageScale);
	if(std::strcmp(key, "storage_scale") == 0) {
		return scale;
	}
	if(std::strcmp(key, "broker_fee_percent") == 0) {
		const auto [n, d] = edit_listino_broker_fee_frac();
		return frac(n, d, fallback);
	}
	if(std::strcmp(key, "artifact_mult") == 0) {
		return frac(r.artifact_mult_num, r.artifact_mult_den, fallback);
	}
	if(std::strcmp(key, "artifact_cost_xp") == 0 ||
	   std::strcmp(key, "artifact_cost_raw") == 0) {
		double xp = frac(r.artifact_cost_xp_num, r.artifact_cost_xp_den, 0.0);
		if(xp <= 0.0) {
			const EditListinoEntry* row = edit_listino_find("artifact");
			xp = (row != nullptr && row->xp > 0) ? static_cast<double>(row->xp) : 0.0;
		}
		if(xp <= 0.0) {
			return fallback;
		}
		return std::strcmp(key, "artifact_cost_raw") == 0 ? xp / scale : xp;
	}
	if(std::strcmp(key, "class_mult_1") == 0) {
		return edit_listino_class_mult(1);
	}
	if(std::strcmp(key, "class_mult_bi") == 0 || std::strcmp(key, "class_mult_2") == 0) {
		return edit_listino_class_mult(2);
	}
	if(std::strcmp(key, "class_mult_tri") == 0 || std::strcmp(key, "class_mult_3") == 0) {
		return edit_listino_class_mult(3);
	}
	if(std::strcmp(key, "malus_mult") == 0) {
		return frac(r.malus_mult_num, r.malus_mult_den, fallback);
	}
	return fallback;
}

std::string portal_listino_seed_tag() {
	if(g_revision.empty()) {
		portal_listino_refresh_revision();
	}
	return g_revision;
}

void portal_listino_refresh_revision() {
#if USE_MYSQL
	g_revision = read_revision();
#endif
	if(g_revision.empty()) {
		g_revision = "edit_listino@seed#" + std::to_string(edit_listino_all_rows().size());
	}
}

bool portal_listino_reload() {
	edit_listino_reload();
	portal_listino_refresh_revision();
	return portal_listino_ready();
}

std::vector<PortalListinoRowStatus> portal_listino_row_status() {
	std::vector<PortalListinoRowStatus> out;
	for(const auto& e : edit_listino_all_rows()) {
		if(e.code.empty()) {
			continue;
		}
		PortalListinoRowStatus st;
		st.code = e.code;
		st.enabled = e.enabled != 0;
		st.supported = e.mud_supported != 0;
		if(e.apply_loc == 0 && e.apply_bit == static_cast<unsigned long>(ITEM_IMMUNE)) {
			st.kind = "flag";
		}
		else if(is_pool_apply_loc(e.apply_loc) && e.apply_bit == 0UL && e.on_pc) {
			st.kind = "pool";
		}
		else if(is_resist_bit_loc(e.apply_loc) && e.apply_bit != 0UL) {
			st.kind = "bit";
		}
		else if((e.apply_loc == APPLY_SPELL || e.apply_loc == APPLY_AFF2 ||
				 e.apply_loc == APPLY_WEAPON_SPELL) &&
				e.apply_bit != 0UL) {
			st.kind = "spell";
		}
		else if(e.apply_loc > 0 && e.apply_bit == 0UL) {
			st.kind = "affect";
		}
		else {
			st.kind = "other";
		}
		const bool usable = row_usable_obj(e) || row_usable_pc(e);
		st.active = usable;
		if(!st.supported) {
			st.reason = "non supportata da questo MUD (mud_supported=0)";
		}
		else if(!st.enabled) {
			st.reason = "spenta (enabled=0)";
		}
		else if(!e.on_obj && !e.on_pc) {
			st.reason = "on_obj=0 e on_pc=0: non vendibile";
		}
		out.push_back(std::move(st));
	}
	return out;
}

bool portal_pool_parse_field_key(const char* key, EditPoolField& out) noexcept {
	if(key == nullptr) {
		return false;
	}
	struct Map {
		const char* key;
		EditPoolField field;
	};
	static const Map kMap[] = {
		{"hit", EditPoolField::Hp},
		{"hp", EditPoolField::Hp},
		{"mana", EditPoolField::Mana},
		{"move", EditPoolField::Move},
		{"hit_regen", EditPoolField::HpRegen},
		{"hp_regen", EditPoolField::HpRegen},
		{"mana_regen", EditPoolField::ManaRegen},
		{"move_regen", EditPoolField::MoveRegen},
	};
	for(const auto& m : kMap) {
		if(std::strcmp(key, m.key) == 0) {
			out = m.field;
			return true;
		}
	}
	return false;
}

namespace {

[[nodiscard]] const char* pool_listino_code(EditPoolField field) noexcept {
	switch(field) {
	case EditPoolField::Hp:
		return "hit";
	case EditPoolField::Mana:
		return "mana";
	case EditPoolField::Move:
		return "move";
	case EditPoolField::HpRegen:
		return "hit_regen";
	case EditPoolField::ManaRegen:
		return "mana_regen";
	case EditPoolField::MoveRegen:
		return "move_regen";
	case EditPoolField::Spellfail:
		return "spellfail";
	}
	return "hit";
}

} // namespace

int portal_pool_field_step(EditPoolField field) noexcept {
	const EditListinoEntry* e = edit_listino_find(pool_listino_code(field));
	if(e != nullptr && e->buy_step > 1) {
		return e->buy_step;
	}
	switch(field) {
	case EditPoolField::Hp:
	case EditPoolField::Mana:
	case EditPoolField::Move:
		return 10;
	case EditPoolField::HpRegen:
	case EditPoolField::ManaRegen:
	case EditPoolField::MoveRegen:
		return 5;
	case EditPoolField::Spellfail:
		return 1;
	}
	return 1;
}

portal_pool_quote portal_pool_quote_paid(EditPoolField field, int units, int n_classes) {
	portal_pool_quote q;
	units = std::abs(units);
	if(units <= 0) {
		return q;
	}
	const auto [xp, pq] = editpool_cost_for_units_n(field, units, std::max(1, n_classes));
	q.xp_raw = std::max(0L, xp);
	q.pq = std::max(0, pq);
	q.mxp = q.xp_raw / 1000000L;
	q.mxp_frac = (q.xp_raw % 1000000L) / 10000L;
	return q;
}

double portal_pool_class_mult(int n_classes) noexcept {
	return edit_listino_class_mult(std::max(1, n_classes));
}

bool portal_pool_set_absolute(struct char_edit_pool_data* pool, EditPoolField field,
							  int value) noexcept {
	if(pool == nullptr) {
		return false;
	}
	const int cap = edit_pool_field_cap(field);
	const sh_int v = static_cast<sh_int>(std::clamp(value, 0, std::max(0, cap)));
	switch(field) {
	case EditPoolField::Hp:
		pool->edit_hp = v;
		return true;
	case EditPoolField::Mana:
		pool->edit_mana = v;
		return true;
	case EditPoolField::Move:
		pool->edit_move = v;
		return true;
	case EditPoolField::HpRegen:
		pool->edit_hp_regen = v;
		return true;
	case EditPoolField::ManaRegen:
		pool->edit_mana_regen = v;
		return true;
	case EditPoolField::MoveRegen:
		pool->edit_move_regen = v;
		return true;
	case EditPoolField::Spellfail:
		return false;
	}
	return false;
}

} // namespace Alarmud
