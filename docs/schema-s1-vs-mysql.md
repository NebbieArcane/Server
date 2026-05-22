# Confronto: MySQL attuale vs mapping S1

Riferimenti:
- **MySQL oggi:** `src/odb/account-mysql.xml`, modello ODB `"account"` v1, DB `nebbie`
- **Target S1:** `docs/schema-s1-mapping.md` (§1–§8)

Legenda stato:

| Stato | Significato |
|-------|-------------|
| ✅ **Coperto** | Campo/dato già in MySQL (anche parziale) |
| 🔶 **Parziale** | Solo una frazione del dato S1; tipi o semantica diverse |
| ❌ **Mancante** | Non in MySQL; solo file o runtime |
| 📦 **Tabella vuota** | Tabella esiste ma zero uso in `.cpp` |
| ➡️ **Estendere** | Convien estendere tabella ODB esistente |
| 🆕 **Nuova** | Nuova tabella / modello ODB |

---

## Panoramica

| Area | MySQL oggi | Mapping S1 | Gap |
|------|------------|------------|-----|
| Account login | `user` | (fuori scope §1–6) | OK, resta separato |
| Registro PG + link account | `toon` | `character_core` + FK | ~5 colonne su ~35 campi core |
| Anagrafica storica | `legacy` | overlap `.aux` / email | Parziale, PK per `name` non per `toon.id` |
| Extra key/value | `toonExtra` 📦 | `character_prefs` / §7 | Schema ok, **nessun dato** |
| Rent oggetti | `toonRent` 📦 | `character_inventory` §6 | Modello **non** compatibile con `obj_file_u` |
| Gameplay `.dat` | — | §1–§5 | **100% su file** |
| Gameplay `.aux` | — | §7 | **100% su file** |
| Inventario rent file | — | §6 | **100% su file** |

**Conclusione:** MySQL è un layer **account/registry**; S1 aggiunge un layer **game state** che oggi non esiste.

---

## 1. `toon` vs `character_core` (§1)

### Colonne `toon` attuali

| Colonna MySQL | Tipo | Uso codice |
|---------------|------|------------|
| `id` | BIGINT UNSIGNED PK | FK interno, `owner_id` su altri PG |
| `name` | VARCHAR(32) UNIQUE | Login, lookup, sync |
| `password` | VARCHAR(128) | Da `.dat`/`desc` in save; login |
| `title` | VARCHAR(128) | Sync da `save_char` |
| `level` | SMALLINT UNSIGNED | **Solo** `BestClassIND` — un numero, non `level[20]` |
| `lastlogin` | DATETIME NULL | Non sempre aggiornato in ogni path |
| `lasthost` | VARCHAR(255) | Registrazione / login |
| `owner_id` | BIGINT UNSIGNED | → `user.id` |

### Mapping campo-per-campo (`char_file_u` → S1)

| Campo S1 / `char_file_u` | `toon` oggi | Stato | Azione consigliata |
|--------------------------|-------------|-------|-------------------|
| `name` | `name` | ✅ | Mantenere; allineare lunghezza S1 `VARCHAR(20)` vs DB `32` |
| `pwd` | `password` | 🔶 | File max 11; DB 128 — OK per hash futuro |
| `title` | `title` | ✅ | File max 80; DB 128 — OK |
| `description` | — | ❌ | 🆕 `character_core` o colonna su estensione |
| `extra_str` | — | ❌ | 🆕 dopo verifica uso |
| `iClass`, `sex`, `race` | — | ❌ | 🆕 `character_core` |
| `level[20]` | `level` (singolo) | 🔶 | 🆕 `character_classes`; **non** sovrascrivere semantica `toon.level` senza adapter |
| `birth`, `played`, `last_logon` | — | ❌ | 🆕; `last_logon` ≠ `lastlogin` (tipi: INT epoch vs DATETIME) |
| `weight`, `height`, `hometown` | — | ❌ | 🆕 |
| `talks`, `speaks`, `user_flags`, `extra_flags`, `agemod` | — | ❌ | 🆕 / `character_prefs` |
| `authcode` | — | ❌ | 🆕 |
| `WimpyLevel` | — | ❌ | 🆕 |
| `load_room`, `startroom` | — | ❌ | 🆕 |
| `spells_to_learn`, `alignment`, `act` | — | ❌ | 🆕 |
| `affected_by`, `affected_by2` | — | ❌ | 🆕 |
| `conditions`, `apply_saving_throw` | — | ❌ | 🆕 |
| `owner_id` (account) | `owner_id` | ✅ | Resta su `toon`; `character_core` FK → `toon.id` |

### Decisione architetturale

