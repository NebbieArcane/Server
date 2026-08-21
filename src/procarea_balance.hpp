/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_BALANCE_HPP
#define __PROCAREA_BALANCE_HPP
#include "procarea.hpp"
#include "procarea_fatigue.hpp"
#include "typedefs.hpp"
namespace Alarmud {

/** Densita' layout/spawn Dimensione Effimera (runtime, WIZ + DB). Default = profilo MID. */
struct ProcDensityConfig {
	float bias = 1.0f;

	int rooms_min_lo = 12;
	int rooms_min_hi = 55;
	int rooms_max_lo = 18;
	int rooms_max_hi = 70;

	int corr_lo = 45;
	int corr_hi = 62;
	int tes_lo = 60;
	int tes_hi = 75;
	int adds_lo = 0;
	int adds_hi = 2;
	int depth_lo = 3;
	int depth_hi = 8;
	int br_lo = 2;
	int br_hi = 5;
	int bc_lo = 25;
	int bc_hi = 42;

	int sec_corr = 12;
	int sec_tre = 22;
	int sec_trap = 25;

	int solo_rooms_pct = 45;
	int solo_rooms_min_floor = 10;
	int solo_rooms_max_floor = 15;
	int solo_branches_delta = 2;
	int solo_branch_chance_delta = 12;
	int solo_branch_chance_floor = 15;
	int solo_corr_pct = 65;
	int solo_corr_floor = 35;
	int solo_tes_delta = 8;
	int solo_tes_floor = 50;
	int solo_adds_delta = 1;
	int solo_depth_delta = 3;
	int solo_depth_floor = 2;
};

struct ProcCrystalBalance {
	float mob_mult = 1.0f;
	float frag_mult = 1.0f;
	int frag_drop_corridor_pct = 15;
	int frag_drop_treasure_pct = 35;
};

/** Premi numerici (fatigue %, frammenti, cristalli, conversione). */
struct ProcRewardsConfig {
	int fatigue_gear_base[PROCAREA_FATIGUE_TIER_COUNT] = { 100, 75, 60, 45, 30, 10 };
	int fatigue_gear_decay[PROCAREA_FATIGUE_TIER_COUNT] = { 25, 19, 15, 11, 8, 10 };
	int fatigue_gold_pct[PROCAREA_FATIGUE_TIER_COUNT] = { 100, 86, 72, 58, 44, 30 };

	/* Corridor, Treasure, Add, Trap, Boss */
	int frag_drop_pct[5] = { 15, 35, 25, 100, 100 };
	int frag_qty_mult[5] = { 100, 130, 110, 250, 400 };
	float frag_qty_bias = 1.0f;
	int fragments_per_rune = 1000;

	ProcCrystalBalance crystal[PROCAREA_CRYSTAL_COUNT] = {
		{ 0.75f, 0.50f, 9, 21 },
		{ 0.88f, 0.75f, 12, 28 },
		{ 1.00f, 1.00f, 15, 35 },
		{ 1.20f, 1.38f, 18, 41 },
		{ 1.30f, 1.50f, 19, 44 },
	};
};

void procarea_balance_boot();
void procarea_balance_save();
void procarea_balance_reset_density();
void procarea_balance_reset_rewards();

[[nodiscard]] const ProcDensityConfig& procarea_density_config();
[[nodiscard]] const ProcRewardsConfig& procarea_rewards_config();
[[nodiscard]] ProcDensityConfig& procarea_density_config_mut();
[[nodiscard]] ProcRewardsConfig& procarea_rewards_config_mut();

[[nodiscard]] int procarea_fragments_per_rune();

/** true se ha gestito densita/premi (anche errori di sintassi). */
bool procarea_try_balance_wiz_command(char_data* ch, const char* subcmd, const char* rest);

} // namespace Alarmud
#endif // __PROCAREA_BALANCE_HPP
