#!/usr/bin/php
<?php
error_reporting(E_RECOVERABLE_ERROR);
error_reporting(E_ALL ^E_NOTICE);
chdir(dirname(__FILE__)); // Sane environment
require('./phplib/getopt.php');
require('./phplib/alarphptemplate-1.0/alarphptemplate.inc.php');
$fname=realpath("./enums.json");
$data=json_decode(@file_get_contents($fname),true);
$allenums=$data['enums'];
$defines=$data['defines'];
$allflags=$data['flags'];
$program=basename(__FILE__);
// ENUMS LOOP
$loop=array();
ksort($allenums);
foreach ($allenums as $enum=>$enums) {
    $loop[]=genFlagEnum($enum, $enums);

}
$enums_loop=$loop;
// FLAGS LOOP
$loop=array();
ksort($allflags);
foreach ($allflags as $flag=>$flags) {
    $loop[]=genFlagEnum($flag, $flags);
}
$flags_loop=$loop;

//DEFINES LOOP
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
$tmpl->addParam('loop_flags',$flags_loop);
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
function genFlagEnum($enum,$enums) {
    asort($enums,SORT_ASC);
    $item=array();
    $item['key']=$enum;
    $max=0;
    $min=256*256;
    $accept0=null;
    $howmany=0;
    foreach ($enums as $desc=>$value) {
        if (is_string($value)) {
            $item['comment']=trim($value);
        }
        else {
            ++$howmany;
            $comment=$value['comment'];
            $value=$value['value'];
            if (is_null($accept0)) {
                if ($value !==0) {
                    list($name,$dummy)=explode('_',$desc,2);
                    $name.="_NONE";
                    //$item['values'][]=array('name'=>$name,'value'=>0,'filler'=>str_repeat(' ',45-strlen($name)),'comment'=>trim($comment));
                    $accept0=false;
                }
                else {
                    $accept0=true;
                }
            }
            $item['values'][]=array('name'=>$desc,'value'=>$value,'filler'=>str_repeat(' ',45-strlen($desc)),'comment'=>trim($comment));
            $max=max($max,$value);
            $min=min($min,$value);
        }
    }
    $item['zero_is_valid']=$accept0;
    $item['max']=$max;
    $item['min']=$min;
    $item['count']=$howmany;
    return $item;
}
