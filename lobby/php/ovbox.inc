<?php

function print_head( $user = "" )
{
  echo "<!DOCTYPE html>\n";
  echo '<html><head><title>House of Consort';
  if( !empty($user) )
    echo ' - ' . $user;
  echo '</title><style>';
  if( file_exists( 'ovbox.css' ) ){
    $css = file_get_contents( 'ovbox.css' );
    echo $css;
  }
  echo '</style>';
  //echo '<script>location.href="/";</script>';
  //echo '<meta http-equiv="refresh" content="8; url=/"/>';
  echo '<script>function toggledisplay(id,msg){var x=document.getElementById(id);if(x.style.display==="none"){x.style.display="block";event.target.value=\'hide \'+msg;}else{x.style.display="none";event.target.value=\'show \'+msg;}}</script>';
  echo '<meta name="viewport" content="width=device-width, initial-scale=1"/>';
  echo '</head><body>';
  echo '<h1>House of Consort</h1>';
}

function print_foot( $show_donate = true )
{
  if( $show_donate ){
    echo '<div class="donate">';
    echo '<form action="https://www.paypal.com/cgi-bin/webscr" method="post" target="_top">';
    echo '<input type="hidden" name="cmd" value="_s-xclick" />';
    echo '<input type="hidden" name="hosted_button_id" value="V63WV7SAVQ6EC" />';
    echo '<input type="image" src="https://www.paypalobjects.com/en_US/DK/i/btn/btn_donateCC_LG.gif" border="0" name="submit" title="PayPal - The safer, easier way to pay online!" alt="Donate with PayPal button" />';
    echo '<img alt="" border="0" src="https://www.paypal.com/en_DE/i/scr/pixel.gif" width="1" height="1" />';
    echo '</form>';
    echo '</div>';
  }
  echo '<div class="foto">Background image by Johannes Gontarski.</div>';
  echo '</body>';
  echo '</html>';
}

function get_rooms()
{
  $rooms = [];
  foreach( glob( '../db/*.room') as $room){
    $rooms[] = basename($room,'.room');
  }
  return $rooms;
}

function select_userdev( $user, $dev )
{
  $fname = '../db/' . $user . '.userdevice';
  file_put_contents( $fname, $dev );
}

function rm_device( $dev )
{
  $fname = '../db/' . $dev . '.device';
  unlink($fname);
}

function rm_room( $room )
{
  $fname = '../db/' . $room . '.room';
  unlink($fname);
}

function get_device( $user )
{
  $device = '';
  $fname = '../db/' . $user . '.userdevice';
  if( file_exists( $fname ) ) {
    $devicefd = fopen( $fname, 'r' );
    if ( $devicefd ) {
      $device = trim(fgets( $devicefd ));
      fclose( $devicefd );
    }
  }
  return $device;
}

// device database:

function get_device_prop( $device )
{
  $prop = array( 'room'=>'',
  'pos'=>'0 0 0',
  'jittersend'=>5,
  'jitterreceive'=>5,
  'peer2peer'=>true,
  'reverb'=>true,
  'owner'=>'',
  'label'=>'',
  'age'=>3600*24*365*100,
  'inputport'=>'system:capture_1',
  'inputport2'=>'',
  'srcdist'=>1.2,
  'access'=>0 );
  $fname = '../db/' . $device . '.device';
  if( file_exists( $fname ) ){
    $prop = array_merge($prop, json_decode( file_get_contents( $fname ), true ));
    $prop['age'] = time()-$prop['access'];
  }
  if( empty($prop['label']))
    $prop['label'] = $prop['owner'];
  if( empty($prop['label']))
    $prop['label'] = $device;
  return $prop;
}

function set_device_prop( $device, $dprop )
{
  $fname = '../db/' . $device . '.device';
  file_put_contents( $fname, json_encode( $dprop ));
}

function modify_device_prop( $device, $key, $value )
{
  $dprop = get_device_prop( $device );
  $dprop[$key] = $value;
  set_device_prop( $device, $dprop );
}

