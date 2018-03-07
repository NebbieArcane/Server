#!/usr/bin/php
<?php
error_reporting(E_RECOVERABLE_ERROR);
error_reporting(E_ALL ^E_NOTICE);
chdir(dirname(__FILE__)); // Sane environment
require('./phplib/getopt.php');
require('./phplib/alarphptemplate-1.0/alarphptemplate.inc.php');
$fname=realpath("./enums.json");
$data=json_decode(@file_get_contents($fname),true);
$globals=$data['enums'];
$program=basename(__FILE__);
// Loading protocol loop
$loop=array();
ksort($globals);
foreach ($globals as $enum=>$enums) {
	$item=array();
	$item['enum']=$enum;
	$max=0;
	$min=256*256;
	$accept0=true;
	$howmany=0;
	foreach ($enums as $desc=>$value) {
	    if (is_string($value)) {
	        $item['comment']=trim($value);
	    }
	    else {
	        ++$howmany;
    	    $comment=$value['comment'];
    	    $value=$value['value'];
    		$item['values'][]=array('name'=>$desc,'value'=>$value,'filler'=>str_repeat(' ',45-strlen($desc)),'comment'=>trim($comment));
    		$max=max($max,$value);
    		$min=min($min,$value);
    		if ($desc=='none' and !$value) {
    			$accept0=false;
    		}
	    }
	}
	$item['zero_is_valid']=$accept0;
	$item['max']=$max;
	$item['min']=$min;
	$item['count']=$howmany;
	$loop[]=$item;
}
$enums_loop=$loop;
$defines=$data['defines'];
$loop=[];
foreach ($defines as $desc => $val) {
    $loop[]=['name'=>$desc,'value'=>$val['value'],'comment'=>trim($value['comment']),'filler'=>str_repeat(' ',45-strlen($desc))];
}
$defines_loop=$loop;
$tmpl=new alarTemplate(
	array(
		'flatcache'=>true,
		'stripSpaces'=>true,
	)
);
$tmpl->addParam('generator',"$source by $program");
$tmpl->addParam('loop_enums',$enums_loop);
$tmpl->addParam('loop_defines',$defines_loop);
printFile('autoenums.cpp');
printFile('autoenums.hpp');
printFile('autoenums.wiki');
echo "Done\n";
function printFile($name,$dump=false) {
	global $tmpl;
	$tmplname=str_replace('.','_',$name).'.tmpl';
	$tmpl->resetTemplate();
	$tmpl->setFileName($tmplname);
	if ($dump) {
		echo $tmpl->output();
	}
	else {
		file_put_contents($name,$tmpl->output());
	}
	$tmpl->showFinalDebug();
}


