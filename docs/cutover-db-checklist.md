# Cutover DB — checklist operativa

Checklist per il **taglio netto** da file (`.dat` / rent / `.aux`) a MySQL come unica fonte di verità in runtime.

**Strategia target:** login **B** (load da DB) + gate **C2** (`toon.migrated_at`) + save **D2** (niente scrittura file per PG migrati). I file restano solo come **backup** e per restore manuale.

**Riferimenti:**

- Mapping campi: [schema-s1-mapping.md](schema-s1-mapping.md) (`.dead` escluso — vedi sezione dedicata sotto)
- DDL: [schema-s1-ddl-draft.sql](schema-s1-ddl-draft.sql) (+ script incrementali: resistance, condition, `extra_str`, classi)
- Inventario storico (soft delete): [schema-s1-inventory-soft-delete.sql](schema-s1-inventory-soft-delete.sql)
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
| Transizione | **Lazy migration al login** | Niente import di massa obbligatorio; `legacyimport` resta tool operativo |

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

### Implicazione operativa (lazy migration)

Con lazy migration in `con_pwdok`, i PG vengono migrati quando entrano.
Quindi:

1. Non serve batch `legacyimport` globale come prerequisito GO.
2. `legacyimport <nome>` resta utile per restore drill, test e casi manuali.
3. PG che non entrano restano su file (`.dat`) finche' non fanno login.

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
| `legacyimport` importa `.dead`? | ✅ | Se esiste `players/<nome>.dead` → `character_death_snapshot` |
| Ha senso una tabella al cutover? | **No** (fase 1) — meccanica file; basso valore per batch/report |
| Dopo D2 (PG migrato, no `.dat`)? | **Resta file** finché morte/sacrificio/resurrezione non vengono riscritti su DB |

**Scelta documentata:** dual-write file `.dead` + tabella `character_death_snapshot` (DDL `schema-s1-death-snapshot.sql`). Load DB-first se migrato; resurrect/reincarnate aggiornano `character_stats.exp`.

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
| 0.2 | Inventario `.dat` vs `toon` (audit) | 🔶 | Consigliato per pulizia, ma non blocca il GO con strategia lazy |
| 0.3 | DDL S1 applicato su DB target | 🔶 | Vagrant: fatto a mano; script: `./scripts/apply-schema-s1.sh` |
| 0.4 | Build con `USE_MYSQL` | ✅ | Verificato da `legacyimport montero` OK |
| 0.5 | `legacy_loader` / `legacy_import` nel link del binario | ✅ | Verificato da `legacyprobe` / `legacyimport` in-game |
| 0.6 | Runbook restore singolo PG | ✅ | Drill **Montero** 2026-05-31: `restore-pg-drill.sh` + `legacyimport` + verify PASS |

**Gate 0:** backup verificato + DDL su staging = prod.

---

## 1. Schema e contratto migrazione (C2)

| # | Voce | Stato | Note |
|---|------|-------|------|
| 1.1 | `ALTER TABLE toon` (`migrated_at`, `schema_version`) | ✅ | Vagrant + `apply-schema-s1.sh`; SQL: [schema-s1-toon-migration-flags.sql](schema-s1-toon-migration-flags.sql) |
| 1.2 | Campi cutover su `toon` (SQL + struct C++ `transient`) | ✅ | DDL: [schema-s1-toon-migration-flags.sql](schema-s1-toon-migration-flags.sql); ODB account resta v1 (no migrate ODB per questi campi) |
| 1.3 | `legacy_import` setta flag in transazione prima di `commit` | ✅ | `toon_mark_migrated_tx` in `legacy_import.cpp` |
| 1.4 | Helper in `toon_migration.hpp` / `.cpp` | ✅ | fetch SQL + mark/clear/sanity |
| 1.5 | Sanità: `migrated_at` set ma no `character_core` → errore | ✅ | `toon_migration_sanity_check` (da chiamare al login in §5) |
| 1.6 | `toon_nuke_blacklist` (nuke audit + ban login) | ✅ | DDL in [schema-s1-ddl-draft.sql](schema-s1-ddl-draft.sql) §9 + [schema-toon-nuke-blacklist.sql](schema-toon-nuke-blacklist.sql); `apply-schema-s1.sh` |

