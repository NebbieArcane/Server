# S1 — Schema mapping (solo dev)

Documento di allineamento dati per la migrazione DB. Fonte di verità per i **tipi colonne**: `char_file_u` e formati file correlati (immutabili).  
Bridge attuale: `store_to_char` / `char_to_store` in `src/db.cpp`. Extra: `load_char_extra` / `write_char_extra` in `src/reception.cpp` (file `RENT_DIR/<name>.aux`).

**Stato:** bozza iniziale — decisioni segnate come *proposta* finché non c’è review con Dev B.

---

## Legenda colonne

| Colonna | Significato |
|---------|-------------|
| **Sorgente file** | Campo in `char_file_u` o altro file binario/testo |
| **Runtime** | Dove finisce in `char_data` dopo load |
| **Persistenza oggi** | `.dat` / `.aux` / rent / ODB `toon` |
| **Tabella S1 (proposta)** | Target MySQL/ODB nuovo modello |
| **SQL tipo (proposta)** | Da `char_file_u`, non da C++17 futuro |
| **Note** | Trasformazioni, gap, non salvato |

---

## 0. Situazione attuale (già in codice)

- **Save/load personaggio:** `lib/players/<name>.dat` → `struct char_file_u` (sizeof fisso, **non modificare**).
- **Extra + achievements + poof:** `lib/rent/<name>.aux` (formato `tag:value` per riga).
- **Inventario rent:** `lib/rent/<name>` → `obj_file_u` (non in `.dat`).
- **ODB parziale:** in `save_char()` esiste già sync verso `toon` (password, title, level) se il record esiste — vedi `src/db.cpp` blocco «sincronizzazione DB». Non copre stats/skills/affects.

---

## 1. `character_core` (proposta)

Una riga per personaggio. PK: `id` (BIGINT), unique su `name` (VARCHAR 20).

| Sorgente file (`char_file_u`) | Runtime | Persistenza oggi | Tabella | SQL (proposta) | Note |
|------------------------------|---------|------------------|---------|----------------|------|
| `name[20]` | `player.name` | `.dat` | `character_core` | `VARCHAR(20)` PK logico | |
| `pwd[11]` | `desc->pwd` (solo save) | `.dat` + `toon.password` | `character_core` | `VARCHAR(11)` | Save da `ch->desc->pwd`, non da struct file in load |
| `title[80]` | `player.title` (heap) | `.dat` + `toon.title` | `character_core` | `VARCHAR(80)` | |
| `description[240]` | `player.description` | `.dat` | `character_core` | `VARCHAR(240)` NULL | |
| `extra_str[255]` | — | `.dat` | `character_core` | `VARCHAR(255)` | Verificare uso in game; in mapping store non visto |
| `iClass` | `player.iClass` | `.dat` | `character_core` | `INT` | |
| `sex` | `player.sex` | `.dat` | `character_core` | `TINYINT UNSIGNED` | |
| `race` | `GET_RACE(ch)` | `.dat` | `character_core` | `INT` | |
| `level[ABS_MAX_CLASS]` (20) | `player.level[]` | `.dat` | `character_classes` | vedi §2 | Normalizzare: una riga per indice classe con `level > 0` |
| `birth` | `player.time.birth` | `.dat` | `character_core` | `INT UNSIGNED` | epoch seconds |
| `played` | `player.time.played` | `.dat` | `character_core` | `INT` | |
| `last_logon` | aggiornato in save | `.dat` | `character_core` | `INT UNSIGNED` | |
| — | `player.time.logon` | runtime only | — | — | Non in file |
| `weight`, `height` | `player.weight/height` | `.dat` | `character_core` | `INT UNSIGNED` | |
| `hometown` | `player.hometown` | `.dat` | `character_core` | `SMALLINT` | |
| `talks[MAX_TOUNGE]` (3) | `player.talks[]` | `.dat` | `character_prefs` | JSON o 3× BOOL | *proposta:* tabella prefs |
| `speaks` | `player.speaks` | `.dat` | `character_core` | `INT` | |
| `user_flags` | `player.user_flags` | `.dat` | `character_core` | `INT` | |
| `extra_flags` | `player.extra_flags` | `.dat` | `character_core` | `INT` | |
| `agemod` | `AgeModifier` | `.dat` | `character_core` | `INT` | |
| `authcode[7]` | `specials.authcode` | `.dat` (load); anche ODB account flow | `character_core` | `VARCHAR(7)` | GGPATCH |
| `WimpyLevel[4]` | `specials.WimpyLevel` (int) | `.dat` | `character_core` | `SMALLINT` | File: stringa `%03d`; load: `atoi` |
| `dummy[19]` | — | `.dat` | — | — | Riservato; save riempie `"123456789012345678"` |
| `load_room` | `in_room` (con hack +65536) | `.dat` (solo save) | `character_core` | `SMALLINT` | Load: se `< -2` → `+ 65536` |
| `startroom` | `specials.start_room` | `.dat` | `character_core` | `INT` | |
| `spells_to_learn` | `specials.spells_to_learn` | `.dat` | `character_core` | `TINYINT` | |
| `alignment` | `specials.alignment` | `.dat` | `character_core` | `INT` | |
| `act` | `specials.act` | `.dat` | `character_core` | `INT UNSIGNED` | PLR_* flags |
| `affected_by` | `specials.affected_by` | `.dat` | `character_core` | `INT UNSIGNED` | bitvector permanente |
| `affected_by2` | `specials.affected_by2` | `.dat` | `character_core` | `INT UNSIGNED` | |
| `conditions[MAX_CONDITIONS]` (5 in file, 3 usati in loop) | `GET_COND` / `specials.conditions` | `.dat` | `character_core` | `TINYINT` ×5 o JSON | Save/load loop `0..2` oggi |
| `apply_saving_throw[MAX_SAVES]` (8) | `specials.apply_saving_throw` | `.dat` | `character_core` | `SMALLINT` ×8 o tabella | Load azzera poi save ripristina |

