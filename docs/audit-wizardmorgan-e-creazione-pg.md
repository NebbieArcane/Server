# Audit wizardmorgan + creazione PG (Pupino) — backlog per sistemazione

Documento ricavato dalla sessione del **31 maggio 2026** (chat + ispezione codice/git).  
Serve come lista di lavoro per chiudere i regressi introdotti con la migrazione DB e ciò che resta aperto.

---

## 1. Caso Pupino (sintomo)

Durante creazione con roll manuale (`nuovo`):

```
Ecco le stats risultanti dalla tua scelta:
19  7  7 13 18  7
Sei soddisfatto(S/N)? s
```

Classe Warrior (0), poi entrata in gioco. Comando `stat pupino`:

```
Stats: Str:[3/0] Int:[3] Wis:[3] Dex:[3] Con:[3] Cha:[3]
```

**Atteso:** stat vicine al roll (19, 7, 7, 13, 18, 7).  
**Ottenuto:** tutte a 3.

---

## 2. Causa root (bug creazione stat)

### Flusso rotto (commit wizardmorgan `a83fe90`)

1. Roll manuale → valori salvati in `d->stat[]` + `TipoRoll` sul **descriptor** (`InterpretaRoll` in `interpreter.cpp`).
2. `roll_abilities()` applica davvero le stat → era **solo** in `do_start()` (`act.wizard.cpp`).
3. `do_start()` parte **solo** se `GetMaxLevel(ch) == 0` (menu opzione `1`, `con_slct`).
4. Con `a83fe90`, in `con_register` viene chiamato **`StartLevels`** → il guerriero è già **livello 1** prima del menu.
5. Quindi `do_start()` **non parte** → `roll_abilities()` **mai eseguito**.
6. `init_char` impostava solo `tmpabilities` (macro `GET_STR` ecc.), non `abilities` → primo `save_char` poteva salvare **abilities a 0** nel DB/file.

### Chi l’ha introdotto

| Campo | Valore |
|--------|--------|
| Commit | `a83fe90ff35037103bef5c113bb0c57518779e55` |
| Autore | **wizardmorgan** (`wizmorgan@gmail.com`) |
| Data | 14 nov 2025 |
| Messaggio | `changed registration nanny function to save new toon password in the DB at the creation and align .dat accordingly` |

`roll_abilities` **non è stata rimossa** da un commit: non è mai stata nel flusso `con_qclass`/`con_register`; è rimasta solo in `do_start`, resa irraggiungibile da `StartLevels` anticipato.

### Fix proposto (in working tree, **non committato**)

File: `src/interpreter.cpp`, in `con_register` dopo `init_char`:

```cpp
init_char(d->character);
roll_abilities(d->character);  // AGGIUNTO
StartLevels(d->character);
```

File: `src/db.cpp`, in `init_char` — impostare **entrambi**:

```cpp
ch->abilities.str = 9;
// ... intel, wis, dex, con, chr ...
ch->tmpabilities = ch->abilities;
```

(invece di solo `GET_STR(ch) = 9` ecc.)

**Azione domani:** commit + rebuild + test (sezione 8).

---

## 3. Creazione personaggio — cosa scrivere (riferimento giocatore)

### Schermata priorità stat (`CON_STAT_LIST`)

| Scelta | Cosa digitare |
|--------|----------------|
| Automatico | **INVIO** (vuoto) → roll semplice dopo la classe |
| Priorità classica | 6 lettere separate da spazi, es. `F I S A CO CA` (ordine da più alta a più bassa) |
| Esperto | `nuovo` o `new` → roll a punti |

Lettere: **F** Forza, **I** Intelligenza, **S** Saggezza, **A** Agilità, **CO** Costituzione, **CA** Carisma.  
**b** = indietro.

### Modalità `nuovo` (roll manuale)

- Base: **7** per stat (`STAT_MIN_VAL`).
- Punti extra da distribuire: **29** (`STAT_MAX_SUM`).
- Formato: `FO IN SA AG CO CA` (punti **aggiuntivi**, non il totale).
- Opzionale 7° token (es. `r`) per random ±1.
- Esempio senza random: `11  7  4  2  2  2` → circa F18 I14 S11 resto 9.
- Somma ≤ 29; rispettare limiti razza mostrati a schermo.
- Conferma: **S** / **N** (rifare) / **b**.

