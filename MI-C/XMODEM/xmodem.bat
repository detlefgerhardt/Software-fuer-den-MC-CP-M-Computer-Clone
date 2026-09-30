set name="XMODEM"

echo off > nul
CPM CCZ /SX %name% > %name%.cerr
CPM M80 =%name%/Z/L > %name%.merr
CPM L80 %name%/N,XXXMAIN,%name%,LIBDG/S,LIB/S/E/Y > %name%.lerr
MICERR %name%.cerr %name%.merr %name%.lerr

del XMODEM.HEX > nul
copy %name%.COM e:\Retro\CPM\RunCPM\F\0\ > nul
