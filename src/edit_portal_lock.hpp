/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_EDIT_PORTAL_LOCK_HPP_
#define SRC_EDIT_PORTAL_LOCK_HPP_

namespace odb {
class database;
}

namespace Alarmud {

/**
 * Semaforo portale edit: true se esiste una sessione edit ancora aperta
 * (status=open e non scaduta) oppure job apply pending/running per quel toon.
 * Best-effort: in caso di errore SQL non blocca il login (fail-open loggato).
 */
bool edit_portal_session_blocks_login(odb::database* db, unsigned long long toon_id,
									  const char* toon_name);

} /* namespace Alarmud */

#endif /* SRC_EDIT_PORTAL_LOCK_HPP_ */
