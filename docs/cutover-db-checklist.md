# Cutover DB — checklist operativa

Checklist per il **taglio netto** da file (`.dat` / rent / `.aux`) a MySQL come unica fonte di verità in runtime.

**Strategia target:** login **B** (load da DB) + gate **C2** (`toon.migrated_at`) + save **D2** (niente scrittura file per PG migrati). I file restano solo come **backup** e per restore manuale.

**Riferimenti:**

- Mapping campi: [schema-s1-mapping.md](schema-s1-mapping.md) (`.dead` escluso — vedi sezione dedicata sotto)
- DDL: [schema-s1-ddl-draft.sql](schema-s1-ddl-draft.sql) (+ script incrementali: resistance, condition, `extra_str`, classi)
- Import file → DB: [legacy-loader.md](legacy-loader.md), `src/legacy_import.cpp`
- Stato schema vs codice: [schema-s1-vs-mysql.md](schema-s1-vs-mysql.md)

---

## Legenda

| Simbolo | Significato |
|---------|-------------|
| ✅ | Esiste e usabile oggi |
| 🔶 | Parziale / da completare |
| ❌ | Mancante |
| 🚫 | Da disabilitare al cutover |
| ☐ | Da fare (ops o sviluppo) |

---

## Architettura scelta (riepilogo)

| Asse | Scelta | Note |
|------|--------|------|
| Login | **B** | Dopo migrate: solo `load_*_from_db`; niente fallback automatico su file |
| Gate migrazione | **C2** | `toon.migrated_at` (+ opz. `schema_version`); non inferire da sola presenza di `character_core` |
| Save | **D2** | PG con `migrated_at` set: niente `fwrite` `.dat`, niente `rent/` / `.aux` in scrittura |
| Transizione | **Nessuna fase lunga A** | Batch + backup prima del deploy; file = archivio |

**Rollback:** binario precedente + file backup intatti (se D2 non ha riscritto i file). Il DB migrato può restare; il vecchio codice rilegge i file.

---

## `.dat` e `toon` — chiarimento

Sono due layer diversi:

| Cosa | Dove | Ruolo |
|------|------|--------|
| `.dat` | `lib/players/<nome>.dat` | Salvataggio storico gameplay (stats, skill, exp, …) |
| `toon` | MySQL | Registro PG (nome, password, title, level, `owner_id` → account) |

### Il `toon` si crea da solo?

**Sì, in alcuni casi — ma non al batch import e non al menu “entra in gioco”.**

| Momento | Crea `toon`? |
|---------|----------------|
| PG **nuovo** (`justCreated` in `con_register`) | **Sì** — `Sql::save` + poi `.dat` |
| Login: scelta **nome** e esiste `.dat` ma no `toon` | **Sì** — `toonFromFileSystem` → `getFromDb` (nome, pwd, title dal file) |
| `con_pwdok` / load da file | **No** |
| Menu `'1'` (entra in gioco) | **No** |
| `legacyimport` / batch a mud spento | **No** — serve già un `toon` o un passo che chiama `getFromDb` |
| Boot con `forceDbInit` (solo primo schema DB vuoto) | **Sì** per tutti i `.dat` — caso raro |

`getFromDb` (`db.cpp`) inserisce un record minimo se manca: `name`, `password`, `title` dal `.dat`. `owner_id` può restare 0 finché l’account non associa il PG in `con_register`.

### Implicazione per il batch pre-cutover

Prima di `legacyimport` su tutti i PG:

1. Per ogni `players/*.dat` da migrare: esiste `toon` con lo stesso nome **oppure**
2. Eseguire un passo che replica `getFromDb` (script o tool) **senza** richiedere il login di ogni giocatore.

PG mai più loggati possono avere solo il file finché non si fa (1) o (2).

---

## File sidecar `.dead` (DEATH_FIX)

Sidecar testuale in `lib/players/<nome>.dead` — **non** è un secondo `.dat` né uno “PG morto”.

| Campo file | Contenuto |
|------------|-----------|
| Riga unica | `saved_exp : saved_at` (INT exp + epoch) |

| Momento | Comportamento codice |
|---------|----------------------|
| Morte PC (senza loss livello) | `save_exp_to_file` → riscrive `.dead` |
| Nuovo PG (`roll_abilities`) | Crea `.dead` iniziale (exp=1) |
| XpMaster (compra stat) | Aggiorna `.dead` con exp corrente |
| Sacrificio (`sacrifice`) | Legge `.dead` (max ~4 h), ripristina `GET_EXP` |
| Resurrezione / reincarnazione | Legge `.dead` per exp su `.dat` |
| Nuke PG | `rm` di `<nome>.dead` |

