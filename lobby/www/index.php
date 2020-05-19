<?php

include '../php/ovbox.inc';

$user = '';

if (isset($_SERVER['REMOTE_USER']))
    $user = $_SERVER['REMOTE_USER'];

if( isset($_SERVER['REDIRECT_REMOTE_USER']))
    $user = $_SERVER['REDIRECT_REMOTE_USER'];

if ($user == 'device') {
    $device = '';
    if( isset($_GET['dev']) )
        $device = $_GET['dev'];
    $devhash = '';
    if( isset($_GET['hash']) )
        $devhash = $_GET['hash'];
    get_tascar_cfg( $device, $devhash );
    die();
}

if( empty($user) )
    die();

if ($user == 'room') {
    if( isset($_GET['port']) && isset($_GET['name']) && isset($_GET['pin']) ) {
        update_room( $_SERVER['REMOTE_ADDR'], $_GET['port'], $_GET['name'], $_GET['pin'] );
    }
    die();
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

if( isset($_GET['setdevprop']) ){
    error_log('setdevprop');
    if( !empty( $device ) ){
        error_log($device);
        $prop = get_device_prop( $device );
        $prop['reverb'] = isset($_GET['reverb']);
        $prop['peer2peer'] = isset($_GET['peer2peer']);
        if( isset($_GET['jittersend']) )
            $prop['jittersend'] = $_GET['jittersend'];
        if( isset($_GET['jitterreceive']) )
            $prop['jitterreceive'] = $_GET['jitterreceive'];
        set_device_prop( $device, $prop );
    }
}

print_head( $user );
    
//html_user_selector( $user );

if ( empty( $device ) ) {
    echo "<p>You are logged in as user {$user}. No device linked to this account.</p>";
    echo '<p><a href="http://:@' . $_SERVER['HTTP_HOST'] . '">logout</a></p>';
} else {
    echo "<p>You are logged in as user <b>{$user}</b> with device <b>{$device}</b>.</p>";
    $devprop = get_device_prop( $device );
    echo '<form class="devprop"><div class="devproptitle">Device properties:</div>' . "\n";
    // device properties:
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
            // only show active rooms
            if( $room == $devprop['room'] ) {
                echo '<div class="myroom">' . "\n";
            } else {
                echo '<div class="room">' . "\n";
            }
            echo '<div class="rname">'.$rprop['name'].'</div>'."\n";
            echo '<div class="rmembers">';
            $roomdev = get_devices_in_room( $room );
            ksort($roomdev);
            foreach( $roomdev as $chair => $dev ){
                $devuser = get_device_user($dev);
                if ( $devuser == $user ){
                    echo '<b>';
                }else{
                    if( $room == $devprop['room'] ) {
                        echo '<a href="?swapdev='.urlencode($dev).'">';
                    }
                }
                //echo htmlspecialchars($devuser) . ' ' . $chair;
                echo htmlspecialchars($devuser);
                if ( $devuser == $user ){
                    echo '</b>';
                }else{
                    if( $room == $devprop['room'] ) {
                        echo '</a>';
                    }
                }
                echo ' ';
            }
            echo '</div>';
            //echo '<div class="rhost">'.$rprop['host'].':'.$rprop['port'].'</div>'."\n";
            if( $room == $devprop['room'] ) {
                echo '<a href="?enterroom=">leave room</a>';
            } else {
                echo '<a href="?enterroom='.urlencode($room).'">enter</a>';
            }
            echo '</div>';
        }
    }
}
print_foot();

?>
