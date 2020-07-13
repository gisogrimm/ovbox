<?php

function defaults( $type )
{
  $defaults = array();
  $defaults['style'] = array(
    'credits'=>'',
    'css'=>'ovbox.css',
    'pagetitle'=>'House of Consort',
    'showdonate'=>true,
  );
  $defaults['device'] = array(
    'access'=>0,
    'label'=>'',
    'owner'=>'',
    'egogain'=>0,
    'host'=>'',
    'inputport'=>'system:capture_1',
    'inputport2'=>'',
    'jitterreceive'=>5,
    'jittersend'=>5,
    'outputport1'=>'system:playback_1',
    'outputport2'=>'system:playback_2',
    'xport'=>'{}',
    'peer2peer'=>true,
    'donotsend'=>false,
    'secrec'=>0,
    'pos'=>'0 0 0',
    'rawmode'=>false,
    'reverb'=>true,
    'rvbgain'=>0,
    'mastergain'=>0,
    'playbackgain'=>0,
    'room'=>'',
    'srcdist'=>0.7,
    'rectype'=>'ortf',
  );
  $defaults['room'] = array(
    'access'=>0,
    'label'=>'',
    'owner'=>'',
    'group'=>'',
    'lat'=>array(),
    'host'=>'',
    'lock'=>false,
    'pin'=>0,
    'port'=>'',
    'private'=>false,
    'rvbabs'=>0.6,
    'rvbdamp'=>0.7,
    'rvbgain'=>-8,
    'size'=>'25 13 7.5',
  );
  $defaults['user'] = array(
    'group'=>'',
    'seesall'=>true,
    'maingroup'=>'',
  );
  $defaults['group'] = array( 'style'=>'default' );
  if( array_key_exists( $type, $defaults ) )
    return $defaults[$type];
  return array();
}

function postproc_prop( &$prop, $type )
{
  if( array_key_exists('access',$prop) )
    $prop['age'] = time()-$prop['access'];
  if( $type == 'device' ){
    if( empty($prop['label']) && (!empty($prop['owner'])))
      $prop['label'] = $prop['owner'];
    $prop['label'] = preg_replace('/[^a-zA-Z0-9-_]*/','',$prop['label']);
  }
  if( $type == 'room' ){
    if( empty($prop['label']) && (!empty($prop['name'])))
      $prop['label'] = $prop['name'];
    unset($prop['name']);
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
  }
}


function get_client_ip() {
    $ipaddress = '';
    if (filter_var($_SERVER['HTTP_CLIENT_IP'],FILTER_VALIDATE_IP,FILTER_FLAG_IPV4))
      $ipaddress = $_SERVER['HTTP_CLIENT_IP'];
    else if(filter_var($_SERVER['HTTP_X_FORWARDED_FOR'],FILTER_VALIDATE_IP,FILTER_FLAG_IPV4))
      $ipaddress = $_SERVER['HTTP_X_FORWARDED_FOR'];
    else if(filter_var($_SERVER['HTTP_X_FORWARDED'],FILTER_VALIDATE_IP,FILTER_FLAG_IPV4))
      $ipaddress = $_SERVER['HTTP_X_FORWARDED'];
    else if(filter_var($_SERVER['HTTP_FORWARDED_FOR'],FILTER_VALIDATE_IP,FILTER_FLAG_IPV4))
      $ipaddress = $_SERVER['HTTP_FORWARDED_FOR'];
    else if(filter_var($_SERVER['HTTP_FORWARDED'],FILTER_VALIDATE_IP,FILTER_FLAG_IPV4))
      $ipaddress = $_SERVER['HTTP_FORWARDED'];
    else if(filter_var($_SERVER['REMOTE_ADDR'],FILTER_VALIDATE_IP,FILTER_FLAG_IPV4))
      $ipaddress = $_SERVER['REMOTE_ADDR'];
    else
      $ipaddress = 'UNKNOWN';
    return $ipaddress;
}

function get_properties( $id, $type )
{
  $fname = '../db/'.$id.'.'.$type;
  $prop = defaults($type);
  if( file_exists( $fname ) )
    $prop = array_merge($prop, json_decode( file_get_contents( $fname ), true ));
  postproc_prop( $prop, $type );
  return $prop;
}

function set_properties( $id, $type, $dprop )
{
  $fname = '../db/'.$id.'.'.$type;
  file_put_contents( $fname, json_encode( $dprop ));
}

function print_head( $user = "", $style = "" )
{
  $css = '';
  $title = 'House of Consort';
  if( !empty($style) ){

    $sprop = get_properties( $style, 'style' );
    $css = $sprop['css'];
    $title = $sprop['pagetitle'];
  }
  echo "<!DOCTYPE html>\n";
  echo '<html><head><title>'.$title;
  if( !empty($user) ){
    echo ' - ' . $user;
  }
  echo '</title><style>';
  if( file_exists( 'ovbox.css' ) ){
    echo file_get_contents( 'ovbox.css' );
  }
  if( (!empty($css)) && file_exists( $css ) ){
    echo file_get_contents( $css );
  }
  echo '</style>';
  //echo '<script>location.href="/";</script>';
  //echo '<meta http-equiv="refresh" content="300; url=/"/>';
  echo '<script>function toggledisplay(id,msg){var x=document.getElementById(id);if(x.style.display==="none"){x.style.display="block";event.target.value=\'hide \'+msg;}else{x.style.display="none";event.target.value=\'show \'+msg;}}</script>';
  echo '<script>function toggledisplayclass(id,msg){var x=document.getElementsByClassName(id);for(var k=0;k<x.length;k++){if(x[k].style.display==="none"){x[k].style.display="block";event.target.value=\'hide \'+msg;}else{x[k].style.display="none";event.target.value=\'show \'+msg;}}}</script>';
  echo '<meta name="viewport" content="width=device-width, initial-scale=1"/>';
  echo '</head><body>';
  echo '<h1>'.$title.'</h1>';
}

function print_foot($style = '')
{
  $show_donate = true;
  $credits = 'Background image by Johannes Gontarski.';
  if( !empty($style) ){
    $sprop = get_properties( $style, 'style' );
    $credits = $sprop['credits'];
    $show_donate = $sprop['showdonate'];
  }
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
  echo '<div class="foto">'.$credits.'</div>';
  echo '</body>';
  echo '</html>';
}

function get_rooms()
{
  $allrooms = list_db('room');
  ksort($allrooms);
  $rooms = array();
  foreach( $allrooms as $room=>$rprop){
    if( !empty($rprop['group']) )
      $rooms[] = $room;
  }
  foreach( $allrooms as $room=>$rprop){
    if( empty($rprop['group']) )
      $rooms[] = $room;
  }
  return $rooms;
}

function list_styles()
{
  $styles = [];
  foreach( glob( '../db/*.style') as $style){
    $styles[] = basename($style,'.style');
  }
  return $styles;
}

