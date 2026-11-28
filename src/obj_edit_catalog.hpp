/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_OBJ_EDIT_CATALOG_HPP_
#define SRC_OBJ_EDIT_CATALOG_HPP_

#include <string>

#include "../contrib/slacking/json.hpp"

namespace Alarmud {

using Json = nlohmann::json;

/** Vnum prototipo tan (TAN_* da skill tan). */
[[nodiscard]] bool object_vnum_is_tan_proto(int vnum) noexcept;

[[nodiscard]] bool object_is_tanned(const struct obj_data* obj) noexcept;

/** Slug e_item_type senza ITEM_ (es. armor, weapon). nullptr se ITEM_NONE o non mappato. */
[[nodiscard]] const char* object_portal_item_type_slug(int item_type) noexcept;

/** Motivo se non editabile (stringa vuota se editabile). */
[[nodiscard]] std::string object_portal_skip_reason(const struct obj_data* obj,
												  const char* toon_name);

/** wearpos MySQL: >0 = indossato. */
bool inventory_row_is_worn(int wearpos) noexcept;

/**
 * true se l'oggetto può essere editato anche indossato.
 * Path portale = MySQL offline: primo edit e ri-edit entrambi OK su wear_pos>0.
 */
[[nodiscard]] bool object_portal_allows_worn_edit(const struct obj_data* obj) noexcept;

/** Oggetto editabile nel portale (esclusioni listino + categorie staff). */
[[nodiscard]] bool object_portal_editable(const struct obj_data* obj,
										  const char* toon_name) noexcept;

/** false = non mostrare in inventario portale.
 * TAN/HAS-GEMS/simbolo: mai. RARO: nascosto salvo pezzi in DB edits (show db)
 * o ITEM2_EDIT + EDNomeToon. Categorie: EDIT/instance bypassano solo le categorie. */
[[nodiscard]] bool object_portal_show_in_inventory_list(const struct obj_data* obj,
														const char* toon_name) noexcept;

/** Slot affect occupati / liberi (MAX_OBJ_AFFECT). */
[[nodiscard]] Json object_affect_slots_json(const struct obj_data* obj);

/** Catalogo edit oggetto (listino obj_value CheckValueObj; pool PG esclusi sull'eq). */
[[nodiscard]] Json object_edit_catalog_json(const struct obj_data* obj);

/**
 * true se l'edit migliora un malus listino (es. INT -3→0, AC +10→0).
 * In quel caso il portale addebita la tariffa 2× e deve settare ITEM2_PAID_MALUS.
 */
[[nodiscard]] bool object_edit_recovers_listino_malus(const struct obj_data* obj,
													 int location,
													 int target_modifier,
													 bool clear_slot) noexcept;

/**
 * true se da malus si passa oltre lo zero fino a un bonus
 * (es. AC +10→−10, INT −3→+2): quote/apply vanno spezzati in
 * malus→0 (2×) + 0→bonus (1×).
 */
[[nodiscard]] bool object_edit_malus_crosses_to_bonus(int location, int current,
													 int target) noexcept;

/** Dettaglio quote spezzata malus→0 + 0→bonus (xp_raw gia' con class/artifact). */
struct ObjEditMalusSplit {
	bool active = false;
	long malus_xp_raw = 0;
	long bonus_xp_raw = 0;
	int from = 0;
	int to = 0;
};

/**
 * Simula affect target → delta costo listino (AnalyzeObjEdit.diff prima/dopo).
 * class_mult e Artifact +50% sono applicati dentro AnalyzeObjEdit (una sola volta).
 * Recupero malus a 0: 2× EditAffectDeltaListinoCost.
 * Malus che attraversa lo zero fino a bonus: malus→0 (2×) + 0→target (1×);
 * se split_out != nullptr viene riempito.
 * Se other_worn_edited_dam >= 0 e l'edit tocca il dam → tetto 30
 * (somma delta vs proto su pezzi ITEM2_EDIT *indossati*, escluso questo).
 * allow_combat_merge: se true e hr==dr (o hr==sp), accorpa in HITNDAM/HITNSP;
 * altrimenti lascia gli slot combat separati (scelta giocatore via UI).
 */
[[nodiscard]] bool object_quote_affect_target(struct obj_data* obj, int location,
											  int target_modifier, long& xp_raw,
											  int& pq, std::string& err,
											  int other_worn_edited_dam = -1,
											  int other_worn_edited_sp = -1,
											  bool clear_slot = false,
											  int other_owned_edited_spellfail = -1,
											  bool allow_combat_merge = false,
											  ObjEditMalusSplit* split_out = nullptr);

/** Imposta affect target (stessa logica di quote). Restituisce false se invalido.
 *  clear_slot: libera lo slot (bonus positivo gratis).
 *  Se recupera un malus listino: setta ITEM2_PAID_MALUS sul pezzo.
 *  allow_combat_merge: vedi object_quote_affect_target. */
[[nodiscard]] bool object_apply_affect_target(struct obj_data* obj, int location,
											  int target_modifier, std::string& err,
											  int other_worn_edited_dam = -1,
											  int other_worn_edited_sp = -1,
											  bool clear_slot = false,
											  int other_owned_edited_spellfail = -1,
											  bool allow_combat_merge = false);

[[nodiscard]] int object_affect_current_modifier(const struct obj_data* obj,
												 int location) noexcept;

/** Totale effettivo sul pezzo (include HIT-N-DAM / HIT-N-SP). */
[[nodiscard]] int object_edit_display_current(const struct obj_data* obj,
											  int location) noexcept;

/** Dam effettivo sul pezzo (DAMROLL + HITNDAM). */
[[nodiscard]] int object_edit_damroll_total(const struct obj_data* obj) noexcept;

/** Spellpower effettivo sul pezzo (SPELLPOWER + HITNSP). */
[[nodiscard]] int object_edit_spellpower_total(const struct obj_data* obj) noexcept;

/**
 * Dam/SP *editato* = max(0, totale_pezzo − baseline).
 * Baseline = create-baseline (procarea / PAID_MALUS), altrimenti proto mondo.
 */
[[nodiscard]] int object_edit_damroll_edited_delta(const struct obj_data* obj) noexcept;
[[nodiscard]] int object_edit_spellpower_edited_delta(const struct obj_data* obj) noexcept;
[[nodiscard]] int object_edit_hitroll_edited_delta(const struct obj_data* obj) noexcept;

/**
 * Breakdown delta combat pezzo vs baseline: full / broker / wiz.
 * I tetti pezzo usano solo wiz (HR<=2, dam+sp<=2); il personaggio DAM+SP
 * usa i delta full (come object_edit_*_edited_delta).
 */
struct ObjEditCombatWizCaps {
	int hr_full = 0;
	int dam_full = 0;
	int sp_full = 0;
	int hr_broker = 0;
	int dam_broker = 0;
	int sp_broker = 0;

