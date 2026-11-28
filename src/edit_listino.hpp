/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __EDIT_LISTINO_HPP
#define __EDIT_LISTINO_HPP

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace Alarmud {

struct obj_data;

/**
 * Riga di default del listino unico (seed in obj_value.cpp).
 * Prezzi base, senza moltiplicatore artifact/classe.
 */
struct EditListinoDefault {
	const char* code;
	const char* label;
	const char* category;
	long xp;
	int rune_num;
	int rune_den;
	int sign; /* +1 migliore se positivo; -1 migliore se negativo (ac, spellfail) */
	int cap_obj;
	int step; /* listino normale: unita' per slot (default 1). AC: 10 */
	int on_obj;
	int on_pc;
	int cap_pc;
	const char* cap_group;
	int over_cap;
	int over_step;
	int over_rune;
	long over_xp;
	long over_gold;
	int over_inc_pct;
	int apply_loc;
	unsigned long apply_bit;
	int resist_equiv;       /* +50 resi, +100 immu, -50 susc; 0 altrimenti */
	const char* resist_code; /* riga numerica resist_<elem>, o "" */
	int buy_step;           /* acquisto pool: unita' per slot (hp/mana/move 10, regen 5) */
	int enabled;            /* 1 = vendibile; serve anche on_obj/on_pc */
};

struct EditListinoCapGroupDefault {
	const char* code;
	const char* label;
};

struct EditListinoRuleDefault {
	const char* conf_key;
	int num;
	int den;
};

/** Copia runtime (cache) di una riga edit_listino. */
struct EditListinoEntry {
	std::string code;
	std::string label;
	std::string category;
	long xp = 0;
	int rune_num = 0;
	int rune_den = 1;
	int sign = 1;
	int cap_obj = 0;
	int step = 1;
	int on_obj = 0;
	int on_pc = 0;
	int cap_pc = 0;
	std::string cap_group;
	int over_cap = 0;
	int over_step = 0;
	int over_rune = 0;
	long over_xp = 0;
	long over_gold = 0;
	int over_inc_pct = 0;
	int apply_loc = 0;
	unsigned long apply_bit = 0;
	int resist_equiv = 0;
	std::string resist_code;
	int buy_step = 1;
	int enabled = 0;
	int mud_supported = 0; /* 1 se il codice e' nel seed MUD */
};

struct EditListinoRules {
	int artifact_mult_num = 3;
	int artifact_mult_den = 2;
	int class_mult_1_num = 1;
	int class_mult_1_den = 1;
	int class_mult_2_num = 3;
	int class_mult_2_den = 2;
	int class_mult_3_num = 2;
	int class_mult_3_den = 1;
	int malus_mult_num = 2;
	int malus_mult_den = 1;
	int xp_split_by_classes = 1;
	int rune_rounding_ceil = 1;
	int over_progression = 1; /* 1 = lineare (slot n: base + (n-1)*base*inc/100) */
	int broker_fee_percent_num = 25;
	int broker_fee_percent_den = 1;
	int artifact_cost_xp_num = 100000000;
	int artifact_cost_xp_den = 1;
};

struct EditListinoCost {
	long long xp = 0;       /* addebito PG (xp gia' ripartito se la regola e' accesa) */
	int rune = 0;
	long long gold = 0;
	long long xp_total = 0; /* xp prima della ripartizione tra classi (anteprima) */
};

extern const EditListinoDefault kEditListinoDefaults[];
extern const std::size_t kEditListinoDefaultsCount;
extern const EditListinoRuleDefault kEditListinoRulesDefaults[];
extern const std::size_t kEditListinoRulesDefaultsCount;
extern const EditListinoCapGroupDefault kEditListinoCapGroupDefaults[];
extern const std::size_t kEditListinoCapGroupDefaultsCount;

void edit_listino_boot();
void edit_listino_reload();

[[nodiscard]] const EditListinoRules& edit_listino_rules_cache();
/* num/den di broker_fee_percent; fallback 25/1 se manca o den<=0. */
[[nodiscard]] std::pair<int, int> edit_listino_broker_fee_frac() noexcept;
[[nodiscard]] double edit_listino_class_mult(int n_classes) noexcept;
[[nodiscard]] std::pair<int, int> edit_listino_class_mult_frac(int n_classes) noexcept;

[[nodiscard]] const EditListinoEntry* edit_listino_find(const char* code);
[[nodiscard]] const EditListinoEntry* edit_listino_find_apply(int apply_loc,
															  unsigned long apply_bit);
[[nodiscard]] std::vector<const EditListinoEntry*>
edit_listino_rows_for_apply(int apply_loc);

/** Tutte le righe in cache (seed + DB, anche spente o non supportate dal MUD).
 *  Sola lettura, per l'API del portale edit (listino-status, catalogo). */
[[nodiscard]] const std::vector<EditListinoEntry>& edit_listino_all_rows();

/** true se code e' un campo editpool (on_pc=1). */
[[nodiscard]] bool edit_listino_is_pc_code(const char* code);

/** Cap personaggio (cap_pc) per un codice, o 0 se assente. */
[[nodiscard]] int edit_listino_cap_pc(const char* code) noexcept;
[[nodiscard]] int edit_listino_over_cap(const char* code) noexcept;

/**
 * Costo listino normale (non over) per `units` unita':
 * artifact * class_mult; xp ripartito per n_classes se la regola e' accesa;
 * rune arrotondate per eccesso.
 */
[[nodiscard]] EditListinoCost edit_listino_normal_cost(const char* code, int units,
													   int n_classes);

/**
 * Costo overedit a slot per passare da `from_units` a `to_units` (valori assoluti,
 * to > from per acquisto). Per lo slot n:
 *   base_n = over_xp + (n-1)*over_xp*over_inc_pct/100  (oro uguale)
 *   xp_totale = base_n * artifact * class_mult
 *   oro       = base_n * artifact * class_mult
 *   rune      = ceil(over_rune * artifact) a slot (NO class_mult)
 * Moltiplicatori da edit_listino_rules (fallback all'array default).
 * Il campo xp e' l'addebito PG: xp_totale ripartito tra le classi se la
 * regola xp_split_by_classes e' accesa. Oro e rune non si ripartiscono.
 */
[[nodiscard]] EditListinoCost edit_listino_over_cost(const char* code, int from_units,
													 int to_units, int n_classes);

/** Valore "raw" AffectSlotValue (xp / 10000) per un APPLY numerico o un bit.
 *  Se step>1 il raw e' il prezzo a slot; il chiamante divide per lo step. */
[[nodiscard]] long edit_listino_raw_unit_value(const EditListinoEntry& e) noexcept;

/** Step listino normale (>=1). */
[[nodiscard]] int edit_listino_entry_step(const EditListinoEntry& e) noexcept;

/**
 * Avvisi step/cap per affect gia' sull'oggetto (stringa vuota se ok).
 * Non cambia i prezzi: i valori non multipli restano valutati.
 */
[[nodiscard]] std::string edit_listino_obj_step_flags(const struct obj_data* obj);

/**
 * true se il nuovo modifier sullo slot `slot` (0-based) e' ammesso da step/cap.
 * `err` in italiano in caso di rifiuto. slot fuori range o obj nullo -> false.
 */
[[nodiscard]] bool edit_listino_accept_obj_mod(const struct obj_data* obj, int slot,
											   int new_mod, std::string& err);

/** Handler wiz: configure editpool [set|reset|help]. Solo righe on_pc=1 + regole. */
void edit_listino_wiz(struct char_data* ch, const char* arg);

} // namespace Alarmud

#endif /* __EDIT_LISTINO_HPP */
