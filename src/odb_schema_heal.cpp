/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "odb_schema_heal.hpp"
#include "logging.hpp"
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>

namespace Alarmud {

#if USE_MYSQL

namespace {

[[nodiscard]] bool is_already_applied_error(const char* msg) {
	if(msg == nullptr || *msg == '\0') {
		return false;
	}
	/* MySQL: 1050 table exists, 1060 duplicate column, 1061 duplicate key name */
	if(std::strstr(msg, "1050") != nullptr || std::strstr(msg, "1060") != nullptr ||
	   std::strstr(msg, "1061") != nullptr) {
		return true;
	}
	if(std::strstr(msg, "already exists") != nullptr ||
	   std::strstr(msg, "Duplicate column") != nullptr ||
	   std::strstr(msg, "Duplicate key") != nullptr) {
		return true;
	}
	return false;
}

void exec_ignore_exists(DB* db, const char* sql) {
	try {
		db->execute(sql);
	}
	catch(const std::exception& e) {
		if(is_already_applied_error(e.what())) {
			mudlog(LOG_CHECK, "schema heal: already present (%s)", e.what());
			return;
		}
		throw;
	}
}

void ensure_column(DB* db, const char* table, const char* column, const char* type_sql) {
	std::string sql = "ALTER TABLE `";
	sql += table;
	sql += "` ADD COLUMN `";
	sql += column;
	sql += "` ";
	sql += type_sql;
	exec_ignore_exists(db, sql.c_str());
}

void ensure_index(DB* db, const char* table, const char* index_name, const char* columns_sql) {
	std::string sql = "CREATE INDEX `";
	sql += index_name;
	sql += "` ON `";
	sql += table;
	sql += "` (";
	sql += columns_sql;
	sql += ")";
	exec_ignore_exists(db, sql.c_str());
}

/* ---- un passo = una model version ODB. Aggiungere heal_vN a ogni bump. ---- */

void heal_v2(DB* /*db*/) {
	/* changeset vuoto: solo bump versione */
}

void heal_v3(DB* db) {
	ensure_column(db, "character_stats", "odb_migration_probe",
				  "INT UNSIGNED NOT NULL DEFAULT 0");
	ensure_index(db, "character_inventory", "parent_inventory_id_i", "`parent_inventory_id`");
}

void heal_v4(DB* db) {
	ensure_column(db, "character_inventory", "instance_id", "BIGINT UNSIGNED NULL");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `object_instance` ("
		"  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		"  `base_vnum` INT UNSIGNED NOT NULL,"
		"  `char_vnum` INT UNSIGNED NULL,"
		"  `type_flag` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `wear_flags` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `extra_flags` INT NOT NULL DEFAULT 0,"
		"  `extra_flags2` INT NOT NULL DEFAULT 0,"
		"  `weight` INT NOT NULL DEFAULT 0,"
		"  `cost` INT NOT NULL DEFAULT 0,"
		"  `cost_per_day` INT NOT NULL DEFAULT 0,"
		"  `timer` INT NOT NULL DEFAULT 0,"
		"  `bitvector` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `value0` INT NOT NULL DEFAULT 0,"
		"  `value1` INT NOT NULL DEFAULT 0,"
		"  `value2` INT NOT NULL DEFAULT 0,"
		"  `value3` INT NOT NULL DEFAULT 0,"
		"  `obj_name` varchar(128) NOT NULL DEFAULT '',"
		"  `short_desc` varchar(128) NOT NULL DEFAULT '',"
		"  `description` varchar(256) NOT NULL DEFAULT '',"
		"  `action_desc` varchar(256) NULL,"
		"  `owner_toon_id` BIGINT UNSIGNED NULL,"
		"  `legacy_edit_vnum` INT UNSIGNED NULL,"
		"  `created_at` TIMESTAMP NOT NULL,"
		"  `updated_at` TIMESTAMP NOT NULL"
		") ENGINE=InnoDB");
	ensure_index(db, "object_instance", "idx_object_instance_base", "`base_vnum`");
	ensure_index(db, "object_instance", "idx_object_instance_legacy", "`legacy_edit_vnum`");
	ensure_index(db, "object_instance", "idx_object_instance_owner", "`owner_toon_id`");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `object_instance_affect` ("
		"  `key_instance_id` BIGINT UNSIGNED NOT NULL,"
		"  `key_affect_slot` TINYINT UNSIGNED NOT NULL,"
		"  `location` SMALLINT NOT NULL DEFAULT 0,"
		"  `modifier` INT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`key_instance_id`, `key_affect_slot`)"
		") ENGINE=InnoDB");
	ensure_index(db, "character_inventory", "idx_inventory_instance", "`instance_id`");
}

void heal_v5(DB* db) {
	ensure_column(db, "object_instance", "owner_name", "varchar(32) NULL");
	ensure_column(db, "object_instance", "created_by_toon_id", "BIGINT UNSIGNED NULL");
	ensure_column(db, "object_instance", "created_by_name", "varchar(32) NULL");
	ensure_column(db, "object_instance", "updated_by_toon_id", "BIGINT UNSIGNED NULL");
	ensure_column(db, "object_instance", "updated_by_name", "varchar(32) NULL");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `object_instance_event` ("
		"  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		"  `instance_id` BIGINT UNSIGNED NOT NULL,"
		"  `at` TIMESTAMP NOT NULL,"
		"  `actor_toon_id` BIGINT UNSIGNED NULL,"
		"  `actor_name` varchar(32) NULL,"
		"  `kind` varchar(32) NOT NULL,"
		"  `note` varchar(256) NULL"
		") ENGINE=InnoDB");
	ensure_index(db, "object_instance_event", "instance_id_i", "`instance_id`");
	ensure_index(db, "object_instance_event", "idx_object_instance_event_at",
				 "`instance_id`, `at`");
	ensure_index(db, "object_instance", "idx_object_instance_owner_name", "`owner_name`");
}

void heal_v6(DB* db) {
	ensure_column(db, "object_instance_event", "detail", "TEXT NULL");
}

void heal_v7(DB* db) {
	ensure_column(db, "object_instance", "deleted", "TINYINT(1) NOT NULL DEFAULT 0");
	ensure_column(db, "object_instance", "deleted_on", "TIMESTAMP NULL");
	ensure_index(db, "object_instance", "idx_object_instance_deleted", "`deleted`, `id`");
}

void heal_v8(DB* db) {
	ensure_column(db, "character_stats", "edit_hp", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "edit_mana", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "edit_move", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "edit_hp_regen", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "edit_mana_regen", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "edit_move_regen", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "overedit_hp", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "overedit_mana", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "overedit_move", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "overedit_hp_regen", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "overedit_mana_regen", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "overedit_move_regen", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "character_stats", "edit_pool_migrated",
				  "TINYINT UNSIGNED NOT NULL DEFAULT 0");
}

void heal_v9(DB* db) {
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `procarea_balance` ("
		"  `conf_key` varchar(64) NOT NULL PRIMARY KEY,"
		"  `conf_value` varchar(64) NOT NULL,"
		"  `updated_at` TIMESTAMP NOT NULL"
		") ENGINE=InnoDB");
}

void heal_v10(DB* db) {
	ensure_column(db, "object_instance", "source", "varchar(32) NULL");
	ensure_index(db, "object_instance", "idx_object_instance_source", "`source`");
	exec_ignore_exists(
		db,
		"UPDATE `object_instance` SET `source`='procarea_loot' "
		"WHERE (`source` IS NULL OR `source`='') "
		"AND `base_vnum`>=65100 AND `base_vnum`<=65325");
	exec_ignore_exists(
		db,
		"UPDATE `object_instance` SET `source`='clan_symbol' "
		"WHERE (`source` IS NULL OR `source`='') "
		"AND `type_flag`=31");
}

using HealStepFn = void (*)(DB*);

/**
 * Indice = model version. nullptr = nessuno step (v0/v1 = create_schema).
 * Quando si apre model 1.N in account.hpp, aggiungere heal_vN qui.
 */
constexpr HealStepFn kHealByVersion[] = {
	nullptr,  /* 0 */
	nullptr,  /* 1 — schema base via create_schema */
	heal_v2,  /* 2 */
	heal_v3,  /* 3 */
	heal_v4,  /* 4 */
	heal_v5,  /* 5 */
	heal_v6,  /* 6 */
	heal_v7,  /* 7 */
	heal_v8,  /* 8 */
	heal_v9,  /* 9 */
	heal_v10, /* 10 */
};

constexpr odb::schema_version kHealStepsMax =
	static_cast<odb::schema_version>(std::size(kHealByVersion) - 1);

void account_schema_apply_through(DB* db, odb::schema_version target_version) {
	const odb::schema_version last =
		target_version < kHealStepsMax ? target_version : kHealStepsMax;
	for(odb::schema_version ver = 2; ver <= last; ++ver) {
		if(kHealByVersion[ver] != nullptr) {
			kHealByVersion[ver](db);
		}
	}
	if(target_version > kHealStepsMax) {
		mudlog(LOG_SYSERR,
			   "schema heal: target version %llu > last heal step %llu — "
			   "aggiungere heal_vN in odb_schema_heal.cpp",
			   static_cast<unsigned long long>(target_version),
			   static_cast<unsigned long long>(kHealStepsMax));
	}
}

void account_schema_set_version(DB* db, odb::schema_version version) {
	char sql[192];
	std::snprintf(sql, sizeof(sql),
				  "UPDATE `schema_version` SET `version` = %llu, `migration` = 0 "
				  "WHERE `name` = 'account'",
				  static_cast<unsigned long long>(version));
	db->execute(sql);
}

} // namespace

odb::schema_version account_schema_heal(DB* db, odb::schema_version target_version) {
	if(db == nullptr || target_version == 0) {
		return 0;
	}

	const odb::schema_version before = db->schema_version("account");
	mudlog(LOG_CHECK,
		   "schema heal: applying all steps 2..%llu (schema_version now %llu)",
		   static_cast<unsigned long long>(target_version < kHealStepsMax ? target_version
																		 : kHealStepsMax),
		   static_cast<unsigned long long>(before));

	{
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		account_schema_apply_through(db, target_version);
		t.commit();
	}

	odb::schema_version after = db->schema_version("account");
	if(after < target_version && target_version <= kHealStepsMax) {
		odb::transaction t(db->begin());
		t.tracer(logTracer);
		account_schema_set_version(db, target_version);
		t.commit();
		after = db->schema_version("account");
		mudlog(LOG_ALWAYS,
			   "schema heal: advanced schema_version account %llu -> %llu (idempotent catch-up)",
			   static_cast<unsigned long long>(before),
			   static_cast<unsigned long long>(after));
	} else if(after < target_version) {
		mudlog(LOG_SYSERR,
			   "schema heal: cannot advance to %llu — heal steps only to %llu",
			   static_cast<unsigned long long>(target_version),
			   static_cast<unsigned long long>(kHealStepsMax));
	} else {
		mudlog(LOG_CHECK, "schema heal: schema_version already %llu",
			   static_cast<unsigned long long>(after));
	}
	return after;
}

#else /* !USE_MYSQL */

odb::schema_version account_schema_heal(DB* /*db*/, odb::schema_version target_version) {
	return target_version;
}

#endif

} // namespace Alarmud
