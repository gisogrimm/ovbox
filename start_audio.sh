#!/bin/bash
export JACK_NO_AUDIO_RESERVATION=1
(
    cd cfg
    ../udpmirror/devconfigclient -l http://box.orlandoviols.com
)