**Gate 1:** almeno un PG test con import OK + `migrated_at` valorizzato.

---

## 2. Import / migrazione runtime (lazy)

| # | Voce | Stato | Note |
|---|------|-------|------|
| 2.1 | `legacy_import_character_mysql` | ✅ | `.dat` + stats/class/skill/affect/resist + rent → inventory; `.dead` → `character_death_snapshot` se file presente; test OK su Montero |
| 2.2 | `.aux` → `character_prefs` (KV grezzo) | ✅ | `legacy_insert_prefs` + chiavi univoche per achie/mercy |
| 2.3 | `.aux` → `character_achievements` / `aliases` / `mercy` | ✅ | `legacy_insert_aux_structured` in `legacy_import.cpp` (stesso parse tag del `.aux`) |
| 2.4 | `quest_mob` / progress quest | ✅ | **Session-only** (decisione 2026-06-02): non in `.aux`/DB; `character_quest_progress` non usata |
| 2.5 | Lazy migration in `con_pwdok` | ✅ | `toon_needs_migration` + `legacy_import_character_mysql` al login |
| 2.6 | Report runtime (log) | 🔶 | Monitorare `lazy migration OK/FAILED`; report SQL opzionale |
| 2.7 | Probe post-import (campione) | 🔶 | Montero: prefs=achie/alias/mercy allineati (2026-06-02); campione più ampio consigliato |

**Gate 2 (lazy):**

- nessun errore `lazy migration FAILED` sui PG che entrano
- PG migrato entra e rientra solo da DB (`migrated_at` set)
- PG con `.aux` ricco: verificare conteggi `achie`/`alias`/`mercy` nel messaggio `legacyimport` / lazy migration

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
| 3.8 | `load_character_inventory_from_db` | 🔶 | `load_rent_mysql` + `load_char_objs` se `migrated_at` |
| 3.9 | `load_character_extra_from_db` | 🔶 | `load_char_extra_mysql` + `load_char_extra` se `migrated_at` |
| 3.10 | PG `justCreated` | ✅ | `con_register` → `con_pwdok` skip reload MySQL; test 7.1 Vagrant OK |
| 3.11 | Poly / ghost / impersonate | 🔶 | Poly ✅ (7.4); extra/alias/achie ✅ (7.6); ghost fix in working tree |

**Gate 3:** 3+ PG rappresentativi — import → load solo DB → confronto con attese (pre-cutover).  
Stato: testato con `legacyloadcheck` su `Montero` e `TheProdigy` (parita campi chiave file vs DB).

---

## 4. Save runtime su DB (D2)

| # | Voce | Stato | Note |
|---|------|-------|------|
| 4.1 | `save_character_to_db(ch)` transazionale | ✅ | `db.cpp`: body+toon+extra+rent via flag; `update_file` migrati = 1 transazione extra+rent |
| 4.2 | Aggiorna `toon` (password, title, level, lastlogin, lasthost) | ✅ | `db_update_toon_registry_tx` in ogni `save_char` → `CHAR_DB_SAVE_TOON` (`lastlogin=NOW()`) |
| 4.3 | `save_character_inventory_to_db` | 🔶 | Via `save_character_to_db` flag `RENT` (`save_rent_mysql_tx`); soft-delete **DEATH** + **RENT_EXPIRED**; **NUKE/TRAP** non collegati |
| 4.4 | `save_character_extra_to_db` | 🔶 | Via `save_character_to_db` flag `EXTRA` (`save_char_extra_mysql_tx`); load runtime ancora solo `character_prefs` |
| 4.5 | Niente `fwrite` `.dat` se `migrated_at` set | ✅ | `save_char` + `toon_is_migrated_by_name` |
| 4.6 | Niente scrittura `rent/<name>` / `.aux` se migrato | ✅ | `update_file` / `write_char_extra` |
| 4.7 | Menu `'1'..'4'`: oggi `load_char_objs` + `save_char` | ✅ | Migrati: no save all'ingresso (`con_slct`/`con_city_choice`, `load_char_objs`); save al quit/rent |
| 4.8 | `.dead` + `character_death_snapshot` | ✅ | `death_snapshot_save/load`; resurrect/reincarnate → DB exp; file ancora scritto |

