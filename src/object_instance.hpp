/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
//  Original intial comments
/* object_instance.hpp - persistenza eq editato in MySQL (no vnum 34k).
 * See docs/object-instance-plan.md.
 * */
#ifndef SRC_OBJECT_INSTANCE_HPP_
#define SRC_OBJECT_INSTANCE_HPP_

#include "config.hpp"
#include "flags.hpp"

struct obj_data;

namespace Alarmud {

#if USE_MYSQL
/** Prototipo mondo da usare come base_vnum (char_vnum / non-34k). 0 se sconosciuto. */
int object_instance_resolve_base_vnum(const struct obj_data* obj);

/**
 * Crea o aggiorna object_instance (+ affect) da obj.
 * Se update_id != 0 aggiorna quella riga; altrimenti usa obj->db_instance_id o INSERT.
 * Ritorna id (>0) e setta obj->db_instance_id. 0 = errore.
 */
unsigned long long object_instance_persist(struct obj_data* obj, int base_vnum,
										   unsigned long long update_id = 0);

/** Overlay stats/affect/name da object_instance su obj gia' read_object(base). */
bool object_instance_apply(struct obj_data* obj, unsigned long long instance_id);

/** Sync rapido: se obj->db_instance_id, UPDATE istanza da obj live. */
bool object_instance_sync(struct obj_data* obj);
#else
inline int object_instance_resolve_base_vnum(const struct obj_data*) {
	return 0;
}
inline unsigned long long object_instance_persist(struct obj_data*, int,
												 unsigned long long = 0) {
	return 0;
}
inline bool object_instance_apply(struct obj_data*, unsigned long long) {
	return false;
}
inline bool object_instance_sync(struct obj_data*) {
	return false;
}
#endif

} // namespace Alarmud
#endif
