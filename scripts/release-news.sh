#!/bin/bash
# Inserisce una news (e opzionalmente wiznews) per una release.
# Uso:
#   ./scripts/release-news.sh
#   TAG=r3.7.0 ./scripts/release-news.sh --message "Procarea tuning"
#   ./scripts/release-news.sh --dry-run --pages-only
#   ./scripts/release-news.sh --wiznews
#
# Credenziali MySQL: Confs/{ENVIRONMENT}.conf (come build.sh), override con env MYSQL_*.
#   ./scripts/release-news.sh --env devel
#   ENVIRONMENT=release ./scripts/release-news.sh --mysql-only
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PAGES_NEWS="${ROOT}/pages/news"
PAGES_WIZNEWS="${ROOT}/pages/wiznews"

ENVIRONMENT="${ENVIRONMENT:-devel}"

DRY_RUN=false
PAGES_ONLY=false
MYSQL_ONLY=false
INCLUDE_WIZNEWS=false
VERSION=""
MESSAGE=""
TAG="${TAG:-}"
AUTHOR=""

usage() {
	sed -n '2,12p' "$0"
	echo "Opzioni:"
	echo "  --dry-run       mostra azioni senza scrivere"
	echo "  --pages-only    aggiorna solo pages/news (e wiznews se --wiznews)"
	echo "  --mysql-only    INSERT MySQL, non tocca i file in pages/"
	echo "  --wiznews       aggiorna anche wiznews (kind=1)"
	echo "  --version V     versione esplicita (default: TAG o ultimo tag r*)"
	echo "  --message M     testo headline dopo la versione (default: subject ultimo commit)"
	echo "  --env E         profilo Confs (default: devel; es. staging, release)"
	echo "  --tag T         alias di TAG= (es. r3.7.0)"
	exit "${1:-0}"
}

while [ $# -gt 0 ]; do
	case "$1" in
		-h|--help) usage 0 ;;
		--dry-run) DRY_RUN=true ;;
		--pages-only) PAGES_ONLY=true ;;
		--mysql-only) MYSQL_ONLY=true ;;
		--wiznews) INCLUDE_WIZNEWS=true ;;
		--version) shift; VERSION="${1:?--version richiede un valore}" ;;
		--message) shift; MESSAGE="${1:?--message richiede un valore}" ;;
		--tag) shift; TAG="${1:?--tag richiede un valore}" ;;
		--env) shift; ENVIRONMENT="${1:?--env richiede un valore}" ;;
		*) echo "Opzione sconosciuta: $1" >&2; usage 1 ;;
	esac
	shift
done

if [ "$PAGES_ONLY" = true ] && [ "$MYSQL_ONLY" = true ]; then
	echo "ERRORE: --pages-only e --mysql-only sono mutuamente esclusivi" >&2
	exit 1
fi

# shellcheck source=scripts/load-mysql-conf.sh
source "${ROOT}/scripts/load-mysql-conf.sh"
load_mysql_conf

# Configurazione in base all'ambiente
case "$ENVIRONMENT" in
	release)
		AUTHOR="release"
		if [ -z "$TAG" ]; then
			TAG="$(git -C "$ROOT" tag --list 'r[0-9]*' --sort=-v:refname | head -n1 || true)"
		fi
		if [ -z "$VERSION" ]; then
			if [ -n "$TAG" ]; then
				VERSION="$TAG"
			else
				VERSION="$(git -C "$ROOT" describe --tags --always 2>/dev/null || git -C "$ROOT" rev-parse --short HEAD)"
			fi
		fi
		# Per release, mantieni il comportamento originale (aggiorna anche pages)
		;;
	staging)
		AUTHOR="staging"
		if [ -z "$VERSION" ]; then
			VERSION="$(git -C "$ROOT" describe --tags --always 2>/dev/null || git -C "$ROOT" rev-parse --short HEAD)"
		fi
		# Per staging, default mysql-only per non toccare i file pages condivisi
		if [ "$MYSQL_ONLY" = false ] && [ "$PAGES_ONLY" = false ]; then
			MYSQL_ONLY=true
		fi
		;;
	devel)
		AUTHOR="devel"
		if [ -z "$VERSION" ]; then
			VERSION="$(git -C "$ROOT" describe --tags --always 2>/dev/null || git -C "$ROOT" rev-parse --short HEAD)"
		fi
		# Per devel, default mysql-only per non toccare i file pages condivisi
		if [ "$MYSQL_ONLY" = false ] && [ "$PAGES_ONLY" = false ]; then
			MYSQL_ONLY=true
		fi
		;;
	*)
		echo "ERRORE: ambiente non valido: $ENVIRONMENT (deve essere: devel, staging, release)" >&2
		exit 1
		;;
esac

if [ -z "$MESSAGE" ]; then
	MESSAGE="$(git -C "$ROOT" log -1 --pretty=format:%s)"
fi

DAY="$(date +%d)"
MONTH="$(date +%m)"
YEAR="$(date +%Y)"
SORT_KEY="$(date +%Y%m%d)"
ENTRY_DATE="${YEAR}-${MONTH}-${DAY}"
DISPLAY_DATE="${DAY}/${MONTH}/${YEAR}"

