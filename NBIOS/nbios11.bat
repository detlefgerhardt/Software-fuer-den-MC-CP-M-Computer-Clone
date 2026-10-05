echo off > nul
set name="NBIOS11"

NDiskDef %name%
cpm z80asm %name%/f,%name%/h
hexreset %name%.HEX
del %name%.COM > nul
copy %name%.HEX ..\SYS > nul
