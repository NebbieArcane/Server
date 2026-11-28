/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "obj_edit_catalog.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

#include "autoenums.hpp"
#include "constants.hpp"
#include "db.hpp"
#include "edit_pool.hpp"
#include "edit_system_config.hpp"
#include "flags.hpp"
#include "handler.hpp"
#include "object_instance.hpp"
#include "obj_value.hpp"
#include "edit_portal_listino.hpp"
#include "clan_symbol.hpp"
#include "procarea.hpp"
#include "spell_parser.hpp"
#include "structs.hpp"
#include "typedefs.hpp"
#include "utility.hpp"
#include "utils.hpp"

namespace Alarmud {

static void catalog_apply_listino_bit_prices(Json& j, int location, unsigned long bit,
											 long mxp_fallback, long rune_fallback);

/**
 * Copia di lavoro per quote/apply portal: a differenza di clone_obj() (solo
 * proto + name/short/long), conserva affect, flags, valori e char_vnum dello
 * stato attuale. Senza questo il delta listino e' spesso 0 (after=proto+nuovo
 * vs before=pezzo gia' editato).
 */
[[nodiscard]] static struct obj_data* portal_clone_obj_state(struct obj_data* obj) {
	if(!obj || obj->item_number < 0) {
		return nullptr;
	}
	struct obj_data* ocopy = read_object(obj->item_number, REAL);
	if(!ocopy) {
		return nullptr;
	}

	auto replace_str = [](char*& dst, const char* src) {
		if(dst) {
			free(dst);
			dst = nullptr;
		}
		if(src) {
			dst = strdup(src);
		}
	};
	replace_str(ocopy->name, obj->name);
	replace_str(ocopy->short_description, obj->short_description);
	replace_str(ocopy->description, obj->description);
	replace_str(ocopy->action_description, obj->action_description);
	replace_str(ocopy->szForbiddenWearToChar, obj->szForbiddenWearToChar);
	replace_str(ocopy->szForbiddenWearToRoom, obj->szForbiddenWearToRoom);

	ocopy->obj_flags = obj->obj_flags;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		ocopy->affected[i] = obj->affected[i];
	}
	ocopy->char_vnum = obj->char_vnum;
	ocopy->sector = obj->sector;
	/*
	 * Conserva db_instance_id: serve a object_edit_load_baseline (create-baseline
	 * premi procarea). Non collegare l'inventario (scratch di analisi).
	 */
	ocopy->db_instance_id = obj->db_instance_id;
	ocopy->db_inventory_id = 0;
	return ocopy;
}

[[nodiscard]] static bool proto_vnum_is_tan(int vnum) noexcept {
	return vnum == TAN_BAG || vnum == TAN_SHIELD || vnum == TAN_JACKET
		   || vnum == TAN_BOOTS || vnum == TAN_GLOVES || vnum == TAN_LEGGINGS
		   || vnum == TAN_SLEEVES || vnum == TAN_HELMET || vnum == TAN_ARMOR;
}

bool object_vnum_is_tan_proto(int vnum) noexcept {
	return proto_vnum_is_tan(vnum);
}

[[nodiscard]] int sum_location_mod(const struct obj_data* obj, int location) {
	if(!obj) {
		return 0;
	}
	int tot = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == location) {
			tot += obj->affected[i].modifier;
		}
	}
	return tot;
}

[[nodiscard]] int find_affect_slot_for_location(const struct obj_data* obj,
												int location) {
	if(!obj) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == location) {
			return i;
		}
	}
	return -1;
}

[[nodiscard]] int find_free_affect_slot(const struct obj_data* obj) {
	if(!obj) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == APPLY_NONE ||
		   obj->affected[i].location == APPLY_SKIP) {
			return i;
		}
	}
	return -1;
}

[[nodiscard]] int count_free_affect_slots(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	int free_slots = 0;
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == APPLY_NONE ||
		   obj->affected[i].location == APPLY_SKIP) {
			++free_slots;
		}
	}
	return free_slots;
}

static void json_listino_pricing(Json& j, const ObjEditListinoSpec& spec) {
	/* Costo per uno step UI (es. armor step −10 → 10 MXP, non 1 MXP/punto). */
	const int abs_step = std::abs(spec.step) > 0 ? std::abs(spec.step) : 1;
	const long raw =
		spec.positive_unit_raw * static_cast<long>(abs_step) * kObjValueStorageScale;
	j["xp_raw_per_step"] = raw;
	j["mxp_per_step"] = raw / 1000000L;
	j["mxp_frac_per_step"] = (raw % 1000000L) / 10000L;
	j["rune_per_step"] = raw / kObjEditRunePerMegaXp;
	j["unit_raw"] = spec.positive_unit_raw;
	j["step_abs"] = abs_step;
}

[[nodiscard]] static int combat_hitroll_total(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	return sum_location_mod(obj, APPLY_HITROLL) + sum_location_mod(obj, APPLY_HITNDAM) +
		   sum_location_mod(obj, APPLY_HITNSP);
}

[[nodiscard]] static int combat_damroll_total(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	return sum_location_mod(obj, APPLY_DAMROLL) + sum_location_mod(obj, APPLY_HITNDAM);
}

[[nodiscard]] static int combat_spellpower_total(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	return sum_location_mod(obj, APPLY_SPELLPOWER) + sum_location_mod(obj, APPLY_HITNSP);
}

static bool place_affect_modifier(struct obj_data* obj, int location,
												int modifier) {
	if(modifier == 0) {
		/* Azzerare = liberare lo slot, non lasciare APPLY_X By 0. */
		const int existing = find_affect_slot_for_location(obj, location);
		if(existing >= 0) {
			obj->affected[existing].location = APPLY_NONE;
			obj->affected[existing].modifier = 0;
		}
		return true;
	}
	int slot = find_affect_slot_for_location(obj, location);
	if(slot < 0) {
		slot = find_free_affect_slot(obj);
		if(slot < 0) {
			return false;
		}
		obj->affected[slot].location = static_cast<sh_int>(location);
	}
	/* modifier e' int su obj_affected_type: NON castare a sh_int — bitvector
	 * APPLY_SPELL (es. AFF_SCRYING=67108864) andrebbero a 0 e scrub li cancella. */
	obj->affected[slot].modifier = modifier;
	return true;
}

/** Rimuove affect con modifier 0 che occupano comunque uno slot (fantasma). */
static void scrub_zero_modifier_affects(struct obj_data* obj) noexcept {
	if(!obj) {
		return;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		if(loc == APPLY_NONE || loc == APPLY_SKIP) {
			continue;
		}
		if(obj->affected[i].modifier != 0) {
			continue;
		}
		/* Bitvector vuoto (immune/spell) o scalare a 0: libera lo slot. */
		obj->affected[i].location = APPLY_NONE;
		obj->affected[i].modifier = 0;
	}
}

void object_compact_edit_affects(struct obj_data* obj) noexcept {
	if(!obj) {
		return;
	}

	struct SavedAffect {
		int location;
		int modifier;
	};
	SavedAffect saved[MAX_OBJ_AFFECT];
	int saved_count = 0;

	int hitroll = 0;
	int damroll = 0;
	int spellpower = 0;

	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		obj->affected[i].location = APPLY_NONE;
		obj->affected[i].modifier = 0;

		switch(loc) {
		case APPLY_HITROLL:
			hitroll += mod;
			break;
		case APPLY_DAMROLL:
			damroll += mod;
			break;
		case APPLY_HITNDAM:
			hitroll += mod;
			damroll += mod;
			break;
		case APPLY_SPELLPOWER:
			spellpower += mod;
			break;
		case APPLY_HITNSP:
			hitroll += mod;
			spellpower += mod;
			break;
		case APPLY_NONE:
		case APPLY_SKIP:
			break;
		default:
			if(mod != 0 && saved_count < MAX_OBJ_AFFECT) {
				saved[saved_count].location = loc;
				saved[saved_count].modifier = mod;
				++saved_count;
			}
			break;
		}
	}

	if(hitroll > 0 && hitroll == damroll && damroll > 0) {
		place_affect_modifier(obj, APPLY_HITNDAM, hitroll);
		hitroll = 0;
		damroll = 0;
	}
	if(hitroll > 0 && hitroll == spellpower && damroll == 0) {
		place_affect_modifier(obj, APPLY_HITNSP, hitroll);
		hitroll = 0;
		spellpower = 0;
	}

	if(hitroll > 0) {
		place_affect_modifier(obj, APPLY_HITROLL, hitroll);
	}
	if(damroll > 0) {
		place_affect_modifier(obj, APPLY_DAMROLL, damroll);
	}
	if(spellpower > 0) {
		place_affect_modifier(obj, APPLY_SPELLPOWER, spellpower);
	}

	for(int i = 0; i < saved_count; ++i) {
		place_affect_modifier(obj, saved[i].location, saved[i].modifier);
	}
	scrub_zero_modifier_affects(obj);
}

[[nodiscard]] static bool scalar_can_edit(const struct obj_data* obj, int location,
										  int free_slots) noexcept {
	if(!obj) {
		return false;
	}
	if(find_affect_slot_for_location(obj, location) >= 0) {
		return true;
	}
	if(sum_location_mod(obj, location) != 0) {
		return true;
	}
	if(free_slots > 0) {
		return true;
	}
	/*
	 * Senza slot liberi: solo merge su uno slot combat gia' presente.
	 * Usare i totali aggregati (combat_*_total) e' sbagliato: HITNDAM conta
	 * come hr>0 e apriva SPELLPOWER anche a pezzo pieno (niente slot, niente
	 * HITROLL dedicato da convertire in HITNSP). Stessa trappola per DAMROLL
	 * se c'e' solo HITNSP. Contano solo gli assi sullo slot dedicato.
	 */
	const int hr_slot = sum_location_mod(obj, APPLY_HITROLL);
	const int dr_slot = sum_location_mod(obj, APPLY_DAMROLL);
	const int hr = combat_hitroll_total(obj);
	const int dr = combat_damroll_total(obj);
	const int sp = combat_spellpower_total(obj);
	/* hitroll dedicato + niente dam → si puo' unire in hit-n-dam */
	if(location == APPLY_DAMROLL && hr_slot > 0 && dr == 0) {
		return true;
	}
	/* damroll dedicato + niente hit → si puo' unire in hit-n-dam */
	if(location == APPLY_HITROLL && dr_slot > 0 && hr == 0) {
		return true;
	}
	/* hitroll dedicato + niente sp → si puo' unire in hit-n-sp */
	if(location == APPLY_SPELLPOWER && hr_slot > 0 && sp == 0) {
		return true;
	}
	if(location == APPLY_HITNDAM && hr > 0 && dr > 0) {
		return true;
	}
	if(location == APPLY_HITNSP && hr > 0 && sp > 0) {
		return true;
	}
	return false;
}

