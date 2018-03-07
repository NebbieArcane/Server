<?php
// BEGIN DEBUG
################################################################################
# PHP-HTML::Template                                                           #
# http://alarphptemplate.sourceforge.net/                                      #
################################################################################
# A template system for PHP based on HTML::Template Perl Module                #
# Version 1.0.0                                                                #
# 01-JAN-2009                                                                  #
# See file README for details                                                  #
################################################################################
# Author: Giovanni Gargani, giovanni@gargani.it                                #
# License: GNU LGPL (included in file "LICENSE")                               #
# (c) 2009 by Giovanni Gargani                                                 #
################################################################################
# HTML::Template Perl module copyright by Sam Tregar                           #
################################################################################
# Require php 5.2.x
/* ************** *
 * TEMPLATE CLASS *
 * ************** */
// END DEBUG
 /*
  * At this time it supports:
  * - <TMPL_VAR NAME=XXXXXXX>
  *		(An optional parameter ESCAPE can be added to escape var for HTML, JS ,URL or JSON)
  * - <TMPL_LOOP NAME=XXXXX></TMPL_LOOP> OK
  * - <TMPL_IF NAME=XXXXX>[<TMPL_ELSEIF NAME=XXX>...][<TMPL_ELSE>]</TMPL_IF> OK
  * - <TMPL_UNLESS NAME=XXXXX>[<TMPL_ELSEUNLESS NAME=XXX>...][<TMPL_ELSE>]</TMPL_UNLESS> OK
  * - <TMPL_INCLUDE NAME=XXXXXX> OK
  * - <TMPL_STATICINCLUDE NAME=XXXXXX>
  *		(Include a file without processing it)
  * - <TMPL_VAR_XML VERSION=XXXXXXX ENCODING=XXXXXX>
  *		(Make a XML PROLOGUE )
  * - <TMPL_PHP></TMPL_PHP>
  *     (Surround output in php tags
  * See README for detailed info.
  */
class alarTemplate {
	const DICT=1;
	const MISS=2;
	const MARK_ERROR=1;
	const MARK_SUCCESS=2;
	const MARK_WARNING=4;
	const MARK_INFO=8;
	const MARK_NOTICE=16;
	const MARK_TRACE=32;
	const MARK_DATA=64;
	const MARK_FULL=255;
	private $errLevels=array(
		self::MARK_ERROR=>"<font color='red'>ERROR</font>",
		self::MARK_SUCCESS=>"<font color='green'>SUCCESS</font>",
		self::MARK_WARNING=>"<font color='orange'>WARNING</font>",
		self::MARK_INFO=>"<font color='silver'>INFO</font>",
		self::MARK_NOTICE=>"<font color='yellow'>NOTICE</font>",
		self::MARK_TRACE=>"<font color='cyan'>TRACE</font>",
		self::MARK_DATA=>"<font color='lightyellow'>DATA</font>",
	);
	private $optionsMap=array(
		'searchpathoninclude'=>'setSearchOnInclude',
		'search_path_on_include'=>'setSearchOnInclude',
		'die_on_bad_params'=>'alwaysOff',
		'strict'=>'alwaysOff',
		'no_includes'=>'alwaysOff',
		'path'=>'setPaths',
		'case_sensitive'=>'setCase',
		'loop_context_vars'=>'alwaysOn',
		'max_includes'=>'notImplemented',
		'global_vars'=>'alwaysOn',
		'hash_comments'=>'alwaysOff',
		'parse_html_comments'=>'alwaysOff',
	);
	protected $dbgMessages=array();
	private $version='0001';
	private $options=array();
	private $data=array();
	private $deferredEval=false;
	private $testMode=true;
	private $includes=array();
	private $unique=0;
	private $currentContexts=array();
	private $cacheDir="/tmp/cache/"; // Default dir for compiled template cache
	private $timers=array();
	private $output=null;
	private $_regExp='/<(\/?)(tmpl_[^ >]+) *([^->]*?)>/i'; // Main regexp....simple but powerful
	private $regExp='';
	private $iMark="<";
	private $eMark=">";
	private $debugTemplate=false; //Show debug informations
	private $debugVars=false; //Print keys instead of values for standard keys
	private $debugDict=false; //Prints keys instead of values for dictionary keys
	private $includePaths=array(); //Array of paths to be searchd for template. Basedir of main template is always searched
	private $dictPrefix="DICT_";
	private $debugLevel=1;
	private $mustRefresh; //Filemtime of class. Used to
	private $templateRoot=''; //Default base dir (include path can be relative to this one)
	private $templateName;
	private $templateSearchOnInclude=true;
	private $templateIncludePaths=array();
	private $caseSensitive=false;
	private $flatCache=false;
	private $stripSpaces=false;
	private $cli=false;
	private $php;
	private $phpend;
	function version() {
		return $this->version;
	}
	function __construct($options=array()) {
		$this->php='<'.'?php';
		$this->phpend='?'.'>';
		$this->cli=PHP_SAPI=='cli';
		$this->regExp=$this->_regExp;
		$this->init($options);
		$this->markInfo("Regexp=".htmlspecialchars($this->regExp));
		$this->mustRefresh=filemtime(__FILE__);
	}

