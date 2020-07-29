#!/bin/sh

sudo rm -f /etc/udev/rules.d/99-sensor_hub_backup.rules
sudo rm -f /usr/local/bin/sensor_hub_backup.sh
sudo rm -f /usr/local/bin/sensor_hub_backup_umount.sh
sudo systemctl restart udev
