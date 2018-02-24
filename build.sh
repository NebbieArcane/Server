#!/bin/bash
# Utilizzato dal sistema di deploy.
# In ambiente di sviluppo va lanciato solo se si aggiunge un nuovo file
cd $(dirname "$0")
environment=${1:-vagrant}
conf="$HOME/Confs/$environment.conf"
if [ -f $conf ] ; then
echo '//Generated file, do not edit' >src/config.hpp.in
	echo '#ifndef __CONFIG_HPP_' >>src/config.hpp.in
	echo '#define __CONFIG_HPP_' >>src/config.hpp.in
	cat  $HOME/Confs/$environment.conf | grep -ve '^#' | sed -e 's/#/\/\//' | sed -e 's/=/ /' | sed -e 's/\(.*\)/#define \1/' >>src/config.hpp.in
	echo '#endif /*__CONFIG_HPP_*/' >>src/config.hpp.in
	cat src/config.hpp.in
	rm -f src/release.h
	rm -f mudroot/myst
	rm -f  CMakeCache.txt
	cmake .
	make clean && make
	if [ -x src/myst ] ; then
		cp src/myst mudroot/myst
		exit 0
	fi
	exit 1
else
	echo "Could not find config file, maybe \$HOME ($HOME) is wrong" >&2
	exit 255
fi

