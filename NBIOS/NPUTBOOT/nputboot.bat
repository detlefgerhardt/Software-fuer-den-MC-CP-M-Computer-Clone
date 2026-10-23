echo off > nul
cpm ..\z80asm NPUTBOOT/f,NPUTBOOT/h
hexreset NPUTBOOT.HEX
del NPUTBOOT.COM > nul
copy NPUTBOOT.HEX ..\..\SYS > nul
