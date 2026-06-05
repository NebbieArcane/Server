FROM ubuntu:24.04

# Imposta variabili d'ambiente per installazioni non interattive
ENV DEBIAN_FRONTEND=noninteractive

# 1. Configurazione Password MySQL non interattiva (password 'secret')
RUN echo "mysql-server mysql-server/root_password password secret" | debconf-set-selections && \
    echo "mysql-server mysql-server/root_password_again password secret" | debconf-set-selections

# Script ODB (prima del RUN lungo: serve rete verso pkg.cppget.org con retry/DNS)
COPY scripts/install-odb-toolchain.sh /usr/local/sbin/install-odb-toolchain.sh
RUN chmod +x /usr/local/sbin/install-odb-toolchain.sh

# 2. Installazione dipendenze, ODB e creazione utente 'vagrant'
RUN apt-get update && \
    apt-get install -y \
        sudo git php8.3-cli gcc-12 g++-12 gcc-12-plugin-dev apache2 make cmake libconfig++-dev lnav libsqlite3-dev libcurlpp-dev gdb wget curl ca-certificates \
        libcurl4-openssl-dev libboost-dev libboost-program-options-dev libboost-system-dev \
        libboost-filesystem-dev liblog4cxx-dev libboost-date-time-dev \
        librtmp-dev libnghttp2-dev libkrb5-dev comerr-dev libpsl-dev libssh-dev libbrotli-dev \
        mysql-server mysql-client libmysqlclient-dev libmysqlcppconn-dev net-tools iproute2 vim less dos2unix && \
    ODB_BUILD=/var/cache/nebbie-odb-build /usr/local/sbin/install-odb-toolchain.sh && \
    # Creazione utente 'vagrant'
    adduser --disabled-password --gecos "" vagrant && \
    usermod -aG sudo vagrant && \
    # Pulizia cache
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# 2b. Configura GCC 12 come default (necessario per C++17)
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 90 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 90

# 3. Copia codice sorgente e impostazione permessi
# Variabile usata dal CMake per selezionare i flag del profilo Docker.
ENV DOCKER_BUILD=ON
COPY . /app
RUN chown -R vagrant:vagrant /app
WORKDIR /app

# 4. Switch a utente 'vagrant' per configurazione e build
USER vagrant

# 5. Creazione configurazione runtime (Confs/vagrant.conf)
RUN cd ~ && \
    mkdir -p Confs && \
    echo 'MYSQL_USER="root" #db user' > Confs/vagrant.conf && \
    echo 'MYSQL_PASSWORD="secret" # db password' >> Confs/vagrant.conf && \
    echo 'MYSQL_HOST="localhost" #db host' >> Confs/vagrant.conf && \
    echo 'MYSQL_DB="nebbie" #db name' >> Confs/vagrant.conf && \
    echo 'SERVER_PORT=4000 #default server port' >> Confs/vagrant.conf && \
    # Configurazione git globale
    git config --global user.email "nebbie@hexkeep.com" && \
    git config --global user.name "Nebbie Server"

# 6. Esecuzione dello script di build
RUN echo "Starting build process" && \
    /app/build.sh vagrant

# 7. Configurazione entrypoint
USER root
COPY docker-entrypoint.sh /usr/local/bin/
# Conversione line endings + permessi (evita errori "no such file")
RUN dos2unix /usr/local/bin/docker-entrypoint.sh && \
    chmod +x /usr/local/bin/docker-entrypoint.sh

# Espone le porte
EXPOSE 4000 4001 4002 10000

# Avvio come root: l'entrypoint inizializza MySQL e poi lancia il server
USER root
ENTRYPOINT ["/usr/local/bin/docker-entrypoint.sh"]
CMD []
