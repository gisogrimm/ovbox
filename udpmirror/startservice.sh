#!/bin/bash
# Usage: ./startservice.sh <numclients> <startport>
NUMCLIENTS=$1
STARTPORT=$2
test -z "${NUMCLIENTS}" && NUMCLIENTS=1
test -z "${STARTPORT}" && STARTPORT=4366
echo "starting ${NUMCLIENTS} services starting at port ${STARTPORT}"
#killall roomservice
#sleep 1
#killall roomservice
#sleep 1
#killall roomservice
#sleep 1
hst=$(hostname)
k=1

while test $k -le ${NUMCLIENTS}; do
		let p=$k+${STARTPORT}
		let p=$p-1
		./roomservice -l http://oldbox.orlandoviols.com  -p $p -n ${hst}${k} &
		sleep 1
		let k=$k+1
done
