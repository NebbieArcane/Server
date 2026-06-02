-- =============================================================================
-- S1 incremental - soft delete storico inventario rent
-- =============================================================================
-- Obiettivo:
-- - evitare restore da dump per refund;
-- - mantenere storico perdita oggetti per causa;
-- - rendere possibile refund SQL-only.
--
-- Nota: applica questo script DOPO schema-s1-ddl-draft.sql.
-- =============================================================================

SET NAMES utf8mb4;

SET @has_deleted := (
  SELECT COUNT(*)
  FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND COLUMN_NAME = 'deleted'
);
SET @sql_deleted := IF(
  @has_deleted = 0,
  "ALTER TABLE `character_inventory` ADD COLUMN `deleted` TINYINT(1) NOT NULL DEFAULT 0 COMMENT '0=attivo, 1=storico/non caricare'",
  "SELECT 1"
);
PREPARE stmt FROM @sql_deleted;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_deleted_on := (
  SELECT COUNT(*)
  FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND COLUMN_NAME = 'deleted_on'
);
SET @sql_deleted_on := IF(
  @has_deleted_on = 0,
  "ALTER TABLE `character_inventory` ADD COLUMN `deleted_on` DATETIME NULL DEFAULT NULL COMMENT 'timestamp cancellazione logica'",
  "SELECT 1"
);
PREPARE stmt FROM @sql_deleted_on;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_deleted_for := (
  SELECT COUNT(*)
  FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND COLUMN_NAME = 'deleted_for'
);
SET @sql_deleted_for := IF(
  @has_deleted_for = 0,
  "ALTER TABLE `character_inventory` ADD COLUMN `deleted_for` ENUM('DEATH','RENT_EXPIRED','NUKE','TRAP','MANUAL') NULL DEFAULT NULL COMMENT 'causa cancellazione logica'",
  "SELECT 1"
);
PREPARE stmt FROM @sql_deleted_for;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Quando deleted=1, list_index non e' piu' univoco (storicizzazione snapshot).
SET @has_uq_inventory := (
  SELECT COUNT(*)
  FROM information_schema.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND INDEX_NAME = 'uq_inventory_toon_index'
);
SET @sql_drop_uq := IF(
  @has_uq_inventory > 0,
  "ALTER TABLE `character_inventory` DROP INDEX `uq_inventory_toon_index`",
  "SELECT 1"
);
PREPARE stmt FROM @sql_drop_uq;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_idx_active := (
  SELECT COUNT(*)
  FROM information_schema.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND INDEX_NAME = 'idx_inventory_toon_active'
);
SET @sql_add_idx_active := IF(
  @has_idx_active = 0,
  "ALTER TABLE `character_inventory` ADD KEY `idx_inventory_toon_active` (`toon_id`, `deleted`, `list_index`)",
  "SELECT 1"
);
PREPARE stmt FROM @sql_add_idx_active;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_idx_deleted_on := (
  SELECT COUNT(*)
  FROM information_schema.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND INDEX_NAME = 'idx_inventory_toon_deleted_on'
);
SET @sql_add_idx_deleted_on := IF(
  @has_idx_deleted_on = 0,
  "ALTER TABLE `character_inventory` ADD KEY `idx_inventory_toon_deleted_on` (`toon_id`, `deleted_on`)",
  "SELECT 1"
);
PREPARE stmt FROM @sql_add_idx_deleted_on;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @has_idx_deleted_for := (
  SELECT COUNT(*)
  FROM information_schema.STATISTICS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND INDEX_NAME = 'idx_inventory_toon_deleted_for'
);
SET @sql_add_idx_deleted_for := IF(
  @has_idx_deleted_for = 0,
  "ALTER TABLE `character_inventory` ADD KEY `idx_inventory_toon_deleted_for` (`toon_id`, `deleted_for`)",
  "SELECT 1"
);
PREPARE stmt FROM @sql_add_idx_deleted_for;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Aggiorna righe legacy: se deleted=0, i campi metadata devono essere NULL.
UPDATE `character_inventory`
SET `deleted_on` = NULL, `deleted_for` = NULL
WHERE `deleted` = 0;

-- Query di servizio (copiare/eseguire manualmente quando serve):
-- 1) Carico inventario attivo:
--    SELECT * FROM character_inventory
--    WHERE toon_id = ? AND deleted = 0
--    ORDER BY list_index;
--
-- 2) Soft-delete per morte:
--    UPDATE character_inventory
--    SET deleted = 1, deleted_on = NOW(), deleted_for = 'DEATH'
--    WHERE toon_id = ? AND deleted = 0;
--
-- 3) Refund DEATH entro N ore:
--    UPDATE character_inventory
--    SET deleted = 0, deleted_on = NULL, deleted_for = NULL
--    WHERE toon_id = ? AND deleted = 1
--      AND deleted_for = 'DEATH'
--      AND deleted_on >= (NOW() - INTERVAL ? HOUR);

