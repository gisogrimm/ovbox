<?php

include '../php/ovbox.inc';

$user = getenv('ovboxuser');

if (isset($_SERVER['REMOTE_USER']))
    $user = $_SERVER['REMOTE_USER'];

if( isset($_SERVER['REDIRECT_REMOTE_USER']))
    $user = $_SERVER['REDIRECT_REMOTE_USER'];

// require a valid user:
if( empty($user) )
    die();

// device update:
if ($user == 'device') {
    $device = '';
    if( isset($_GET['dev']) )
        $device = $_GET['dev'];
    if( !empty($device) ){
        $devhash = '';
        if( isset($_GET['hash']) )
            $devhash = $_GET['hash'];
        get_tascar_cfg( $device, $devhash );
        // touch device file:
        touch( '../db/' . $device . '.device' );
    }
    die();
}

// admin page:
if( $user == 'admin' ){
    if( isset($_GET['setdevowner']) )
        modify_device_prop( $_GET['setdevowner'], 'owner', $_GET['owner'] );
    if( isset($_GET['setdevlabel']) )
        modify_device_prop( $_GET['setdevlabel'], 'label', $_GET['label'] );
    if( isset($_GET['rmdevice']) )
        rm_device( $_GET['rmdevice'] );
    if( isset($_GET['setroomlabel']) )
        modify_room_prop( $_GET['setroomlabel'], 'name', $_GET['label'] );
    if( isset($_GET['setroomreverb']) ){
        modify_room_prop( $_GET['setroomreverb'], 'size', $_GET['size'] );
        modify_room_prop( $_GET['setroomreverb'], 'rvbgain', $_GET['rvbgain'] );
        modify_room_prop( $_GET['setroomreverb'], 'rvbdamp', $_GET['rvbdamp'] );
        modify_room_prop( $_GET['setroomreverb'], 'rvbabs', $_GET['rvbabs'] );
    }
    if( isset($_GET['rmroom']) )
        rm_room( $_GET['rmroom'] );
    print_head( $user );
    echo '<form><button>Refresh</button></form>';
    $devs = list_devices();
    echo "<h2>Devices:</h2><table>\n";
    echo "<tr><th>device</th><th>age</th><th>owner</th><th>label</th></tr>\n";
    foreach( $devs as $dev ){
        $dprop = get_device_prop( $dev['dev'] );
        echo '<tr><td>' .
            $dev['dev'] . '</td><td>' .
            $dev['age'] . '</td><td>' .
            '<form><input type="hidden" name="setdevowner" value="'.$dev['dev'].'"><input type="text" name="owner" pattern="[a-zA-Z0-9]*" value="'.$dprop['owner'].'"><button>Save</button></form>' .
            '</td><td>' .
            '<form><input type="hidden" name="setdevlabel" value="'.$dev['dev'].'"><input type="text" name="label" pattern="[a-zA-Z0-9]*" value="'.$dprop['label'].'"><button>Save</button></form>' .
            '</td><td>' .
            '<form><input type="hidden" name="rmdevice" value="'.$dev['dev'].'"><button>Delete</button></form>' .
            '</td></tr>' . "\n"; 
    }
    echo "</table>\n";
    $rooms = list_rooms();
    echo "<h2>Rooms:</h2><table>\n";
    echo "<tr><th>room</th><th>age</th><th>label</th><th>reverb (size/gain/damping/absorption)</th></tr>\n";
    foreach( $rooms as $room ){
        $dprop = get_room_prop( $room['room'] );
        echo '<tr><td>' .
            $room['room'] . '</td><td>' .
            $room['age'] . '</td><td>' .
            '<form><input type="hidden" name="setroomlabel" value="'.$room['room'].'"><input type="text" name="label" pattern="[a-zA-Z0-9]*" value="'.$dprop['name'].'"><button>Save</button></form>' .
            '</td><td>' .
            '<form>' .
            '<input type="hidden" name="setroomreverb" value="'.$room['room'].'">'.
            '<input type="text" title="Size x y z in m" name="size" pattern="[ 0-9\.]*" value="'.$dprop['size'].'">'.
            '<input type="number" tite="Reverb gain in dB" name="rvbgain" min="-20" max="0" step="0.1" value="'.$dprop['rvbgain'].'">'.
            '<input type="number" title="Damping low-pass coeff" name="rvbdamp" min="0" max="1" step="0.01" value="'.$dprop['rvbdamp'].'">'.
            '<input type="number" title="Wall absorption" name="rvbabs" min="0" max="1" step="0.01" value="'.$dprop['rvbabs'].'">'.
            '<button>Save</button>' .
            '</form>' .
            '</td><td>' .
            '<form><input type="hidden" name="rmroom" value="'.$room['room'].'"><button>Delete</button></form>' .
            '</td></tr>' . "\n"; 
    }
    echo "</table>\n";
    print_foot();
    die();
}

// room update:
if ($user == 'room') {
    if( isset($_GET['port']) && isset($_GET['name']) && isset($_GET['pin']) ) {
        update_room( $_SERVER['REMOTE_ADDR'], $_GET['port'], $_GET['name'], $_GET['pin'] );
    }
    die();
}

if( isset($_GET['devselect']) ){
    select_userdev( $user, $_GET['devselect'] );
}

