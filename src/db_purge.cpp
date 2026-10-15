/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
/* db_purge.cpp - purge temporale centralizzata delle tabelle "storico" DB.
 *
 * Un solo helper template (purge_older_than) implementa la cancellazione delle
 * righe piu' vecchie di una finestra di retention; le funzioni per tabella sono
 * sottili wrapper che scelgono tipo ODB e finestra. Aggiungere una nuova
 * tabella "storico" con membro `at` = una riga qui.
 * */

#include "config.hpp"

#include "structs.hpp"

#include "db_purge.hpp"

#include "logging.hpp"

#if USE_MYSQL

#include "Sql.hpp"
#include "character_item_loss.hpp"
#include "db.hpp"
#include "odb/account-odb.hxx"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/mysql/database.hxx>
#include <string>

namespace Alarmud {
namespace {

/* Cancella da T le righe con `at` piu' vecchio di retention_hours.
 * Ritorna il numero di righe cancellate. retention_hours <= 0 = no-op. */
template <typename T>
unsigned long long purge_older_than(const char* label, int retention_hours) {
	if(retention_hours <= 0) {
		return 0;
	}
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return 0;
	}
	try {
		const boost::posix_time::ptime since =
		    boost::posix_time::second_clock::local_time() -
		    boost::posix_time::hours(retention_hours);
		odb::transaction t(db->begin());
		const unsigned long long n = db->erase_query<T>(odb::query<T>::at < since);
		t.commit();
		if(n > 0) {
			mudlog(LOG_CHECK, "%s: purged %llu rows older than %d hours",
			       label, n, retention_hours);
		}
		else {
			mudlog(LOG_CHECK, "%s: purge ok (0 rows older than %d hours)",
			       label, retention_hours);
		}
		return n;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "%s: %s", label, e.what());
		return 0;
	}
}

} // namespace

void character_item_loss_purge_old() {
	purge_older_than<character_item_loss>("character_item_loss",
	                                      ITEM_LOSS_RETENTION_HOURS);
}

/* vault_log e' raw SQL (nessuna classe ODB): solo i movimenti deposit/
 * withdraw scadono; pagamenti (buy/rent/fee) restano come storico. */
void vault_purge_old() {
	if(VAULT_TX_RETENTION_HOURS <= 0) {
		return;
	}
	DB* db = Sql::getMysql();
	if(db == nullptr) {
		return;
	}
	try {
		odb::transaction t(db->begin());
		const unsigned long long n = db->execute(
			("DELETE FROM vault_log WHERE action IN ('deposit','withdraw') AND at < (NOW() - "
			 "INTERVAL " + std::to_string(VAULT_TX_RETENTION_HOURS) + " HOUR)").c_str());
		t.commit();
		mudlog(LOG_CHECK, "vault_log: purged %llu rows older than %d hours", n,
		       VAULT_TX_RETENTION_HOURS);
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "vault_log: %s", e.what());
	}
}

} // namespace Alarmud

#else /* !USE_MYSQL */

/* Stub senza MySQL: unica definizione (non inline) per entrambe le funzioni
 * dichiarate in db_purge.hpp. */
namespace Alarmud {
void character_item_loss_purge_old() {}
void vault_purge_old() {}
} // namespace Alarmud

#endif /* USE_MYSQL */
