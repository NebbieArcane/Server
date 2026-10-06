/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/***************************  System  include ************************************/
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
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
#include "edit_affect_broker.hpp"
#include "act.other.hpp"
#include "comm.hpp"
#include "db.hpp"
#include "handler.hpp"
#include "interpreter.hpp"
#include "obj_value.hpp"
#include "object_instance.hpp"
#include "spec_procs.hpp"
#include "utility.hpp"

namespace Alarmud {
namespace {

constexpr long kEditBrokerPrinceFloor = PRINCEEXP;
constexpr int kEditBrokerPercentKeep = 25; /* pagamento / rimborso = 25% del listino */

struct AffectPick {
	int location{APPLY_NONE};
	int delta{0}; /* modifier numerico oppure bitmask da trasferire */
	std::string label; /* nome umano per messaggi/history */
};

[[nodiscard]] std::string normalize_key(std::string_view in) {
	std::string out;
	out.reserve(in.size());
	for(unsigned char c : in) {
		if(std::isalnum(c)) {
			out.push_back(static_cast<char>(UPPER(c)));
		}
	}
	return out;
}

[[nodiscard]] std::pair<std::string, std::string_view> next_arg(std::string_view input) {
	for(;;) {
		while(!input.empty() && std::isspace(static_cast<unsigned char>(input.front()))) {
			input.remove_prefix(1);
		}
		if(input.empty()) {
			return {"", {}};
		}
		std::size_t len = 0;
		while(len < input.size() && static_cast<unsigned char>(input[len]) > ' ') {
			++len;
		}
		std::string word;
		word.reserve(len);
		for(std::size_t i = 0; i < len; ++i) {
			word.push_back(LOWER(input[i]));
		}
		input.remove_prefix(len);
		if(!fill_word(word.c_str())) {
			return {std::move(word), input};
		}
	}
}

[[nodiscard]] bool ask_is_for_mob(struct char_data* ch, const char* arg, struct char_data* mob,
								  std::string& rest) {
	const auto [who, after] = next_arg(arg ? arg : "");
	if(who.empty()) {
		return false;
	}
	struct char_data* vict = get_char_room_vis(ch, who.c_str());
	if(vict != mob) {
		return false;
	}
	rest.assign(after.begin(), after.end());
	while(!rest.empty() && std::isspace(static_cast<unsigned char>(rest.front()))) {
		rest.erase(rest.begin());
	}
	return true;
}

void tell_broker(struct char_data* ch, struct char_data* mob, const char* msg) {
	act(("$N ti dice '" + std::string(msg) + "'").c_str(), FALSE, ch, nullptr, mob, TO_CHAR);
}

[[nodiscard]] bool is_bitfield_location(int loc) noexcept {
	return loc == APPLY_AFF2 || loc == APPLY_IMMUNE || loc == APPLY_M_IMMUNE ||
		   loc == APPLY_SPELL || loc == APPLY_SUSC;
}

/** Affect che non si sommano: serve uno slot libero sul target. */
[[nodiscard]] bool is_nonstackable_location(int loc) noexcept {
	return is_bitfield_location(loc) || loc == APPLY_WEAPON_SPELL || loc == APPLY_EAT_SPELL ||
		   loc == APPLY_RACE_SLAYER || loc == APPLY_ALIGN_SLAYER;
}

[[nodiscard]] long sum_location(const struct obj_data* obj, int loc) {
	long sum = 0;
	if(!obj) {
		return 0;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc) {
			sum += obj->affected[i].modifier;
		}
	}
	return sum;
}

[[nodiscard]] unsigned or_bits(const struct obj_data* obj, int loc) {
	unsigned bits = 0;
	if(!obj) {
		return 0;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc) {
			bits |= static_cast<unsigned>(obj->affected[i].modifier);
		}
	}
	return bits;
}

[[nodiscard]] bool has_location(const struct obj_data* obj, int loc) {
	if(!obj) {
		return false;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc && obj->affected[i].modifier != 0) {
			return true;
		}
	}
	return false;
}