	function __destruct() {
		unset($this->callback);

	}
	function showFinalDebug() {
		if ($this->debugTemplate and !empty($this->dbgMessages)) {
			$this->markData("Template data",false,$this->dump());
			$this->jsProlog();
			echo $this->dbgOpen(true);
			if ($this->cli) {
    			fprintf(STDERR,"%s",@implode("\n",$this->dbgMessages));
			}
			else {
    			echo @implode("\n",$this->dbgMessages);
			}
    		echo $this->dbgClose();
		}

	}
	function raiseError($method,$message) {
		if ($this->useException)
			throw new Exception(get_class()."::$method - $message");
		else
			trigger_error(get_class()."::$method - $message",E_USER_ERROR);
	}
	function registerCallback($callback) {
		if (is_callable($callback))
			$this->fire($callback);
	}

	function fire($event,$args=array(),$default=null) {
		static $callback;
		if (func_num_args()==1 and is_callable($event)) {
			$callback=$event;
			return;
		}
		if (is_callable($callback)) {
			array_unshift($args,$event);
			return call_user_func_array($callback,$args);
		}
		else
			return $default;
	}
	function mark($level,$msg,$trace=false,$data=null) {
		static $unique=0;
		if (!$this->debugTemplate) return;
		//$msg=sprintf("\n%d\n%06d\n%06d",$level,decbin($level),decbin($this->debugLevel)) . $msg;
		if (!($level & $this->debugLevel)){
			 return;
		}
		$unique++;
		$s=$this->errLevels[$level]." - $msg ";
		if (!empty($data)) {
			if ($this->cli) {
				$s.=" => " . json_encode($data);
			}
			else {
				$s.="<a href=\"#\" onclick=\"return alartemplatedebughelper.toggle('tngappdebug$unique','block')\">Toggle</a><blockquote style=\"display:none\" id=\"tngappdebug$unique\">".print_r($data,1).'</blockquote>';
			}
		}
		$this->dbg($s);
	}
	function markError($msg,$trace=true,$data=null) {
		$this->mark(self::MARK_ERROR,$msg,$trace,$data);
	}
	function markSuccess($msg,$trace=true,$data=null) {
		$this->mark(self::MARK_SUCCESS,$msg,$trace,$data);
	}
	function markWarning($msg,$trace=false,$data=null) {
		$this->mark(self::MARK_WARNING,$msg,$trace,$data);
	}
	function markInfo($msg,$trace=false,$data=null) {
		$this->mark(self::MARK_INFO,$msg,$trace,$data);
	}
	function markNotice($msg,$trace=false,$data=null) {
		$this->mark(self::MARK_NOTICE,$msg,$trace,$data);
	}
	function markTrace($msg,$trace=false,$data=null) {
		$this->mark(self::MARK_TRACE,$msg,$trace,$data);
	}
	function markDebug($msg,$trace=false,$data=null) {
		$this->mark(self::MARK_TRACE,$msg,$trace,$data);
	}
	function markData($msg,$trace=false,$data=null) {
		$this->mark(self::MARK_DATA,$msg,$trace,$data);
	}
	private function _mark($args) {
		$this->mark($args[0],$args[1],$args[2],$args[3],$args[4]);
	}
	function dbgOpen($force=false) {
		if ($this->debugTemplate and !$this->cli) {
			if (!$force)
				return "<div id=debug><pre>\n";
			else
				return
	'<div style="clear:both"><hr><a href=# onclick="return alartemplatedebughelper.toggle(\'alartemplatedebughelperid\')">Debug</a></div>' .
	'<div id="alartemplatedebughelperid" style="border: thin inset Aqua;' .
	'display:none;'.
	'margin-left:20px;' .
	'margin-right:20px;' .
	'background-color: Black;' .
	'color: Silver;' .
	'font-family: monospace;"><pre>'."\n";
		}
	}
	function dbg($msg) {
		if ($this->debugTemplate) {
			array_push($this->dbgMessages,date('h:i:s') . ' - ' . $msg);
		}
	}

