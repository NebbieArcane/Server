#!/bin/bash
set -e

# Variabili di base
MYSQL_DATA_DIR="/var/lib/mysql"
MYSQL_RUN_DIR="/var/run/mysqld"
MYSQL_USER="root"
MYSQL_PASSWORD="secret"
MYSQL_DB="nebbie"

# 1. Gestione dei permessi e directory (Eseguito come root)
echo "Setting up MySQL directories and ownership..."
# Assicura che le directory esistano e siano di proprietà di mysql
mkdir -p ${MYSQL_RUN_DIR}
mkdir -p /var/log/mysql
chown -R mysql:mysql ${MYSQL_DATA_DIR} ${MYSQL_RUN_DIR} /var/log/mysql

# CLEANUP: Elimina vecchi PID files che possono bloccare l'avvio
rm -f ${MYSQL_RUN_DIR}/mysqld.pid

# 2. Inizializzazione del Data Directory se vuoto
if [ ! -d "${MYSQL_DATA_DIR}/mysql" ]; then
    echo "Initializing MySQL data directory (First run)..."
    # Esegue l'inizializzazione come utente 'mysql' e specifica la datadir
    su -s /bin/bash mysql -c "/usr/sbin/mysqld --initialize-insecure --user=mysql --datadir=${MYSQL_DATA_DIR}"
    echo "Data directory initialized."
fi

# 3. Avvia il demone MySQL in background (con bind forzato e datadir)
echo "Starting MySQL daemon with explicit bind address and datadir..."
# Avvia mysqld in background come utente 'mysql' forzando la connessione TCP
su -s /bin/bash mysql -c "/usr/sbin/mysqld --bind-address=0.0.0.0 --datadir=${MYSQL_DATA_DIR}" &

# Breve pausa per dare tempo al processo di partire.
sleep 3

# 4. Attende che MySQL sia pronto: USO DI TCP/IP
echo "Attempting TCP connection to MySQL on 127.0.0.1:3306..."
TIMEOUT=30
for i in $(seq 1 $TIMEOUT); do
  if mysqladmin ping -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -p$MYSQL_PASSWORD > /dev/null 2>&1; then
    echo "MySQL is running."
    break
  fi
  if [ $i -eq $TIMEOUT ]; then
    echo "--------------------------------------------------------"
    echo "ERROR: MySQL startup timed out. Check container logs."
    echo "--------------------------------------------------------"
    exit 1
  fi
  echo "Waiting for MySQL server to start on TCP port 3306... ($i/$TIMEOUT)"
  sleep 1
done

# 5. Crea il database 'nebbie' se non esiste: USO DI TCP/IP
if ! mysql -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -p$MYSQL_PASSWORD -e "use $MYSQL_DB" 2>/dev/null; then
    echo "Creating database '$MYSQL_DB'..."
    mysql -h 127.0.0.1 -P 3306 --protocol=TCP -u$MYSQL_USER -p$MYSQL_PASSWORD -e "CREATE DATABASE $MYSQL_DB;"
fi

# 6. Avvia l'applicazione come utente 'vagrant'
SERVER_PORT=${SERVER_PORT:-4000}
EXEC_PATH="/app" # <-- ESEGUI DALLA RADICE DEL PROGETTO
DATA_DIR="mudroot/lib" # <-- PERCORSO DATI RELATIVO ALLA RADICE

# Comando che DEVE essere eseguito dalla shell: cd /app && /app/mudroot/myst 4000 -d mudroot/lib
# Usiamo i flag -P e -d per chiarezza e robutezza.
COMMAND_STRING="cd ${EXEC_PATH} && /app/mudroot/myst -P ${SERVER_PORT} -d ${DATA_DIR}"
echo "Starting application: ${COMMAND_STRING}"

# Esegue il comando finale con l'utente 'vagrant'.
exec su -l vagrant -c "${COMMAND_STRING}"