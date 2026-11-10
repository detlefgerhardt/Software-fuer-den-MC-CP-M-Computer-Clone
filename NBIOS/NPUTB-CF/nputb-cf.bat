echo off > nul
set name="NPUTB-CF"
cpm z80asm %name%/f,%name%/h
rem hexreset %name%.HEX
unload %name%.HEX 0000
del %name%.COM > nul
copy %name%.HEX ..\SYS > nul

