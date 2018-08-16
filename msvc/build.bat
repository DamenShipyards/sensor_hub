@echo off
rem Setup variables. You may need to change these for your particular machine setup
rem ================================================
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
set PROJECT="sensor_hub.sln"
set EXE="sensor_hub.exe"
set BUILDDIR="Release"
set DESCRIPTION="Damen Sensor Hub"
set URL="http://www.damen.com/"
set SIGNTOOL="c:\Program Files\Microsoft SDKs\Windows\v7.1\bin\signtool.exe"
set SIGNSHA1=742ECBBE6C66BB716C80C1FEA18FAB8FFEBC15BA
rem ================================================

rem Remove existing build directory
del /S %BUILDDIR% /Y

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

goto success

:error
echo Error in build!
pause
goto end

:success
echo Done!

:end