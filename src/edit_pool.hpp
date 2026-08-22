/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_EDIT_POOL_HPP_
#define SRC_EDIT_POOL_HPP_

#include "structs.hpp"

namespace Alarmud {

/** Massimali listino edits (per personaggio), unità raw come APPLY sull'eq. */
inline constexpr int kEditPoolMaxHit = 100;
inline constexpr int kEditPoolMaxMana = 150;
inline constexpr int kEditPoolMaxMove = 100;
inline constexpr int kEditPoolMaxHitRegen = 50;
inline constexpr int kEditPoolMaxManaRegen = 50;
inline constexpr int kEditPoolMaxMoveRegen = 50;

[[nodiscard]] bool edit_pool_is_pool_apply(int location) noexcept;

/** true se location e' uno dei 6 APPLY migrati sul PG. */
[[nodiscard]] bool edit_pool_location_blocked_on_eq(int location) noexcept;

/**
 * Delta vs prototipo (solo APPLY pool). Proto nullptr → tutto conta come edit.
 */
void edit_pool_accumulate_obj_delta(const struct obj_data* obj,
									const struct obj_data* proto,
									struct char_edit_pool_data* add);

/**
 * Porta gli APPLY pool dell'oggetto ai valori del prototipo: toglie l'extra
 * editato (gia' accreditato a parte) e lascia HIT/MANA/MOVE/regen di base.
 * Proto nullptr → azzera solo i pool apply (legacy).
 */
bool edit_pool_strip_obj(struct obj_data* obj, const struct obj_data* proto);

/**
 * Heal una tantum: istanze gia' strippate a APPLY_NONE ripristinano i pool
 * apply del base_vnum. Idempotente (event edit_pool_proto_restore).
 */
void edit_pool_heal_proto_pool_affects();

/**
 * Applica somma grezza ai campi edit/overedit rispettando i cap listino.
 * Non tocca `migrated`.
 */
void edit_pool_credit_raw(struct char_edit_pool_data* pool, int hit, int mana,
						  int move, int hit_regen, int mana_regen, int move_regen);

/**
 * Migrazione automatica EQ→PG per un personaggio online (dopo load inventorio).
 * Solo oggetti in range edit (34k) o con db_instance_id, proprietario (pers_on),
 * esclusi simboli del clan (ITEM_CLAN_SYMBOL / vnum in lista). Idempotente se edit_pool.migrated != 0.
 */
void edit_pool_migrate_char(struct char_data* ch);

/**
 * Boot: per ogni object_instance attiva, somma delta pool all'owner, strip affect,
 * aggiorna character_stats (solo se edit_pool_migrated=0). Poi marca migrated.
 */
void edit_pool_boot_migrate();

} // namespace Alarmud

#endif /* SRC_EDIT_POOL_HPP_ */