---

## 4. Commit wizardmorgan (nov 2025)

| Hash | Data | Messaggio | Rischio |
|------|------|-----------|---------|
| `7a49c93` | 11 nov | Refactor passwd change to use database | Medio |
| `fd16b8c` | 11 nov | do_refund directory structure | Basso |
| `b492184` | 14 nov | Old functions restored (commenti) | Nessuno |
| `2204cd5` | 14 nov | nuke/ghost/refund DB | **Alto** |
| `3f30c59` | 14 nov | save_char sync password su `toon` | Medio |
| `a83fe90` | 14 nov | con_register + toon DB creazione | **Alto** (stat) |
| `3ac0bf1` | 14 nov | refund check toon in DB | Basso |
| `38b6740` | 14 nov | do_stat/do_set/... C++ refactor | **Alto** (regressioni) |
| `33740de` | 17 nov | Merge PR fix-commands | — |

---

## 5. Stato fix: cosa è già a posto

### Nei commit su `sviluppo` (Montero / cutover — **non** wizardmorgan)

| Problema | Commit / nota |
|----------|----------------|
| Corpo PG su MySQL al save | `8dd6759` — `save_char_mysql_snapshot` |
| Login DB-first + lazy migration | `2934a11` |
| Load da MySQL | `4d43a53` — `load_char_mysql` |
| Delete menu cancella `character_*` | `bb6d942` — `legacy_delete_character_rows` in `con_delete_me` |
| `justCreated` evita reload MySQL prematuro | `bb6d942` |
| Ghost migrati (load/save DB) | evoluzione + checklist **7.5 OK** |
| Refund migliorato | `8a2ce9a`, `3ac0bf1`, ecc. |
| `file_to_string` ripulito | `e39406c` |
| Checklist test 7.1–7.9 | `docs/cutover-db-checklist.md` (aggiornata 31 mag 2026) |

**Nota:** il test **7.1** (PG nuovo) in checklist verifica login/DB/crash menu — **non** verifica esplicitamente il roll manuale delle stat.

### Creazione stat (P0) — **in repo**

| Fix | File |
|-----|------|
| `roll_abilities` in `con_register` | `src/interpreter.cpp` |
| `init_char` → `abilities` + `tmpabilities` | `src/db.cpp` |

Altri file modificati localmente (death snapshot, magic, spec_procs, script gate): vedi `git status` — allineati al cutover `.dead` / gate 7.9, non al pacchetto wizardmorgan sotto.

---

## 6. Ancora da sistemare (backlog domani)

Priorità suggerita.

### P0 — Bloccante gameplay

- [x] **Commit** fix `roll_abilities` + `init_char` (sezione 2) — in `interpreter.cpp` / `db.cpp`.
- [ ] **Ritest creazione:** roll `nuovo` → numeri mostrati = `stat self` dopo opzione menu `1`.
- [ ] Eventuale **repair PG già creati** (es. Pupino): wizset stat, o delete + ricrea, o script SQL.

### P1 — DB coerenza (wizardmorgan `2204cd5`)

- [x] **`do_nuke`**: blacklist `toon_nuke_blacklist` (toon_id, nome, quando, chi); purge; no erase `toon`/`character_*`; login bloccato (`schema-toon-nuke-blacklist.sql`).
- [x] **`forgive`**: `do_forgive` — rimuove dalla blacklist nuke; il PG puo' rifare login (dati MySQL intatti).

### P2 — Password / file

- [x] **`do_passwd`**: aggiorna DB; patch `.dat` se PG non migrato e file esiste.
- [x] Nessuna password in chiaro nel messaggio imm.
- [ ] **`CMD_SAVE`** in `do_passwd` — obsoleto (ghost); messaggio ridotto, nessun save file.

### P3 — Sicurezza / pulizia

