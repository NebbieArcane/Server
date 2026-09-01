/*ALARMUD*
 * Spec proc Incastonatore: incastona pietre da miniera (vnum 19509-19537)
 * secondo il listino e la logica gia' usata da do_insert. Il PG tiene
 * oggetto e pietre con se'; il mob lavora sul banco.
 */
#ifndef SRC_MOB_EDITOR_HPP_
#define SRC_MOB_EDITOR_HPP_

#include "typedefs.hpp"

namespace Alarmud {

MOBSPECIAL_FUNC(Incastonatore);

/* jeweler == nullptr: comando immortale insert (prima persona). */
void incastona_from_command(struct char_data* ch, const char* arg,
							struct char_data* jeweler);

} // namespace Alarmud
#endif
