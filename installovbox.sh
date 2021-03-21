#!/bin/bash
(
    # minimal error handling:
    trap "echo an error occured.;exit 1" ERR

    export DEBIAN_FRONTEND=noninteractive

    sudo sed -i -e '/apt.hoertech.de/ d' -e '/^[[:blank:]]*$/ d' /etc/apt/sources.list|| echo "unable to remove previous apt entries"
    wget -qO- https://apt.hoertech.de/openmha-packaging.pub | sudo apt-key add -
    (echo "";echo "deb [arch=armhf] http://apt.hoertech.de bionic universe")|sudo tee -a /etc/apt/sources.list
    sync

    # install dependencies:
    # retry 10 times because the raspios servers are not very stable:
    cnt=10
    while test $cnt -gt 0; do
	sudo -E apt update && cnt=0 || ( sleep 20; let cnt=$cnt-1 )
	sync
    done
    cnt=10
    while test $cnt -gt 0; do
	sudo -E apt upgrade --assume-yes && cnt=0 || ( sleep 20; let cnt=$cnt-1 )
	sync
    done
    cnt=10
    while test $cnt -gt 0; do
	sudo -E apt install --no-install-recommends --assume-yes ov-client libnss-mdns && cnt=0 || ( sleep 20; let cnt=$cnt-1 )
	sync
    done

    # install user to run the scripts - do not provide root priviledges:
    sudo useradd -m -G audio,dialout ov || echo "user already exists."
    sync

    # get autorun file:
    rm -f autorun
    wget https://github.com/gisogrimm/ov-client/raw/master/tools/pi/autorun
    chmod a+x autorun
    sync

    # update real-time priority priviledges:
    sudo sed -i -e '/.audio.*rtprio/ d' -e '/.audio.*memlock/ d' /etc/security/limits.conf
    echo "@audio - rtprio 99"|sudo tee -a /etc/security/limits.conf
    echo "@audio - memlock unlimited"|sudo tee -a /etc/security/limits.conf
    sync

    # register autorun script in /etc/rc.local:
    sudo sed -i -e '/exit 0/ d' -e '/.*autorun.*autorun/ d' -e '/.*home.pi.install.*home.pi.install/ d' -i /etc/rc.local
    echo "test -x /home/pi/autorun && su -l pi /home/pi/autorun &"|sudo tee -a /etc/rc.local
    echo "exit 0"|sudo tee -a /etc/rc.local
    sync

    # setup host name
    HOSTN=$(ov-client_hostname)
    echo "ovbox${HOSTN}" | sudo tee /etc/hostname
    sudo sed -i "s/127\.0\.1\.1.*/127.0.1.1\tovbox${HOSTN}/g" /etc/hosts
    sync

    # configure country code and prepare WiFi config:
    sudo touch /boot/ovclient-wifi.txt
    sync

    # activate overlay image to avoid damage of the SD card upon power off:
    sudo raspi-config nonint enable_overlayfs
    sync

    # ready, reboot:
    sudo shutdown -r now
)
