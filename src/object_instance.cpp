/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* object_instance.cpp - CRUD MySQL per eq editato (edit db). */
#include "object_instance.hpp"

#if USE_MYSQL

#include "logging.hpp"
#include "structs.hpp"
#include "utils.hpp"
#include "autoenums.hpp"
#include "constants.hpp"
#include "db.hpp"
#include "comm.hpp"
#include "handler.hpp"
#include "Sql.hpp"
#include "odb/account-odb.hxx"
#include "utility.hpp"
#include "spell_parser.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/mysql/database.hxx>
#include <mysql/mysql.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace Alarmud {

namespace {

const char* safe_cstr(const char* s) {
	return s ? s : "";
}

void set_nullable_name(odb::nullable<std::string>& field, const std::string& name) {
	if(name.empty()) {
		field = odb::nullable<std::string>();
	}
	else {
		field = name;
	}
}

void set_nullable_id(odb::nullable<unsigned long long>& field, unsigned long long id) {
	if(id == 0) {
		field = odb::nullable<unsigned long long>();
	}
	else {
		field = id;
	}
}

unsigned long long lookup_toon_id_tx(DB* db, const std::string& name) {
	if(name.empty()) {
		return 0;
	}
	toon pg;
	if(db->query_one<toon>(odb::query<toon>::name == name, pg)) {
		return pg.id;
	}
	return 0;
}

void copy_personal_owner(struct obj_data* obj, const std::string& owner) {
	if(!obj) {
		return;
	}
	if(owner.empty()) {
		obj->personal_owner[0] = '\0';
		return;
	}
	strncpy(obj->personal_owner, owner.c_str(), sizeof(obj->personal_owner) - 1);
	obj->personal_owner[sizeof(obj->personal_owner) - 1] = '\0';
}

std::string resolve_owner_name(const struct obj_data* obj, char_data* actor) {
	if(obj && obj->personal_owner[0] != '\0') {
		return obj->personal_owner;
	}
	const std::string from_ed = object_instance_extract_ed_owner(obj ? obj->name : nullptr);
	if(!from_ed.empty()) {
		return from_ed;
	}
	/* Solo per PERSONAL: il wiz che salva non diventa owner di eq generico. */
	if(obj && IS_OBJ_STAT2(obj, ITEM2_PERSONAL) && actor && !IS_NPC(actor) &&
	   GET_NAME(actor)) {
		return GET_NAME(actor);
	}
	return {};
}

void append_instance_event_tx(DB* db, unsigned long long instance_id, char_data* actor,
							  const char* kind, const char* note, const char* detail) {
	object_instance_event ev {};
	ev.instance_id = instance_id;
	ev.at = boost::posix_time::second_clock::local_time();
	ev.kind = kind ? kind : "update";
	if(actor && !IS_NPC(actor) && GET_NAME(actor)) {
		const std::string aname = GET_NAME(actor);
		set_nullable_name(ev.actor_name, aname);
		set_nullable_id(ev.actor_toon_id, lookup_toon_id_tx(db, aname));
	}
	if(note && *note) {
		ev.note = std::string(note);
	}
	if(detail && *detail) {
		ev.detail = std::string(detail);
	}
	db->persist(ev);
}

void diff_push(std::string& out, const std::string& piece) {
	if(piece.empty()) {
		return;
	}
	if(!out.empty()) {
		out += "; ";
	}
	out += piece;
}

std::string nullable_str(const odb::nullable<std::string>& v) {
	return (!v.null() && !v.get().empty()) ? v.get() : "";
}

std::string truncate_for_diff(const std::string& s, size_t maxn = 40) {
	if(s.size() <= maxn) {
		return s;
	}
	return s.substr(0, maxn - 3) + "...";
}

std::string apply_loc_name(short loc) {
	if(loc >= 0 && loc <= APPLY_SKIP) {
		return apply_types[loc];
	}
	return "loc" + std::to_string(loc);
}

/* Come sprintbit, ma su std::string: niente strcat su buffer fissi. */
std::string format_bit_names(unsigned long bits, const char* names[]) {
	if(!names) {
		return "NONE";
	}
	std::string result;
	long nr = 0;
	for(; bits; bits >>= 1) {
		if(IS_SET(1, bits)) {
			if(!result.empty()) {
				result += ' ';
			}
			if(*names[nr] != '\n') {
				result += names[nr];
			}
			else {
				result += "UNDEFINED";
			}
		}
		if(*names[nr] != '\n') {
			nr++;
		}
	}
	return result.empty() ? "NONE" : result;
}

std::string format_named_type(int type, const char* names[]) {
	if(!names) {
		return std::to_string(type);
	}
	int nr = 0;
	for(; names[nr] && *names[nr] != '\n'; ++nr) {
	}
	if(type >= 0 && type < nr) {
		return names[type];
	}
	return std::string("UNDEFINED(") + std::to_string(type) + ")";
}

std::string format_item_type_name(int type_flag) {
	return format_named_type(type_flag, item_types);
}

/* Nei value scroll/potion/wand/staff lo spell è 1-based (0 = nessuno). */
std::string format_spell_ref(int spell_num) {
	if(spell_num < 1) {
		return "none";
	}
	return format_named_type(spell_num - 1, spells);
}

std::string format_yes_no(int v) {
	return v ? "Yes" : "No";
}

using ValueField = std::pair<std::string, std::string>;

/* Label/decode v0–v3 come in stat/identify, in forma compatta per history. */
std::vector<ValueField> typed_obj_value_fields(int type_flag, int v0, int v1, int v2,
											   int v3) {
	std::vector<ValueField> fields;
	auto add = [&](const char* key, std::string val) {
		fields.emplace_back(key, std::move(val));
	};

	switch(type_flag) {
	case ITEM_LIGHT:
		add("colour", std::to_string(v0));
		add("light_type", std::to_string(v1));
		add("hours", std::to_string(v2));
		break;
	case ITEM_SCROLL:
	case ITEM_POTION:
		add("level", std::to_string(v0));
		add("spell1", format_spell_ref(v1));
		add("spell2", format_spell_ref(v2));
		add("spell3", format_spell_ref(v3));
		break;
	case ITEM_WAND:
	case ITEM_STAFF:
		add("level", std::to_string(v0));
		add("max_charges", std::to_string(v1));
		add("charges", std::to_string(v2));
		add("spell", format_spell_ref(v3));
		break;
	case ITEM_WEAPON:
		add("reserved", std::to_string(v0));
		add("todam", std::to_string(v1) + "D" + std::to_string(v2));
		add("damtype", format_named_type(v3, aszWeaponType));
		break;
	case ITEM_FIREWEAPON:
		add("min_str", std::to_string(v0));
		add("max_range", std::to_string(v1));
		add("bonus_range", std::to_string(v2));
		add("fw_type", std::to_string(v3));
		break;
	case ITEM_MISSILE:
		add("break_pct", std::to_string(v0));
		add("todam", std::to_string(v1) + "D" + std::to_string(v2));
		add("missile_type", std::to_string(v3));
		break;
	case ITEM_ARMOR:
		add("AC", std::to_string(v0));
		add("full_str", std::to_string(v1));
		break;
	case ITEM_TRAP:
		add("eff_type", std::to_string(v0));
		add("dam_type", std::to_string(v1));
		add("level", std::to_string(v2));
		add("charges", std::to_string(v3));
		break;
	case ITEM_CONTAINER:
		add("max_contains", std::to_string(v0));
		add("locktype", std::to_string(v1));
		add("corpse", format_yes_no(v3));
		break;
	case ITEM_DRINKCON:
		add("max_contains", std::to_string(v0));
		add("contains", std::to_string(v1));
		add("liquid", format_named_type(v2, drinks));
		add("poisoned", format_yes_no(v3));
		break;
	case ITEM_NOTE:
		add("tongue", std::to_string(v0));
		break;
	case ITEM_KEY:
		add("keytype", std::to_string(v0));
		break;
	case ITEM_FOOD:
		add("full", std::to_string(v0));
		add("poisoned", format_yes_no(v3));
		break;
	case ITEM_MONEY:
		add("coins", std::to_string(v0));
		break;
	default:
		add("v0", std::to_string(v0));
		add("v1", std::to_string(v1));
		add("v2", std::to_string(v2));
		add("v3", std::to_string(v3));
		break;
	}
	return fields;
}

std::string summarize_typed_values(int type_flag, int v0, int v1, int v2, int v3) {
	std::string s;
	for(const auto& f : typed_obj_value_fields(type_flag, v0, v1, v2, v3)) {
		if(!s.empty()) {
			s += ' ';
		}
		s += f.first;
		s += '=';
		s += f.second;
	}
	return s;
}

void append_typed_values(std::string& out, int type_flag, int v0, int v1, int v2, int v3) {
	for(const auto& f : typed_obj_value_fields(type_flag, v0, v1, v2, v3)) {
		diff_push(out, f.first + "=" + f.second);
	}
}

void append_typed_value_diffs(std::string& out, const object_instance& before,
							  const object_instance& after) {
	const bool vals_changed = before.value0 != after.value0 || before.value1 != after.value1 ||
							  before.value2 != after.value2 || before.value3 != after.value3;
	if(before.type_flag != after.type_flag) {
		/* Cambia il significato degli slot: mostra i due riassunti tipizzati. */
		diff_push(out, std::string("values '") +
						   summarize_typed_values(before.type_flag, before.value0, before.value1,
												  before.value2, before.value3) +
						   "'->'" +
						   summarize_typed_values(after.type_flag, after.value0, after.value1,
												  after.value2, after.value3) +
						   "'");
		return;
	}
	if(!vals_changed) {
		return;
	}
	const auto a =
		typed_obj_value_fields(before.type_flag, before.value0, before.value1, before.value2,
							   before.value3);
	const auto b = typed_obj_value_fields(after.type_flag, after.value0, after.value1,
										  after.value2, after.value3);
	const size_t n = std::min(a.size(), b.size());
	for(size_t i = 0; i < n; ++i) {
		if(a[i].second != b[i].second) {
			diff_push(out, a[i].first + " " + a[i].second + "->" + b[i].second);
		}
	}
}

std::string format_affect_modifier(short loc, int mod) {
	switch(loc) {
	case APPLY_M_IMMUNE:
	case APPLY_IMMUNE:
	case APPLY_SUSC:
		return format_bit_names(static_cast<unsigned long>(mod), immunity_names);
	case APPLY_SPELL:
		return format_bit_names(static_cast<unsigned long>(mod), affected_bits);
	case APPLY_AFF2:
		return format_bit_names(static_cast<unsigned long>(mod), affected_bits2);
	default:
		return std::to_string(mod);
	}
}

std::string format_affect_piece(short loc, int mod, char sign) {
	std::string out;
	out.push_back(sign);
	out += apply_loc_name(loc);
	out += " ";
	out += format_affect_modifier(loc, mod);
	return out;
}

std::string build_instance_snapshot(const object_instance& row,
									const short* aff_loc, const int* aff_mod) {
	std::string out;
	char buf[220];
	snprintf(buf, sizeof(buf), "base=%u cost=%d cost/day=%d weight=%d", row.base_vnum,
			 row.cost, row.cost_per_day, row.weight);
	diff_push(out, buf);
	diff_push(out, std::string("short=") + truncate_for_diff(row.short_desc, 60));
	diff_push(out, std::string("name=") + truncate_for_diff(row.obj_name, 40));
	const std::string owner = nullable_str(row.owner_name);
	if(!owner.empty()) {
		diff_push(out, std::string("owner=") + owner);
	}
	diff_push(out, std::string("type=") + format_item_type_name(row.type_flag));
	diff_push(out, std::string("wear=") + format_bit_names(row.wear_flags, wear_bits));
	diff_push(out,
			  std::string("extra=") +
				  format_bit_names(static_cast<unsigned long>(row.extra_flags), extra_bits));
	diff_push(out,
			  std::string("extra2=") +
				  format_bit_names(static_cast<unsigned long>(row.extra_flags2), extra_bits2));
	if(row.bitvector != 0) {
		diff_push(out, std::string("bitv=") +
						   format_bit_names(row.bitvector, affected_bits));
	}
	append_typed_values(out, row.type_flag, row.value0, row.value1, row.value2, row.value3);
	if(aff_loc && aff_mod) {
		for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
			if(aff_loc[i] == 0 && aff_mod[i] == 0) {
				continue;
			}
			diff_push(out, format_affect_piece(aff_loc[i], aff_mod[i], '+'));
		}
	}
	return out;
}

