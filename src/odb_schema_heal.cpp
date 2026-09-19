/*ALARMUD* (Do not remove *ALARMUD*, used to automagically manage these lines
 *ALARMUD* AlarMUD 2.0
 *ALARMUD* See COPYING for licence information
 *ALARMUD*/
#include "config.hpp"
#include "odb_schema_heal.hpp"
#include "Sql.hpp"
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
	/* 1050 table/view, 1060 column, 1061 key, 1068 PK, 1091 DROP missing,
	 * 1054 unknown column (CHANGE/DROP gia' fatto), 1826 FK, 3780/3822 CHECK */
	if(std::strstr(msg, "1050") != nullptr || std::strstr(msg, "1060") != nullptr ||
	   std::strstr(msg, "1061") != nullptr || std::strstr(msg, "1068") != nullptr ||
	   std::strstr(msg, "1091") != nullptr || std::strstr(msg, "1054") != nullptr ||
	   std::strstr(msg, "1826") != nullptr || std::strstr(msg, "3780") != nullptr ||
	   std::strstr(msg, "3822") != nullptr || std::strstr(msg, "3823") != nullptr) {
		return true;
	}
	if(std::strstr(msg, "already exists") != nullptr ||
	   std::strstr(msg, "Duplicate column") != nullptr ||
	   std::strstr(msg, "Duplicate key") != nullptr ||
	   std::strstr(msg, "Duplicate foreign key") != nullptr ||
	   std::strstr(msg, "Can't DROP") != nullptr ||
	   std::strstr(msg, "Unknown column") != nullptr ||
	   std::strstr(msg, "Duplicate check") != nullptr ||
	   std::strstr(msg, "check constraint") != nullptr) {
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

/* S1 (toon_id): stesso schema di docs/schema-s1-ddl-draft.sql + incrementali. */
void heal_v2_create_s1_tables(DB* db) {
	exec_ignore_exists(db, "SET FOREIGN_KEY_CHECKS=0");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_core` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `description` VARCHAR(240) NULL,"
		"  `class_primary` INT NOT NULL DEFAULT 0,"
		"  `sex` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `race` INT NOT NULL DEFAULT 0,"
		"  `birth` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `played` INT NOT NULL DEFAULT 0,"
		"  `last_logon` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `weight` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `height` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `hometown` SMALLINT NOT NULL DEFAULT 0,"
		"  `talks_0` TINYINT(1) NOT NULL DEFAULT 0,"
		"  `talks_1` TINYINT(1) NOT NULL DEFAULT 0,"
		"  `talks_2` TINYINT(1) NOT NULL DEFAULT 0,"
		"  `speaks` INT NOT NULL DEFAULT 0,"
		"  `user_flags` INT NOT NULL DEFAULT 0,"
		"  `extra_flags` INT NOT NULL DEFAULT 0,"
		"  `age_modifier` INT NOT NULL DEFAULT 0,"
		"  `authcode` VARCHAR(7) NOT NULL DEFAULT '',"
		"  `wimpy_level` SMALLINT NOT NULL DEFAULT 0,"
		"  `load_room` SMALLINT NOT NULL DEFAULT 0,"
		"  `start_room` INT NOT NULL DEFAULT 0,"
		"  `spells_to_learn` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `alignment` INT NOT NULL DEFAULT 0,"
		"  `act` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `affected_by` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `affected_by2` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `condition_drunk` TINYINT NOT NULL DEFAULT 0,"
		"  `condition_full` TINYINT NOT NULL DEFAULT 0,"
		"  `condition_thirst` TINYINT NOT NULL DEFAULT 0,"
		"  `save_throw_0` SMALLINT NOT NULL DEFAULT 0,"
		"  `save_throw_1` SMALLINT NOT NULL DEFAULT 0,"
		"  `save_throw_2` SMALLINT NOT NULL DEFAULT 0,"
		"  `save_throw_3` SMALLINT NOT NULL DEFAULT 0,"
		"  `save_throw_4` SMALLINT NOT NULL DEFAULT 0,"
		"  `save_throw_5` SMALLINT NOT NULL DEFAULT 0,"
		"  `save_throw_6` SMALLINT NOT NULL DEFAULT 0,"
		"  `save_throw_7` SMALLINT NOT NULL DEFAULT 0,"
		"  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
		"  PRIMARY KEY (`toon_id`),"
		"  CONSTRAINT `fk_character_core_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_classes` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `class_index` TINYINT UNSIGNED NOT NULL,"
		"  `level` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`, `class_index`),"
		"  CONSTRAINT `fk_character_classes_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE,"
		"  CONSTRAINT `chk_character_classes_index` CHECK (`class_index` < 11)"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_stats` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `str` TINYINT NOT NULL DEFAULT 0,"
		"  `str_add` TINYINT NOT NULL DEFAULT 0,"
		"  `intel` TINYINT NOT NULL DEFAULT 0,"
		"  `wis` TINYINT NOT NULL DEFAULT 0,"
		"  `dex` TINYINT NOT NULL DEFAULT 0,"
		"  `con` TINYINT NOT NULL DEFAULT 0,"
		"  `chr` TINYINT NOT NULL DEFAULT 0,"
		"  `extra` TINYINT NOT NULL DEFAULT 0,"
		"  `extra2` TINYINT NOT NULL DEFAULT 0,"
		"  `mana` SMALLINT NOT NULL DEFAULT 0,"
		"  `max_mana` SMALLINT NOT NULL DEFAULT 0,"
		"  `mana_gain` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `hit` SMALLINT NOT NULL DEFAULT 0,"
		"  `max_hit` SMALLINT NOT NULL DEFAULT 0,"
		"  `hit_gain` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `move` SMALLINT NOT NULL DEFAULT 0,"
		"  `max_move` SMALLINT NOT NULL DEFAULT 0,"
		"  `move_gain` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `p_rune_dei` SMALLINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `points_extra1` SMALLINT NOT NULL DEFAULT 0,"
		"  `points_extra2` SMALLINT NOT NULL DEFAULT 0,"
		"  `points_extra3` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `armor` SMALLINT NOT NULL DEFAULT 100,"
		"  `gold` INT NOT NULL DEFAULT 0,"
		"  `bank_gold` INT NOT NULL DEFAULT 0,"
		"  `exp` INT NOT NULL DEFAULT 0,"
		"  `true_exp` INT NOT NULL DEFAULT 0,"
		"  `extra_dual` INT NOT NULL DEFAULT 0,"
		"  `hitroll` TINYINT NOT NULL DEFAULT 0,"
		"  `damroll` TINYINT NOT NULL DEFAULT 0,"
		"  `libero` TINYINT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`),"
		"  CONSTRAINT `fk_character_stats_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_skills` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `skill_id` SMALLINT UNSIGNED NOT NULL,"
		"  `learned` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `flags` TINYINT NOT NULL DEFAULT 0,"
		"  `special` TINYINT NOT NULL DEFAULT 0,"
		"  `nummem` TINYINT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`, `skill_id`),"
		"  CONSTRAINT `fk_character_skills_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_affects` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `slot` TINYINT UNSIGNED NOT NULL,"
		"  `type` SMALLINT NOT NULL DEFAULT 0,"
		"  `duration` SMALLINT NOT NULL DEFAULT 0,"
		"  `modifier` INT NOT NULL DEFAULT 0,"
		"  `location` INT NOT NULL DEFAULT 0,"
		"  `bitvector` INT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`, `slot`),"
		"  CONSTRAINT `fk_character_affects_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_resistance` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `damage_type` INT UNSIGNED NOT NULL,"
		"  `value` SMALLINT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`, `damage_type`),"
		"  CONSTRAINT `fk_character_resistance_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_rent` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `gold_left` INT NOT NULL DEFAULT 0,"
		"  `total_cost` INT NOT NULL DEFAULT 0,"
		"  `last_update` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `minimum_stay` INT NOT NULL DEFAULT 0,"
		"  `object_count` INT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`),"
		"  CONSTRAINT `fk_character_rent_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_inventory` ("
		"  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `list_index` SMALLINT UNSIGNED NOT NULL,"
		"  `item_number` SMALLINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `value0` INT NOT NULL DEFAULT 0,"
		"  `value1` INT NOT NULL DEFAULT 0,"
		"  `value2` INT NOT NULL DEFAULT 0,"
		"  `value3` INT NOT NULL DEFAULT 0,"
		"  `extra_flags` INT NOT NULL DEFAULT 0,"
		"  `extra_flags2` INT NOT NULL DEFAULT 0,"
		"  `weight` INT NOT NULL DEFAULT 0,"
		"  `timer` INT NOT NULL DEFAULT 0,"
		"  `bitvector` INT UNSIGNED NOT NULL DEFAULT 0,"
		"  `obj_name` VARCHAR(128) NOT NULL DEFAULT '',"
		"  `short_desc` VARCHAR(128) NOT NULL DEFAULT '',"
		"  `description` VARCHAR(256) NOT NULL DEFAULT '',"
		"  `wear_pos` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `depth` TINYINT UNSIGNED NOT NULL DEFAULT 0,"
		"  `parent_inventory_id` BIGINT UNSIGNED NULL,"
		"  `deleted` TINYINT(1) NOT NULL DEFAULT 0,"
		"  `deleted_on` DATETIME NULL,"
		"  `deleted_for` ENUM('DEATH','RENT_EXPIRED','NUKE','TRAP','MANUAL','SCRAP') NULL,"
		"  PRIMARY KEY (`id`),"
		"  KEY `idx_inventory_toon` (`toon_id`),"
		"  KEY `idx_inventory_toon_active` (`toon_id`, `deleted`, `list_index`),"
		"  CONSTRAINT `fk_character_inventory_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_inventory_affect` ("
		"  `inventory_id` BIGINT UNSIGNED NOT NULL,"
		"  `affect_slot` TINYINT UNSIGNED NOT NULL,"
		"  `location` SMALLINT NOT NULL DEFAULT 0,"
		"  `modifier` INT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`inventory_id`, `affect_slot`),"
		"  CONSTRAINT `fk_inventory_affect_item` FOREIGN KEY (`inventory_id`)"
		"  REFERENCES `character_inventory` (`id`) ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_prefs` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `pref_key` VARCHAR(32) NOT NULL,"
		"  `pref_value` VARCHAR(1024) NOT NULL,"
		"  PRIMARY KEY (`toon_id`, `pref_key`),"
		"  CONSTRAINT `fk_character_prefs_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_aliases` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `slot` TINYINT UNSIGNED NOT NULL,"
		"  `alias_text` VARCHAR(512) NOT NULL,"
		"  PRIMARY KEY (`toon_id`, `slot`),"
		"  CONSTRAINT `fk_character_aliases_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_achievements` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `category` TINYINT UNSIGNED NOT NULL,"
		"  `slot_index` SMALLINT UNSIGNED NOT NULL,"
		"  `value` INT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`, `category`, `slot_index`),"
		"  CONSTRAINT `fk_character_achievements_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_mercy` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `quest_index` SMALLINT UNSIGNED NOT NULL,"
		"  `value` INT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`, `quest_index`),"
		"  CONSTRAINT `fk_character_mercy_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_quest_progress` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `quest_index` SMALLINT UNSIGNED NOT NULL,"
		"  `mob_slot` TINYINT UNSIGNED NOT NULL,"
		"  `mob_vnum` INT NOT NULL DEFAULT 0,"
		"  PRIMARY KEY (`toon_id`, `quest_index`, `mob_slot`),"
		"  CONSTRAINT `fk_character_quest_progress_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE ON UPDATE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `character_death_snapshot` ("
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `saved_exp` INT NOT NULL,"
		"  `saved_at` INT UNSIGNED NOT NULL,"
		"  `updated_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
		"  PRIMARY KEY (`toon_id`),"
		"  CONSTRAINT `fk_death_snapshot_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)"
		"  ON DELETE CASCADE"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(
		db,
		"CREATE TABLE IF NOT EXISTS `toon_nuke_blacklist` ("
		"  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,"
		"  `toon_id` BIGINT UNSIGNED NOT NULL,"
		"  `toon_name` VARCHAR(32) NOT NULL,"
		"  `nuked_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
		"  `nuked_by` VARCHAR(32) NOT NULL,"
		"  PRIMARY KEY (`id`),"
		"  UNIQUE KEY `uq_toon_id` (`toon_id`),"
		"  KEY `idx_toon_name` (`toon_name`)"
		") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
	exec_ignore_exists(db, "SET FOREIGN_KEY_CHECKS=1");
}

