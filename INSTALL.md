# Installation instructions

The installation instructions are probably incomplete, however, they may give a rough idea of the steps involved.

## Repository

Create a fork of this repository, or create your own repo. We use the git repository to update the settings in the headless remote boxes upon reboot.

## Download and install raspbian

Download the raspbian image:

[https://www.raspberrypi.org/downloads/raspbian/](https://www.raspberrypi.org/downloads/raspbian/)

Do not forget to increase the image size after installation.

##  Install tools

Install zita-njbridge and jackd2:

````
sudo apt install zita-njbridge jackd2
````

Follow the system optimization instructions on
[https://wiki.linuxaudio.org/wiki/raspberrypi](https://wiki.linuxaudio.org/wiki/raspberrypi).

We use a manually compiled version of jackd to avoid problems with missing dbus in the headless systems. However, another workaround is documented on the linux audio page.

For general system setup (especially `limits.conf`) see [https://wiki.linuxaudio.org/wiki/system_configuration](https://wiki.linuxaudio.org/wiki/system_configuration).


To install TASCAR, follow the instructions on the github page:

[https://github.com/gisogrimm/tascar/blob/master/INSTALL](https://github.com/gisogrimm/tascar/blob/master/INSTALL)

Essentiall these instructions are:

````
git clone https://github.com/gisogrimm/tascar
sudo apt-get install build-essential default-jre doxygen dpkg-dev g++ git imagemagick jackd2 libasound2-dev libboost-all-dev libcairomm-1.0-dev libcurl4-openssl-dev libeigen3-dev libfftw3-dev libfftw3-double3 libfftw3-single3 libgsl-dev libgtkmm-3.0-dev libgtksourceviewmm-3.0-dev libjack-jackd2-dev liblo-dev libltc-dev libmatio-dev libsndfile1-dev libwebkit2gtk-4.0-dev libxml++2.6-dev lsb-release make portaudio19-dev ruby-dev software-properties-common texlive-latex-extra texlive-latex-recommended vim-common wget
cd tascar
make
sudo make install
````


For the remote mixer, also `node-js` is needed, which can be installed with

````
sudo apt install nodejs
````

## Create user

Create a user without superuser priviledges but belonging to the audio group:

````
sudo adduser ov
usermod -a audio ov
````

## Add shutdown button to the device

Since the Raspberry 3 B+ does not have a power button, it is required to add a button for soft shutdown to avoid damage of the SD card file system. We added a button as described here:

[https://www.makeuseof.com/tag/add-power-button-raspberry-pi/](https://www.makeuseof.com/tag/add-power-button-raspberry-pi/)

The python script crashes sometimes, thus we execute it in an endless loop (see below, section autostart). This script is run from the user `pi` for correct priviledges.


## Setup autostart

We added entries to /etc/rc.local to autostart the processes, by adding these lines:

````
test -x /home/pi/autorun && su -l pi /home/pi/autorun &
test -x /home/ov/autorun && su -l ov /home/ov/autorun &
````

In the script `/home/pi/autostart` we configure system settings which require superuser priviledges. In `/home/ov/autostart` we start  the script `start_all.sh` from this repository  (see also file [tools/ov/autorun](tools/ov/autorun).

The autostart script for the system setup `/home/pi/autorun` looks like this (see also file [tools/pi/autorun](tools/pi/autorun):

````
# deactivate power saving:
for cpu in /sys/devices/system/cpu/cpu[0-9]*; do echo -n performance \
| sudo tee $cpu/cpufreq/scaling_governor; done

## Stop the ntp service
sudo service ntp stop

## Stop the triggerhappy service
sudo service triggerhappy stop

## Stop the dbus service. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
sudo service dbus stop

## Stop the console-kit-daemon service. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
sudo killall console-kit-daemon

## Stop the polkitd service. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
sudo killall polkitd

## Kill the usespace gnome virtual filesystem daemon. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
killall gvfsd

## Kill the userspace D-Bus daemon. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
killall dbus-daemon

## Kill the userspace dbus-launch daemon. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
killall dbus-launch

## Stop	all wifi/bluetooth devices
rfkill block all

## shutdown button:
while true; do
~/shutdown-press-simple.py
sleep 1
done
````

