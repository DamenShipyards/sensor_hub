#!/bin/sh

target=$1
test -z $target && target=install_sensor_hub_backup.sh

add()
{
  echo >> $target
  echo "cat << \"EOF\" | sudo tee $1 >/dev/null" >> $target
  cat $1 >> $target
  echo EOF >> $target
  test -z $2 || echo "sudo chmod +x $1" >> $target
}


echo "#!/bin/sh" > $target
add /etc/udev/rules.d/99-sensor_hub_backup.rules
add /usr/local/bin/sensor_hub_backup.sh exec
add /usr/local/bin/sensor_hub_backup_umount.sh exec
echo >> $target
echo "sudo systemctl restart udev" >> $target
chmod +x $target