[[nodiscard]] int find_free_affect_slot(const struct obj_data* obj) {
	if(!obj) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == APPLY_NONE || obj->affected[i].location == 0) {
			return i;
		}
	}
	return -1;
}

[[nodiscard]] int find_location_slot(const struct obj_data* obj, int loc) {
	if(!obj) {
		return -1;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location == loc) {
			return i;
		}
	}
	return -1;
}

[[nodiscard]] struct obj_data* load_prototype(const struct obj_data* obj) {
	if(!obj) {
		return nullptr;
	}
	int iVNum = object_instance_resolve_base_vnum(obj);
	if(iVNum <= 0) {
		iVNum = (obj->item_number >= 0) ? obj_index[obj->item_number].iVNum : 0;
	}
	const int rNum = real_object(iVNum);
	if(rNum < 0) {
		return nullptr;
	}
	return read_object(rNum, REAL);
}

[[nodiscard]] std::string apply_display_name(int loc) {
	char buf[MAX_STRING_LENGTH];
	sprinttype(loc, apply_types, buf);
	return buf;
}

[[nodiscard]] std::string bit_display_name(int loc, unsigned bits) {
	char buf[MAX_STRING_LENGTH];
	buf[0] = '\0';
	if(loc == APPLY_SPELL) {
		sprintbit(bits, affected_bits, buf);
	}
	else if(loc == APPLY_AFF2) {
		sprintbit(bits, affected_bits2, buf);
	}
	else if(loc == APPLY_IMMUNE || loc == APPLY_M_IMMUNE || loc == APPLY_SUSC) {
		sprintbit(bits, immunity_names, buf);
	}
	else {
		snprintf(buf, sizeof(buf), "%u", bits);
	}
	return buf;
}

[[nodiscard]] bool match_bit_table(const char* names[], const std::string& key, unsigned& bit_out) {
	if(!names || key.empty()) {
		return false;
	}
	for(int i = 0; names[i] && names[i][0] != '\n'; ++i) {
		if(normalize_key(names[i]) == key) {
			bit_out = (1u << i);
			return true;
		}
	}
	return false;
}

/**
 * Risolve il nome dato dal toon in un affect presente come delta vs prototipo su src.
 * Accetta nomi apply_types (HITROLL, DAMROLL, SPELL AFFECT, ...) oppure bit
 * (DARKNESS, FIRE, ...).
 */
