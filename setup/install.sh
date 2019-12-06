#!/bin/sh

if [ -z $1 ]; then
  echo "Usage install.sh <target root>"
  exit 1
fi

scriptdir=`dirname $0`
rootdir=`readlink -f $scriptdir/..`
targetroot=`readlink -f "$1"`

echo "Installing from: $rootdir"
echo "Installing to: $targetroot"

sbindir="$targetroot/usr/sbin"
configdir="$targetroot/etc/sensor_hub"
systemddir="$targetroot/lib/systemd/system"
udevdir="$targetroot/etc/udev/rules.d"
manualdir="$targetroot/usr/share/doc/sensor_hub"

sudo mkdir -p "$sbindir"
sudo mkdir -p "$configdir"
sudo mkdir -p "$systemddir"
sudo mkdir -p "$udevdir"
sudo mkdir -p "$manualdir"

sudo scp "$rootdir/build/sensor_hub" "$sbindir/"
[ -f "$rootdir/build/manual/sensor_hub.pdf" ] && sudo scp "$rootdir/build/manual/sensor_hub.pdf" "$manualdir/"
sudo scp "$rootdir/setup/files/sensor_hub.conf" "$configdir/sensor_hub.conf.sample"
sudo scp "$rootdir/setup/files/99-sensor_hub.rules" "$udevdir/"
sudo scp "$rootdir/setup/files/sensor_hub.service" "$systemddir/"
