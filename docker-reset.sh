#!/bin/bash
# Reset completo stack Docker locale (container + volumi + immagine).
# NON tocca ./mysql_data/ né ./mudroot/lib/ sul Mac.
set -e
cd "$(dirname "$0")"

echo "==> Stop e rimozione container/volumi Compose..."
docker compose down -v --remove-orphans 2>/dev/null || true
docker rm -f nebbieserver 2>/dev/null || true

echo "==> Rimozione volumi MySQL residui..."
docker volume rm server_nebbie_mysql_data nebbie_mysql_data 2>/dev/null || true

echo "==> Rimozione immagine locale..."
docker rmi server-nebbieserver:latest 2>/dev/null || true

echo "==> Build immagine..."
./docker-run.sh build

echo "==> Avvio container..."
./docker-run.sh up -d

echo "==> Attesa avvio (MySQL + myst)..."
for i in $(seq 1 90); do
  if docker logs nebbieserver 2>&1 | grep -q "Starting application:"; then
    sleep 5
    break
  fi
  if docker logs nebbieserver 2>&1 | grep -q "ERROR: MySQL startup timed out"; then
    echo "ERRORE: MySQL non partito. Log:"
    docker logs nebbieserver 2>&1 | tail -30
    exit 1
  fi
  sleep 2
done

echo
docker ps --filter name=nebbieserver --format 'Container: {{.Names}} | {{.Status}} | {{.Ports}}'
echo
echo "Prova: telnet localhost 4000"
echo "Log live: docker logs -f nebbieserver"