| Opzione | Pro | Contro |
|---------|-----|--------|
| **A — Estendere `toon`** | Meno tabelle; login già su `toon` | Mescola account-registry con 30+ colonne gameplay; migrazione ODB pesante |
| **B — `character_core` + FK `toon_id`** ✅ | Separazione chiara; `toon` resta sottile | Una join in più in adapter S2 |
| **C — Solo nuove tabelle, deprecare `toon`** | — | Rompe tutto `interpreter.cpp` / `register` |

**Raccomandazione S1:** **B** — lasciare `toon` com’è per login/registrazione; aggiungere `character_core` (1:1 con `toon.id` o `toon.name` UNIQUE).

---

## 2. `character_classes` (§2)

| S1 | MySQL | Stato |
|----|-------|-------|
| Tabella figlia `level[class_index]` | — | ❌ 🆕 |

`toon.level` non sostituisce questa tabella (è solo il livello della classe “migliore”).

---

## 3. `character_stats` (§3)

| Blocco S1 | Campi | MySQL | Stato |
|-----------|-------|-------|-------|
| `char_ability_data` (9× sbyte) | str, dex, … | — | ❌ 🆕 |
| `char_point_data` (~20 campi) | mana, hit, gold, exp, … | — | ❌ 🆕 |

Nessuna colonna in `toon` / `toonExtra` copre stats. Tutto in `.dat` oggi.

---

## 4. `character_skills` (§4)

| S1 | MySQL | Stato |
|----|-------|-------|
| fino a 350 righe × (learned, flags, special, nummem) | — | ❌ 🆕 |

Volume stimato: fino a 350 righe/PG se si salva tutto; mapping propone solo righe non vuote.

---

## 5. `character_affects` (§5)

| S1 | MySQL | Stato |
|----|-------|-------|
| fino a 40 affect attivi | — | ❌ 🆕 |

Nota: `affected_by` / `affected_by2` in §1 sono **bitvector permanenti** su `character_core`, non la lista affect temporanei di §5.

---

## 6. Inventario — `toonRent` vs `obj_file_u` (§6)

### `toonRent` (MySQL, 📦 non usata)

| Colonna | Semantica DB | `obj_file_u` / rent file |
|---------|--------------|---------------------------|
| `id` | PK auto | — |
| `vnum` | BIGINT | `obj_file_elem.item_number` (ush_int) |
| `effects` | VARCHAR(10240) blob testo? | Non corrisponde a struct binaria oggetto |
| `bank` | BOOL | Non mappato 1:1 |

### Rent file (realtà oggi)

| Dato | File | MySQL |
|------|------|-------|
| Header rent (`gold_left`, `total_cost`, `last_update`, …) | `RENT_DIR/<name>` | ❌ |
| Oggetti (fino a 200), nested `depth` | stesso file | ❌ |
| Extra poof/achie (overlap) | `.aux` | ❌ / parziale `toonExtra` |

**Verdetto:** `toonRent` **non** è un mapping di `obj_file_u`. Per S1:

- 🆕 tabelle `rent_header` + `character_inventory` (come in mapping), **oppure**
- riprogettare `toonRent` e migrare (sconsigliato: nome/colonne fuorvianti)

---

## 7. `.aux` — `toonExtra` vs §7

### `toonExtra` (MySQL)

| Colonna | Proposta S1 equivalente |
|---------|-------------------------|
| `id` | PK riga (manca FK esplicita a personaggio in schema ODB!) |
| `field` VARCHAR(32) | chiave tag (`prompt`, `in`, …) |
| `value` VARCHAR(1024) | valore |

**Problema schema attuale:** `toonExtra.id` è AUTO_INCREMENT della **riga**, non `toon.id`. L’indice `(id, field)` non lega il PG. Per S1 serve **`toon_id` BIGINT** (o `character_id`) + unique `(toon_id, field)`.

### Tag `.aux` vs destinazione

| Tag `.aux` | `toonExtra`? | `legacy`? | `user`? | S1 proposta |
|------------|--------------|-----------|---------|-------------|
| `in:` / `out:` | 🔶 possibile | — | — | `character_prefs` o `toonExtra` fix FK |
| `prompt:` | 🔶 | — | — | idem |
| `email:` | — | 🔶 `email1` | 🔶 `email` account | Separare: account email ≠ PG email |
| `realname:` | — | ✅ `realname` | — | `legacy` già ok per nome PG |
| `principe:` | 🔶 | — | — | 🆕 prefs |
| `version:` | 🔶 | — | — | prefs |
| `zone:` | 🔶 | — | — | prefs (immortali) |
| `achie_*` / `mercy:` | ❌ (troppo strutturato per KV) | — | — | 🆕 `character_achievements` |
| `0:`–`9:` alias | 🔶 | — | — | 🆕 `character_aliases` o `toonExtra` |

