set name="SHOWF16"
set name2="SHOWF16L"

CPM CCZ /SX %name% > %name%.cerr
CPM M80 =%name%/Z/L > %name%.merr"
CPM M80 =%name2%/Z/L > %name2%.merr
CPM L80 %name%/N,XXXMAIN,%name%,%name2%,LIBDG/S,LIB/S/E/Y > %name%.lerr

MICERR %name%.cerr %name%.merr %name2%.merr %name%.lerr

COPY %name%.COM ..\SYS
rem COPY %name%.COM e:\Retro\CPM\RunCPM\F\0

