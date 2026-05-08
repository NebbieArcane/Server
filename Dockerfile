FROM ubuntu:22.04

# Imposta variabili d'ambiente per installazioni non interattive
ENV DEBIAN_FRONTEND=noninteractive

# 1. Configurazione Password MySQL non interattiva (password 'secret')
RUN echo "mysql-server mysql-server/root_password password secret" | debconf-set-selections && \
    echo "mysql-server mysql-server/root_password_again password secret" | debconf-set-selections

# 2. Installazione dipendenze e creazione utente 'vagrant'
RUN apt-get update && \
    apt-get install -y \
        sudo git php8.1-cli gcc-12 g++-12 gcc-12-plugin-dev apache2 make cmake libconfig++-dev lnav libsqlite3-dev libcurlpp-dev gdb wget \
        libcurl4-openssl-dev libboost-dev libboost-program-options-dev libboost-system-dev \
        libboost-filesystem-dev liblog4cxx-dev libboost-date-time-dev \
        librtmp-dev libnghttp2-dev libkrb5-dev comerr-dev libpsl-dev libssh-dev libbrotli-dev \
        mysql-server mysql-client libmysqlclient-dev libmysqlcppconn-dev net-tools iproute2 vim less dos2unix && \
    BUILD2_DEB_BASE="https://download.build2.org/0.18.1/bindist/ubuntu/ubuntu22.04/x86_64" && \
    mkdir -p /tmp/build2-debs && \
    cd /tmp/build2-debs && \
    wget -q "${BUILD2_DEB_BASE}/build2-toolchain_0.18.1-0~ubuntu22.04_amd64.deb" && \
    apt-get install -y ./build2-toolchain_0.18.1-0~ubuntu22.04_amd64.deb && \
    BPKG_STABLE_REPO="https://pkg.cppget.org/1/stable" && \
    mkdir -p /tmp/odb-build && \
    bpkg create -d /tmp/odb-build/odb-gcc-12 cc \
      config.cxx=g++-12 \
      config.cc.coptions=-O2 \
      config.bin.rpath=/usr/local/lib \
      config.install.root=/usr/local && \
    printf 'y\n' | bpkg build --trust-yes -d /tmp/odb-build/odb-gcc-12 "odb@${BPKG_STABLE_REPO}" && \
    bpkg install -d /tmp/odb-build/odb-gcc-12 odb && \
    bpkg create -d /tmp/odb-build/libodb-gcc-12 cc \
      config.cxx=g++-12 \
      config.cc.coptions=-O2 \
      config.install.root=/usr/local && \
    bpkg add --trust-yes -d /tmp/odb-build/libodb-gcc-12 "${BPKG_STABLE_REPO}" && \
    bpkg fetch -d /tmp/odb-build/libodb-gcc-12 && \
    printf 'y\n' | bpkg build --trust-yes -d /tmp/odb-build/libodb-gcc-12 libodb && \
    printf 'y\n' | bpkg build --trust-yes -d /tmp/odb-build/libodb-gcc-12 "libodb-sqlite ?sys:libsqlite3" && \
    printf 'y\n' | bpkg build --trust-yes -d /tmp/odb-build/libodb-gcc-12 "libodb-mysql ?sys:libmysqlclient" && \
    printf 'y\n' | bpkg build --trust-yes -d /tmp/odb-build/libodb-gcc-12 libodb-boost && \
    bpkg install -d /tmp/odb-build/libodb-gcc-12 --all --recursive && \
    echo "/usr/local/lib" > /etc/ld.so.conf.d/odb-local.conf && \
    ldconfig && \
    rm -rf /tmp/build2-debs /tmp/odb-build && \
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
