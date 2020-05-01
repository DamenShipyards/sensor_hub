Damen Sensor Hub
================

Introduction
------------

This readme provides some basic information about this service application. See the full documentation for further details.


Motivation
----------

This service application was initially written for capturing data from Xsens motion and orientation sensing devices and either log or redistribute this data in raw or processed form. 

Download
--------

Use `git clone  --recurse-submodules <sensor hub repo>` in order to checkout sub modules, or `git submodule init && git submodule update` after a 'regular' clone.

Building
------------

On linux:
* Requirements: gcc 4.7+, cmake 3.14+, boost 1.70+ (1.72 doesn't work)
* Create a build directory and run ``cmake <source directory>`` in this directory.
* Run ``make -j 4 && make test``

On Windows:
* Requirements: Visual Studio 2017 (Community Edition),  Boost 1.70+ (installed to `C:\Boost`)
  (e.g. https://sourceforge.net/projects/boost/files/boost-binaries/1.73.0/boost_1_73_0-msvc-14.1-64.exe)
* Execute ``build.bat`` in the "msvc" directory. In case of build errors, open sensor_hub.sln in Visual Studio
* Run ``build_msi.bat`` in the "setup" directory