	function dbgClose() {
		if ($this->debugTemplate and !$this->cli) {
			return "\n</pre></div>";
		}
	}
	function __get($nome) {
		$nome=strtolower($nome);
		switch ($nome) {
			case "output": return $this->output;
			case "casesensitive" : return $this->caseSensitive;
			default: return null;
		}
	}
	function __call($method,$args) {
		if (!strncasecmp($method,"TMPL",4))
			$this->_mark(array('Error',"Unknown tag $method"));
		elseif (!strncasecmp($method,"MARK",4)) {
			array_unshift($args,substr($method,4));
			$this->_mark($args);
		}
		else {
			$this->_mark(array("Error","Unknown method $method"));
			throw new Exception("Missing method $method.");

		}
	}

	function inlineDebug($msg) {
		if ($this->cli) {
			fprintf(STDERR,"%s\n",$msg);
		}
		else {
      		print '<div style="margin: 10px 30px; padding: 5px; border: 2px solid black; background: gray; color: white; font-size: 12px; font-weight: bold;">';
      		echo $msg;
      		print '</div>';
		}
	}
	function jsProlog() {
		static $done=false;
		if ($done) return;
		$done=true;
		if ($this->cli) return;
$testo=<<<HEREDOC
<script type="text/javascript">
var alartemplatedebughelper={

getStyle: function (elem, style) {
  if (elem.getStyle) {
    return elem.getStyle(style);
  } else {
    return elem.style[style];
  }
}
,
setStyle: function(elem, style, value) {
  if (elem.setStyle) {
    elem.setStyle(style, value);
  } else {
    elem.style[style] = value;
  }
}
,
toggle: function(id) {
    var pre = document.getElementById( id);
    if (pre) {
      if (this.getStyle(pre, 'display') == 'block') {
        this.setStyle(pre, 'display', 'none');
      } else {
        this.setStyle(pre, 'display', 'block');
      }
    }
  	return false;
  }
}
</script>
HEREDOC;
	echo $testo;
	}

	function init($options) {
		$this->markInfo("Template init",false,$options);
		$this->options=array_change_key_case($options);
		foreach ($this->options as $option=>$value) {
			$method=$this->optionsMap[$option];
			if (empty($method)) {
				$method="set{$option}";
			}
			if (!method_exists($this,$method)) {
				$method='notImplemented';
			}
			$this->$method($value,$option);
		}
	}

	/* Helpers
	 *
	 */
	function getInput($var) {
		return $_REQUEST[$var];
	}
	function echoOutput() {
		if ($this->output)
			echo $this->output;
		else
			echo $this->output();
		$this->showFinalDebug();
	}

	function resolveFile($fname,$root='',$includePaths=array()) {
		if (substr($fname,0,1)== DIRECTORY_SEPARATOR) {
			$this->markNotice("Absolute $fname");
		}
		if (empty($root))
			$root=$this->templateRoot;
		if (empty($includePaths))
			$includePaths=(array)$this->templateIncludePaths;
		if (file_exists($fname))
			return realpath($fname);
		$this->markNotice("Resolving $fname",false,$includePaths);
		foreach ($includePaths as $path) {
			$this->markNotice("Trying $root$path$fname");
			$found=@realpath("$root$path$fname");
			if ($found) return $found;
		}
		$this->markError("$fname not found");
		return $fname;
	}


