set name=SERIAL

@echo off > nul
call mic %name%
CPM L80 %name%/N,XXXMAIN,%name%,LIBDG/S,LIB/S/E/Y > %name%.lerr
MICERR %name%.cerr %name%.zerr %name%.lerr
