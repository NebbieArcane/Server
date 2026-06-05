#!/bin/bash
set -e

# Variabili di base
MYSQL_DATA_DIR="/var/lib/mysql"
MYSQL_RUN_DIR="/var/run/mysqld"
MYSQL_USER="root"
MYSQL_PASSWORD="secret"
MYSQL_DB="nebbie"

# 1) Prepara directory e permessi MySQL (entrypoint eseguito come root)
echo "Setting up MySQL directories and ownership..."
# Assicura che le directory esistano e siano di proprietà di mysql
mkdir -p ${MYSQL_RUN_DIR}
mkdir -p /var/log/mysql
chown -R mysql:mysql ${MYSQL_DATA_DIR} ${MYSQL_RUN_DIR} /var/log/mysql

# Cleanup: rimuove PID stale che possono bloccare un nuovo avvio
rm -f ${MYSQL_RUN_DIR}/mysqld.pid

# 2) Inizializza la datadir al primo avvio
if [ ! -d "${MYSQL_DATA_DIR}/mysql" ]; then
    echo "Initializing MySQL data directory (First run)..."
    # Inizializzazione esplicita con utente mysql
    su -s /bin/bash mysql -c "/usr/sbin/mysqld --initialize-insecure --user=mysql --datadir=${MYSQL_DATA_DIR}"
    echo "Data directory initialized."
fi

# 3) Avvia MySQL in background con bind esplicito
echo "Starting MySQL daemon with explicit bind address and datadir..."
# Avvio mysqld come utente mysql con path espliciti per log/socket/pid
su -s /bin/bash mysql -c "/usr/sbin/mysqld --bind-address=0.0.0.0 --datadir=${MYSQL_DATA_DIR} --socket=${MYSQL_RUN_DIR}/mysqld.sock --pid-file=${MYSQL_RUN_DIR}/mysqld.pid --log-error=/var/log/mysql/error.log" &

# Breve pausa per dare tempo al processo di partire.
sleep 3

# 4) Attende readiness MySQL via TCP locale
echo "Attempting TCP connection to MySQL on 127.0.0.1:3306..."
TIMEOUT=60
for i in $(seq 1 $TIMEOUT); do
  if mysqladmin ping -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -p$MYSQL_PASSWORD > /dev/null 2>&1; then
    echo "MySQL is running."
    break
  fi
  if [ $i -eq $TIMEOUT ]; then
    echo "--------------------------------------------------------"
    echo "ERROR: MySQL startup timed out. Check container logs."
    echo "MySQL error log snapshot:"
    ls -la /var/log/mysql || true
    if [ -f /var/log/mysql/error.log ]; then
      sed -n '1,200p' /var/log/mysql/error.log || true
    fi
    echo "--------------------------------------------------------"
    exit 1
  fi
  echo "Waiting for MySQL server to start on TCP port 3306... ($i/$TIMEOUT)"
  sleep 1
done

# 5) Assicura esistenza DB e credenziali root
echo "Configuring database..."

# Primo tentativo con password già impostata
echo "Executing: mysql -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -p$MYSQL_PASSWORD -e \"CREATE DATABASE IF NOT EXISTS $MYSQL_DB;\""
if ! mysql -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -p$MYSQL_PASSWORD -e "CREATE DATABASE IF NOT EXISTS $MYSQL_DB;"; then
  echo "First attempt failed, trying without password (fresh MySQL install)..."
  if ! mysql -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -e "CREATE DATABASE IF NOT EXISTS $MYSQL_DB;"; then
    echo "--------------------------------------------------------"
    echo "ERROR: Failed to create database '$MYSQL_DB'"
    echo "Check MySQL credentials and connectivity."
    echo "--------------------------------------------------------"
    exit 1
  fi
  
  echo "Database created, now setting password..."
  if ! mysql -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -e "ALTER USER 'root'@'localhost' IDENTIFIED BY '$MYSQL_PASSWORD';"; then
    echo "--------------------------------------------------------"
    echo "ERROR: Failed to set MySQL password"
    echo "--------------------------------------------------------"
    exit 1
  fi
  
  echo "Password set successfully."
fi

echo "Database '$MYSQL_DB' created successfully."

# 5b) Flag cutover su toon (migrated_at / schema_version) se mancanti
MIGRATION_FLAGS=/app/docs/schema-s1-toon-migration-flags.sql
if [ -f "$MIGRATION_FLAGS" ]; then
  HAS_MIGRATED_AT=$(mysql -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -p$MYSQL_PASSWORD -N -e \
    "SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='${MYSQL_DB}' AND TABLE_NAME='toon' AND COLUMN_NAME='migrated_at';" 2>/dev/null || echo "0")
  if [ "$HAS_MIGRATED_AT" = "0" ]; then
    echo "Applying toon migration flags (migrated_at, schema_version)..."
    mysql -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -p$MYSQL_PASSWORD "$MYSQL_DB" < "$MIGRATION_FLAGS"
    echo "Toon migration flags applied."
  fi
fi

# 6) Avvia il server nebbieserver come utente vagrant
SERVER_PORT=${SERVER_PORT:-4000}
EXEC_PATH="/app" # Esegui dalla root del progetto
DATA_DIR="mudroot/lib" # Path dati relativo alla root del progetto

# Comando finale: cd /app && /app/mudroot/myst -P <porta> -d mudroot/lib (nebbieserver)
# Usiamo -P e -d in modo esplicito per robustezza.
COMMAND_STRING="cd ${EXEC_PATH} && /app/mudroot/myst -P ${SERVER_PORT} -d ${DATA_DIR}"
echo "Starting application: ${COMMAND_STRING}"

# Esegue il comando finale con utente vagrant.
exec su -l vagrant -c "${COMMAND_STRING}"