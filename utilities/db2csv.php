<?php
error_reporting(E_ALL ^E_NOTICE);
$emails=[];
$players=[];
$h=popen('./convert ../mudroot/lib/regdb_i.db','r');
while ($row=fgets($h)) {
    list($index,$email)=explode(' :@: ',trim($row));
    $emails[$index]=$email;
}
pclose($h);
$h=popen('./convert ../mudroot/lib/regdb_d.db','r');
while ($row=fgets($h)) {
    list($name,$data)=explode(' :@: ',trim($row));
    list($key,$god)=explode(' ',$data);
    $players[$name]=[
      'god'=>$god,
      'email'=>[$emails[$key]],
    ];
}
pclose($h);
echo "From regdb: ",count($players),"\n";
$fnames=glob("./register*");
foreach($fnames as $fname) {
    echo "Reading $fname\n";
    foreach(file($fname) as $row) {
        if ($row[0]!='*') {
            list($name,$god,$code,$realname,$email1,$email2,$note)=explode(',',trim(str_replace(['*','..','no email','no_email'],'',$row)));
            if (!array_key_exists($name, $players)) {
                $players[$name]=['email'=>[$email1],'god'=>$god];
                //echo "New: $name\n";
                if (!is_array($players[$name]['email'])) {
                    print_r($players[$name]);
                    die();
                }
            }
            else {
                if($god and $god!=$players['god']) {
                    $players[$name]['god2']=$god;
                }
                if($email1 and array_search($email1,$players[$name]['email'])===false) {
                    $players[$name]['email'][]=$email1;
                }
            }                
            if($realname) {
                $players[$name]['realname']=$realname;
            }
            if($email2 and array_search($email2,$players[$name]['email'])===false) {
                $players[$name]['email'][]=$email2;
            }
            if($note) {
                $players[$name]['note']=$note;
            }
        }
    }
    echo "Dopo $fname: ",count($players),"\n";
}
foreach ($players as $name=>$player) {
    foreach($player as &$val) {
        if (is_array($val)) {
            foreach($val as &$v) {
                $v=str_replace(['"',"'"],['\\"','\\"'],$v);
                if (!empty($v) and strpos($v,'@')===false) echo "$name bad mail: $v\n";
            }
        }
        $val=str_replace(['"',"'"],['\\"','\\"'],$val);
    }
    echo sprintf("INSERT INTO registered VALUES('%s','%s','%s','%s','%s','%s');\n",
        $name,ucwords(str_replace(['-','_'],' ',$player['realname'])),$player['god'],$player['email'][0],$player['email'][1],$player['note']);
}