[[nodiscard]] static bool object_has_owner_lock(const struct obj_data* obj) noexcept {
	if(!obj) {
		return false;
	}
	if(obj->personal_owner[0] != '\0') {
		return true;
	}
	return !object_instance_extract_ed_owner(obj->name).empty();
}

[[nodiscard]] bool owner_matches(const struct obj_data* obj, const char* toon_name) {
	if(!obj || !toon_name || !*toon_name) {
		return false;
	}
	/* Proto / non ancora editato: nessun ED nel name né personal_owner. */
	if(!object_has_owner_lock(obj)) {
		return true;
	}
	if(obj->personal_owner[0] != '\0' &&
	   strcasecmp(obj->personal_owner, toon_name) == 0) {
		return true;
	}
	const std::string ed = object_instance_extract_ed_owner(obj->name);
	return !ed.empty() && strcasecmp(ed.c_str(), toon_name) == 0;
}

[[nodiscard]] static bool is_combat_edit_location(int location) noexcept {
	switch(location) {
	case APPLY_HITROLL:
	case APPLY_DAMROLL:
	case APPLY_SPELLPOWER:
	case APPLY_HITNDAM:
	case APPLY_HITNSP:
		return true;
	default:
		return false;
	}
}

/**
 * Cap listino = bonus *oltre* il prototipo (stats +3, armor −40, hit/dam +2, …).
 * Tutti gli scalar del listino oggetto usano questo modello.
 */
[[nodiscard]] static bool listino_uses_proto_relative_range(int location) noexcept {
	ObjEditListinoSpec spec;
	return obj_edit_listino_spec(location, spec);
}

[[nodiscard]] static int prototype_display_current(const struct obj_data* obj,
												   int location) noexcept {
	struct obj_data* baseline = object_edit_load_baseline(obj);
	if(!baseline) {
		return 0;
	}
	const int v = object_edit_display_current(baseline, location);
	extract_obj(baseline);
	return v;
}

/**
 * Proto usato per il delta listino. Hit-n-dam / hit-n-sp: se il proto ha
 * hit/dam (o hit/sp) separati e nessuno slot combinato, il proto "vero" e'
 * il max degli assi — altrimenti +1 sul combinato partiva da 0 e poteva
 * settare sotto i bonus proto.
 */
[[nodiscard]] static int listino_proto_base_for_delta(const struct obj_data* obj,
													 int location) noexcept {
	const int slot_proto = prototype_display_current(obj, location);
	if(location == APPLY_HITNDAM) {
		const int hr = prototype_display_current(obj, APPLY_HITROLL);
		const int dr = prototype_display_current(obj, APPLY_DAMROLL);
		return std::max(slot_proto, std::max(hr, dr));
	}
	if(location == APPLY_HITNSP) {
		const int hr = prototype_display_current(obj, APPLY_HITROLL);
		const int sp = prototype_display_current(obj, APPLY_SPELLPOWER);
		return std::max(slot_proto, std::max(hr, sp));
	}
	return slot_proto;
}

/**
 * Minimo totale assoluto consentito in apply (solo aumentare / unire).
 * Hit-n-dam/sp: max degli assi effettivi gia' presenti sul pezzo.
 */
[[nodiscard]] static int object_edit_apply_floor(const struct obj_data* obj,
												int location) noexcept {
	if(!obj) {
		return 0;
	}
	switch(location) {
	case APPLY_HITNDAM:
		return std::max(combat_hitroll_total(obj), combat_damroll_total(obj));
	case APPLY_HITNSP:
		return std::max(combat_hitroll_total(obj), combat_spellpower_total(obj));
	case APPLY_HITROLL:
		return combat_hitroll_total(obj);
	case APPLY_DAMROLL:
		return combat_damroll_total(obj);
	case APPLY_SPELLPOWER:
		return combat_spellpower_total(obj);
	case APPLY_AC:
	case APPLY_SPELLFAIL:
		/* Lower-is-better: gestito da listino_target_worsens_bonus. */
		return object_edit_display_current(obj, location);
	default:
		return object_edit_display_current(obj, location);
	}
}

[[nodiscard]] static bool listino_higher_is_better(int location) noexcept {
	return location != APPLY_AC && location != APPLY_SPELLFAIL;
}

[[nodiscard]] static bool listino_target_allowed(const struct obj_data* obj,
												 const ObjEditListinoSpec& spec,
												 int target_modifier,
												 std::string& err) {
	if(!listino_uses_proto_relative_range(spec.location)) {
		if(target_modifier < spec.min_total || target_modifier > spec.max_total) {
			err = std::string("valore fuori listino per ") + spec.label + " (consentito "
				  + std::to_string(spec.min_total) + "…" + std::to_string(spec.max_total)
				  + ", richiesto " + std::to_string(target_modifier) + ")";
			return false;
		}
		return true;
	}

	const int proto_val = listino_proto_base_for_delta(obj, spec.location);
	const int delta = target_modifier - proto_val;
	const int abs_step = std::abs(spec.step) > 0 ? std::abs(spec.step) : 1;

	if(spec.min_total < 0) {
		/* Armor/spellfail: extra da min_total…0 (es. −40…0) oltre il proto.
		 * Il valore proto non consuma il tetto (delta puro). */
		if(delta > 0 || delta < spec.min_total) {
			err = std::string("extra fuori listino per ") + spec.label + " (consentito "
				  + std::to_string(spec.min_total) + "…0 oltre proto "
				  + std::to_string(proto_val) + "; richiesto extra "
				  + std::to_string(delta) + ", totale " + std::to_string(target_modifier)
				  + ")";
			return false;
		}
	}
	else if(spec.location == APPLY_HITROLL || spec.location == APPLY_DAMROLL ||
			spec.location == APPLY_SPELLPOWER || spec.location == APPLY_HITNDAM ||
			spec.location == APPLY_HITNSP) {
		/*
		 * Combat: il tetto listino " +2 oltre proto" vale solo per la quota [delta]
		 * (wiz/portale). I [trasferimento] broker sono un budget a parte (+2) e
		 * possono far superare +2 vs proto. Soft-max = edit+broker; i tetti wiz
		 * veri (HR<=2, dam+sp<=2) sono in enforce_piece_combat_wiz_caps.
		 */
		const int soft_max = spec.max_total * 2;
		if(delta < spec.min_total || delta > soft_max) {
			err = std::string("extra fuori listino per ") + spec.label + " (consentito +"
				  + std::to_string(spec.min_total) + "…+" + std::to_string(soft_max)
				  + " oltre proto " + std::to_string(proto_val)
				  + " incl. trasferimenti; richiesto extra +" + std::to_string(delta)
				  + ", totale " + std::to_string(target_modifier) + ")";
			return false;
		}
	}
	else if(delta < spec.min_total || delta > spec.max_total) {
		err = std::string("extra fuori listino per ") + spec.label + " (consentito +"
			  + std::to_string(spec.min_total) + "…+" + std::to_string(spec.max_total)
			  + " oltre proto " + std::to_string(proto_val) + "; richiesto extra +"
			  + std::to_string(delta) + ", totale " + std::to_string(target_modifier)
			  + ")";
		return false;
	}

	if((std::abs(delta) % abs_step) != 0) {
		err = std::string("step non allineato per ") + spec.label + " (step "
			  + std::to_string(spec.step) + ")";
		return false;
	}
	return true;
}

/**
 * Riscrive i totali combat preservando gli altri affect.
 * Accorpamento HITNDAM/HITNSP solo se allow_merge (conferma esplicita UI):
 * altrimenti lascia HITROLL/DAMROLL/SPELLPOWER separati anche se uguali.
 * Compatta PRIMA di ripristinare non-combat: con MAX_OBJ_AFFECT=5
 * piazzare HITROLL+DAMROLL separati mangia uno slot di troppo e droppava
 * IMMUNE/SPELL in silenzio (es. hit-n-dam dopo resi+spy).
 */
