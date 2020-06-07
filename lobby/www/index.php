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
        $host = '';
        if( isset($_GET['host']) )
            $host = $_GET['host'];
        get_tascar_cfg( $device, $devhash );
        // touch device file:
        modify_device_prop( $device, 'access', time() );
        modify_device_prop( $device, 'host', $host );
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
        die();
    }
    if( isset($_GET['rmgroup']) ){
        rm_group($_GET['rmgroup']);
        header( "Location: /#groups" );
        die();
    }
    if( isset($_GET['addusertogroup']) ){
        add_user_to_group($_GET['newuser'],$_GET['addusertogroup']);
        header( "Location: /#groups" );
        die();
    }
    if( isset($_GET['removeuserfromgroup']) ){
        remove_user_from_group($_GET['groupuser'],$_GET['removeuserfromgroup']);
        header( "Location: /#groups" );
        die();
    }
    if( isset($_GET['setgrpstyle'])){
        modify_group_prop( $_GET['setgrpstyle'], 'style', $_GET['grpstyle']);
        header( "Location: /#groups" );
        die();
    }
    if( isset($_GET['moduser']) ){
        modify_user_prop( $_GET['moduser'], 'seesall', isset($_GET['seesall']));
        modify_user_prop( $_GET['moduser'], 'maingroup', $_GET['maingroup']);
        header( "Location: /#users" );
        die();
    }
    if( isset($_GET['setdeviceowner']) ){
        modify_device_prop( $_GET['setdeviceowner'], 'owner', $_GET['owner'] );
        header( "Location: /#devices" );
        die();
    }
    if( isset($_GET['setdevicelabel']) ){
        modify_device_prop( $_GET['setdevicelabel'], 'label', $_GET['label'] );
        header( "Location: /#devices" );
        die();
    }
    if( isset($_GET['rmdevice']) ){
        rm_device( $_GET['rmdevice'] );
        header( "Location: /#devices" );
        die();
    }
    if( isset($_GET['setroomowner']) ){
        modify_room_prop( $_GET['setroomowner'], 'owner', $_GET['owner'] );
        header( "Location: /#rooms" );
        die();
    }
    if( isset($_GET['setroomlabel']) ){
        modify_room_prop( $_GET['setroomlabel'], 'label', $_GET['label'] );
        header( "Location: /#rooms" );
        die();
    }
    if( isset($_GET['rmroom']) ){
        rm_room( $_GET['rmroom'] );
        header( "Location: /#rooms" );
        die();
    }
    print_head( $user );
    echo '<input type="button" onclick="location.replace(\'/\');" value="Refresh"/>';
    html_admin_db('device');
    html_admin_db('room');
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
    $devprop = get_properties( $device, 'device' );
    if($user != $devprop['owner']){
        select_userdev( $user, '' );
        header( "Location: /" );
        die();
    }
}
$usergroups = list_groups($user);
$userprop = get_properties($user,'user');
$maingroup = $userprop['maingroup'];
if( !in_array($maingroup,$usergroups) )
    $maingroup = '';
$style = '';
if( !empty($maingroup) ){
    $groupprop = get_properties( $maingroup, 'group' );
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
        $prop = get_properties( $device, 'device' );
        $prop['reverb'] = isset($_GET['reverb']);
        $prop['peer2peer'] = isset($_GET['peer2peer']);
        $prop['rawmode'] = isset($_GET['rawmode']);
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
            $rprop['label'] = $_GET['label'];
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

if( isset($_GET['unclaim']) ){
    if( $devprop['owner'] = $user )
        modify_device_prop( $device, 'owner', '');
    header( "Location: /" );
}

if ( empty( $device ) ) {
    foreach( owned_devices($user) as $dev=>$dprop ){
        header( "Location: /?devselect=" . $dev );
        die();
    }
}
    
print_head( $user, $style );

$devs = list_unclaimed_devices();
if( !empty($devs) ){
    echo '<div class="devclaim">';
    echo "Unclaimed active devices exist. If your device is active now, you may claim one of these devices: <br/>\n";
    foreach( $devs as $dev ){
        echo '<form style="display: inline;"><input type="hidden" name="claim" value="'.$dev.'"/><button>'.$dev."</button></form>\n ";
    }
    echo "</div>";
}

if ( empty( $device ) ) {
    echo "<p>You are logged in as user {$user}. You have no registered device.</p>";
} else {
    $devprop = get_properties( $device, 'device' );
    html_show_user( $user, $device, $devprop );
    html_show_device( $user, $device, $devprop );
    echo '<p>Rooms: (<a href="http://' . $_SERVER['HTTP_HOST'] . '">refresh</a>)</p>' . "\n";
    foreach( get_rooms() as $room){
        html_show_room( $room, $device, $devprop, $room == $devprop['room'], $user, $userprop, $usergroups );
    }
}
print_foot($style);

?>
