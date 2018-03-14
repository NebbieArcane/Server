#!/bin/bash
fname="$1"
if [ -z "$fname" ] ; then
	echo "Missing declaration name"
	exit
fi
ctags -x --c-kinds=xvf * | grep "$fname"