	/*
	 * Configuration method. Array format option is remapped to method
	 * based configuration
	 */
	function notImplemented($value,$option) {
		if (!$value)
			$this->markError("$option not implemented and currently undefined");
	}
	function alwaysOn($value,$option) {
		if (!$value)
			$this->markError("$option is always assumed true");
	}
	function alwaysOff($value,$option) {
		if ($value)
			$this->markError("$option is always assumed false");
	}
	function setPaths(array $path) {
		$this->templateIncludePaths=array();
		foreach((array)$path as $p) {
			$p=realpath($p);
			if ($p)
				$this->templateIncludePaths[]= "$p/";
		}

	}
	private function updateRegexp() {
		$this->regExp=str_replace('<',$this->iMark,$this->_regExp);
		$this->regExp=str_replace('>',$this->eMark,$this->regExp);
		$this->markSuccess("Changed regexp to " .$this->regExp);
	}
	function setIMark($flag) {
		$flag = preg_quote($flag);
		$this->iMark=$flag;
		$this->updateRegexp();
	}
	function setEMark($flag) {
		$flag = preg_quote($flag);
		$this->eMark=$flag;
		$this->updateRegexp();
	}
	function setTmplRoot($path) {
		$this->templateRoot=$path;
	}
	function setFlatCache($value) {
		$this->flatCache=$value;
	}
	function setstripSpaces($value) {
		$this->stripSpaces=$value;
	}
	function setFileName($path) {
		$this->templateName=$path;
	}
	function setSearchOnInclude($flag) {
		$this->templateSearchOnInclude=$flag;
	}
	function setCacheDir($path=null) {
		$this->cacheDir=$path;
	}
	function setNoCompile($flag) {
		$this->noCompile=$flag;
		if ($this->noCompile) {
			$this->mustRefresh=time();
			$this->markWarning("Template compilation disabled");
		}
	}
	function setCase($flag) {
		$this->caseSensitive=$flag;
	}

	function setDebug($flag) {
		$this->debugTemplate=$flag;
		if ($flag) {
		  $this->debugLevel=$this->debugLevel | self::MARK_SUCCESS | self::MARK_ERROR;
    }
	}
	function setDebugLevel($flag) {
		$this->debugLevel=$flag;

	}
	function setNoVars($flag) {
		$this->debugVars=$flag;
	}
	function setNoDict($flag) {
		$this->debugDict=$flag;
	}
	function setCallback($flag) {
		$this->fire($flag);
	}
	function output() {
		ob_start();
		$this->_output();
		$this->output=ob_get_clean();
		return $this->output;
	}
	function resetParams() {
		$this->resetTemplate();
		$this->data=array();
	}
	function resetTemplate() {
		$this->output=null;
	}
	private function _output() {
		$start=microtime(true);
		$this->rootData=$this->newNode("root",$this->data,1,1);
		$fname=$this->resolveFile($this->templateName);
		if (substr($fname,0,1)!='/') {
			$tag="INITIAL_SMARTINCLUDE NAME=$fname";
			$fname=$this->resolveFile($fname);
		}
		else {
			$tag="INITIAL_INCLUDE NAME=$fname";
		}
		$s=$this->CompileTemplate($fname,$tag);
		if ($this->deferredEval) {
			$this->_eval($s);
			if ($this->testMode) highlight_string($s);
		}
		$this->markInfo("Template ouput done in ". $this->elapsed($start));
	}


	function dump() {
		return $this->data;
	}

	function addParam($key,$value=null) {
		if (func_num_args()==1) {
			if (is_array($key))
				$this->paramArray($key);
			else
				$this->raiseError(__FUNCTION__,"Only one param and not an associative array");
		}
		elseif (func_num_args()==2) {
			if (is_array($key))
				$this->raiseError(__FUNCTION__,"Two params but first one is an array");
			if ($this->caseSensitive)
				$this->data[$key]=$value;
			else
				$this->data[strtoupper($key)]=$value;

		}
	}
	function param($key,$value) {
		$this->addParam($key,$value);
	}
	function paramArray(array $lista,$prefix='') {
		if ($prefix) $prefix.='_';
		foreach ($lista as $k=>$v) {
			$this->addParam($prefix.$k,$v);
		}
	}
	function paramLoop($nome,array $lista) {
		$this->addParam("LOOP_$nome",$lista);
	}
	function unQuote($stringa) {
		return  str_replace('"','',$stringa);
	}
	function quote(&$param) {
		$param=sprintf('"%s"',$param);
	}
	function wrap($stringa) {
		return ("{$this->php} {$stringa} {$this->phpend}");
	}
	/*
	 * Tag Management
	 */