[[nodiscard]] bool resolve_affect_pick(struct obj_data* src, const std::string& raw_name,
									   AffectPick& out, std::string& err) {
	out = AffectPick{};
	const std::string key = normalize_key(raw_name);
	if(key.empty()) {
		err = "Devi specificare il nome dell'effetto da trasferire.";
		return false;
	}

	struct obj_data* proto = load_prototype(src);
	if(!proto) {
		err = "Non riesco a confrontare l'oggetto con il suo prototipo.";
		return false;
	}

	auto finish_fail = [&](const char* msg) {
		extract_obj(proto);
		err = msg;
		return false;
	};

	/* 1) Match su apply_types (location intera). */
	int matched_loc = APPLY_NONE;
	for(int i = 0; apply_types[i] && apply_types[i][0] != '\n'; ++i) {
		if(normalize_key(apply_types[i]) == key) {
			matched_loc = i;
			break;
		}
	}

	if(matched_loc != APPLY_NONE && matched_loc != APPLY_SKIP) {
		if(is_bitfield_location(matched_loc)) {
			const unsigned added =
				or_bits(src, matched_loc) & ~or_bits(proto, matched_loc);
			if(added == 0) {
				return finish_fail(
					"Su quell'oggetto non c'e' un delta di quell'effetto rispetto al prototipo.");
			}
			out.location = matched_loc;
			out.delta = static_cast<int>(added);
			out.label = apply_display_name(matched_loc) + " " +
						bit_display_name(matched_loc, added);
			extract_obj(proto);
			return true;
		}

		const long cur = sum_location(src, matched_loc);
		const long base = sum_location(proto, matched_loc);
		long delta = 0;
		if(matched_loc == APPLY_AC) {
			if(cur < base) {
				delta = cur - base; /* negativo = miglioramento */
			}
		}
		else if(cur > base) {
			delta = cur - base;
		}
		if(delta == 0) {
			return finish_fail(
				"Su quell'oggetto non c'e' un delta di quell'effetto rispetto al prototipo.");
		}
		out.location = matched_loc;
		out.delta = static_cast<int>(delta);
		out.label = apply_display_name(matched_loc) + " by " + std::to_string(delta);
		extract_obj(proto);
		return true;
	}

	/* 2) Match su singolo bit (SPELL / IMMUNE / AFF2). */
	unsigned bit = 0;
	int bit_loc = APPLY_NONE;
	if(match_bit_table(affected_bits, key, bit)) {
		bit_loc = APPLY_SPELL;
	}
	else if(match_bit_table(affected_bits2, key, bit)) {
		bit_loc = APPLY_AFF2;
	}
	else if(match_bit_table(immunity_names, key, bit)) {
		/* Preferisci la location dove il bit e' stato aggiunto vs proto. */
		const unsigned add_imm = or_bits(src, APPLY_IMMUNE) & ~or_bits(proto, APPLY_IMMUNE);
		const unsigned add_mimm =
			or_bits(src, APPLY_M_IMMUNE) & ~or_bits(proto, APPLY_M_IMMUNE);
		const unsigned add_susc = or_bits(src, APPLY_SUSC) & ~or_bits(proto, APPLY_SUSC);
		if(add_imm & bit) {
			bit_loc = APPLY_IMMUNE;
		}
		else if(add_mimm & bit) {
			bit_loc = APPLY_M_IMMUNE;
		}
		else if(add_susc & bit) {
			bit_loc = APPLY_SUSC;
		}
		else {
			bit_loc = APPLY_IMMUNE; /* fallback per messaggio di assenza */
		}
	}

	if(bit_loc == APPLY_NONE) {
		return finish_fail(
			"Non riconosco quel nome di effetto. Usa il nome come in stat (es. DAMROLL, DARKNESS).");
	}

	const unsigned added = or_bits(src, bit_loc) & ~or_bits(proto, bit_loc);
	if((added & bit) == 0) {
		return finish_fail(
			"Su quell'oggetto non c'e' un delta di quell'effetto rispetto al prototipo.");
	}

	out.location = bit_loc;
	out.delta = static_cast<int>(bit);
	out.label = apply_display_name(bit_loc) + " " + bit_display_name(bit_loc, bit);
	extract_obj(proto);
	return true;
}

[[nodiscard]] bool obj_is_owned_edit(struct char_data* ch, struct obj_data* obj) {
	return obj && IS_OBJ_STAT2(obj, ITEM2_EDIT) && IS_OBJ_STAT2(obj, ITEM2_PERSONAL) &&
		   pers_on(ch, obj);
}

[[nodiscard]] long percent_of_listino(long listino_cost) {
	if(listino_cost <= 0) {
		return 0;
	}
	return (listino_cost * kEditBrokerPercentKeep) / 100;
}

[[nodiscard]] bool can_afford_prince_floor(struct char_data* ch, long cost) {
	if(cost <= 0) {
		return true;
	}
	return (static_cast<long long>(GET_EXP(ch)) - static_cast<long long>(cost)) >=
		   kEditBrokerPrinceFloor;
}

