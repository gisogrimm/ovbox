#!/bin/bash
(
    # minimal error handling:
    trap "echo an error occured.;exit 1" ERR

    export DEBIAN_FRONTEND=noninteractive

    # install dependencies:
    sudo -E apt update
    sudo -E apt upgrade --assume-yes
    sudo -E apt install --assume-yes git zita-njbridge jackd2 liblo-dev nodejs libcurl4-openssl-dev build-essential libxml++2.6-dev libwebkit2gtk-4.0-dev libasound2-dev libboost-all-dev libcairomm-1.0-dev libeigen3-dev libfftw3-dev libfftw3-double3 libfftw3-single3 libgsl-dev libgtkmm-3.0-dev libgtksourceviewmm-3.0-dev libjack-jackd2-dev libltc-dev libmatio-dev libsndfile1-dev imagemagick libsamplerate0-dev

    # clone and install TASCAR acoustic simulator repository:
    test -e tascar || git clone https://github.com/gisogrimm/tascar.git
    (
	cd tascar
	make
	sudo make install
    )

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

    # clone ovbox repo if not yet available:
    sudo su -l ov -c "test -e ov-client || git clone http://github.com/gisogrimm/ov-client"
    sudo su -l ov -c "make -C ov-client"

    # activate overlay image to avoid damage of the SD card upon power off:
    sudo raspi-config nonint enable_overlayfs

    # ready, reboot:
    sudo shutdown -r now
)
