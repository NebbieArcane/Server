#!/usr/bin/env bash
set -euo pipefail

# Vagrant/MySQL migration helper (no Docker).
# Usage:
#   ./vagrant_migrate_mysql57_to_8.sh /path/to/dump.sql
#   ./vagrant_migrate_mysql57_to_8.sh /path/to/dump.sql.gz
#
# It will:
# 1) Create a safety backup of current DB
# 2) Recreate target DB
# 3) Import the provided dump

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MIGRATION_ROOT="${PROJECT_DIR}/migration_artifacts"
STAMP="$(date +%Y%m%d_%H%M%S)"

MYSQL_USER="${MYSQL_USER:-root}"
MYSQL_PASSWORD="${MYSQL_PASSWORD:-secret}"
MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
MYSQL_PORT="${MYSQL_PORT:-3306}"
MYSQL_DB="${MYSQL_DB:-nebbie}"

INPUT_DUMP="${1:-}"
BACKUP_FILE="${MIGRATION_ROOT}/vagrant_${MYSQL_DB}_backup_${STAMP}.sql"
PREPARED_DUMP="${MIGRATION_ROOT}/vagrant_import_${STAMP}.sql"

die() {
  echo "ERROR: $*" >&2
  exit 1
}

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "comando non trovato: $1"
}

mysql_cmd() {
  mysql -h "${MYSQL_HOST}" -P "${MYSQL_PORT}" -u"${MYSQL_USER}" -p"${MYSQL_PASSWORD}" "$@"
}

mysqldump_cmd() {
  mysqldump -h "${MYSQL_HOST}" -P "${MYSQL_PORT}" -u"${MYSQL_USER}" -p"${MYSQL_PASSWORD}" "$@"
}

need_cmd mysql
need_cmd mysqldump
need_cmd gzip

[ -n "${INPUT_DUMP}" ] || die "specifica un dump SQL (.sql o .sql.gz)"
[ -f "${INPUT_DUMP}" ] || die "dump non trovato: ${INPUT_DUMP}"

mkdir -p "${MIGRATION_ROOT}"

echo "[1/5] Verifica connessione MySQL..."
mysql_cmd -e "SELECT 1;" >/dev/null

echo "[2/5] Backup di sicurezza DB corrente (${MYSQL_DB})..."
if mysql_cmd -Nse "SHOW DATABASES LIKE '${MYSQL_DB}'" | grep -q "${MYSQL_DB}"; then
  mysqldump_cmd --default-character-set=utf8mb4 --routines --triggers --events --databases "${MYSQL_DB}" > "${BACKUP_FILE}"
  echo "Backup creato: ${BACKUP_FILE}"
else
  echo "Database ${MYSQL_DB} non presente: salto backup."
fi

echo "[3/5] Preparo dump di import..."
case "${INPUT_DUMP}" in
  *.sql)
    cp -f "${INPUT_DUMP}" "${PREPARED_DUMP}"
    ;;
  *.sql.gz)
    gzip -dc "${INPUT_DUMP}" > "${PREPARED_DUMP}"
    ;;
  *)
    die "formato non supportato: usa .sql o .sql.gz"
    ;;
esac

[ -s "${PREPARED_DUMP}" ] || die "dump preparato vuoto: ${PREPARED_DUMP}"

echo "[4/5] Ricreo database ${MYSQL_DB}..."
mysql_cmd -e "DROP DATABASE IF EXISTS \`${MYSQL_DB}\`; CREATE DATABASE \`${MYSQL_DB}\` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;"

echo "[5/5] Import dump..."
mysql_cmd "${MYSQL_DB}" < "${PREPARED_DUMP}"

echo
echo "Migrazione Vagrant completata."
echo "- Backup sicurezza: ${BACKUP_FILE:-<non creato>}"
echo "- Dump importato:   ${PREPARED_DUMP}"