void remove_numeric_delta(struct obj_data* obj, int loc, int delta) {
	/* delta > 0: togliere delta dai modifier; delta < 0 (AC): aggiungere |delta| verso proto. */
	if(!obj || delta == 0) {
		return;
	}
	if(delta > 0) {
		int left = delta;
		for(int i = 0; i < MAX_OBJ_AFFECT && left > 0; ++i) {
			if(obj->affected[i].location != loc) {
				continue;
			}
			const int take = std::min(obj->affected[i].modifier, left);
			obj->affected[i].modifier -= take;
			left -= take;
			if(obj->affected[i].modifier == 0) {
				obj->affected[i].location = APPLY_NONE;
			}
		}
		return;
	}
	/* AC improvement (delta negativo): alza il modifier verso il proto. */
	int left = -delta;
	for(int i = 0; i < MAX_OBJ_AFFECT && left > 0; ++i) {
		if(obj->affected[i].location != loc) {
			continue;
		}
		obj->affected[i].modifier += left;
		left = 0;
	}
}

void add_numeric_delta(struct obj_data* obj, int loc, int delta) {
	if(!obj || delta == 0) {
		return;
	}
	const int slot = find_location_slot(obj, loc);
	if(slot < 0) {
		return;
	}
	obj->affected[slot].modifier += delta;
}

void remove_bit_delta(struct obj_data* obj, int loc, unsigned bits) {
	if(!obj || bits == 0) {
		return;
	}
	for(int i = 0; i < MAX_OBJ_AFFECT; ++i) {
		if(obj->affected[i].location != loc) {
			continue;
		}
		obj->affected[i].modifier =
			static_cast<int>(static_cast<unsigned>(obj->affected[i].modifier) & ~bits);
		if(obj->affected[i].modifier == 0) {
			obj->affected[i].location = APPLY_NONE;
		}
	}
}

[[nodiscard]] bool add_bit_delta_new_slot(struct obj_data* obj, int loc, unsigned bits) {
	if(!obj || bits == 0) {
		return false;
	}
	const int slot = find_free_affect_slot(obj);
	if(slot < 0) {
		return false;
	}
	obj->affected[slot].location = static_cast<short>(loc);
	obj->affected[slot].modifier = static_cast<int>(bits);
	return true;
}

[[nodiscard]] bool persist_edit_obj(struct obj_data* obj, struct char_data* actor,
									const char* kind, const char* note, const char* detail) {
	if(!obj || obj->db_instance_id == 0) {
		return false;
	}
	const int base = object_instance_resolve_base_vnum(obj);
	if(base <= 0) {
		return false;
	}
	if(object_instance_persist(obj, base, obj->db_instance_id, actor, true) == 0) {
		return false;
	}
	if(kind && *kind) {
		object_instance_append_event(obj->db_instance_id, kind, note, detail, nullptr, actor);
	}
	return true;
}

void show_usage(struct char_data* ch, struct char_data* mob) {
	tell_broker(ch, mob,
				"Posso trasferire un effetto tra due tuoi oggetti EDIT, oppure distruggerne uno.");
	send_to_char(
		"$c0015Comandi:\n\r"
		"  $c0010ask$c0007 <me> $c0011trasferisci$c0007 <effetto> <oggettoA> <oggettoB>\n\r"
		"  $c0010ask$c0007 <me> $c0011distruggi$c0007 <oggettoC>\n\r"
		"$c0015Trasferisci:$c0007 sposta l'intero delta vs prototipo di <effetto> da A a B.\n\r"
		"  Costo: 25% del listino dell'effetto (classi/artifact inclusi). Floor XP 400M.\n\r"
		"$c0015Distruggi:$c0007 soft-delete dell'edit e rimborso 25% del valore vs prototipo.\n\r",
		ch);
}

