#!/bin/sh

echo "Please use CMake instead of this script if you can"

scriptpath=`realpath "$0"`
scriptdir=`dirname "$scriptpath"`
basedir=`realpath "$scriptdir/.."`
curdir=`realpath $PWD`
version=`grep Version $basedir/version.ini | cut -c 9-`
year=`date +%Y`


echo "Base directory: $basedir"
echo "Current directory: $curdir"
echo "Version: $version"
echo "Year: $year"

sed s#@CMAKE_CURRENT_SOURCE_DIR@#$basedir#g Makefile.in | sed s#@CMAKE_CURRENT_BINARY_DIR@#$curdir#g > Makefile

sed s#@VERSION@#$version#g manual.texi.in | sed s#@YEAR@#$year#g > manual.texi

make
