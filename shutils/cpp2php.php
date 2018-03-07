#!/usr/bin/php
<?php
error_reporting(E_RECOVERABLE_ERROR);
error_reporting(E_ALL ^E_NOTICE);
chdir(dirname(__FILE__)); // Sane environment
require('./phplib/getopt.php');
$opts=new getopts([
    '&debug'=>[false,'Show generated php instead of executing it'],
    '&json:'=>[false,'Merge with this file'],
]
);
if ($opts->help()) {
    $opts->showHelp("cpp2php [options] filename:",true);
}
$fname=realpath($opts->argv(1));
if (!$fname) {
    $opts->showHelp("Invalid file:",true);
    exit;
}
$lines=file($fname);
array_walk($lines,function(&$value,$key) {$value=trim($value);});
$php="";
if ($opts->json())  {
    $appo=@json_decode(@file_get_contents($opts->json()),true);
    $constants=$appo['defines'];
    $enums=$appo['enums'];
}
else {
    $constants=[];
    $enums=[];
}
foreach( $lines as $line) {
    if (strpos($line,'*ALARMUD*')) continue;
    if (strpos($line,'#define')!==false) {
        list($dummy,$name,$value,$extra)=preg_split('/\s+/',$line,4);
        if (strncmp($name,'__',2)) {
            if (empty($value)) $value='null';
            $php.=sprintf("\$constants['%s']=['value'=>%s,'comment'=>'%s'];\n",$name,$value,$multicomment.=clean($extra));
            $php.=sprintf("define('%s',%s);\n",$name,$value);
            $multicomment='';
        }
    }
    elseif (strpos($line,'enum ')!==false) {
        list($dummy,$name,$value,$extra)=preg_split('/\s+/',$line,4);
        $php.=sprintf("\$enums['%s']=[ 'comment'=>'%s',\n",$name,$multicomment.=clean($extra));
        $multicomment='';
    }
    elseif (strpos($line,'#')===0) {
        
    }
    elseif (strpos($line,'}')!==false) {
        $php.=str_replace('}',']',$line)."\n";
    }
    elseif (strpos($line,'=')!==false) {
        $line=str_replace('=',' = ',$line);
        $line=str_replace(',',' ',$line);
        list($name,$op,$value,$extra)=preg_split('/\s+/',$line,4);
        if (!empty($name)) {
            $php.=sprintf("'%s'=>['value'=>%s,'comment'=>'%s'],\n",$name,$value,$multicomment.clean($extra));
        }
    }
    else { 
        $comment=clean($line);
        if (!empty($comment)) {
            $multicomment.="$comment\n";
        }
    }
}
$php.='echo json_encode(["defines"=>$constants,"enums"=>$enums],JSON_PRETTY_PRINT),"\n";';
if ($opts->debug()) {
    echo "<?php\n$php";
}
else {
    eval($php);
}
function clean($stringa) {
    $stringa=preg_replace(['|^\s*\/*\**|','|\**\/\s*$|'],['',''],$stringa);
    return trim(addslashes($stringa));    
}