$device = get_device( $user );

if( isset($_GET['enterroom']) ) {
    if( !empty( $device ) )
        device_enter_room( $device, $_GET['enterroom'] );
}

if( isset($_GET['swapdev']) ){
    if( !empty( $device ) ){
        room_swap_devices( $device, $_GET['swapdev'] );
    }
}

if( isset($_GET['lockroom']) ){
    if( !empty( $device ) ){
        lock_room( $_GET['lockroom'], $device, $_GET['lck'] );
    }
}

if( isset($_GET['setdevprop']) ){
    if( !empty( $device ) ){
        $prop = get_device_prop( $device );
        $prop['reverb'] = isset($_GET['reverb']);
        $prop['peer2peer'] = isset($_GET['peer2peer']);
        if( isset($_GET['jittersend']) )
            $prop['jittersend'] = $_GET['jittersend'];
        if( isset($_GET['jitterreceive']) )
            $prop['jitterreceive'] = $_GET['jitterreceive'];
        if( isset($_GET['label']) )
            $prop['label'] = $_GET['label'];
        set_device_prop( $device, $prop );
    }
}

print_head( $user );
    

if ( empty( $device ) ) {
    echo "<p>You are logged in as user {$user}. No device linked to this account.</p>";
    html_device_selector( $user, $device );
} else {
    echo "<p>You are logged in as user <b>{$user}</b> with device <b>{$device}</b>.</p>";
    html_device_selector( $user, $device );
    $devprop = get_device_prop( $device );
    echo '<form class="devprop"><div class="devproptitle">Device properties:</div>' . "\n";
    // device properties:
    echo '<label for="label">device label (appears in the mixer of the others): </label><br>';
    echo '<input id="label" name="label" type="text" value="'.$devprop['label'].'" pattern="[a-zA-Z0-9]*"><br>' . "\n";
    echo '<label for="jittersend">sender jitter (affects buffer length of others): </label><br>';
    echo '<input id="jittersend" name="jittersend" type="number" min="2" max="30" value="'.$devprop['jittersend'].'"><br>' . "\n";
    echo '<label for="jitterreceive">receiver jitter (affects your own buffer length): </label><br>';
    echo '<input id="jitterreceive" name="jitterreceive" type="number" min="2" max="30" value="'.$devprop['jitterreceive'].'"><br>' . "\n";
    echo '<input id="breverb" name="reverb" type="checkbox"';
    if( $devprop['reverb'] )
        echo ' checked';
    echo '><label for="breverb">render reverb</label><br>';
    echo '<input id="bp2p" name="peer2peer" type="checkbox"';
    if( $devprop['peer2peer'] )
        echo ' checked';
    echo '><label for="bp2p">peer-to-peer mode</label><br>';
    echo '<button>Save</button>'."\n";
    echo '<input type="hidden" name="setdevprop" value="">';
    // end device properties.
    echo '</form>';
    echo '<p>Rooms: (<a href="http://' . $_SERVER['HTTP_HOST'] . '">refresh</a>)</p>' . "\n";
    foreach( get_rooms() as $room){
        $rprop = get_room_prop( $room );
        if( $rprop['age'] < 3600 ) {
            $myroom = false;
            // only show active rooms
            if( $room == $devprop['room'] ) {
                $myroom = true;
                echo '<div class="myroom">' . "\n";
            } else {
                echo '<div class="room">' . "\n";
            }
            echo '<div><span class="rname">'.$rprop['name'].'</span> (A='.$rprop['area'].' m<sup>2</sup>, V='.$rprop['volume'].' m<sup>3</sup>, T60='.sprintf("%1.2f",$rprop['t60']).' s)</div>'."\n";
            echo '<div class="rmembers">';
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
                if ( $dev == $device ){
                    echo '<span class="'.$bclass.'" style="border: 2px solid #000000;">';
                }else{
                    if( $room == $devprop['room'] ) {
                        echo '<a href="?swapdev='.urlencode($dev).'" class="'.$bclass.'">';
                    }else{
                        echo '<span class="'.$bclass.'">';
                    }
                }
                //echo htmlspecialchars($devuser) . ' ' . $chair;
                if( empty($lab) )
                    $lab = $dev;
                echo htmlspecialchars($lab);
                if ( $dev == $device ){
                    echo '</span>';
                }else{
                    if( $room == $devprop['room'] ) {
                        echo '</a>';
                    }else{
                        echo "</span>";
                    }
                }
                echo ' ';
            }
            echo '</div>';
            //echo '<div class="rhost">'.$rprop['host'].':'.$rprop['port'].'</div>'."\n";
            if( $myroom ) {
                echo '<a href="?enterroom=">leave room</a> ';
                if( $rprop['lock'] ){
                    echo '<a href="?lockroom='.urlencode($room).'&lck=0">unlock room</a>';
                }else{
                    echo '<a href="?lockroom='.urlencode($room).'&lck=1">lock room</a>';
                }
            } else {
                if( $rprop['lock'] ){
                    echo 'room is locked.';
                }else{
                    echo '<a href="?enterroom='.urlencode($room).'">enter</a>';
                }
            }
            echo '</div>';
        }
    }
}
print_foot();

?>
