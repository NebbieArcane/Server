-- =============================================================================
-- S2 incremental - parent_inventory_id su character_inventory
-- =============================================================================
-- Obiettivo:
-- - FK esplicita parent -> id (solo container validi in C++ al save/load)
-- - depth/list_index restano per compatibilita' e ordinamento flat
--
-- Nota: applica DOPO schema-s1-inventory-soft-delete.sql
-- Backfill runtime: backfill_inventory_parent_ids_for_toon() al primo load PG
-- =============================================================================

SET NAMES utf8mb4;

SET @has_parent := (
  SELECT COUNT(*)
  FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND COLUMN_NAME = 'parent_inventory_id'
);
SET @sql_parent := IF(
  @has_parent = 0,
  "ALTER TABLE `character_inventory`
     ADD COLUMN `parent_inventory_id` BIGINT UNSIGNED NULL DEFAULT NULL
       COMMENT 'FK esplicita al contenitore padre; NULL = root (carry/equip)'
       AFTER `depth`",
  "SELECT 1"
);
PREPARE stmt FROM @sql_parent;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_parent_idx := (
  SELECT COUNT(*)
  FROM information_schema.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND INDEX_NAME = 'idx_inventory_parent'
);
SET @sql_parent_idx := IF(
  @has_parent_idx = 0,
  "ALTER TABLE `character_inventory`
     ADD INDEX `idx_inventory_parent` (`parent_inventory_id`)",
  "SELECT 1"
);
PREPARE stmt FROM @sql_parent_idx;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- FK opzionale: abilitare dopo backfill se il DB e' pulito
-- ALTER TABLE `character_inventory`
--   ADD CONSTRAINT `fk_inventory_parent`
--   FOREIGN KEY (`parent_inventory_id`) REFERENCES `character_inventory` (`id`)
--   ON DELETE CASCADE;