function agesort( $a, $b )
{
  if( $a['age'] > $b['age'] )
    return 1;
  if( $a['age'] < $b['age'] )
    return -1;
  return 0;
}

function list_devices()
{
  $devs = array();
  foreach( glob( '../db/*.device') as $dev){
    $device = basename($dev,'.device');
    $prop = get_device_prop($device);
    $devs[] = array('dev'=>$device,'age'=>$prop['age'],'owner'=>$prop['owner']);
  }
  usort( $devs, "agesort" );
  return $devs;
}

function list_unclaimed_devices()
{
  $devs = array();
  foreach( list_devices() as $dev){
    if( ($dev['age'] < 120) && empty($dev['owner']) )
      $devs[] = $dev['dev'];
  }
  return $devs;
}


function list_rooms()
{
  $rooms = array();
  foreach( glob( '../db/*.room') as $roomf){
    $room = basename($roomf,'.room');
    $prop = get_room_prop($room);
    $rooms[] = array('room'=>$room,'age'=>$prop['age']);
  }
  usort( $rooms, "agesort" );
  return $rooms;
}

function owned_devices( $user )
{
  $devs = array();
  foreach( list_devices() as $dev ){
    $prop = get_device_prop( $dev['dev']);
    if( $prop['owner'] == $user )
      $devs[] = array_merge($dev, $prop );
  }
  return $devs;
}

function lock_room( $room, $device, $lock )
{
  if( in_array($device, get_devices_in_room( $room ) ) ){
    modify_room_prop( $room, 'lock', $lock );
  }
}

function get_room_prop( $room )
{
  $prop = array( 'name'=>'', 'size'=>'25 13 7.5', 'rvbgain'=>-8, 'rvbdamp'=>0.7, 'rvbabs'=>0.6, 'host'=>'', 'port'=>'', 'pin'=>0, 'age'=>3600*24*365*100, 'lock'=>false, 'owner'=>'', 'access'=>0 );
  $fname = '../db/' . $room . '.room';
  if( file_exists( $fname ) ){
    $prop = array_merge($prop, json_decode( file_get_contents( $fname ), true ));
    $prop['age'] = time()-$prop['access'];
  }
  $dim = explode(" ",$prop['size']);
  if( count($dim)==3){
    $vol = 1;
    foreach($dim as $d)
      $vol = $vol*$d;
    $prop['volume'] = $vol;
    $t60 = 0.161*$vol/($prop['rvbabs']*2*($dim[0]*$dim[1]+$dim[1]*$dim[2]+$dim[2]*$dim[0]));
    $prop['t60'] = $t60;
    $prop['area'] = $dim[0]*$dim[1];
    $prop['sx'] = $dim[0];
    $prop['sy'] = $dim[1];
    $prop['sz'] = $dim[2];
  }else{
    $prop['volume'] = 0;
    $prop['t60'] = 0;
    $prop['area'] = 0;
    $prop['sx'] = 0;
    $prop['sy'] = 0;
    $prop['sz'] = 0;
  }
  return $prop;
}

function set_room_prop( $room, $dprop )
{
  $fname = '../db/' . $room . '.room';
  file_put_contents( $fname, json_encode( $dprop ));
}

function modify_room_prop( $room, $key, $value )
{
  $dprop = get_room_prop( $room );
  $dprop[$key] = $value;
  set_room_prop( $room, $dprop );
}

function update_room( $host, $port, $name, $pin )
{
  $room = $host . ':' . $port;
  $dprop = get_room_prop( $room );
  if( empty($dprop['name']) )
    $dprop['name'] = $name;
  $dprop['pin'] = $pin;
  $dprop['access'] = time();
  set_room_prop( $room, $dprop );
}

function get_devices_in_room( $room )
{
  $devices = array();
  foreach( glob( '../db/*.device') as $dev){
    $device = basename($dev,'.device');
    $prop = get_device_prop( $device );
    if( $room == $prop['room'] ){
      if( array_key_exists( 'chair', $prop ) )
        $devices[$prop['chair']] = $device;
      else
        $devices[] = $device;
    }
  }
  return $devices;
}

