set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x64\signtool.exe"

if not '%SIGNSHA1%'=='' goto havekey
set SIGNSHA1=742ECBBE6C66BB716C80C1FEA18FAB8FFEBC15BA
:havekey

if '%2'=='' goto nodesc
%SIGNTOOL% sign /sha1 %SIGNSHA1% /d %2 %1
if errorlevel 1 goto error
goto timestamp

:nodesc
%SIGNTOOL% sign /sha1 %SIGNSHA1% /d %~n1 %1
if errorlevel 1 goto error

:timestamp
%SIGNTOOL% timestamp /t http://timestamp.globalsign.com/scripts/timestamp.dll %1
if errorlevel 1 goto error
goto end

:error
if not defined unattended pause

:end 
