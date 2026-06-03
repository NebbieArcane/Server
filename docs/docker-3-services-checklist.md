# Docker a 3 servizi — checklist preparazione

Obiettivo: **sito** | **mud (nebbieserver)** | **MySQL** come tre container sulla stessa rete Docker, invece del modello attuale (mud + MySQL nello stesso container / VM Vagrant).

**Non** significa tre database server: **un** MySQL con almeno due schema (`nebbie` + DB sito).

**Stato oggi (repo):**

- `docker-compose.yml` → un solo servizio `nebbieserver` (MySQL interno + myst)
- `Vagrantfile` → stesso concetto su VM
- `Confs/vagrant.conf` → `MYSQL_HOST=localhost` (da cambiare in `mysql` o nome servizio Compose)

---

## Architettura target

```
                    rete: nebbie-net (bridge)
┌──────────────┐         ┌──────────────┐
│   website    │────────▶│              │
│  (apache/    │  :3306  │    mysql     │
│   php/…)     │         │  volume:     │
└──────────────┘         │  mysql_data  │
                         │              │
┌──────────────┐         │  DB: nebbie  │
│ nebbieserver │────────▶│  DB: sito_*  │
│  myst        │  :3306  └──────────────┘
│  volume:     │
│  mudroot/lib │
│  backups?    │
└──────────────┘
   :4000-4002, :10000
```

| Servizio Compose | Ruolo | Porte tipiche (host) |
|------------------|-------|----------------------|
| `mysql` | Solo DB | `127.0.0.1:3306:3306` (dev) |
| `mud` (ex nebbieserver) | Binario `myst`, mudlib | 4000, 4001, 4002, 10000 |
| `website` | Sito pubblico | 80 / 443 |

---

## Fase 0 — Decisioni (prima di scrivere YAML)

| # | Decisione | Note |
|---|-----------|------|
| 0.1 | Ambiente target | Solo dev locale, staging, produzione — stesso Compose con override? |
| 0.2 | Dove vive il codice **sito** | Repo separato? Path su host per `build` / `volume` |
| 0.3 | Nome DB sito | Es. `nebbie_web`, `wordpress`, … — utente MySQL dedicato o `root` solo in dev |
| 0.4 | Apple Silicon | Tenere `platform: linux/amd64` per mud se toolchain allineata a Vagrant |
| 0.5 | Vagrant vs Docker | Coesistenza (porta 3306 in conflitto) o migrazione definitiva |
| 0.6 | Segreti | `.env` (non in git): root password, DB user sito, porte |

**Gate 0:** schema su carta + nomi servizi fissati (`mysql`, `mud`, `web`).

---

## Fase 1 — Container MySQL (primo da isolare)

| # | Task | ☐ |
|---|------|---|
| 1.1 | Aggiungere servizio `mysql` con immagine ufficiale `mysql:8.0` (o 8.4) | |
| 1.2 | Volume named `mysql_data` (non bind `./mysql_data` del mud se possibile — datadir pulito) | |
| 1.3 | Variabili: `MYSQL_ROOT_PASSWORD`, `MYSQL_DATABASE=nebbie` | |
| 1.4 | Init script in `docker/mysql/init/` | |
| | → `CREATE DATABASE IF NOT EXISTS nebbie;` | |
| | → `CREATE DATABASE IF NOT EXISTS <sito_db>;` | |
| | → utente app mud: `GRANT` su `nebbie` | |
| | → utente app sito: `GRANT` su `<sito_db>` | |
| 1.5 | Healthcheck: `mysqladmin ping -h localhost` | |
| 1.6 | **Non** esporre 3306 su `0.0.0.0` in prod; in dev `127.0.0.1:3306` ok | |
| 1.7 | Import dati: dump `nebbie.sql` + dump sito al primo avvio o script `make db-import` | |
| 1.8 | Applicare DDL S1: `schema-s1-ddl-draft.sql` + script incrementali | |
| 1.9 | Test da host: `mysql -h 127.0.0.1 -P 3306 -u root -p` | |

**Gate 1:** MySQL up, due database visibili, tabelle `toon` / sito presenti.

---

## Fase 2 — Container mud (senza MySQL dentro)

| # | Task | ☐ |
|---|------|---|
| 2.1 | Nuovo `Dockerfile.mud` (o multi-stage) **senza** `mysql-server` | |
| 2.2 | Tenere toolchain: gcc-12, cmake, ODB, boost, … (come `Dockerfile` attuale) | |
| 2.3 | Build `myst` in immagine; POST_BUILD copia in `mudroot/` come oggi | |
| 2.4 | `Confs/docker.conf` o env: `MYSQL_HOST=mysql`, `MYSQL_DB=nebbie`, user/password | |
| 2.5 | Aggiornare `build.sh` / CMake per leggere host da env in build Docker | |
| 2.6 | Entrypoint mud **solo** myst (rimuovere avvio mysqld da `docker-entrypoint.sh`) | |
| 2.7 | `depends_on: mysql` + condition `service_healthy` | |
| 2.8 | Volumi bind: | |
| | `./mudroot/lib` → `/app/mudroot/lib` (players, rent, zones…) | |
| | opz. `./backups` → `/vagrant/backups` o path configurabile per `do_refund` | |
| | **Non** montare tutto `./mudroot` su `/app/mudroot` (sovrascrive `myst`) | |
| 2.9 | Porte: 4000–4002, 10000 come oggi | |
| 2.10 | `BACKUP_DIR` in codice: path env o config, non hardcoded `/vagrant/backups` | |
| 2.11 | Test: mud boot, login PG, query log senza errori SQL | |

