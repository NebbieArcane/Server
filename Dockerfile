ARG TARGETPLATFORM=linux/amd64
FROM --platform=${TARGETPLATFORM} ubuntu:bionic

ENV DEBIAN_FRONTEND=noninteractive

# 1. Configurazione Password MySQL non interattiva (password 'secret')
RUN echo "mysql-server mysql-server/root_password password secret" | debconf-set-selections && \
    echo "mysql-server mysql-server/root_password_again password secret" | debconf-set-selections

# 2. Installazione dipendenze e creazione utente 'vagrant'
RUN apt-get update && \
    apt-get install -y \
        sudo git php7.2-cli g++ apache2 make cmake libconfig++-dev lnav libsqlite3-dev libcurlpp-dev gdb \
        libcurl4-openssl-dev libboost-dev libboost-program-options-dev libboost-system-dev \
        libboost-filesystem-dev liblog4cxx-dev libboost-date-time-dev \
        odb libodb-dev libodb-mysql-dev libodb-sqlite-dev libodb-boost-dev \
        librtmp-dev libnghttp2-dev libkrb5-dev comerr-dev libpsl-dev \
        mysql-server mysql-client libmysqld-dev libmysqlcppconn-dev net-tools iproute2 vim less dos2unix && \
    adduser --disabled-password --gecos "" vagrant && \
    usermod -aG sudo vagrant && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# 3. Fix Apple Silicon / QEMU: i file ODB sono pre-generati su ARM64 nativo
# (senza QEMU) e inclusi nel build context. Vengono copiati con COPY . /app
# e poi "toccati" per garantire che CMake li veda più recenti di account.hpp
# ed eviti di rieseguire ODB sotto emulazione x86_64 (dove cc1plus fallisce).

# 4. Copia codice sorgente e impostazione permessi
ENV DOCKER_BUILD=ON
COPY . /app
RUN touch /app/src/odb/account-odb.cxx \
          /app/src/odb/account-odb.hxx \
          /app/src/odb/account-odb.ixx \
          /app/src/odb/account-odb-mysql.cxx \
          /app/src/odb/account-odb-mysql.hxx \
          /app/src/odb/account-odb-mysql.ixx \
          /app/src/odb/account-schema-mysql.cxx && \
    chown -R vagrant:vagrant /app
WORKDIR /app

# 5. Switch a utente 'vagrant' per configurazione e build
USER vagrant

# 6. Replicazione configurazione utente (Confs/vagrant.conf)
RUN cd ~ && \
    mkdir -p Confs && \
    echo 'MYSQL_USER="root" #db user' > Confs/vagrant.conf && \
    echo 'MYSQL_PASSWORD="secret" # db password' >> Confs/vagrant.conf && \
    echo 'MYSQL_HOST="localhost" #db host' >> Confs/vagrant.conf && \
    echo 'MYSQL_DB="nebbie" #db name' >> Confs/vagrant.conf && \
    echo 'SERVER_PORT=4000 #default server port' >> Confs/vagrant.conf && \
    git config --global user.email "nebbie@hexkeep.com" && \
    git config --global user.name "Nebbie Server"

# 7. Esecuzione dello script di build
RUN echo "Starting build process" && \
    /app/build.sh vagrant

# 8. Configurazione Entrypoint
USER root
COPY docker-entrypoint.sh /usr/local/bin/
RUN dos2unix /usr/local/bin/docker-entrypoint.sh && \
    chmod +x /usr/local/bin/docker-entrypoint.sh

EXPOSE 4000 4001 4002 10000

USER root
ENTRYPOINT ["/usr/local/bin/docker-entrypoint.sh"]
CMD []
