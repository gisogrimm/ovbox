#!/bin/bash
if test "$1" = ""; then
    CMD="ssh ov@mplx.yourdomain.com cat ovbox/udpmirror/mplx.log"
else
    CMD="ssh $1 cat ovbox/udpmirror/mplx.log"
fi
DATE=$(LC_ALL=C date -u +"%a %b %e")
ONAME=$(date +"%Y_%m_%d")
echo $DATE
$CMD | grep -e "${DATE}" > $ONAME
octave --eval "plotstat('${ONAME}');"