---

## 2. `character_classes` (proposta)

`level[20]` in file → tabella figlia.

| Campo file | SQL (proposta) | Note |
|------------|----------------|------|
| `level[i]` | `class_index TINYINT`, `level TINYINT UNSIGNED` | Solo indici `MAGE_LEVEL_IND..MAX_CLASS-1` copiati in store; resto 0 |

---

## 3. `character_stats` (proposta)

Blocco `char_ability_data` + `char_point_data` (copia bulk in `store_to_char`).

### 3.1 Abilities (`st.abilities` → `ch.abilities`)

| Campo | Tipo C | SQL (proposta) |
|-------|--------|----------------|
| `str`, `str_add`, `intel`, `wis`, `dex`, `con`, `chr`, `extra`, `extra2` | `sbyte` | `TINYINT` |

### 3.2 Points (`st.points` → `ch.points`)

| Campo | Tipo C | SQL (proposta) | Note |
|-------|--------|----------------|------|
| `mana`, `max_mana`, `mana_gain` | sh_int / ubyte | `SMALLINT`, `SMALLINT`, `TINYINT` | |
| `hit`, `max_hit`, `hit_gain` | idem | idem | |
| `move`, `max_move`, `move_gain` | idem | idem | |
| `pRuneDei` | `ush_int` | `SMALLINT UNSIGNED` | commento: rune dei |
| `extra1`, `extra2`, `extra3` | sh_int / ubyte | `SMALLINT`, `SMALLINT`, `TINYINT` | |
| `armor`, `gold`, `bankgold` | sh_int / int | `SMALLINT`, `INT`, `INT` | |
| `exp`, `true_exp`, `extra_dual` | int | `INT`, `INT`, `INT` | |
| `hitroll`, `damroll` | sbyte | `TINYINT` | **Azzerati** in save su file (`armor=100`, roll=0) |
| `libero` | sbyte | `TINYINT` | |

*Proposta:* una riga `character_stats` con tutte le colonne sopra + FK `character_id`.

---

## 4. `character_skills` (proposta)

Array fisso `skills[MAX_SKILLS]` (350).

| Campo `char_skill_data` | SQL (proposta) | Note |
|-------------------------|----------------|------|
| `skill_id` (indice 0..349) | `SMALLINT` PK parte | |
| `learned` | `TINYINT UNSIGNED` | Cap in load: 100 / 81 / 86 / 95 |
| `flags`, `special`, `nummem` | `TINYINT` ×3 | |

*Proposta:* salvare solo righe con `flags != 0` o `learned > 0` per ridurre volume.

---

## 5. `character_affects` (proposta)

Array fisso `affected[MAX_AFFECT]` (40), tipo `affected_type_u` (no puntatori).

