echo off > nul
cpm ..\z80asm NPUTSYS/f,NPUTSYS/h
hexreset NPUTSYS.HEX
del NPUTSYS.COM > nul
copy NPUTSYS.HEX ..\..\SYS > nul