**Gate 4:** login → modifica stato/oggetti → quit → re-login identico **solo da DB**; mtime file invariato.

---

## 5. Wiring login / menu

| Punto | Stato | Comportamento (staging) |
|-------|-------|-------------------------|
| `con_pwdok` | ✅ | Lazy import se `toon_needs_migration`; PG migrato: solo `load_char_mysql`, no fallback file |
| Fallimento load DB | ✅ | Migrati bloccati (no fallback file) |
| Entrata menu `'1'` | ✅ | Reload da MySQL; migrati: **no** `do_save` all'ingresso (`skip_menu_enter_save`); rent/extra da DB |
| Scelte città `'1'..'5'` | ✅ | Come menu `1` per migrati (`con_city_choice`) |
| Quit / `rent` | ✅ | `save_character_to_db` (body+toon / extra+rent); no `.dat`/rent/`.aux` file |
| `do_refund` | ✅ | `eq` SQL-first multi-causa + fallback zip; `all` = `eq` + zip `pg`/`achie` |

**Gate 5:** ciclo account → pwd → menu 1 → gioco → quit senza scrittura su `players/*.dat`.  
Stato: **Alar** 2026-05-31; **Montero** 2026-06-02 (lazy import, menu senza save ingresso, `rent`+re-login, log `skip enter save` / `skip post-load save`).

---

## 6. Gap noti (non dimenticare)

| Area | Rischio |
|------|---------|
| `.aux` strutturato | Import popola prefs + tabelle strutturate; load runtime usa ancora solo `character_prefs` (come `load_char_extra_mysql`) |
| `quest_mob` | **Session-only** — progresso mob-per-quest solo in RAM; non import/load/save DB (contatori `achie_quest` restano su prefs) |
| Achievements fuori `.aux` | Verificare se `.achie` è ancora usato |
| `do_refund` | Policy: **solo `eq` serve da MySQL**; `pg`/`achie` restano zip | `eq` = SQL-first + fallback zip; finestra oraria: `m` 00–09, `p` 10–15, `s` 16–23 |
| `.dead` (DEATH_FIX) | Dual-write file + `character_death_snapshot` su migrati | |
| Nome `.dat` ≠ `toon.name` | Audit consigliato (0.2), non blocco GO in lazy | |
| `character_core` senza title/pwd | By design su `toon`; load deve unire le fonti | |
| Soft delete inventario | 🔶 | **DEATH** + **RENT_EXPIRED** wired; storico non sovrascritto a morti/rent ripetuti; refund reindex `list_index` post-restore |
| `do_nuke` | `Sql::erase(toon)` + CASCADE | Cancella righe inventario in DB (no soft-delete); **refund SQL `eq` bloccato** se PG non esiste; causa `NUKE` in refund solo se righe marcate manualmente / futuro |
| `TRAP` | Non collegato | Causa in lista refund SQL; nessun `mark_inventory_deleted_mysql` runtime (voluto per ora) |

---

## Refund e soft-delete (policy 2026-06-02)

| Comando | Equip (`eq`) | PG / achie |
|---------|----------------|------------|
| `refund <pg> <data> <m\|p\|s> eq` | SQL-first (tutte le cause in codice), poi zip rent se 0 righe | — |
| `refund … all` | Come `eq` | Solo backup zip (`pg` / `.aux`) |

**MySQL per refund:** serve **solo l’inventario** (`character_inventory` soft-delete). Non c’è restore storico SQL di `.dat` / achievements.

**Marcatura automatica `deleted_for`:**

| Causa | Wired runtime |
|-------|----------------|
| `DEATH` | ✅ `die()` |
| `RENT_EXPIRED` | ✅ `load_char_objs` quando `found=FALSE` (zeroing rent) |
| `NUKE` | ❌ `do_nuke` fa `Sql::erase(toon)` → CASCADE cancella inventario; refund SQL bloccato (PG assente) |
| `TRAP` | ❌ rinviato |
| `MANUAL` | Solo operazioni DB / tool |

