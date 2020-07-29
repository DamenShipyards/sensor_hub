#!/bin/sh

cat << "EOF" | sudo tee /etc/udev/rules.d/99-sensor_hub_backup.rules >/dev/null
# Automount none sensor_hub USB stick and copy sensor hub logs to it
ACTION=="add", ENV{ID_FS_LABEL}!="", ENV{ID_FS_LABEL}!="sensor_hub", RUN{program}+="/bin/sh -c '/usr/local/bin/sensor_hub_backup.sh $devnode'"
ACTION=="remove", ENV{ID_FS_LABEL}!="", ENV{ID_FS_LABEL}!="sensor_hub", RUN{program}+="/bin/sh -c /usr/local/bin/sensor_hub_backup_umount.sh"
EOF

cat << "EOF" | sudo tee /usr/local/bin/sensor_hub_backup.sh >/dev/null
#!/bin/sh

devnode=$1

fail()
{
  msg=$1
  echo $msg >&2
  exit 1
}

test -z $devnode && fail "No device node specified"
/sbin/fsck -y $devnode
/bin/mkdir -p /media/sensor_hub_backup || fail "Failed to create target directory"
/usr/bin/systemd-mount --no-block --automount=yes $devnode /media/sensor_hub_backup || failed "Failed to mount backup SD card"
/usr/bin/systemd-run --no-block --on-active=5 --description=sensor_hub_backup sh -c "cd /media/sensor_hub/device_logs && /usr/bin/rsync -avzPp \`ls -tp *\` /media/sensor_hub_backup/ && /bin/sync"
EOF
sudo chmod +x /usr/local/bin/sensor_hub_backup.sh

cat << "EOF" | sudo tee /usr/local/bin/sensor_hub_backup_umount.sh >/dev/null
#!/bin/sh

/bin/umount -l -f /media/sensor_hub_backup
/usr/bin/systemd-run --no-block --on-active=5 --description=sensor_hub_backup_umount /bin/rm -rf /media/sensor_hub_backup
EOF
sudo chmod +x /usr/local/bin/sensor_hub_backup_umount.sh

sudo systemctl restart udev
