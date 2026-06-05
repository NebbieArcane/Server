-- Apply incrementale: solo character_resistance (se il DB ha già le altre character_*)
-- mysql -h 127.0.0.1 -u root -psecret nebbie < docs/schema-s1-ddl-add-resistance.sql

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS `character_resistance` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `damage_type`     INT UNSIGNED NOT NULL COMMENT 'bit IMM_* da autoenums.hpp',
  `value`           SMALLINT NOT NULL DEFAULT 0
                    COMMENT '-100=danni x2, 0=normale, +100=immune',

  PRIMARY KEY (`toon_id`, `damage_type`),
  CONSTRAINT `fk_character_resistance_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `chk_character_resistance_value`
    CHECK (`value` BETWEEN -100 AND 100)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
