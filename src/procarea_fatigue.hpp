/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_FATIGUE_HPP
#define __PROCAREA_FATIGUE_HPP
#include "typedefs.hpp"
#include <string>
namespace Alarmud {

namespace procarea_internal {
struct ProcAreaInstance;
}

/** Run piene (boss ucciso) al giorno prima del decay: 3 solo + 3 gruppo, contatori separati. */
constexpr int PROCAREA_FATIGUE_FULL_CLEARS = 3;
constexpr int PROCAREA_FATIGUE_TIER_COUNT = 6;
/** Gruppo: tier tesoro da 80% media clear + 20% picco clear (come power index). */
constexpr float PROCAREA_GROUP_FATIGUE_AVG_WEIGHT = 0.80f;
constexpr float PROCAREA_GROUP_FATIGUE_MAX_WEIGHT = 0.20f;

[[nodiscard]] int procarea_fatigue_tier_from_clears(int clears_before);
[[nodiscard]] int procarea_fatigue_tier_from_effective_clears(float effective_clears);
[[nodiscard]] int procarea_fatigue_tier_for_name(const std::string& name, bool solo_mode);
[[nodiscard]] int procarea_fatigue_solo_clears_for_name(const char* name);
[[nodiscard]] int procarea_fatigue_group_clears_for_name(const char* name);
[[nodiscard]] int procarea_fatigue_gear_drop_pct(int hoard_index, int fatigue_tier);
[[nodiscard]] int procarea_fatigue_gold_drop_pct(int fatigue_tier);
[[nodiscard]] bool procarea_fatigue_roll_gold(int fatigue_tier);
[[nodiscard]] float procarea_fatigue_group_effective_clears_for_instance(
	const procarea_internal::ProcAreaInstance& inst);

/** Tier premi tesoro per questa run (prima dell'incremento contatori). */
[[nodiscard]] int procarea_fatigue_treasure_tier_for_instance(
	const procarea_internal::ProcAreaInstance& inst);

/** Incrementa contatori giornalieri per owner e member_names. */
void procarea_fatigue_on_boss_killed(procarea_internal::ProcAreaInstance& inst, int treasure_tier);

/** Clear lifetime (boss ucciso) per personaggio. */
[[nodiscard]] int procarea_clears_solo_total_get(const char_data* ch);
[[nodiscard]] int procarea_clears_group_total_get(const char_data* ch);
[[nodiscard]] int procarea_clears_total_get(const char_data* ch);
void procarea_clears_sync_achievements(char_data* ch);

/** YYYYMMDD locale per reset giornaliero fatigue. */
[[nodiscard]] int procarea_fatigue_day_id();

} // namespace Alarmud
#endif // __PROCAREA_FATIGUE_HPP