[[nodiscard]] static bool rewrite_combat_totals(struct obj_data* obj, int hitroll,
												int damroll, int spellpower,
												std::string& err, bool allow_merge) {
	if(!obj) {
		err = "oggetto null";
		return false;
	}

	struct SavedAffect {
		int location;
		int modifier;
	};
	SavedAffect saved[MAX_OBJ_AFFECT];
	int saved_count = 0;

	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		obj->affected[i].location = APPLY_NONE;
		obj->affected[i].modifier = 0;

		switch(loc) {
		case APPLY_HITROLL:
		case APPLY_DAMROLL:
		case APPLY_HITNDAM:
		case APPLY_SPELLPOWER:
		case APPLY_HITNSP:
		case APPLY_NONE:
		case APPLY_SKIP:
			break;
		default:
			if(mod != 0 && saved_count < MAX_OBJ_AFFECT) {
				saved[saved_count].location = loc;
				saved[saved_count].modifier = mod;
				++saved_count;
			}
			break;
		}
	}

	int hr = hitroll;
	int dr = damroll;
	int sp = spellpower;
	auto place_or_fail = [&](int location, int modifier) -> bool {
		if(!place_affect_modifier(obj, location, modifier)) {
			err = "slot affect insufficienti per salvare combat e gli altri bonus "
				  "(max " +
				  std::to_string(MAX_OBJ_AFFECT) +
				  "): libera uno slot o unisci hit/dam";
			return false;
		}
		return true;
	};

	if(allow_merge && hr > 0 && hr == dr && dr > 0) {
		if(!place_or_fail(APPLY_HITNDAM, hr)) {
			return false;
		}
		hr = 0;
		dr = 0;
	}
	if(allow_merge && hr > 0 && hr == sp && dr == 0) {
		if(!place_or_fail(APPLY_HITNSP, hr)) {
			return false;
		}
		hr = 0;
		sp = 0;
	}
	if(hr > 0 && !place_or_fail(APPLY_HITROLL, hr)) {
		return false;
	}
	if(dr > 0 && !place_or_fail(APPLY_DAMROLL, dr)) {
		return false;
	}
	if(sp > 0 && !place_or_fail(APPLY_SPELLPOWER, sp)) {
		return false;
	}

	for(int i = 0; i < saved_count; ++i) {
		if(!place_affect_modifier(obj, saved[i].location, saved[i].modifier)) {
			err = "slot affect insufficienti: non posso mantenere tutti i bonus "
				  "non-combat dopo l'edit (max " +
				  std::to_string(MAX_OBJ_AFFECT) + " slot)";
			return false;
		}
	}

	scrub_zero_modifier_affects(obj);
	return true;
}

/** Etichetta umana per modifier (bitvector spell/immune, nomi spell, ecc.). */
[[nodiscard]] static std::string format_portal_affect_modifier(int loc, int mod) {
	char buf[MAX_STRING_LENGTH];
	buf[0] = '\0';
	switch(loc) {
	case APPLY_M_IMMUNE:
	case APPLY_IMMUNE:
	case APPLY_SUSC:
		sprintbit(static_cast<unsigned long>(mod), immunity_names, buf);
		return buf;
	case APPLY_SPELL:
		sprintbit(static_cast<unsigned long>(mod), affected_bits, buf);
		return buf;
	case APPLY_AFF2:
		sprintbit(static_cast<unsigned long>(mod), affected_bits2, buf);
		return buf;
	case APPLY_WEAPON_SPELL:
	case APPLY_EAT_SPELL:
		if(mod >= 1 && spells[mod - 1] && *spells[mod - 1] && *spells[mod - 1] != '\n') {
			return spells[mod - 1];
		}
		return std::to_string(mod);
	case APPLY_RACE_SLAYER:
		if(mod >= 0 && RaceName[mod] && *RaceName[mod] && *RaceName[mod] != '\n') {
			return RaceName[mod];
		}
		return std::to_string(mod);
	case APPLY_ALIGN_SLAYER:
		sprintbit(static_cast<unsigned long>(mod), gaszAlignSlayerBits, buf);
		return buf;
	case APPLY_ATTACKS: {
		char num[64];
		snprintf(num, sizeof(num), "%.1f", static_cast<double>(mod) / 10.0);
		return num;
	}
	default:
		return std::to_string(mod);
	}
}

Json object_affect_slots_json(const struct obj_data* obj) {
	Json root;
	Json slots = Json::array();
	int used = 0;
	if(!obj) {
		root["slots"] = slots;
		root["max_slots"] = MAX_OBJ_AFFECT;
		root["used"] = 0;
		root["free_count"] = MAX_OBJ_AFFECT;
		return root;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		const int loc = obj->affected[i].location;
		const int mod = obj->affected[i].modifier;
		const bool free =
			loc == APPLY_NONE || loc == APPLY_SKIP || (loc == 0 && mod == 0);
		Json s;
		s["slot"] = i;
		s["free"] = free;
		if(!free) {
			++used;
			s["location"] = loc;
			if(loc > APPLY_NONE) {
				char buf[MAX_STRING_LENGTH];
				sprinttype(loc, apply_types, buf);
				s["location_name"] = buf;
			}
			s["modifier"] = mod;
			const std::string label = format_portal_affect_modifier(loc, mod);
			s["modifier_label"] = label;
			/* Retrocompat UI: immune_labels resta per RESI/IMM. */
			if(loc == APPLY_IMMUNE || loc == APPLY_M_IMMUNE || loc == APPLY_SUSC) {
				s["immune_labels"] = label;
			}
		}
		slots.push_back(s);
	}
	root["slots"] = slots;
	root["max_slots"] = MAX_OBJ_AFFECT;
	root["used"] = used;
	root["free_count"] = MAX_OBJ_AFFECT - used;
	return root;
}

static void wipe_affect_slot(struct obj_data* obj, int slot) noexcept {
	if(!obj || slot < 0 || slot >= MAX_OBJ_AFFECT) {
		return;
	}
	obj->affected[slot].location = APPLY_NONE;
	obj->affected[slot].modifier = 0;
}

/**
 * Libera lo slot listino per `location` (combat → azzera quel componente).
 * Listino: malus a 2× / effetto positivo gratis; lo slot deve risultare libero.
 */
[[nodiscard]] static bool clear_listino_affect(struct obj_data* obj, int location,
											   std::string& err) {
	if(!obj) {
		err = "oggetto null";
		return false;
	}
	if(is_combat_edit_location(location)) {
		const int before_hr = combat_hitroll_total(obj);
		const int before_dr = combat_damroll_total(obj);
		const int before_sp = combat_spellpower_total(obj);
		int hitroll = before_hr;
		int damroll = before_dr;
		int spellpower = before_sp;
		switch(location) {
		case APPLY_HITROLL:
			hitroll = 0;
			break;
		case APPLY_DAMROLL:
			damroll = 0;
			break;
		case APPLY_SPELLPOWER:
			spellpower = 0;
			break;
		case APPLY_HITNDAM:
			hitroll = 0;
			damroll = 0;
			break;
		case APPLY_HITNSP:
			hitroll = 0;
			spellpower = 0;
			break;
		default:
			break;
		}
		if(hitroll == before_hr && damroll == before_dr && spellpower == before_sp) {
			err = "nessuno slot da rimuovere";
			return false;
		}
		/* Clear: mai accorpare d'ufficio i restanti combat. */
		return rewrite_combat_totals(obj, hitroll, damroll, spellpower, err, false);
	}

	const int slot = find_affect_slot_for_location(obj, location);
	if(slot < 0) {
		err = "nessuno slot da rimuovere";
		return false;
	}
	wipe_affect_slot(obj, slot);
	return true;
}

[[nodiscard]] static bool listino_current_is_malus(int location, int current) noexcept {
	if(location == APPLY_AC || location == APPLY_SPELLFAIL) {
		return current > 0;
	}
	return current < 0;
}

bool object_edit_recovers_listino_malus(const struct obj_data* obj, int location,
									   int target_modifier, bool clear_slot) noexcept {
	if(!obj || clear_slot) {
		return false;
	}
	/* Bitfield: rimozione gratis via clear_slot, non "recupero malus" listino. */
	if(location == APPLY_IMMUNE || location == APPLY_M_IMMUNE || location == APPLY_SPELL
	   || location == APPLY_AFF2 || location == APPLY_SUSC) {
		return false;
	}
	ObjEditListinoSpec spec;
	if(!obj_edit_listino_spec(location, spec)) {
		return false;
	}
	const int cur = object_edit_display_current(obj, location);
	if(!listino_current_is_malus(location, cur)) {
		return false;
	}
	if(location == APPLY_AC || location == APPLY_SPELLFAIL) {
		/* Piu' basso = meglio (meno malus AC/spellfail). */
		return target_modifier < cur;
	}
	/* Stats/combat: -3 → 0 / +1 migliora. */
	return target_modifier > cur;
}

bool object_edit_malus_crosses_to_bonus(int location, int current,
									   int target) noexcept {
	if(!listino_current_is_malus(location, current)) {
		return false;
	}
	if(location == APPLY_AC || location == APPLY_SPELLFAIL) {
		/* Malus >0, bonus <0: attraversa lo zero. */
		return target < 0;
	}
	/* Stats: malus <0, bonus >0. */
	return target > 0;
}

static void portal_force_paid_malus(struct obj_data* obj) noexcept {
	if(obj) {
		SET_BIT(obj->obj_flags.extra_flags2, ITEM2_PAID_MALUS);
	}
}

[[nodiscard]] static bool listino_current_is_positive_effect(int location,
															int current) noexcept {
	if(location == APPLY_AC || location == APPLY_SPELLFAIL) {
		return current < 0;
	}
	return current > 0;
}

/** true se `target` e' peggiorativo rispetto a un bonus gia' presente. */
[[nodiscard]] static bool listino_target_worsens_bonus(int location, int current,
													  int target) noexcept {
	if(!listino_current_is_positive_effect(location, current)) {
		return false;
	}
	if(location == APPLY_AC || location == APPLY_SPELLFAIL) {
		/* Piu' alto = peggio (meno armor / piu' spellfail). */
		return target > current;
	}
	return target < current;
}

