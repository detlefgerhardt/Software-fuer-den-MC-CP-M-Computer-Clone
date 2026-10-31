set name=SHOWF16
set name2=SHOWF16L

@echo off > nul
call mic %name%
call asm %name2%
CPM L80 %name%/N,XXXMAIN,%name%,%name2%,LIBDG/S,LIB/S/E/Y > %name%.lerr
MICERR %name%.cerr %name%.zerr %name2%.zerr %name%.lerr

del %name%.HEX > nul
IntelHex bh NFORM.COM > nul

copy %name%.COM ..\SYS > nul
rem COPY %name%.COM e:\Retro\CPM\RunCPM\F\0