void send_wrapped_detail_lines(char_data* ch, const std::string& detail) {
	if(!ch || detail.empty()) {
		return;
	}
	std::string rest = detail;
	while(!rest.empty()) {
		size_t cut = rest.size();
		if(cut > 90) {
			const size_t semi = rest.rfind("; ", 90);
			cut = (semi != std::string::npos && semi > 20) ? semi + 2 : 90;
		}
		std::string chunk = rest.substr(0, cut);
		while(!chunk.empty() && (chunk.back() == ' ' || chunk.back() == ';')) {
			chunk.pop_back();
		}
		rest = (cut < rest.size()) ? rest.substr(cut) : std::string();
		while(!rest.empty() && rest.front() == ' ') {
			rest.erase(rest.begin());
		}
		/* chunk ≤90 + indent; evita snprintf se la riga cresce. */
		send_to_char(("    " + chunk + "\n\r").c_str(), ch);
	}
}

std::string build_instance_diff(const object_instance* before, const object_instance& after,
								const short* before_loc, const int* before_mod,
								const struct obj_data* obj) {
	std::string out;
	char buf[220];

	if(!before) {
		snprintf(buf, sizeof(buf), "create base=%u cost=%d cost/day=%d", after.base_vnum,
				 after.cost, after.cost_per_day);
		diff_push(out, buf);
		if(!after.short_desc.empty()) {
			diff_push(out, std::string("short=") + truncate_for_diff(after.short_desc, 60));
		}
		const std::string owner = nullable_str(after.owner_name);
		if(!owner.empty()) {
			diff_push(out, std::string("owner=") + owner);
		}
		diff_push(out, std::string("type=") + format_item_type_name(after.type_flag));
		diff_push(out, std::string("wear=") + format_bit_names(after.wear_flags, wear_bits));
		diff_push(out, std::string("extra=") +
						   format_bit_names(static_cast<unsigned long>(after.extra_flags),
											extra_bits));
		diff_push(out, std::string("extra2=") +
						   format_bit_names(static_cast<unsigned long>(after.extra_flags2),
											extra_bits2));
		append_typed_values(out, after.type_flag, after.value0, after.value1, after.value2,
							after.value3);
		if(obj) {
			for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
				if(obj->affected[i].location == 0 && obj->affected[i].modifier == 0) {
					continue;
				}
				diff_push(out, format_affect_piece(obj->affected[i].location,
												   obj->affected[i].modifier, '+'));
			}
		}
		return out;
	}

	auto push_int = [&](const char* label, int a, int b) {
		if(a != b) {
			snprintf(buf, sizeof(buf), "%s %d->%d", label, a, b);
			diff_push(out, buf);
		}
	};
	auto push_uint = [&](const char* label, unsigned a, unsigned b) {
		if(a != b) {
			snprintf(buf, sizeof(buf), "%s %u->%u", label, a, b);
			diff_push(out, buf);
		}
	};
	auto push_str = [&](const char* label, const std::string& a, const std::string& b) {
		if(a != b) {
			diff_push(out, std::string(label) + " '" + truncate_for_diff(a) + "'->'" +
							   truncate_for_diff(b) + "'");
		}
	};
	auto push_flags = [&](const char* label, unsigned long a, unsigned long b,
						  const char* names[]) {
		if(a != b) {
			diff_push(out, std::string(label) + " '" + format_bit_names(a, names) + "'->'" +
							   format_bit_names(b, names) + "'");
		}
	};

	push_uint("base", before->base_vnum, after.base_vnum);
	push_int("cost", before->cost, after.cost);
	push_int("cost/day", before->cost_per_day, after.cost_per_day);
	push_int("weight", before->weight, after.weight);
	push_int("timer", before->timer, after.timer);
	if(before->type_flag != after.type_flag) {
		diff_push(out, std::string("type '") + format_item_type_name(before->type_flag) +
						   "'->'" + format_item_type_name(after.type_flag) + "'");
	}
	push_flags("wear", before->wear_flags, after.wear_flags, wear_bits);
	push_flags("extra", static_cast<unsigned long>(before->extra_flags),
			   static_cast<unsigned long>(after.extra_flags), extra_bits);
	push_flags("extra2", static_cast<unsigned long>(before->extra_flags2),
			   static_cast<unsigned long>(after.extra_flags2), extra_bits2);
	push_flags("bitv", before->bitvector, after.bitvector, affected_bits);
	append_typed_value_diffs(out, *before, after);
	push_str("name", before->obj_name, after.obj_name);
	push_str("short", before->short_desc, after.short_desc);
	push_str("long", before->description, after.description);
	push_str("owner", nullable_str(before->owner_name), nullable_str(after.owner_name));

	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const short ol = before_loc ? before_loc[i] : 0;
		const int om = before_mod ? before_mod[i] : 0;
		const short nl = obj ? obj->affected[i].location : 0;
		const int nm = obj ? obj->affected[i].modifier : 0;
		if(ol == nl && om == nm) {
			continue;
		}
		if(ol == 0 && om == 0) {
			diff_push(out, format_affect_piece(nl, nm, '+'));
		}
		else if(nl == 0 && nm == 0) {
			diff_push(out, format_affect_piece(ol, om, '-'));
		}
		else {
			diff_push(out, apply_loc_name(nl) + " '" + format_affect_modifier(ol, om) +
							   "'->'" + format_affect_modifier(nl, nm) + "'");
		}
	}

	if(out.empty()) {
		out = "(nessuna differenza rilevata)";
	}
	return out;
}

