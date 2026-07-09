/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef __PROCAREA_RUNE_FRAGMENTS_HPP
#define __PROCAREA_RUNE_FRAGMENTS_HPP
#include "typedefs.hpp"
namespace Alarmud {

namespace procarea_internal {
struct ProcAreaInstance;
}

constexpr int PROCAREA_RUNE_FRAGMENTS_PER_RUNE = 1000;

/** Frammenti di runa degli Dei raccolti in Dimensione Effimera (contatore PG). */
[[nodiscard]] int procarea_rune_fragments_get(const char_data* ch);
[[nodiscard]] int procarea_rune_fragments_get_for_name(const char* name);

/** Aggiunge @p delta (puo' essere negativo); ritorna il nuovo totale. */
[[nodiscard]] int procarea_rune_fragments_add(char_data* ch, int delta);

/** Imposta il contatore (>= 0) e persiste. */
void procarea_rune_fragments_set(char_data* ch, int value);

/** Drop frammenti al kill di un mob procarea (istanza @p inst). */
void procarea_rune_fragments_on_mob_death(char_data* victim,
										  const procarea_internal::ProcAreaInstance& inst);

/** Tempio DarkStar: `pray darkstar converti` (o frammenti) se >= 1000 frammenti. */
bool procarea_try_convert_rune_fragments(char_data* ch, const char* prayer);

} // namespace Alarmud
#endif // __PROCAREA_RUNE_FRAGMENTS_HPP