# Formato storico player news (colori mud).
PLAYER_HEADLINE="${DISPLAY_DATE} \$c0011Server\$c0010 ${VERSION} ${MESSAGE}\$c0007"
# Wiznews: testo semplice, senza color codes.
WIZ_HEADLINE="${DISPLAY_DATE} Release ${VERSION}: ${MESSAGE}"

sql_escape() {
	printf '%s' "$1" | sed "s/\\\\/\\\\\\\\/g; s/'/\\\\'/g"
}

mysql_base() {
	mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u"$MYSQL_USER" -p"$MYSQL_PASSWORD" "$@"
}

ensure_server_text_table() {
	mysql_base -D "$MYSQL_DB" <<'SQL'
CREATE TABLE IF NOT EXISTS server_text_entry (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  kind TINYINT UNSIGNED NOT NULL,
  component TINYINT UNSIGNED NOT NULL DEFAULT 0,
  headline TEXT NOT NULL,
  version_str VARCHAR(32) NULL,
  body_long TEXT NULL,
  entry_date DATE NULL,
  sort_key INT NOT NULL DEFAULT 0,
  active TINYINT(1) NOT NULL DEFAULT 1,
  author VARCHAR(32) NOT NULL DEFAULT '',
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  KEY kind_active_sort_i (kind, active, sort_key),
  KEY kind_active_component_i (kind, active, component)
) ENGINE=InnoDB;
SQL
}

news_exists_mysql() {
	local kind="$1"
	local needle="$2"
	local esc
	esc="$(sql_escape "$needle")"
	local n
	n="$(mysql_base -N -D "$MYSQL_DB" -e \
		"SELECT COUNT(*) FROM server_text_entry WHERE kind=${kind} AND component=1 AND active=1 AND author='${AUTHOR}' AND version_str='${esc}'")"
	[ "${n:-0}" -gt 0 ]
}

insert_news_mysql() {
	local kind="$1"
	local headline="$2"
	if news_exists_mysql "$kind" "$VERSION"; then
		echo "==> MySQL kind=${kind}: news per versione '${VERSION}' (author=${AUTHOR}) già presente, skip"
		return 0
	fi
	local esc_headline esc_author
	esc_headline="$(sql_escape "$headline")"
	esc_version="$(sql_escape "$VERSION")"
	esc_author="$(sql_escape "$AUTHOR")"
	local sql="INSERT INTO server_text_entry (kind, component, headline, version_str, body_long, entry_date, sort_key, active, author, created_at, updated_at) VALUES (${kind}, 1, '${esc_headline}', '${esc_version}', NULL, '${ENTRY_DATE}', ${SORT_KEY}, 1, '${esc_author}', NOW(), NOW());"
	if [ "$DRY_RUN" = true ]; then
		echo "==> [dry-run] MySQL kind=${kind}: ${sql}"
		return 0
	fi
	mysql_base -D "$MYSQL_DB" -e "$sql"
	echo "==> MySQL kind=${kind}: inserita news per ${VERSION} (author=${AUTHOR})"
}

prepend_pages_line() {
	local file="$1"
	local line="$2"
	local marker="$3"
	if [ ! -f "$file" ]; then
		echo "ERRORE: file assente: $file" >&2
		exit 1
	fi
	if grep -Fq "$marker" "$file"; then
		echo "==> pages $(basename "$file"): riga per '${VERSION}' già presente, skip"
		return 0
	fi
	if [ "$DRY_RUN" = true ]; then
		echo "==> [dry-run] pages $(basename "$file"): prepend '${line}'"
		return 0
	fi
	local tmp
	tmp="$(mktemp)"
	{
		awk -v line="$line" '
			/^---------------------------------$/ {
				print
				print ""
				print line
				print ""
				found=1
				next
			}
			{ print }
			END {
				if (!found) {
					print "---------------------------------"
					print ""
					print line
					print ""
				}
			}
		' "$file"
	} >"$tmp"
	mv "$tmp" "$file"
	echo "==> pages $(basename "$file"): aggiornato"
}

echo "==> release-news version='${VERSION}' message='${MESSAGE}' environment='${ENVIRONMENT}' author='${AUTHOR}' mysql_only='${MYSQL_ONLY}'"

if [ "$PAGES_ONLY" != true ]; then
	if [ "$DRY_RUN" = true ]; then
		echo "==> [dry-run] ensure_server_text_table"
	else
		ensure_server_text_table
	fi
	insert_news_mysql 0 "$PLAYER_HEADLINE"
	if [ "$INCLUDE_WIZNEWS" = true ]; then
		insert_news_mysql 1 "$WIZ_HEADLINE"
	fi
fi

if [ "$MYSQL_ONLY" != true ]; then
	prepend_pages_line "$PAGES_NEWS" "$PLAYER_HEADLINE" "$VERSION"
	if [ "$INCLUDE_WIZNEWS" = true ]; then
		prepend_pages_line "$PAGES_WIZNEWS" "$WIZ_HEADLINE" "$VERSION"
	fi
fi

echo "==> Completato"
