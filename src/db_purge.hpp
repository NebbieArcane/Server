/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* db_purge.hpp - purge temporale centralizzata delle tabelle "storico" DB.
 *
 * Un unico helper template cancella le righe piu' vecchie di una finestra di
 * retention da tutte le tabelle ODB che espongono un membro `at`
 * (boost::posix_time::ptime). Usato da character_item_loss e dal vault.
 * */
#ifndef SRC_DB_PURGE_HPP_
#define SRC_DB_PURGE_HPP_

#include "config.hpp"
#include "flags.hpp"

namespace Alarmud {

/* Boot: cancella le perdite oggetti piu' vecchie di ITEM_LOSS_RETENTION_HOURS. */
void character_item_loss_purge_old();

/* Boot: cancella i movimenti vault piu' vecchi di VAULT_TX_RETENTION_HOURS ore. */
void vault_purge_old();

} // namespace Alarmud

#endif /* SRC_DB_PURGE_HPP_ */

