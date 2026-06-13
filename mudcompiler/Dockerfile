# Nebbie myst compiler image — Ubuntu 24.04, GCC 13, ODB 2.5 from CodeSynthesis debs.
#
# Build:
#   docker build --platform linux/amd64 -t nebbiearcane/mudcompiler .
#
# Compile (mount checkout at /app):
#   docker run --rm --platform linux/amd64 \
#     -v "$PWD:/app" -w /app -e DOCKER_BUILD=ON \
#     nebbiearcane/mudcompiler devel

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV DOCKER_BUILD=ON

ARG ODB_DEB_BASE=https://codesynthesis.com/download/odb/2.5.0/debian/debian12/x86_64
ARG ODB_DEB_REV=2.5.0-0~debian12_amd64

RUN apt-get update && \
    apt-get install -y \
        git php8.3-cli php-mysql gcc-13 g++-13 g++ gcc-13-plugin-dev \
        make cmake pkg-config \
        libconfig++-dev libsqlite3-dev libcurlpp-dev libevent-dev libcrypt-dev \
        libcurl4-openssl-dev libboost-dev libboost-program-options-dev libboost-system-dev \
        libboost-filesystem-dev liblog4cxx-dev libboost-date-time-dev \
        librtmp-dev libnghttp2-dev libkrb5-dev comerr-dev libpsl-dev libssh-dev libbrotli-dev \
        libmariadb3 libmariadb-dev wget curl ca-certificates dos2unix && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 90 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 90 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# ODB 2.5: download official .deb packages (Debian 12 amd64 on Ubuntu 24.04 Noble).
# libodb-mysql-dev wants default-libmysqlclient-dev; use MariaDB + --force-depends instead
# of Oracle libmysqlclient-dev (ABI mismatch with libodb-mysql at link time).
RUN mkdir -p /tmp/odb-debs && cd /tmp/odb-debs && \
    for pkg in \
        odb \
        libodb libodb-dev \
        libodb-boost libodb-boost-dev \
        libodb-sqlite libodb-sqlite-dev \
        libodb-mysql; do \
        wget -q "${ODB_DEB_BASE}/${pkg}_${ODB_DEB_REV}.deb"; \
    done && \
    dpkg -i ./*.deb && \
    wget -q "${ODB_DEB_BASE}/libodb-mysql-dev_${ODB_DEB_REV}.deb" && \
    dpkg -i --force-depends "./libodb-mysql-dev_${ODB_DEB_REV}.deb" && \
    cd / && rm -rf /tmp/odb-debs && \
    ln -sf libmariadb.so.3 /usr/lib/x86_64-linux-gnu/libmysqlclient.so && \
    ln -sf libmariadb.so.3 /usr/lib/x86_64-linux-gnu/libmysqlclient.so.18 && \
    ldconfig

WORKDIR /app

ENTRYPOINT ["/app/build.sh"]
CMD ["devel"]
