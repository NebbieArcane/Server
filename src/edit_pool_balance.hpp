/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __EDIT_POOL_BALANCE_HPP
#define __EDIT_POOL_BALANCE_HPP
#include "edit_pool.hpp"

namespace Alarmud {

/**
 * Cap + prezzi per unita' del listino editpool (6 campi + over).
 * Persistenza: tabella edit_pool_balance (KV), heal_v15 + CREATE al boot.
 *
 *   configure editpool
 *   configure editpool set <chiave> <val>
 *   configure editpool reset
 *
 * Cap: max_hp … max_move_regen
 * Prezzo: <campo>_xp, <campo>_rune_num, <campo>_rune_den
 * Over: max_over_* e over_<campo>_{xp,rune_num,rune_den}
 *   −1 = non configurato (nessun cap/prezzo dedicato all'over).
 */
struct EditPoolUnitPrice {
	long xp = 0;
	int rune_num = 1;
	int rune_den = 1;
};

/** Prezzo over: default −1 = non configurato. */
inline constexpr EditPoolUnitPrice kEditPoolOverPriceUnset{-1L, -1, -1};

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

	/* Over: −1 = non configurato. */
	int max_over_hp = -1;
	int max_over_mana = -1;
	int max_over_move = -1;
	int max_over_hp_regen = -1;
	int max_over_mana_regen = -1;
	int max_over_move_regen = -1;
	EditPoolUnitPrice over_hp = kEditPoolOverPriceUnset;
	EditPoolUnitPrice over_mana = kEditPoolOverPriceUnset;
	EditPoolUnitPrice over_move = kEditPoolOverPriceUnset;
	EditPoolUnitPrice over_hp_regen = kEditPoolOverPriceUnset;
	EditPoolUnitPrice over_mana_regen = kEditPoolOverPriceUnset;
	EditPoolUnitPrice over_move_regen = kEditPoolOverPriceUnset;
};

void edit_pool_balance_boot();
void edit_pool_balance_save();
void edit_pool_balance_reset();

[[nodiscard]] const EditPoolBalanceConfig& edit_pool_balance_config();

/** Prezzo per unita' dal balance (stesso contratto di editpool_listino_unit). */
[[nodiscard]] bool edit_pool_balance_listino_unit(EditPoolField field, long& xp_per_unit,
												  int& rune_num, int& rune_den) noexcept;

/**
 * Prezzo over se configurato (xp != -1); altrimenti false (usa listino normale).
 */
[[nodiscard]] bool edit_pool_balance_over_listino_unit(EditPoolField field, long& xp_per_unit,
													   int& rune_num, int& rune_den) noexcept;

/** Cap over se >= 0; altrimenti -1 = non configurato. */
[[nodiscard]] int edit_pool_balance_over_cap(EditPoolField field) noexcept;

/** Handler wiz (arg = resto dopo "configure editpool"). */
void edit_pool_balance_wiz(struct char_data* ch, const char* arg);

} // namespace Alarmud

#endif
