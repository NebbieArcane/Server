[![Build Status](https://api.travis-ci.org/NebbieArcane/Server.svg)](https://travis-ci.org/NebbieArcane/Server)
# Nebbie Arcane
## Server code
* Getting started
    * FIRST TIME
        * apt install vagrant
        * vagrant plugin install vagrant-timezone
        * ./getworld
        * vagrant up
        * vagrant ssh
    * DAY BY DAY
        * vagrant up
        * vagrant ssh
        * cd /vagrant
        * make
        * cd mudroot
        * ./myst
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
* docker-run.sh auto-detects the host architecture (Intel or Apple Silicon) and sets the correct platform
* On Apple Silicon, ODB files (src/odb/account-odb*.cxx) are pre-generated in the repo. If missing, docker-run.sh regenerates them automatically before building
* MySQL runs inside the container, same as Vagrant. Data is persisted in the mud_data Docker volume
* The server listens on 127.0.0.1 only by default (see docker-compose.yml to expose it publicly)
* MySQL migration helper (legacy MySQL 5.7 datadir -> MySQL 8): `./docker_migrate_mysql57_to_8.sh`
* Vagrant/MySQL migration helper (no Docker, import da dump): `./vagrant_migrate_mysql57_to_8.sh /path/to/dump.sql`
