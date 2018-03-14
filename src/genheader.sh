#!/bin/bash
fname="$1.cpp"
if [ -z "$fname" ] ; then
	echo "Missing file name"
	exit
fi
fnameout="$1.hpp"
echo $fname to $fnameout
tag=$(echo "$fnameout" | sed -e 's/\./_/g' | tr [:lower:] [:upper:])
if [ -f "$fnameout" ] ; then
	echo "Appending, please check $fnameout"
else
	echo "#ifndef __$tag" >"$fnameout"
	echo "#define __$tag" >>"$fnameout"
fi
ctags -x --c-kinds=f --format=2 "$fname"  | sed -e 's/  \+/ /g' | cut -d' ' -f5- | sed -e 's/{/;/' >> "$fnameout"
echo "#endif // __$tag" >>"$fnameout" #This intentionally break the modified header file, so you are forced to check it
