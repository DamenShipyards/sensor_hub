#!/bin/sh

d=`dirname $0`
d=`readlink -f "$d"`
cd "$d"
version=`awk -F= '/Version/ { print $2 }' version.ini | awk -F. '{ print $1"."$2"."$3 }'`
if ! grep $version debian/changelog >/dev/null; then
  echo "debian/changelog does not appear to be up to date.";
  exit 1;
fi
dpkg-buildpackage -b -us -uc 
mv "$d/../sensor-hub_"* "$d/build/"
