#!/bin/bash
# Wrapper di docker compose con rilevamento automatico architettura host.
#
# Su Apple Silicon (ARM64): forza linux/amd64 per allinearsi all'ambiente
# x86_64 (Vagrant/CI). Se mancano i file ODB, li genera in un container ARM64.
# Su Intel/AMD (x86_64): usa linux/amd64 nativo.
#
# Uso:
#   ./docker-run.sh up            # avvia sulla porta di default (4000)
#   ./docker-run.sh up -d         # avvia in background
#   SERVER_PORT=4002 ./docker-run.sh up
#   ./docker-run.sh down
#   ./docker-run.sh build

set -e

ARCH=$(uname -m)
OS=$(uname -s)

echo "Host: $OS / $ARCH"

if [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then
    # Apple Silicon (M1/M2/M3) o Linux ARM64
    export DOCKER_PLATFORM=linux/amd64

    # Se i file ODB non sono presenti nel workspace, li generiamo localmente
    # in un container ARM64 nativo prima della build linux/amd64.
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
            ubuntu:24.04 \
            sh -c "
                apt-get update -qq 2>/dev/null
                apt-get install -qq -y g++ odb libodb-dev libodb-boost-dev \
                    libodb-mysql-dev libboost-dev libboost-date-time-dev 2>/dev/null
                cd /src/odb && odb \
                    --profile boost/smart-ptr --profile boost/date-time \
                    --std c++17 -m dynamic -d common -d mysql \
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
