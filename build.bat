@echo off

pushd manual
call build.bat
if ERRORLEVEL 1 goto error
popd

pushd msvc
call build.bat
if ERRORLEVEL 1 goto error
popd

pushd setup
call build.bat
if ERRORLEVEL 1 goto error
popd

goto success

:error
popd
echo "Build Error."
goto end

:success
echo "Done."
:end

