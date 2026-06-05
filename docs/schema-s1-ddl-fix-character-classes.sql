-- Apply incrementale: character_classes solo indici classe reali (MAX_CLASS=11).
-- Il file .dat ha level[20] (ABS_MAX_CLASS); store/load usano solo 0..10.
-- Rimuove righe spurie (es. class_index 11..19 da import grezzo) e aggiunge CHECK.
--
-- mysql -h 127.0.0.1 -u root -psecret nebbie < docs/schema-s1-ddl-fix-character-classes.sql

SET NAMES utf8mb4;

DELETE FROM `character_classes` WHERE `class_index` >= 11;

ALTER TABLE `character_classes`
  MODIFY COLUMN `class_index` TINYINT UNSIGNED NOT NULL
    COMMENT '0=mage .. 10=psi (MAX_CLASS=11)',
  ADD CONSTRAINT `chk_character_classes_index`
    CHECK (`class_index` < 11);
