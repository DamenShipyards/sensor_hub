#!/bin/sh

is_user_root () { [ ${EUID:-$(id -u)} -eq 0 ]; }
leave () { echo $1; exit 3; }

if ! is_user_root; then
  echo "This script requires root permissions."
  exit 1
fi

if [ ! -e /dev/sda ]; then
  echo "Device /dev/sda doesn't exist. Exiting."
  exit 2
fi

echo "Considering:"
lsblk -f /dev/sda
echo
echo "If you continue, I WILL DESTROY ALL DATA on /dev/sda. Press CTRL-C if you don't want this"
read -p "Press <ENTER> to continue" result

echo "Stopping Sensor Hub"
systemctl stop sensor_hub
echo "Unmounting existing logging mount"
umount -f /media/sensor_hub || leave "Failed to unmount."
echo "Create new partition table"
parted -s /dev/sda -- mklabel msdos
echo "Create new partition"
parted -s /dev/sda -- mkpart primary fat32 4MiB -1s
partprobe
echo "Format new partition"
mkfs.vfat /dev/sda1
echo "Set partition label"
fatlabel /dev/sda1 sensor_hub
partprobe
echo
echo "Result:"
lsblk -f /dev/sda
echo
echo "Mounting as logging target"
/usr/bin/systemd-mount --no-block --automount=yes /dev/sda1 /media/sensor_hub 2>/dev/null
while ! df | grep /media/sensor_hub >/dev/null; do
  echo "Waiting for mount to complete"
  sleep 5
done
echo "Restarting Sensor Hub"
systemctl start sensor_hub
