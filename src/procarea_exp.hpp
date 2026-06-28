/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_EXP_HPP
#define __PROCAREA_EXP_HPP
#include "typedefs.hpp"
namespace Alarmud {

namespace procarea_internal {
enum class ProcMobKind;
}

/** Fascia PG 0..7: newbie, allievo, apprendista, iniziato, esperto, maestro, barone, principe. */
[[nodiscard]] int procarea_pc_tier(int group_max_level);

/** XP sul mob alla spawn: stessa curva in solitaria e in gruppo (HP restano scalati). */
[[nodiscard]] int procarea_compute_mob_exp(int group_max_level, int effective_band,
											procarea_internal::ProcMobKind kind, bool solo_mode);

/** Mob runtime procarea (nr==-1) in stanza istanza. */
[[nodiscard]] bool procarea_is_procarea_victim(const char_data* victim);

/** Cap XP al kill in Dimensione Effimera (stessa logica ExpCaps; solo base 200k, Principe 250k). */
[[nodiscard]] long procarea_exp_cap(struct char_data* ch, int group_count, long passedtotal);

} // namespace Alarmud
#endif // __PROCAREA_EXP_HPP
