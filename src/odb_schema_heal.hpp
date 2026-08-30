/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#ifndef SRC_ODB_SCHEMA_HEAL_HPP_
#define SRC_ODB_SCHEMA_HEAL_HPP_

#include "odb/odb.hpp"

namespace Alarmud {

/**
 * Rende idempotenti le migrate ODB account (MySQL DDL auto-commit).
 * Applica SEMPRE tutti gli step heal dalla 2 fino a @p target_version
 * (non solo un sottoinsieme), ignorando "gia' esiste", poi allinea
 * schema_version se il contatore e' indietro.
 * A ogni nuovo model 1.N in account.hpp: aggiungere heal_vN in odb_schema_heal.cpp.
 * heal_v2 crea le tabelle character_* (S1, nomi toon_id) se mancano.
 */
odb::schema_version account_schema_heal(DB* db, odb::schema_version target_version);

} // namespace Alarmud

#endif
