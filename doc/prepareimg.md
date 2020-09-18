# modify raspios image to ovbox installation

## Prepare loopback device

````
sudo losetup -f -P 2020-08-20-raspios-buster-armhf-lite.img
sudo losetup -l | grep raspios
````

Output will be something like
````
/dev/loop28
````

Mount boot partition with:
````
sudo mount /dev/loop28p1 /mnt/
````

Check that this image was not yet booted; the cmdline.txt should look like this:
````
> cat /mnt/cmdline.txt
console=serial0,115200 console=tty1 root=PARTUUID=907af7d0-02 rootfstype=ext4 elevator=deadline fsck.repair=yes rootwait quiet init=/usr/lib/raspi-config/init_resize.sh
````
Unmount `boot` partition with
````
sudo umount /mnt
````

# Create installer script

Now mount rootfs partition with:
````
sudo mount /dev/loop28p2 /mnt/
````

Create file `/usr/lib/raspi-config/install_ovclient.sh` with this content:


````
#!/bin/sh

reboot_pi () {
    umount /boot
    mount / -o remount,ro
    sync
    echo b > /proc/sysrq-trigger
    sleep 5
    exit 0
}

mount -t proc proc /proc
mount -t sysfs sys /sys
mount -t tmpfs tmp /run
mkdir -p /run/systemd

mount /boot

sed -i 's| init=/usr/lib/raspi-config/install_ovclient\.sh||' /boot/cmdline.txt

mount /boot -o remount,ro
mount / -o remount,rw

sync

# get autorun file:
rm -f autorun
echo "wget https://github.com/gisogrimm/ov-client/raw/master/installovclient.sh" > /home/pi/install
echo ". installovclient.sh" >> /home/pi/install
chmod a+x /home/pi/install

# register autorun script in /etc/rc.local:
sed -i -e '/exit 0/ d' -e '/.*home.pi.install.*home.pi.install/ d' -i /etc/rc.local
echo "test -x /home/pi/install && su -l pi /home/pi/install &"|tee -a /etc/rc.local
echo "exit 0"|tee -a /etc/rc.local

reboot_pi
````


Make executable with
````
chmod a+x /usr/lib/raspi-config/install_ovclient.sh
````

# Modify raspi-config scripts

In `/mnt/usr/lib/raspi-config/init_resize.sh` line 187 modify line
````
sed -i 's| init=/usr/lib/raspi-config/init_resize\.sh||' /boot/cmdline.txt
````
to
````
sed -i 's| init=/usr/lib/raspi-config/init_resize\.sh| init=/usr/lib/raspi-config/install_ovclient\.sh|' /boot/cmdline.txt
````

Unmount loopback device with