| Campo | SQL (proposta) | Note |
|-------|----------------|------|
| `slot` (0..39) | `TINYINT` | |
| `type` | `SMALLINT` | 0 = slot vuoto |
| `duration` | `SMALLINT` | |
| `modifier` | `INT` | |
| `location` | `INT` | APPLY_* |
| `bitvector` | `INT` | |
| `next` | — | sempre 0 in file; ignorare in DB |

Runtime usa lista `ch->affected`; save **unequip + strip affects** temporaneamente, poi ripristina.

Affect con `location` 26/27/28 (`APPLY_IMMUNE` / `APPLY_SUSC` / `APPLY_M_IMMUNE`) restano per buff **temporanei** fino al refactor resistenze.

---

## 5b. `character_resistance` — scala -100..+100 (proposta)

Sostituisce a regime il tri-stato bitvector (`immune`, `M_immune`, `susc` su `char_data`).  
**Non** in `char_file_u` oggi; **non** colonne su `character_core`.

| Colonna SQL | Tipo | Note |
|-------------|------|------|
| `toon_id` | BIGINT UNSIGNED PK (con damage_type) | FK → `toon.id` |
| `damage_type` | INT UNSIGNED | Valore bit `IMM_*` (`autoenums.hpp`) |
| `value` | SMALLINT | `-100` … `+100` |

| `value` | Significato |
|---------|-------------|
| `-100` | Suscettibile (danni raddoppiati) |
| `0` | Normale |
| `+1`…`+99` | Resistente (gradiente) |
| `+100` | Immune |

**Migrazione da bitvector:** vedi `docs/resistance-bit-to-value.md`.

| Sorgente runtime oggi | Tabella S1 |
|----------------------|------------|
| `ch->immune`, `ch->M_immune`, `ch->susc` (RAM; mob da `.mob`) | `character_resistance` (una riga per tipo) |
| Affect 26/27/28 su `.dat` | Transizione: ancora `character_affects`; poi modificano `value` |

**Fisico vs magico:** stessa tabella; distinzione per `damage_type` (BLUNT/PIERCE/SLASH = fisico; FIRE/COLD/… = magico) — tabella in `resistance-bit-to-value.md`.

**Import iniziale senza bit in file:** tutte le righe per ogni `IMM_*` usato → `value = 0`, oppure conversione da affect/mob al primo save.

---

## 6. Inventario ed equip — `obj_file_u` (proposta)

**File:** `RENT_DIR/<name>` (non `.dat`). Struct: `obj_file_u` + `obj_file_elem objects[MAX_OBJ_SAVE]` (200).

| Campo `obj_file_u` | SQL (proposta) | Note |
|--------------------|----------------|------|
| `owner[20]` | FK su `name` | |
| `gold_left` | `INT` | |
| `total_cost` | `INT` | |
| `last_update` | `INT UNSIGNED` | |
| `minimum_stay` | `INT` | |
| `number` | `INT` | conteggio oggetti validi |

Per ogni `obj_file_elem` → `character_inventory` (o `rent_objects`):

| Campo | SQL (proposta) |
|-------|----------------|
| `item_number` | `SMALLINT UNSIGNED` |
| `value[4]` | 4× `INT` |
| `extra_flags`, `extra_flags2` | `INT` |
| `weight`, `timer` | `INT` |
| `bitvector` | `INT UNSIGNED` |
| `name`, `sd`, `desc` | `VARCHAR(128)`, `VARCHAR(128)`, `VARCHAR(256)` |
| `wearpos`, `depth` | `TINYINT` |
| `affected[MAX_OBJ_AFFECT]` | tabella figlia o JSON |

Equip indossato: oggi gestito via rent file + `depth`/`wearpos`; non è in `char_file_u`.

---

## 7. Dati solo in `.aux` (S1f — proposta)

Caricati da `load_char_extra`, scritti da `write_char_extra`. **Non** in `char_file_u`.

