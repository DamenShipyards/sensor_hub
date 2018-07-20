@echo off

echo Creating version data
cscript version.js
if ERRORLEVEL 1 goto versionerror


:normal
if not exist "%WIX%/bin/candle.exe" goto nothing

"%WIX%\bin\candle.exe" -arch x64 -ext WixUtilExtension.dll -o Output\setup.wixobj setup.wxs @setup.resp
if ERRORLEVEL 1 goto wixerror 

rem suppress ICE09 I *do* want to install a non permanent dll in the system folder. Sorry about that. Windows
rem had no other way (that I know of) to make sure a dll is found by "LoadLibrary" without providing path information 
rem suppress ICE61 because I do want to allow same version upgrades
rem suppress ICE03 because the string lengths should not be a problem
rem suppress ICE80 64 bit madness: don't know how to get around this one, so disable it.
rem suppress ICE82 not sure if duplicate sequence numbers are a problem. They don't appear to be. Apparently some issue with mergemod.dll
"%WIX%\bin\light.exe" -ext WixUtilExtension.dll -sice:ICE09 -sice:ICE61 -sice:ICE03 -sice:ICE80 -sice:ICE82 Output\setup.wixobj @msi.resp
if ERRORLEVEL 1 goto wixerror 

for %%m in (Output\*.msi) do set msifile=%%m
echo MSI package: %msifile%
call "%SWDROOT%\Scripts\Sign.bat" %msifile% "Sensor Hub Windows Installer"
goto end

:nothing
echo Wix not found.

goto end
:wixerror
echo Error building MSI installer
echo - Press Enter -
if not defined unattended pause
goto end
:versionerror
echo Error getting version
echo - Press Enter -
if not defined unattended pause
goto end
:comperror
echo Error compiling setup
echo - Press Enter -
if not defined unattended pause
goto end
:innoerror
echo Error finding Inno Setup. Is it installed ?
echo - Press Enter -
if not defined unattended pause
goto end
:end
