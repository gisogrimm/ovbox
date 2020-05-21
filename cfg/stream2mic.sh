#!/bin/bash
LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH
killall tascar_cli zita-n2j zita-j2n mplx_client jackd
sleep 1
killall -9 tascar_cli zita-n2j zita-j2n mplx_client jackd
sleep 1
jackd --sync -P 40 -d alsa -d hw:US2x2 -r 48000 -p 144 -n 2 &
sleep 4
zita-j2n 192.168.188.38 4567 &
zita-n2j --buff 20 0.0.0.0 4567 &
sleep 2
jack_connect system:capture_1 zita-j2n:in_1
jack_connect system:capture_2 zita-j2n:in_2
jack_connect zita-n2j:out_1 system:playback_1
jack_connect zita-n2j:out_2 system:playback_2