[[nodiscard]] bool apply_target_modifier(struct obj_data* obj, int location,
										 int target_modifier, std::string& err,
										 bool clear_slot = false,
										 bool allow_combat_merge = false) {
	if(!obj) {
		err = "oggetto null";
		return false;
	}
	/*
	 * Pool HIT/MANA/...: di regola solo sul PG. Eccezione: clear gratis di
	 * affect nativi gia' presenti sul pezzo (premi Dimensione o proto mondo).
	 */
	const bool native_pool_clear =
		clear_slot && (edit_pool_is_pool_apply(location) || location == APPLY_SAVE_ALL);
	if(edit_pool_location_blocked_on_eq(location) && !native_pool_clear) {
		err = "campo pool: edit sul personaggio, non sull'oggetto";
		return false;
	}
	if(edit_system_blocked_on_object(location, target_modifier) && !native_pool_clear) {
		err = "campo configurato sul personaggio";
		return false;
	}

	if(location == APPLY_IMMUNE || location == APPLY_M_IMMUNE
	   || location == APPLY_SPELL || location == APPLY_AFF2 || location == APPLY_SUSC) {
		/*
		 * Listino: rimozione effetto positivo gratis.
		 * clear_slot + bit in target_modifier: spegne quel bit; se lo slot
		 * resta a 0 lo libera (es. Res. Slash → «No, rimuovi dallo slot»).
		 */
		if(clear_slot) {
			if(target_modifier == 0) {
				err = "specifica il bit da rimuovere dallo slot";
				return false;
			}
			const int slot = find_affect_slot_for_location(obj, location);
			if(slot < 0) {
				err = "nessuno slot da rimuovere";
				return false;
			}
			if((obj->affected[slot].modifier & target_modifier) == 0) {
				err = "effetto non presente sullo slot";
				return false;
			}
			obj->affected[slot].modifier &= ~target_modifier;
			if(obj->affected[slot].modifier == 0) {
				wipe_affect_slot(obj, slot);
			}
			return true;
		}
		if(target_modifier == 0) {
			err = (location == APPLY_SPELL || location == APPLY_AFF2)
					  ? "per togliere una spell usa «No, rimuovi dallo slot»"
					  : "per togliere una resistenza/immunità usa «No, rimuovi dallo slot»";
			return false;
		}
		const int slot = find_affect_slot_for_location(obj, location);
		if(slot >= 0) {
			obj->affected[slot].modifier |= target_modifier;
			return true;
		}
		const int free = find_free_affect_slot(obj);
		if(free < 0) {
			err = "nessuno slot affect libero";
			return false;
		}
		obj->affected[free].location = static_cast<sh_int>(location);
		obj->affected[free].modifier = target_modifier;
		return true;
	}

	ObjEditListinoSpec spec;
	const bool in_listino = obj_edit_listino_spec(location, spec);

	/* Rimuovi slot: solo effetti positivi (gratis). I malus si tolgono portando a 0.
	 * HIT/MANA/MOVE/regen/SAVE_ALL nativi sul pezzo: clear gratis (fuori listino). */
	if(clear_slot) {
		const int cur = object_edit_display_current(obj, location);
		if(native_pool_clear) {
			if(find_affect_slot_for_location(obj, location) < 0 && cur == 0) {
				err = "nessuno slot da rimuovere";
				return false;
			}
			return clear_listino_affect(obj, location, err);
		}
		if(!in_listino) {
			err = "campo non presente nel listino oggetto";
			return false;
		}
		if(listino_current_is_malus(location, cur)) {
			err = "per eliminare un malus porta il selettore a 0 (o un bonus); "
				  "il costo e' il doppio del listino e lo slot si libera";
			return false;
		}
		if(!listino_current_is_positive_effect(location, cur)) {
			err = "nessuno slot bonus da rimuovere";
			return false;
		}
		return clear_listino_affect(obj, location, err);
	}

	if(!in_listino) {
		err = "campo non presente nel listino oggetto";
		return false;
	}

	const int current_total = object_edit_display_current(obj, location);
	const int apply_floor = object_edit_apply_floor(obj, location);
	/*
	 * Bonus gia' presenti: solo aumentare (o «Rimuovi slot»).
	 * Floor combat include assi su slot separati (hitndam current=0 ma hr/dam=3).
	 */
	if(listino_target_worsens_bonus(location, current_total, target_modifier)) {
		err = "non puoi ridurre un bonus: usa «Rimuovi slot» (gratis) oppure migliora";
		return false;
	}
	if(listino_higher_is_better(location) && target_modifier < apply_floor) {
		err = std::string("non puoi sovrascrivere un bonus gia' presente (floor ")
			  + std::to_string(apply_floor) + ", richiesto "
			  + std::to_string(target_modifier)
			  + "): scegli un totale >= floor oppure «Rimuovi slot»";
		return false;
	}

	/*
	 * Target 0 su malus (es. armor +10→0): libera lo slot pagando 2×.
	 * Su bonus non arriva qui (bloccato sopra → clear_slot).
	 */
	if(target_modifier == 0) {
		if(!listino_target_allowed(obj, spec, target_modifier, err)) {
			return false;
		}
		if(is_combat_edit_location(location)) {
			std::string clear_err;
			(void)clear_listino_affect(obj, location, clear_err);
			return true;
		}
		const int slot = find_affect_slot_for_location(obj, location);
		if(slot >= 0) {
			wipe_affect_slot(obj, slot);
		}
		return true;
	}

	if(!listino_target_allowed(obj, spec, target_modifier, err)) {
		return false;
	}

	if(is_combat_edit_location(location)) {
		int hitroll = combat_hitroll_total(obj);
		int damroll = combat_damroll_total(obj);
		int spellpower = combat_spellpower_total(obj);
		switch(location) {
		case APPLY_HITROLL:
			hitroll = target_modifier;
			break;
		case APPLY_DAMROLL:
			damroll = target_modifier;
			break;
		case APPLY_SPELLPOWER:
			spellpower = target_modifier;
			break;
		case APPLY_HITNDAM:
			hitroll = target_modifier;
			damroll = target_modifier;
			break;
		case APPLY_HITNSP:
			hitroll = target_modifier;
			spellpower = target_modifier;
			break;
		default:
			break;
		}
		/* Hit-n-dam / Hit-n-sp scelti esplicitamente = merge voluto.
		 * Hit/dam/sp separati: merge solo con conferma UI (allow_combat_merge). */
		const bool merge =
			!clear_slot
			&& (allow_combat_merge || location == APPLY_HITNDAM
				|| location == APPLY_HITNSP);
		return rewrite_combat_totals(obj, hitroll, damroll, spellpower, err, merge);
	}

	int slot = find_affect_slot_for_location(obj, location);
	if(slot < 0) {
		slot = find_free_affect_slot(obj);
		if(slot < 0) {
			err = "nessuno slot affect libero";
			return false;
		}
		obj->affected[slot].location = static_cast<sh_int>(location);
	}
	obj->affected[slot].modifier = target_modifier;
	return true;
}

bool inventory_row_is_worn(int wearpos) noexcept {
	return wearpos > 0;
}

bool object_portal_allows_worn_edit(const struct obj_data* obj) noexcept {
	/* Apply/quote/options lavorano su character_inventory MySQL a PG offline:
	 * wear_pos e' solo un flag sulla riga, non eq live. Primo edit e ri-edit
	 * sono entrambi sicuri anche se indossato. */
	return obj != nullptr;
}

bool object_is_tanned(const struct obj_data* obj) noexcept {
	if(!obj) {
		return false;
	}
	if(obj->item_number >= 0 && obj->item_number <= top_of_objt
	   && proto_vnum_is_tan(obj_index[obj->item_number].iVNum)) {
		return true;
	}
	if(obj->char_vnum > 0 && proto_vnum_is_tan(obj->char_vnum)) {
		return true;
	}
	return false;
}

const char* object_portal_item_type_slug(int item_type) noexcept {
	switch(item_type) {
	case ITEM_LIGHT:
		return "light";
	case ITEM_SCROLL:
		return "scroll";
	case ITEM_WAND:
		return "wand";
	case ITEM_STAFF:
		return "staff";
	case ITEM_WEAPON:
		return "weapon";
	case ITEM_FIREWEAPON:
		return "fireweapon";
	case ITEM_MISSILE:
		return "missile";
	case ITEM_TREASURE:
		return "treasure";
	case ITEM_ARMOR:
		return "armor";
	case ITEM_POTION:
		return "potion";
	case ITEM_WORN:
		return "worn";
	case ITEM_OTHER:
		return "other";
	case ITEM_TRASH:
		return "trash";
	case ITEM_TRAP:
		return "trap";
	case ITEM_CONTAINER:
		return "container";
	case ITEM_NOTE:
		return "note";
	case ITEM_DRINKCON:
		return "drinkcon";
	case ITEM_KEY:
		return "key";
	case ITEM_FOOD:
		return "food";
	case ITEM_MONEY:
		return "money";
	case ITEM_PEN:
		return "pen";
	case ITEM_BOAT:
		return "boat";
	case ITEM_AUDIO:
		return "audio";
	case ITEM_BOARD:
		return "board";
	case ITEM_TREE:
		return "tree";
	case ITEM_ROCK:
		return "rock";
	case ITEM_M_GEM:
		return "m_gem";
	case ITEM_M_MINERAL:
		return "m_mineral";
	case ITEM_BAR:
		return "bar";
	case ITEM_JEWEL:
		return "jewel";
	case ITEM_CLAN_SYMBOL:
		return "clan_symbol";
	default:
		return nullptr;
	}
}

/**
 * Pezzo gia' in DB edits (show db / oload db = object_instance) oppure
 * personalizzato ITEM2_EDIT + EDNomeToon / personal_owner.
 * Questi possono essere ri-editati anche se [RARO]. TAN resta sempre escluso.
 */
[[nodiscard]] static bool object_portal_is_registered_edit(
	const struct obj_data* obj) noexcept {
	if(!obj) {
		return false;
	}
	if(obj->db_instance_id != 0) {
		return true;
	}
	if(IS_OBJ_STAT2(obj, ITEM2_EDIT) && object_has_owner_lock(obj)) {
		return true;
	}
	return false;
}

/**
 * Esclusioni dure:
 * - TAN: mai (ne' primo edit ne' ri-edit)
 * - HAS-GEMS / simbolo clan: mai
 * - RARO: bloccato al primo edit; permesso in ri-edit se in DB edits
 *   (object_instance) oppure ITEM2_EDIT + EDNomeToon
 */
[[nodiscard]] static bool object_portal_passes_exclusions(const struct obj_data* obj) noexcept {
	if(!obj) {
		return false;
	}
	if(object_is_tanned(obj)) {
		return false;
	}
	if(obj->obj_flags.type_flag == ITEM_CLAN_SYMBOL) {
		return false;
	}
	if(IS_OBJ_STAT2(obj, ITEM2_INSERT)) {
		return false;
	}
	if(obj->obj_flags.cost >= LIM_ITEM_COST_MIN &&
	   !object_portal_is_registered_edit(obj)) {
		return false;
	}
	return true;
}

