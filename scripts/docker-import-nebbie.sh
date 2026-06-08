#!/bin/bash
# Import nebbie.sql nel MySQL del container nebbieserver + DDL S1.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DUMP="${1:-$ROOT/../nebbie.sql}"
CONTAINER="${NEBBIE_CONTAINER:-nebbieserver}"
MYSQL_USER="${MYSQL_USER:-root}"
MYSQL_PASSWORD="${MYSQL_PASSWORD:-secret}"
MYSQL_DB="${MYSQL_DB:-nebbie}"

if [ ! -f "$DUMP" ]; then
	echo "ERROR: dump non trovato: $DUMP"
	exit 1
fi

if ! docker ps --format '{{.Names}}' | grep -qx "$CONTAINER"; then
	echo "ERROR: container '$CONTAINER' non in esecuzione. Avvia prima: ./docker-run.sh up -d"
	exit 1
fi

mysql_exec() {
	docker exec -i "$CONTAINER" mysql -h 127.0.0.1 -P 3306 --protocol=TCP \
		-u"$MYSQL_USER" -p"$MYSQL_PASSWORD" "$@"
}

echo "==> Import dump: $DUMP"
mysql_exec "$MYSQL_DB" <"$DUMP"

echo "==> Rimuovi trigger legacy pwd_sync (nebbie_mas non esiste in Docker)"
mysql_exec "$MYSQL_DB" -e "DROP TRIGGER IF EXISTS pwd_sync;" 2>/dev/null || true

echo "==> Applica schema S1"
APPLY_SCHEMA_VIA_DOCKER=1 NEBBIE_CONTAINER="$CONTAINER" "$ROOT/scripts/apply-schema-s1.sh"

echo "==> Riepilogo"
mysql_exec -N "$MYSQL_DB" -e "SELECT COUNT(*) AS users FROM user; SELECT COUNT(*) AS toons FROM toon;"
echo "==> Completato"
