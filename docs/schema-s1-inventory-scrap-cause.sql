-- =============================================================================
-- S1 incremental - aggiunge SCRAP a character_inventory.deleted_for
-- =============================================================================
-- Applica DOPO schema-s1-inventory-soft-delete.sql.
-- Idempotente: esegue MODIFY solo se la colonna esiste e SCRAP non c'e' ancora.
-- =============================================================================

SET NAMES utf8mb4;

SET @has_deleted_for := (
  SELECT COUNT(*)
  FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND COLUMN_NAME = 'deleted_for'
);

SET @has_scrap_enum := (
  SELECT COUNT(*)
  FROM information_schema.COLUMNS
  WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'character_inventory'
    AND COLUMN_NAME = 'deleted_for'
    AND COLUMN_TYPE LIKE '%''SCRAP''%'
);

SET @sql_add_scrap := IF(
  @has_deleted_for > 0 AND @has_scrap_enum = 0,
  "ALTER TABLE `character_inventory` MODIFY COLUMN `deleted_for` ENUM('DEATH','RENT_EXPIRED','NUKE','TRAP','MANUAL','SCRAP') NULL DEFAULT NULL COMMENT 'causa cancellazione logica'",
  "SELECT 1"
);
PREPARE stmt FROM @sql_add_scrap;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