void do_trasferisci(struct char_data* ch, struct char_data* mob, std::string_view args) {
	auto [aff_name, rest1] = next_arg(args);
	auto [name_a, rest2] = next_arg(rest1);
	auto [name_b, rest3] = next_arg(rest2);
	(void)rest3;

	if(aff_name.empty() || name_a.empty() || name_b.empty()) {
		tell_broker(ch, mob, "Sintassi: trasferisci <effetto> <oggettoA> <oggettoB>.");
		return;
	}

	struct obj_data* obj_a = get_obj_in_list_vis(ch, name_a.c_str(), ch->carrying);
	struct obj_data* obj_b = get_obj_in_list_vis(ch, name_b.c_str(), ch->carrying);
	if(!obj_a) {
		tell_broker(ch, mob, "Non vedo l'oggetto A nel tuo inventario.");
		return;
	}
	if(!obj_b) {
		tell_broker(ch, mob, "Non vedo l'oggetto B nel tuo inventario.");
		return;
	}
	if(obj_a == obj_b) {
		tell_broker(ch, mob, "Oggetto A e oggetto B devono essere due pezzi distinti.");
		return;
	}
	if(!obj_is_owned_edit(ch, obj_a)) {
		tell_broker(ch, mob,
					"L'oggetto A deve essere EDIT, PERSONAL e di tua proprieta'.");
		return;
	}
	if(!obj_is_owned_edit(ch, obj_b)) {
		tell_broker(ch, mob,
					"L'oggetto B deve essere EDIT, PERSONAL e di tua proprieta'.");
		return;
	}
	if(obj_a->db_instance_id == 0 || obj_b->db_instance_id == 0) {
		tell_broker(ch, mob,
					"Uno dei due oggetti non e' ancora collegato al database edit. "
					"Contatta uno staffer.");
		mudlog(LOG_ERROR,
			   "EditAffectBroker transfer: missing instance_id A=%llu B=%llu owner=%s",
			   static_cast<unsigned long long>(obj_a->db_instance_id),
			   static_cast<unsigned long long>(obj_b->db_instance_id), GET_NAME(ch));
		return;
	}

	AffectPick pick;
	std::string err;
	if(!resolve_affect_pick(obj_a, aff_name, pick, err)) {
		tell_broker(ch, mob, err.c_str());
		mudlog(LOG_PLAYERS, "EditAffectBroker transfer denied %s: %s (affect=%s A=%s)",
			   GET_NAME(ch), err.c_str(), aff_name.c_str(),
			   obj_a->short_description ? obj_a->short_description : "?");
		return;
	}

	/* Controlli su B PRIMA di qualsiasi mutazione / DB. */
	if(is_nonstackable_location(pick.location)) {
		if(find_free_affect_slot(obj_b) < 0) {
			tell_broker(ch, mob,
						"L'oggetto B non ha uno slot affect libero per questo effetto "
						"(non sommabile).");
			mudlog(LOG_PLAYERS,
				   "EditAffectBroker transfer denied %s: no free slot on B for %s",
				   GET_NAME(ch), pick.label.c_str());
			return;
		}
	}
	else {
		if(!has_location(obj_b, pick.location)) {
			tell_broker(ch, mob,
						"L'oggetto B non ha gia' lo stesso effetto: non posso sommerlo.");
			mudlog(LOG_PLAYERS,
				   "EditAffectBroker transfer denied %s: B missing %s", GET_NAME(ch),
				   pick.label.c_str());
			return;
		}
	}

	const long listino =
		EditAffectDeltaListinoCost(obj_a, pick.location, pick.delta);
	const long payment = percent_of_listino(listino);
	if(!can_afford_prince_floor(ch, payment)) {
		char buf[256];
		snprintf(buf, sizeof(buf),
				 "Non hai abbastanza esperienza. Servono %ld XP (restando almeno %ld).",
				 payment, kEditBrokerPrinceFloor);
		tell_broker(ch, mob, buf);
		mudlog(LOG_PLAYERS,
			   "EditAffectBroker transfer denied %s: XP floor (need %ld have %ld) affect=%s",
			   GET_NAME(ch), payment, static_cast<long>(GET_EXP(ch)), pick.label.c_str());
		return;
	}

	/* Snapshot per rollback se lo slot su B sparisce tra check e write. */
	struct obj_affected_type snap_a[MAX_OBJ_AFFECT];
	struct obj_affected_type snap_b[MAX_OBJ_AFFECT];
	memcpy(snap_a, obj_a->affected, sizeof(snap_a));
	memcpy(snap_b, obj_b->affected, sizeof(snap_b));

	auto restore_snaps = [&]() {
		memcpy(obj_a->affected, snap_a, sizeof(snap_a));
		memcpy(obj_b->affected, snap_b, sizeof(snap_b));
	};

	/* --- da qui operazioni in memoria e DB --- */
	if(is_bitfield_location(pick.location)) {
		remove_bit_delta(obj_a, pick.location, static_cast<unsigned>(pick.delta));
		if(!add_bit_delta_new_slot(obj_b, pick.location,
								   static_cast<unsigned>(pick.delta))) {
			restore_snaps();
			tell_broker(ch, mob, "Operazione fallita: slot su B non piu' disponibile.");
			mudlog(LOG_ERROR, "EditAffectBroker transfer race: no slot B for %s",
				   GET_NAME(ch));
			return;
		}
	}
	else if(is_nonstackable_location(pick.location)) {
		remove_numeric_delta(obj_a, pick.location, pick.delta);
		const int free_slot = find_free_affect_slot(obj_b);
		if(free_slot < 0) {
			restore_snaps();
			tell_broker(ch, mob, "Operazione fallita: slot su B non piu' disponibile.");
			mudlog(LOG_ERROR, "EditAffectBroker transfer race: no slot B for %s",
				   GET_NAME(ch));
			return;
		}
		obj_b->affected[free_slot].location = static_cast<short>(pick.location);
		obj_b->affected[free_slot].modifier = pick.delta;
	}
	else {
		remove_numeric_delta(obj_a, pick.location, pick.delta);
		add_numeric_delta(obj_b, pick.location, pick.delta);
	}

	if(payment > 0) {
		GET_EXP(ch) = static_cast<int>(static_cast<long long>(GET_EXP(ch)) - payment);
	}

	char note_a[256];
	char note_b[256];
	char detail[512];
	snprintf(note_a, sizeof(note_a), "transfer remove %s -> instance %llu", pick.label.c_str(),
			 static_cast<unsigned long long>(obj_b->db_instance_id));
	snprintf(note_b, sizeof(note_b), "transfer add %s <- instance %llu", pick.label.c_str(),
			 static_cast<unsigned long long>(obj_a->db_instance_id));
	snprintf(detail, sizeof(detail),
			 "affect=%s payment_xp=%ld listino=%ld actor=%s", pick.label.c_str(), payment,
			 listino, GET_NAME(ch));

	const bool ok_a =
		persist_edit_obj(obj_a, ch, "affect_transfer", note_a, detail);
	const bool ok_b =
		persist_edit_obj(obj_b, ch, "affect_transfer", note_b, detail);
	if(!ok_a || !ok_b) {
		tell_broker(ch, mob,
					"Trasferimento applicato in memoria ma salvataggio DB parziale. "
					"Avvisa immediatamente uno staffer.");
		mudlog(LOG_SYSERR,
			   "EditAffectBroker transfer persist fail owner=%s A=%llu(%d) B=%llu(%d) %s",
			   GET_NAME(ch), static_cast<unsigned long long>(obj_a->db_instance_id), ok_a,
			   static_cast<unsigned long long>(obj_b->db_instance_id), ok_b,
			   pick.label.c_str());
	}

	schedule_inventory_save(ch);
	save_char(ch, AUTO_RENT, 0);

	char okmsg[320];
	snprintf(okmsg, sizeof(okmsg),
			 "Fatto: trasferito %s. Ti ho addebitato %ld XP (25%% del listino).",
			 pick.label.c_str(), payment);
	tell_broker(ch, mob, okmsg);
	mudlog(LOG_PLAYERS,
		   "EditAffectBroker transfer OK %s affect=%s A_inst=%llu B_inst=%llu pay=%ld listino=%ld",
		   GET_NAME(ch), pick.label.c_str(),
		   static_cast<unsigned long long>(obj_a->db_instance_id),
		   static_cast<unsigned long long>(obj_b->db_instance_id), payment, listino);
}

