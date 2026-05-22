-- =============================================================================
-- S1 — DDL draft (gameplay layer)
-- =============================================================================
-- Database: nebbie (default)
-- Prerequisito: schema ODB "account" v1 già presente (user, toon, legacy, …)
--
-- NON eseguire in produzione senza review.
-- NON droppa user / toon / legacy.
-- Modello ODB "character" da generare in un secondo momento da character.hpp.
--
-- Gerarchia:
--   user (account) 1 — N toon (owner_id)
--   toon 1 — 1 character_core (toon_id)
--   toon 1 — N character_* (tutte con FK toon_id → toon.id)
--
-- Riferimenti: docs/schema-s1-mapping.md, docs/schema-s1-vs-mysql.md,
--               docs/resistance-bit-to-value.md
-- Costanti: MAX_SKILLS=350, MAX_AFFECT=40, MAX_OBJ_SAVE=200, MAX_OBJ_AFFECT=5
-- =============================================================================

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- -----------------------------------------------------------------------------
-- 1. character_core — dati da char_file_u (esclusi array normalizzati)
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `character_core` (
  `toon_id`           BIGINT UNSIGNED NOT NULL COMMENT 'FK → toon.id, 1 PG = 1 riga',

  `description`       VARCHAR(240)    NULL,
  `extra_str`         VARCHAR(255)    NULL,

  `class_primary`     INT             NOT NULL DEFAULT 0 COMMENT 'char_file_u.iClass',
  `sex`               TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `race`              INT             NOT NULL DEFAULT 0,

  `birth`             INT UNSIGNED    NOT NULL DEFAULT 0 COMMENT 'epoch seconds',
  `played`            INT             NOT NULL DEFAULT 0,
  `last_logon`        INT UNSIGNED    NOT NULL DEFAULT 0 COMMENT 'char_file_u; diverso da toon.lastlogin DATETIME',

  `weight`            INT UNSIGNED    NOT NULL DEFAULT 0,
  `height`            INT UNSIGNED    NOT NULL DEFAULT 0,
  `hometown`          SMALLINT        NOT NULL DEFAULT 0,

  `talks_0`           TINYINT(1)      NOT NULL DEFAULT 0,
  `talks_1`           TINYINT(1)      NOT NULL DEFAULT 0,
  `talks_2`           TINYINT(1)      NOT NULL DEFAULT 0 COMMENT 'MAX_TOUNGE=3',

  `speaks`            INT             NOT NULL DEFAULT 0,
  `user_flags`        INT             NOT NULL DEFAULT 0,
  `extra_flags`       INT             NOT NULL DEFAULT 0,
  `age_modifier`      INT             NOT NULL DEFAULT 0 COMMENT 'agemod',

  `authcode`          VARCHAR(7)      NOT NULL DEFAULT '',
  `wimpy_level`       SMALLINT        NOT NULL DEFAULT 0 COMMENT 'da WimpyLevel[4], non stringa file',

  `load_room`         SMALLINT        NOT NULL DEFAULT 0,
  `start_room`        INT             NOT NULL DEFAULT 0,

  `spells_to_learn`   TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `alignment`         INT             NOT NULL DEFAULT 0,
  `act`               INT UNSIGNED    NOT NULL DEFAULT 0,
  `affected_by`       INT UNSIGNED    NOT NULL DEFAULT 0,
  `affected_by2`      INT UNSIGNED    NOT NULL DEFAULT 0,

  `condition_0`       TINYINT         NOT NULL DEFAULT 0,
  `condition_1`       TINYINT         NOT NULL DEFAULT 0,
  `condition_2`       TINYINT         NOT NULL DEFAULT 0,
  `condition_3`       TINYINT         NOT NULL DEFAULT 0,
  `condition_4`       TINYINT         NOT NULL DEFAULT 0 COMMENT 'MAX_CONDITIONS=5; loop save/load usa 0..2',

  `save_throw_0`      SMALLINT        NOT NULL DEFAULT 0,
  `save_throw_1`      SMALLINT        NOT NULL DEFAULT 0,
  `save_throw_2`      SMALLINT        NOT NULL DEFAULT 0,
  `save_throw_3`      SMALLINT        NOT NULL DEFAULT 0,
  `save_throw_4`      SMALLINT        NOT NULL DEFAULT 0,
  `save_throw_5`      SMALLINT        NOT NULL DEFAULT 0,
  `save_throw_6`      SMALLINT        NOT NULL DEFAULT 0,
  `save_throw_7`      SMALLINT        NOT NULL DEFAULT 0 COMMENT 'MAX_SAVES=8',

  `updated_at`        TIMESTAMP       NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

  PRIMARY KEY (`toon_id`),
  CONSTRAINT `fk_character_core_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Nota: name, password, title, level “display”, lastlogin, lasthost, owner_id
-- restano su `toon` (account/registry layer).

-- -----------------------------------------------------------------------------
-- 2. character_classes — char_file_u.level[ABS_MAX_CLASS] (20)
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `character_classes` (
  `toon_id`       BIGINT UNSIGNED NOT NULL,
  `class_index`   TINYINT UNSIGNED NOT NULL COMMENT '0 .. ABS_MAX_CLASS-1',
  `level`         TINYINT UNSIGNED NOT NULL DEFAULT 0,

  PRIMARY KEY (`toon_id`, `class_index`),
  CONSTRAINT `fk_character_classes_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- -----------------------------------------------------------------------------
-- 3. character_stats — char_ability_data + char_point_data (1 riga per PG)
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `character_stats` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,

  `str`             TINYINT NOT NULL DEFAULT 0,
  `str_add`         TINYINT NOT NULL DEFAULT 0,
  `intel`           TINYINT NOT NULL DEFAULT 0,
  `wis`             TINYINT NOT NULL DEFAULT 0,
  `dex`             TINYINT NOT NULL DEFAULT 0,
  `con`             TINYINT NOT NULL DEFAULT 0,
  `chr`             TINYINT NOT NULL DEFAULT 0,
  `extra`           TINYINT NOT NULL DEFAULT 0,
  `extra2`          TINYINT NOT NULL DEFAULT 0,

  `mana`            SMALLINT NOT NULL DEFAULT 0,
  `max_mana`        SMALLINT NOT NULL DEFAULT 0,
  `mana_gain`       TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `hit`             SMALLINT NOT NULL DEFAULT 0,
  `max_hit`         SMALLINT NOT NULL DEFAULT 0,
  `hit_gain`        TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `move`            SMALLINT NOT NULL DEFAULT 0,
  `max_move`        SMALLINT NOT NULL DEFAULT 0,
  `move_gain`       TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `p_rune_dei`      SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `points_extra1`   SMALLINT NOT NULL DEFAULT 0,
  `points_extra2`   SMALLINT NOT NULL DEFAULT 0,
  `points_extra3`   TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `armor`           SMALLINT NOT NULL DEFAULT 100,
  `gold`            INT NOT NULL DEFAULT 0,
  `bank_gold`       INT NOT NULL DEFAULT 0,
  `exp`             INT NOT NULL DEFAULT 0,
  `true_exp`        INT NOT NULL DEFAULT 0,
  `extra_dual`      INT NOT NULL DEFAULT 0,
  `hitroll`         TINYINT NOT NULL DEFAULT 0,
  `damroll`         TINYINT NOT NULL DEFAULT 0,
  `libero`          TINYINT NOT NULL DEFAULT 0,

  PRIMARY KEY (`toon_id`),
  CONSTRAINT `fk_character_stats_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- -----------------------------------------------------------------------------
-- 4. character_skills — char_skill_data × MAX_SKILLS (350)
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `character_skills` (
  `toon_id`       BIGINT UNSIGNED NOT NULL,
  `skill_id`      SMALLINT UNSIGNED NOT NULL COMMENT '0 .. MAX_SKILLS-1',
  `learned`       TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `flags`         TINYINT NOT NULL DEFAULT 0,
  `special`       TINYINT NOT NULL DEFAULT 0,
  `nummem`        TINYINT NOT NULL DEFAULT 0,

  PRIMARY KEY (`toon_id`, `skill_id`),
  CONSTRAINT `fk_character_skills_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Migrazione: inserire solo skill con learned>0 OR flags!=0.

-- -----------------------------------------------------------------------------
-- 5. character_affects — affected_type_u × MAX_AFFECT (40)
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `character_affects` (
  `toon_id`       BIGINT UNSIGNED NOT NULL,
  `slot`          TINYINT UNSIGNED NOT NULL COMMENT '0 .. MAX_AFFECT-1',
  `type`          SMALLINT NOT NULL DEFAULT 0 COMMENT '0 = slot vuoto',
  `duration`      SMALLINT NOT NULL DEFAULT 0,
  `modifier`      INT NOT NULL DEFAULT 0,
  `location`      INT NOT NULL DEFAULT 0,
  `bitvector`     INT NOT NULL DEFAULT 0,

  PRIMARY KEY (`toon_id`, `slot`),
  CONSTRAINT `fk_character_affects_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Migrazione: solo slot con type != 0.

-- -----------------------------------------------------------------------------
-- 5b. Resistenze — scala -100 (suscettibile) .. +100 (immune)
-- -----------------------------------------------------------------------------
-- Sostituisce a regime immune / M_immune / susc (bitvector su char_data).
-- Migrazione bit → value: docs/resistance-bit-to-value.md
-- Codice combat non usa ancora questa tabella fino a refactor dedicato.

CREATE TABLE IF NOT EXISTS `character_resistance` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `damage_type`     INT UNSIGNED NOT NULL COMMENT 'bit IMM_* da autoenums.hpp (1,2,4,8,16,...)',
  `value`           SMALLINT NOT NULL DEFAULT 0
                    COMMENT '-100=danni x2, 0=normale, +100=immune',

  PRIMARY KEY (`toon_id`, `damage_type`),
  CONSTRAINT `fk_character_resistance_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `chk_character_resistance_value`
    CHECK (`value` BETWEEN -100 AND 100)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- -----------------------------------------------------------------------------
-- 6. Rent / inventario — obj_file_u (file RENT_DIR/<name>)
-- -----------------------------------------------------------------------------

CREATE TABLE IF NOT EXISTS `character_rent` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `gold_left`       INT NOT NULL DEFAULT 0,
  `total_cost`      INT NOT NULL DEFAULT 0,
  `last_update`     INT UNSIGNED NOT NULL DEFAULT 0,
  `minimum_stay`    INT NOT NULL DEFAULT 0,
  `object_count`    INT NOT NULL DEFAULT 0 COMMENT 'obj_file_u.number',

  PRIMARY KEY (`toon_id`),
  CONSTRAINT `fk_character_rent_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `character_inventory` (
  `id`              BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `list_index`      SMALLINT UNSIGNED NOT NULL COMMENT 'ordine in obj_file_u.objects[]',

  `item_number`     SMALLINT UNSIGNED NOT NULL DEFAULT 0,
  `value0`          INT NOT NULL DEFAULT 0,
  `value1`          INT NOT NULL DEFAULT 0,
  `value2`          INT NOT NULL DEFAULT 0,
  `value3`          INT NOT NULL DEFAULT 0,
  `extra_flags`     INT NOT NULL DEFAULT 0,
  `extra_flags2`    INT NOT NULL DEFAULT 0,
  `weight`          INT NOT NULL DEFAULT 0,
  `timer`           INT NOT NULL DEFAULT 0,
  `bitvector`       INT UNSIGNED NOT NULL DEFAULT 0,
  `obj_name`        VARCHAR(128) NOT NULL DEFAULT '',
  `short_desc`      VARCHAR(128) NOT NULL DEFAULT '',
  `description`     VARCHAR(256) NOT NULL DEFAULT '',
  `wear_pos`        TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `depth`           TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'annidamento inventario',

  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_inventory_toon_index` (`toon_id`, `list_index`),
  KEY `idx_inventory_toon` (`toon_id`),
  CONSTRAINT `fk_character_inventory_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `character_inventory_affect` (
  `inventory_id`    BIGINT UNSIGNED NOT NULL,
  `affect_slot`     TINYINT UNSIGNED NOT NULL COMMENT '0 .. MAX_OBJ_AFFECT-1',
  `location`        SMALLINT NOT NULL DEFAULT 0,
  `modifier`        INT NOT NULL DEFAULT 0,

  PRIMARY KEY (`inventory_id`, `affect_slot`),
  CONSTRAINT `fk_inventory_affect_item`
    FOREIGN KEY (`inventory_id`) REFERENCES `character_inventory` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Sostituisce il concetto di `toonRent` (non usata, schema incompatibile).

-- -----------------------------------------------------------------------------
-- 7. Dati da .aux
-- -----------------------------------------------------------------------------

-- Preferenze / poof / prompt (ex toonExtra con FK corretto)
CREATE TABLE IF NOT EXISTS `character_prefs` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `pref_key`        VARCHAR(32)     NOT NULL COMMENT 'prompt, poof_in, poof_out, zone, version, principe, …',
  `pref_value`      VARCHAR(1024)   NOT NULL,

  PRIMARY KEY (`toon_id`, `pref_key`),
  CONSTRAINT `fk_character_prefs_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `character_aliases` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `slot`            TINYINT UNSIGNED NOT NULL COMMENT '0..9 da file .aux',
  `alias_text`      VARCHAR(512)    NOT NULL,

  PRIMARY KEY (`toon_id`, `slot`),
  CONSTRAINT `fk_character_aliases_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- achievement category: 0=CLASS, 1=BOSSKILL, 2=QUEST, 3=OTHER, 4=RACESLAYER (autoenums.hpp)
CREATE TABLE IF NOT EXISTS `character_achievements` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `category`        TINYINT UNSIGNED NOT NULL,
  `slot_index`      SMALLINT UNSIGNED NOT NULL,
  `value`           INT NOT NULL DEFAULT 0,

  PRIMARY KEY (`toon_id`, `category`, `slot_index`),
  CONSTRAINT `fk_character_achievements_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `character_mercy` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `quest_index`     SMALLINT UNSIGNED NOT NULL COMMENT '0 .. MAX_QUEST_ACHIE-1',
  `value`           INT NOT NULL DEFAULT 0,

  PRIMARY KEY (`toon_id`, `quest_index`),
  CONSTRAINT `fk_character_mercy_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Opzionale: progresso quest mob (oggi non salvato in .aux — fix bug persistenza)
CREATE TABLE IF NOT EXISTS `character_quest_progress` (
  `toon_id`         BIGINT UNSIGNED NOT NULL,
  `quest_index`     SMALLINT UNSIGNED NOT NULL,
  `mob_slot`        TINYINT UNSIGNED NOT NULL COMMENT '0 .. MAX_MOB_QUEST-1',
  `mob_vnum`        INT NOT NULL DEFAULT 0,

  PRIMARY KEY (`toon_id`, `quest_index`, `mob_slot`),
  CONSTRAINT `fk_character_quest_progress_toon`
    FOREIGN KEY (`toon_id`) REFERENCES `toon` (`id`)
    ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- email/realname PG: continuare su `legacy` (name PK = nome PG) oppure prefs.
-- Non duplicare user.email (account) con specials.email (PG).

-- -----------------------------------------------------------------------------
-- 8. Migrazione opzionale toonExtra → character_prefs
-- -----------------------------------------------------------------------------
-- La tabella `toonExtra` attuale NON ha toon_id e non è usata dal codice.
-- Opzione A: lasciare toonExtra e ignorarla fino a drop manuale
-- Opzione B: dopo backfill, DROP TABLE toonExtra;
--
-- Esempio backfill (solo se in futuro popolata con convenzione field='toon:42:prompt'):
-- INSERT INTO character_prefs (toon_id, pref_key, pref_value)
-- SELECT … FROM toonExtra …;

-- -----------------------------------------------------------------------------
-- 9. Vista di comodo (debug)
-- -----------------------------------------------------------------------------
CREATE OR REPLACE VIEW `v_character_summary` AS
SELECT
  t.id            AS toon_id,
  t.name          AS toon_name,
  t.owner_id      AS account_id,
  u.email         AS account_email,
  t.title,
  t.level         AS toon_best_level,
  cc.race,
  cc.alignment,
  cs.hit,
  cs.max_hit,
  cs.gold,
  cs.exp
FROM `toon` t
LEFT JOIN `user` u ON u.id = t.owner_id
LEFT JOIN `character_core` cc ON cc.toon_id = t.id
LEFT JOIN `character_stats` cs ON cs.toon_id = t.id;

SET FOREIGN_KEY_CHECKS = 1;

-- -----------------------------------------------------------------------------
-- 10. Apply incrementale (DB già creato senza character_resistance)
-- -----------------------------------------------------------------------------
-- Vedi: docs/schema-s1-ddl-add-resistance.sql

-- =============================================================================
-- Dopo apply manuale:
-- 1. Registrare modello ODB "character" (src/odb/character.hpp) — non fatto qui
-- 2. legacy_loader.cpp — import da .dat / rent / .aux
-- 3. Adapter S2 — char_data ↔ tabelle sopra
-- 4. USE_DB_LOAD=OFF fino a test completi
-- =============================================================================
