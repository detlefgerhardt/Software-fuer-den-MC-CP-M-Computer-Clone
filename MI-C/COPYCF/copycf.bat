set name=COPYCF
set name2=COPYCFLL

@echo off > nul
call mic %name%
call asm %name2%
CPM L80 %name%/N,XXXMAIN,%name%,%name2%,LIBDG/S,LIB/S/E/Y > %name%.lerr
MICERR %name%.cerr %name%.zerr %name2%.zerr %name%.lerr

copy %name%.COM ..\SYS > nul