### Nel database S1?

| Domanda | Risposta |
|---------|----------|
| Tabella `character_*` prevista? | **No** — exp “ufficiale” resta in `character_stats.exp` (da `.dat`) |
| `legacyimport` importa `.dead`? | **No** |
| Ha senso una tabella al cutover? | **No** (fase 1) — meccanica file; basso valore per batch/report |
| Dopo D2 (PG migrato, no `.dat`)? | **Resta file** finché morte/sacrificio/resurrezione non vengono riscritti su DB |

**Scelta documentata:** `.dead` = **solo filesystem**, come oggi. Opzionale in futuro: `character_death_snapshot (toon_id, saved_exp, saved_at)` se si elimina del tutto il sidecar file.

### Implicazioni cutover

| Fase | Cosa fare |
|------|-----------|
| Backup (0.1) | Includere `players/*.dead` nello snapshot (piccoli, spesso in zip `pg*`) |
| Import batch | Nessuna azione DB; non blocca il GO |
| Load/save DB | Non coprire `.dead`; codice `fight.cpp` / `spec_procs*.cpp` / `magic*.cpp` invariato |
| `do_refund` | Opzionale: copiare anche `./lib/players/<nome>.dead` dal backup pg (non implementato oggi) |
| PG migrato in gioco | `.dead` continua ad aggiornarsi su disco anche con D2 attivo |

---

## 0. Pre-cutover (ops)

| # | Voce | Stato | Note |
|---|------|-------|------|
| 0.1 | Backup `players/` (`.dat` + `.dead`), `rent/`, dump MySQL | ☐ | Snapshot datato; path documentato; `.dead` = sidecar DEATH_FIX |
| 0.2 | Inventario `.dat` vs `toon` | ☐ | Vedi sezione sopra; risolvere orfani prima del batch |
| 0.3 | DDL S1 applicato su DB target | ☐ | `schema-s1-ddl-draft.sql` + `schema-s1-ddl-add-resistance.sql`, `drop-unused-conditions`, `drop-extra-str`, `fix-character-classes` |
| 0.4 | Build con `USE_MYSQL` | ✅ | Verificato da `legacyimport montero` OK |
| 0.5 | `legacy_loader` / `legacy_import` nel link del binario | ✅ | Verificato da `legacyprobe` / `legacyimport` in-game |
| 0.6 | Runbook restore singolo PG | ☐ | Backup file → `migrated_at = NULL` → `legacyimport` |

**Gate 0:** backup verificato + DDL su staging = prod.

---

## 1. Schema e contratto migrazione (C2)

| # | Voce | Stato | Note |
|---|------|-------|------|
| 1.1 | `ALTER TABLE toon` (`migrated_at`, `schema_version`) | 🔶 | SQL: [schema-s1-toon-migration-flags.sql](schema-s1-toon-migration-flags.sql); oppure ODB migrate |
| 1.2 | Campi cutover su `toon` (SQL + struct C++ `transient`) | ✅ | DDL: [schema-s1-toon-migration-flags.sql](schema-s1-toon-migration-flags.sql); ODB account resta v1 (no migrate ODB per questi campi) |
| 1.3 | `legacy_import` setta flag in transazione prima di `commit` | ✅ | `toon_mark_migrated_tx` in `legacy_import.cpp` |
| 1.4 | Helper in `toon_migration.hpp` / `.cpp` | ✅ | fetch SQL + mark/clear/sanity |
| 1.5 | Sanità: `migrated_at` set ma no `character_core` → errore | ✅ | `toon_migration_sanity_check` (da chiamare al login in §5) |

**Gate 1:** almeno un PG test con import OK + `migrated_at` valorizzato.

---

## 2. Import batch (prima del deploy gameplay)

| # | Voce | Stato | Note |
|---|------|-------|------|
| 2.1 | `legacy_import_character_mysql` | ✅ | `.dat` + stats/class/skill/affect/resist + rent → inventory; test OK su Montero; **non** `.dead` |
| 2.2 | `.aux` → `character_prefs` (KV grezzo) | 🔶 | Non equivale al parse di `load_char_extra` |
| 2.3 | `.aux` → `character_achievements` / `aliases` / `mercy` | ❌ | DDL c’è; import cancella tabelle ma non popola queste righe strutturate |
| 2.4 | `quest_mob` / progress quest | ❌ | Gap in mapping |
| 2.5 | Script batch su tutti i `.dat` | ❌ | Oggi: `legacyimport <nome>` in-game |
| 2.6 | Report batch (OK / FAIL / no toon) | ❌ | |
| 2.7 | Probe post-import (campione) | 🔶 | Montero verificato via `legacyprobe`, SQL, `export-legacy-import-csv.py`; da ripetere su campione più ampio |

