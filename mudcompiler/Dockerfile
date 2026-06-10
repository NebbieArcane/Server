# Nebbie Server — isolated build environment for ./build.sh
#
# Host needs only Docker. Toolchain, ODB, and libraries are installed in the image.
#
# Build image (from repository root; requires network for ODB via pkg.cppget.org):
#   docker build --platform linux/amd64 -f executors/Dockerfile -t nebbie-build-env .
#
# Compile myst (mount checkout at /app):
#   docker run --rm --platform linux/amd64 \
#     -v "$PWD:/app" -w /app -e DOCKER_BUILD=ON \
#     nebbie-build-env
#
# Optional: mount host Confs for a named environment (e.g. production):
#   -v "$HOME/Confs:/root/Confs:ro" nebbie-build-env production
#
# Base: Ubuntu 24.04 (Noble), GCC 12, ODB 2.5 via build2/bpkg (linux/amd64).

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV DOCKER_BUILD=ON

COPY install-odb-toolchain.sh /usr/local/sbin/install-odb-toolchain.sh
RUN chmod +x /usr/local/sbin/install-odb-toolchain.sh

RUN apt-get update && \
    apt-get install -y \
    git php8.3-cli gcc-13 g++-13 gcc-13-plugin-dev make cmake pkg-config \
    libconfig++-dev libsqlite3-dev libcurlpp-dev libevent-dev libcrypt-dev \
    libcurl4-openssl-dev libboost-dev libboost-program-options-dev libboost-system-dev \
    libboost-filesystem-dev liblog4cxx-dev libboost-date-time-dev \
    librtmp-dev libnghttp2-dev libkrb5-dev comerr-dev libpsl-dev libssh-dev libbrotli-dev \
    libmysqlclient-dev libmysqlcppconn-dev wget curl ca-certificates dos2unix 
RUN ODB_BUILD=/var/cache/nebbie-odb-build /usr/local/sbin/install-odb-toolchain.sh && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 90 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 90

WORKDIR /app

ENTRYPOINT ["/app/build.sh"]
CMD ["devel"]