	function includeFile($fname,$smart=false,$static=false,$flags=array()) {
		/* Deferring file name resolution to run time context */
		$tag=$flags['_FULL'];
		if ($static) {
			return $this->wrap("\$this->includeStaticFile('$fname','$smart','$tag')");
		}
		else {
			return $this->wrap("\$this->includeParsedFile('$fname','$smart','$tag')");
		}
	}
	function includeParsedFile($fname,$smart,$tag) {
		if (substr($fname,0,1)=='/') {
			$fname2=$fname; // absolute name, checking for smartness is pointless
			$this->markInfo("Absolute Including $fname:",false,$this->options);
		}
		else {
			$fname2=$this->resolveFile($fname);
			$this->markInfo("Including $fname:",false,$this->options);
		}
		return $this->parseTemplate($fname2,$tag);
	}
	function includeStaticFile($fname,$smart,$tag)
		{
		//--- Avoid to go updir
		$fname = str_replace("../","",$fname);

		if (preg_match("/^([A-Za-z0-9\_\.\/]+|\[[A-Za-z0-9\_]+\])+$/",$fname,$regs)) {

			$rfname = preg_replace_callback("/\[([A-Za-z0-9\_]+)\]/",array($this, 'replace_varname'),$fname);
				$fname2=$this->resolveFile($fname);

			$this->markInfo("Static including $fname:",false,$this->options);
			return $this->wrap("\$this->includeTemplate('$fname2','$tag');");
		}
		else
		{
			$this->markError("Invalid static name '$fname'");
			return "Invalid static name '$fname'. Sorry.<br>";
		}
		}

	function tmpl_smartinclude($flags) {
		if (empty($flags['NAME'])) {
			$this->markInfo("TMPL_SMARTINCLUDE without name=: ".htmlspecialchars($flags['_FULL']));
			return 'FAILED INCLUSION';
		}
		return $this->includeFile($flags['NAME'],true,false,$flags);
	}
	function tmpl_include($flags) {
		if (empty($flags['NAME'])) {
			$this->markInfo("TMPL_INCLUDE without name=: ".htmlspecialchars($flags['_FULL']));
			return 'FAILED INCLUSION';
		}
		return $this->includeFile($flags['NAME'],false,false,$flags);
	}

	function tmpl_staticinclude($flags) {
		if (empty($flags['NAME'])) {
			$this->markInfo("TMPL_STATICINCLUDE without name=: ".htmlspecialchars($flags['_FULL']));
			return 'FAILED INCLUSION';
		}
		return $this->includeFile($flags['NAME'],false,true,$flags);
	}

	function tmpl_var($flags) {
		$param=$flags['NAME'];
		$escape=$flags['ESCAPE'];
		return $this->wrap("\$this->show('$param','$escape');");
	}
	function tmpl_var_xml($flag) {
		$version=$flag['VERSION'];
		$encodinge=$flag['ENCODING'];
		return $this->wrap("echo '<'.'?xml version=\"$version\" encoding=\"$encoding\" ?'.'>';");
	}
	function tmpl_if($flags) {
		if ($flags['CLOSE'])
			return $this->wrap('}');
		else {
			$param=$flags['NAME'];
			return $this->wrap("if (\$this->isTrue('$param')) {");
		}
	}
	function tmpl_unless($flags) {

		if ($flags['CLOSE'])
			return $this->wrap('}');
		else {
			$param=$flags['NAME'];
			return $this->wrap("if (\$this->isFalse('$param')) {");
		}
	}
	function tmpl_else() {
		return $this->wrap(" } else {");
	}
	function tmpl_elseif($flags) {
		$param=$flags['NAME'];
		return $this->wrap(" } elseif  (\$this->isTrue('$param')) {");
	}
	function tmpl_elseunless($flags) {
		$param=$flags['NAME'];
		return $this->wrap(" } elseif  (\$this->isFalse('$param')) {");
	}
	function tmpl_loop($flags) {
		if ($flags['CLOSE']) {
			return $this->wrap('$this->popCurrentContext();}');
		}
		else {
			$param=$flags['NAME'];
			$prefix=$this->getContextPrefix($param);
			$k=$prefix."_key";
			$v=$prefix."_value";
			$d=$prefix."_data";
			return $this->wrap("\$$d=(array) \$this->getVar('$param');\$$k=0;foreach (\$$d as \$$v ) {\$this->pushCurrentContext('$prefix',\$$v,\$$k++,count(\$$d));");
		}
	}
	function tmpl_php($flags) {
		if ($flags['CLOSE']) {
			return $this->wrap("echo '?>';");
		}
		else {
			return $this->wrap("echo '<?php';");
		}
	}

