#!/bin/bash
mus="0 1 2 3 4"
if test ! -z "$1"; then
    mus="$*"
fi
echo $mus
names[0]=JuliaGiso
names[1]=Marthe
names[2]=Frauke
names[3]=Hille
names[4]=Claas
names[5]=Giso
dup[0]="0"
dup[1]="0"
dup[2]="0"
dup[3]="0"
dup[4]="0"
dup[5]="1000"
#dhost[0]="mplx.yourdomain.com"
#dhost[1]="mplx.yourdomain.com"
#dhost[2]="mplx.yourdomain.com"
#dhost[3]="mplx.yourdomain.com"
#dhost[4]="mplx.yourdomain.com"
#dhost[5]="mplx.yourdomain.com"
dhost[0]="mplx.yourdomain.com"
dhost[1]="mplx.yourdomain.com"
dhost[2]="mplx.yourdomain.com"
dhost[3]="mplx.yourdomain.com"
dhost[4]="mplx.yourdomain.com"
dhost[5]="mplx.yourdomain.com"
buff[0]=10
buff[1]=10
buff[2]=10
buff[3]=11
buff[4]=10
buff[5]=10
numel=$(echo $mus|wc -w)
let daz=200/$numel
for k in $mus; do
    echo ${names[$k]}
    (
	let k2=$k+$k
	let oport=$k2+4464
	name=${names[$k]}
	echo '<?xml version="1.0"?>'
	echo '<session license="CC BY-SA 4.0" attribution="Giso Grimm" initcmd="jackd --sync -P 40 -d alsa -d hw:US2x2 -r 48000 -p 96 -n 2" initcmdsleep="5.0" levelmeter_tc="0.5">'
	echo "  <!-- $name/$oport -->"
	echo "  <scene>"
	az=-96
	for j in $mus; do
	    if test $k -ne $j; then
		let az=$az+$daz
		echo "    <source name=\"${names[$j]}\"><sound maxdist=\"50\" r=\"4\" az=\"${az}\" layers=\"1\"/></source>"
	    fi
	done
	echo "    <include name=\"base_scene.itsc\"/>"
	echo "  </scene>"
	echo "  <modules>"
	echo "    <system command=\"../udpmirror/mplx_client -d ${dhost[$k]} -p 4464 -l ${oport} -c ${k} -o ${dup[$k]}\" onunload=\"killall mplx_client\"/>"
	for j in $mus; do
	    if test $k -ne $j; then
		let j2=$j+$j
		let iport=$j2+4464+${dup[$k]}
		let tbuff=${buff[$k]}+${buff[$j]}
		echo "    <system command=\"zita-n2j --chan 1 --jname ${names[$j]} --buff ${tbuff} 0.0.0.0 ${iport}\" onunload=\"killall zita-n2j\"/>"
	    fi
	done
	#echo "    <system command=\"../headtracker/headtracker\" onunload=\"killall headtracker\"/>"
	echo "    <system command=\"zita-j2n --chan 1 --jname sender --16bit 127.0.0.1 ${oport}\" onunload=\"killall zita-j2n\"/>"
	echo "    <system command=\"sleep 2;sleep 2\"/>"
	echo "    <savegains/>"
	echo "    <touchosc/>"
	echo "    <system command=\"node bridge.js\"/>"
	echo "  </modules>"
	echo "  <connect src=\"render.scene:master_l\" dest=\"system:playback_1\"/>"
	echo "  <connect src=\"render.scene:master_r\" dest=\"system:playback_2\"/>"
	for j in $mus; do
	    if test $k -ne $j; then
		echo "  <connect src=\"${names[$j]}:out_1\" dest=\"render.scene:${names[$j]}.0.0\"/>"
	    fi
	done
	echo "  <connect src=\"system:capture_1\" dest=\"sender:in_1\"/>"
	echo "</session>"
    ) > ovbox${k}.tsc
done
