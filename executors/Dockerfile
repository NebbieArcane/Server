# Nebbie Server — isolated build environment for ./build.sh
#
# Host needs only Docker. Toolchain, ODB, and libraries are installed in the image.
#
# Build image (context must be executors/; requires network for ODB debs):
#   docker build --platform linux/amd64 -t nebbie/compiler executors/
#
# Compile myst (mount checkout at /app):
#   docker run --rm --platform linux/amd64 \
#     -v "$PWD:/app" -w /app -e DOCKER_BUILD=ON \
#     nebbie/compiler
#
# Optional: mount host Confs for a named environment (e.g. production):
#   -v "$HOME/Confs:/home/builder/Confs:ro" nebbie/compiler production
#
# Run docker CLI inside the container (host socket; match host docker group gid):
#   docker run --rm -it --platform linux/amd64 \
#     -v /var/run/docker.sock:/var/run/docker.sock \
#     --group-add "$(stat -c '%g' /var/run/docker.sock)" \
#     -v "$PWD:/app" -w /app -e DOCKER_BUILD=ON \
#     --entrypoint /bin/bash nebbie/compiler
#
# ODB 2.5: official CodeSynthesis .deb packages (Debian 12 / amd64 on Ubuntu 24.04).
# See https://codesynthesis.com/download/odb/2.5.0/debian/README.xhtml

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV DOCKER_BUILD=ON

ARG ODB_DEB_BASE=https://codesynthesis.com/download/odb/2.5.0/debian/debian12/x86_64
ARG ODB_DEB_REV=2.5.0-0~debian12_amd64

RUN apt-get update && \
    apt-get install -y \
    git php8.3-cli gcc-13 g++-13 gcc-13-plugin-dev \
    make cmake pkg-config docker.io \
    libconfig++-dev libsqlite3-dev libcurlpp-dev libevent-dev libcrypt-dev \
    libcurl4-openssl-dev libboost-dev libboost-program-options-dev libboost-system-dev \
    libboost-filesystem-dev liblog4cxx-dev libboost-date-time-dev \
    librtmp-dev libnghttp2-dev libkrb5-dev comerr-dev libpsl-dev libssh-dev libbrotli-dev \
    libmysqlclient-dev libmysqlcppconn-dev wget curl ca-certificates dos2unix && \
    mkdir -p /tmp/odb-debs && cd /tmp/odb-debs && \
    for pkg in \
    odb \
    libodb libodb-dev \
    libodb-boost libodb-boost-dev \
    libodb-sqlite libodb-sqlite-dev \
    libodb-mysql libodb-mysql-dev; do \
    wget -q "${ODB_DEB_BASE}/${pkg}_${ODB_DEB_REV}.deb"; \
    done && \
    apt-get install -y ./*.deb && \
    rm -rf /tmp/odb-debs && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 90 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 90

WORKDIR /app

ENTRYPOINT ["/app/build.sh"]
