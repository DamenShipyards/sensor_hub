@echo off
rem Setup variables. You may need to change these for your particular machine setup
rem ================================================
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
set PROJECT="sensor_hub.sln"
set EXE="sensor_hub.exe"
set BUILDDIR="Release"
set DESCRIPTION="Damen Sensor Hub"
set URL="http://www.damen.com/"
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
msbuild -m %PROJECT%
@if errorlevel 1 goto error

rem Assemble full path to executable
set FULLEXE=%BUILDDIR%\\%EXE%

rem Sign the executable
if "X%SIGNTOOL%"=="X" goto skipsign
if "X%SIGNSHA1%"=="X" echo "You must provide the SHA1 of your signing key as in environment variable SIGNSHA1"
"%SIGNTOOL%" sign /sha1 %SIGNSHA1% /d %DESCRIPTION% /du %URL% /t http://timestamp.globalsign.com/scripts/timestamp.dll %FULLEXE%
@if errorlevel 1 goto error
goto copylibusb
:skipsign
echo "SIGNTOOL environment variable not provided: not signing executable."

:copylibusb
@copy ..\depends\libusb\lib\libusb-1.0.dll %BUILDDIR%\\

goto success

:error
echo Error in build!
pause
goto end

:success
echo Done!

:end