| Tag `.aux` | Runtime | Tabella (proposta) | Note |
|------------|---------|-------------------|------|
| `in:` | `specials.poofin` | `character_prefs` o `toonExtra` | Solo immortali in write |
| `out:` | `specials.poofout` | idem | |
| `prompt:` | `specials.prompt` | `character_prefs` | |
| `email:` | `specials.email` | `legacy` / `user` email | Overlap ODB account |
| `realname:` | `specials.realname` | `legacy.realname` | Esiste classe ODB `legacy` |
| `principe:` | `GET_PRINCE(ch)` | `character_prefs` | |
| `version:` | `specials.lastversion` | `character_prefs` | Salvato come `version()` server |
| `zone:` | `GET_ZONE(ch)` | `character_prefs` | Immortali |
| `achie_racekill:` | `achievements[RACESLAYER][n]` | `character_achievements` | formato `n#value` |
| `achie_bosskill:` | idem BOSSKILL | idem | |
| `achie_class:` | CLASS_ACHIE | idem | |
| `achie_quest:` | QUEST_ACHIE | idem | |
| `achie_other:` | OTHER_ACHIE | idem | |
| `mercy:` | `specials.mercy[n]` | `character_quests` o achievements | |
| `0:`..`9:` | `Alias` list | `character_aliases` | `n: testo` |

### 7.1 Runtime **non** persistito (o perso al reboot)

Da tenere fuori schema S1 o marcare *session-only*:

| Runtime | Note |
|---------|------|
| `specials.quest_mob[][]` | Usato in game; **non** trovato in `write_char_extra` — probabile perdita |
| `poofin/out/email/...` | Azzerati in `store_to_char` da file; ripopolati solo da `.aux` |
| `group_name` | `char_to_store`: «do not store group_name» |
| Puntatori combattimento, mount, edit, ecc. | Session/NPC |
| `immune` / `M_immune` / `susc` (PG) | Non in `.dat`; → `character_resistance` al save/migrazione |

*Proposta solo:* `quest_mob` → `character_quest_progress` se si decide di persistere (oggi gap).

---

## 8. Mapping ODB esistente (`src/odb/account.hpp`)

| ODB attuale | Overlap con file | Azione S1 |
|-------------|------------------|-----------|
| `toon` (name, password, title, level, lastlogin, lasthost, owner_id) | parziale `.dat` | Estendere o affiancare `character_core`; non duplicare senza adapter |
| `toonExtra` (field/value) | candidato per prefs `.aux` | *proposta:* mappare `prompt`, alias, ecc. come key/value |
| `toonRent` | rent effects? | Verificare vs `obj_file_u` |
| `user` / `legacy` | account/email | Fuori scope personaggio in-game |

---

## 9. Decisioni da chiudere in S1 (checklist solo)

- [ ] **S1a** — Confermare tipi SQL sopra (o correggere dopo `utilities/sizes.cpp` su target arch)
- [ ] **S1b** — Nomi tabelle finali
- [ ] **S1c** — Dual write: già parzialmente vero (file + sync `toon`)
- [ ] **S1d** — Ordine lavoro: questo doc → `legacy_loader` → ODB `character.hpp`
- [ ] **S1e** — `USE_DB_LOAD` default OFF
- [ ] **S1f** — `.aux` → `toonExtra` vs tabelle dedicate vs ibrido
- [ ] **quest_mob** — bug/feature: persistere o no?
- [x] **resistenze** — `character_resistance` + `resistance-bit-to-value.md` (scala -100..+100)

---

## 10. Ordine consigliato (un solo dev)

1. **Questo file** — rivedere e spuntare checklist §9  
2. **`docs/schema-s1-ddl-draft.sql`** — generare DDL da §1–§7  
3. **`legacy_loader.cpp`** — lettura `.dat` / `obj_file_u` / `.aux` senza write  
4. **ODB** `src/odb/character.hpp` — record separati, nessun `#pragma` su `char_data`  
5. **Adapter** (S2) — `CharacterRecord` ↔ `char_data` dopo eventuale C++17 su struct di gioco  

---

## Riferimenti codice

| Funzione | File |
|----------|------|
| `store_to_char` / `char_to_store` | `src/db.cpp` ~2850–3267 |
| `save_char` (+ sync `toon`) | `src/db.cpp` ~3271–3354 |
| `load_char_extra` / `write_char_extra` | `src/reception.cpp` ~1946–2187 |
| `load_char_objs` | `src/reception.cpp` ~703+ |
| `struct char_file_u` | `src/structs.hpp` ~879–919 |

*Generato in S1 — aggiornare quando cambiano costanti in `autoenums.hpp` (MAX_SKILLS, MAX_AFFECT, ecc.).*