**Nota `do_nuke`:** non ha senso chiamare `mark_inventory_deleted_mysql(..., "NUKE")` prima dell’erase se si cancella subito il `toon` (nessun refund SQL possibile). Per recuperare un nuked serve backup zip/file, non DB.

---

## 7. Test obbligatori (GO / NO-GO)

| # | Test | Stato | Note staging Vagrant (2026-05-31) |
|---|------|-------|-----------------------------------|
| 7.1 | PG nuovo: solo DB, login/logout | ✅ | Ritest post `bb6d942`: niente crash menu 5 / delete |
| 7.2 | PG legacy migrato: gioco + save + re-login | ✅ | **Alar**: `loaded … from MySQL`, quit room 7801/1000, re-login rent da DB (1 oggetto), 1M coins |
| 7.3 | Rent: equip + pensione | ✅ | A: mortale &lt;58 rent+quit drop OK; B: Alar ≥58 quit conserva eq + rent OK |
| 7.4 | Poly + save | ✅ | Quit in poly bloccato; `return` + quit + re-login OK |
| 7.5 | Ghost / reconnect (se usato) | ✅ | **A OK** TheProdigy 2026-05-31: LD → `Riconnessione...` + mtime file invariati; B ghost opz./N/A |
| 7.6 | Achievement / alias / mercy (PG con `.aux` ricco) | ✅ | **TheProdigy** 2026-05-31: alias 5, achie (es. 9/500, 110/1000), SQL 19/12/5, `.aux` invariato; mercy N/A |
| 7.7 | mtime `.dat` / rent dopo sessione migrata | ✅ | **Alar**: `.dat` 3040 B e `.aux` 92 B invariati; `rent/alar` 0 B, solo mtime (+67 s), log `skip rent file` |
| 7.8 | Restore drill: 1 PG da backup file + re-import | ✅ | **Montero** 2026-05-31: reset-db → `legacyimport` → verify (`migrated_at`, core+78 prefs) |
| 7.9 | Morte / sacrificio / resurrect (`.dead`) | ✅ | TheProdigy: snapshot file+DB; fix resurrect/reincarnate su migrati (no fwrite `.dat`) |
| 7.10 | Refund SQL `eq` su finestra data+ora | ✅ | **TheProdigy** 2026-06-02: `s` + `RENT_EXPIRED` OK; `p` senza match → fallback zip; reindex `list_index` dopo refund (no oggetto perso se eq preso dopo perdita) |
| 7.11 | `RENT_EXPIRED` login + storico SQL | ✅ | **TheProdigy** 2026-06-02: rent arretrato → ingresso nudo; 42 righe `deleted_for=RENT_EXPIRED`; `save` non cancella storico |
| 7.12 | Save unificato + menu §4.7 (Montero) | ✅ | **Montero** 2026-06-02: `save_character_to_db` al `rent`; ingresso menu **senza** save DB; quit→re-login 17 oggetti / gold invariati (`2eca00c`) |

Comando 7.7: `./scripts/check-gate-7.7.sh <nome> before` → gioco → quit → `after` → `diff` (PASS se `.dat`/`.aux` identici; rent stub 0 byte può cambiare solo mtime).

**GO** se 7.1–7.5 e 7.7 passano; 7.6 se avete PG con `.aux` non banali; 7.9 se il mud usa DEATH_FIX in produzione.  
**Staging oggi:** **7.1–7.12** OK su Vagrant (ultimo: Montero save/menu 2026-06-02) |

---

## 8. Deploy day (ordine)

1. Manutenzione / mud down  
2. Backup finale (0.1)  
3. Creare `toon` mancanti da `.dat` se serve (sezione `.dat` e `toon`)  
4. Deploy binario con lazy migration attiva (`con_pwdok`)  
5. Deploy binario con §3–§5  
6. Smoke 7.1–7.3  
7. Monitor log import / load / SQL  
8. Tenere backup file 30+ giorni  

---

## 9. Stato complessivo (aggiornato 2026-06-02)