**Gate 2:**

- 100% PG attivi nel cutover: import OK + `migrated_at` set
- 0 errori su PG che devono entrare
- Se i PG con achievement/alias/mercy in `.aux` sono critici → **2.3 obbligatorio** prima del GO

---

## 3. Load runtime da DB (B)

Funzioni indicative (`db.cpp` o modulo dedicato):

| # | Voce | Stato | Equivalente file oggi |
|---|------|-------|------------------------|
| 3.1 | `load_character_from_db(toon_id, char_data*)` | 🔶 | Implementato MVP come `load_char_mysql(name, char_file_u*)` + `store_to_char` |
| 3.2 | `character_core` + `character_stats` | ✅ | Popolati da `load_char_mysql` |
| 3.3 | `character_classes` | ✅ | Popolate in `level[]` |
| 3.4 | `character_skills` | ✅ | Popolate in `skills[]` |
| 3.5 | `character_affects` + resistenze | 🔶 | `character_affects` caricato; resistenze da tabella non ancora applicate |
| 3.6 | Titolo / password / level da `toon` | 🔶 | Oggi molto da `.dat` / `desc` |
| 3.7 | Kludge `talks[2]`, `load_room` +65536, mana/hit | ❌ | Come `load_char` / `store_to_char` |
| 3.8 | `load_character_inventory_from_db` | ❌ | `load_char_objs` + `ReadObjs` |
| 3.9 | `load_character_extra_from_db` | ❌ | `load_char_extra` (.aux) |
| 3.10 | PG `justCreated` | ☐ | Solo DB, mai file |
| 3.11 | Poly / ghost / impersonate | ☐ | Test espliciti |

**Gate 3:** 3+ PG rappresentativi — import → load solo DB → confronto con attese (pre-cutover).  
Stato: testato con `legacyloadcheck` su `Montero` e `TheProdigy` (parita campi chiave file vs DB).

---

## 4. Save runtime su DB (D2)

| # | Voce | Stato | Note |
|---|------|-------|------|
| 4.1 | `save_character_to_db(ch)` transazionale | ❌ | |
| 4.2 | Aggiorna `toon` (password, title, level, lastlogin, lasthost) | 🔶 | Oggi sync parziale in `save_char` |
| 4.3 | `save_character_inventory_to_db` | ❌ | `update_file` / `WriteObjs` su rent file |
| 4.4 | `save_character_extra_to_db` | ❌ | `write_char_extra` su `.aux` |
| 4.5 | Niente `fwrite` `.dat` se `migrated_at` set | ❌ | |
| 4.6 | Niente scrittura `rent/<name>` / `.aux` se migrato | ❌ | |
| 4.7 | Menu `'1'..'4'`: oggi `load_char_objs` + `save_char` | 🚫 | `interpreter.cpp` ~3293 — riscrive file subito |
| 4.8 | `.dead`: scrittura file **consentita** anche se migrato | ☐ | Sidecar DEATH_FIX; nessuna tabella DB; vedi sezione `.dead` |

**Gate 4:** login → modifica stato/oggetti → quit → re-login identico **solo da DB**; mtime file invariato.

---

## 5. Wiring login / menu

| Punto | Oggi | Dopo cutover |
|-------|------|--------------|
| `con_pwdok` | 🔶 DB-first con fallback | resolve `toon` → se !`migrated_at` lazy import → `load_char_mysql` |
| Fallimento load DB | fallback file con log | temporaneo in staging; da rimuovere prima del cutover definitivo |
| Entrata `'1'`… | `load_char_objs` + `save_char` | load/save inventario + corpo da DB |
| Quit / autorent | `save_char` → file | `save_character_to_db` |
| `do_refund` | restore da zip → file | Runbook: file + reset flag + re-import |

**Gate 5:** ciclo account → pwd → menu 1 → gioco → quit senza scrittura su `players/*.dat`.  
Stato: lazy migration + DB-first login verificati su staging (`TheProdigy` reset DB → import automatico OK → load MySQL OK).

---

## 6. Gap noti (non dimenticare)

| Area | Rischio |
|------|---------|
| `.aux` strutturato | Import in `character_prefs` non alimenta RAM; serve load/save extra o import 2.3 |
| Achievements fuori `.aux` | Verificare se `.achie` è ancora usato |
| `do_refund` | Resta su file; post-cutover = procedura manuale; opz. includere `.dead` nel restore pg |
| `.dead` (DEATH_FIX) | Non in DB; sacrificio/resurrezione leggono ancora `players/<nome>.dead` |
| Nome `.dat` ≠ `toon.name` | Allineare prima del batch |
| `character_core` senza title/pwd | By design su `toon`; load deve unire le fonti |