[[nodiscard]] static bool object_portal_is_existing_edit(const struct obj_data* obj) noexcept {
	if(!obj) {
		return false;
	}
	if(object_portal_is_registered_edit(obj)) {
		return true;
	}
	if(IS_OBJ_STAT2(obj, ITEM2_EDIT)) {
		return true;
	}
	if(object_has_owner_lock(obj)) {
		return true;
	}
	return false;
}

[[nodiscard]] static bool object_portal_included(const struct obj_data* obj) noexcept {
	if(!obj) {
		return false;
	}
	const char* slug = object_portal_item_type_slug(ITEM_TYPE(obj));
	if(slug && edit_system_portal_type_always_hidden(slug)) {
		return false;
	}
	/*
	 * Pezzi gia' personalizzati / instance: restano in lista anche con
	 * categoria ITEM_* spenta (TAN escluso a monte; RARO solo se named edit).
	 */
	if(object_portal_is_existing_edit(obj)) {
		return true;
	}
	if(slug) {
		return edit_system_portal_category_enabled(slug);
	}
	return false;
}

std::string object_portal_skip_reason(const struct obj_data* obj,
									  const char* toon_name) {
	if(!obj || !toon_name || !*toon_name) {
		return "oggetto o PG non valido";
	}
	if(object_is_tanned(obj)) {
		return "conciato (skill tan)";
	}
	if(obj->obj_flags.cost >= LIM_ITEM_COST_MIN &&
	   !object_portal_is_registered_edit(obj)) {
		return "RARO";
	}
	if(obj->obj_flags.type_flag == ITEM_CLAN_SYMBOL) {
		return "simbolo clan";
	}
	if(IS_OBJ_STAT2(obj, ITEM2_INSERT)) {
		return "HAS-GEMS (insert)";
	}
	if(!owner_matches(obj, toon_name)) {
		return "owner diverso dal PG (ED/personal per altro PG)";
	}
	if(!object_portal_included(obj)) {
		const char* slug = object_portal_item_type_slug(ITEM_TYPE(obj));
		if(slug) {
			return "categoria disabilitata (staff)";
		}
		return "tipo non supportato nel portale";
	}
	return {};
}

bool object_portal_show_in_inventory_list(const struct obj_data* obj,
										  const char* toon_name) noexcept {
	(void)toon_name;
	if(!obj) {
		return false;
	}
	/*
	 * Visibilita' inventario:
	 * - TAN / HAS-GEMS / simbolo: mai
	 * - RARO senza registrazione in DB edits / EDIT+EDNomeToon: nascosto;
	 *   se in object_instance (show db) o EDIT+EDNomeToon: visibile per ri-edit
	 * - categorie staff (EDIT/instance/owner bypassano solo le categorie)
	 */
	if(!object_portal_passes_exclusions(obj)) {
		return false;
	}
	return object_portal_included(obj);
}

bool object_portal_editable(const struct obj_data* obj, const char* toon_name) noexcept {
	if(!obj || !toon_name || !*toon_name) {
		return false;
	}
	if(!object_portal_passes_exclusions(obj)) {
		return false;
	}
	if(!owner_matches(obj, toon_name)) {
		return false;
	}
	return object_portal_included(obj);
}

Json object_edit_catalog_json(const struct obj_data* obj) {
	const int free_slots = count_free_affect_slots(obj);

	Json entries = Json::array();
	const bool is_reward = procarea_obj_is_reward(obj);
	for(int i = 0; i < obj_edit_listino_scalar_count(); ++i) {
		ObjEditListinoSpec spec;
		if(!obj_edit_listino_scalar_at(i, spec)) {
			continue;
		}
		if(edit_pool_location_blocked_on_eq(spec.location)) {
			continue;
		}
		const int occupied_slot = find_affect_slot_for_location(obj, spec.location);
		const int current_total = object_edit_display_current(obj, spec.location);
		const int proto_total = prototype_display_current(obj, spec.location);
		const bool relative = listino_uses_proto_relative_range(spec.location);
		const bool has_affect = occupied_slot >= 0 || current_total != 0;
		const bool can_edit = scalar_can_edit(obj, spec.location, free_slots);
		Json j;
		j["id"] = spec.id;
		j["label"] = spec.label;
		j["location"] = spec.location;
		j["step"] = spec.step;
		j["min"] = spec.min_total;
		j["max"] = spec.max_total;
		j["proto"] = proto_total;
		j["proto_effective"] = listino_proto_base_for_delta(obj, spec.location);
		j["relative"] = relative;
		j["kind"] = "scalar";
		j["has_affect"] = has_affect;
		j["occupied_slot"] = occupied_slot;
		j["can_add"] = can_edit && !has_affect;
		j["can_edit"] = can_edit;
		j["value_floor"] = object_edit_apply_floor(obj, spec.location);
		{
			/* Solo bonus positivi: «Rimuovi slot» gratis. Malus → selettore a 0. */
			const bool clear_positive =
				listino_current_is_positive_effect(spec.location, current_total);
			j["can_clear_slot"] = can_edit && clear_positive;
		}
		json_listino_pricing(j, spec);
		entries.push_back(j);
	}

	/*
	 * HIT/MANA/MOVE/regen/SAVE_ALL sul pezzo: fuori listino oggetto (pool PG).
	 * Espone solo «Rimuovi slot» gratis se presenti — premi Dimensione e anche
	 * pezzi mondo (es. old sandals MOVE +30).
	 */
	{
		static const struct {
			int location;
			const char* id_reward;
			const char* id_native;
			const char* label_reward;
			const char* label_native;
		} native_clearables[] = {
			{APPLY_HIT, "reward.hit", "native.hit", "Hit points (nativo Dimensione)",
			 "Hit points (nativo pezzo)"},
			{APPLY_MANA, "reward.mana", "native.mana", "Mana (nativo Dimensione)",
			 "Mana (nativo pezzo)"},
			{APPLY_MOVE, "reward.move", "native.move", "Move (nativo Dimensione)",
			 "Move (nativo pezzo)"},
			{APPLY_HIT_REGEN, "reward.hit_regen", "native.hit_regen",
			 "Hit regen (nativo Dimensione)", "Hit regen (nativo pezzo)"},
			{APPLY_MANA_REGEN, "reward.mana_regen", "native.mana_regen",
			 "Mana regen (nativo Dimensione)", "Mana regen (nativo pezzo)"},
			{APPLY_MOVE_REGEN, "reward.move_regen", "native.move_regen",
			 "Move regen (nativo Dimensione)", "Move regen (nativo pezzo)"},
			{APPLY_SAVE_ALL, "reward.save_all", "native.save_all",
			 "Save all (nativo Dimensione)", "Save all (nativo pezzo)"},
		};
		for(const auto& rc : native_clearables) {
			const int occupied_slot = find_affect_slot_for_location(obj, rc.location);
			const int current_total = object_edit_display_current(obj, rc.location);
			if(occupied_slot < 0 && current_total == 0) {
				continue;
			}
			Json j;
			j["id"] = is_reward ? rc.id_reward : rc.id_native;
			j["label"] = is_reward ? rc.label_reward : rc.label_native;
			j["location"] = rc.location;
			j["step"] = 1;
			j["min"] = current_total;
			j["max"] = current_total;
			j["proto"] = current_total;
			j["relative"] = false;
			j["kind"] = "scalar";
			j["clear_only"] = true;
			j["has_affect"] = true;
			j["occupied_slot"] = occupied_slot;
			j["can_add"] = false;
			j["can_edit"] = true;
			j["can_clear_slot"] = true;
			j["current"] = current_total;
			j["mxp_per_step"] = 0;
			j["rune_per_step"] = 0;
			j["hint"] = is_reward
							? "Bonus nativo del premio: puoi solo liberare lo slot (gratis)"
							: "Bonus nativo del pezzo (pool): puoi solo liberare lo slot "
							  "(gratis), non modificarlo qui";
			entries.push_back(j);
		}
	}

	/*
	 * Listino https://www.nebbiearcane.it/listino-edits/
	 * Resistenze (APPLY_IMMUNE) ≠ Immunità concesse (APPLY_M_IMMUNE).
	 * Solo voci del listino: niente sleep/charm/nonmag come "resistenza".
	 */
	static const struct {
		unsigned bit;
		const char* slug;
		const char* label;
		long mxp;
		long rune;
	} listino_resists[] = {
		/* Resistenze Spells */
		{IMM_ACID, "acid", "Res. Acid", 75, 90},
		{IMM_ELEC, "elec", "Res. Electricity", 150, 180},
		{IMM_FIRE, "fire", "Res. Fire", 100, 120},
		{IMM_COLD, "cold", "Res. Cold", 75, 90},
		{IMM_ENERGY, "energy", "Res. Energy", 150, 180},
		{IMM_DRAIN, "drain", "Res. Drain", 30, 35},
		{IMM_HOLD, "hold", "Res. Hold", 75, 90},
		{IMM_POISON, "poison", "Res. Poison", 30, 35},
		/* Resistenze ai Fisici */
		{IMM_SLASH, "slash", "Res. Slash", 150, 200},
		{IMM_PIERCE, "pierce", "Res. Pierce", 150, 200},
		{IMM_BLUNT, "blunt", "Res. Blunt", 300, 400},
	};
	for(const auto& r : listino_resists) {
		if(!edit_system_resistance_enabled(r.bit)) {
			continue;
		}
		if(edit_system_resistance_target(r.bit) != EditSystemTarget::Object) {
			continue;
		}
		const int immune_bits = object_immune_current_bits(obj);
		const bool has_affect = (immune_bits & static_cast<int>(r.bit)) != 0;
		const int imm_slot = find_affect_slot_for_location(obj, APPLY_IMMUNE);
		const bool can_add = !has_affect && (imm_slot >= 0 || free_slots > 0);
		Json j;
		j["id"] = std::string("resist.") + r.slug;
		j["label"] = r.label;
		j["location"] = APPLY_IMMUNE;
		j["immune_bit"] = r.bit;
		j["kind"] = "immune";
		j["listino_group"] = "resistance";
		j["has_affect"] = has_affect;
		j["occupied_slot"] = imm_slot;
		j["can_add"] = can_add;
		j["can_edit"] = has_affect || can_add;
		/* Resistenza presente: rimozione gratis (stesso listino dei bonus). */
		j["can_clear_slot"] = has_affect;
		catalog_apply_listino_bit_prices(j, APPLY_IMMUNE, r.bit, r.mxp, r.rune);
		entries.push_back(j);
	}

	/* Immunità Concesse — APPLY_M_IMMUNE (listino), non confondere con Res. */
	static const struct {
		unsigned bit;
		const char* slug;
		const char* label;
		long mxp;
		long rune;
	} listino_immunities[] = {
		{IMM_DRAIN, "drain", "Imm. Drain", 100, 100},
		{IMM_CHARM, "charm", "Imm. Charm", 60, 60},
		{IMM_POISON, "poison", "Imm. Poison", 100, 100},
	};
	for(const auto& r : listino_immunities) {
		const int bits = object_m_immune_current_bits(obj);
		const bool has_affect = (bits & static_cast<int>(r.bit)) != 0;
		const int slot = find_affect_slot_for_location(obj, APPLY_M_IMMUNE);
		const bool can_add = !has_affect && (slot >= 0 || free_slots > 0);
		Json j;
		j["id"] = std::string("immunity.") + r.slug;
		j["label"] = r.label;
		j["location"] = APPLY_M_IMMUNE;
		j["immune_bit"] = r.bit;
		j["kind"] = "m_immune";
		j["listino_group"] = "immunity";
		j["has_affect"] = has_affect;
		j["occupied_slot"] = slot;
		j["can_add"] = can_add;
		j["can_edit"] = has_affect || can_add;
		j["can_clear_slot"] = has_affect;
		catalog_apply_listino_bit_prices(j, APPLY_M_IMMUNE, r.bit, r.mxp, r.rune);
		entries.push_back(j);
	}

	/* Spell editabili — listino https://www.nebbiearcane.it/listino-edits/
	 * Spy = AFF_SCRYING (APPLY_SPELL); Danger Sense = AFF2_DANGER_SENSE (APPLY_AFF2). */
	static const struct {
		int location;
		unsigned long bit;
		const char* slug;
		const char* label;
		long mxp;
		long rune;
	} spell_edits[] = {
		{APPLY_SPELL, AFF_TELEPATHY, "telepathy", "Telepathy", 50, 70},
		{APPLY_SPELL, AFF_GLOBE_DARKNESS, "darkness", "Darkness", 50, 70},
		{APPLY_SPELL, AFF_WATERBREATH, "waterbreath", "Waterbreath", 50, 70},
		{APPLY_SPELL, AFF_TRUE_SIGHT, "true_sight", "True Sight", 50, 70},
		{APPLY_SPELL, AFF_INVISIBLE, "invisibility", "Invisibility", 30, 70},
		{APPLY_SPELL, AFF_SENSE_LIFE, "sense_life", "Sense Life", 50, 70},
		{APPLY_AFF2, AFF2_DANGER_SENSE, "danger_sense", "Psionic Danger Sense", 150, 180},
		{APPLY_SPELL, AFF_SCRYING, "spy", "Spy", 150, 180},
		{APPLY_SPELL, AFF_PROTECT_FROM_EVIL, "prot_evil", "Protection from Evil", 50, 70},
		{APPLY_SPELL, AFF_FLYING, "fly", "Fly", 50, 70},
	};
	for(const auto& sp : spell_edits) {
		const int bits = sum_location_mod(obj, sp.location);
		const int sp_slot = find_affect_slot_for_location(obj, sp.location);
		const bool has_affect = (bits & static_cast<int>(sp.bit)) != 0;
		const bool can_add = !has_affect && (sp_slot >= 0 || free_slots > 0);
		Json j;
		j["id"] = std::string("spell.") + sp.slug;
		j["label"] = sp.label;
		j["location"] = sp.location;
		j["spell_bit"] = sp.bit;
		j["kind"] = "spell";
		j["has_affect"] = has_affect;
		j["occupied_slot"] = sp_slot;
		j["can_add"] = can_add;
		j["can_edit"] = has_affect || can_add;
		j["can_clear_slot"] = has_affect;
		j["can_remove"] = has_affect;
		j["mxp_per_step"] = sp.mxp;
		j["rune_per_step"] = sp.rune;
		entries.push_back(j);
	}

	/* Flag ARTIFACT: costo listino artifact_cost_xp x class_mult (no +50%
	 * su questa voce); non marca ITEM2_EDIT ne' cost 20001. Non rimovibile. */
	{
		const bool is_artifact = IS_OBJ_STAT(obj, ITEM_IMMUNE);
		const long art_xp =
			is_artifact ? 0L : EditArtifactFlagListinoCost(const_cast<struct obj_data*>(obj));
		Json j;
		j["id"] = "artifact";
		j["label"] = "Artifact";
		j["kind"] = "flag";
		j["flag"] = "artifact";
		j["location"] = 0;
		j["current"] = is_artifact ? 1 : 0;
		j["has_affect"] = is_artifact;
		j["occupied_slot"] = -1;
		j["can_add"] = !is_artifact;
		j["can_edit"] = !is_artifact;
		j["can_remove"] = false;
		j["xp_raw"] = art_xp;
		j["mxp"] = art_xp / 1000000L;
		/* Max rune alternative (slider), non un costo extra obbligatorio. */
		const int art_runes = is_artifact ? 0 : EditArtifactFlagListinoRunes();
		j["pq"] = art_runes;
		j["rune_listino"] = art_runes;
		j["hint"] = is_artifact
						? "Permanente. Gli edit affect successive usano il listino "
						  "(artifact_mult sul pezzo/pacchetto)."
						: "Costo listino (100 MXP base x class_mult), oppure fino a "
						  "100 rune in alternativa (senza class_mult), con lo slider. "
						  "Non marca EDIT ne' [RARO]; +50% solo sugli affect, non su "
						  "questa voce.";
		entries.push_back(j);
	}

	return entries;
}