function get_free_chair( $room )
{
  $devs = get_devices_in_room( $room );
  $keys = array_keys( $devs );
  $k = 0;
  while( in_array( $k, $keys ) )
    $k++;
  return $k;
}

function device_enter_room( $device, $room )
{
  $othersinroom = get_devices_in_room( $room );
  if( !in_array( $device, $othersinroom ) ){
    $dprop = get_device_prop( $device );
    $dprop['room'] = $room;
    $dprop['chair'] = get_free_chair($room);
    set_device_prop( $device, $dprop );
  }
}

function room_swap_devices( $dev1, $dev2 )
{
  $dprop = get_device_prop( $dev1 );
  $room = $dprop['room'];
  $members = get_devices_in_room( $room );
  $chair1 = -1;
  $chair2 = -1;
  foreach( $members as $key => $dev ){
    if( $dev == $dev1 )
      $chair1 = $key;
    if( $dev == $dev2 )
      $chair2 = $key;
  }
  if( ($chair1>=0) && ($chair2>=0) ){
    modify_device_prop( $dev1, 'chair', $chair2 );
    modify_device_prop( $dev2, 'chair', $chair1 );
  }
}

function xml_add_sound( &$source, $doc, $x, $y, $z, $port = '' )
{
  $sound = $doc->createElement("sound");
  $source->appendChild($sound);
  $sound->setAttribute("maxdist","50");
  $sound->setAttribute("x",$x);
  $sound->setAttribute("y",$y);
  $sound->setAttribute("z",$z);
  if( !empty($port) )
    $sound->setAttribute("connect",$port );
  return $sound;
}

