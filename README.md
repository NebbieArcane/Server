[![Build Status](https://api.travis-ci.org/NebbieArcane/Server.svg)](https://travis-ci.org/NebbieArcane/Server)
# Nebbie Arcane
## Server code
## instructions
* Getting started
    * FIRST TIME
        1 Install Docker
        2 `./runEnv` Verranno scaricate le immagini dei container, poi apparirà il prompt
        3 `./build.sh` Esegue la prtima compilazione e genera i Makefile, se non aggiungete file nuovi non serve piu'
        4 ci sono 3 servizi:
            * Il prompt da cui si puo' compilare e lanciare il mud
            * Adminer all'indirizzo :8080
            * Mysql sulla porta 3306
    *   DAY BY DAY
        Ignorate gli step 1 e 3
    * Il branching model è git floww, le build verranno fatte da github:
        * Alla chiusura di un branch di release `git flow release finish` la versione taggata andrà automaticamente su release
        * Alla chusura di un feature `git flow feature finish` su develop, develop andrà su master (che darà rinominata staging)
        * Al push `git flow feature publish` di un feature, la feature (se selezionata da ci) andrà su devel

## Old instructions
* Getting started
    * FIRST TIME
        * apt install vagrant
        * vagrant plugin install vagrant-timezone
        * ./getworld
        * vagrant up   # VM Ubuntu 24.04 Noble (box bento/ubuntu-24.04, allineata a Docker)
        * vagrant ssh
    * DAY BY DAY
        * vagrant up
        * vagrant ssh
        * cd /vagrant
        * ./build.sh vagrant
        * cd mudroot
        * ./myst
    * UPGRADE da VM Jammy (22.04) a Noble
        * vagrant destroy -f
        * vagrant box remove ubuntu/jammy64   # opzionale
        * vagrant up
    * NOTA box: da Ubuntu 24.04 Canonical non pubblica più `ubuntu/*` su Vagrant Cloud;
      usiamo `bento/ubuntu-24.04` (Noble vanilla, VirtualBox amd64/arm64).
    * Provision fallito o interrotto
        * `vagrant up` **non** rilancia il provision se la VM esiste già
        * Riprendi con: `vagrant up --provision` oppure `vagrant provision`
        * ODB parziale resta in `/var/cache/nebbie-odb-build` (riprende da dove era)
        * Log in VM: `vagrant ssh -c 'sudo tail -100 /var/log/nebbie-vagrant-provision.log'`
        * Per rifare tutto ODB: `vagrant ssh` poi `sudo rm /var/lib/nebbie/odb-toolchain-installed`
    * In emergency (or when you add a new header or code file to src):
        * ./build.sh # to rebuild the makefiles

## IMPORTANT
* ALL FILES inside src will be taken and compiled in myst. Do not put under src unrelated files.
 use utilities for this
* The /vagrant folder in the vagrant machine is mounted from the folder where the vagrantfile resides and is synced 2-way

---

## Roadmap di modernizzazione
* `roadmap-modernizzazione.html` — apri nel browser, niente dipendenze
* `roadmap-modernizzazione.canvas.tsx` — versione interattiva per Cursor:
    * Copia il file in `~/.cursor/projects/<tuo-workspace>/canvases/`
    * Aprilo in Cursor con **Open Canvas**

---

## Docker
* Getting started
    * FIRST TIME
        * install Docker Desktop from https://www.docker.com/products/docker-desktop/
        * ./getworld
        * ./docker-run.sh build
        * ./docker-run.sh up
    * DAY BY DAY
        * ./docker-run.sh up
        * ./docker-run.sh down
    * To start on a custom port:
        * SERVER_PORT=4001 ./docker-run.sh up
        * SERVER_PORT=4002 ./docker-run.sh up

## NOTES (Docker)
* Immagine base: **Ubuntu 24.04 (Noble)** con MySQL 8, GCC 12 e toolchain ODB via build2
* La build scarica ODB da `pkg.cppget.org` (script `scripts/install-odb-toolchain.sh`, retry DNS). Serve rete durante `docker compose build`; se fallisce, riprova o verifica DNS/firewall Docker Desktop
* docker-run.sh auto-detects the host architecture (Intel or Apple Silicon) and sets the correct platform
* On Apple Silicon, ODB files (src/odb/account-odb*.cxx) are pre-generated in the repo. If missing, docker-run.sh regenerates them automatically before building
* MySQL runs inside the container, same as Vagrant. Data is persisted in the mud_data Docker volume
* The server listens on 127.0.0.1 only by default (see docker-compose.yml to expose it publicly)
* MySQL migration helper (legacy MySQL 5.7 datadir -> MySQL 8): `./docker_migrate_mysql57_to_8.sh`
* Vagrant/MySQL migration helper (no Docker, import da dump): `./vagrant_migrate_mysql57_to_8.sh /path/to/dump.sql`
