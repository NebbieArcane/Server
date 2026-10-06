/*ALARMUD*
 * Spec proc mob editoriali:
 * - Incastonatore: incastona pietre da miniera (vnum 19509-19537)
 * - EditAffectBroker: trasferisci affect / distruggi edit PERSONAL
 */
#ifndef SRC_MOB_EDITOR_HPP_
#define SRC_MOB_EDITOR_HPP_

#include "typedefs.hpp"

namespace Alarmud {

MOBSPECIAL_FUNC(Incastonatore);
MOBSPECIAL_FUNC(EditAffectBroker);

ACTION_FUNC(do_incastona);

/* jeweler == nullptr: comando immortale insert (prima persona). */
void incastona_from_command(struct char_data* ch, const char* arg,
							struct char_data* jeweler);

} // namespace Alarmud
#endif
