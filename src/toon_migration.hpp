/*
 * toon_migration.hpp — gate cutover C2 (migrated_at / schema_version su toon).
 */

#ifndef SRC_TOON_MIGRATION_HPP_
#define SRC_TOON_MIGRATION_HPP_

#include "odb/account.hpp"

namespace odb {
class database;
}

namespace Alarmud {

/** Schema character_* atteso da legacy_import (bump quando cambia DDL/mapping). */
constexpr unsigned short CHARACTER_MIGRATION_SCHEMA_VERSION = 1;

/** Legge migrated_at e schema_version da MySQL (affidabile anche prima di rigenerare ODB). */
bool toon_fetch_migration_state(odb::database* db, unsigned long long toon_id, bool& migrated,
								unsigned short& schema_version);

/** true se migrated_at valorizzato (usa fetch se db != nullptr). */
bool toon_is_migrated(odb::database* db, const toon& pg);

/** Gate D2: lookup per nome PG (no-op se !USE_MYSQL). */
bool toon_is_migrated_by_name(const char* name);

/** true se serve import character_* (non migrato o schema obsoleto). */
bool toon_needs_migration(odb::database* db, const toon& pg);

/** true se esiste riga character_core per toon_id. */
bool toon_has_character_core(odb::database* db, unsigned long long toon_id);

/**
 * Imposta migrated_at e schema_version nella transazione aperta.
 * Chiamare solo dopo insert character_* riusciti, prima di commit.
 */
void toon_mark_migrated_tx(odb::database* db, unsigned long long toon_id,
						   unsigned short schema_version = CHARACTER_MIGRATION_SCHEMA_VERSION);

/** Azzera flag (restore / re-import). */
bool toon_clear_migration(unsigned long long toon_id);

/**
 * migrated_at set ma manca character_core → log errore, ritorna false.
 * Usabile al login post-cutover.
 */
bool toon_migration_sanity_check(odb::database* db, const toon& pg);

} /* namespace Alarmud */

#endif /* SRC_TOON_MIGRATION_HPP_ */
