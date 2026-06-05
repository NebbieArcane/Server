#!/bin/bash
# Applica DDL cutover S1 su MySQL (database nebbie).
# Uso: ./scripts/apply-schema-s1.sh
#      vagrant ssh -c '/vagrant/scripts/apply-schema-s1.sh'
set -euo pipefail

MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
MYSQL_USER="${MYSQL_USER:-root}"
MYSQL_PASSWORD="${MYSQL_PASSWORD:-secret}"
MYSQL_DB="${MYSQL_DB:-nebbie}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

mysql_base() {
	mysql -h "$MYSQL_HOST" -u"$MYSQL_USER" -p"$MYSQL_PASSWORD" "$@"
}

echo "==> DDL S1 su ${MYSQL_DB}"
mysql_base -D "$MYSQL_DB" <"$ROOT/docs/schema-s1-ddl-draft.sql"

for inc in \
	schema-s1-ddl-add-resistance.sql \
	schema-s1-ddl-drop-unused-conditions.sql \
	schema-s1-ddl-drop-extra-str.sql \
	schema-s1-ddl-fix-character-classes.sql \
	schema-s1-death-snapshot.sql \
	schema-s1-inventory-soft-delete.sql \
	schema-toon-nuke-blacklist.sql
do
	echo "==> incrementale ${inc} (errori ignorati se già applicato)"
	mysql_base -D "$MYSQL_DB" <"$ROOT/docs/$inc" 2>/dev/null || true
done

if mysql_base -N -D "$MYSQL_DB" -e \
	"SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA='${MYSQL_DB}' AND TABLE_NAME='toon';" | grep -q '^1$'; then
	echo "==> flag cutover su toon"
	mysql_base -D "$MYSQL_DB" <"$ROOT/docs/schema-s1-toon-migration-flags.sql" 2>/dev/null || true
else
	echo "==> skip migrated_at: tabella toon assente"
fi

echo "==> Tabelle character_*:"
mysql_base -N -D "$MYSQL_DB" -e "SHOW TABLES LIKE 'character_%';"
echo "==> Completato"