void fill_instance_from_obj(object_instance& row, const struct obj_data* obj, int base_vnum,
							char_data* actor, bool is_create) {
	row.base_vnum = static_cast<unsigned int>(base_vnum);
	/* char_vnum = prototipo originale; mai un vnum del range edit 34k. */
	if(obj->char_vnum > 0 &&
	   (obj->char_vnum < LOW_EDITED_ITEMS || obj->char_vnum > HIGH_EDITED_ITEMS)) {
		row.char_vnum = static_cast<unsigned int>(obj->char_vnum);
	}
	else {
		row.char_vnum = static_cast<unsigned int>(base_vnum);
	}
	row.type_flag = obj->obj_flags.type_flag;
	row.wear_flags = obj->obj_flags.wear_flags;
	row.extra_flags = static_cast<int>(obj->obj_flags.extra_flags);
	row.extra_flags2 = static_cast<int>(obj->obj_flags.extra_flags2);
	row.weight = obj->obj_flags.weight;
	row.cost = obj->obj_flags.cost;
	row.cost_per_day = obj->obj_flags.cost_per_day;
	row.timer = obj->obj_flags.timer;
	row.bitvector = obj->obj_flags.bitvector;
	row.value0 = obj->obj_flags.value[0];
	row.value1 = obj->obj_flags.value[1];
	row.value2 = obj->obj_flags.value[2];
	row.value3 = obj->obj_flags.value[3];
	/* Keyword senza ED*: ownership va in owner_name. */
	row.obj_name = object_instance_strip_ed_tokens(obj->name);
	if(row.obj_name.empty()) {
		row.obj_name = safe_cstr(obj->name);
	}
	row.short_desc = safe_cstr(obj->short_description);
	row.description = safe_cstr(obj->description);
	if(obj->action_description && *obj->action_description) {
		row.action_desc = std::string(obj->action_description);
	}
	else {
		row.action_desc = odb::nullable<std::string>();
	}
	const int cur_vnum =
		(obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	if(cur_vnum >= LOW_EDITED_ITEMS && cur_vnum <= HIGH_EDITED_ITEMS) {
		row.legacy_edit_vnum = static_cast<unsigned int>(cur_vnum);
	}

	set_nullable_name(row.owner_name, resolve_owner_name(obj, actor));

	const boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	if(is_create) {
		row.created_at = now;
		row.deleted = false;
		row.deleted_on = odb::nullable<boost::posix_time::ptime>();
	}
	row.updated_at = now;

	if(actor && !IS_NPC(actor) && GET_NAME(actor)) {
		const std::string aname = GET_NAME(actor);
		if(is_create) {
			set_nullable_name(row.created_by_name, aname);
		}
		set_nullable_name(row.updated_by_name, aname);
	}
}

void fill_actor_and_owner_ids_tx(DB* db, object_instance& row, char_data* actor,
								 bool is_create) {
	if(!row.owner_name.null() && !row.owner_name.get().empty()) {
		set_nullable_id(row.owner_toon_id, lookup_toon_id_tx(db, row.owner_name.get()));
	}
	if(actor && !IS_NPC(actor) && GET_NAME(actor)) {
		const unsigned long long aid = lookup_toon_id_tx(db, GET_NAME(actor));
		if(is_create) {
			set_nullable_id(row.created_by_toon_id, aid);
		}
		set_nullable_id(row.updated_by_toon_id, aid);
	}
}

bool replace_instance_affects_tx(DB* db, unsigned long long instance_id,
								 const struct obj_data* obj) {
	using AffQ = odb::query<object_instance_affect>;
	db->erase_query<object_instance_affect>(AffQ::key.instance_id == instance_id);
	for(unsigned char slot = 0; slot < MAX_OBJ_AFFECT; ++slot) {
		const auto& af = obj->affected[slot];
		if(af.location == 0 && af.modifier == 0) {
			continue;
		}
		object_instance_affect row;
		row.key.instance_id = instance_id;
		row.key.affect_slot = slot;
		row.location = af.location;
		row.modifier = af.modifier;
		db->persist(row);
	}
	return true;
}

/** Se c'e' gia' una tx sul thread, riusala (no begin/commit). Altrimenti aprine una. */
template <typename F>
auto with_odb_tx(DB* db, F&& work) -> decltype(work()) {
	if(odb::transaction::has_current()) {
		return work();
	}
	odb::transaction t(db->begin());
	t.tracer(logTracer);
	auto result = work();
	t.commit();
	return result;
}

unsigned long long persist_body_tx(DB* db, struct obj_data* obj, int base_vnum,
								   unsigned long long update_id, char_data* actor,
								   bool write_event) {
	unsigned long long id = update_id ? update_id : obj->db_instance_id;
	bool is_create = (id == 0);
	object_instance before {};
	object_instance row {};
	short before_loc[MAX_OBJ_AFFECT] {};
	int before_mod[MAX_OBJ_AFFECT] {};
	bool have_before = false;

	if(id != 0) {
		try {
			db->load<object_instance>(id, row);
			if(row.deleted) {
				/* Soft-deleted: non riaprire, crea nuovo edit. */
				id = 0;
				row = object_instance {};
				is_create = true;
				have_before = false;
			}
			else {
				before = row;
				have_before = true;
				is_create = false;
				using AffQ = odb::query<object_instance_affect>;
				for(const auto& af :
					db->query<object_instance_affect>(AffQ::key.instance_id == id)) {
					if(af.key.affect_slot >= MAX_OBJ_AFFECT) {
						continue;
					}
					before_loc[af.key.affect_slot] = af.location;
					before_mod[af.key.affect_slot] = af.modifier;
				}
			}
		}
		catch(const odb::exception&) {
			id = 0;
			row = object_instance {};
			is_create = true;
			have_before = false;
		}
	}
	fill_instance_from_obj(row, obj, base_vnum, actor, is_create);
	fill_actor_and_owner_ids_tx(db, row, actor, is_create);

	std::string detail;
	if(write_event || is_create) {
		detail = build_instance_diff(have_before ? &before : nullptr, row,
									 have_before ? before_loc : nullptr,
									 have_before ? before_mod : nullptr, obj);
	}

	if(is_create) {
		row.id = 0;
		db->persist(row);
		id = row.id;
	}
	else {
		row.id = id;
		db->update(row);
	}
	replace_instance_affects_tx(db, id, obj);
	if(write_event || is_create) {
		char note[64];
		snprintf(note, sizeof(note), "base=%d", base_vnum);
		append_instance_event_tx(db, id, actor, is_create ? "create" : "update", note,
								 detail.c_str());
	}
	return id;
}

void apply_strings(struct obj_data* obj, const std::string& name, const std::string& sd,
				   const std::string& desc, const odb::nullable<std::string>& action) {
	if(obj->name) {
		free(obj->name);
	}
	if(obj->short_description) {
		free(obj->short_description);
	}
	if(obj->description) {
		free(obj->description);
	}
	if(obj->action_description) {
		free(obj->action_description);
		obj->action_description = nullptr;
	}
	obj->name = static_cast<char*>(malloc(name.size() + 1));
	obj->short_description = static_cast<char*>(malloc(sd.size() + 1));
	obj->description = static_cast<char*>(malloc(desc.size() + 1));
	std::strcpy(obj->name, name.c_str());
	std::strcpy(obj->short_description, sd.c_str());
	std::strcpy(obj->description, desc.c_str());
	if(!action.null() && !action.get().empty()) {
		const std::string& a = action.get();
		obj->action_description = static_cast<char*>(malloc(a.size() + 1));
		std::strcpy(obj->action_description, a.c_str());
	}
}

} // namespace

