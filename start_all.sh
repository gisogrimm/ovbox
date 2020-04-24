#!/bin/bash
CODIR=$(dirname `which $0`)
(
    echo $CODIR
    cd $CODIR
    git pull || (sleep 20 ; git pull)
    . get_boxname.sh
    ./start_audio.sh &
    while true; do 
	sleep 6
	ssh -R "1440${thisboxno}:localhost:22" ov@mplx.yourdomain.com sleep 60
    done
)
