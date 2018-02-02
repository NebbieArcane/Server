#!/bin/bash
# Utilizzato dal sistema di deploy.
# In ambiente di sviluppo va lancianto solo se si aggiunge un nuovo file
cd $(dirname "$0")
rm -f mudroot/myst
rm -f  CMakeCache.txt
cmake .
make
if [ -x src/myst ] ; then
	cp src/myst mudroot/myst
fi

