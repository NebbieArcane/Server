#!/bin/bash
# Utilizzato dal sistema di deploy.
# In ambiente di sviluppo va lanciato solo se si aggiunge un nuovo file
cd $(dirname "$0")
environment=${1:-vagrant}
conf="$HOME/Confs/$environment.conf"
if [ -f $conf ] ; then
	echo '#ifndef CONFIG_HPP_' >src/config.hpp.in
	echo '#define CONFIG_HPP_' >>src/config.hpp.in
	cat  $HOME/Confs/$environment.conf | grep -ve '^#' | sed -e 's/#/\/\//' | sed -e 's/=/ /' | sed -e 's/\(.*\)/#define \1/' >>src/config.hpp.in
	echo '#endif /*CONFIG_HPP_*/' >>src/config.hpp.in
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
	exit 255
fi

