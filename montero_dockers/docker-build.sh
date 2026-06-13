#!/bin/bash
# Build myst inside Docker using the same toolchain as the runtime image.
#
# Usage:
#   ./docker-build.sh              # compile with ./build.sh vagrant
#   ./docker-build.sh production   # use Confs/production.conf if present on host
#   ./docker-build.sh --image-only # build the build-env image, do not compile
#
# Requires Docker. On Apple Silicon, forces linux/amd64 (same as docker-run.sh).

set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
IMAGE="nebbie-build-env"
ENVIRONMENT="vagrant"
BUILD_IMAGE_ONLY=false

while [ $# -gt 0 ]; do
	case "$1" in
		--image-only)
			BUILD_IMAGE_ONLY=true
			shift
			;;
		-*)
			echo "Unknown option: $1" >&2
			exit 1
			;;
		*)
			ENVIRONMENT="$1"
			shift
			;;
	esac
done

ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then
	export DOCKER_PLATFORM=linux/amd64
else
	export DOCKER_PLATFORM=linux/amd64
fi

echo "Building ${IMAGE} (${DOCKER_PLATFORM})..."
docker build \
	--platform "${DOCKER_PLATFORM}" \
	--target build-env \
	-t "${IMAGE}" \
	-f "${ROOT}/Dockerfile" \
	"${ROOT}"

if [ "$BUILD_IMAGE_ONLY" = "true" ]; then
	echo "Image ${IMAGE} ready."
	exit 0
fi

CONF_MOUNT=()
if [ -f "${HOME}/Confs/${ENVIRONMENT}.conf" ]; then
	CONF_MOUNT=(-v "${HOME}/Confs:/home/vagrant/Confs:ro")
fi

echo "Running ./build.sh ${ENVIRONMENT} in container..."
# Host build/ may be root-owned from a prior Docker build; clean as root first.
docker run --rm \
	--platform "${DOCKER_PLATFORM}" \
	-v "${ROOT}:/app" \
	-w /app \
	-u root \
	--entrypoint sh \
	"${IMAGE}" \
	-c 'rm -rf build src/myst src/info src/release.h mudroot/myst CMakeCache.txt src/CMakeCache.txt 2>/dev/null || true; chown -R vagrant:vagrant /app'

docker run --rm \
	--platform "${DOCKER_PLATFORM}" \
	-v "${ROOT}:/app" \
	-w /app \
	-e DOCKER_BUILD=ON \
	-u vagrant \
	"${CONF_MOUNT[@]}" \
	"${IMAGE}" \
	"${ENVIRONMENT}"

# Restore host ownership of artifacts written by vagrant (uid 1000).
docker run --rm \
	--platform "${DOCKER_PLATFORM}" \
	-v "${ROOT}:/app" \
	-w /app \
	-u root \
	--entrypoint sh \
	"${IMAGE}" \
	-c 'chown -R '"$(id -u):$(id -g)"' /app'

if [ -x "${ROOT}/mudroot/myst" ]; then
	echo "Ready: ${ROOT}/mudroot/myst"
else
	echo "Build finished but mudroot/myst is missing." >&2
	exit 1
fi
