-- Apply incrementale: rimuove character_core.extra_str (slot legacy solo nel .dat).
-- Il mud non usa extra_str in runtime; non serve in MySQL per S1.
--
-- mysql -h 127.0.0.1 -u root -psecret nebbie < docs/schema-s1-ddl-drop-extra-str.sql

SET NAMES utf8mb4;

ALTER TABLE `character_core`
  DROP COLUMN `extra_str`;
