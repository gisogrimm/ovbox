#!/bin/bash

# install dependencies:
sudo apt install git zita-njbridge jackd2 liblo-dev nodejs libcurl4-openssl-dev build-essential libxml++2.6-dev libwebkit2gtk-4.0-dev $

# clone and install TASCAR acoustic simulator repository:
test -e tascar || git clone https://github.com/gisogrimm/tascar.git
(
  cd tascar
  make
  sudo make install
)
sudo adduser ov
sudo adduser ov audio

# get autorun file:
rm -f autorun
wget https://github.com/gisogrimm/ovbox/raw/master/tools/pi/autorun
chmod a+x autorun

sudo sed -i -e '/.audio.*rtprio/ d' -e '/.audio.*memlock/ d' /etc/security/limits.conf
echo "@audio - rtprio 99"|sudo tee -a /etc/security/limits.conf
echo "@audio - memlock unlimited"|sudo tee -a /etc/security/limits.conf

sudo sed -i -e '/exit 0/ d' -e '/.*autorun.*autorun/ d' -i /etc/rc.local
echo "test -x /home/pi/autorun && su -l pi /home/pi/autorun &"|sudo tee -a /etc/rc.local
echo "exit 0"|sudo tee -a /etc/rc.local

# setup host name
echo ovbox | sudo tee /etc/hostname
sudo sed -i "s/127.0.1.1.*raspberry/127.0.1.1\tovbox/g" /etc/hosts

# clone ovbox repo if not yet available:
sudo su -l ov -c "test -e ovbox || git clone http://github.com/gisogrimm/ovbox"

sudo su -l ov -c "make -C ovbox"

sudo raspi-config nonint enable_overlayfs