void heal_v2_s1_incrementals(DB* db) {
	ensure_column(db, "toon", "migrated_at", "DATETIME NULL DEFAULT NULL");
	ensure_column(db, "toon", "schema_version", "SMALLINT UNSIGNED NOT NULL DEFAULT 0");
	ensure_column(db, "character_inventory", "parent_inventory_id", "BIGINT UNSIGNED NULL");
	ensure_column(db, "character_inventory", "deleted", "TINYINT(1) NOT NULL DEFAULT 0");
	ensure_column(db, "character_inventory", "deleted_on", "DATETIME NULL");
	ensure_column(db, "character_inventory", "deleted_for",
				  "ENUM('DEATH','RENT_EXPIRED','NUKE','TRAP','MANUAL','SCRAP') NULL");
	exec_ignore_exists(db, "ALTER TABLE `character_inventory` DROP INDEX `uq_inventory_toon_index`");
	ensure_index(db, "character_inventory", "idx_inventory_toon_active",
				 "`toon_id`, `deleted`, `list_index`");
	ensure_index(db, "character_inventory", "idx_inventory_toon_deleted_on",
				 "`toon_id`, `deleted_on`");
	ensure_index(db, "character_inventory", "idx_inventory_toon_deleted_for",
				 "`toon_id`, `deleted_for`");
	exec_ignore_exists(
		db,
		"ALTER TABLE `character_inventory` MODIFY COLUMN `deleted_for` "
		"ENUM('DEATH','RENT_EXPIRED','NUKE','TRAP','MANUAL','SCRAP') NULL");
	exec_ignore_exists(db, "ALTER TABLE `character_core` DROP COLUMN `extra_str`");
	exec_ignore_exists(db, "ALTER TABLE `character_core` DROP COLUMN `condition_3`");
	exec_ignore_exists(db, "ALTER TABLE `character_core` DROP COLUMN `condition_4`");
	exec_ignore_exists(
		db,
		"ALTER TABLE `character_core` CHANGE COLUMN `condition_0` `condition_drunk` "
		"TINYINT NOT NULL DEFAULT 0");
	exec_ignore_exists(
		db,
		"ALTER TABLE `character_core` CHANGE COLUMN `condition_1` `condition_full` "
		"TINYINT NOT NULL DEFAULT 0");
	exec_ignore_exists(
		db,
		"ALTER TABLE `character_core` CHANGE COLUMN `condition_2` `condition_thirst` "
		"TINYINT NOT NULL DEFAULT 0");
	exec_ignore_exists(db, "DELETE FROM `character_classes` WHERE `class_index` >= 11");
}

/* ---- un passo = una model version ODB. Aggiungere heal_vN a ogni bump. ---- */

void heal_v2(DB* db) {
	mudlog(LOG_ALWAYS, "schema heal: applying S1 character_* tables");
	heal_v2_create_s1_tables(db);
	heal_v2_s1_incrementals(db);
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

void heal_v11(DB* db) {
	ensure_column(db, "object_instance", "dust_hp", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "object_instance", "dust_mana", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "object_instance", "dust_move", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "object_instance", "dust_hp_regen", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "object_instance", "dust_mana_regen", "SMALLINT NOT NULL DEFAULT 0");
	ensure_column(db, "object_instance", "dust_move_regen",
				  "SMALLINT NOT NULL DEFAULT 0");
}

void heal_v12(DB* db) {
	ensure_column(db, "object_instance", "dust_spellfail",
				  "SMALLINT NOT NULL DEFAULT 0");
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
	heal_v11, /* 11 */
	heal_v12, /* 12 */
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