std::string object_instance_extract_ed_owner(const char* keywords) {
	if(!keywords || !*keywords) {
		return {};
	}
	const char* p = keywords;
	while(*p) {
		while(*p && isspace(static_cast<unsigned char>(*p))) {
			++p;
		}
		if(!*p) {
			break;
		}
		const char* start = p;
		while(*p && !isspace(static_cast<unsigned char>(*p))) {
			++p;
		}
		if((p - start) > 2 && start[0] == 'E' && start[1] == 'D') {
			return std::string(start + 2, p);
		}
	}
	return {};
}

std::string object_instance_strip_ed_tokens(const char* keywords) {
	if(!keywords || !*keywords) {
		return {};
	}
	std::string out;
	const char* p = keywords;
	while(*p) {
		while(*p && isspace(static_cast<unsigned char>(*p))) {
			++p;
		}
		if(!*p) {
			break;
		}
		const char* start = p;
		while(*p && !isspace(static_cast<unsigned char>(*p))) {
			++p;
		}
		if((p - start) > 2 && start[0] == 'E' && start[1] == 'D') {
			continue;
		}
		if(!out.empty()) {
			out.push_back(' ');
		}
		out.append(start, p);
	}
	return out;
}

int object_instance_resolve_base_vnum(const struct obj_data* obj) {
	if(!obj) {
		return 0;
	}
	const int iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	if(obj->char_vnum > 0) {
		const int cv = obj->char_vnum;
		if(cv < LOW_EDITED_ITEMS || cv > HIGH_EDITED_ITEMS) {
			return cv;
		}
	}
	if(iVNum > 0 && (iVNum < LOW_EDITED_ITEMS || iVNum > HIGH_EDITED_ITEMS)) {
		return iVNum;
	}
	return 0;
}

