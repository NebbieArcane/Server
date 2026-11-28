/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_EDIT_PORTAL_LISTINO_HPP_
#define SRC_EDIT_PORTAL_LISTINO_HPP_

/*
 * Ponte tra l'API del portale edit (edit_portal.cpp, obj_edit_catalog.cpp) e il
 * listino unico del Server (edit_listino.cpp + edit_pool). Nessuna cache propria:
 * ogni lettura passa da edit_listino_find / edit_listino_find_apply /
 * edit_listino_rules_cache, quindi vale la stessa regola del MUD
 * (riga valida solo con mud_supported && enabled && on_obj / on_pc).
 * Il ricarico e' solo esplicito: boot, 'configure editpool reload' o
 * /internal/listino-reload (Sync / salvataggio staff del portale).
 * Da usare solo dal thread di gioco (edit_listino non e' thread-safe).
 */

#include <string>
#include <vector>

#include "edit_pool.hpp"

namespace Alarmud {

/** Massimo listino edits per pezzo (STR/DEX/WIS/INT/CHR). */
inline constexpr int kObjEditMaxStatPerPiece = 3;
/** Hitroll delta (wiz) per pezzo; i trasferimenti broker non consumano questo tetto. */
inline constexpr int kObjEditMaxHitrollPerPiece = 2;
/**
 * Damroll/spellpower: sul pezzo la somma dei delta wiz (DAM+HITNDAM)+(SP+HITNSP)
 * e' al massimo 2. I trasferimenti broker sono un budget separato e non contano.
 */
inline constexpr int kObjEditMaxDamrollPerPiece = 2;
inline constexpr int kObjEditMaxSpellpowerPerPiece = 2;
inline constexpr int kObjEditMaxDamSpPerPiece = 2;
/** Tetto personaggio DAM+SP combinato (come EditAffectBroker / show edits). */
inline constexpr int kObjEditMaxDamSpEditableTotal = 30;
/** Hitroll editato totale (2/pezzo x 21 slot, escluso simbolo clan). */
inline constexpr int kObjEditMaxHitrollEditableTotal = 42;
/** Spellfail: step -5, per pezzo fino a -100 oltre proto; tetto personaggio 100. */
inline constexpr int kObjEditSpellfailStep = -5;
inline constexpr int kObjEditSpellfailMinTotal = -100;
inline constexpr int kObjEditMaxSpellfailEditableTotal = 100;
/** Listino portale: 1 MXP = 1 runa (spell su bit e spell delle armi). */
inline constexpr long kObjEditRunePerMegaXp = 1000000L;
/** Conversione MXP -> rune nel portale (1 MXP listino = 1e6 XP raw). */
inline constexpr long kEditPoolPqPerMegaXp = 1000000L;
/** Max lunghezza campi testo oggetto (VARCHAR inventorio/istanza). */
inline constexpr int kObjEditTextNameMax = 128;
inline constexpr int kObjEditTextShortMax = 128;
inline constexpr int kObjEditTextLongMax = 256;

/** Parametri listino per un APPLY editabile sull'oggetto (totale sul pezzo). */
struct ObjEditListinoSpec {
	int location = 0;
	const char* id = "";
	const char* label = "";
	int step = 1;
	int min_total = 0;
	int max_total = 0;
	/** Unita' raw CheckValueObj per +1 (o magnitudine -1 su AC). */
	long positive_unit_raw = 0;
	long negative_unit_raw = 0;
};

/** Voce oggetto numerica (APPLY senza bit) valida per il MUD. */
[[nodiscard]] bool obj_edit_listino_spec(int location, ObjEditListinoSpec& out) noexcept;
[[nodiscard]] int obj_edit_listino_scalar_count() noexcept;
[[nodiscard]] bool obj_edit_listino_scalar_at(int index, ObjEditListinoSpec& out) noexcept;

/** true se edit_listino ha righe in RAM (seed o DB). */
[[nodiscard]] bool portal_listino_ready() noexcept;
[[nodiscard]] int portal_listino_affect_count() noexcept;
[[nodiscard]] bool portal_listino_affect_at(int index, ObjEditListinoSpec& out) noexcept;
[[nodiscard]] bool portal_listino_affect_by_location(int location,
													 ObjEditListinoSpec& out) noexcept;
/** Rune per UNA unita' (rune_num / (rune_den * step)), senza moltiplicatori. */
[[nodiscard]] bool portal_listino_affect_rune(int location, int& rune_num,
											  int& rune_den) noexcept;
/** Voce a bit (APPLY_IMMUNE / APPLY_M_IMMUNE / APPLY_SUSC) valida per il MUD. */
[[nodiscard]] bool portal_listino_affect_by_bit(int location, unsigned long apply_bit,
												ObjEditListinoSpec& out) noexcept;
[[nodiscard]] bool portal_listino_affect_rune_bit(int location, unsigned long apply_bit,
												  int& rune_num, int& rune_den) noexcept;
[[nodiscard]] std::string portal_listino_affect_cap_group(int location);
/**
 * Parametri storici del portale letti da edit_listino_rules:
 * storage_scale, broker_fee_percent, artifact_mult, artifact_cost_xp,
 * artifact_cost_raw, class_mult_1/bi/tri. Chiave sconosciuta -> fallback.
 */
[[nodiscard]] double portal_listino_param(const char* key, double fallback) noexcept;
/** Revisione: "edit_listino@<unix max(updated_at)>#<firma tabelle>" (calcolata al reload). */
[[nodiscard]] std::string portal_listino_seed_tag();
/** Ricalcola solo la revisione (SELECT), senza ricaricare il listino. */
void portal_listino_refresh_revision();
/** edit_listino_reload() + revisione. Solo dal thread di gioco. */
bool portal_listino_reload();

struct PortalListinoRowStatus {
	std::string code;
	std::string kind; /* affect | pool | flag | bit | spell | other */
	bool enabled = false;
	bool supported = false; /* mud_supported */
	bool active = false;    /* mud_supported && enabled && on_obj/on_pc */
	std::string reason;
};
[[nodiscard]] std::vector<PortalListinoRowStatus> portal_listino_row_status();

/* ---- Pool PG (editpool) ---- */

struct portal_pool_quote {
	long xp_raw = 0;
	int pq = 0;
	long mxp = 0;
	long mxp_frac = 0;
};

/** Chiavi portale: hit, mana, move, hit_regen, mana_regen, move_regen.
 *  Spellfail sul PG non passa dal portale (resta su editpool nel MUD). */
[[nodiscard]] bool portal_pool_parse_field_key(const char* key, EditPoolField& out) noexcept;
/** Step di acquisto (buy_step del listino; default 10 hit/mana/move, 5 regen). */
[[nodiscard]] int portal_pool_field_step(EditPoolField field) noexcept;
/** Costo come editpool (editpool_cost_for_units_n): artifact x class_mult, xp / classi. */
[[nodiscard]] portal_pool_quote portal_pool_quote_paid(EditPoolField field, int units,
													   int n_classes);
[[nodiscard]] double portal_pool_class_mult(int n_classes) noexcept;
/** Imposta edit_* (0..cap) su una copia del pool letta da character_stats. */
bool portal_pool_set_absolute(struct char_edit_pool_data* pool, EditPoolField field,
							  int value) noexcept;

} // namespace Alarmud

#endif /* SRC_EDIT_PORTAL_LISTINO_HPP_ */
