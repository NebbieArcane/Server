# Gate 7.6 — Achievement / alias / mercy (`.aux` ricco)

**PG pilota staging:** `TheProdigy` (`.aux` ~347 B: achievements + alias; DB già con prefs/ach/alias dopo sessioni migrato).

**Nota codice:** il load migrato usa solo `character_prefs` (KV da `.aux`), non le tabelle strutturate in lettura — il save dual-write popola anche `character_achievements` / `character_aliases`.

## Prima (SQL)

```bash
vagrant ssh -c '/vagrant/scripts/gate-sql.sh theprodigy all' | tee /tmp/tp-7.6-before.txt
```

Atteso (ordine di grandezza): ~19 `character_prefs`, ~12 `character_achievements`, ~5 `character_aliases`.

## In-game

1. Login **TheProdigy** (migrato).
2. **Alias:** `alias` — devono comparire almeno `group`, `assist demetro`, `bash`, … (come in `.aux` righe `0:`…`9:`).
3. **Achievement:** comando/info achievement del mud (es. lista achie se esiste) — verifica che non sia tutto a zero (es. `achie_other:6#110` in file).
4. Modifica leggera (opz.): aggiungi alias `test7.6` slot libero, oppure gioca fino a un save (`save` / quit con rent).
5. Quit normale (≥58 o mortale con flusso abituale).

## Dopo (SQL)

```bash
vagrant ssh -c '/vagrant/scripts/gate-sql.sh theprodigy all' | tee /tmp/tp-7.6-after.txt
diff -u /tmp/tp-7.6-before.txt /tmp/tp-7.6-after.txt
```

## Re-login

6. Entra di nuovo → alias e achievement ancora coerenti con il passo 4.
7. `./scripts/check-gate-7.7.sh theprodigy` → `.aux` **non** deve essere riscritto (mtime/size come 7.7); i dati vivono in DB (`character_prefs`).

## PASS

| Check | OK se |
|-------|--------|
| Alias in gioco | Visibili e stabili al re-login |
| Achievement | Non azzerati; prefs/ach in SQL ≥ prima |
| D2 file | `.aux` mtime/size invariati post quit |

## FAIL tipici

- Alias spariti → load prefs KO o save non chiamato.
- Doppio alias / crash → non caricare anche tabelle strutturate in load (già fixato in `load_char_extra_mysql`).
