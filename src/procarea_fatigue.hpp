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

[[nodiscard]] int procarea_fatigue_tier_from_clears(int clears_before);
[[nodiscard]] int procarea_fatigue_tier_for_name(const std::string& name, bool solo_mode);
[[nodiscard]] int procarea_fatigue_gear_drop_pct(int hoard_index, int fatigue_tier);
[[nodiscard]] bool procarea_fatigue_roll_gold(int fatigue_tier);

/** Tier premi tesoro per questa run (prima dell'incremento contatori). */
[[nodiscard]] int procarea_fatigue_treasure_tier_for_instance(
	const procarea_internal::ProcAreaInstance& inst);

/** Incrementa contatori giornalieri per owner e member_names; notifica se tier > 0. */
void procarea_fatigue_on_boss_killed(procarea_internal::ProcAreaInstance& inst, int treasure_tier);

/** YYYYMMDD locale per reset giornaliero fatigue. */
[[nodiscard]] int procarea_fatigue_day_id();

} // namespace Alarmud
#endif // __PROCAREA_FATIGUE_HPP