int object_affect_current_modifier(const struct obj_data* obj, int location) noexcept {
	return sum_location_mod(obj, location);
}

int object_edit_display_current(const struct obj_data* obj, int location) noexcept {
	if(!obj) {
		return 0;
	}
	switch(location) {
	case APPLY_HITROLL:
		return combat_hitroll_total(obj);
	case APPLY_DAMROLL:
		return combat_damroll_total(obj);
	case APPLY_SPELLPOWER:
		return combat_spellpower_total(obj);
	case APPLY_HITNDAM:
		/* Solo lo slot combinato reale: hit/dam separati uguali restano distinti
		 * finché il giocatore non conferma l'accorpamento. */
		return sum_location_mod(obj, APPLY_HITNDAM);
	case APPLY_HITNSP:
		return sum_location_mod(obj, APPLY_HITNSP);
	default:
		return sum_location_mod(obj, location);
	}
}

static void catalog_apply_listino_bit_prices(Json& j, int location, unsigned long bit,
											 long mxp_fallback, long rune_fallback) {
	j["mxp_per_step"] = mxp_fallback;
	j["rune_per_step"] = rune_fallback;
	if(!portal_listino_ready()) {
		return;
	}
	ObjEditListinoSpec spec {};
	if(!portal_listino_affect_by_bit(location, bit, spec) || spec.positive_unit_raw <= 0) {
		return;
	}
	const double scale = portal_listino_param("storage_scale",
										  static_cast<double>(kObjValueStorageScale));
	const long xp = static_cast<long>(
		std::llround(static_cast<double>(spec.positive_unit_raw) * scale));
	j["mxp_per_step"] = xp / 1000000L;
	int rn = 0;
	int rd = 1;
	if(portal_listino_affect_rune_bit(location, bit, rn, rd) && rn > 0 && rd > 0) {
		j["rune_per_step"] =
			static_cast<long>(std::llround(static_cast<double>(rn) / static_cast<double>(rd)));
	}
}

int object_immune_current_bits(const struct obj_data* obj) noexcept {
	return sum_location_mod(obj, APPLY_IMMUNE);
}

int object_m_immune_current_bits(const struct obj_data* obj) noexcept {
	return sum_location_mod(obj, APPLY_M_IMMUNE);
}

int object_susc_current_bits(const struct obj_data* obj) noexcept {
	return sum_location_mod(obj, APPLY_SUSC);
}

int object_spell_current_bits(const struct obj_data* obj) noexcept {
	return sum_location_mod(obj, APPLY_SPELL);
}

int object_aff2_current_bits(const struct obj_data* obj) noexcept {
	return sum_location_mod(obj, APPLY_AFF2);
}

bool object_edit_location_affects_dam(int location) noexcept {
	return location == APPLY_DAMROLL || location == APPLY_HITNDAM;
}

bool object_edit_location_affects_spellpower(int location) noexcept {
	return location == APPLY_SPELLPOWER || location == APPLY_HITNSP;
}

bool object_edit_location_affects_spellfail(int location) noexcept {
	return location == APPLY_SPELLFAIL;
}

int object_edit_damroll_total(const struct obj_data* obj) noexcept {
	return combat_damroll_total(obj);
}

int object_edit_spellpower_total(const struct obj_data* obj) noexcept {
	return combat_spellpower_total(obj);
}

int object_edit_hitroll_total(const struct obj_data* obj) noexcept {
	return combat_hitroll_total(obj);
}