- [x] **`wizRegister`**: `register account` disabilitato (`if(false && …)`).
- [x] **`Rollata == 'N'`** in `roll_abilities` (era `=` per errore).

### P4 — Architettura (non urgente)

- [ ] `save_char`: doppio percorso (`save_char_mysql_snapshot` + update parziale tabella `toon`) — documentato, ridefinibile dopo cutover pieno.
- [x] `do_ghost`: `ghost_load_char_store` allineato a `con_pwdok` (lazy import, DB-first, fallback file + sync pwd/title da `toon`).
- [ ] Allineare checklist §7.1 con test esplicito **roll manuale stat**.

---

## 7. Cosa NON era di wizardmorgan (ma utile sapere)

- `justCreated` / skip reload MySQL → commit **Montero** `bb6d942`.
- `save_char_mysql_snapshot`, load DB-first → **Montero** (maggio 2026).
- Messaggio “Benvenuto su Nebbie Arcane. Buon divertimento” al menu può venire da **`WELC_MESSG`**, non da `do_start` (che scrive “**Bene**venuto” con typo).

---

## 8. Test rapidi (domani)

### 8.1 Nuovo PG — roll manuale

1. Crea pg, razza Half-Giant (o altra), sesso, `nuovo`.
2. Inserisci es. `12 0 0 6 11 0` (o i numeri che mostra il mud dopo conferma).
3. Conferma **S**, classe Warrior.
4. MOTD → menu → **1** (entra in gioco).
5. `stat <nome>` → verificare Str/Int/… coerenti con il roll.

### 8.2 Nuovo PG — INVIO (roll semplice)

1. A schermata stat: solo **INVIO**.
2. Scegli classe → entra → stat plausibili per classe (non tutte 3).

### 8.3 Nuke (se tocchi `do_nuke`)

1. PG di test migrato con righe in `character_*`.
2. `nuke` in gioco.
3. SQL: nessuna riga orfana per quel `toon_id` in `character_stats`, `character_core`, ecc.

### 8.4 chpwd (se tocchi `do_passwd`)

1. `chpwd <nome> <nuovapwd>` su pg con `.dat` ancora presente.
2. Verificare login: DB vs file a seconda di `migrated_at` / fallback.

---

## 9. Comandi git utili

```bash
cd /Users/corrado/Documents/GitHub/nebbietest

# Cosa non è ancora committato (fix stat inclusi)
git diff HEAD -- src/interpreter.cpp src/db.cpp

# Commit wizardmorgan creazione PG
git show a83fe90 -- src/interpreter.cpp

# Ultimi fix cutover (Montero)
git log --oneline -10 -- src/interpreter.cpp src/db.cpp
```

---

## 10. File codice toccati (riferimento rapido)

| Argomento | File principali |
|-----------|-----------------|
| Creazione / nanny | `src/interpreter.cpp` (`con_stat_list`, `InterpretaRoll`, `con_register`, `con_pwdok`, `con_slct`) |
| Roll stat | `src/act.wizard.cpp` (`roll_abilities`, `do_start`) |
| Init / save | `src/db.cpp` (`init_char`, `save_char`, `save_char_mysql_snapshot`) |
| Limiti stat | `src/interpreter.hpp` (`STAT_MIN_VAL`, `STAT_MAX_SUM`) |
| Nuke | `src/act.wizard.cpp` (`do_nuke`) |
| Delete menu | `src/interpreter.cpp` (`con_delete_me`) |
| Delete righe DB | `src/legacy_import.cpp` (`legacy_delete_character_rows`) |
| Password imm | `src/act.wizard.cpp` (`do_passwd`) |
| Checklist ops | `docs/cutover-db-checklist.md` |

---

## 11. Riepilogo una riga

**Cutover DB a posto; creazione stat (P0) in repo; nuke/chpwd/Rollata/register sistemati in `act.wizard.cpp` (2 giu 2026). Resta ritest roll `nuovo` e eventuale repair PG rovinati (Pupino).**

---

*Generato per lavoro successivo — aggiornare questo file spuntando le checkbox e aggiungendo hash commit quando chiudi le voci.*
