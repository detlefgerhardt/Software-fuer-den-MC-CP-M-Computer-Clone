@ECHO OFF > nul
rem copy %1.MAC %1.Z80 > nul
rem del %1.MAC > nul
CPM Z80ASM %1/fm7 > %1.zerr
ECHO ASM %1
