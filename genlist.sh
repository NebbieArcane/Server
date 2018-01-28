#!/bin/bash
name=${1:-sourcelist}
output=${2:-sourcelist}
extensions=${3:-cpp c}
echo -n "$name=" >$output
for ext in $extensions ; do
	for i in $(find ./src -name "*.$ext") ; do
		echo " \\" >> $output
		echo -n $(basename "$i" ) >> $output
		
	done	
done	
echo "">>$output
