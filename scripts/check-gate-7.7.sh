#!/bin/bash
# Gate 7.7: snapshot mtime file PG (prima/dopo sessione migrata).
# Uso:
#   ./scripts/check-gate-7.7.sh alar before > /tmp/alar-mtime-before.txt
#   ... gioco + quit ...
#   ./scripts/check-gate-7.7.sh alar after  > /tmp/alar-mtime-after.txt
#   diff -u /tmp/alar-mtime-before.txt /tmp/alar-mtime-after.txt
set -euo pipefail

NAME_LOWER="$(echo "${1:-}" | tr '[:upper:]' '[:lower:]')"
PHASE="${2:-snap}"
[ -n "$NAME_LOWER" ] || { echo "Uso: $0 <nome_pg> [before|after|snap]" >&2; exit 1; }

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIB="${LIB:-$ROOT/mudroot/lib}"
DAT="$LIB/players/${NAME_LOWER}.dat"
RENT="$LIB/rent/${NAME_LOWER}"
AUX="$LIB/rent/${NAME_LOWER}.aux"
DEAD="$LIB/players/${NAME_LOWER}.dead"

echo "# gate 7.7 phase=$PHASE pg=$NAME_LOWER $(date -Iseconds)"
for f in "$DAT" "$RENT" "$AUX" "$DEAD"; do
	if [ -e "$f" ]; then
		stat -f '%m %z %N' "$f" 2>/dev/null || stat -c '%Y %s %n' "$f"
	else
		echo "missing $f"
	fi
done
