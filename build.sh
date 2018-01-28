#!/bin/bash
if [ ! -L install-sh ] ; then
autoconf && aclocal & automake --add-missing && autoconf 
fi
autoreconf && ./configure && make