**Uso codice:** nessun `Sql::save<toonExtra>` — tabella **sempre vuota** in pratica.

**Raccomandazione S1f:**

| Dato | Dove |
|------|------|
| poof, prompt, zone, version, principe | `toonExtra` **con FK `toon_id`** oppure `character_prefs` |
| achievements | tabella normalizzata (non KV) |
| alias | `character_aliases` (indice 0–9) |
| email/realname PG | `legacy` + eventuale sync da `.aux` |

---

## 8. `user` e `legacy` vs S1

### `user` (account)

| Ruolo | Overlap S1 |
|-------|------------|
| Login email/password, livello immortale account, PTR | **Nessun** campo in `char_file_u` |
| `owner_id` su `toon` | Collegamento account ↔ PG |

**Azione:** **non toccare** in S1 gameplay; resta modello `"account"` separato.

### `legacy`

| Colonna | `.aux` / file |
|---------|---------------|
| `name` PK | nome PG |
| `realname` | tag `realname:` |
| `email1`, `email2` | tag `email:` (parziale) |

**Azione:** tenere per migrazione storica; nuovi dati `.aux` possono aggiornare `legacy` via adapter, oppure spostarsi su `character_prefs` a lungo termine.

---

## 9. Matrice tabelle: esiste / da creare

| Tabella S1 (proposta) | Esiste in MySQL? | Relazione con esistente |
|----------------------|------------------|-------------------------|
| `character_core` | ❌ | FK → `toon.id` (1:1) |
| `character_classes` | ❌ | FK → `toon.id` |
| `character_stats` | ❌ | FK → `toon.id` |
| `character_skills` | ❌ | FK → `toon.id` |
| `character_affects` | ❌ | FK → `toon.id` |
| `character_resistance` | ❌ | FK → `toon.id`; scala -100..+100; vedi `resistance-bit-to-value.md` |
| `character_inventory` / rent header | ❌ | sostituisce concetto `toonRent` |
| `character_achievements` | ❌ | da `.aux` |
| `character_aliases` | ❌ | da `.aux` |
| `character_prefs` | ❌ | oppure `toonExtra` rifatto |
| `character_quest_progress` | ❌ | opzionale (`quest_mob`) |
| `toon` | ✅ | resta |
| `user` | ✅ | resta |
| `legacy` | ✅ | resta |
| `toonExtra` | ✅ 📦 | **rifare FK** o sostituire |
| `toonRent` | ✅ 📦 | **non riusare** per §6 |
| `schema_version` | ✅ | ODB; nuovo modello `"character"` v1 in futuro |

---

## 10. Sync codice oggi vs copertura S1

| Operazione | Cosa aggiorna MySQL | Cosa NON aggiorna |
|------------|---------------------|-------------------|
| `save_char` | `toon`: password, title, level | tutto §3–§6, `.aux` (separate fn) |
| `con_register` / login | `toon`, `user` | gameplay |
| `write_char_extra` | — (solo file `.aux`) | MySQL |
| `load_char` | — (solo `.dat`) | MySQL |

**Dual write S1c:** oggi è **file + toon parziale**, non “file + game state DB”.

---

## 11. Checklist §9 aggiornata dopo confronto

| ID | Decisione | Proposta post-confronto |
|----|-----------|-------------------------|
| S1a | Tipi SQL | Confermati; attenzione `last_logon` INT vs `lastlogin` DATETIME |
| S1b | Nomi tabelle | Nuovo modello ODB `"character"` separato da `"account"` |
| S1c | Dual write | Estendere sync: file pieno + `character_*`; `toon` resta leggero |
| S1d | Ordine | mapping ✅ → **questo doc** → ddl-draft → legacy_loader |
| S1e | Feature flag | `USE_DB_LOAD=OFF` fino a adapter |
| S1f | `.aux` | Achievements/alias → tabelle; poof/prompt → `toonExtra` con **`toon_id`** |
| quest_mob | Persistenza | ❌ oggi; 🆕 tabella se si vuole fix bug |

---

## 12. Prossimo passo

- [x] `docs/schema-s1-ddl-draft.sql` — bozza CREATE (+ `character_resistance`)
- [x] `docs/resistance-bit-to-value.md` — migrazione bit → -100..+100
- [x] `docs/schema-s1-ddl-add-resistance.sql` — apply solo tabella resistenze
- [ ] `legacy_loader.cpp` — lettura file senza write
- [ ] `src/odb/character.hpp` — modello ODB `"character"`

---

*Generato da confronto schema account v1 vs schema-s1-mapping.md*
