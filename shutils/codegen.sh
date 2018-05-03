#!/bin/bash
php=$(which php)
if [ -z "$php" ] ; then
	echo "Missing php, skipping autoenum regen"
else
	(
	cd $(dirname $0)
	$php ./code_generator.php
	cp *.?pp ../src/
	)
fi