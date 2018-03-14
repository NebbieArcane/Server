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
function addNamespace {
	header=$1.hpp
	work=$1.wrk
	sysheaders=$1.sys
	headers=$1.hdr
	nocomments=$1.noc
	comments=$1.com
	fname=$1.$2
	backup=$fname.bak
	cp $fname $backup
	grep -Ee '#include +<' $fname >$sysheaders
	grep -Ee '#include +"' $fname >$headers
	#rimuovo gli header che gestisco in modo automatico
	index=0
	while [ "$index" -lt "$len" ] ; do
        sed -i -e /${reserved[$index]}/d $headers   
        ((index++))
	done
	#rimuovo header corrispondente per riaggiungerlo al posto giusto
    sed -i -e /$header/d $headers   
	#rimuovo protos.hpp per includere solo il necessario
    sed -i -e /protos.hpp/d $headers   
	grep -v '#include' $fname | grep -v '*ALARMUD*' >$work
	# Ci possono essere diversi gruppi di commenti iniziali, meglio abbondare
	sed -e '1,/\*\// { /\/\*/,/\*\//d  } ' $work >$nocomments
	#sed -i -e '1,/\*\// { /\/\*/,/\*\//d  } ' $nocomments
	#sed -i -e '1,/\*\// { /\/\*/,/\*\//d  } ' $nocomments
	#sed -i -e '1,/\*\// { /\/\*/,/\*\//d  } ' $nocomments
	diff --changed-group-format="%>" --unchanged-group-format="" $nocomments $work >$comments
	#adesso ho:
# $work: il file senza commenti iniziali e senza include
# $comment i commenti inziali
# $sysheaders gli include di sistema
# $headers gli include locali
	(
	cat header.txt
	echo "//  Original intial comments"
	cat $comments
	echo "/***************************  System  include ************************************/"	
	cat $sysheaders
	index=0
	if [ $2 == 'cpp' ] ; then
		echo "/***************************  General include ************************************/"
		while [ "$index" -lt "$len" ] ; do
	        echo "#include \"${reserved[$index]}\""   
	        ((index++))
		done
	fi
	echo "/***************************  Local    include ************************************/"
	if [ $2 == 'cpp' ] ; then
		echo "#include \"$header\""
	fi
	cat $headers
	echo namespace Alarmud {
	cat $nocomments
	echo '} // namespace Alarmud'
	echo
	) > $fname
	rm -f $1.com  $1.wrk $1.sys $1.hdr $1.noc
}
len=${#reserved[@]}
(
cd $(dirname $0)
cd ../src
index=0
rm -f autoenums.?pp
echo "Starting..."
for i in *.cpp ; do
	name=$(basename $i .cpp)
	echo "Processing... $i"
	addNamespace $name cpp
done
for i in *.hpp ; do
	name=$(basename $i .hpp)
	echo "Processing... $i"
	addNamespace $name hpp
done
cd ../shutils
./code_generator.php
cp -f *.?pp ../src
)