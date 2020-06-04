# Installation instructions

The installation instructions are probably incomplete, however, they may give a rough idea of the steps involved.

## Download and install raspbian

Download the Raspberry Pi OS image. The 'Lite' version is sufficient, since the system will be headless:

[https://www.raspberrypi.org/downloads/raspberry-pi-os/](https://www.raspberrypi.org/downloads/raspberry-pi-os/)

To enable ssh access in a headless environment, follow these instructions:
[https://www.raspberrypi.org/documentation/remote-access/ssh/](https://www.raspberrypi.org/documentation/remote-access/ssh/). Essentially you need to create a file `ssh` in the `/boot` partition, e.g. with
````
touch /your/path/to/boot/ssh
````

Now you should be able to login via ssh, with
````
ssh pi@raspberrypi
````
The default password is `raspberry`. This needs to be changed, e.g., to `ovbox2020`. To change the password, type `passwd`.

At this point you may try to use our installer script. If everything goes well, you will have a ready-to-use ovbox. It may take several hours to run. To use the installer script, type these commands as user pi:
````
wget https://github.com/gisogrimm/ovbox/raw/master/installovbox.sh
. installovbox.sh
````
Please mind the space between the dot and `installovbox.sh`.
Your input is required sometimes. Type "yes" when asked if realtime priority should be activated. Type Enter when the TASCAR installer asks to install files.

Finally, restart the system with
````
sudo shutdown -r now
````
If everything went well, approximately 1 minute after powering on the device you should hear an announcement via headphones. You may shutdown the device by simply unplugging the power.


Alternatively, you may follow the manual installation instructions below.

##  Install tools

Install zita-njbridge, jackd2, node-js and OSC library:

````
sudo apt install zita-njbridge jackd2 liblo-dev nodejs libcurl4-openssl-dev build-essential
````
When asked wether to enable realtime priority or not, say `yes`.

Follow the system optimization instructions on
[https://wiki.linuxaudio.org/wiki/raspberrypi](https://wiki.linuxaudio.org/wiki/raspberrypi).

For general system setup (especially `limits.conf`) see [https://wiki.linuxaudio.org/wiki/system_configuration](https://wiki.linuxaudio.org/wiki/system_configuration).


If you are on an arm-based system (e.g., Raspberry Pi), to install TASCAR, follow the instructions on the github page:

[https://github.com/gisogrimm/tascar/blob/master/INSTALL](https://github.com/gisogrimm/tascar/blob/master/INSTALL)

Essentiall these instructions are (to be performed as user `pi`):

````
sudo apt-get install build-essential default-jre doxygen dpkg-dev g++ git imagemagick jackd2 libasound2-dev libboost-all-dev libcairomm-1.0-dev libcurl4-openssl-dev libeigen3-dev libfftw3-dev libfftw3-double3 libfftw3-single3 libgsl-dev libgtkmm-3.0-dev libgtksourceviewmm-3.0-dev libjack-jackd2-dev liblo-dev libltc-dev libmatio-dev libsndfile1-dev libwebkit2gtk-4.0-dev libxml++2.6-dev lsb-release make portaudio19-dev ruby-dev software-properties-common texlive-latex-extra texlive-latex-recommended vim-common wget
git clone https://github.com/gisogrimm/tascar
cd tascar
make
sudo make install
````

These steps may take some time.


On an x86 Ubuntu PC, you may follow the instructions on [http://install.tascar.org](http://install.tascar.org) instead.


## Create user

Create a user without superuser priviledges but belonging to the audio group:

````
sudo adduser ov
sudo adduser ov audio
````

Provide real-time priviledges to the new user:
````
sudo nano /etc/security/limits.conf
````

And add these lines:
````
@audio - rtprio 99
@audio - memlock unlimited
````

## Repository

Create a fork of this repository, or create your own repo. We use the git repository to update the settings in the headless remote boxes upon reboot.



## Add shutdown button to the device

If you activate an overlay file system (see below), this step is not needed. However, when using an overlay file system, the startup time of the device will be significantly longer, because the tool set will be updated and compiled upon every boot.

Since the Raspberry 3 B+ does not have a power button, it is required to add a button for soft shutdown to avoid damage of the SD card file system. We added a button as described here:

[https://www.makeuseof.com/tag/add-power-button-raspberry-pi/](https://www.makeuseof.com/tag/add-power-button-raspberry-pi/)

The python script crashes sometimes, thus we execute it in an endless loop (see below, section autostart). This script is run from the user `pi` for correct priviledges.


## Setup autostart

We added entries to /etc/rc.local to autostart the processes, by adding these lines **before** the `exit 0` line:

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

Make sure that the execution bits are set for the autorun scripts as well as for the start scripts. You can set the execution bits with:

````
chmod a+x autorun ovbox/*.sh
````
as user ov, and similarly as user pi:

````
chmod a+x autorun
````

## Configure sound card

The sound is started in the script `start_audio.sh`. This script is managed by the git repository. It is configured to always use the second sound card. To change the sound device or device settings, you may override the `JACKCMD` variable in a script `ovbox` in the `cfg` directory (assuming that your device has the hostname `ovbox`, otherwise name it according to your host name). In that script add the line

````
export JACKCMD="jackd --sync -P 40 -d alsa -d hw:YourCard -r 48000 -p 96 -n 2"
````

## Finalizing setup

Once everything is setup correctly, you should protect your SD card against failure when powering the system down. You should activate the overlay file system, which can be achieved via `sudo rasbpi-config`.
