# Gate 7.8 — Restore drill (backup file → re-import)

Automazione: `scripts/restore-pg-drill.sh`

## PG consigliati

| PG | Perché |
|----|--------|
| **montero** | `migrated_at` NULL — drill import pulito da file |
| **theprodigy** | già migrato — drill “disaster recovery” (reset DB + re-import) |

## Procedura (es. montero)

Mud **up**, immortale con `legacyimport`.

```bash
# host o vagrant ssh
cd /vagrant   # in VM

./scripts/restore-pg-drill.sh montero backup
# annota BACKUP_DIR stampato

./scripts/restore-pg-drill.sh montero reset-db
```

In gioco:

```text
legacyimport montero
```

Poi:

```bash
./scripts/restore-pg-drill.sh montero verify
./scripts/gate-sql.sh montero migration
```

## Variante PG già migrato (theprodigy)

Stessi passi; dopo `reset-db` il PG non carica più da DB finché non rifai `legacyimport theprodigy`.

Opzionale corruzione file simulata:

```bash
./scripts/restore-pg-drill.sh theprodigy backup
./scripts/restore-pg-drill.sh theprodigy reset-db
./scripts/restore-pg-drill.sh theprodigy restore-files   # ripristina .dat/rent/.aux dal backup
legacyimport theprodigy
./scripts/restore-pg-drill.sh theprodigy verify
```

## PASS

- `legacyimport` OK in log
- `verify` → `PASS 7.8`
- `migrated_at` valorizzato, `character_core` ≥ 1 riga
- Login menu 1 → gioco normale

## §0.6 runbook

Questo script **è** il runbook operativo §0.6 per un singolo PG.
