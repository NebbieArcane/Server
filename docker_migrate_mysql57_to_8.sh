#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMPOSE_FILE="${PROJECT_DIR}/docker-compose.yml"

OLD_DATA_DIR="${PROJECT_DIR}/mysql_data"
MIGRATION_ROOT="${PROJECT_DIR}/migration_artifacts"
STAMP="$(date +%Y%m%d_%H%M%S)"
WORK_DIR="${MIGRATION_ROOT}/mysql57_work_${STAMP}"
BACKUP_DIR="${MIGRATION_ROOT}/mysql_data_backup_${STAMP}"
DUMP_FILE="${MIGRATION_ROOT}/nebbie_dump_${STAMP}.sql"

TEMP_CONT="nebbie_mysql57_recovery_${STAMP}"
MYSQL57_IMAGE="mysql:5.7"
MYSQL57_PORT="3307"
MYSQL57_ROOT_PWD="secret"

MYSQL8_ROOT_PWD="secret"
MYSQL_DB="nebbie"

cleanup() {
  docker rm -f "${TEMP_CONT}" >/dev/null 2>&1 || true
}
trap cleanup EXIT

echo "[1/9] Pre-check..."
if ! command -v docker >/dev/null 2>&1; then
  echo "ERROR: docker non trovato nel PATH."
  exit 1
fi

if [ ! -d "${OLD_DATA_DIR}" ]; then
  echo "ERROR: cartella mysql_data non trovata: ${OLD_DATA_DIR}"
  exit 1
fi

mkdir -p "${MIGRATION_ROOT}"

echo "[2/9] Stop stack corrente..."
docker compose -f "${COMPOSE_FILE}" down || true

echo "[3/9] Backup datadir originale..."
cp -a "${OLD_DATA_DIR}" "${BACKUP_DIR}"
cp -a "${OLD_DATA_DIR}" "${WORK_DIR}"
echo "Backup salvato in: ${BACKUP_DIR}"

echo "[4/9] Avvio MySQL 5.7 recovery su porta ${MYSQL57_PORT}..."
docker run -d \
  --name "${TEMP_CONT}" \
  -p "${MYSQL57_PORT}:3306" \
  -v "${WORK_DIR}:/var/lib/mysql" \
  "${MYSQL57_IMAGE}" \
  --innodb-force-recovery=1 \
  --skip-networking=0 \
  --bind-address=0.0.0.0 >/dev/null

echo "[5/9] Attesa startup MySQL 5.7..."
for i in $(seq 1 90); do
  if docker exec "${TEMP_CONT}" mysqladmin ping -uroot -p"${MYSQL57_ROOT_PWD}" >/dev/null 2>&1; then
    MYSQL57_AUTH=(-uroot -p"${MYSQL57_ROOT_PWD}")
    break
  fi
  if docker exec "${TEMP_CONT}" mysqladmin ping -uroot >/dev/null 2>&1; then
    MYSQL57_AUTH=(-uroot)
    break
  fi
  sleep 1
done

if [ "${MYSQL57_AUTH+x}" != "x" ]; then
  echo "ERROR: MySQL 5.7 recovery non è partito correttamente."
  docker logs "${TEMP_CONT}" || true
  exit 1
fi

echo "[6/9] Dump database '${MYSQL_DB}'..."
docker exec "${TEMP_CONT}" \
  mysqldump \
    "${MYSQL57_AUTH[@]}" \
    --skip-lock-tables \
    --default-character-set=utf8mb4 \
    --routines \
    --triggers \
    --events \
    --databases "${MYSQL_DB}" > "${DUMP_FILE}"

if [ ! -s "${DUMP_FILE}" ]; then
  echo "ERROR: dump vuoto/non creato: ${DUMP_FILE}"
  exit 1
fi
echo "Dump creato in: ${DUMP_FILE}"

echo "[7/9] Reset datadir MySQL 8 e avvio stack..."
rm -rf "${OLD_DATA_DIR}"
mkdir -p "${OLD_DATA_DIR}"

docker compose -f "${COMPOSE_FILE}" up -d

echo "[8/9] Attesa MySQL 8 nel container nebbieserver..."
for i in $(seq 1 90); do
  if docker compose -f "${COMPOSE_FILE}" exec -T nebbieserver \
      mysqladmin ping -h 127.0.0.1 -P 3306 --protocol=TCP -uroot -p"${MYSQL8_ROOT_PWD}" >/dev/null 2>&1; then
    break
  fi
  sleep 1
done

if ! docker compose -f "${COMPOSE_FILE}" exec -T nebbieserver \
    mysqladmin ping -h 127.0.0.1 -P 3306 --protocol=TCP -uroot -p"${MYSQL8_ROOT_PWD}" >/dev/null 2>&1; then
  echo "ERROR: MySQL 8 non raggiungibile nel container nebbieserver."
  docker compose -f "${COMPOSE_FILE}" logs --no-color nebbieserver || true
  exit 1
fi

echo "[9/9] Import dump nel nuovo database..."
docker compose -f "${COMPOSE_FILE}" exec -T nebbieserver \
  mysql -h 127.0.0.1 -P 3306 --protocol=TCP -uroot -p"${MYSQL8_ROOT_PWD}" < "${DUMP_FILE}"

echo
echo "Migrazione completata."
echo "- Backup originale: ${BACKUP_DIR}"
echo "- Dump SQL: ${DUMP_FILE}"
echo "- Datadir di lavoro 5.7: ${WORK_DIR}"
