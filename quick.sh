#!/bin/bash
# Utilizzato dal sistema di deploy.
# In ambiente di sviluppo va lanciato solo se si aggiunge un nuovo file
cd $(dirname "$0")
make
if [ -x src/myst ] ; then
	cp src/myst mudroot/myst
	exit 0
fi
exit 1

