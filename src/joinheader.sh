#!/bin/bash
#Source cleanup

declare -a reserved
reserved=(
config.hpp
typedefs.hpp
flags.hpp
autoenums.hpp
structs.hpp
logging.hpp
constants.hpp
utils.hpp
)
len=${#reserved[@]}
(
#	echo "/***************************  System  include ************************************/"	
#	cat $sysheaders
	index=0
#	echo "/***************************  General include ************************************/"
index=0
while [ "$index" -lt "$len" ] ; do
    cat ${reserved[$index]}   
    ((index++))
done
for i in *.hpp ; do 
	if [[ ${reserved[@]} =~ $i ]] ; then
		echo -e ''
	else
		cat $i
	fi
	
done
) > joined.h

