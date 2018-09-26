@echo off
rem Setup variables. You may need to change these for your particular machine setup
rem ================================================
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
set PROJECT="sensor_hub.sln"
set EXE="sensor_hub.exe"
set BUILDDIR="Release"
set DESCRIPTION="Damen Sensor Hub"
set URL="http://www.damen.com/"
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x64\signtool.exe"
set SIGNSHA1=742ECBBE6C66BB716C80C1FEA18FAB8FFEBC15BA
rem ================================================

rem Remove existing build directory
del /S %BUILDDIR% /Q

rem Configure first...
call configure.bat
@if errorlevel 1 goto error

rem Get visual studio build environment
call %VCVARS%
@if errorlevel 1 goto error

rem Build the project
msbuild %PROJECT%
@if errorlevel 1 goto error

rem Assemble full path to executable
set FULLEXE=%BUILDDIR%\\%EXE%

rem Sign the executable
%SIGNTOOL% sign /sha1 %SIGNSHA1% /d %DESCRIPTION% /du %URL% /t http://timestamp.globalsign.com/scripts/timestamp.dll %FULLEXE%
@if errorlevel 1 goto error

@copy ..\3rdparty\libusb\lib\libusb-1.0.dll %BUILDDIR%\\

goto success

:error
echo Error in build!
pause
goto end

:success
echo Done!

:end