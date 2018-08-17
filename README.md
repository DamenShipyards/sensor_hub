Damen Sensor Hub
================

Introduction
------------

This readme provides some basic information about this service application. See the full documentation for further details.


Motivation
----------

This service application was initially written for capturing data from Xsens motion and orientation sensing devices and either log or redistribute this data in raw or processed form. 


Installation
------------

On linux:
* Requirements: gcc 4.7+, cmake 3.0+, boost 1.67+
* Create a build directory and run ``cmake <source directory>``
* Run ``make -j 4 && make test``

On Windows:
* Requirements: Visual Studio 2017 (Community Edition),  Boost 1.67+
* Execute ``build.bat`` in the "msvc" directory. In case of build errors, open sensor_hub.sln in Visual Studio
* Run ``build_msi.bat`` in the "setup" directory


