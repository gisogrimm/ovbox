#!/bin/bash
(
    # minimal error handling:
    trap "echo an error occured.;exit 1" ERR

    export DEBIAN_FRONTEND=noninteractive

    wget -qO- https://apt.hoertech.de/openmha-packaging.pub | sudo apt-key add -
(echo "";echo "deb [arch=armhf] http://apt.hoertech.de bionic universe")|sudo tee -a /etc/apt/sources.list

    # install dependencies:
    sudo -E apt update
    sudo -E apt upgrade --assume-yes
    sudo -E apt install --no-install-recommends --assume-yes ov-client

    # install user to run the scripts - do not provide root priviledges:
    sudo useradd -m -G audio,dialout ov

    # get autorun file:
    rm -f autorun
    wget https://github.com/gisogrimm/ov-client/raw/master/tools/pi/autorun
    chmod a+x autorun

    # update real-time priority priviledges:
    sudo sed -i -e '/.audio.*rtprio/ d' -e '/.audio.*memlock/ d' /etc/security/limits.conf
    echo "@audio - rtprio 99"|sudo tee -a /etc/security/limits.conf
    echo "@audio - memlock unlimited"|sudo tee -a /etc/security/limits.conf

    # register autorun script in /etc/rc.local:
    sudo sed -i -e '/exit 0/ d' -e '/.*autorun.*autorun/ d' -i /etc/rc.local
    echo "test -x /home/pi/autorun && su -l pi /home/pi/autorun &"|sudo tee -a /etc/rc.local
    echo "exit 0"|sudo tee -a /etc/rc.local

    # setup host name
    echo ovbox | sudo tee /etc/hostname
    sudo sed -i "s/127.0.1.1.*raspberry/127.0.1.1\tovbox/g" /etc/hosts

    # activate overlay image to avoid damage of the SD card upon power off:
    sudo raspi-config nonint enable_overlayfs

    # ready, reboot:
    sudo shutdown -r now
)
