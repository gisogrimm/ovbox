#!/bin/bash
JACKCMD="jackd --sync -P 40 -d alsa -d hw:1 -r 48000 -p 96 -n 2"
. get_boxname.sh
killall jackd zita-j2n zita-n2j tascar_cli
while ! (cat /proc/asound/cards | grep -q -e US2x2); do
    sleep 4
done
sleep 2
$JACKCMD &
sleep 4
(
    cd cfg
    ../udpmirror/devconfigclient -l http://box.orlandoviols.com -d `cat devicename`
)