unsigned long long object_instance_persist(struct obj_data* obj, int base_vnum,
										   unsigned long long update_id,
										   char_data* actor, bool write_event) {
	if(!obj || base_vnum <= 0) {
		return 0;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		mudlog(LOG_SYSERR, "object_instance_persist: no database");
		return 0;
	}

	unsigned long long id = 0;
	try {
		id = with_odb_tx(db, [&]() {
			return persist_body_tx(db, obj, base_vnum, update_id, actor, write_event);
		});
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_persist: %s", e.what());
		return 0;
	}

	obj->db_instance_id = id;
	{
		const std::string owner = resolve_owner_name(obj, actor);
		copy_personal_owner(obj, owner);
		const std::string stripped = object_instance_strip_ed_tokens(obj->name);
		if(!stripped.empty() && obj->name && stripped != obj->name) {
			free(obj->name);
			obj->name = static_cast<char*>(malloc(stripped.size() + 1));
			std::strcpy(obj->name, stripped.c_str());
		}
		if(obj->char_vnum == 0 ||
		   (obj->char_vnum >= LOW_EDITED_ITEMS && obj->char_vnum <= HIGH_EDITED_ITEMS)) {
			obj->char_vnum = base_vnum;
		}
	}
	return id;
}

bool object_instance_apply(struct obj_data* obj, unsigned long long instance_id) {
	if(!obj || instance_id == 0) {
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return false;
	}
	try {
		const bool ok = with_odb_tx(db, [&]() {
			object_instance row;
			db->load<object_instance>(instance_id, row);
			if(row.deleted) {
				return false;
			}

			obj->obj_flags.type_flag = row.type_flag;
			obj->obj_flags.wear_flags = row.wear_flags;
			obj->obj_flags.extra_flags = static_cast<unsigned int>(row.extra_flags);
			obj->obj_flags.extra_flags2 = static_cast<unsigned int>(row.extra_flags2);
			obj->obj_flags.weight = row.weight;
			obj->obj_flags.cost = row.cost;
			obj->obj_flags.cost_per_day = row.cost_per_day;
			obj->obj_flags.timer = row.timer;
			obj->obj_flags.bitvector = row.bitvector;
			obj->obj_flags.value[0] = row.value0;
			obj->obj_flags.value[1] = row.value1;
			obj->obj_flags.value[2] = row.value2;
			obj->obj_flags.value[3] = row.value3;
			apply_strings(obj, row.obj_name, row.short_desc, row.description, row.action_desc);
			/* Original = prototipo (base); non ripristinare mai un 34k salvato per sbaglio. */
			{
				int orig = static_cast<int>(row.base_vnum);
				if(!row.char_vnum.null()) {
					const int cv = static_cast<int>(row.char_vnum.get());
					if(cv > 0 && (cv < LOW_EDITED_ITEMS || cv > HIGH_EDITED_ITEMS)) {
						orig = cv;
					}
				}
				obj->char_vnum = orig;
			}
			SET_BIT(obj->obj_flags.extra_flags2, ITEM2_EDIT);
			if(!row.owner_name.null() && !row.owner_name.get().empty()) {
				copy_personal_owner(obj, row.owner_name.get());
			}
			else {
				obj->personal_owner[0] = '\0';
			}

			for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
				obj->affected[i].location = 0;
				obj->affected[i].modifier = 0;
			}
			using AffQ = odb::query<object_instance_affect>;
			for(const auto& af :
				db->query<object_instance_affect>(AffQ::key.instance_id == instance_id)) {
				if(af.key.affect_slot >= MAX_OBJ_AFFECT) {
					continue;
				}
				obj->affected[af.key.affect_slot].location = af.location;
				obj->affected[af.key.affect_slot].modifier = af.modifier;
			}
			return true;
		});
		if(!ok) {
			return false;
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_apply(%llu): %s",
			   static_cast<unsigned long long>(instance_id), e.what());
		return false;
	}
	obj->db_instance_id = instance_id;
	return true;
}

bool object_instance_sync(struct obj_data* obj, char_data* actor) {
	if(!obj || obj->db_instance_id == 0) {
		return false;
	}
	int base = object_instance_resolve_base_vnum(obj);
	if(base <= 0) {
		base = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	}
	if(base <= 0) {
		return false;
	}
	/* Sync inventorio: aggiorna riga, non flood di event. */
	return object_instance_persist(obj, base, obj->db_instance_id, actor, false) != 0;
}

obj_data* object_instance_materialize(unsigned long long instance_id) {
	if(instance_id == 0) {
		return nullptr;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return nullptr;
	}
	unsigned int base_vnum = 0;
	try {
		base_vnum = with_odb_tx(db, [&]() {
			object_instance row;
			db->load<object_instance>(instance_id, row);
			if(row.deleted) {
				return 0u;
			}
			return row.base_vnum;
		});
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_materialize(%llu): %s",
			   static_cast<unsigned long long>(instance_id), e.what());
		return nullptr;
	}
	if(base_vnum == 0 || real_object(static_cast<int>(base_vnum)) < 0) {
		mudlog(LOG_SYSERR, "object_instance_materialize(%llu): bad base_vnum %u",
			   static_cast<unsigned long long>(instance_id), base_vnum);
		return nullptr;
	}
	struct obj_data* obj = read_object(static_cast<int>(base_vnum), VIRTUAL);
	if(!obj) {
		return nullptr;
	}
	if(!object_instance_apply(obj, instance_id)) {
		extract_obj(obj);
		return nullptr;
	}
	return obj;
}

bool object_instance_latest_event_line(unsigned long long instance_id, char* buf,
									   size_t buflen) {
	if(!buf || buflen == 0 || instance_id == 0) {
		return false;
	}
	buf[0] = '\0';
	DB* db = Sql::getMysql();
	if(!db) {
		return false;
	}
	try {
		return with_odb_tx(db, [&]() {
			using EvQ = odb::query<object_instance_event>;
			bool have = false;
			object_instance_event best {};
			for(const auto& ev :
				db->query<object_instance_event>(EvQ::instance_id == instance_id)) {
				if(!have || ev.at > best.at || (ev.at == best.at && ev.id > best.id)) {
					best = ev;
					have = true;
				}
			}
			if(!have) {
				return false;
			}
		const std::string actor =
			(!best.actor_name.null() && !best.actor_name.get().empty())
				? best.actor_name.get()
				: "-";
		const std::string detail =
			(!best.detail.null() && !best.detail.get().empty())
				? best.detail.get()
				: ((!best.note.null() && !best.note.get().empty()) ? best.note.get() : "");
		std::string shown = detail;
		if(shown.size() > 120) {
			shown = shown.substr(0, 117) + "...";
		}
		snprintf(buf, buflen, "%s by %s at %s%s%s", best.kind.c_str(), actor.c_str(),
				 boost::posix_time::to_simple_string(best.at).c_str(),
				 shown.empty() ? "" : " — ", shown.c_str());
		return true;
		});
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_latest_event_line: %s", e.what());
		return false;
	}
}

