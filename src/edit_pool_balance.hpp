/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __EDIT_POOL_BALANCE_HPP
#define __EDIT_POOL_BALANCE_HPP
#include "edit_pool.hpp"

namespace Alarmud {

/**
 * Cap + prezzi per unita' del listino editpool (6 campi).
 * Persistenza: tabella edit_pool_balance (KV), CREATE al boot.
 *
 *   configure editpool
 *   configure editpool set <chiave> <val>
 *   configure editpool reset
 *
 * Cap: max_hp max_mana max_move max_hp_regen max_mana_regen max_move_regen
 * Prezzo: <campo>_xp, <campo>_rune_num, <campo>_rune_den
 *   campo = hp|mana|move|hp_regen|mana_regen|move_regen
 * rune = units * num / den (es. mana 3/2).
 */
struct EditPoolUnitPrice {
	long xp = 0;
	int rune_num = 1;
	int rune_den = 1;
};

struct EditPoolBalanceConfig {
	int max_hp = kEditPoolMaxHit;
	int max_mana = kEditPoolMaxMana;
	int max_move = kEditPoolMaxMove;
	int max_hp_regen = kEditPoolMaxHitRegen;
	int max_mana_regen = kEditPoolMaxManaRegen;
	int max_move_regen = kEditPoolMaxMoveRegen;

	/* Default = listino ufficiale (come editpool_listino_unit storico). */
	EditPoolUnitPrice hp{3000000L, 3, 1};
	EditPoolUnitPrice mana{1500000L, 3, 2};
	EditPoolUnitPrice move{1000000L, 3, 2};
	EditPoolUnitPrice hp_regen{3000000L, 3, 1};
	EditPoolUnitPrice mana_regen{3000000L, 3, 1};
	EditPoolUnitPrice move_regen{2000000L, 6, 1};
};

void edit_pool_balance_boot();
void edit_pool_balance_save();
void edit_pool_balance_reset();

[[nodiscard]] const EditPoolBalanceConfig& edit_pool_balance_config();

/** Prezzo per unita' dal balance (stesso contratto di editpool_listino_unit). */
[[nodiscard]] bool edit_pool_balance_listino_unit(EditPoolField field, long& xp_per_unit,
												  int& rune_num, int& rune_den) noexcept;

/** Handler wiz (arg = resto dopo "configure editpool"). */
void edit_pool_balance_wiz(struct char_data* ch, const char* arg);

} // namespace Alarmud

#endif
