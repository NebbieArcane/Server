#!/bin/bash
# Wrapper di docker compose con rilevamento automatico dell'architettura host.
#
# Su Apple Silicon (ARM64): forza l'immagine linux/amd64 via QEMU e verifica
# che i file ODB siano pre-generati (non possono essere generati sotto QEMU).
# Su Intel/AMD (x86_64): usa linux/amd64 nativo, senza emulazione.
#
# Uso:
#   ./run.sh up            # avvia sulla porta di default (4000)
#   ./run.sh up -d         # avvia in background
#   SERVER_PORT=4002 ./run.sh up
#   ./run.sh down
#   ./run.sh build

set -e

ARCH=$(uname -m)
OS=$(uname -s)

echo "Host: $OS / $ARCH"

if [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then
    # Apple Silicon (M1/M2/M3) o Linux ARM64
    export DOCKER_PLATFORM=linux/amd64

    # I file ODB non possono essere generati sotto QEMU x86_64 (bug cc1plus).
    # Se mancano, li generiamo con un container ARM64 nativo.
    ODB_FILES=(
        src/odb/account-odb.cxx
        src/odb/account-odb-mysql.cxx
        src/odb/account-schema-mysql.cxx
    )
    NEEDS_ODB=false
    for f in "${ODB_FILES[@]}"; do
        if [ ! -f "$(dirname "$0")/$f" ]; then
            NEEDS_ODB=true
            break
        fi
    done

    if [ "$NEEDS_ODB" = "true" ]; then
        echo "File ODB mancanti — generazione su container ARM64 nativo..."
        docker run --rm --platform linux/arm64 \
            -v "$(cd "$(dirname "$0")" && pwd)/src:/src" \
            ubuntu:bionic \
            sh -c "
                apt-get update -qq 2>/dev/null
                apt-get install -qq -y g++ odb libodb-dev libodb-boost-dev \
                    libodb-mysql-dev libboost-dev libboost-date-time-dev 2>/dev/null
                cd /src/odb && odb \
                    --profile boost/smart-ptr --profile boost/date-time \
                    --std c++11 -m dynamic -d common -d mysql \
                    --generate-query --generate-prepared --show-sloc \
                    --generate-session --generate-schema \
                    --schema-format separate --at-once \
                    --schema-name account --input-name account account.hpp
            "
        echo "File ODB generati."
    fi

else
    # Intel/AMD x86_64 (Mac o Linux)
    export DOCKER_PLATFORM=linux/amd64
fi

echo "DOCKER_PLATFORM=$DOCKER_PLATFORM"
docker compose "$@"
