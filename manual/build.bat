@echo off
C:
set PATH=C:\cygwin64\bin;%PATH%
bash -c ./build.sh
if ERRORLEVEL 1 goto error
goto end
:error
pause
:end