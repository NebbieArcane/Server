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

/** Cleanup pending wizard broker su extract del PG. */
void edit_broker_on_char_extract(struct char_data* ch);

/** Editor description rinomina: ogni riga scritta rinfresca il pending. */
void edit_broker_desc_write_line(struct descriptor_data* d);

/** Fine editor description rinomina (@). true = era nostro, d->str gestito qui. */
bool edit_broker_finish_desc_write(struct descriptor_data* d);

/** close_socket/linkdead: libera il buffer della description e annulla la rinomina. */
void edit_broker_on_descriptor_close(struct descriptor_data* d);

/** Cleanup offerte/pending legate al mob (incastonatore/broker) su extract dell'NPC. */
void edit_broker_on_mob_extract(struct char_data* mob);

/** Riconcilia transfer applied senza done (boot: tutti). */
void edit_broker_reconcile_boot();

/** Riconcilia transfer aperti per il PG che entra in gioco. */
void edit_broker_reconcile_login(struct char_data* ch);

} // namespace Alarmud
#endif
