#!/usr/bin/php
<?php
/**
 * Lancia la compilazione prendendo nota degli errori e calcolando gli include mancanti
 * 
 */
exec("make 2>&1",$match);
/*
 * C++
    c  classes
    d  macro definitions
    e  enumerators (values inside an enumeration)
    f  function definitions
    g  enumeration names
    l  local variables [off]
    m  class, struct, and union members
    n  namespaces
    p  function prototypes [off]
    s  structure names
    t  typedefs
    u  union names
    v  variable definitions
    x  external and forward variable declarations [off]
*/
if (!@file_exists("tags.json")) {
	exec("ctags -x --c-kinds=xvf --file-scope=no *.?pp",$rawtags);
	foreach ($rawtags as $tag) {
		list($name,$type,$line,$file)=preg_split('/\s+/',trim($tag),5);
		$tags[$name]=$file;
	}
	file_put_contents("tags.json",json_encode($tags));
}
else {
	$tags=json_decode(file_get_contents('tags.json'),true);
}
$errors=[];
foreach ($match as $riga) {
	$riga=str_replace(['‘','’'],"'",$riga);
	$reg="|(.*?):\d+:\d+\W*error: '(.*)'|";
	if (preg_match($reg,$riga,$error)) {
		$fname=basename($error[1]);
		$func=($error[2]);
		$errors[$fname][$func]=true;
	}
}
$includes=[];
$missing=[];
foreach ($errors as $fname =>$aerror) {
	foreach ($aerror as $error=>$dummy) {
		if(array_key_exists($error,$tags)) {
			$includes[$fname][]=str_replace('.cpp','.hpp',$tags[$error]);
		}
		else {
			$missing[$fname][]=$error;
		}
	}
}
if (empty($includes)) {
    foreach ($missing as $fname=>$item) {
        echo $fname,"\n";
        sort($item);
        $item=array_unique($item);
        foreach($item as $name) {
            echo "\tCheck $name\n";
            system("grep -n $name *.hpp");
        }
    }
}
foreach ($includes as $fname=>$item) {
	sort($item);
	$item=array_unique($item);
	echo "in $fname:\n";
	foreach($item as $name) {
		echo "#include \"$name\"\n";
	}
}
if (empty($missing) and empty($includes)) {
    echo implode("\n",$match);
}
