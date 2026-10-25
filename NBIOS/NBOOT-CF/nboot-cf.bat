echo off > nul
set name="NBOOT-CF"

cpm z80asm %name%/f,%name%/h
hexreset %name%.HEX
del %name%.COM > nul
copy %name%.HEX ..\SYS > nul
