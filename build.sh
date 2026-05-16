#!/bin/bash
# Utilizzato dal sistema di deploy.
# In ambiente di sviluppo va lanciato solo se si aggiunge un nuovo file
set -o allexport
cd $(dirname "$0")
MYSQL_USER=${MYSQL_USER:-root} #db user'
MYSQL_PASSWORD=${MYSQL_PASSWORD:-secret} # db password
MYSQL_HOST=${MYSQL_HOST:-localhost} #db host
MYSQL_DB=${MYSQL_DB:-nebbie} #db name
SERVER_PORT=${SERVER_PORT:-4002} #default server port
environment=${1:-vagrant}
ENVIRONMENT=$environment
conf="$HOME/Confs/$environment.conf"
if [ -f $conf ] ; then
	. $conf
else
	echo "No Conf file for $environment present, using builtin defaults"
fi
if [ -n "${DOCKER_BUILD}" ]; then
	MYSQL_HOST=127.0.0.1
fi
rm -f CMakeCache.txt
rm -f src/CMakeCache.txt
rm -f src/release.h
rm -f src/myst src/info
rm -f mudroot/myst
rm -rf build
(
export PATH="/usr/lib/cache:$PATH"
mkdir -p build/src/include
cd build
cmake ..
jobs=$(cat makejobs)
MYST_OBJ_DIR="src/CMakeFiles/myst.dir"
cmake -E make_directory "${MYST_OBJ_DIR}"
cmake -E make_directory "${MYST_OBJ_DIR}/odb"
sed -e "s|FOLDER|./../build|" -e "s/MAKEJOBS/$jobs/" ../Makefile.source > ../mudroot/Makefile
sed -e "s|FOLDER|./../build|" -e "s/MAKEJOBS/$jobs/" ../Makefile.source > ../src/Makefile
sed -e "s|FOLDER|./build|" -e "s/MAKEJOBS/$jobs/" ../Makefile.source > ../Makefile
if ! cmake --build . --parallel "${jobs}"; then
	echo "Parallel build failed; retrying with -j1 (common on Vagrant/shared FS)..."
	cmake --build . --parallel 1
fi
)
if [ -x mudroot/myst ] ; then
	echo "Ready"
	exit 0
fi
echo "Myst executable not found"
exit 1
