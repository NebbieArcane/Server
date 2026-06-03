-- Blacklist PG nukati: audit (chi/quando) + blocco login.
-- Incluso in schema-s1-ddl-draft.sql (§9) e in scripts/apply-schema-s1.sh.
-- Apply standalone se il DB esisteva prima del draft aggiornato:
--   mysql … nebbie < docs/schema-toon-nuke-blacklist.sql

SET NAMES utf8mb4;

CREATE TABLE IF NOT EXISTS `toon_nuke_blacklist` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `toon_id` bigint(20) unsigned NOT NULL,
  `toon_name` varchar(32) NOT NULL,
  `nuked_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `nuked_by` varchar(32) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_toon_id` (`toon_id`),
  KEY `idx_toon_name` (`toon_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
  COMMENT='PG rimossi dal gioco con nuke; login bloccato';
