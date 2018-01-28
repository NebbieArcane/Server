#!/bin/bash
autoconf && aclocal & automake --add-missing && autoreconf && ./configure && make

