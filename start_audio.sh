#!/bin/bash
export JACK_NO_AUDIO_RESERVATION=1
(
    cd cfg
    VER=$(git rev-parse --short HEAD)
    DEV=$(../udpmirror/getmacaddr)
    ../udpmirror/devconfigclient -l http://oldbox.orlandoviols.com
)