	[[nodiscard]] int hr_wiz() const noexcept {
		return hr_full > hr_broker ? hr_full - hr_broker : 0;
	}
	[[nodiscard]] int dam_wiz() const noexcept {
		return dam_full > dam_broker ? dam_full - dam_broker : 0;
	}
	[[nodiscard]] int sp_wiz() const noexcept {
		return sp_full > sp_broker ? sp_full - sp_broker : 0;
	}
};

[[nodiscard]] ObjEditCombatWizCaps
object_edit_combat_wiz_caps(const struct obj_data* obj) noexcept;

/**
 * Spellfail *editato* = max(0, proto − current) (piu' negativo = piu' edit).
 * Conta verso il tetto personaggio kObjEditMaxSpellfailEditableTotal.
 */
[[nodiscard]] int object_edit_spellfail_edited_delta(const struct obj_data* obj) noexcept;
[[nodiscard]] int object_edit_spellfail_total(const struct obj_data* obj) noexcept;
[[nodiscard]] int object_edit_spellfail_prototype_total(const struct obj_data* obj) noexcept;

/** Hitroll effettivo sul pezzo (HITROLL + HITNDAM + HITNSP). */
[[nodiscard]] int object_edit_hitroll_total(const struct obj_data* obj) noexcept;

/** Dam sul prototipo risolto (0 se proto assente). */
[[nodiscard]] int object_edit_damroll_prototype_total(const struct obj_data* obj) noexcept;

/** Vnum prototipo usato per il delta edit (0 se sconosciuto). */
[[nodiscard]] int object_edit_prototype_vnum(const struct obj_data* obj) noexcept;

/**
 * true se il pezzo entra nel tetto dam/sp del toon:
 * ITEM2_EDIT + owner (personal_owner o ED*) del toon; esclusi simboli di clan.
 * Conta sia indossati che in inventario.
 */
[[nodiscard]] bool object_edit_counts_toward_combat_budget(const struct obj_data* obj,
														  const char* toon_name) noexcept;

/** true se location può cambiare il dam effettivo del pezzo. */
[[nodiscard]] bool object_edit_location_affects_dam(int location) noexcept;

/** true se location può cambiare lo spellpower effettivo del pezzo. */
[[nodiscard]] bool object_edit_location_affects_spellpower(int location) noexcept;

/** true se location tocca APPLY_SPELLFAIL. */
[[nodiscard]] bool object_edit_location_affects_spellfail(int location) noexcept;

[[nodiscard]] int object_immune_current_bits(const struct obj_data* obj) noexcept;

/** Bit APPLY_M_IMMUNE (immunità concesse listino: drain/charm/poison). */
[[nodiscard]] int object_m_immune_current_bits(const struct obj_data* obj) noexcept;

[[nodiscard]] int object_susc_current_bits(const struct obj_data* obj) noexcept;

[[nodiscard]] int object_spell_current_bits(const struct obj_data* obj) noexcept;

[[nodiscard]] int object_aff2_current_bits(const struct obj_data* obj) noexcept;

/** Ottimizza slot combat (hit-n-dam, hit-n-sp). */
void object_compact_edit_affects(struct obj_data* obj) noexcept;

} // namespace Alarmud

#endif /* SRC_OBJ_EDIT_CATALOG_HPP_ */
