#!/bin/bash
name=${1:-sourcelist}
output=${2:-sourcelist}
extensions=${3:-cpp c}
format=${4:-cmake}
if [ format = 'autotools' ] ; then
echo -n "$name=" >$output
if [ format = 'cmake' ] ; then
	echo ''
fi
for ext in $extensions ; do
	for i in $(find ./src -name "*.$ext") ; do
		echo " \\" >> $output
		echo -n $(basename "$i" ) >> $output
		
	done	
done	
echo "">>$output
