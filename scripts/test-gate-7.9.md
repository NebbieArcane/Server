# Gate 7.9 — `.dead` (DEATH_FIX)

`DEATH_FIX` è **attivo** in build (`flags.hpp`). Il sidecar **non** va in MySQL; resta su `lib/players/<nome>.dead`.

## PG pilota

Mortale migrato **&lt; 58** (es. PG test dedicato) o PG staging non critico. **Non** usare immortali per la morte con loss.

## A — Creazione `.dead` (minimo)

1. `./scripts/check-gate-7.7.sh <nome> before` — nota `missing` o mtime `.dead`.
2. Login PG; annota `GET_EXP` (o `score`).
3. Muori **senza perdere livello** (morte che chiama `save_exp_to_file` in `fight.cpp`).
4. Verifica file:

```bash
cat mudroot/lib/players/<nome>.dead
# atteso: "<exp> : <epoch>"
```

5. Log mud: `Saving xp per <nome>` / `Opening ...dead`.

## B — Sacrificio (opzionale, se avete il mob/room)

- Richiede `.dead` fresco (&lt; 4 h) e CON sufficiente.
- Room con proc sacrificio (`spec_procs.cpp`); log `sacrifice accepted`.
- `GET_EXP` ripristinato da `.dead`.

## C — Resurrezione / reincarnate (opzionale)

- Spell su PG con file `.dat` + `.dead` presenti.
- Verifica messaggi errore se `.dead` mancante.

## D — PG migrato + D2

Dopo morte, controlla:

```bash
./scripts/check-gate-7.7.sh <nome> after
```

- `.dat` **invariato** (migrato)
- `.dead` **può** essere creato/aggiornato (§4.8 consentito)

## PASS minimo (7.9)

| Check | OK |
|-------|-----|
| A | `.dead` creato con exp:epoch dopo morte |
| D | `.dat` non riscritto su migrato |
| Re-login | Exp/coerenza da DB + `.dead` ancora su disco |

Segnare **N/A** su B/C se non avete room sacrificio / spell in staging.
