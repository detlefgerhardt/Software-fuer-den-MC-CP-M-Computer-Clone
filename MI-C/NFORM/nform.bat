set name="NFORM"
set name2="NFORMLL"

echo off > nul
CPM CCZ /SX %name% > %name%.cerr
CPM M80 =%name%/Z/L > %name%.merr
CPM M80 =%name2%/Z/L > %name2%.merr
CPM L80 %name%/N,XXXMAIN,%name%,%name2%,LIBDG/S,LIB/S/E/Y > %name%.lerr
MICERR %name%.cerr %name%.merr %name2%.merr %name%.lerr

del %name%.HEX > nul
IntelHex bh NFORM.COM > nul
