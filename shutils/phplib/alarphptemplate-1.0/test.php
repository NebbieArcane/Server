<?php
try {
require("alarphptemplate.inc.php");
$debugLevel=alartemplate::MARK_ERROR | alartemplate::MARK_SUCCESS | alartemplate::MARK_DATA;
//$debugLevel=alartemplate::MARK_FULL;
$options=array(
	'debug'=>true,
	'debugLevel'=>$debugLevel,
	'callback'=>'callback',
	'filename'=>'test.html',
	);
/* 
 * Calling contructor with $options array or invoking setter after "new"
 * is equivalent
 */
$number=rand(0,1);
if ($number) {
	$tmpl=new alarTemplate();
	$tmpl->setDebug(true);
	$tmpl->setDebugLevel($debugLevel);
	$tmpl->setCallback("callBack");
	$tmpl->setFileName("test.tmpl");
	$tmpl->param('method',"Inited via setters");
}
else {
	$tmpl=new alarTemplate($options);	
	$tmpl->param('method',"Inited via options array");
}
$tmpl->setNoCompile(true);
$tmpl->param("WELCOME","AlarTemplate class used: ");
$tmpl->addParam(array("GIRO"=>array(array("Nome"=>"pippo"),
									array("Nome"=>"pluto"),
									array("Nome"=>"paperino"))));
									
$tmpl->addParam("GIRetto",array(array("nome"=>"pippo"),
									array("nome"=>"pluto"),
									array("nome"=>"paperino")));									
$tmpl->echoOutput();
}
catch (exception $e) {
	echo "<pre>";
	print_r($e);
	echo "</pre>";
}
function callBack($event,$data,$context) {
	if ($event==alartemplate::DICT)
		return 'we could set up an external dictionary and have it be called via callback';
	else
		return null;
}
?>
