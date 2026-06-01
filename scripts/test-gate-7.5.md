# Gate 7.5 — Ghost / reconnect (staging)

Prerequisiti: binario con fix `do_ghost` / `save_ghost_forcerent` per PG migrati; PG pilota **Alar** (migrato) e un mortale migrato o nuovo.

## A — Link dead + reconnect (obbligatorio)

1. `./scripts/check-gate-7.7.sh <nome> before` (es. `alar`)
2. Entra in gioco con il PG; annota stanza, oro, un oggetto equipaggiato.
3. **Senza `quit`**: chiudi il client (Ctrl+] su telnet, chiudi tab, o `kill` del processo client).
4. Sul log mud (in VM: `mudroot/log/` o stdout myst) verifica:
   - `Closing link to: <nome>`
   - `<nome> e' linkdead`
   - `save_char: skip .dat file for migrated` (se migrato)
   - `update_file: skip rent file for migrated` (se rent salvato)
5. Rilogga stesso account → password → menu **1**.
6. Deve comparire **`Riconnessione...`** e lo stesso PG in stanza con stato coerente.
7. `./scripts/check-gate-7.7.sh <nome> after` → **PASS** se `.dat`/`.aux` identici a `before`.

## B — Ghost + forcerent (se usate `ghost` in prod)

Richiede immortale ≥ `MAESTRO_DEL_CREATO`, PG target **offline**.

1. `./scripts/check-gate-7.7.sh alar before`
2. Immortale: `ghost alar` — log: `do_ghost: corpo da MySQL per alar`
3. Sposta/anima brevemente il fantasma; poi `forcerent alar` (o `forcerent alldead` se LD).
4. Log: `save_ghost_forcerent: skip .dat for migrated alar`
5. `./scripts/check-gate-7.7.sh alar after` → `.dat` invariato
6. Target rilogga: inventario/stats coerenti con post-forcerent

## SQL rapido (opzionale)

```sql
SELECT t.name, t.migrated_at, cs.gold, cs.exp
FROM toon t
JOIN character_stats cs ON cs.toon_id = t.id
WHERE t.name = 'alar';
```

## Esito

| Parte | OK se |
|-------|--------|
| A | Riconnessione OK + mtime file invariati (7.7) |
| B | Ghost da DB + forcerent senza riscrivere `.dat` |

Se il mud **non** usa ghost in produzione, segnare **7.5 B = N/A** e chiudere solo **A**.