| Blocco | Pronto? |
|--------|---------|
| Schema + `migrated_at` | ✅ (Vagrant) |
| Import runtime lazy (+ aux strutturato) | ✅ | `legacy_insert_aux_structured`; `quest_mob` session-only |
| Load da DB | 🔶 | Corpo + rent + extra se migrato; gap resistenze load, kludge `talks`/`load_room` |
| Save DB + stop file | ✅ | `save_character_to_db` (§4.1–4.2); D2; menu senza save ingresso (§4.7) |
| Login/menu wired | ✅ | `con_pwdok`, menu `1`, città `1`–`5`, quit/`rent` |
| Test 7.x su staging | ✅ | **7.1–7.12** su Vagrant |
| Runbook ops | ☐ | §0.1 backup produzione |

**Commit cutover recenti (branch `sviluppo`):** `bdf500b` save unificato + aux strutturato; `2eca00c` menu migrati senza save ingresso.

**Verdetto attuale:** **staging GO tecnico** per lazy cutover (login → gioco → quit → re-login solo DB). **Non** GO produzione senza **§0.1 backup finale** e deploy binario su ambiente target.

---

## 10. Ordine di implementazione consigliato

1. ~~`migrated_at` + set in `legacy_import` (§1)~~ ✅  
2. ~~Test **7.7** + **7.2** su Alar~~ ✅ (2026-05-31); ~~**7.1** PG nuovo~~ ✅  
3. ~~Test **7.3** rent + quit~~ ✅ — ~~**7.4** poly~~ ✅ — ~~**7.5** LD/reconnect (TheProdigy)~~ ✅  
4. ~~§4.2 + §4.1 + §4.7 menu~~ ✅ (2026-06-02; test Montero)  
5. ~~Import strutturato `.aux` (§2.3)~~ ✅  
6. ~~Rimuovere fallback file in `con_pwdok` se migrato~~ ✅  
7. ~~Refund SQL `eq` multi-causa + `RENT_EXPIRED`~~ ✅  
8. **`do_nuke`:** verificato — erase `toon` + CASCADE; refund SQL solo pre-nuke o da zip  
9. **`TRAP`:** wiring `mark_inventory_deleted_mysql` — rinviato  
10. **Adesso:** §0.1 backup produzione → deploy binario → smoke 7.1–7.3 in prod → monitor log `lazy migration`  
11. Opzionale: load resistenze da `character_resistance`; campione PG su `legacyloadcheck`  

---

## Ambiente Vagrant (sviluppo locale)

Su questo PC si usa **Vagrant**, non Docker. Il repo è montato in `/vagrant` nella VM `nebbieserver` (Ubuntu **24.04 Noble**, allineata al Dockerfile Docker).

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

### DDL cutover su MySQL (§0.3 / §1)

MySQL gira **nella VM** (come in README), non in un container Docker.

Dopo import dump `nebbie-import.sql` (repo) o `nebbie.sql`:

```bash
cd /vagrant
./scripts/apply-schema-s1.sh
./build.sh vagrant
```

Al provision Noble, `scripts/vagrant-provision-noble.sh` applica solo `migrated_at` se `toon` esiste già (non il DDL S1 completo).

I flag cutover **non** passano da migrate ODB (`migrated_at` / `schema_version` sono `transient` in `account.hpp`); usare sempre lo script SQL sopra se il provision non li ha ancora applicati.

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
legacyloadcheck <nome>
```

```bash
python3 docs/export-legacy-import-csv.py <nome>
./scripts/check-gate-7.7.sh <nome> before   # gioco → quit → after
```

### Log utili (PG migrato)

- `lazy migration OK` / `loaded … from MySQL`
- `skip enter save for migrated` / `skip post-load save for migrated`
- `save_character_to_db: OK … flags=0x3` (body+toon) / `0xc` (extra+rent)
- `skip .dat file` / `skip rent/.aux file`

### Verifica SQL esempio

```sql
SELECT t.id, t.name, t.owner_id, t.migrated_at, t.lastlogin, t.lasthost,
       cc.toon_id, (SELECT COUNT(*) FROM character_inventory ci
                    WHERE ci.toon_id = t.id AND (ci.deleted = 0 OR ci.deleted IS NULL)) AS inv
FROM toon t
LEFT JOIN character_core cc ON cc.toon_id = t.id
WHERE t.name = 'Montero';
```