function get_tascar_cfg( $device, $devhash )
{
  $cuccess = false;
  $localdevprop = get_device_prop( $device );
  // create TASCAR document:
  $doc = new DOMDocument();
  $session = $doc->createElement("session");
  $doc->appendChild($session);
  $session->setAttribute("license","CC0");
  $session->setAttribute("levelmeter_tc","0.5");
  $scene = $doc->createElement("scene");
  $session->appendChild($scene);
  $modules = $doc->createElement("modules");
  $session->appendChild($modules);
  // add receiver:
  $rec = $doc->createElement("receiver");
  $rec->setAttribute("type","ortf");
  $rec->setAttribute("name","master");
  $rec->setAttribute("delaycomp","0.05");
  $scene->appendChild($rec);
  // master connections
  $conn = $doc->createElement("connect");
  $conn->setAttribute("src","render.scene:master_l");
  $conn->setAttribute("dest","system:playback_1");
  $session->appendChild($conn);
  $conn = $doc->createElement("connect");
  $conn->setAttribute("src","render.scene:master_r");
  $conn->setAttribute("dest","system:playback_2");
  $session->appendChild($conn);
  // room specific setup:    
  if ( !empty( $localdevprop['room'] ) ){
    $room = $localdevprop['room'];
    $roomprop = get_room_prop( $room );
    $mychair = -1;
    if( $roomprop['age'] < 3600 ) {
      // add users
      $roomdev = get_devices_in_room( $room );
      ksort($roomdev);
      $daz = 360/count($roomdev)*(pi()/180);
      $az = 0;
      $radius = 1.7;
      foreach( $roomdev as $chair => $dev ){
        $az = $az+$daz;
        $pos_x = sprintf("%1.3f",$radius * cos( $az ));
        $pos_y = sprintf("%1.3f",$radius * sin( $az ));
        // get configuration of player device:
        $devprop = get_device_prop( $dev );
        $numsource = !empty($devprop['inputport']) + !empty($devprop['inputport2']);
        if( $numsource > 0 ){
          $source = $doc->createElement("source");
          $scene->appendChild($source);
          $inport = '';
          if ( $dev == $device ){
            $source->setAttribute("name","ego");
          }else{
            $source->setAttribute("name",$devprop['label']);
          }
          $dy = $devprop['srcdist'] * ($numsource > 1) * 0.5;
          if( !empty($devprop['inputport']) ){
            $inport = '';
            if ( $dev == $device )
              $inport = $devprop['inputport'];
            xml_add_sound( $source, $doc, 0.4, $dy, -0.7, $inport );
          }
          if( !empty($devprop['inputport2']) ){
            $inport = '';
            if ( $dev == $device )
              $inport = $devprop['inputport2'];
            xml_add_sound( $source, $doc, 0.4, -$dy, -0.7, $inport );
          }
          $xmlpos = $doc->createElement("position");
          $source->appendChild($xmlpos);
          $xmlpos->appendChild($doc->createTextNode("0 ".$pos_x." ".$pos_y." 0"));
          $xmlrot = $doc->createElement("orientation");
          $source->appendChild($xmlrot);
          $xmlrot->appendChild($doc->createTextNode("0 ".(180/pi()*($az+pi()))." 0 0"));
          if ( $dev == $device ){
            $mychair = $chair;
            // set receiver position:
            $xmlpos = $doc->createElement("position");
            $rec->appendChild($xmlpos);
            $xmlpos->appendChild($doc->createTextNode("0 ".$pos_x." ".$pos_y." 0"));
            $xmlrot = $doc->createElement("orientation");
            $rec->appendChild($xmlrot);
            $xmlrot->appendChild($doc->createTextNode("0 ".(180/pi()*($az+pi()))." 0 0"));
          }
        }
      }
      if( $localdevprop['reverb'] ){
        // reverb
        $reverb = $doc->createElement("reverb");
        $reverb->setAttribute("type","simplefdn");
        $reverb->setAttribute("volumetric",$roomprop['size']);
        $reverb->setAttribute("image","false");
        $reverb->setAttribute("fdnorder","5");
        $reverb->setAttribute("absorption",$roomprop['rvbabs']);
        $reverb->setAttribute("damping",$roomprop['rvbdamp']);
        $reverb->setAttribute("gain",$roomprop['rvbgain']);
        $scene->appendChild($reverb);
      }
      // modules
      $mod = $doc->createElement("system");
      $roomurl = explode( ":", $room );
      $myport = 4464+2*$mychair;
      $p2p = '';
      if( $localdevprop['peer2peer'] )
        $p2p = ' -2';
      $mod->setAttribute("command","../udpmirror/ovboxclient -s ".$roomprop['pin']." -d ". $roomurl[0] . " -p " . $roomurl[1] . " -c " . $mychair . " -l " . $myport . $p2p);
      $mod->setAttribute("onunload","killall ovboxclient ../udpmirror/ovboxclient");
      $modules->appendChild($mod);
      // receiver:
      foreach( $roomdev as $chair => $dev ){
        $devprop = get_device_prop( $dev );
        $devuser = $devprop['label'];
        $numsource = !empty($devprop['inputport']) + !empty($devprop['inputport2']);
        if ( ( $numsource > 0 ) && ($dev != $device) ){
          $iport = 4464+2*$chair;
          $buff = $localdevprop['jitterreceive']+$devprop['jittersend'];
          $chan = '1';
          if( $numsource > 1 )
            $chan = $chan . ',2';
          // zita
          $mod = $doc->createElement("system");
          $mod->setAttribute("command","zita-n2j --chan ".$chan." --jname " . $devuser . " --buf " . $buff . " 0.0.0.0 " . $iport);
          $mod->setAttribute("onunload","killall zita-n2j");
          $modules->appendChild($mod);
          // connection
          for($k=0;$k<$numsource;$k++){
            $conn = $doc->createElement("connect");
            $conn->setAttribute("src",$devuser . ":out_".($k+1));
            $conn->setAttribute("dest","render.scene:" . $devuser . ".".$k.".0");
            $session->appendChild($conn);
          }
        }
      }
      $numsource = !empty($localdevprop['inputport']) + !empty($localdevprop['inputport2']);
      if( $numsource > 0 ){
        $mod = $doc->createElement("system");
        //<system command=\"zita-j2n --chan 1 --jname sender --16bit 127.0.0.1 ${oport}\" onunload=\"killall zita-j2n\"/>"
        $mod->setAttribute("command","zita-j2n --chan ".$numsource." --jname sender --16bit 127.0.0.1 " . $myport);
        $mod->setAttribute("onunload","killall zita-j2n");
        $modules->appendChild($mod);
        if( !empty($localdevprop['inputport']) ){
          $conn = $doc->createElement("connect");
          $conn->setAttribute("src",$localdevprop['inputport']);
          $conn->setAttribute("dest","sender:in_1");
          $session->appendChild($conn);
        }
        if( !empty($localdevprop['inputport2']) ){
          $conn = $doc->createElement("connect");
          $conn->setAttribute("src",$localdevprop['inputport2']);
          $conn->setAttribute("dest","sender:in_".$numsource);
          $session->appendChild($conn);
        }
      }
      $mod = $doc->createElement("system");
      $mod->setAttribute("command","sleep 2;sleep 2");
      $modules->appendChild($mod);
      $mod = $doc->createElement("savegains");
      $modules->appendChild($mod);
      $mod = $doc->createElement("touchosc");
      $modules->appendChild($mod);
      $mod = $doc->createElement("system");
      $mod->setAttribute("command","node bridge.js");
      $modules->appendChild($mod);
      $success = true;
    }
  }
  if( !$success ){
    // add announcement source
    $source = $doc->createElement("source");
    $scene->appendChild($source);
    $source->setAttribute("name","announce");
    $egosound = xml_add_sound($source, $doc, 4, 0, 0 );
    $plugs = $doc->createElement("plugins");
    $egosound->appendChild($plugs);
    $sndfile = $doc->createElement("sndfile");
    $plugs->appendChild($sndfile);
    $sndfile->setAttribute("name","announce.flac");
    $sndfile->setAttribute("level","65");
    $sndfile->setAttribute("transport","false");
    $sndfile->setAttribute("loop","0");
  }
  $tsc = $doc->saveXML();
  $hash = hash("md5",$tsc);
  if( $hash != $devhash ){
    echo $hash . "\n";
    echo $tsc;
  }
}