[[nodiscard]] static int resolve_edit_prototype_vnum(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	int iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	const bool useOriginal =
		(IS_OBJ_STAT2(obj, ITEM2_PERSONAL) || IS_OBJ_STAT2(obj, ITEM2_EDIT)) &&
		obj->char_vnum > 0 && static_cast<int>(obj->char_vnum) != iVNum;
	if(useOriginal) {
		return static_cast<int>(obj->char_vnum);
	}
	/* object_instance: prefer base world vnum when available. */
	const int base = object_instance_resolve_base_vnum(obj);
	if(base > 0) {
		return base;
	}
	return iVNum;
}

int object_edit_damroll_edited_delta(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	const int cur = combat_damroll_total(obj);
	struct obj_data* baseline = object_edit_load_baseline(obj);
	if(!baseline) {
		return 0;
	}
	const int base = combat_damroll_total(baseline);
	extract_obj(baseline);
	return std::max(0, cur - base);
}

int object_edit_spellpower_edited_delta(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	const int cur = combat_spellpower_total(obj);
	struct obj_data* baseline = object_edit_load_baseline(obj);
	if(!baseline) {
		return 0;
	}
	const int base = combat_spellpower_total(baseline);
	extract_obj(baseline);
	return std::max(0, cur - base);
}

int object_edit_spellfail_total(const struct obj_data* obj) noexcept {
	return sum_location_mod(obj, APPLY_SPELLFAIL);
}

int object_edit_spellfail_prototype_total(const struct obj_data* obj) noexcept {
	struct obj_data* baseline = object_edit_load_baseline(obj);
	if(!baseline) {
		return 0;
	}
	const int base = sum_location_mod(baseline, APPLY_SPELLFAIL);
	extract_obj(baseline);
	return base;
}

int object_edit_spellfail_edited_delta(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	/* Piu' negativo = piu' edit: baseline 0, current −20 → delta 20. */
	const int cur = sum_location_mod(obj, APPLY_SPELLFAIL);
	struct obj_data* baseline = object_edit_load_baseline(obj);
	if(!baseline) {
		return 0;
	}
	const int base = sum_location_mod(baseline, APPLY_SPELLFAIL);
	extract_obj(baseline);
	return std::max(0, base - cur);
}

int object_edit_hitroll_edited_delta(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	const int cur = combat_hitroll_total(obj);
	struct obj_data* baseline = object_edit_load_baseline(obj);
	if(!baseline) {
		return 0;
	}
	const int base = combat_hitroll_total(baseline);
	extract_obj(baseline);
	return std::max(0, cur - base);
}

int object_edit_damroll_prototype_total(const struct obj_data* obj) noexcept {
	if(!obj) {
		return 0;
	}
	struct obj_data* baseline = object_edit_load_baseline(obj);
	if(!baseline) {
		return 0;
	}
	const int base = combat_damroll_total(baseline);
	extract_obj(baseline);
	return base;
}

int object_edit_prototype_vnum(const struct obj_data* obj) noexcept {
	return resolve_edit_prototype_vnum(obj);
}

bool object_edit_counts_toward_combat_budget(const struct obj_data* obj,
											 const char* toon_name) noexcept {
	if(!obj || !toon_name || !*toon_name) {
		return false;
	}
	if(!IS_OBJ_STAT2(obj, ITEM2_EDIT)) {
		return false;
	}
	/* Simbolo di clan: dam/sp non entrano nei tetti edit di nessun toon. */
	if(clan_symbol_is_obj(obj) || obj->obj_flags.type_flag == ITEM_CLAN_SYMBOL) {
		return false;
	}
	/* Deve essere dell'owner (personal_owner o keyword ED<nome>). */
	if(obj->personal_owner[0] != '\0') {
		return strcasecmp(obj->personal_owner, toon_name) == 0;
	}
	const std::string ed = object_instance_extract_ed_owner(obj->name);
	return !ed.empty() && strcasecmp(ed.c_str(), toon_name) == 0;
}

[[nodiscard]] static int broker_delta_clamped_for_piece(unsigned long long iid, int loc,
													   int total_delta) noexcept {
	if(iid == 0 || total_delta <= 0) {
		return 0;
	}
	const long raw = object_instance_sum_broker_delta(iid, loc);
	return static_cast<int>(
		std::max(0L, std::min(static_cast<long>(total_delta), raw)));
}

/**
 * Quota [delta] wiz vs [trasferimento] broker sui canali combat (come
 * EditAffectBroker / AppendAffectDeltaLines). I trasferimenti non consumano
 * i tetti pezzo del portale.
 */
struct PieceCombatWizBreakdown {
	int hr_total = 0;
	int dam_total = 0;
	int sp_total = 0;
	int hr_broker = 0;
	int dam_broker = 0;
	int sp_broker = 0;

	[[nodiscard]] int hr_wiz() const noexcept {
		return std::max(0, hr_total - hr_broker);
	}
	[[nodiscard]] int dam_wiz() const noexcept {
		return std::max(0, dam_total - dam_broker);
	}
	[[nodiscard]] int sp_wiz() const noexcept {
		return std::max(0, sp_total - sp_broker);
	}
};

[[nodiscard]] static PieceCombatWizBreakdown piece_combat_wiz_breakdown(
	const struct obj_data* obj, const struct obj_data* frozen_baseline = nullptr) {
	PieceCombatWizBreakdown b {};
	if(!obj) {
		return b;
	}
	struct obj_data* owned_baseline = nullptr;
	const struct obj_data* baseline = frozen_baseline;
	if(!baseline) {
		owned_baseline = object_edit_load_baseline(obj);
		baseline = owned_baseline;
	}
	if(!baseline) {
		return b;
	}
	b.hr_total =
		std::max(0, combat_hitroll_total(obj) - combat_hitroll_total(baseline));
	b.dam_total =
		std::max(0, combat_damroll_total(obj) - combat_damroll_total(baseline));
	b.sp_total =
		std::max(0, combat_spellpower_total(obj) - combat_spellpower_total(baseline));
	if(owned_baseline) {
		extract_obj(owned_baseline);
	}

	const unsigned long long iid = obj->db_instance_id;
	const int br_hr = broker_delta_clamped_for_piece(iid, APPLY_HITROLL, b.hr_total);
	const int br_dam = broker_delta_clamped_for_piece(iid, APPLY_DAMROLL, b.dam_total);
	const int br_sp = broker_delta_clamped_for_piece(iid, APPLY_SPELLPOWER, b.sp_total);
	const int br_hd = broker_delta_clamped_for_piece(
		iid, APPLY_HITNDAM, std::min(b.hr_total, b.dam_total));
	const int br_hs = broker_delta_clamped_for_piece(
		iid, APPLY_HITNSP, std::min(b.hr_total, b.sp_total));
	b.hr_broker = std::min(b.hr_total, br_hr + br_hd + br_hs);
	b.dam_broker = std::min(b.dam_total, br_dam + br_hd);
	b.sp_broker = std::min(b.sp_total, br_sp + br_hs);
	return b;
}

ObjEditCombatWizCaps object_edit_combat_wiz_caps(const struct obj_data* obj) noexcept {
	const PieceCombatWizBreakdown b = piece_combat_wiz_breakdown(obj, nullptr);
	ObjEditCombatWizCaps out {};
	out.hr_full = b.hr_total;
	out.dam_full = b.dam_total;
	out.sp_full = b.sp_total;
	out.hr_broker = b.hr_broker;
	out.dam_broker = b.dam_broker;
	out.sp_broker = b.sp_broker;
	return out;
}

[[nodiscard]] static bool location_touches_piece_combat_caps(int location) noexcept {
	return location == APPLY_HITROLL || location == APPLY_DAMROLL ||
		   location == APPLY_SPELLPOWER || location == APPLY_HITNDAM ||
		   location == APPLY_HITNSP;
}

/**
 * Tetti pezzo sul solo [delta] wiz: HR<=2, dam+sp<=2.
 * I [trasferimento] broker restano fuori (budget separato).
 */
[[nodiscard]] static bool enforce_piece_combat_wiz_caps(
	const struct obj_data* after_obj, std::string& err,
	const struct obj_data* frozen_baseline = nullptr) {
	if(!after_obj) {
		return true;
	}
	const PieceCombatWizBreakdown b =
		piece_combat_wiz_breakdown(after_obj, frozen_baseline);
	if(b.hr_wiz() > kObjEditMaxHitrollPerPiece) {
		err = "tetto hitroll delta per pezzo superato (" + std::to_string(b.hr_wiz()) +
			  "/" + std::to_string(kObjEditMaxHitrollPerPiece)
			  + "; esclusi trasferimenti broker +" + std::to_string(b.hr_broker) + ")";
		return false;
	}
	const int dam_sp_wiz = b.dam_wiz() + b.sp_wiz();
	if(dam_sp_wiz > kObjEditMaxDamSpPerPiece) {
		err = "tetto dam+spellpower delta per pezzo superato (" +
			  std::to_string(dam_sp_wiz) + "/" + std::to_string(kObjEditMaxDamSpPerPiece)
			  + "; dam delta +" + std::to_string(b.dam_wiz()) + " sp delta +"
			  + std::to_string(b.sp_wiz()) + "; esclusi trasferimenti broker dam +"
			  + std::to_string(b.dam_broker) + " sp +" + std::to_string(b.sp_broker)
			  + ")";
		return false;
	}
	return true;
}

/** Tetto personaggio DAM+SP combinato <= 30 (come EditAffectBroker). */
[[nodiscard]] static bool enforce_char_dam_sp_budget(
	const struct obj_data* after_obj, int other_edited_dam, int other_edited_sp,
	std::string& err, const struct obj_data* frozen_baseline = nullptr) {
	if(!after_obj) {
		return true;
	}
	if(other_edited_dam < 0 && other_edited_sp < 0) {
		return true;
	}
	const int od = other_edited_dam < 0 ? 0 : other_edited_dam;
	const int os = other_edited_sp < 0 ? 0 : other_edited_sp;
	int piece_dam = 0;
	int piece_sp = 0;
	if(frozen_baseline) {
		piece_dam = std::max(
			0, combat_damroll_total(after_obj) - combat_damroll_total(frozen_baseline));
		piece_sp = std::max(0, combat_spellpower_total(after_obj) -
								   combat_spellpower_total(frozen_baseline));
	}
	else {
		piece_dam = object_edit_damroll_edited_delta(after_obj);
		piece_sp = object_edit_spellpower_edited_delta(after_obj);
	}
	const int total = od + os + piece_dam + piece_sp;
	if(total > kObjEditMaxDamSpEditableTotal) {
		err = "tetto DAM+SP personaggio superato (" + std::to_string(total) + "/"
			  + std::to_string(kObjEditMaxDamSpEditableTotal) + "; altri pezzi dam +"
			  + std::to_string(od) + " sp +" + std::to_string(os) + ", questo pezzo dam +"
			  + std::to_string(piece_dam) + " sp +" + std::to_string(piece_sp)
			  + " vs baseline; hit-n-dam conta come dam, hit-n-sp come spellpower)";
		return false;
	}
	return true;
}

