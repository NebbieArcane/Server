-- Sidecar DEATH_FIX: exp + epoch (equivalente a players/<nome>.dead).
-- Dual-write con file finché spell_resurrection/reincarnate/save_exp_to_file non sono solo DB.

CREATE TABLE IF NOT EXISTS `character_death_snapshot` (
  `toon_id` BIGINT UNSIGNED NOT NULL,
  `saved_exp` INT NOT NULL,
  `saved_at` INT UNSIGNED NOT NULL COMMENT 'Unix epoch come nel file .dead',
  `updated_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`toon_id`),
  CONSTRAINT `fk_death_snapshot_toon` FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
