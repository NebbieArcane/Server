#!/bin/bash
# Carica MYSQL_* da Confs/{ENVIRONMENT}.conf (come build.sh).
# Uso: ENVIRONMENT=devel source scripts/load-mysql-conf.sh
# Cerca in ordine: $HOME/Confs/, poi $ROOT/Confs/ (repo Server/Confs/).
load_mysql_conf() {
	local env="${ENVIRONMENT:-devel}"
	local root="${ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
	local conf=""

	# Se tutte le variabili MySQL sono già impostate, non caricare il file di configurazione
	if [ -n "${MYSQL_HOST:-}" ] && [ -n "${MYSQL_USER:-}" ] && [ -n "${MYSQL_PASSWORD:-}" ] && [ -n "${MYSQL_DB:-}" ] && [ -n "${MYSQL_PORT:-}" ]; then
		echo "==> MySQL conf: using environment variables (skipping config file)" >&2
		return
	fi

	if [ -f "${HOME}/Confs/${env}.conf" ]; then
		conf="${HOME}/Confs/${env}.conf"
	elif [ -f "${root}/Confs/${env}.conf" ]; then
		conf="${root}/Confs/${env}.conf"
	fi

	if [ -n "$conf" ]; then
		echo "==> MySQL conf: ${conf}" >&2
		# Carica il file di configurazione ma non sovrascrivere le variabili già impostate
		while IFS='=' read -r key value; do
			# Salta le righe vuote e i commenti
			[[ -z "$key" || "$key" =~ ^[[:space:]]*# ]] && continue
			# Rimuovi spazi e virgolette
			key=$(echo "$key" | tr -d '[:space:]')
			value=$(echo "$value" | tr -d '"' | tr -d "'" | tr -d '[:space:]')
			# Imposta solo se la variabile d'ambiente non è già impostata
			case "$key" in
				MYSQL_HOST) MYSQL_HOST="${MYSQL_HOST:-$value}" ;;
				MYSQL_USER) MYSQL_USER="${MYSQL_USER:-$value}" ;;
				MYSQL_PASSWORD) MYSQL_PASSWORD="${MYSQL_PASSWORD:-$value}" ;;
				MYSQL_DB) MYSQL_DB="${MYSQL_DB:-$value}" ;;
				MYSQL_PORT) MYSQL_PORT="${MYSQL_PORT:-$value}" ;;
			esac
		done < "$conf"
	else
		echo "==> MySQL conf: defaults (manca Confs/${env}.conf in HOME e repo)" >&2
	fi

	# Applica i default solo se le variabili non sono già impostate
	MYSQL_HOST="${MYSQL_HOST:-127.0.0.1}"
	MYSQL_USER="${MYSQL_USER:-root}"
	MYSQL_PASSWORD="${MYSQL_PASSWORD:-secret}"
	MYSQL_DB="${MYSQL_DB:-nebbie}"
	MYSQL_PORT="${MYSQL_PORT:-3306}"
}
