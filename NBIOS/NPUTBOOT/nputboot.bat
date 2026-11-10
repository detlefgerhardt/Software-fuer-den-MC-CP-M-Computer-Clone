echo off > nul
cpm z80asm NPUTBOOT/f,NPUTBOOT/h
rem hexreset NPUTBOOT.HEX
unload NPUTBOOT.HEX 0000
del NPUTBOOT.COM > nul
copy NPUTBOOT.HEX ..\SYS > nul
