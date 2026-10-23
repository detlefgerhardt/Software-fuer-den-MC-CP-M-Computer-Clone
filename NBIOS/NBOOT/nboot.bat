echo off
cpm ..\z80asm NBOOT/f,NBOOT/h
hexreset NBOOT.HEX
del NBOOT.COM > nul
copy NBOOT.HEX ..\..\SYS > nul