std::vector<object_instance> load_instance_list_tx(DB* db, bool deleted_list) {
	std::vector<object_instance> out;
	for(const auto& row : db->query<object_instance>()) {
		if(row.deleted != deleted_list) {
			continue;
		}
		out.push_back(row);
	}
	std::sort(out.begin(), out.end(),
			  [](const object_instance& a, const object_instance& b) {
				  return a.id < b.id;
			  });
	return out;
}

unsigned list_num_of_tx(const std::vector<object_instance>& list, unsigned long long pk) {
	for(size_t i = 0; i < list.size(); ++i) {
		if(list[i].id == pk) {
			return static_cast<unsigned>(i + 1);
		}
	}
	return 0;
}

unsigned object_instance_active_list_num(unsigned long long instance_id) {
	if(instance_id == 0) {
		return 0;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return 0;
	}
	try {
		return with_odb_tx(db, [&]() {
			return list_num_of_tx(load_instance_list_tx(db, false), instance_id);
		});
	}
	catch(const odb::exception&) {
		return 0;
	}
}

unsigned object_instance_deleted_list_num(unsigned long long instance_id) {
	if(instance_id == 0) {
		return 0;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return 0;
	}
	try {
		return with_odb_tx(db, [&]() {
			return list_num_of_tx(load_instance_list_tx(db, true), instance_id);
		});
	}
	catch(const odb::exception&) {
		return 0;
	}
}

unsigned long long object_instance_resolve_id(struct char_data* ch, const char* filter,
											 bool deleted_list) {
	if(!filter || !*filter) {
		if(ch) {
			send_to_char(
				deleted_list
					? "Uso: show db history deleted <n>\n\r"
					: "Uso: oload/odelete/show db <n|short|nome|owner>\n\r",
				ch);
		}
		return 0;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		if(ch) {
			send_to_char("Nessuna connessione MySQL.\n\r", ch);
		}
		return 0;
	}

	const char* list_label = deleted_list ? "cancellati" : "attivi";

	if(isdigit(static_cast<unsigned char>(*filter))) {
		const unsigned long long list_n = strtoull(filter, nullptr, 10);
		if(list_n == 0) {
			if(ch) {
				send_to_char("numero lista non valido.\n\r", ch);
			}
			return 0;
		}
		try {
			unsigned long long pk = 0;
			const bool ok = with_odb_tx(db, [&]() {
				const auto list = load_instance_list_tx(db, deleted_list);
				if(list_n > list.size()) {
					return false;
				}
				pk = list[static_cast<size_t>(list_n - 1)].id;
				return true;
			});
			if(!ok || pk == 0) {
				if(ch) {
					char buf[160];
					snprintf(buf, sizeof(buf),
							 "Nessun edit #%llu nella lista %s.\n\r",
							 static_cast<unsigned long long>(list_n), list_label);
					send_to_char(buf, ch);
				}
				return 0;
			}
			return pk;
		}
		catch(const odb::exception& e) {
			mudlog(LOG_SYSERR, "object_instance_resolve_id: %s", e.what());
			if(ch) {
				send_to_char("Errore lettura object_instance.\n\r", ch);
			}
			return 0;
		}
	}

	std::string needle = filter;
	for(char& c : needle) {
		c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
	}

	struct hit {
		unsigned list_n;
		unsigned long long id;
		unsigned int base;
		std::string owner;
		std::string short_desc;
	};
	std::vector<hit> hits;

	try {
		with_odb_tx(db, [&]() {
			const auto list = load_instance_list_tx(db, deleted_list);
			for(size_t i = 0; i < list.size(); ++i) {
				const auto& row = list[i];
				const std::string owner =
					(!row.owner_name.null() && !row.owner_name.get().empty())
						? row.owner_name.get()
						: "";
				std::string hay = row.short_desc + " " + row.obj_name + " " + owner;
				for(char& c : hay) {
					c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
				}
				if(hay.find(needle) == std::string::npos) {
					continue;
				}
				hits.push_back({static_cast<unsigned>(i + 1), row.id, row.base_vnum,
								owner.empty() ? "-" : owner, row.short_desc});
				if(hits.size() >= 20) {
					break;
				}
			}
			return true;
		});
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_resolve_id: %s", e.what());
		if(ch) {
			send_to_char("Errore lettura object_instance.\n\r", ch);
		}
		return 0;
	}

	if(hits.empty()) {
		if(ch) {
			send_to_char("Nessun edit corrisponde.\n\r", ch);
		}
		return 0;
	}
	if(hits.size() == 1) {
		return hits[0].id;
	}
	if(ch) {
		char line[256];
		send_to_char("Piu' edit corrispondono, specifica il numero lista:\n\r", ch);
		for(const hit& h : hits) {
			snprintf(line, sizeof(line), "  #%u base %u owner %-20.20s %s\n\r", h.list_n,
					 h.base, h.owner.c_str(), h.short_desc.c_str());
			send_to_char(line, ch);
		}
	}
	return 0;
}

void object_instance_show_list(struct char_data* ch, const char* filter, bool deleted_list) {
	if(!ch) {
		return;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("Nessuna connessione MySQL.\n\r", ch);
		return;
	}

	unsigned long long want_list_n = 0;
	std::string needle;
	if(filter && *filter) {
		if(isdigit(static_cast<unsigned char>(*filter))) {
			want_list_n = strtoull(filter, nullptr, 10);
		}
		else {
			needle = filter;
			for(char& c : needle) {
				c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
			}
		}
	}

	try {
		with_odb_tx(db, [&]() {
			const auto list = load_instance_list_tx(db, deleted_list);
			char line[320];
			int shown = 0;
			snprintf(line, sizeof(line), "%s\n\r",
					 deleted_list ? "Edits cancellati:" : "Edits attivi:");
			send_to_char(line, ch);
			send_to_char(" #    base  owner                short / name\n\r", ch);
			send_to_char("---- ------ -------------------- ------------------------------\n\r", ch);
			for(size_t i = 0; i < list.size(); ++i) {
				const auto& row = list[i];
				const unsigned list_n = static_cast<unsigned>(i + 1);
				if(want_list_n > 0 && list_n != want_list_n) {
					continue;
				}
				const std::string owner =
					(!row.owner_name.null() && !row.owner_name.get().empty())
						? row.owner_name.get()
						: "-";
				if(!needle.empty()) {
					std::string hay = row.short_desc + " " + row.obj_name + " " + owner;
					for(char& c : hay) {
						c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
					}
					if(hay.find(needle) == std::string::npos) {
						continue;
					}
				}
				snprintf(line, sizeof(line), "%4u %6u  %-20.20s %s (%s)\n\r", list_n,
						 row.base_vnum, owner.c_str(), row.short_desc.c_str(),
						 row.obj_name.c_str());
				send_to_char(line, ch);
				++shown;
				if(shown >= 200) {
					send_to_char("... tronco a 200 righe.\n\r", ch);
					break;
				}
			}
			if(shown == 0) {
				send_to_char(deleted_list ? "Nessun edit cancellato.\n\r"
										  : "Nessun edit attivo.\n\r",
							 ch);
			}
			else {
				snprintf(line, sizeof(line), "%d edit.\n\r", shown);
				send_to_char(line, ch);
			}
			return true;
		});
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_show_list: %s", e.what());
		send_to_char("Errore lettura object_instance.\n\r", ch);
	}
}

