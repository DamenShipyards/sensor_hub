git rev-parse --short HEAD > gitrev.txt
cscript version.js
copy config.h.in ..\src\config.h