**Gate 2:** myst si connette a `mysql:3306`, gioco funziona, nessun demone MySQL nel container mud.

---

## Fase 3 — Container sito

| # | Task | ☐ |
|---|------|---|
| 3.1 | Inventariare stack sito (PHP+Apache? Node? altro) e versione PHP | |
| 3.2 | `Dockerfile.web` minimale (solo dipendenze sito, non gcc/ODB) | |
| 3.3 | Config DB sito: host `mysql`, database `<sito_db>`, user dedicato | |
| 3.4 | Volume codice sito (bind mount repo sito o COPY in immagine per prod) | |
| 3.5 | `depends_on: mysql` (healthy) | |
| 3.6 | Porta 80/443 verso host; eventuale reverse proxy (traefik/nginx) in fase 2 | |
| 3.7 | Verificare: login sito, registrazione, link verso mud (se API condivise) | |
| 3.8 | CORS / sessioni / cookie: dominio localhost vs nomi servizi | |

**Gate 3:** sito legge/scrive solo il suo DB su `mysql`; nessun accesso file del mud necessario (salvo eccezioni documentate).

---

## Fase 4 — `docker-compose.yml` unificato

| # | Task | ☐ |
|---|------|---|
| 4.1 | Rete `nebbie-net` (driver bridge) | |
| 4.2 | Servizi: `mysql`, `mud`, `web` | |
| 4.3 | File `.env.example` committato; `.env` in `.gitignore` | |
| 4.4 | `docker-compose.override.yml` per dev (mount sorgente live, porte) | |
| 4.5 | `docker-compose.prod.yml` senza bind del sorgente, immagini versionate | |
| 4.6 | Documentare comandi: | |
| | `docker compose up -d mysql` → wait healthy → `mud` `web` | |
| | `docker compose build mud` dopo cambio C++ | |
| 4.7 | Deprecare / commentare monolite `nebbieserver` con MySQL interno | |

**Gate 4:** `docker compose up` avvia tutti e tre; README aggiornato.

---

## Fase 5 — Dati, backup, operazioni

| # | Task | ☐ |
|---|------|---|
| 5.1 | Backup MySQL: `docker exec mysql mysqldump …` per **entrambi** i DB | |
| 5.2 | Backup mudlib: tar `mudroot/lib/players`, `rent`, `backups/` | |
| 5.3 | Restore drill documentato (come cutover checklist §0.1) | |
| 5.4 | Log: `mudroot/alarmud.log` su volume o bind | |
| 5.5 | Migrazione datadir da `./mysql_data` monolite → volume mysql dedicato | |
| 5.6 | Policy `.dead`, refund, file PG — restano su volume mud (non in DB) | |

---

## Fase 6 — Config e codice da toccare (nebbietest)

| Area | Azione |
|------|--------|
| `Confs/vagrant.conf` | Duplicare profilo `docker.conf` con `MYSQL_HOST=mysql` |
| `docker-entrypoint.sh` | Split: `docker-entrypoint-mud.sh` senza MySQL |
| `src/Sql.cpp` / ODB | Host da config (già da file conf?) |
| `act.wizard.cpp` `BACKUP_DIR` | Env `REFUND_BACKUP_DIR` |
| `README.md` | Sezione “Stack 3 container” vs Vagrant |
| CI (`.travis.yml` / GitHub Actions) | Eventuale `compose build` smoke |

---

## Fase 7 — Test obbligatori (GO stack Docker)

| # | Test | ☐ |
|---|------|---|
| 7.1 | `docker compose down -v` (solo dev) → up → DB init ok | |
| 7.2 | Mud: creazione PG, save, quit, re-login | |
| 7.3 | Mud: pensione / rent (file + DB se migrato) | |
| 7.4 | Sito: pagine principali + login | |
| 7.5 | Sito + mud: stesso utente/PG coerente (se integrati) | |
| 7.6 | `legacyimport` / `legacyprobe` contro `mysql` | |
| 7.7 | `do_refund` con backup su volume montato | |
| 7.8 | Restart solo `mud`: sito e DB restano up | |
| 7.9 | Restart `mysql`: mud riconnette dopo healthy (o retry) | |

---

## Ordine consigliato (minimo rischio)

1. **Fase 0** decisioni  
2. **Fase 1** MySQL standalone + import dump  
3. **Fase 2** mud punta a MySQL esterno (puoi tenere Vagrant parallelamente su altra porta)  
4. **Fase 3** sito  
5. **Fase 4–7** compose finale, backup, test  

**Non** partire dal sito se il mud non è stabile su DB esterno.

---

## Cosa **non** fare subito

- Due container MySQL (uno per sito, uno per mud) — salvo requisiti legali/ops  
- Montare l’intero repo su `/app` nel container mud in produzione  
- Committare password in `docker-compose.yml` (usare `.env`)  
- Esporre MySQL su Internet senza firewall  

---

## Riferimenti repo

| File | Uso attuale |
|------|-------------|
| `docker-compose.yml` | Monolite mud+MySQL |
| `Dockerfile` | Build mud + install mysql-server |
| `docker-entrypoint.sh` | Init MySQL + avvio myst |
| `Vagrantfile` | Parità dev x86 |
| `docs/cutover-db-checklist.md` | Migrazione dati PG file → DB |

---

## Stato (aggiornare a mano)

| Blocco | Pronto? |
|--------|---------|
| Decisioni §0 | ☐ |
| MySQL container | ☐ |
| Mud senza MySQL interno | ☐ |
| Sito container | ☐ |
| Compose unificato | ☐ |
| Test §7 | ☐ |
| Prod / deploy | ☐ |