---

## 7. Test obbligatori (GO / NO-GO)

| # | Test | ☐ |
|---|------|---|
| 7.1 | PG nuovo: solo DB, login/logout | |
| 7.2 | PG legacy batch-migrato: gioco + save + re-login | |
| 7.3 | Rent: equip + pensione | |
| 7.4 | Poly + save | |
| 7.5 | Ghost / reconnect (se usato) | |
| 7.6 | Achievement / alias / mercy (PG con `.aux` ricco) | |
| 7.7 | mtime `.dat` e `rent/*` invariati dopo sessione migrata | |
| 7.8 | Restore drill: 1 PG da backup file + re-import | |
| 7.9 | Morte / sacrificio / resurrect (PG con `.dead` rilevante) | | Solo se usate quelle meccaniche; file sidecar presente |

**GO** se 7.1–7.5 e 7.7 passano; 7.6 se avete PG con `.aux` non banali; 7.9 se il mud usa DEATH_FIX in produzione.

---

## 8. Deploy day (ordine)

1. Manutenzione / mud down  
2. Backup finale (0.1)  
3. Creare `toon` mancanti da `.dat` se serve (sezione `.dat` e `toon`)  
4. Batch `legacyimport` + `migrated_at`  
5. Deploy binario con §3–§5  
6. Smoke 7.1–7.3  
7. Monitor log import / load / SQL  
8. Tenere backup file 30+ giorni  

---

## 9. Stato complessivo (aggiornare a mano)

| Blocco | Pronto? |
|--------|---------|
| Schema + `migrated_at` | 🔶 |
| Import batch (+ aux strutturato se serve) | 🔶 |
| Load da DB | 🔶 |
| Save DB + stop file | ❌ |
| Login/menu wired | 🔶 |
| Test 7.x su staging | 🔶 |
| Runbook ops | ☐ |

**Verdetto attuale:** si può migrare e verificare su **staging**; **non** tagliare in produzione finché §3–§5 non sono implementati.

---

## 10. Ordine di implementazione consigliato

1. `migrated_at` + set in `legacy_import` (§1)  
2. Completare `load_character_from_db` inventario+extra (§3.8, §3.9)  
3. `save_character_to_db` + inventario + stop file (§4)  
4. Load/save extra **oppure** import strutturato da `.aux` (§2.3)  
5. Script batch + report (§2.5–2.6)  
6. Rimuovere fallback file in `con_pwdok` (hard cutover §5)  
7. Test §7 → batch prod → deploy  

---

## Ambiente Vagrant (sviluppo locale)

Su questo PC si usa **Vagrant**, non Docker. Il repo è montato in `/vagrant` nella VM `nebbieserver` (Ubuntu Jammy).

### Giorno per giorno

```bash
# host
vagrant up
vagrant ssh

# nella VM
cd /vagrant
./build.sh vagrant          # rigenera ODB se account.hpp è cambiato (target CMake account)
cd mudroot && ./myst        # come README
```

`./build.sh vagrant` limita i job paralleli a 2 sul filesystem condiviso; se fallisce, riprova con build seriale (già gestito dallo script).

### DDL cutover su MySQL (§1)

MySQL gira **nella VM** (come in README), non in un container Docker.

```bash
cd /vagrant
mysql -u root -p"${MYSQL_PASSWORD:-secret}" "${MYSQL_DB:-nebbie}" \
  < docs/schema-s1-toon-migration-flags.sql
```

I flag cutover **non** passano da migrate ODB (`migrated_at` / `schema_version` sono `transient` in `account.hpp`); usare sempre lo script SQL sopra.

### Test §1 in-game

1. Applicare DDL SQL (obbligatorio)  
2. `./build.sh vagrant`  
3. `cd mudroot && ./myst`  
4. Immortale: `legacyimport montero`  
5. Verifica SQL (sotto)

**Nota:** finché ODB non è rigenerato, `toon_migration` legge/scrive `migrated_at` via **SQL diretto** — `legacyimport` funziona ugualmente dopo l’`ALTER TABLE`.

---

## Comandi utili oggi (staging)

```text
legacyprobe <nome>
legacyimport <nome>
```

```bash
python3 docs/export-legacy-import-csv.py <nome>
```

Verifica SQL esempio:

```sql
SELECT t.id, t.name, t.migrated_at, t.schema_version, cc.toon_id
FROM toon t
LEFT JOIN character_core cc ON cc.toon_id = t.id
WHERE t.name = 'montero';
```
