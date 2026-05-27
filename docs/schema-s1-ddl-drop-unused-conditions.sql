-- Apply incrementale: rimuove condition_3/condition_4 e rinomina le tre condition usate.
-- Il file .dat ha MAX_CONDITIONS=5 ma store_char/load_char usano solo indici 0..2.
--
-- mysql -h 127.0.0.1 -u root -psecret nebbie < docs/schema-s1-ddl-drop-unused-conditions.sql

SET NAMES utf8mb4;

ALTER TABLE `character_core`
  CHANGE COLUMN `condition_0` `condition_drunk` TINYINT NOT NULL DEFAULT 0
    COMMENT 'char_file_u.conditions[0] DRUNK',
  CHANGE COLUMN `condition_1` `condition_full` TINYINT NOT NULL DEFAULT 0
    COMMENT 'conditions[1] FULL',
  CHANGE COLUMN `condition_2` `condition_thirst` TINYINT NOT NULL DEFAULT 0
    COMMENT 'conditions[2] THIRST',
  DROP COLUMN `condition_3`,
  DROP COLUMN `condition_4`;
