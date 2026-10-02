set name="NXDISK"

echo off > nul
CPM CCZ /SX %name% > %name%.cerr
CPM M80 =%name%/Z/L > %name%.merr
CPM L80 %name%/N,XXXMAIN,%name%,LIBDG/S,LIB/S/E/Y > %name%.lerr
MICERR %name%.cerr %name%.merr %name%.lerr

del %name%.HEX > nul
copy %name%.COM ..\SYS > nul