	/**
	 * Data access
	 */

	function isTrue($param) {
		return (boolean)$this->getVar($param);
	}
	function isFalse($param) {
		return !(boolean)$this->getVar($param);
	}
	function getVar($param,$escape='',$show=false) {
		static $criticalSection=false;
		$param=strtoupper($param);
		$k=reset($this->currentContexts);
		if (!is_object($k))
			$k=$this->rootData;
		if (preg_match('/^\_\_([^_]*)\_\_$/U',$param,$match)) {
			if ($show and $this->debugVars) return "[$param]";
			$param=$match[1];
			$c=$k->context($param);
			$this->markNotice("Context var $param in " . $k->name().": $c",false,$k->dump());
			return $k->context($param);
		}
		elseif (!strncasecmp($param,$this->dictPrefix,strlen($this->dictPrefix))) {
			if ($show and $this->debugDict) return "[$param]";
			$this->markNotice("Used dictionary key: $param");
			return $this->escape($this->fire(self::DICT,array(substr($param,strlen($this->dictPrefix)),$k)),$escape);
		}
		else {
			if ($show and $this->debugVars) return "[$param]";
			foreach ($this->currentContexts as $context) {
				if ( $context->has($param)) {
					return $context->get($param,$escape);
				}
			}
		}
		//$this->markInfo("Accessing $param in root data");
		if ($this->rootData->has($param))
			return $this->rootData->get($param,$escape);
		return $this->escape($this->fire(self::MISS,array($param,$k)),$escape);
	}

	function debugInclude($tag,$result) {
		if (!$this->debugTemplate) return;
		$this->inlineDebug('<b>' .htmlspecialchars($tag) .":</b> $result");
		//"\n\n".'<table width="100%" border="1" bgcolor="#FF9999" cellpadding="1" cellspacing="1"><tr><td style="font-size: 12px; color: black;"><b>' .htmlspecialchars($tag) .":</b> $result</td></tr></table>\n\n";
	}


	function show($param,$escape='') {
		echo $this->getVar($param,$escape,true);
	}

	function pushCurrentContext($uid,$param,$iteration,$total) {
		$this->markDebug(sprintf("Memory before %dK", (memory_get_usage()/1024)));
		array_unshift($this->currentContexts,$this->newNode($uid,$param,$iteration,$total));
	}
	function popCurrentContext() {
		array_shift($this->currentContexts);

		$this->markDebug(sprintf("Memory after_ %dK", (memory_get_usage()/1024)));

	}
	/*
	 * CompileTime functions
	 */

