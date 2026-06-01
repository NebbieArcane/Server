#!/bin/bash
# Gate 7.8 — restore drill: backup file PG → reset DB migrazione → legacyimport in-game.
# Uso:
#   ./scripts/restore-pg-drill.sh <nome> backup
#   vagrant ssh -c '/vagrant/scripts/restore-pg-drill.sh montero reset-db'
#   # in mud (immortale): legacyimport <nome>
#   ./scripts/restore-pg-drill.sh <nome> verify
set -euo pipefail

NAME_LOWER="$(echo "${1:-}" | tr '[:upper:]' '[:lower:]')"
ACTION="${2:-}"
[ -n "$NAME_LOWER" ] && [ -n "$ACTION" ] || {
	echo "Uso: $0 <nome_pg> backup|reset-db|verify|restore-files" >&2
	exit 1
}

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GATE_SQL="$ROOT/scripts/gate-sql.sh"
LIB="${LIB:-$ROOT/mudroot/lib}"
PLAYERS="$LIB/players"
RENT="$LIB/rent"
STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP_DIR="$ROOT/backups/drill-${NAME_LOWER}-${STAMP}"

MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
MYSQL_USER="${MYSQL_USER:-root}"
MYSQL_PASSWORD="${MYSQL_PASSWORD:-secret}"
MYSQL_DB="${MYSQL_DB:-nebbie}"

mysql_q() {
	mysql -h "$MYSQL_HOST" -u"$MYSQL_USER" -p"$MYSQL_PASSWORD" -N -D "$MYSQL_DB" "$@"
}

toon_id() {
	mysql_q -e "SELECT id FROM toon WHERE name='${NAME_LOWER}' LIMIT 1"
}

backup_files() {
	mkdir -p "$BACKUP_DIR/players" "$BACKUP_DIR/rent"
	for f in "$PLAYERS/${NAME_LOWER}.dat" "$PLAYERS/${NAME_LOWER}.dead"; do
		if [ -e "$f" ]; then
			cp -a "$f" "$BACKUP_DIR/players/"
			echo "copied $f"
		else
			echo "skip missing $f"
		fi
	done
	for f in "$RENT/${NAME_LOWER}" "$RENT/${NAME_LOWER}.aux"; do
		if [ -e "$f" ]; then
			cp -a "$f" "$BACKUP_DIR/rent/"
			echo "copied $f"
		else
			echo "skip missing $f"
		fi
	done
	"$GATE_SQL" "$NAME_LOWER" all >"$BACKUP_DIR/gate-sql-before.txt" 2>/dev/null || true
	echo "BACKUP_DIR=$BACKUP_DIR"
	mkdir -p "$ROOT/backups"
	echo "$BACKUP_DIR" >"$ROOT/backups/.last-drill-${NAME_LOWER}"
}

reset_db() {
	TID="$(toon_id)"
	[ -n "$TID" ] || { echo "toon ${NAME_LOWER} assente" >&2; exit 1; }
	echo "reset-db toon_id=$TID"
	mysql_q -e "
		UPDATE toon SET migrated_at = NULL, schema_version = 0 WHERE id = ${TID};
		DELETE FROM character_affects WHERE toon_id = ${TID};
		DELETE FROM character_skills WHERE toon_id = ${TID};
		DELETE FROM character_classes WHERE toon_id = ${TID};
		DELETE FROM character_stats WHERE toon_id = ${TID};
		DELETE FROM character_core WHERE toon_id = ${TID};
		DELETE FROM character_rent WHERE toon_id = ${TID};
		DELETE FROM character_prefs WHERE toon_id = ${TID};
		DELETE FROM character_achievements WHERE toon_id = ${TID};
		DELETE FROM character_aliases WHERE toon_id = ${TID};
		DELETE FROM character_mercy WHERE toon_id = ${TID};
		DELETE FROM character_death_snapshot WHERE toon_id = ${TID};
	" 2>/dev/null || true
	echo "OK: migrated_at cleared, character_* vuote. Ora: legacyimport ${NAME_LOWER} in-game (mud up)."
}

restore_files() {
	LAST="$ROOT/backups/.last-drill-${NAME_LOWER}"
	[ -f "$LAST" ] || { echo "nessun backup recente; esegui prima: $0 ${NAME_LOWER} backup" >&2; exit 1; }
	BACKUP_DIR="$(cat "$LAST")"
	[ -d "$BACKUP_DIR" ] || { echo "backup dir mancante: $BACKUP_DIR" >&2; exit 1; }
	[ -d "$BACKUP_DIR/players" ] && cp -a "$BACKUP_DIR/players/"* "$PLAYERS/" 2>/dev/null || true
	[ -d "$BACKUP_DIR/rent" ] && cp -a "$BACKUP_DIR/rent/"* "$RENT/" 2>/dev/null || true
	echo "file ripristinati da $BACKUP_DIR"
}

verify() {
	[ -x "$GATE_SQL" ] || chmod +x "$GATE_SQL" 2>/dev/null || true
	"$GATE_SQL" "$NAME_LOWER" migration
	MIG="$(mysql_q -e "SELECT migrated_at IS NOT NULL FROM toon WHERE name='${NAME_LOWER}'")"
	CORE="$(mysql_q -e "SELECT COUNT(*) FROM character_core WHERE toon_id=$(toon_id)")"
	if [ "$MIG" = "1" ] && [ "${CORE:-0}" -ge 1 ]; then
		echo "PASS 7.8: migrated_at set e character_core presente"
		exit 0
	fi
	echo "FAIL 7.8: migrated_at=$MIG character_core rows=$CORE" >&2
	exit 1
}

case "$ACTION" in
	backup) backup_files ;;
	reset-db) reset_db ;;
	restore-files) restore_files ;;
	verify) verify ;;
	*)
		echo "azione sconosciuta: $ACTION" >&2
		exit 1
		;;
esac
