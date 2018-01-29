#!/bin/bash
autoheader
aclocal
autoconf
touch sourcelist
automake --add-missing
automake
./configure 
make dist-clean
./configure
make


