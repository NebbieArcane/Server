/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __DISPEL_BALANCE_HPP
#define __DISPEL_BALANCE_HPP
#include "typedefs.hpp"

namespace Alarmud {

constexpr int DISPEL_MONO_BAND_MAX = 4;

/**
 * Seconda chance dispel + percorso mono a fasce SP.
 * Persistenza: tabella spell_balance (KV), schema ODB 15 / heal_v15 + CREATE al boot.
 *
 * Mono (IS_CASTER_N + monclasse, se mono_band):
 *   tiro fasce una volta per cast; se fallisce → solo seconda chance (niente 1d20).
 *   T[k] = mono_t[k-1] → 100% auto vs caster+k; a T[k]-1 → mono_soft_pct%;
 *   sotto, rampa 0..soft_pct nella fascia.
 *
 * Seconda chance (uguale mono e cl/mu, default soft):
 *   over = SP - sp_baseline
 *   diff = vict - caster   (niente bonus fasce)
 *   div  = soft ? max(1, diff/2+1) : max(1, diff)
 *   pct  = min(pct_cap, over/div); se vict >= high_level: * high_level_pct/100
 */
struct DispelBalanceConfig {
	bool soft = true;
	int sp_baseline = 10;
	int pct_cap = 50;
	int level_bonus_cap = 5; /* legacy /10 se mono_band off */
	int high_level = 56;
	int high_level_pct = 50;

	bool mono_band = true;
	int mono_soft_pct = 85;
	int mono_max_bonus = DISPEL_MONO_BAND_MAX;
	/* SP per 100% auto su +1 .. +mono_max_bonus livelli */
	int mono_t[DISPEL_MONO_BAND_MAX] = {30, 61, 92, 122};
};

void dispel_balance_boot();
void dispel_balance_save();
void dispel_balance_reset();

[[nodiscard]] const DispelBalanceConfig& dispel_balance_config();
[[nodiscard]] DispelBalanceConfig& dispel_balance_config_mut();

/** Mono eleggibile e mono_band attivo. */
[[nodiscard]] bool DispelMonoBandPath(struct char_data* ch);
/** % auto fasce (0..100) per questa vittima; 100 se vittima <= caster. */
[[nodiscard]] int DispelMonoBandAutoChance(struct char_data* ch, struct char_data* victim);
/** Un tiro fasce: true = auto-dispel per tutto il cast. */
[[nodiscard]] bool DispelMonoBandAutoRoll(struct char_data* ch, struct char_data* victim);
/** Quante fasce piene ha sbloccato (per diff seconda chance). */
[[nodiscard]] int DispelMonoBandUnlocked(struct char_data* ch);

ACTION_FUNC(do_dispelconf);

} // namespace Alarmud

#endif