function select_userdev( $user, $dev )
{
  $fname = '../db/' . $user . '.userdevice';
  if( empty($dev) ){
    unlink($fname);
  }else{
    file_put_contents( $fname, $dev );
  }
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


function modify_device_prop( $device, $key, $value )
{
  $dprop = get_properties( $device, 'device' );
  $dprop[$key] = $value;
  set_properties( $device, 'device', $dprop );
}

function agesort( $a, $b )
{
  if( $a['age'] > $b['age'] )
    return 1;
  if( $a['age'] < $b['age'] )
    return -1;
  return 0;
}

function list_db($type)
{
  $devs = array();
  foreach( glob( '../db/*.'.$type) as $dev){
    $device = basename($dev,'.'.$type);
    $prop = get_properties($device,$type);
    $devs[$device] = $prop;
  }
  uasort( $devs, "agesort" );
  //ksort( $devs );
  return $devs;
}

function list_devices()
{
  return list_db('device');
}

function list_unclaimed_devices()
{
  $devs = array();
  foreach( list_devices() as $dev=>$devprop){
    if( ($devprop['age'] < 20) && empty($devprop['owner']) ){
      $devs[] = $dev;
    }
  }
  return $devs;
}

function owned_devices( $user )
{
  $devs = array();
  foreach( list_devices() as $dev=>$devprop ){
    if( $devprop['owner'] == $user )
      $devs[$dev] = $devprop;
  }
  return $devs;
}

function lock_room( $room, $device, $lock )
{
  if( in_array($device, get_devices_in_room( $room ) ) ){
    modify_room_prop( $room, 'lock', $lock );
  }
}

function modify_group_prop( $group, $key, $value )
{
  $dprop = get_properties( $group, 'group' );
  $dprop[$key] = $value;
  set_properties( $group, 'group', $dprop );
}

function modify_user_prop( $user, $key, $value )
{
  $dprop = get_properties( $user, 'user' );
  $dprop[$key] = $value;
  set_properties( $user, 'user', $dprop );
}

function get_room_prop( $room )
{
  $prop = get_properties( $room, 'room' );

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

function modify_room_prop( $room, $key, $value )
{
  $dprop = get_room_prop( $room );
  $dprop[$key] = $value;
  set_properties( $room, 'room', $dprop );
}

function update_room( $host, $port, $name, $pin )
{
  $room = $host . ':' . $port;
  $dprop = get_room_prop( $room );
  if( empty($dprop['label']) )
    $dprop['label'] = $name;
  $dprop['pin'] = $pin;
  $dprop['access'] = time();
  set_properties( $room, 'room', $dprop );
}

function update_room_lat( $host, $port, $src, $dest, $lat, $jit )
{
  $room = $host . ':' . $port;
  $dprop = get_room_prop( $room );
  $latdb = $dprop['lat'];
  $latdb[$src.'-'.$dest] = array('lat'=>$lat,'jit'=>$jit,'access'=>time());
  $dprop['lat'] = $latdb;
  set_properties( $room, 'room', $dprop );
}

function issender( $devprop )
{
  return (!empty($devprop['inputport']))||(!empty($devprop['inputport2']));
}

function get_devices_in_room( $room, $senderonly = false )
{
  $devices = array();
  foreach( glob( '../db/*.device') as $dev){
    $device = basename($dev,'.device');
    $prop = get_properties( $device, 'device' );
    if( ($room == $prop['room']) && ((!$senderonly) || issender($prop)) ){
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
    $dprop = get_properties( $device, 'device' );
    $dprop['room'] = $room;
    $dprop['chair'] = get_free_chair($room);
    set_properties( $device, 'device', $dprop );
  }
}

function room_swap_devices( $dev1, $dev2 )
{
  $dprop = get_properties( $dev1, 'device' );
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

function xml_add_sound( &$source, $doc, $attrs )
{
  $sound = $doc->createElement("sound");
  $source->appendChild($sound);
  $sound->setAttribute("maxdist","50");
  $sound->setAttribute("gainmodel","1");
  foreach( $attrs as $attr=>$value ){
    $sound->setAttribute($attr,$value);
  }
  return $sound;
}

function get_tascar_cfg( $device, $devhash )
{
  $cuccess = false;
  $waitports = '';
  $localdevprop = get_properties( $device, 'device' );
  $amsender = issender($localdevprop);
  // create TASCAR document:
  $doc = new DOMDocument();
  $session = $doc->createElement("session");
  $doc->appendChild($session);
  $session->setAttribute('duration','36000');
  $session->setAttribute('name',$device);
  $session->setAttribute('license','CC0');
  $session->setAttribute("levelmeter_tc","0.5");
  $scene = $doc->createElement('scene');
  $scene->setAttribute('name',$device);
  $scene->setAttribute('guiscale','25');
  if( ! $localdevprop['rawmode'] ){
    $session->appendChild($scene);
    $session->setAttribute('srv_port','9871');
  }else{
    $session->setAttribute('srv_port','0');
  }
  $modules = $doc->createElement("modules");
  $session->appendChild($modules);
  // add receiver:
  $rec = $doc->createElement("receiver");
  $rec->setAttribute('type',$localdevprop['rectype']);
  if( $localdevprop['rectype'] == 'ortf' ){
    $rec->setAttribute('angle','140');
    $rec->setAttribute('f6db','12000');
    $rec->setAttribute('fmin','3000');
  }
  $rec->setAttribute("name","master");
  $rec->setAttribute("delaycomp","0.05");
  $scene->appendChild($rec);
  // master connections
  if( ! $localdevprop['rawmode'] ){
    $conn = $doc->createElement("connect");
    $conn->setAttribute("src",'render.'.$device.':master_l');
    $conn->setAttribute("dest",$localdevprop['outputport1']);
    if( !empty($localdevprop['outputport1']))
      $session->appendChild($conn);
    $conn = $doc->createElement("connect");
    $conn->setAttribute("src",'render.'.$device.':master_r');
    $conn->setAttribute("dest",$localdevprop['outputport2']);
    if( !empty($localdevprop['outputport2']))
      $session->appendChild($conn);
  }
  // room specific setup:
  if ( !empty( $localdevprop['room'] ) ){
    $room = $localdevprop['room'];
    $roomprop = get_room_prop( $room );
    $mychair = -1;
    if( $roomprop['age'] < 3600 ) {
      // get all sending devives:
      $roomdev = get_devices_in_room( $room );
      foreach( $roomdev as $chair => $dev )
        if ( $dev == $device )
          $mychair = $chair;
      // get all sending devives:
      $roomdev = get_devices_in_room( $room, true );
      // sort by chair number:
      ksort($roomdev);
      // if I am sending sit in circle, otherwise use 120 degree stage
      $stagewidth = 160;
      $az = -80;
      if( $amsender ){
        $stagewidth = 360;
        $az = 0;
      }
      $daz = $stagewidth/count($roomdev)*(pi()/180);
      $az = $az*(pi()/180)-0.5*$daz;
      $radius = 1.2;
      foreach( $roomdev as $chair => $dev ){
        $az = $az+$daz;
        $pos_x = sprintf("%1.3f",$radius * cos( $az ));
        $pos_y = sprintf("%1.3f",-$radius * sin( $az ));
        // get configuration of player device:
        $devprop = get_properties( $dev, 'device' );
        $numsource = !empty($devprop['inputport']) + !empty($devprop['inputport2']);
        if( $numsource > 0 ){
          $dz = 0;
          $dx = 0;
          // relative gain:
          $gain = -4.41;
          $source = $doc->createElement("source");
          $scene->appendChild($source);
          $inport = '';
          if ( $dev == $device ){
            $source->setAttribute("name","ego");
            $dz = -0.3;
            $dx = 0.2;
            $gain = $devprop['egogain'];
          }else{
            $source->setAttribute("name",$devprop['label'].'_'.$chair);
          }
          $gain = $gain + $devprop['playbackgain'];
          $dy = $devprop['srcdist'] * ($numsource > 1) * 0.5;
          if( !empty($devprop['inputport']) ){
            $inport = '';
            if ( $dev == $device )
              $inport = $devprop['inputport'];
            $snd = xml_add_sound( $source, $doc, array('x'=>$dx, 'y'=>$dy, 'z'=>$dz, 'connect'=>$inport, 'gain'=>$gain) );
            if ( $dev == $device ){
              $plugs = $snd->appendChild($doc->createElement('plugins'));
              $sndf = $plugs->appendChild($doc->createElement('sndfile'));
              $sndf->setAttribute('name','akkord_up_48.wav');
              $sndf->setAttribute('level','60');
              $sndf->setAttribute('triggered','true');
              $sndf->setAttribute('transport','false');
              $sndf = $plugs->appendChild($doc->createElement('sndfile'));
              $sndf->setAttribute('name','akkord_down_48.wav');
              $sndf->setAttribute('level','60');
              $sndf->setAttribute('triggered','true');
              $sndf->setAttribute('transport','false');
            }
          }
          if( !empty($devprop['inputport2']) ){
            $inport = '';
            if ( $dev == $device )
              $inport = $devprop['inputport2'];
            $snd = xml_add_sound( $source, $doc, array('x'=>$dx, 'y'=>-$dy, 'z'=>$dz, 'connect'=>$inport, 'gain'=>$gain) );
            if ( $dev == $device ){
              $plugs = $snd->appendChild($doc->createElement('plugins'));
              $sndf = $plugs->appendChild($doc->createElement('sndfile'));
              $sndf->setAttribute('name','akkord_up_48.wav');
              $sndf->setAttribute('level','60');
              $sndf->setAttribute('triggered','true');
              $sndf->setAttribute('transport','false');
              $sndf = $plugs->appendChild($doc->createElement('sndfile'));
              $sndf->setAttribute('name','akkord_down_48.wav');
              $sndf->setAttribute('level','60');
              $sndf->setAttribute('triggered','true');
              $sndf->setAttribute('transport','false');
            }
          }
          $xmlpos = $doc->createElement("position");
          $source->appendChild($xmlpos);
          $xmlpos->appendChild($doc->createTextNode("0 ".$pos_x." ".$pos_y." 0"));
          $xmlrot = $doc->createElement("orientation");
          $source->appendChild($xmlrot);
          $xmlrot->appendChild($doc->createTextNode("0 ".(180/pi()*(-$az+pi()))." 0 0"));
          if ( $dev == $device ){
            $mychair = $chair;
            // set receiver position:
            $xmlpos = $doc->createElement("position");
            $rec->appendChild($xmlpos);
            $xmlpos->appendChild($doc->createTextNode("0 ".$pos_x." ".$pos_y." 0"));
            $xmlrot = $doc->createElement("orientation");
            $rec->appendChild($xmlrot);
            $xmlrot->appendChild($doc->createTextNode("0 ".(180/pi()*(-$az+pi()))." 0 0"));
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
        $reverb->setAttribute("dw","60");
        $reverb->setAttribute("absorption",$roomprop['rvbabs']);
        $reverb->setAttribute("damping",$roomprop['rvbdamp']);
        $reverb->setAttribute("gain",$roomprop['rvbgain']+$localdevprop['rvbgain']);
        $scene->appendChild($reverb);
      }
      // modules
      $mod = $doc->createElement("system");
      $roomurl = explode( ":", $room );
      $myport = 4464+2*$mychair;
      $p2p = '';
      if( $localdevprop['peer2peer'] )
        $p2p = ' -2';
      if( $localdevprop['donotsend'] )
        $p2p = $p2p.' --donotsend';
      if( $localdevprop['secrec'] > 0 )
        $p2p = $p2p . ' -x 100';
      $mod->setAttribute("command","../udpmirror/ovboxclient --rtprio=30 -s ".$roomprop['pin']." -d ". $roomurl[0] . " -p " . $roomurl[1] . " -c " . $mychair . " -l " . $myport . $p2p);
      $mod->setAttribute("onunload","killall ovboxclient ../udpmirror/ovboxclient");
      $modules->appendChild($mod);
      // receiver:
      foreach( $roomdev as $chair => $dev ){
        $devprop = get_properties( $dev, 'device' );
        $devuser = $devprop['label'];
        $numsource = !empty($devprop['inputport']) + !empty($devprop['inputport2']);
        if ( ( $numsource > 0 ) && ($dev != $device) ){
          $iport = 4464+2*$chair;
          $buff = $localdevprop['jitterreceive']+$devprop['jittersend'];
          $chan = '1';
          if( $numsource > 1 )
            $chan = $chan . ',2';
          // zita
          $mod = $modules->appendChild($doc->createElement('system'));
          $mod->setAttribute('command','zita-n2j --chan '.$chan.' --jname ' . $devuser . '_'.$chair.' --buf ' . $buff . ' 0.0.0.0 ' . $iport);
          $mod->setAttribute('onunload','killall zita-n2j');
          if( $localdevprop['secrec'] > 0 ){
            $mod = $modules->appendChild($doc->createElement('system'));
            $port = 'n2j_'.$chair.'_sec';
            $mod->setAttribute('command','zita-n2j --chan '.$chan.' --jname '.$port.' --buf ' . ($buff+$localdevprop['secrec']) . ' 0.0.0.0 ' . ($iport+100));
            $waitports = $waitports . ' ' . $port.':out_1';
            if( $numsource > 1 )
              $waitports = $waitports . ' ' . $port.':out_2';
            $mod = $modules->appendChild($doc->createElement('route'));
            $mod->setAttribute('name',$devuser . '_'.$chair.'_sec');
            $mod->setAttribute('channels',$numsource);
            $mod->setAttribute('gain',$devprop['playbackgain']);
            $mod->setAttribute('connect',$port.':out_[12]');
          }
          // connection
          for($k=0;$k<$numsource;$k++){
            $conn = $doc->createElement('connect');
            $port = $devuser . '_'.$chair.':out_'.($k+1);
            $waitports = $waitports . ' ' . $port;
            $conn->setAttribute('src',$port);
            if( $localdevprop['rawmode'] ){
              $conn->setAttribute('dest',$localdevprop['outputport'.($k+1)]);
              if( !empty($localdevprop['outputport'.($k+1)]))
                $session->appendChild($conn);
            }else{
              $conn->setAttribute('dest','render.'.$device.':' . $devuser . '_'.$chair.'.'.$k.'.0');
              $session->appendChild($conn);
            }
          }
        }
      }
      $numsource = !empty($localdevprop['inputport']) + !empty($localdevprop['inputport2']);
      if( $numsource > 0 ){
        $mod = $doc->createElement('system');
        //<system command=\'zita-j2n --chan 1 --jname sender --16bit 127.0.0.1 ${oport}\' onunload=\'killall zita-j2n\'/>'
        $mod->setAttribute('command','zita-j2n --chan '.$numsource.' --jname sender --16bit 127.0.0.1 ' . $myport);
        $mod->setAttribute('onunload','killall zita-j2n');
        $modules->appendChild($mod);
        if( !empty($localdevprop['inputport']) ){
          $conn = $doc->createElement('connect');
          $conn->setAttribute('src',$localdevprop['inputport']);
          $port = 'sender:in_1';
          $waitports = $waitports . ' ' . $port;
          $conn->setAttribute('dest',$port);
          $session->appendChild($conn);
        }
        if( !empty($localdevprop['inputport2']) ){
          $conn = $doc->createElement('connect');
          $conn->setAttribute('src',$localdevprop['inputport2']);
          $port = 'sender:in_'.$numsource;
          $waitports = $waitports . ' ' . $port;
          $conn->setAttribute('dest',$port);
          $session->appendChild($conn);
        }
        if( $localdevprop['secrec'] > 0 ){
          $mod = $modules->appendChild($doc->createElement('route'));
          $mod->setAttribute('name','egodelayed');
          $mod->setAttribute('channels',$numsource);
          $plgs = $mod->appendChild($doc->createElement('plugins'));
          $del = $plgs->appendChild($doc->createElement('delay'));
          $del->setAttribute('delay',0.001*($localdevprop['secrec']+$localdevprop['jitterreceive']+$localdevprop['jittersend']));
          $mod->setAttribute('gain',$localdevprop['playbackgain']);
          if( !empty($localdevprop['inputport']) ){
            $conn = $doc->createElement('connect');
            $conn->setAttribute('src',$localdevprop['inputport']);
            $conn->setAttribute('dest', 'egodelayed:in.0');
            $session->appendChild($conn);
          }
          if( !empty($localdevprop['inputport2']) ){
            $conn = $doc->createElement('connect');
            $conn->setAttribute('src',$localdevprop['inputport2']);
            $conn->setAttribute('dest','egodelayed:in.'.($numsource-1));
            $session->appendChild($conn);
          }
        }
      }
      $mod = $doc->createElement('system');
      //$mod->setAttribute('command','sleep 2;sleep 2');
      $mod->setAttribute('command','../udpmirror/jackwaitforport' . $waitports . ';sleep 1');
      $modules->appendChild($mod);
      $mod = $doc->createElement('touchosc');
      if( ! $localdevprop['rawmode'] )
        $modules->appendChild($mod);
      $mod = $doc->createElement('system');
      $mod->setAttribute('command','node bridge.js');
      if( ! $localdevprop['rawmode'] )
      $modules->appendChild($mod);
      $cmd = $modules->appendChild($doc->createElement('system'));
      $cmd->setAttribute('command','../headtracker/headtracker');
      $xport = json_decode($localdevprop['xport']);
      foreach( $xport as $xpsrc=>$xpdest ){
        $conn = $session->appendChild($doc->createElement('connect'));
        $conn->setAttribute('src',$xpsrc);
        $conn->setAttribute('dest',$xpdest);
      }
      $rec->setAttribute('gain',$localdevprop['mastergain']);
      $success = true;
    }
  }
  if( !$success ){
    // add announcement source
    $source = $doc->createElement('source');
    $scene->appendChild($source);
    $source->setAttribute('name','announce');
    $egosound = xml_add_sound($source, $doc, array('x'=>4) );
    $plugs = $doc->createElement('plugins');
    $egosound->appendChild($plugs);
    $sndfile = $doc->createElement('sndfile');
    $plugs->appendChild($sndfile);
    $sndfile->setAttribute('name','announce.flac');
    $sndfile->setAttribute('level','57');
    $sndfile->setAttribute('transport','false');
    $sndfile->setAttribute('loop','0');
  }
  $tsc = $doc->saveXML();
  $hash = hash('md5',$tsc);
  if( $hash != $devhash ){
    echo $hash . "\n";
    echo $tsc;
  }
}

function html_device_selector( $user, $currentdevice )
{
  $devs = list_unclaimed_devices();
  $doc = new DOMDocument('1.0');
  $root = $doc->appendChild($doc->createElement('div'));
  $div = $root->appendChild($doc->createElement('div'));
  $select = $div->appendChild($doc->createElement('select'));
  $select->setAttribute('class','uibutton');
  $select->setAttribute('oninput','location.href=\'?devselect=\'+event.target.value;');
  $select->appendChild($doc->createElement('option'))->appendChild($doc->createTextNode(' -- please select a device -- '));
  foreach( owned_devices( $user ) as $dev=>$dprop ){
    $opt = $select->appendChild($doc->createElement('option'));
    $opt->setAttribute('value',$dev);
    if( $currentdevice == $dev )
      $opt->setAttribute('selected','');
    $active = '';
    if( $dprop['age'] < 20 )
      $active = ' *active*';
    $opt->appendChild($doc->createTextNode($dev.' ('.$dprop['label'].')'.$active));
  }
  $inp = $div->appendChild($doc->createElement('input'));
  $inp->setAttribute('type','button');
  $inp->setAttribute('class','roomsettingstoggle uibutton');
  $inp->setAttribute('onclick','toggledisplay(\'devsettings\',\'device settings\');');
  $inp->setAttribute('value','show device settings');
  echo $doc->saveHTML();
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

function list_users()
{
  $users = array();
  $fname = '../ovboxusers';
  if( file_exists( $fname ) ){
    $fcontent = file_get_contents( $fname );
    $lines = explode("\n",$fcontent);
    foreach( $lines as $line ){
      $linet = explode("#",trim($line));
      $line = $linet[0];
      if( !empty($line) ){
        $linet = explode(":",trim($line));
        if( !empty($linet[0]) ){
          if( ($linet[0]!='room') && ($linet[0]!='device') && ($linet[0]!='admin'))
            $users[] = $linet[0];
        }
      }
    }
  }
  sort($users);
  return $users;
}

function list_groups( $user = "" )
{
  $groups = array();
  $fname = '../db/groups';
  if( file_exists( $fname ) ){
    $fgroups = json_decode( file_get_contents( $fname ), true );
    if( empty($user) ){
      $groups = array_keys($fgroups);
    }else{
      foreach( $fgroups as $group=>$users ){
        if( in_array( $user, $users ) ){
          $groups[] = $group;
        }
      }
    }
  }
  sort($groups);
  return $groups;
}

function add_group( $group )
{
  if( !empty($group) ){
    $fname = '../db/groups';
    $groups = array();
    if( file_exists( $fname ) )
      $groups = json_decode( file_get_contents( $fname ), true );
    if( !array_key_exists( $group, $groups ) ){
      $groups[$group] = array();
      file_put_contents( $fname, json_encode( $groups ));
    }
  }
}

function rm_group( $group )
{
  if( !empty($group) ){
    $fname = '../db/groups';
    $groups = array();
    if( file_exists( $fname ) )
      $groups = json_decode( file_get_contents( $fname ), true );
    if( array_key_exists( $group, $groups ) ){
      unset($groups[$group]);
      file_put_contents( $fname, json_encode( $groups ));
    }
  }
}

function add_user_to_group( $user, $group )
{
  if( !(empty($group)||empty($user)) ){
    $fname = '../db/groups';
    if( file_exists( $fname ) ){
      $groups = json_decode( file_get_contents( $fname ), true );
      if( array_key_exists( $group, $groups ) ){
        $users = $groups[$group];
        if( !in_array( $user, $users ) ){
          $users[] = $user;
          $groups[$group] = $users;
          file_put_contents( $fname, json_encode( $groups ));
        }
      }
    }
  }
}

function remove_user_from_group( $user, $group )
{
  if( !(empty($group)||empty($user)) ){
    $fname = '../db/groups';
    if( file_exists( $fname ) ){
      $groups = json_decode( file_get_contents( $fname ), true );
      if( array_key_exists( $group, $groups ) ){
        $users = $groups[$group];
        $us = array_search( $user, $users );
        if( $us ){
          unset($users[$us]);
          $groups[$group] = $users;
          file_put_contents( $fname, json_encode( $groups ));
        }
      }
    }
  }
}

function get_group_users( $group )
{
  $users = array();
  if( !empty($group) ){
    $fname = '../db/groups';
    if( file_exists( $fname ) ){
      $groups = json_decode( file_get_contents( $fname ), true );
      if( array_key_exists( $group, $groups ) )
        $users = $groups[$group];
    }
  }
  return $users;
}

function html_show_room( $room, $device, $deviceprop, $inroom, $user, $userprop, $usergroups )
{
  $rprop = get_room_prop( $room );
  if( ($rprop['age'] < 3600) &&
  ((!$rprop['private']) || ($rprop['owner']==$user)) &&
  ($userprop['seesall'] || ($rprop['owner']==$user) || in_array($rprop['group'],$usergroups)) &&
  (empty($rprop['group']) || (in_array($rprop['group'],$usergroups)))
  ) {
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
    $span->appendChild($doc->createTextNode($rprop['label'].' '));
    $span = $doc->createElement('span');
    $tit->appendChild($span);
    $span->setAttribute('class','rdesc');
    if( $deviceprop['reverb'] ){
      $span->appendChild($doc->createTextNode('('.$rprop['sx'].' x '.$rprop['sy'].' x '.$rprop['sz'].' m'));
      $sup = $doc->createElement('sup');
      $span->appendChild($sup);
      $sup->appendChild($doc->createTextNode('3'));
      $span->appendChild($doc->createTextNode(', T'));
      $sup = $doc->createElement('sub');
      $span->appendChild($sup);
      $sup->appendChild($doc->createTextNode('60'));
      $span->appendChild($doc->createTextNode(': '.sprintf("%1.2f",$rprop['t60']).' s)'));
    }
    if( empty($rprop['group']) )
      $span->appendChild($doc->createTextNode(' public'));
    else
      $span->appendChild($doc->createTextNode(' group \''.$rprop['group'].'\''));
    if( !empty($rprop['owner']) )
      $span->appendChild($doc->createTextNode(', managed by '.$rprop['owner']));
    $memb = $root->appendChild($doc->createElement('div'));
    $memb->setAttribute('class','rmembers');
    $senders = $memb->appendChild($doc->createElement('span'));
    $senders->setAttribute('class','roomsender');
    $listeners = $memb->appendChild($doc->createElement('span'));
    $listeners->setAttribute('class','roomlistener');
    $roomdev = get_devices_in_room( $room );
    if( $rprop['lock'] && empty($roomdev) ){
      modify_room_prop( $room, 'lock', false );
      $rprop['lock'] = false;
    }
    ksort($roomdev);
    $mychair = -1;
    foreach( $roomdev as $chair => $dev ){
      if( $dev == $device )
        $mychair = $chair;
    }
    $amsender = issender($deviceprop);
    foreach( $roomdev as $chair => $dev ){
      $rdevprop = get_properties($dev, 'device');
      $issender = issender($rdevprop);
      $latdisp = '';
      if( $deviceprop['peer2peer'] && $rdevprop['peer2peer'] ){
        $latkey = $chair.'-'.$mychair;
        if( array_key_exists($latkey,$rprop['lat']) ){
          // only display values from last 15 minutes:
          if( time()-$rprop['lat'][$latkey]['access'] < 900 ){
            $latdisp = sprintf(' %1.1fms ',
            0.5*$rprop['lat'][$latkey]['lat']+$rdevprop['jittersend']+$deviceprop['jitterreceive']+10);
          }
        }
      }else{
        $latkey1 = $chair.'-200';
        $latkey2 = $mychair.'-200';
        if( array_key_exists($chair.'-200',$rprop['lat']) && array_key_exists($mychair.'-200',$rprop['lat']) && ($dev != $device)){
          // only display values from last 15 minutes:
          if( time()-$rprop['lat'][$latkey1]['access'] < 900 ){
            $latdisp = sprintf(' %1.1fms ',
            0.5*($rprop['lat'][$latkey1]['lat']+$rprop['lat'][$latkey2]['lat'])+$rdevprop['jittersend']+$deviceprop['jitterreceive']+10);
          }
        }
      }
      $lab = $rdevprop['label'];
      $tagsuffix = 'member';
      if( !$issender )
        $tagsuffix = 'listener';
      $bclass = 'psv';
      if( $rdevprop['age'] < 20 )
        $bclass = 'act';
      $bclass = $bclass . $tagsuffix;
      $mtype = 'span';
      if( ($dev != $device) && $inroom && $issender && $amsender )
        $mtype = 'a';
      $mem = $doc->createElement('span');
      if( $issender ){
        $senders->appendChild($mem);
        $senders->appendChild($doc->createTextNode(' '));
      }else{
        $listeners->appendChild($mem);
        $listeners->appendChild($doc->createTextNode(' '));
      }
      $mem->setAttribute('class',$bclass);
      if ( $dev == $device ){
        if( $amsender )
          $mem->setAttribute('style','border: 3px solid #000000;');
        else
          $mem->setAttribute('style','border: 2px solid #606060;');
      }
      $memlink = $mem->appendChild($doc->createElement($mtype));
      $memlink->setAttribute('class',$bclass);
      if( $mtype == 'a')
        $memlink->setAttribute('href','?swapdev='.urlencode($dev));
      if( empty($lab) )
        $lab = $dev;
      $memlink->appendChild($doc->createTextNode( htmlspecialchars($lab) ));
      if( !empty($latdisp) ){
        $span = $mem->appendChild($doc->createElement('span'));
        $span->setAttribute('class','latency');
        $span->appendChild($doc->createTextNode($latdisp));
      }
      if( ($rprop['owner'] == $user) || ($rdevprop['owner'] == $user) ){
        $kick = $mem->appendChild($doc->createElement('input'));
        $kick->setAttribute('value','X');
        $kick->setAttribute('class','kick');
        $kick->setAttribute('type','button');
        $kick->setAttribute('title','Kick this device out of my room.');
        $kick->setAttribute('onclick','location.href=\'?kick='.urlencode($dev).'\';');
      }
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
        $a->appendChild($doc->createTextNode('kick all'));
      }
      $tog = $doc->createElement('input');
      $ctl->appendChild($tog);
      $tog->setAttribute('type','button');
      $tog->setAttribute('class','roomsettingstoggle uibutton');
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
      $inp->setAttribute('value',$rprop['label']);
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
      // group:
      $inp = $form->appendChild($doc->createElement('label'));
      $inp->appendChild($doc->createTextNode('Group: '));
      $inp = $form->appendChild($doc->createElement('select'));
      $inp->setAttribute('name','group');
      $opt = $inp->appendChild($doc->createElement('option'));
      $opt->setAttribute('value','');
      $opt->appendChild($doc->createTextNode(' - public - '));
      $usgroups = list_groups($user);
      foreach($usgroups as $grp){
        $opt = $inp->appendChild($doc->createElement('option'));
        $opt->setAttribute('value',$grp);
        $opt->appendChild($doc->createTextNode($grp));
        if( $rprop['group'] == $grp )
          $opt->setAttribute('selected','');
      }
      // private room:
      $inp = $doc->createElement('input');
      $form->appendChild($inp);
      $inp->setAttribute('type','checkbox');
      $inp->setAttribute('id','private');
      $inp->setAttribute('name','private');
      if( $rprop['private'] )
        $inp->setAttribute('checked','');
      $inp = $doc->createElement('label');
      $form->appendChild($inp);
      $inp->setAttribute('for','private');
      $inp->appendChild($doc->createTextNode('Private room - visible only to me (overrides group)'));
      $form->appendChild($doc->createElement('br'));
      // save button:
      $inp = $doc->createElement('button');
      $form->appendChild($inp);
      $inp->appendChild($doc->createTextNode('Save'));
    }
    echo $doc->saveHTML();
  }
}

function html_admin_db($type)
{
  $devs = list_db($type);
  $doc = new DOMDocument('1.0');
  $root = $doc->appendChild($doc->createElement('div'));
  $root->appendChild($doc->createElement('a'))->setAttribute('name',$type.'s');
  $h = $root->appendChild($doc->createElement('h2'));
  $h->appendChild($doc->createTextNode($type.'s: '));
  $s = $h->appendChild($doc->createElement('span'));
  $s->setAttribute('style','font-size: small;');
  $s->appendChild($doc->createTextNode('('.count($devs).')'));
  $tab = $root->appendChild($doc->createElement('table'));
  $tr = $tab->appendChild($doc->createElement('tr'));
  $tr->appendChild($doc->createElement('th'))->appendChild($doc->createTextNode($type));
  $tr->appendChild($doc->createElement('th'))->appendChild($doc->createTextNode('age'));
  $tr->appendChild($doc->createElement('th'))->appendChild($doc->createTextNode('owner'));
  $tr->appendChild($doc->createElement('th'))->appendChild($doc->createTextNode('label'));
  foreach( $devs as $dev=>$dprop ){
    $tr = $tab->appendChild($doc->createElement('tr'));
    $tr->appendChild($doc->createElement('td'))->appendChild($doc->createTextNode($dev));
    $age = $dprop['age'].'s';
    if( $dprop['age'] > 599 )
      $age = sprintf('%1.1fm',$dprop['age']/60);
    if( $dprop['age'] > 6*3600 )
      $age = sprintf('%1.1fh',$dprop['age']/3600);
    if( $dprop['age'] > 3*24*3600 )
      $age = sprintf('%1.1fd',$dprop['age']/(3600*24));

    $tr->appendChild($doc->createElement('td'))->appendChild($doc->createTextNode($age));
    // user:
    $td = $tr->appendChild($doc->createElement('td'));
    $inp = $td->appendChild($doc->createElement('select'));
    $inp->setAttribute('oninput','location.href=\'?set'.$type.'owner='.urlencode($dev).'&owner=\'+event.target.value;');
    $opt = $inp->appendChild($doc->createElement('option'));
    $opt->setAttribute('value','');
    $opt->appendChild($doc->createTextNode(' -- unclaimed -- '));
    if( $dprop['owner'] == '' )
      $opt->setAttribute('selected','');
    $users = list_users();
    foreach($users as $us){
      $opt = $inp->appendChild($doc->createElement('option'));
      $opt->setAttribute('value',$us);
      $opt->appendChild($doc->createTextNode($us));
      if( $dprop['owner'] == $us )
        $opt->setAttribute('selected','');
    }
    //label:
    $td = $tr->appendChild($doc->createElement('td'));
    $form = $td->appendChild($doc->createElement('form'));
    $inp = $form->appendChild($doc->createElement('input'));
    $inp->setAttribute('type','hidden');
    $inp->setAttribute('name','set'.$type.'label');
    $inp->setAttribute('value',$dev);
    $inp = $form->appendChild($doc->createElement('input'));
    $inp->setAttribute('type','text');
    $inp->setAttribute('name','label');
    $inp->setAttribute('pattern','[a-zA-Z0-9]*');
    $inp->setAttribute('value',$dprop['label']);
    $form->appendChild($doc->createElement('button'))->appendChild($doc->createTextNode('Save'));
    // delete
    $td = $tr->appendChild($doc->createElement('td'));
    $form = $td->appendChild($doc->createElement('form'));
    $inp = $form->appendChild($doc->createElement('input'));
    $inp->setAttribute('type','checkbox');
    $inp->setAttribute('name','rm'.$type);
    $inp->setAttribute('value',$dev);
    $form->appendChild($doc->createElement('button'))->appendChild($doc->createTextNode('Delete'));
  }
  echo $doc->saveHTML() . "\n";
}

function html_admin_users()
{
  $users = list_users();
  $doc = new DOMDocument('1.0');
  $root = $doc->createElement('div');
  $doc->appendChild($root);
  $root->appendChild($doc->createElement('a'))->setAttribute('name','users');
  $el = $root->appendChild($doc->createElement('h2'));
  $el->appendChild($doc->createTextNode('users: '));
  $s = $el->appendChild($doc->createElement('span'));
  $s->setAttribute('style','font-size: small;');
  $s->appendChild($doc->createTextNode('('.count($users).')'));
  $tab = $root->appendChild($doc->createElement('table'));
  $tr = $tab->appendChild($doc->createElement('tr'));
  $th = $tr->appendChild($doc->createElement('th'));
  $th->appendChild($doc->createTextNode('user'));
  $th = $tr->appendChild($doc->createElement('th'));
  $th->appendChild($doc->createTextNode('settings'));
  $tab->appendChild($doc->createTextNode("\n"));
  foreach($users as $us){
    $uprop = get_properties($us,'user');
    $tr = $tab->appendChild($doc->createElement('tr'));
    $td = $tr->appendChild($doc->createElement('td'));
    $td->appendChild($doc->createTextNode($us));
    $td = $tr->appendChild($doc->createElement('td'));
    $form = $td->appendChild($doc->createElement('form'));
    $inp = $form->appendChild($doc->createElement('input'));
    $inp->setAttribute('type','hidden');
    $inp->setAttribute('name','moduser');
    $inp->setAttribute('value',$us);
    $inp = $form->appendChild($doc->createElement('input'));
    $inp->setAttribute('title','If checked, the user can see all public rooms, otherwise only the owned rooms.');
    $inp->setAttribute('type','checkbox');
    $inp->setAttribute('name','seesall');
    $inp->setAttribute('id','seesall');
    if( $uprop['seesall'] )
      $inp->setAttribute('checked','');
    $inp = $form->appendChild($doc->createElement('label'));
    $inp->setAttribute('for','seesall');
    $inp->appendChild($doc->createTextNode(' sees public rooms '));

    $inp = $form->appendChild($doc->createElement('select'));
    $inp->setAttribute('name','maingroup');
    $opt = $inp->appendChild($doc->createElement('option'));
    $opt->setAttribute('value','');
    $opt->appendChild($doc->createTextNode(' -- no main group -- '));
    $usgroups = list_groups($us);
    foreach($usgroups as $grp){
      $opt = $inp->appendChild($doc->createElement('option'));
      $opt->setAttribute('value',$grp);
      $opt->appendChild($doc->createTextNode($grp));
      if( $uprop['maingroup'] == $grp )
        $opt->setAttribute('selected','');
    }

    $inp = $form->appendChild($doc->createElement('button'));
    $inp->appendChild($doc->createTextNode('Save'));
    $tab->appendChild($doc->createTextNode("\n"));
  }
  echo $doc->saveHTML() . "\n";
}

function html_admin_groups()
{
  $users = list_users();
  $grps = list_groups();
  $styles = list_styles();
  $doc = new DOMDocument('1.0');
  $root = $doc->createElement('div');
  $doc->appendChild($root);
  $root->appendChild($doc->createElement('a'))->setAttribute('name','groups');
  $el = $root->appendChild($doc->createElement('h2'));
  $el->appendChild($doc->createTextNode('groups: '));
  $s = $el->appendChild($doc->createElement('span'));
  $s->setAttribute('style','font-size: small;');
  $s->appendChild($doc->createTextNode('('.count($grps).')'));
  $form = $root->appendChild($doc->createElement('form'));
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('name','addgroup');
  $el->setAttribute('type','text');
  $el->setAttribute('pattern','[a-zA-Z0-9]*');
  $el = $form->appendChild($doc->createElement('button'));
  $el->appendChild($doc->createTextNode('add group'));
  $tab = $root->appendChild($doc->createElement('table'));
  $tr = $tab->appendChild($doc->createElement('tr'));
  $th = $tr->appendChild($doc->createElement('th'));
  $th->appendChild($doc->createTextNode('group'));
  $th = $tr->appendChild($doc->createElement('th'));
  $th->appendChild($doc->createTextNode('users'));
  $th = $tr->appendChild($doc->createElement('th'));
  $th->appendChild($doc->createTextNode('style'));
  $tab->appendChild($doc->createTextNode("\n"));
  foreach($grps as $group){
    $gprop = get_properties( $group, 'group' );
    $grpusers = get_group_users( $group );
    sort($grpusers);
    $tr = $tab->appendChild($doc->createElement('tr'));
    $td = $tr->appendChild($doc->createElement('td'));
    $td->appendChild($doc->createTextNode($group));
    $td = $tr->appendChild($doc->createElement('td'));
    $td->setAttribute('style','line-height: 2.1em;');
    foreach( $grpusers as $us ){
      $sp = $td->appendChild($doc->createElement('span'));
      $sp->setAttribute('class','psvmember');
      $sp->appendChild($doc->createTextNode($us));
      $form = $sp->appendChild($doc->createElement('form'));
      $form->setAttribute('style','display: inline;');
      $inp = $form->appendChild($doc->createElement('input'));
      $inp->setAttribute('type','hidden');
      $inp->setAttribute('name','removeuserfromgroup');
      $inp->setAttribute('value',$group);
      $inp = $form->appendChild($doc->createElement('input'));
      $inp->setAttribute('type','hidden');
      $inp->setAttribute('name','groupuser');
      $inp->setAttribute('value',$us);
      $inp = $form->appendChild($doc->createElement('button'));
      $inp->setAttribute('class','kick');
      $inp->appendChild($doc->createTextNode('X'));
      $td->appendChild($doc->createTextNode(' '));
    }
    $inp = $td->appendChild($doc->createElement('select'));
    $inp->setAttribute('oninput','location.href=\'?addusertogroup='.urlencode($group).'&newuser=\'+event.target.value;');
    $opt = $inp->appendChild($doc->createElement('option'));
    $opt->setAttribute('value','');
    $opt->appendChild($doc->createTextNode(' -- select a user to add -- '));
    foreach($users as $us){
      if( !in_array($us,$grpusers) ){
        $opt = $inp->appendChild($doc->createElement('option'));
        $opt->setAttribute('value',$us);
        $opt->appendChild($doc->createTextNode($us));
      }
    }

    $td = $tr->appendChild($doc->createElement('td'));
    $inp = $td->appendChild($doc->createElement('select'));
    $inp->setAttribute('oninput','location.href=\'?setgrpstyle='.urlencode($group).'&grpstyle=\'+event.target.value;');
    //$inp->setAttribute('name','css');
    $opt = $inp->appendChild($doc->createElement('option'));
    $opt->setAttribute('value','');
    $opt->appendChild($doc->createTextNode(' -- default style -- '));
    $usgroups = list_groups($us);
    foreach($styles as $style){
      $opt = $inp->appendChild($doc->createElement('option'));
      $opt->setAttribute('value',$style);
      $opt->appendChild($doc->createTextNode($style));
      if( $gprop['style'] == $style )
        $opt->setAttribute('selected','');
    }

    $td = $tr->appendChild($doc->createElement('td'));
    $form = $td->appendChild($doc->createElement('form'));
    $inp = $form->appendChild($doc->createElement('input'));
    $inp->setAttribute('type','hidden');
    $inp->setAttribute('name','rmgroup');
    $inp->setAttribute('value',$group);
    $inp = $form->appendChild($doc->createElement('button'));
    $inp->appendChild($doc->createTextNode('delete group'));
    $tab->appendChild($doc->createTextNode("\n"));
  }
  echo $doc->saveHTML() . "\n";
}

function html_show_user( $user, $device, $devprop )
{
  $doc = new DOMDocument('1.0');
  $root = $doc->appendChild($doc->createElement('div'));
  $p = $root->appendChild($doc->createElement('p'));
  $p->appendChild($doc->createTextNode('You are logged in as user '));
  $p->appendChild($doc->createElement('b'))->appendChild($doc->createTextNode($user));
  $p->appendChild($doc->createTextNode(' with device '));
  $p->appendChild($doc->createElement('b'))->appendChild($doc->createTextNode($device.' ('.$devprop['label'].')' ));
  $p->appendChild($doc->createTextNode('.'));
  $inf = $root->appendChild($doc->createElement('div'));
  $inf->setAttribute('id','groupinfo');
  $inf->setAttribute('class','devprop');
  $inf->setAttribute('style','display: none;');
  $el = $inf->appendChild($doc->createElement('div'));
  $el->setAttribute('class','devproptitle');
  $el->appendChild($doc->createTextNode('You are a member of these groups:'));
  foreach( list_groups($user) as $grp ){
    $p1 = $inf->appendChild($doc->createElement('div'));
    $p1->appendChild($doc->createElement('b'))->appendChild($doc->createTextNode($grp.': '));
    foreach( get_group_users( $grp ) as $us ){
      $p1->appendChild($doc->createTextNode($us.' '));
    }
  }

  $inp = $p->appendChild($doc->createElement('input'));
  $inp->setAttribute('type','button');
  $inp->setAttribute('class','roomsettingstoggle uibutton');
  $inp->setAttribute('onclick','toggledisplay(\'groupinfo\',\'group info\');');
  $inp->setAttribute('value','show group info');
  echo $doc->saveHTML() . "\n";
}

function add_expert_div( &$el, &$doc )
{
  $div = $el->appendChild( $doc->createElement('div') );
  $div->setAttribute('id','expert');
  $div->setAttribute('class','expert');
  $div->setAttribute('style','display: none;');
  return $div;
}

function html_show_device( $user, $device, $devprop )
{
  html_device_selector( $user, $device );
  $doc = new DOMDocument('1.0');
  $root = $doc->appendChild($doc->createElement('div'));
  $form = $root->appendChild($doc->createElement('form'));
  $form->setAttribute('class','devprop');
  $form->setAttribute('id','devsettings');
  $form->setAttribute('style','display: none;');
  $inp = $form->appendChild($doc->createElement('input'));
  $inp->setAttribute('type','button');
  $inp->setAttribute('class','roomsettingstoggle');
  $inp->setAttribute('onclick','toggledisplayclass(\'expert\',\'expert settings\');');
  $inp->setAttribute('value','show expert settings');
  $el = $form->appendChild($doc->createElement('div'));
  $el->setAttribute('class','devproptitle');
  $el->appendChild($doc->createTextNode('Device settings:'));
  // device properties:
  // label:
  $el = $form->appendChild($doc->createElement('label'));
  $el->setAttribute('for','label');
  $el->appendChild($doc->createTextNode('device label (appears in rooms and the mixer of the others): '));
  $form->appendChild($doc->createElement('br'));
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('id','label');
  $el->setAttribute('name','label');
  $el->setAttribute('type','text');
  $el->setAttribute('pattern','[a-zA-Z0-9]*');
  $el->setAttribute('value',$devprop['label']);
  $form->appendChild($doc->createElement('br'));
  $form->appendChild($doc->createElement('hr'));
  $el = $form->appendChild($doc->createElement('div'));
  $el->setAttribute('class','devproptitle');
  $el->appendChild($doc->createTextNode('Audio settings:'));
  // input port:
  $el = $form->appendChild($doc->createElement('label'));
  $el->setAttribute('for','inputport');
  $el->appendChild($doc->createTextNode('input ports (to which your microphones/instruments are connected): '));
  $form->appendChild($doc->createElement('br'));
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('id','inputport');
  $el->setAttribute('name','inputport');
  $el->setAttribute('type','text');
  $el->setAttribute('value',$devprop['inputport']);
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('id','inputport2');
  $el->setAttribute('name','inputport2');
  $el->setAttribute('type','text');
  $el->setAttribute('value',$devprop['inputport2']);
  $form->appendChild($doc->createElement('br'));
  // output port:
  $div = add_expert_div($form,$doc);
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','outputport');
  $el->appendChild($doc->createTextNode('output ports (to which your headphones are connected): '));
  $div->appendChild($doc->createElement('br'));
  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('id','outputport1');
  $el->setAttribute('name','outputport1');
  $el->setAttribute('type','text');
  $el->setAttribute('value',$devprop['outputport1']);
  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('id','outputport2');
  $el->setAttribute('name','outputport2');
  $el->setAttribute('type','text');
  $el->setAttribute('value',$devprop['outputport2']);
  $div->appendChild($doc->createElement('br'));
  // extra ports:
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','xport');
  $el->appendChild($doc->createTextNode('extra ports (json expression, e.g., {"Giso:out_1":"ardour:Giso/in"}): '));
  $div->appendChild($doc->createElement('br'));
  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('size','40');
  $el->setAttribute('id','xport');
  $el->setAttribute('name','xport');
  $el->setAttribute('type','text');
  $el->setAttribute('value',$devprop['xport']);
  $div->appendChild($doc->createElement('br'));
  // receiver type:
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','rectype');
  $el->appendChild($doc->createTextNode('receiver type: '));
  $el = $div->appendChild($doc->createElement('select'));
  $el->setAttribute('id','rectype');
  $el->setAttribute('name','rectype');
  foreach( array('ortf','hrtf') as $rectype ){
    $opt = $el->appendChild($doc->createElement('option'));
    $opt->setAttribute('value',$rectype);
    if( $devprop['rectype'] == $rectype )
      $opt->setAttribute('selected','');
    $opt->appendChild($doc->createTextNode($rectype));
  }
  $div->appendChild($doc->createElement('br'));
  // source distance:
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','srcdist');
  $el->appendChild($doc->createTextNode('distance between sources (in case of you send more than one channel): '));
  $div->appendChild($doc->createElement('br'));
  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('id','srcdist');
  $el->setAttribute('name','srcdist');
  $el->setAttribute('type','number');
  $el->setAttribute('min','0');
  $el->setAttribute('step','0.01');
  $el->setAttribute('value',$devprop['srcdist']);
  $div->appendChild($doc->createElement('br'));
  // playback gain:
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','playbackgain');
  $el->appendChild($doc->createTextNode('playback gain in dB (equivalent to changing the input gain): '));
  $div->appendChild($doc->createElement('br'));
  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('id','playbackgain');
  $el->setAttribute('name','playbackgain');
  $el->setAttribute('type','number');
  $el->setAttribute('min','-20');
  $el->setAttribute('max','20');
  $el->setAttribute('step','0.1');
  $el->setAttribute('value',$devprop['playbackgain']);
  $div->appendChild($doc->createElement('br'));
  // master gain:
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','playbackgain');
  $el->appendChild($doc->createTextNode('master gain in dB (equivalent to changing the headphone gain): '));
  $div->appendChild($doc->createElement('br'));
  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('id','mastergain');
  $el->setAttribute('name','mastergain');
  $el->setAttribute('type','number');
  $el->setAttribute('min','-20');
  $el->setAttribute('max','20');
  $el->setAttribute('step','0.1');
  $el->setAttribute('value',$devprop['mastergain']);
  $div->appendChild($doc->createElement('br'));
  // ego monitor:
  $el = $form->appendChild($doc->createElement('label'));
  $el->setAttribute('for','egogain');
  $el->appendChild($doc->createTextNode('ego monitor gain in dB: '));
  $form->appendChild($doc->createElement('br'));
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('id','egogain');
  $el->setAttribute('name','egogain');
  $el->setAttribute('type','number');
  $el->setAttribute('min','-30');
  $el->setAttribute('max','10');
  $el->setAttribute('step','0.1');
  $el->setAttribute('value',$devprop['egogain']);
  $form->appendChild($doc->createElement('br'));
  // reverb:
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('name','reverb');
  $el->setAttribute('id','reverb');
  $el->setAttribute('type','checkbox');
  if( $devprop['reverb'] )
    $el->setAttribute('checked','');
  $el = $form->appendChild($doc->createElement('label'));
  $el->setAttribute('for','reverb');
  $el->appendChild($doc->createTextNode('render reverb '));
  $form->appendChild($doc->createElement('br'));
  // reverb gain:
  $div = add_expert_div($form,$doc);
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','devrvbgain');
  $el->appendChild($doc->createTextNode('extra reverb gain in dB: '));
  $div->appendChild($doc->createElement('br'));
  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('id','devrvbgain');
  $el->setAttribute('name','rvbgain');
  $el->setAttribute('type','number');
  $el->setAttribute('min','-20');
  $el->setAttribute('max','20');
  $el->setAttribute('step','0.1');
  $el->setAttribute('value',$devprop['rvbgain']);
  $div->appendChild($doc->createElement('br'));
  // raw mode:

  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('name','rawmode');
  $el->setAttribute('id','rawmode');
  $el->setAttribute('type','checkbox');
  if( $devprop['rawmode'] )
    $el->setAttribute('checked','');
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','rawmode');
  $el->appendChild($doc->createTextNode('raw mode - no virtual acoustics '));
  $form->appendChild($doc->createElement('hr'));
  $el = $form->appendChild($doc->createElement('div'));
  $el->setAttribute('class','devproptitle');
  $el->appendChild($doc->createTextNode('Network settings:'));
  // jitter (send):
  $el = $form->appendChild($doc->createElement('label'));
  $el->setAttribute('for','jittersend');
  $el->appendChild($doc->createTextNode('sender jitter (affects buffer length of others): '));
  $form->appendChild($doc->createElement('br'));
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('id','jittersend');
  $el->setAttribute('name','jittersend');
  $el->setAttribute('type','number');
  $el->setAttribute('min','2');
  $el->setAttribute('max','50');
  $el->setAttribute('step','1');
  $el->setAttribute('value',$devprop['jittersend']);
  $form->appendChild($doc->createElement('br'));
  // jitter (receive):
  $el = $form->appendChild($doc->createElement('label'));
  $el->setAttribute('for','jitterreceive');
  $el->appendChild($doc->createTextNode('receiver jitter (affects your own buffer length): '));
  $form->appendChild($doc->createElement('br'));
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('id','jitterreceive');
  $el->setAttribute('name','jitterreceive');
  $el->setAttribute('type','number');
  $el->setAttribute('min','2');
  $el->setAttribute('max','50');
  $el->setAttribute('step','1');
  $el->setAttribute('value',$devprop['jitterreceive']);
  $form->appendChild($doc->createElement('br'));
  // peer-to-peer:
  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('name','peer2peer');
  $el->setAttribute('id','peer2peer');
  $el->setAttribute('type','checkbox');
  if( $devprop['peer2peer'] )
    $el->setAttribute('checked','');
  $el = $form->appendChild($doc->createElement('label'));
  $el->setAttribute('for','peer2peer');
  $el->appendChild($doc->createTextNode('peer-to-peer mode '));
  $form->appendChild($doc->createElement('br'));
  // do not send:
  //$div = add_expert_div($form, $doc );
  //$el = $div->appendChild($doc->createElement('input'));
  //$el->setAttribute('name','donotsend');
  //$el->setAttribute('id','donotsend');
  //$el->setAttribute('type','checkbox');
  //if( $devprop['donotsend'] )
  //  $el->setAttribute('checked','');
  //$el = $div->appendChild($doc->createElement('label'));
  //$el->setAttribute('for','donotsend');
  //$el->appendChild($doc->createTextNode('alternative data source, do not send to this device '));
  //$div->appendChild($doc->createElement('br'));
  // extra destinations:
  $div = add_expert_div($form, $doc );
  $el = $div->appendChild($doc->createElement('label'));
  $el->setAttribute('for','secrec');
  $el->appendChild($doc->createTextNode('additional local receiver delay for secondary receiver (0 for no secondary receiver): '));
  $div->appendChild($doc->createElement('br'));
  $el = $div->appendChild($doc->createElement('input'));
  $el->setAttribute('id','secrec');
  $el->setAttribute('name','secrec');
  $el->setAttribute('type','number');
  $el->setAttribute('min','0');
  $el->setAttribute('max','100');
  $el->setAttribute('step','1');
  $el->setAttribute('value',$devprop['secrec']);
  // save:
  $form->appendChild($doc->createElement('button'))->appendChild($doc->createTextNode('Save'));;
  $inp = $form->appendChild($doc->createElement('input'));
  $inp->setAttribute('type','button');
  $inp->setAttribute('class','roomsettingstoggle');
  $inp->setAttribute('onclick','location.href=\'?unclaim\';');
  $inp->setAttribute('value','not my device');
  $inp = $form->appendChild($doc->createElement('input'));
  $inp->setAttribute('type','button');
  //$inp->setAttribute('class','roomsettingstoggle');
  $inp->setAttribute('onclick','location.href=\'?devreset\';');
  $inp->setAttribute('value','reset settings');


  $el = $form->appendChild($doc->createElement('input'));
  $el->setAttribute('name','setdevprop');
  $el->setAttribute('type','hidden');
  if( (!empty($devprop['host'])) && (!empty($devprop['room'])) && ($devprop['age']<20) && (!$devprop['rawmode']) ){
    // device is active and we know the host name:
    $div = $root->appendChild($doc->createElement('div'));
    $a = $div->appendChild($doc->createElement('a'));
    $a->setAttribute('href','http://'.$devprop['host'].':8080/');
    $a->setAttribute('target','_blank');
    $a->setAttribute('rel','noopener noreferrer');
    $a->setAttribute('class','mixer');
    $div->setAttribute('class','mixer');
    $a->appendChild($doc->createTextNode('open mixer'));
    $div->appendChild($doc->createTextNode(' (works only when your browser is in the same network as your device)'));
  }
  echo $doc->saveHTML() . "\n";
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * mode: php
 * End:
 */

?>