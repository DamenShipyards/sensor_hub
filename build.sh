#!/bin/sh

d=`dirname $0`
d=`readlink -f "$d"`
cd "$d"
dpkg-buildpackage -b -us -uc 
mv "$d/../sensor-hub_"* "$d/build/"