void object_instance_show_history(struct char_data* ch, unsigned long long instance_id) {
	if(!ch || instance_id == 0) {
		return;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("Nessuna connessione MySQL.\n\r", ch);
		return;
	}
	char line[320];
	try {
		with_odb_tx(db, [&]() {
			object_instance row {};
			try {
				db->load<object_instance>(instance_id, row);
			}
			catch(const odb::exception&) {
				snprintf(line, sizeof(line), "Nessun edit (pk %llu).\n\r",
						 static_cast<unsigned long long>(instance_id));
				send_to_char(line, ch);
				return false;
			}

			const std::string owner =
				(!row.owner_name.null() && !row.owner_name.get().empty())
					? row.owner_name.get()
					: "-";
			if(row.deleted) {
				const unsigned dn =
					list_num_of_tx(load_instance_list_tx(db, true), instance_id);
				snprintf(line, sizeof(line),
						 "Lista cancellati #%u  base %u  owner %s\n\r  %s (%s)\n\r", dn,
						 row.base_vnum, owner.c_str(), row.short_desc.c_str(),
						 row.obj_name.c_str());
			}
			else {
				const unsigned an =
					list_num_of_tx(load_instance_list_tx(db, false), instance_id);
				snprintf(line, sizeof(line),
						 "Lista attivi #%u  base %u  owner %s\n\r  %s (%s)\n\r", an,
						 row.base_vnum, owner.c_str(), row.short_desc.c_str(),
						 row.obj_name.c_str());
			}
			send_to_char(line, ch);

			short aff_loc[MAX_OBJ_AFFECT] {};
			int aff_mod[MAX_OBJ_AFFECT] {};
			using AffQ = odb::query<object_instance_affect>;
			for(const auto& af :
				db->query<object_instance_affect>(AffQ::key.instance_id == instance_id)) {
				if(af.key.affect_slot >= MAX_OBJ_AFFECT) {
					continue;
				}
				aff_loc[af.key.affect_slot] = af.location;
				aff_mod[af.key.affect_slot] = af.modifier;
			}
			send_to_char("Stato attuale (salvato in DB):\n\r", ch);
			send_wrapped_detail_lines(ch, build_instance_snapshot(row, aff_loc, aff_mod));
			if(!row.deleted_on.null()) {
				snprintf(line, sizeof(line), "  deleted_on %s\n\r",
						 boost::posix_time::to_simple_string(row.deleted_on.get()).c_str());
				send_to_char(line, ch);
			}
			snprintf(line, sizeof(line), "  updated_at %s\n\r",
					 boost::posix_time::to_simple_string(row.updated_at).c_str());
			send_to_char(line, ch);

			send_to_char("Storico (recente → vecchio):\n\r", ch);

			using EvQ = odb::query<object_instance_event>;
			std::vector<object_instance_event> events;
			for(const auto& ev :
				db->query<object_instance_event>(EvQ::instance_id == instance_id)) {
				events.push_back(ev);
			}
			std::sort(events.begin(), events.end(),
					  [](const object_instance_event& a, const object_instance_event& b) {
						  if(a.at != b.at) {
							  return a.at > b.at;
						  }
						  return a.id > b.id;
					  });
			if(events.empty()) {
				send_to_char("  (nessun event)\n\r", ch);
				return true;
			}
			int shown = 0;
			for(const auto& ev : events) {
				const std::string actor =
					(!ev.actor_name.null() && !ev.actor_name.get().empty())
						? ev.actor_name.get()
						: "-";
				snprintf(line, sizeof(line), "  %s  %-8s by %s\n\r",
						 boost::posix_time::to_simple_string(ev.at).c_str(),
						 ev.kind.c_str(), actor.c_str());
				send_to_char(line, ch);
				const std::string detail =
					(!ev.detail.null() && !ev.detail.get().empty())
						? ev.detail.get()
						: ((!ev.note.null() && !ev.note.get().empty()) ? ev.note.get()
																	   : "");
				send_wrapped_detail_lines(ch, detail);
				++shown;
				if(shown >= 100) {
					send_to_char("  ... tronco a 100 event.\n\r", ch);
					break;
				}
			}
			return true;
		});
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_show_history: %s", e.what());
		send_to_char("Errore lettura object_instance_event.\n\r", ch);
	}
}

