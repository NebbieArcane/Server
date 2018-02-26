#!/bin/bash
# Utilizzato dal sistema di deploy.
# In ambiente di sviluppo va lanciato solo se si aggiunge un nuovo file
set -o allexport
cd $(dirname "$0")
MYSQL_USER="root" #db user'
MYSQL_PASSWORD="secret" # db password
MYSQL_HOST="localhost" #db host
MYSQL_DB="nebbie" #db name
SERVER_PORT=4002 #default server port
environment=${1:-vagrant}
conf="$HOME/Confs/$environment.conf"
if [ -f $conf ] ; then
	. $conf
fi
echo "No Conf file present, using builtin defaults"
#	echo '//Generated file, do not edit' >src/config.hpp.in
#	echo '#ifndef __CONFIG_HPP_' >>src/config.hpp.in
#	echo '#define __CONFIG_HPP_' >>src/config.hpp.in
#	cat  $HOME/Confs/$environment.conf | grep -ve '^#' | sed -e 's/#/\/\//' | sed -e 's/=/ /' | sed -e 's/\(.*\)/#define \1/' >>src/config.hpp.in
#	echo '#endif /*__CONFIG_HPP_*/' >>src/config.hpp.in
#	cat src/config.hpp.in
rm -f src/release.h
rm -f mudroot/myst
rm -f  CMakeCache.txt
cmake . && make clean && make
if [ -x src/myst ] ; then
	cp src/myst mudroot/myst
	exit 0
fi
exit 1