function html_device_selector( $user, $currentdevice )
{
  $devs = list_unclaimed_devices();
  if( !empty($devs) ){
    echo '<div class="devclaim">';
    echo "Unclaimed active devices exist. If your device is active now, you may claim one of these devices: \n";
    foreach( $devs as $dev ){
      echo '<form><input type="hidden" name="claim" value="'.$dev.'"/><button>'.$dev."</button></form>\n ";
    }
    echo "</div>";
  }
  echo "<div>";
  echo '<select id="devselect" name="devselect" oninput="location.href=\'?devselect=\'+event.target.value;">';
  foreach( owned_devices( $user ) as $dev ){
    echo '<option value="'.$dev['dev'].'"';
    if( $currentdevice == $dev['dev'] )
      echo ' selected';
    echo '>'.$dev['dev'].' ('.$dev['label'].')</option>';
  }
  echo '</select>';
  echo '<input type="button" class="roomsettingstoggle" onclick="toggledisplay(\'devsettings\',\'device settings\');" value="show device settings"/>';
  echo "</div>\n";
}

function auth( $username, $password )
{
  $fname = '../ovpasswd';
  $pws = array();
  if( !file_exists( $fname ) )
    return false;
  $pws = json_decode( file_get_contents( $fname ), true );
  if( !array_key_exists( $username, $pws ) )
    return false;
  return password_verify( $password, $pws[$username] );
}

