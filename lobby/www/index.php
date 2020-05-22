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
        modify_device_prop( $device, 'access', time() );
    }
    die();
}

// room update:
if ($user == 'room') {
    if( isset($_GET['port']) && isset($_GET['name']) && isset($_GET['pin']) ) {
        update_room( $_SERVER['REMOTE_ADDR'], $_GET['port'], $_GET['name'], $_GET['pin'] );
    }
    die();
}

// admin page:
if( $user == 'admin' ){
    if( isset($_GET['setdevowner']) ){
        modify_device_prop( $_GET['setdevowner'], 'owner', $_GET['owner'] );
        header( "Location: /" );
    }
    if( isset($_GET['setdevlabel']) ){
        modify_device_prop( $_GET['setdevlabel'], 'label', $_GET['label'] );
        header( "Location: /" );
    }
    if( isset($_GET['rmdevice']) ){
        rm_device( $_GET['rmdevice'] );
        header( "Location: /" );
    }
    if( isset($_GET['setroomowner']) ){
        modify_room_prop( $_GET['setroomowner'], 'owner', $_GET['owner'] );
        header( "Location: /" );
    }
    if( isset($_GET['setroomlabel']) ){
        modify_room_prop( $_GET['setroomlabel'], 'name', $_GET['label'] );
        header( "Location: /" );
    }
    if( isset($_GET['setroom']) ){
        if( isset($_GET['label']))
            modify_room_prop( $_GET['setroom'], 'name', $_GET['label'] );
        if( isset($_GET['size']))
            modify_room_prop( $_GET['setroom'], 'size', $_GET['size'] );
        if( isset($_GET['sx'])&&isset($_GET['sy'])&&isset($_GET['sz']))
            modify_room_prop( $_GET['setroom'], 'size', $_GET['sx'].' '.$_GET['sy'].' '.$_GET['sz']);
        modify_room_prop( $_GET['setroom'], 'rvbgain', $_GET['rvbgain'] );
        modify_room_prop( $_GET['setroom'], 'rvbdamp', $_GET['rvbdamp'] );
        modify_room_prop( $_GET['setroom'], 'rvbabs', $_GET['rvbabs'] );
        header( "Location: /" );
    }
    if( isset($_GET['rmroom']) ){
        rm_room( $_GET['rmroom'] );
        header( "Location: /" );
    }
    print_head( $user );
    echo '<input type="button" onclick="location.replace(\'/\');" value="Refresh"/>';
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
    echo "<tr><th>room</th><th>age</th><th>label</th><th>owner</th></tr>\n";
    foreach( $rooms as $room ){
        $dprop = get_room_prop( $room['room'] );
        echo '<tr><td>' .
            $room['room'] . '</td><td>' .
            $room['age'] . '</td><td>' .
            '<form><input type="hidden" name="setroomlabel" value="'.$room['room'].'"><input type="text" name="label" pattern="[a-zA-Z0-9]*" value="'.$dprop['name'].'"><button>Save</button></form>' .
            '</td><td>' .
            '<form>' .
            '<input type="hidden" name="setroomowner" value="'.$room['room'].'">'.
            '<input type="text" title="Owner" name="owner" pattern="[a-zA-Z0-9\.]*" value="'.$dprop['owner'].'">'.
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

if( isset($_GET['devselect']) ){
    select_userdev( $user, $_GET['devselect'] );
    header( "Location: /" );
}

$device = get_device( $user );

if( isset($_GET['enterroom']) ) {
    if( !empty( $device ) )
        device_enter_room( $device, $_GET['enterroom'] );
    header( "Location: /" );
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
    header( "Location: /" );
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
        if( isset($_GET['inputport']) )
            $prop['inputport'] = $_GET['inputport'];
        set_device_prop( $device, $prop );
    }
    header( "Location: /" );
}

if( isset($_GET['setroom']) ){
    $rprop = get_room_prop($_GET['setroom']);
    if( $user == $rprop['owner']){
        if( isset($_GET['label']))
            modify_room_prop( $_GET['setroom'], 'name', $_GET['label'] );
        if( isset($_GET['size']))
            modify_room_prop( $_GET['setroom'], 'size', $_GET['size'] );
        if( isset($_GET['sx'])&&isset($_GET['sy'])&&isset($_GET['sz']))
            modify_room_prop( $_GET['setroom'], 'size', $_GET['sx'].' '.$_GET['sy'].' '.$_GET['sz']);
        if( isset($_GET['rvbgain']) )
            modify_room_prop( $_GET['setroom'], 'rvbgain', $_GET['rvbgain'] );
        if( isset($_GET['rvbdamp']) )
            modify_room_prop( $_GET['setroom'], 'rvbdamp', $_GET['rvbdamp'] );
        if( isset($_GET['rvbabs']) )
            modify_room_prop( $_GET['setroom'], 'rvbabs', $_GET['rvbabs'] );
    }
    header( "Location: /" );
}

if( isset($_GET['clearroom']) ){
    $room = $_GET['clearroom'];
    $rprop = get_room_prop($room);
    if( $user == $rprop['owner']){
        $roomdev = get_devices_in_room( $room );
        foreach( $roomdev as $dev ){
            modify_device_prop( $dev, 'room', '');
        }
    }
    header( "Location: /" );
}

print_head( $user );
    

if ( empty( $device ) ) {
    echo "<p>You are logged in as user {$user}. Select a device to book a room.</p>";
    html_device_selector( $user, $device );
} else {
    $devprop = get_device_prop( $device );
    echo "<p>You are logged in as user <b>{$user}</b> with device <b>{$device} (".$devprop['label'].")</b>.</p>";
    html_device_selector( $user, $device );
    echo '<form class="devprop" id="devsettings" style="display: none;"><div class="devproptitle">Device settings:</div>' . "\n";
    // device properties:
    echo '<label for="label">device label (appears in rooms and the mixer of the others): </label><br>';
    echo '<input id="label" name="label" type="text" value="'.$devprop['label'].'" pattern="[a-zA-Z0-9]*"><br>' . "\n";
    echo '<label for="inputport">input port (to which your microphone/instrument is connected): </label><br>';
    echo '<input id="inputport" name="inputport" type="text" value="'.$devprop['inputport'].'"><br>' . "\n";
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
        html_show_room( $room, $device, $room == $devprop['room'], $user );
    }
}
print_foot();

?>