	function getContextPrefix($param) {
		return sprintf("%s_%04d",$param,$this->unique++);
	}
	function parseParams($param) {
		if (empty($param)) return array();
		$param=preg_replace(array('/ +/','/ ?= ?/'),array(' ','='),$param); //squeeze
		$coppie=explode(' ',$param);
		$rv=array();
		foreach ($coppie as $coppia) {
			list($k,$v)=explode('=',$coppia);
			if (empty($v)) {
				$v=$k;
				$k='NAME';
			}
			else {
				$k=strtoupper($k);
			}
			$v=preg_replace('/[^\w\/\.\-\]\[\*]/','',$v);
			$rv[$k]=$v;
		}
		return $rv;
	}
	function parse($matches) {

		$match=$matches[0];
		$closed=$matches[1];
		$tag=$matches[2];
		$flags=$this->parseParams(trim($matches[3]));
		$flags['CLOSE']=!empty($closed);
		$flags['_FULL']=$match;
		extract($flags,EXTR_SKIP);
		$this->markNotice(htmlspecialchars($match). ": $tag NAME=$NAME ESCAPE=$ESCAPE");
		return $this->$tag($flags);
	}
	function compileTemplate($template,$tag) {
		return $this->parseTemplate($template,$tag);

	}
	function getCompiledName($fname) {
		return str_replace('//','/',$this->cacheDir . '/' . ($this->flatCache?basename($fname):$fname) . '.v'.$this->version.'.php');
	}
	function saveCompiledTemplate($fname,$s) {
		$output=$this->getCompiledName($fname);

		@mkdir(dirname($output),0777,true);
		$ok=file_put_contents($output,$this->wrap('/* <!'.gmdate('U').'!> Template cached on ' . gmdate('c') . "*/") . "\n$s");
		if ($ok>0) {
			$this->markSuccess("Saved compiled template to $output");
		}
		else {
			$this->markError("Unable to write $output for writing" ,true, error_get_last());
		}
	}
	function checkCompiledTemplate($fname) {
		$output=$this->getCompiledName($fname);
		$this->inlineDebug("Compiled name is $output");
		$s=@filemtime($fname);
		$c=@filemtime($output);
		if ($s and $c) {
			if ($c<$this->mustRefresh ) {
				$this->markWarning("Class rewritten. All templates not fresh ($fname)");
				return false;
			}
			if ($c>$s) {
				return $output;
			}
		}
		$this->markWarning(sprintf("Compiled template $fname missing or not fresh. Compiled: %s Source: %s",gmdate('c',$c),gmdate('c',$s)));
		return false;
	}
	function _eval($s) {
		$start=microtime(true);
		$ok=eval('?'.'>'.$s);
		if ($ok===false) {
			foreach (preg_split('/\v/',$s) as $line=>$row) {
				printf("%6d.",$line+1);
				highlight_string($row);
				echo "<br>";
			}
			die();
		}
		$start=microtime(true)-$start;
		$this->evalTime=$this->readingTime+$start;

	}
	function _file_get_contents($fname) {
		$start=microtime(true);
		$s=@file_get_contents($fname);
		$start=microtime(true)-$start;
		$this->readingTime=$this->readingTime+$start;
		return $s;
	}
	function includeTemplate($template,$originalname='',$tag='<TMPL_STATICINCLUDE>') {
		$this->markInfo("Starting include $template ");
		$this->debugInclude($originalname,$template,$tag);
		@readfile($template);
	}
	function parseTemplate($template,$originalname='',$tag='<TMPL_INCLUDE>') {
		$this->markTrace("Using regexp:" . $this->regExp);
		$deferredEval=$this->deferredEval;
		$this->debugInclude($originalname,$template,$tag);
		$this->markDebug("Starting compilation $template ". ($deferredEval?'in deferred mode':'in real time'));
		$this->includes[]=$template;
		$cname=$this->checkCompiledTemplate($template);
		if ($cname) {
			if ($deferredEval) {
				return $this->_file_get_contents($cname);
			}
			else {
				$start=microtime(true);
				include($cname);
				$this->incTime+=(microtime(true)-$start);
				$this->markSuccess("Included $cname from cached php in ".$this->elapsed($start));
			}
			return;
		}
		if (@file_exists($template)) {
			$s=$this->_file_get_contents($template);
			$start=microtime(true);
			$s=preg_replace_callback($this->regExp,array($this,"parse"),$s);
			if ($this->stripSpaces) {
				$lines=preg_split('/\v/',$s);
				$s="";
				foreach($lines as $line) {
					if (preg_match('/^\s*(<\?php.*\?>)\s*$/',$line,$matches)) {
						if (stripos($matches[1],'$this->show(')===false) {
							$s.=$matches[1]."\n";
						}
						else {
							$s.="$line\n";
						}
					}
					else {
						$s.="$line\n";
					}
				}

			}
			if (substr($s,0,5)=='<?xml') {
				$s=$this->wrap("echo '<?xml';") . substr($s,5);
			}
			$this->parsingTime+=(microtime(true)-$start);
			$this->markSuccess("Parsed $template in ".$this->elapsed($start));
			$this->saveCompiledTemplate($template,$s);
		}
		else {
			$this->markError("Missing template file [$template]");
			$s="<font color='red'>Missing template [$template]</font>";
		}
		/*
		 < : matches initial <
		 (\/?) : matches eventual / (closed tag)
		 (tmpl_[^ >]+) : matches tag name
		 ([^>])* : matches parameters
		 > : matches final >
		 */
		$this->markInfo("Compilation done $template");
		if ($deferredEval) {
			return $s;
		}
		else {
			$this->_eval($s);
		}
	}