function html_show_room( $room, $device, $inroom, $user )
{
  $rprop = get_room_prop( $room );
  if( $rprop['age'] < 3600 ) {
    // only show active rooms
    $doc = new DOMDocument('1.0');
    $root = $doc->createElement('div');
    $doc->appendChild($root);
    if( $inroom )
      $root->setAttribute('class','myroom');
    else
      $root->setAttribute('class','room');
    $tit = $doc->createElement('div');
    $root->appendChild($tit);
    $span = $doc->createElement('span');
    $tit->appendChild($span);
    $span->setAttribute('class','rname');
    $span->appendChild($doc->createTextNode($rprop['name'].' '));
    $span = $doc->createElement('span');
    $tit->appendChild($span);
    $span->setAttribute('class','rdesc');
    $span->appendChild($doc->createTextNode('(size: '.$rprop['sx'].' x '.$rprop['sy'].' x '.$rprop['sz'].' m'));
    $sup = $doc->createElement('sup');
    $span->appendChild($sup);
    $sup->appendChild($doc->createTextNode('3'));
    $span->appendChild($doc->createTextNode(', T'));
    $sup = $doc->createElement('sub');
    $span->appendChild($sup);
    $sup->appendChild($doc->createTextNode('60'));
    $span->appendChild($doc->createTextNode(': '.sprintf("%1.2f",$rprop['t60']).' s)'));
    if( !empty($rprop['owner']) )
      $span->appendChild($doc->createTextNode(' managed by '.$rprop['owner']));
    $memb = $doc->createElement('div');
    $root->appendChild($memb);
    $memb->setAttribute('class','rmembers');
    $roomdev = get_devices_in_room( $room );
    if( $rprop['lock'] && empty($roomdev) ){
      modify_room_prop( $room, 'lock', false );
      $rprop['lock'] = false;
    }
    ksort($roomdev);
    foreach( $roomdev as $chair => $dev ){
      $devuser = get_device_prop($dev);
      $lab = $devuser['label'];
      $bclass = "psvmember";
      if( $devuser['age'] < 20 )
        $bclass = "actmember";
      $mtype = 'span';
      if( ($dev != $device) && $inroom )
        $mtype = 'a';
      $mem = $doc->createElement($mtype);
      $memb->appendChild($mem);
      $mem->setAttribute('class',$bclass);
      if ( $dev == $device )
        $mem->setAttribute('style','border: 2px solid #000000;');
      if( $mtype == 'a')
        $mem->setAttribute('href','?swapdev='.urlencode($dev));
      //echo htmlspecialchars($devuser) . ' ' . $chair;
      if( empty($lab) )
        $lab = $dev;
      $mem->appendChild($doc->createTextNode( htmlspecialchars($lab) ));
      $memb->appendChild($doc->createTextNode(' '));
    }
    $ctl = $doc->createElement('div');
    $root->appendChild($ctl);
    if( $inroom ) {
      $a = $doc->createElement('a');
      $ctl->appendChild($a);
      $a->setAttribute('href','?enterroom=');
      $a->appendChild($doc->createTextNode('leave room'));
      $ctl->appendChild($doc->createTextNode(' '));
      $a = $doc->createElement('a');
      $ctl->appendChild($a);
      if( $rprop['lock'] ){
        $a->setAttribute('href','?lockroom='.urlencode($room).'&lck=0');
        $a->appendChild($doc->createTextNode('unlock room'));
      }else{
        $a->setAttribute('href','?lockroom='.urlencode($room).'&lck=1');
        $a->appendChild($doc->createTextNode('lock room'));
      }
    } else {
      if( $rprop['lock'] ){
        $ctl->appendChild($doc->createTextNode('room is locked.'));
      }else{
        $a = $doc->createElement('a');
        $ctl->appendChild($a);
        $a->setAttribute('href','?enterroom='.urlencode($room));
        $a->appendChild($doc->createTextNode('enter'));
      }
    }
    if( $user == $rprop['owner'] ){
      // my room, provide settings box:
      $ctl->appendChild($doc->createTextNode(' '));
      if( !empty($roomdev) ){
        $a = $doc->createElement('a');
        $ctl->appendChild($a);
        $a->setAttribute('href','?clearroom='.urlencode($room));
        $a->appendChild($doc->createTextNode('kick'));
      }
      $tog = $doc->createElement('input');
      $ctl->appendChild($tog);
      $tog->setAttribute('type','button');
      $tog->setAttribute('class','roomsettingstoggle');
      $tog->setAttribute('onclick','toggledisplay("'.$room.'ctl","room settings");');
      $tog->setAttribute('value','show room settings');
      $rp = $doc->createElement('div');
      $root->appendChild($rp);
      $rp->setAttribute('style','display: none;');
      $rp->setAttribute('id',$room.'ctl');
      $rp->setAttribute('class','devprop');
      $div = $doc->createElement('div');
      $rp->appendChild($div);
      $div->setAttribute('class','roomsettingstitle');
      $div->appendChild($doc->createTextNode('Room settings:'));
      $form = $doc->createElement('form');
      $rp->appendChild($form);
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','hidden');
      $inp->setAttribute('name','setroom');
      $inp->setAttribute('value',$room);
      $form->appendChild($doc->createTextNode($room));
      $form->appendChild($doc->createElement('br'));
      $form->appendChild($doc->createTextNode('Name: '));
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','text');
      $inp->setAttribute('name','label');
      $inp->setAttribute('pattern','[a-zA-Z0-9]*');
      $inp->setAttribute('value',$rprop['name']);
      //<input type="text" name="label" pattern="[a-zA-Z0-9]*" value="'.$dprop['name'].'">
      $form->appendChild($doc->createElement('br'));
      $form->appendChild($doc->createTextNode('Size L x W x H [m]: '));
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','number');
      $inp->setAttribute('step','0.1');
      $inp->setAttribute('min','0');
      $inp->setAttribute('max','300');
      $inp->setAttribute('name','sx');
      $inp->setAttribute('value',$rprop['sx']);
      $inp->setAttribute('style','width: 50px;');
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','number');
      $inp->setAttribute('step','0.1');
      $inp->setAttribute('min','0');
      $inp->setAttribute('max','200');
      $inp->setAttribute('name','sy');
      $inp->setAttribute('value',$rprop['sy']);
      $inp->setAttribute('style','width: 50px;');
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','number');
      $inp->setAttribute('step','0.1');
      $inp->setAttribute('min','0');
      $inp->setAttribute('max','100');
      $inp->setAttribute('name','sz');
      $inp->setAttribute('value',$rprop['sz']);
      $inp->setAttribute('style','width: 50px;');
      $form->appendChild($doc->createElement('br'));
      $form->appendChild($doc->createTextNode('Gain / dB: '));
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','number');
      $inp->setAttribute('step','0.1');
      $inp->setAttribute('name','rvbgain');
      $inp->setAttribute('min','-20');
      $inp->setAttribute('max','0');
      $inp->setAttribute('value',$rprop['rvbgain']);
      $inp->setAttribute('style','width: 50px;');
      $form->appendChild($doc->createTextNode(' Damping: '));
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','number');
      $inp->setAttribute('step','0.01');
      $inp->setAttribute('name','rvbdamp');
      $inp->setAttribute('min','0');
      $inp->setAttribute('max','1');
      $inp->setAttribute('value',$rprop['rvbdamp']);
      $inp->setAttribute('style','width: 50px;');
      $form->appendChild($doc->createTextNode(' Absorption: '));
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','number');
      $inp->setAttribute('step','0.01');
      $inp->setAttribute('min','0');
      $inp->setAttribute('max','1');
      $inp->setAttribute('name','rvbabs');
      $inp->setAttribute('value',$rprop['rvbabs']);
      $inp->setAttribute('style','width: 50px;');
      $form->appendChild($doc->createElement('br'));
      $inp = $doc->createElement('button');
      $form->appendChild($inp);
      $inp->appendChild($doc->createTextNode('Save'));
    }
    echo $doc->saveHTML();
  }
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * mode: php
 * End:
 */

?>