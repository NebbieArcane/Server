#!/bin/bash
# Utilizzato dal sistema di deploy.
# In ambiente di sviluppo va lanciato solo se si aggiunge un nuovo file
set -o allexport
cd $(dirname "$0")
MYSQL_USER=${MYSQL_USER:-root} #db user'
MYSQL_PASSWORD=${MYSQL_PASSWORD:-secret} # db password
MYSQL_HOST=${MYSQL_HOST:-localhost} #db host
MYSQL_DB=${MYSQL_DB:-nebbie} #db name
MYSQL_PORT=${MYSQL_PORT:-3306} #db port
SERVER_PORT=${SERVER_PORT:-4002} #default server port
environment=${1:-vagrant}
echo "MYSQL_USER: $MYSQL_USER"
echo "MYSQL_PASSWORD: $MYSQL_PASSWORD"
echo "MYSQL_HOST: $MYSQL_HOST"
echo "MYSQL_DB: $MYSQL_DB"
echo "MYSQL_PORT: $MYSQL_PORT"
echo "SERVER_PORT: $SERVER_PORT"
echo "environment: $environment"
echo "DOCKER_BUILD: $DOCKER_BUILD"
echo "HOME: $HOME"
ENVIRONMENT=$environment
conf="$HOME/Confs/$environment.conf"
if [ -f $conf ] ; then
	. $conf
else
	echo "No Conf file for [$environment] present, using builtin defaults"
fi
ensure_odb_sources() {
	ODB_FILES=(
		src/odb/account-odb.cxx
		src/odb/account-odb-mysql.cxx
		src/odb/account-schema-mysql.cxx
	)
	for f in "${ODB_FILES[@]}"; do
		if [ ! -f "$f" ]; then
			if ! command -v odb >/dev/null 2>&1; then
				echo "Missing ODB sources ($f) and odb compiler not found in PATH."
				exit 1
			fi
			echo "Missing ODB sources — running odb on account.hpp..."
			(
				cd src/odb && odb \
					--profile boost/smart-ptr --profile boost/date-time \
					--std c++17 -m dynamic -d common -d mysql \
					--generate-query --generate-prepared --show-sloc \
					--generate-session --generate-schema \
					--omit-drop --schema-format separate --at-once \
					--schema-name account --input-name account account.hpp
			) || exit 1
			return 0
		fi
	done
}
ensure_odb_sources
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
ensure_myst_obj_dirs() {
	cmake -E make_directory "${MYST_OBJ_DIR}"
	cmake -E make_directory "${MYST_OBJ_DIR}/odb"
}
ensure_myst_obj_dirs
# Shared /vagrant mounts often break parallel writes into myst.dir/odb/.
sed -e "s|FOLDER|./../build|" -e "s/MAKEJOBS/$jobs/" ../Makefile.source > ../mudroot/Makefile
sed -e "s|FOLDER|./../build|" -e "s/MAKEJOBS/$jobs/" ../Makefile.source > ../src/Makefile
sed -e "s|FOLDER|./build|" -e "s/MAKEJOBS/$jobs/" ../Makefile.source > ../Makefile
if ! cmake --build . --parallel "${jobs}"; then
	echo "Build failed"
	cmake ..
	exit 1
fi
)
if [ -x mudroot/myst ] ; then
	echo "Ready"
	cp mudroot/myst "./mudrunner"
	ln -sf mudroot/myst "./myst"
	exit 0
else 
	exit 1
fi
