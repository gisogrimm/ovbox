#!/bin/bash
CODIR=$(dirname `which $0`)
(
    echo $CODIR
    cd $CODIR
    export LD_LIBRARY_PATH=/usr/local/lib
    # make sure this is the first step, so we can fix anything later remotely:
    git pull || (sleep 20 ; git pull)
    # compile binary tools:
    make -C udpmirror
    make -C headtracker
    # read hostname-specific configuration:
    . get_boxname.sh
    # start the audio system, including all network tools:
    ./start_audio.sh &
    # optionally, create reverse tunnel for remote maintenance:
    #while true; do 
    #	sleep 6
    #	ssh -R "1440${thisboxno}:localhost:22" ov@mplx.yourdomain.com sleep 60
    #done
)
