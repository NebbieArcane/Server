<?php
/**
 *
 * getopt. Nothing else needed.
 * @author giovanni
 *
 */
class getopts {
	protected $short='';
	protected $long=array();
	protected $defaults=array();
	protected $map=array();
	protected $types=array();
	protected $argv=array();
	protected $mandatory=array();

	function __construct(array $config) {
		try {
			foreach (array_keys($config) as $key) {
				if (strpos($key,'&h')!==false) {
					throw new Exception("dummy");
				}
			}
			$config['&help']=false;
		}
		catch(Exception $e) {

		}
		foreach($config as $opt=>$default) {
			if (substr($opt,0,1)=='!') {
				$opt=substr($opt,1);
				$mandatory=true;
			}
			else {
				$mandatory=false;
			}
			$type=preg_replace('/[^:]/','',$opt);
			$opt=str_replace(':','',$opt);
			list($a,$b)=explode('&',$opt,2);
			if (empty($b)) {
				$b=$a;
				$a='';
			}
			$short=substr($b,0,1);
			$long=$a.$b;
			$this->short.=$short.$type;
			$this->long[]=$long.$type;
			if ($mandatory) $this->mandatory[]=$long;
			$this->map[$short]=$long;
			if (!is_array($default)) {
				$default=array($default);
			}
			$this->defaults[$long]=array_shift($default);
			$this->help[$long]=array_shift($default);
			$this->hints[$long]=array_shift($default);
			$this->positionals[$long]=array_shift($default);
			if (empty($this->hint[$long]) and $type>=":") {
				$this->hints[$long]=strtolower($long);
			}
			$this->types[$long]=$type;
		}
		$this->get();
	}
	function optionList($message="") {
		echo $message;
		foreach ($this->map as $short=>$long ) {
			$m="-$short";
			if ($this->types[$long]) $m.=" {$this->hints[$long]}";
			if ($this->types[$long]!="::") {
				printf(" [%s]",$m);
			}
			else {
				echo " " . $m;
			}
		}
	}
	function usage($message="") {
		echo $message;
		foreach ($this->map as $short=>$long ) {
			$m="\t-{$short} --{$long}: ";
			$m.=$this->help[$long];
			if ( $this->defaults[$long] ) {
				if ($this->types[$long]) {
					$m.=" ({$this->defaults[$long]})";
				}
				else {
					$m.=" (" . ( $this->defaults[$long] ? "on" : "off" ) . ")";
				}
			}
			echo "$m\n";
		}
	}
	function set($option,$value=true) {
		$this->result[$option]=$value;
	}
	protected function get(&$argv=NULL) {
		$opts=getopt($this->short,$this->long);
		if (is_null($argv)) $argv=$GLOBALS['argv'];
		$this->argv=array();
		//Cleanups config switch
		foreach ($argv as $arg) {
			if (strncasecmp($arg,'-',1)) {
				if (!array_search($arg,$opts,true)) {
					$this->argv[]=$arg;
				}
			}
		}
		//Remaps all switch to long version
		$stdopts=array();
		foreach ($opts as $k=>$v) {
			if (array_key_exists($k,$this->map)) $k=$this->map[$k];
			$t=$this->types[$k];
			if (empty($t)) $v=true;
			$stdopts[$k]=$v;
		}
		//Kicks in positionals
		foreach ($this->positionals as $k=>$v) {
			if (!array_key_exists($k,$stdopts) and $v) $stdopts[$k]=$this->argv[$v];
		}
		//Kicks in defaults
		foreach ($this->defaults as $k=>$v) {
			if (!array_key_exists($k,$stdopts)) $stdopts[$k]=$v;
		}
		foreach ($this->mandatory as $k) {
			if (empty($stdopts[$k]))  {
				$this->usage("Missing mandatory parameter $k\nOptions:\n");
				echo "\n";
				exit(1);
			}
		}
		$this->result=$stdopts;
		return $stdopts;
	}
	function opts($opt=null) {
		if (is_null($opt)) return $this->result;
		if (array_key_exists($opt,$this->result)) return $this->result[$opt];
		$opt=$this->map[$opt];
		if (array_key_exists($opt,$this->result)) return $this->result[$opt];
		return null;
	}
	function argv($i=null) {
		if (is_null($i)) return $this->argv;
		return $this->argv[$i];
	}
	function __call($method,$arg) {
		if (strncasecmp($method,'arg',3)==0) {
			return $this->argv(intval(substr($method,3)));
		}
		else {
			return $this->opts($method);
		}
	}
	function constants($prefix='OPT') {
		foreach($this->result as $k=>$v) {
			define("{$prefix}_{$k}",empty($v)?false:$v);
		}
	}
	function atLeastOne() {
		$args=func_get_args();
		if ($args) {
			foreach($args as $arg) {
				if ($this->result[$arg]) return true;
			}
		}
		else {
			foreach ($this->result as $k=>$v) {
				if ($v and $k!='help') return true;
			}
		}
		return false;
	}
	function showHelp($message="",$stopHere=false) {
		if ($this->help()) {
			if (empty($message)) {
				$message=$this->argv(0);
			}
			echo "$message\n";
			$this->optionList("Usage: {$fname}");
			$this->usage("\nParams\n");
			if ($stopHere) exit(0);
		}
	}
}