void object_instance_where(struct char_data* ch, unsigned long long instance_id) {
	if(!ch || instance_id == 0) {
		return;
	}
	char buf[256];
	int found = 0;

	const unsigned list_n = object_instance_active_list_num(instance_id);
	const unsigned del_n = object_instance_deleted_list_num(instance_id);
	if(list_n > 0) {
		snprintf(buf, sizeof(buf), "Lista attivi #%u — online:\n\r", list_n);
	}
	else if(del_n > 0) {
		snprintf(buf, sizeof(buf), "Lista cancellati #%u — online:\n\r", del_n);
	}
	else {
		snprintf(buf, sizeof(buf), "Edit (pk %llu) — online:\n\r",
				 static_cast<unsigned long long>(instance_id));
	}
	send_to_char(buf, ch);

	for(struct obj_data* obj = object_list; obj; obj = obj->next) {
		if(obj->db_instance_id != instance_id) {
			continue;
		}
		++found;
		const char* label = obj->short_description ? obj->short_description : obj->name;
		if(obj->carried_by) {
			snprintf(buf, sizeof(buf), "  %s — inventario di %s (room %d)\n\r", label,
					 GET_NAME_DESC(obj->carried_by),
					 obj->carried_by->in_room);
		}
		else if(obj->equipped_by) {
			snprintf(buf, sizeof(buf), "  %s — equip di %s (room %d)\n\r", label,
					 GET_NAME_DESC(obj->equipped_by),
					 obj->equipped_by->in_room);
		}
		else if(obj->in_obj) {
			snprintf(buf, sizeof(buf), "  %s — dentro %s\n\r", label,
					 obj->in_obj->short_description ? obj->in_obj->short_description
													: obj->in_obj->name);
		}
		else if(obj->in_room != NOWHERE) {
			snprintf(buf, sizeof(buf), "  %s — room %d\n\r", label, obj->in_room);
		}
		else {
			snprintf(buf, sizeof(buf), "  %s — (posizione sconosciuta)\n\r", label);
		}
		send_to_char(buf, ch);
	}
	if(found == 0) {
		send_to_char("  (nessuna copia online)\n\r", ch);
	}

	DB* db = Sql::getMysql();
	if(!db) {
		return;
	}
	send_to_char("Inventori MySQL (migrati):\n\r", ch);
	try {
		odb::connection_ptr cp(db->connection());
		auto& mc = static_cast<odb::mysql::connection&>(*cp);
		MYSQL* h = mc.handle();
		std::ostringstream sql;
		sql << "SELECT t.name, COUNT(*) FROM character_inventory ci "
			   "INNER JOIN toon t ON t.id = ci.toon_id "
			   "WHERE ci.instance_id = "
			<< instance_id
			<< " AND (ci.deleted = 0 OR ci.deleted IS NULL) "
			   "AND t.migrated_at IS NOT NULL "
			   "GROUP BY t.name ORDER BY t.name";
		if(mysql_query(h, sql.str().c_str()) != 0) {
			mudlog(LOG_SYSERR, "object_instance_where: %s", mysql_error(h));
			send_to_char("  (query inventori fallita)\n\r", ch);
			return;
		}
		MYSQL_RES* res = mysql_store_result(h);
		if(!res) {
			send_to_char("  (nessuna riga inventorio)\n\r", ch);
			return;
		}
		int db_found = 0;
		while(MYSQL_ROW row = mysql_fetch_row(res)) {
			++db_found;
			snprintf(buf, sizeof(buf), "  [mysql] %s x%s\n\r", row[0] ? row[0] : "?",
					 row[1] ? row[1] : "0");
			send_to_char(buf, ch);
		}
		mysql_free_result(res);
		if(db_found == 0) {
			send_to_char("  (nessuna riga inventorio)\n\r", ch);
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_where: %s", e.what());
		send_to_char("  (errore MySQL)\n\r", ch);
	}
}

bool object_instance_send_summary(struct char_data* ch, unsigned long long instance_id) {
	if(!ch || instance_id == 0) {
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		send_to_char("Nessuna connessione MySQL.\n\r", ch);
		return false;
	}
	char line[320];
	try {
		return with_odb_tx(db, [&]() {
			object_instance row;
			db->load<object_instance>(instance_id, row);
			if(row.deleted) {
				send_to_char("Questo edit e' gia' nella lista cancellati.\n\r", ch);
				return false;
			}
			const unsigned list_n =
				list_num_of_tx(load_instance_list_tx(db, false), instance_id);
			const std::string owner =
				(!row.owner_name.null() && !row.owner_name.get().empty())
					? row.owner_name.get()
					: "-";
			snprintf(line, sizeof(line),
					 "Lista attivi #%u\n\r"
					 "  base %u  cost %d  owner %s\n\r"
					 "  short: %s\n\r"
					 "  name:  %s\n\r",
					 list_n, row.base_vnum, row.cost, owner.c_str(),
					 row.short_desc.c_str(), row.obj_name.c_str());
			send_to_char(line, ch);

			int online = 0;
			for(struct obj_data* obj = object_list; obj; obj = obj->next) {
				if(obj->db_instance_id == instance_id) {
					++online;
				}
			}
			snprintf(line, sizeof(line), "  copie online con questo id: %d\n\r", online);
			send_to_char(line, ch);

			odb::connection_ptr cp(db->connection());
			auto& mc = static_cast<odb::mysql::connection&>(*cp);
			MYSQL* h = mc.handle();
			std::ostringstream sql;
			sql << "SELECT COUNT(*) FROM character_inventory WHERE instance_id="
				<< instance_id << " AND (deleted=0 OR deleted IS NULL)";
			unsigned long long inv_n = 0;
			if(mysql_query(h, sql.str().c_str()) == 0) {
				MYSQL_RES* res = mysql_store_result(h);
				if(res) {
					if(MYSQL_ROW r = mysql_fetch_row(res)) {
						inv_n = r[0] ? strtoull(r[0], nullptr, 10) : 0;
					}
					mysql_free_result(res);
				}
			}
			snprintf(line, sizeof(line),
					 "  righe inventorio MySQL collegate: %llu\n\r",
					 static_cast<unsigned long long>(inv_n));
			send_to_char(line, ch);
			return true;
		});
	}
	catch(const odb::exception&) {
		send_to_char("Nessun edit attivo con quel numero.\n\r", ch);
		return false;
	}
}

bool object_instance_delete(unsigned long long instance_id, char_data* actor) {
	if(instance_id == 0) {
		return false;
	}
	DB* db = Sql::getMysql();
	if(!db) {
		return false;
	}
	try {
		const bool ok = with_odb_tx(db, [&]() {
			object_instance row;
			db->load<object_instance>(instance_id, row);
			if(row.deleted) {
				return false;
			}

			std::string detail;
			diff_push(detail, "delete base=" + std::to_string(row.base_vnum));
			diff_push(detail, "cost=" + std::to_string(row.cost));
			diff_push(detail, "short=" + truncate_for_diff(row.short_desc));
			{
				const std::string owner = nullable_str(row.owner_name);
				if(!owner.empty()) {
					diff_push(detail, "owner=" + owner);
				}
			}

			append_instance_event_tx(db, instance_id, actor, "delete", nullptr,
									 detail.c_str());

			row.deleted = true;
			const boost::posix_time::ptime now =
				boost::posix_time::second_clock::local_time();
			row.deleted_on = now;
			row.updated_at = now;
			db->update(row);

			/* Affect e event restano; inventori perdono il legame. */
			std::ostringstream sql;
			sql << "UPDATE character_inventory SET instance_id=NULL WHERE instance_id="
				<< instance_id;
			db->execute(sql.str().c_str());
			return true;
		});
		if(!ok) {
			return false;
		}
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "object_instance_delete(%llu): %s",
			   static_cast<unsigned long long>(instance_id), e.what());
		return false;
	}

	for(struct obj_data* obj = object_list; obj; obj = obj->next) {
		if(obj->db_instance_id == instance_id) {
			obj->db_instance_id = 0;
		}
	}
	return true;
}

} // namespace Alarmud

#endif /* USE_MYSQL */
