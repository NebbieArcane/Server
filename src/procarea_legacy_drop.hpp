/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_PROCAREA_LEGACY_DROP_HPP_
#define SRC_PROCAREA_LEGACY_DROP_HPP_

/* Pezza transitoria (boot): foto drop 651xx per il create event delle 34k
 * già editate staff (34868/34869). Non tocca l'eq live.
 *
 * Dopo il migrate in produzione la foto vive in MySQL: cancellare questo
 * header, procarea_legacy_drop.cpp e la chiamata in object_instance_boot_migrate.
 */

namespace Alarmud {

struct obj_data;

bool procarea_legacy_drop_has(unsigned legacy_edit_vnum);
bool procarea_legacy_drop_apply_to_proto(struct obj_data* proto, unsigned legacy_edit_vnum);

} // namespace Alarmud
#endif
