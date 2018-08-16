set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
set EXE="Release\sensor_hub.exe"
set DESCRIPTION="Damen Sensor Hub"
if not '%SIGNSHA1%'=='' goto havekey
set SIGNSHA1=742ECBBE6C66BB716C80C1FEA18FAB8FFEBC15BA
:havekey

rem Configure first...
call configure.bat

rem Get visual studio environment
call %VCVARS%
rem Build the project
msbuild sensor_hub.sln

rem Sign ...
"c:\Program Files\Microsoft SDKs\Windows\v7.1\bin\signtool.exe" sign /sha1 %SIGNSHA1% /d "%DESCRIPTION%" %EXE%
rem ... and timestamp
"c:\Program Files\Microsoft SDKs\Windows\v7.1\bin\signtool.exe" timestamp /t http://timestamp.globalsign.com/scripts/timestamp.dll %EXE%