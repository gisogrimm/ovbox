# Installation instructions

The installation instructions are probably incomplete, however, they may
give a rough idea of the steps involved. Any comments and improvements are
welcome!

## Download and install raspbian

Download the Raspberry Pi OS image. The 'Lite' version is sufficient, since the system will be headless:

[https://www.raspberrypi.org/downloads/raspberry-pi-os/](https://www.raspberrypi.org/downloads/raspberry-pi-os/)

If you use the 'Raspberry Pi Imager', choose 'Raspberry Pi OS (other)', 
then  'Raspberry Pi OS Lite (32-bit)'.

### a) headless installation

To enable ssh access in a headless environment (no screen connected to the Pi), follow these instructions:
[https://www.raspberrypi.org/documentation/remote-access/ssh/](https://www.raspberrypi.org/documentation/remote-access/ssh/). Essentially you need to create a file `ssh` in the `/boot` partition, e.g. with
````
touch /your/path/to/boot/ssh
````
After writing the image to the disk, you may need to re-insert the SD-card before you can create that file.

The next step is to insert the SD-card into the Raspberry Pi and boot it. Your PC and the Raspberry Pi should be in the same network.
Now you should be able to login via ssh, with
````
ssh pi@raspberrypi
````
The default password is `raspberry`. This needs to be changed after first boot; to change the password, type `passwd`.

### b) installation via screen

Connect a screen (you will need a micro-HDMI cable) and a keyboard. 
Login as user 'pi'.

## ovbox installer script

At this point you may try to use our installer script. If everything goes well, you will have a ready-to-use ovbox. It may take about one hour to run. To use the installer script, type these commands as user pi:
````
wget https://github.com/gisogrimm/ovbox/raw/master/installovbox.sh
. installovbox.sh
````
Please mind the space between the dot and `installovbox.sh`.
Your input is required sometimes. Type "yes" when asked if realtime priority should be activated. Enter a password for user `ov` when asked for it. All names/room numbers etc can left empty, by pressing Enter. Finally, the system will restart.

If everything went well and if the sound card is connected, approximately 1 minute after powering on the device you should hear an announcement via headphones. You may shutdown the device by simply unplugging the power.

**Warning**: The installer script activates the overlay file system to
prevent a damage of the SD card when powering off the system. This means
that all changes you make after installation will be lost. The overlay file system can be deactivated with:
````
sudo raspi-config nonint disable_overlayfs
````
After applying changes it can be reactivated with:
````
sudo raspi-config nonint enable_overlayfs
sudo shutdown -r now
````