	function tree() {
		return array('includes'=>$this->includes,
			'deferred'=>$this->deferredEval?'yes':'no',
					'reading time'=>$this->readingTime,
					'parsing time'=>$this->parsingTime,
					'eval time'=>$this->evalTime);
	}

	function newNode($uid,$data,$iteration,$total) {
		static $nodes=array();
		//if (!is_object($nodes[$uid])) $nodes[$uid];
		$nodes[$uid]=new alarTemplateNode($uid,$data,$iteration,$total,$this);
		return $nodes[$uid];
	}
	function elapsed($start) {
		return sprintf("%.5f",microtime(true)-$start);

	}
	function escape($rvalue,$escape) {
		switch ($escape) {
			case 'HTML':
				return htmlspecialchars($rvalue, ENT_QUOTES, $this->encoding);
			case 'JS':
				return addcslashes($rvalue, "\\\n\r\t\"'");
			case 'DOUBLE':
				return addcslashes($rvalue, "\\\n\r\t\"");
				case 'URL':
				return urlencode($rvalue);
			case '_AS_JSON':
			case 'JSON':
				return json_encode($rvalue);
			case 'AS_CDATA':
			case 'CDATA':
				return '<![CDATA['.$rvalue.']]>';
			case 'UPPERCASE':
			case 'UPPER':
				return strtoupper($rvalue);
			case 'LOWERCASE':
			case 'LOWER':
				return strtolower($rvalue);
			default:
				if (function_exists($escape)) {
					return call_user_func($escape,$rvalue);
				}
				return $rvalue;
		}
	}
}
class alarTemplateNode {
	private $data=array();
	private $uid="0";
	public $iteration=0;
	public $total=0;
	private $engine=null;
	function __construct($uid,$data,$iteration,$total,$engine) {
		$start=microtime(true);
		$this->uid=$uid;
		$this->data=$data;
		$this->engine=$engine;

		if ($uid=="root")
			$this->data=$data;
		else {
			if (!is_array($data)) {
				$this->engine->markError("Not an array creating context $uid iteration $iteration",false,$data);
				$this->data=array();
			}
			else {
				if (!$this->engine->caseSensitive)
					$this->data=array_change_key_case($data,CASE_UPPER);
			}
		}
		$this->iteration=$iteration+1;
		$this->total=$total;
		$this->engine->markNotice("Loaded new node for context $uid, iteration $iteration in ".$this->engine->elapsed($start));
	}

	function setIteration($iteration) {
		die("iterating");
		$this->iteration=$iteration+1;
		$this->engine->markInfo("Context $uid: Executing iteration $this->iteration of $this->total");
	}
	function name() {
		return $this->uid;
	}
	function has($key) {
		return array_key_exists(strtoupper($key), $this->data);

	}

	function get($key,$escape) {
		$rvalue=$this->data[strtoupper($key)];
		if ($escape) {
			$this->engine->markInfo("$key requested with escape: $escape");
		}
		return $this->engine->escape($rvalue,$escape);
	}
	function dump() {
		return $this->data;
	}
	function context($param) {

		switch (strtoupper($param)) {
			case "FIRST" : return $this->iteration==1;
			case "LAST" : return $this->iteration==$this->total;
			case "INNER" : return $this->iteration > 1 and $this->iteration < $this->total;
			case "COUNTER" : return $this->iteration;
			case "EVEN" : return  ($this->iteration % 2);
			case "ODD" : return  (($this->iteration+1) % 2);
			case "ROW{$this->iteration}" : return true;
		}
		return false;
	}
}

