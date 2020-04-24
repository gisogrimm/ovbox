#!/bin/bash
JACKCMD="echo start"
TSCCMD="tascar_cli -s"
. get_boxname.sh
killall jackd zita-j2n zita-n2j tascar_cli
make -C headtracker
make -C udpmirror
while ! (cat /proc/asound/cards | grep -q -e US2x2); do
    sleep 4
done
sleep 2
$TSCCMD cfg/${thisbox}.tsc
