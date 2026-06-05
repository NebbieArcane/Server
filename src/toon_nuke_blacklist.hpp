/*
 * toon_nuke_blacklist.hpp — PG nukati: audit + blocco ingresso.
 */

#ifndef SRC_TOON_NUKE_BLACKLIST_HPP_
#define SRC_TOON_NUKE_BLACKLIST_HPP_

namespace odb {
class database;
}

namespace Alarmud {

/** Tabella toon_nuke_blacklist presente (applica docs/schema-toon-nuke-blacklist.sql). */
bool toon_nuke_table_exists(odb::database* db);

/** true se toon_id o nome (case-insensitive) sono in blacklist. */
bool toon_nuke_is_blocked(odb::database* db, unsigned long long toon_id, const char* name);

/** Inserisce/aggiorna riga blacklist. Ritorna false se tabella assente o errore SQL. */
bool toon_nuke_blacklist_add(odb::database* db, unsigned long long toon_id, const char* name,
							 const char* nuked_by);

/** Rimuove da blacklist (per toon_id e/o nome). Ritorna true se c'era una riga. */
bool toon_nuke_blacklist_remove(odb::database* db, unsigned long long toon_id, const char* name);

} /* namespace Alarmud */

#endif /* SRC_TOON_NUKE_BLACKLIST_HPP_ */