void do_distruggi(struct char_data* ch, struct char_data* mob, std::string_view args) {
	auto [name_c, rest] = next_arg(args);
	(void)rest;
	if(name_c.empty()) {
		tell_broker(ch, mob, "Sintassi: distruggi <oggettoC>.");
		return;
	}

	struct obj_data* obj = get_obj_in_list_vis(ch, name_c.c_str(), ch->carrying);
	if(!obj) {
		tell_broker(ch, mob, "Non vedo quell'oggetto nel tuo inventario.");
		return;
	}
	if(!obj_is_owned_edit(ch, obj)) {
		tell_broker(ch, mob,
					"L'oggetto deve essere EDIT, PERSONAL e di tua proprieta'.");
		return;
	}
	if(obj->db_instance_id == 0) {
		tell_broker(ch, mob,
					"L'oggetto non e' collegato al database edit. Contatta uno staffer.");
		mudlog(LOG_ERROR, "EditAffectBroker destroy: missing instance_id owner=%s",
			   GET_NAME(ch));
		return;
	}

	const ObjEditAnalysis edit = AnalyzeObjEdit(obj);
	const long refund = percent_of_listino(edit.diff.valore);
	const unsigned long long inst = obj->db_instance_id;
	const std::string shortn =
		obj->short_description ? obj->short_description : std::string("?");

	/* Soft-delete DB prima di togliere l'oggetto dal mondo. */
	if(!object_instance_delete(inst, ch)) {
		tell_broker(ch, mob, "Non sono riuscito a cancellare l'edit nel database. Operazione annullata.");
		mudlog(LOG_SYSERR, "EditAffectBroker destroy delete fail inst=%llu owner=%s",
			   static_cast<unsigned long long>(inst), GET_NAME(ch));
		return;
	}

	obj_from_char(obj);
	extract_obj(obj);

	if(refund > 0) {
		const long long cur = static_cast<long long>(GET_EXP(ch));
		const long long next = std::min(cur + static_cast<long long>(refund),
										static_cast<long long>(MAX_XP));
		GET_EXP(ch) = static_cast<int>(next);
	}

	schedule_inventory_save(ch);
	save_char(ch, AUTO_RENT, 0);

	char okmsg[320];
	snprintf(okmsg, sizeof(okmsg),
			 "Ho distrutto %s. Ti rimborso %ld XP (25%% del valore vs prototipo).",
			 shortn.c_str(), refund);
	tell_broker(ch, mob, okmsg);
	mudlog(LOG_PLAYERS,
		   "EditAffectBroker destroy OK %s inst=%llu refund=%ld listino=%ld short=%s",
		   GET_NAME(ch), static_cast<unsigned long long>(inst), refund, edit.diff.valore,
		   shortn.c_str());
}

} // namespace

MOBSPECIAL_FUNC(EditAffectBroker) {
	if(!ch || !mob || type != EVENT_COMMAND) {
		return FALSE;
	}
	if(cmd != CMD_ASK) {
		return FALSE;
	}

	std::string rest;
	if(!ask_is_for_mob(ch, arg, mob, rest)) {
		return FALSE;
	}

	if(IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
		return FALSE;
	}
	if(IS_POLY(ch)) {
		tell_broker(ch, mob, "Mi dispiace, non puoi farlo in questa forma.");
		return TRUE;
	}
	if(!IS_PC(ch)) {
		tell_broker(ch, mob, "Mi dispiace, non posso aiutarti.");
		return TRUE;
	}

	const auto [topic, after] = next_arg(rest);
	if(topic.empty() || topic == "aiuto" || topic == "help") {
		show_usage(ch, mob);
		return TRUE;
	}
	if(topic == "trasferisci") {
		do_trasferisci(ch, mob, after);
		return TRUE;
	}
	if(topic == "distruggi") {
		do_distruggi(ch, mob, after);
		return TRUE;
	}

	show_usage(ch, mob);
	return TRUE;
}

} // namespace Alarmud
