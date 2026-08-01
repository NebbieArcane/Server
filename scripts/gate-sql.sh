#!/bin/bash
# Query MySQL cutover per gate 7.6 / 7.8 (Vagrant o host con client mysql).
# Credenziali: Confs/{ENVIRONMENT}.conf (default devel), come build.sh.
# Uso: ./scripts/gate-sql.sh <nome_pg> [prefs|ach|alias|migration|all]
set -euo pipefail

NAME="${1:-}"
MODE="${2:-all}"
[ -n "$NAME" ] || { echo "Uso: $0 <nome_pg> [prefs|ach|alias|migration|all]" >&2; exit 1; }

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ENVIRONMENT="${ENVIRONMENT:-devel}"
# shellcheck source=scripts/load-mysql-conf.sh
source "${ROOT}/scripts/load-mysql-conf.sh"
load_mysql_conf

mysql_q() {
	mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u"$MYSQL_USER" -p"$MYSQL_PASSWORD" -N -D "$MYSQL_DB" "$@"
}

TOON_ID="$(mysql_q -e "SELECT id FROM toon WHERE name='${NAME}' LIMIT 1")"
if [ -z "$TOON_ID" ]; then
	echo "toon '${NAME}' non trovato" >&2
	exit 1
fi

echo "# gate-sql pg=${NAME} id=${TOON_ID} $(date -Iseconds)"

case "$MODE" in
	migration|all)
		mysql_q -e "SELECT id, name, migrated_at, schema_version FROM toon WHERE id=${TOON_ID}"
		mysql_q -e "SELECT 'character_core' t, COUNT(*) c FROM character_core WHERE toon_id=${TOON_ID}
			UNION SELECT 'character_stats', COUNT(*) FROM character_stats WHERE toon_id=${TOON_ID}
			UNION SELECT 'character_rent', COUNT(*) FROM character_rent WHERE toon_id=${TOON_ID}
			UNION SELECT 'character_prefs', COUNT(*) FROM character_prefs WHERE toon_id=${TOON_ID}
			UNION SELECT 'character_achievements', COUNT(*) FROM character_achievements WHERE toon_id=${TOON_ID}
			UNION SELECT 'character_aliases', COUNT(*) FROM character_aliases WHERE toon_id=${TOON_ID}
			UNION SELECT 'character_mercy', COUNT(*) FROM character_mercy WHERE toon_id=${TOON_ID}
			UNION SELECT 'character_death_snapshot', COUNT(*) FROM character_death_snapshot WHERE toon_id=${TOON_ID}"
		;;
esac

case "$MODE" in
	prefs|all)
		echo "# character_prefs"
		mysql_q -e "SELECT pref_key, LEFT(pref_value,80) FROM character_prefs WHERE toon_id=${TOON_ID} ORDER BY pref_key"
		;;
esac

case "$MODE" in
	ach|all)
		echo "# character_achievements (sample)"
		mysql_q -e "SELECT category, slot_index, value FROM character_achievements WHERE toon_id=${TOON_ID} ORDER BY category, slot_index LIMIT 20"
		mysql_q -e "SELECT COUNT(*) ach_total FROM character_achievements WHERE toon_id=${TOON_ID}"
		;;
esac

case "$MODE" in
	alias|all)
		echo "# character_aliases"
		mysql_q -e "SELECT slot, LEFT(alias_text,60) FROM character_aliases WHERE toon_id=${TOON_ID} ORDER BY slot"
		;;
esac
