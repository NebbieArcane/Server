-- Incrementale: flag cutover su toon (gate C2).
-- Applica su DB nebbie se non usi ODB schema_catalog::migrate per modello "account".
-- Con ODB: bump account.hpp a version(1,2,open) e avvia il mud (Sql::dbUpdate migra).

SET NAMES utf8mb4;

ALTER TABLE `toon`
  ADD COLUMN `migrated_at` DATETIME NULL DEFAULT NULL
    COMMENT 'character_* import completato; NULL = non migrato',
  ADD COLUMN `schema_version` SMALLINT UNSIGNED NOT NULL DEFAULT 0
    COMMENT 'versione schema character_* all ultima migrazione OK';
