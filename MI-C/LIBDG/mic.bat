@ECHO OFF > nul
CPM CCZ /SX %1 > %1.cerr
copy %1.LST %1.clst > nul
copy %1.MAC %1.Z80  > nul
del %1.MAC > nul
CPM Z80ASM %1/fm7 > %1.zerr
ECHO MIC %1
