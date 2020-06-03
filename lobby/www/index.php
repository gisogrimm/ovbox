<?php

$fp = fopen("../lock.txt", "a+");
flock($fp, LOCK_EX );

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
    if( isset($_GET['addgroup']) ){
        add_group($_GET['addgroup']);
        header( "Location: /#groups" );
    }
    if( isset($_GET['rmgroup']) ){
        rm_group($_GET['rmgroup']);
        header( "Location: /#groups" );
    }
    if( isset($_GET['addusertogroup']) ){
        add_user_to_group($_GET['newuser'],$_GET['addusertogroup']);
        header( "Location: /#groups" );
    }
    if( isset($_GET['removeuserfromgroup']) ){
        remove_user_from_group($_GET['groupuser'],$_GET['removeuserfromgroup']);
        header( "Location: /#groups" );
    }
    if( isset($_GET['setgrpstyle'])){
        modify_group_prop( $_GET['setgrpstyle'], 'style', $_GET['grpstyle']);
        header( "Location: /#groups" );
    }
    if( isset($_GET['moduser']) ){
        modify_user_prop( $_GET['moduser'], 'seesall', isset($_GET['seesall']));
        modify_user_prop( $_GET['moduser'], 'maingroup', $_GET['maingroup']);
        header( "Location: /#users" );
    }
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
    if( isset($_GET['rmroom']) ){
        rm_room( $_GET['rmroom'] );
        header( "Location: /" );
    }
    print_head( $user );
    echo '<input type="button" onclick="location.replace(\'/\');" value="Refresh"/>';
    html_admin_devices();
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
    html_admin_users();
    html_admin_groups();
    print_foot();
    die();
}

if( isset($_GET['devselect']) ){
    select_userdev( $user, $_GET['devselect'] );
    header( "Location: /" );
}

$device = get_device( $user );
if( !empty($device) ){
    $devprop = get_device_prop( $device );
    if($user != $devprop['owner']){
        select_userdev( $user, '' );
        header( "Location: /" );
        die();
    }
}
$usergroups = list_groups($user);
$userprop = get_user_prop($user);
$maingroup = $userprop['maingroup'];
if( !in_array($maingroup,$usergroups) )
    $maingroup = '';
$style = '';
if( !empty($maingroup) ){
    $groupprop = get_group_prop($maingroup);
    $style = $groupprop['style'];
}

if( isset($_GET['enterroom']) ) {
    if( !empty( $device ) )
        device_enter_room( $device, $_GET['enterroom'] );
    header( "Location: /" );
}

if( isset($_GET['swapdev']) ){
    if( !empty( $device ) ){
        room_swap_devices( $device, $_GET['swapdev'] );
    }
    header( "Location: /" );
}

if( isset($_GET['lockroom']) ){
    if( !empty( $device ) ){
        lock_room( $_GET['lockroom'], $device, $_GET['lck'] );
    }
    header( "Location: /" );
}

function set_getprop( &$prop, $key )
{
    if( isset($_GET[$key]) ){
        $prop[$key] = $_GET[$key];
    }
}
    
if( isset($_GET['setdevprop']) ){
    if( !empty( $device ) ){
        $prop = get_device_prop( $device );
        $prop['reverb'] = isset($_GET['reverb']);
        $prop['peer2peer'] = isset($_GET['peer2peer']);
        set_getprop($prop,'jittersend');
        set_getprop($prop,'jitterreceive');
        set_getprop($prop,'label');
        set_getprop($prop,'egogain');
        set_getprop($prop,'inputport');
        set_getprop($prop,'inputport2');
        set_getprop($prop,'srcdist');
        set_getprop($prop,'outputport1');
        set_getprop($prop,'outputport2');
        set_properties( $device, 'device', $prop );
    }
    header( "Location: /" );
}

if( isset($_GET['setroom']) ){
    $room = $_GET['setroom'];
    $rprop = get_room_prop($room);
    if( $user == $rprop['owner']){
        if( isset($_GET['label']))
            $rprop['name'] = $_GET['label'];
        set_getprop( $rprop, 'size' );
        if( isset($_GET['sx'])&&isset($_GET['sy'])&&isset($_GET['sz']))
            $rprop['size'] = $_GET['sx'].' '.$_GET['sy'].' '.$_GET['sz'];
        set_getprop( $rprop, 'rvbgain' );
        set_getprop( $rprop, 'rvbdamp' );
        set_getprop( $rprop, 'rvbabs' );
        $rprop['private'] = isset($_GET['private']);
        set_getprop( $rprop, 'group' );
        set_properties( $room, 'room', $rprop );
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

if( isset($_GET['claim']) ){
    $devs = list_unclaimed_devices();
    if( in_array( $_GET['claim'], $devs ) )
        modify_device_prop( $_GET['claim'], 'owner', $user );
    header( "Location: /" );
}

if ( empty( $device ) ) {
    foreach( owned_devices($user) as $dev ){
        header( "Location: /?devselect=" . $dev['dev'] );
        die();
    }
}
    
print_head( $user, $style );


if ( empty( $device ) ) {
    echo "<p>You are logged in as user {$user}. You have no registered device.</p>";
} else {
    $devprop = get_device_prop( $device );
    html_show_user( $user, $device, $devprop );
    html_device_selector( $user, $device );
    echo '<form class="devprop" id="devsettings" style="display: none;"><div class="devproptitle">Device settings:</div>' . "\n";
    // device properties:
    echo '<label for="label">device label (appears in rooms and the mixer of the others): </label><br>';
    echo '<input id="label" name="label" type="text" value="'.$devprop['label'].'" pattern="[a-zA-Z0-9]*"><br>' . "\n";
    echo '<label for="inputport">input ports (to which your microphones/instruments are connected): </label><br>';
    echo '<input id="inputport" name="inputport" type="text" value="'.$devprop['inputport'].'">' . "\n";
    echo '<input id="inputport2" name="inputport2" type="text" value="'.$devprop['inputport2'].'"><br>' . "\n";
    echo '<label for="outputport1">output ports (to which your headphones are connected): </label><br>';
    echo '<input id="outputport1" name="outputport1" type="text" value="'.$devprop['outputport1'].'">' . "\n";
    echo '<input id="outputport2" name="outputport2" type="text" value="'.$devprop['outputport2'].'"><br>' . "\n";
    echo '<label for="srcdist">distance between sources (in case of you send more than one channel): </label><br>';
    echo '<input id="srcdist" name="srcdist" type="number" min="0" step="0.01" value="'.$devprop['srcdist'].'"><br>' . "\n";
    echo '<label for="egogain">ego monitor gain in dB: </label><br>';
    echo '<input id="egogain" name="egogain" type="number" min="-30" max="10" step="0.1" value="'.$devprop['egogain'].'"><br>' . "\n";
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
        html_show_room( $room, $device, $devprop, $room == $devprop['room'], $user, $userprop, $usergroups );
    }
}
print_foot($style);

?>
