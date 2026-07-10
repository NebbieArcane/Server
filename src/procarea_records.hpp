/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_RECORDS_HPP
#define __PROCAREA_RECORDS_HPP
#include "typedefs.hpp"
namespace Alarmud {

namespace procarea_internal {
struct ProcAreaInstance;
}

/** YYYYMMDD locale (come fatigue). */
[[nodiscard]] int procarea_records_day_id();
/** Anno+settimana locale (YYYYWW, settimana da lunedi). */
[[nodiscard]] int procarea_records_week_id();

void procarea_records_on_mob_death(char_data* victim, procarea_internal::ProcAreaInstance& inst);
void procarea_records_on_boss_killed(procarea_internal::ProcAreaInstance& inst, int clear_sec);
void procarea_records_on_instance_end(const procarea_internal::ProcAreaInstance& inst);

[[nodiscard]] int procarea_records_best_eff_band_get(const char_data* ch);
[[nodiscard]] int procarea_records_best_power_centi_get(const char_data* ch);
[[nodiscard]] int procarea_records_fast_day_sec_get(const char_data* ch);
[[nodiscard]] int procarea_records_fast_week_sec_get(const char_data* ch);
[[nodiscard]] int procarea_records_captain_clears_get(const char_data* ch);
[[nodiscard]] int procarea_records_best_hoard_get(const char_data* ch);
[[nodiscard]] int procarea_records_best_sigils_get(const char_data* ch);

[[nodiscard]] int procarea_records_best_eff_band_for_name(const char* name);
[[nodiscard]] int procarea_records_best_power_centi_for_name(const char* name);
[[nodiscard]] int procarea_records_fast_day_sec_for_name(const char* name);
[[nodiscard]] int procarea_records_fast_week_sec_for_name(const char* name);
[[nodiscard]] int procarea_records_captain_clears_for_name(const char* name);
[[nodiscard]] int procarea_records_best_hoard_for_name(const char* name);
[[nodiscard]] int procarea_records_best_sigils_for_name(const char* name);

void procarea_send_personal_records(char_data* ch);

/** Flush persist/achievement work differito (non chiamare da die()/combattimento). */
void procarea_records_flush_deferred();
void procarea_records_flush_deferred_for(const char* name);

} // namespace Alarmud
#endif // __PROCAREA_RECORDS_HPP
