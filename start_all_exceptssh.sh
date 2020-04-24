#!/bin/bash
CODIR=$(dirname `which $0`)
(
    echo $CODIR
    cd $CODIR
    git pull
    . get_boxname.sh
    ./start_audio.sh
)
