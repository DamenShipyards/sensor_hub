if not '%SIGNSHA1%'=='' goto havekey
set SIGNSHA1=742ECBBE6C66BB716C80C1FEA18FAB8FFEBC15BA
:havekey

if '%2'=='' goto nodesc
"c:\Program Files\Microsoft SDKs\Windows\v7.1\bin\signtool.exe" sign /sha1 %SIGNSHA1% /d %2 %1
if errorlevel 1 goto error
goto timestamp

:nodesc
"c:\Program Files\Microsoft SDKs\Windows\v7.1\bin\signtool.exe" sign /sha1 %SIGNSHA1% /d %~n1 %1
if errorlevel 1 goto error

:timestamp
"c:\Program Files\Microsoft SDKs\Windows\v7.1\bin\signtool.exe" timestamp /t http://timestamp.globalsign.com/scripts/timestamp.dll %1
if errorlevel 1 goto error
goto end

:error
if not defined unattended pause

:end 