[[nodiscard]] static bool enforce_char_spellfail_budget(const struct obj_data* after_obj,
														int other_edited_sf,
														std::string& err) {
	if(other_edited_sf < 0 || !after_obj) {
		return true;
	}
	const int piece_sf = object_edit_spellfail_edited_delta(after_obj);
	const int total = other_edited_sf + piece_sf;
	if(total > kObjEditMaxSpellfailEditableTotal) {
		err = "tetto spellfail editabile personaggio superato (" +
			  std::to_string(total) + "/" +
			  std::to_string(kObjEditMaxSpellfailEditableTotal)
			  + "; altri pezzi EDIT +" + std::to_string(other_edited_sf)
			  + ", questo pezzo +" + std::to_string(piece_sf) + " vs proto)";
		return false;
	}
	return true;
}

bool object_quote_affect_target(struct obj_data* obj, int location, int target_modifier,
								long& xp_raw, int& pq, std::string& err,
								int other_worn_edited_dam, int other_worn_edited_sp,
								bool clear_slot, int other_owned_edited_spellfail,
								bool allow_combat_merge, ObjEditMalusSplit* split_out) {
	if(split_out) {
		*split_out = {};
	}
	if(!obj) {
		err = "oggetto null";
		return false;
	}
	/*
	 * ITEM2_PAID_MALUS: senza flag AnalyzeObjEdit ignora i malus (NonNeg) e il
	 * recupero costerebbe 0. Se l'edit recupera un malus (o il pezzo ha gia' il
	 * flag), forziamo PAID_MALUS sui clone di analisi cosi' diff.valore include
	 * la tariffa 2× e, in un solo posto, class_mult + Artifact +50%.
	 */
	const bool recovers_malus =
		object_edit_recovers_listino_malus(obj, location, target_modifier, clear_slot);
	const bool use_paid_malus =
		IS_OBJ_STAT2(obj, ITEM2_PAID_MALUS) || recovers_malus;

	/*
	 * Baseline congelata prima dell'edit: per premi senza instance gli affect
	 * vivi sono la baseline sospesa; dopo la mutazione del clone non vanno
	 * rilette da object_edit_load_baseline(clone).
	 */
	struct obj_data* frozen_baseline = object_edit_load_baseline(obj);
	if(!frozen_baseline) {
		err = "baseline pezzo non disponibile";
		return false;
	}

	struct obj_data* clone = portal_clone_obj_state(obj);
	if(!clone) {
		extract_obj(frozen_baseline);
		err = "impossibile clonare oggetto";
		return false;
	}
	if(use_paid_malus) {
		portal_force_paid_malus(clone);
	}
	if(!apply_target_modifier(clone, location, target_modifier, err, clear_slot,
							  allow_combat_merge)) {
		extract_obj(clone);
		extract_obj(frozen_baseline);
		return false;
	}
	if(location_touches_piece_combat_caps(location)
	   && !enforce_piece_combat_wiz_caps(clone, err, frozen_baseline)) {
		extract_obj(clone);
		extract_obj(frozen_baseline);
		return false;
	}
	if((object_edit_location_affects_dam(location)
		|| object_edit_location_affects_spellpower(location))
	   && !enforce_char_dam_sp_budget(clone, other_worn_edited_dam,
									  other_worn_edited_sp, err, frozen_baseline)) {
		extract_obj(clone);
		extract_obj(frozen_baseline);
		return false;
	}
	if(object_edit_location_affects_spellfail(location)
	   && !enforce_char_spellfail_budget(clone, other_owned_edited_spellfail, err)) {
		extract_obj(clone);
		extract_obj(frozen_baseline);
		return false;
	}

	/*
	 * Recupero malus: 2× sul tratto fino a 0. Se il target e' un bonus oltre
	 * lo zero (AC +10→−10), spezza: 2×(cur→0) + 1×(0→target).
	 */
	if(recovers_malus) {
		const int cur = object_edit_display_current(obj, location);
		pq = 0;
		if(!clear_slot &&
		   object_edit_malus_crosses_to_bonus(location, cur, target_modifier)) {
			const long malus_unit =
				EditAffectDeltaListinoCost(obj, location, 0 - cur);
			const long malus_xp = (malus_unit > 0) ? (malus_unit * 2) : 0L;

			struct obj_data* mid = portal_clone_obj_state(obj);
			if(!mid) {
				extract_obj(clone);
				extract_obj(frozen_baseline);
				err = "impossibile clonare oggetto";
				return false;
			}
			std::string mid_err;
			if(!apply_target_modifier(mid, location, 0, mid_err, false, false)) {
				extract_obj(mid);
				extract_obj(clone);
				extract_obj(frozen_baseline);
				err = mid_err.empty() ? "impossibile azzerare malus in quote" : mid_err;
				return false;
			}
			portal_force_paid_malus(mid);
			const long bonus_xp =
				EditAffectDeltaListinoCost(mid, location, target_modifier);
			extract_obj(mid);

			xp_raw = malus_xp + std::max(0L, bonus_xp);
			if(split_out) {
				split_out->active = true;
				split_out->malus_xp_raw = malus_xp;
				split_out->bonus_xp_raw = std::max(0L, bonus_xp);
				split_out->from = cur;
				split_out->to = target_modifier;
			}
			extract_obj(clone);
			extract_obj(frozen_baseline);
			return true;
		}

		const int delta_mod = target_modifier - cur;
		const long unit = EditAffectDeltaListinoCost(obj, location, delta_mod);
		xp_raw = (unit > 0) ? (unit * 2) : 0L;
		extract_obj(clone);
		extract_obj(frozen_baseline);
		return true;
	}

	struct obj_data* before_obj = portal_clone_obj_state(obj);
	if(!before_obj) {
		extract_obj(clone);
		extract_obj(frozen_baseline);
		err = "impossibile clonare oggetto";
		return false;
	}
	if(use_paid_malus) {
		portal_force_paid_malus(before_obj);
	}
	const ObjEditAnalysis before =
		AnalyzeObjEditAgainst(before_obj, frozen_baseline, true);
	extract_obj(before_obj);

	const ObjEditAnalysis after =
		AnalyzeObjEditAgainst(clone, frozen_baseline, true);
	extract_obj(clone);
	extract_obj(frozen_baseline);

	/* diff.valore e' gia' scalato e include class_mult + Artifact. */
	xp_raw = std::max(0L, after.diff.valore - before.diff.valore);
	pq = std::max(0, after.diff.rune - before.diff.rune);
	return true;
}

bool object_apply_affect_target(struct obj_data* obj, int location, int target_modifier,
								std::string& err, int other_worn_edited_dam,
								int other_worn_edited_sp, bool clear_slot,
								int other_owned_edited_spellfail,
								bool allow_combat_merge) {
	if(!obj) {
		err = "oggetto null";
		return false;
	}
	const bool recovers_malus =
		object_edit_recovers_listino_malus(obj, location, target_modifier, clear_slot);
	const bool need_budget =
		location_touches_piece_combat_caps(location)
		|| ((object_edit_location_affects_dam(location)
			 || object_edit_location_affects_spellpower(location))
			&& (other_worn_edited_dam >= 0 || other_worn_edited_sp >= 0))
		|| (object_edit_location_affects_spellfail(location)
			&& other_owned_edited_spellfail >= 0);
	if(need_budget) {
		struct obj_data* frozen_baseline = object_edit_load_baseline(obj);
		if(!frozen_baseline) {
			err = "baseline pezzo non disponibile";
			return false;
		}
		struct obj_data* clone = portal_clone_obj_state(obj);
		if(!clone) {
			extract_obj(frozen_baseline);
			err = "impossibile clonare oggetto";
			return false;
		}
		if(!apply_target_modifier(clone, location, target_modifier, err, clear_slot,
								  allow_combat_merge)) {
			extract_obj(clone);
			extract_obj(frozen_baseline);
			return false;
		}
		if(location_touches_piece_combat_caps(location)
		   && !enforce_piece_combat_wiz_caps(clone, err, frozen_baseline)) {
			extract_obj(clone);
			extract_obj(frozen_baseline);
			return false;
		}
		if((object_edit_location_affects_dam(location)
			|| object_edit_location_affects_spellpower(location))
		   && !enforce_char_dam_sp_budget(clone, other_worn_edited_dam,
										  other_worn_edited_sp, err,
										  frozen_baseline)) {
			extract_obj(clone);
			extract_obj(frozen_baseline);
			return false;
		}
		if(!enforce_char_spellfail_budget(clone, other_owned_edited_spellfail, err)) {
			extract_obj(clone);
			extract_obj(frozen_baseline);
			return false;
		}
		extract_obj(clone);
		extract_obj(frozen_baseline);
	}
	if(!apply_target_modifier(obj, location, target_modifier, err, clear_slot,
							  allow_combat_merge)) {
		return false;
	}
	/* Persistenza: dopo aver pagato il recupero malus a 2×, il pezzo deve avere
	 * ITEM2_PAID_MALUS (Montero: listino si, edit pool no). */
	if(recovers_malus) {
		portal_force_paid_malus(obj);
	}
	return true;
}

} // namespace Alarmud
