/*
 * toon_migration.cpp — gate cutover C2 (see toon_migration.hpp).
 * CUTOVER_V3: scalar SELECT via odb::mysql::connection::handle(), not query_value().
 */

#include "toon_migration.hpp"

#include "config.hpp"
#include "flags.hpp"
#include "logging.hpp"

#if USE_MYSQL

#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <odb/mysql/connection.hxx>
#include <mysql/mysql.h>

#include <cstdlib>
#include <string>

#include "Sql.hpp"

namespace Alarmud {
namespace {

unsigned long long toon_query_scalar_conn(odb::database* db, const std::string& sql) {
	odb::connection_ptr cp(db->connection());
	auto& mc = static_cast<odb::mysql::connection&>(*cp);
	MYSQL* h = mc.handle();
	if(mysql_query(h, sql.c_str()) != 0) {
		mudlog(LOG_SYSERR, "toon_query_scalar: %s", mysql_error(h));
		return 0;
	}
	MYSQL_RES* res = mysql_store_result(h);
	unsigned long long n = 0;
	if(res) {
		if(MYSQL_ROW row = mysql_fetch_row(res)) {
			if(row[0]) {
				n = std::strtoull(row[0], nullptr, 10);
			}
		}
		mysql_free_result(res);
	}
	return n;
}

unsigned long long toon_query_scalar(odb::database* db, const std::string& sql) {
	odb::transaction t(db->begin());
	t.tracer(logTracer);
	const unsigned long long n = toon_query_scalar_conn(db, sql);
	t.commit();
	return n;
}

bool toon_column_exists(odb::database* db, const char* column) {
	if(!db || !column || !*column) {
		return false;
	}
	try {
		const std::string sql =
			"SELECT COUNT(*) FROM information_schema.COLUMNS "
			"WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'toon' AND COLUMN_NAME = '" +
			std::string(column) + "'";
		return toon_query_scalar(db, sql) > 0;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "toon_column_exists: %s", e.what());
		return false;
	}
}

} /* anonymous */

bool toon_fetch_migration_state(odb::database* db, unsigned long long toon_id, bool& migrated,
								unsigned short& schema_version) {
	migrated = false;
	schema_version = 0;
	if(!db || !toon_id) {
		return false;
	}
	if(!toon_column_exists(db, "migrated_at")) {
		return true;
	}
	try {
		const std::string id = std::to_string(toon_id);
		const std::string count_sql =
			"SELECT COUNT(*) FROM toon WHERE id = " + id + " AND migrated_at IS NOT NULL";
		const unsigned long long migrated_count = toon_query_scalar(db, count_sql);
		migrated = migrated_count > 0;
		if(migrated) {
			const std::string ver_sql =
				"SELECT schema_version FROM toon WHERE id = " + id + " LIMIT 1";
			schema_version =
				static_cast<unsigned short>(toon_query_scalar(db, ver_sql));
		}
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "toon_fetch_migration_state: %s", e.what());
		return false;
	}
}

bool toon_is_migrated(odb::database* db, const toon& pg) {
	if(!pg.migrated_at.null()) {
		return true;
	}
	if(!db || !pg.id) {
		return false;
	}
	bool migrated = false;
	unsigned short ver = 0;
	if(!toon_fetch_migration_state(db, pg.id, migrated, ver)) {
		return false;
	}
	return migrated;
}

bool toon_needs_migration(odb::database* db, const toon& pg) {
	bool migrated = false;
	unsigned short ver = 0;
	if(db && pg.id && toon_fetch_migration_state(db, pg.id, migrated, ver)) {
		if(!migrated) {
			return true;
		}
		return ver < CHARACTER_MIGRATION_SCHEMA_VERSION;
	}
	if(!pg.migrated_at.null()) {
		return pg.schema_version < CHARACTER_MIGRATION_SCHEMA_VERSION;
	}
	return true;
}

bool toon_has_character_core(odb::database* db, unsigned long long toon_id) {
	if(!db || !toon_id) {
		return false;
	}
	try {
		const std::string sql =
			"SELECT COUNT(*) FROM character_core WHERE toon_id = " + std::to_string(toon_id);
		return toon_query_scalar(db, sql) > 0;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "toon_has_character_core: %s", e.what());
		return false;
	}
}

void toon_mark_migrated_tx(odb::database* db, unsigned long long toon_id,
						   unsigned short schema_version) {
	if(!db || !toon_id) {
		return;
	}
	std::string sql = "UPDATE toon SET migrated_at = NOW(), schema_version = ";
	sql += std::to_string(static_cast<unsigned>(schema_version));
	sql += " WHERE id = ";
	sql += std::to_string(toon_id);
	db->execute(sql.c_str());
}

bool toon_clear_migration(unsigned long long toon_id) {
	if(!toon_id) {
		return false;
	}
	try {
		DB* db = Sql::getMysql();
		if(!toon_column_exists(db, "migrated_at")) {
			return true;
		}
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		const std::string sql =
			"UPDATE toon SET migrated_at = NULL, schema_version = 0 WHERE id = " +
			std::to_string(toon_id);
		db->execute(sql.c_str());
		t.commit();
		return true;
	}
	catch(const odb::exception& e) {
		mudlog(LOG_SYSERR, "toon_clear_migration: %s", e.what());
		return false;
	}
}

bool toon_migration_sanity_check(odb::database* db, const toon& pg) {
	if(!toon_is_migrated(db, pg)) {
		return true;
	}
	if(toon_has_character_core(db, pg.id)) {
		return true;
	}
	mudlog(LOG_SYSERR,
		   "toon_migration: %s (id=%lu) ha migrated_at ma manca character_core",
		   pg.name.c_str(), pg.id);
	return false;
}

} /* namespace Alarmud */

#else /* !USE_MYSQL */

namespace Alarmud {

bool toon_fetch_migration_state(odb::database* db, unsigned long long toon_id, bool& migrated,
								unsigned short& schema_version) {
	(void)db;
	(void)toon_id;
	migrated = false;
	schema_version = 0;
	return false;
}

bool toon_is_migrated(odb::database* db, const toon& pg) {
	(void)db;
	(void)pg;
	return false;
}

bool toon_needs_migration(odb::database* db, const toon& pg) {
	(void)db;
	(void)pg;
	return false;
}

bool toon_has_character_core(odb::database* db, unsigned long long toon_id) {
	(void)db;
	(void)toon_id;
	return false;
}

void toon_mark_migrated_tx(odb::database* db, unsigned long long toon_id,
						   unsigned short schema_version) {
	(void)db;
	(void)toon_id;
	(void)schema_version;
}

bool toon_clear_migration(unsigned long long toon_id) {
	(void)toon_id;
	return false;
}

bool toon_migration_sanity_check(odb::database* db, const toon& pg) {
	(void)db;
	(void)pg;
	return true;
}

} /* namespace Alarmud */

#endif /* USE_MYSQL */
