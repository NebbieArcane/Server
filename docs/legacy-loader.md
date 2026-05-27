# legacy_loader — come provarlo

Modulo **sola lettura**: `src/legacy_loader.hpp`, `src/legacy_loader.cpp`.

## Perché la directory conta

Il server parte con `-d mudroot/lib` e fa `chdir` lì. I path sono **relativi**:

| Costante | Path reale |
|----------|------------|
| `PLAYERS_DIR` = `"players"` | `mudroot/lib/players/<nome>.dat` |
| `RENT_DIR` = `"rent"` | `mudroot/lib/rent/<nome>` e `<nome>.aux` |

`legacy_loader` usa le stesse costanti: funziona **solo** se il processo ha cwd = `mudroot/lib` (come il mud in esecuzione). Non serve cambiare codice se avvii il server come sempre.

---

## Metodo consigliato: comando `legacyprobe` (immortale)

Dopo rebuild del server:

1. Avvia il mud come al solito (Docker o locale), con dati in `mudroot/lib`.
2. Entra con un PG **immortale** (livello ≥ Maestro del Creato).
3. In gioco:

```
legacyprobe montero
```

(sostituisci con un nome che ha `mudroot/lib/players/<nome>.dat` in minuscolo, es. `montero`, `pippo`, `alar`).

### Output atteso

- `legacyprobe .dat OK` — nome, exp, oro, hit/mana dal file
- `resist (da affect in .dat)` — bit ricostruiti (spesso tutti 0 se nel save non ci sono affect 26/27/28)
- `.aux: N righe` oppure `assente`
- `rent (new|old): oggetti=…` oppure `assente`

Confronta **exp/oro/hit** con quello che vedi con `score` sullo stesso PG (dopo login normale): devono essere coerenti.

### Se dice “.dat non trovato”

- Nome sbagliato o PG senza file (solo account DB, mai salvato su disco).
- Server non avviato con `-d mudroot/lib` (cwd diverso).
- Controlla a mano:

```bash
ls mudroot/lib/players/*.dat
```

---

## Metodo 2: verifica file a mano (senza mud)

Dal root del repo:

```bash
ls -la mudroot/lib/players/montero.dat
ls -la mudroot/lib/rent/montero.aux
ls -la mudroot/lib/rent/montero
```

Se i file ci sono, `legacyprobe montero` dovrebbe funzionare a mud avviato.

---

## Metodo 3: log da codice (sviluppo)

In qualsiasi punto del server (solo debug):

```cpp
#include "legacy_loader.hpp"
char_file_u st;
if (Alarmud::legacy_load_char_file("montero", st)) {
    mudlog(LOG_ALWAYS, "legacy: %s exp=%d", st.name, st.points.exp);
}
```

---

## Cosa testa ogni API

| Comando / API | File | OK se |
|---------------|------|--------|
| `legacy_load_char_file` | `players/<name>.dat` | `.dat OK` con dati plausibili |
| `legacy_load_char_aux` | `rent/<name>.aux` | `N righe` > 0 |
| `legacy_load_rent_file` | `rent/<name>` | `oggetti` ≥ 0, formato new/old |
| `legacy_derive_resistance_from_file` | affect dentro `.dat` | bit hex (spesso 0) |

---

## Build

```bash
./build.sh vagrant    # o il tuo ambiente abituale
# Docker: rebuild immagine / riavvia container con il nuovo myst
```

Il sorgente è già nel target `myst` (tutti i `.cpp` in `src/`).

---

## Import in MySQL (`legacyimport`)

Dopo rebuild, da immortale:

```
legacyimport montero
```

1. Legge `players/montero.dat` (e `.aux`, rent se presenti).
2. Trova `toon.id` per nome (es. Montero → id 4).
3. **Cancella** le righe `character_*` esistenti per quel `toon_id` e reinserisce.

`character_classes`: solo indici **0–10** (`MAX_CLASS`), non gli slot 11–19 del file `.dat`.

Se il DB ha righe spurie da un import precedente:

```bash
mysql -h 127.0.0.1 -u root -psecret nebbie < docs/schema-s1-ddl-fix-character-classes.sql
```

Verifica:

```bash
docker exec nebbieserver mysql -h 127.0.0.1 -u root -psecret nebbie -e \
  "SELECT * FROM character_core WHERE toon_id=4;
   SELECT hit,max_hit,gold,exp FROM character_stats WHERE toon_id=4;
   SELECT class_index,level FROM character_classes WHERE toon_id=4;"
```

Atteso per Montero: `exp=1`, `gold=500`, `hit=29`, `max_hit=23` (come `legacyprobe`); classi `(1,2)` e `(2,14)` (chierico 2, guerriero 14).

Export CSV da MySQL (dump leggibile; `description` su una riga; `extra_str` solo nel `.dat`):

Rimuovere colonna `extra_str` da DB già creato:

```bash
mysql -h 127.0.0.1 -u root -psecret nebbie < docs/schema-s1-ddl-drop-extra-str.sql
```

Export CSV:

```bash
python3 docs/export-legacy-import-csv.py montero
```

## Prossimo passo

Import batch di tutti i `.dat` in `lib/players/`.
