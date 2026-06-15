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

FROM nebbiearcane/mudcompiler:latest

ENV DEBIAN_FRONTEND=noninteractive
ENV DOCKER_BUILD=ON
USER root
RUN ldconfig
WORKDIR /run
COPY myst .
COPY version.txt .
COPY config.txt .

ENTRYPOINT ["./myst"]
CMD ["-v"]
