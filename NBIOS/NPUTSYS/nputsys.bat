echo off > nul
cpm z80asm NPUTSYS/f,NPUTSYS/h
rem hexreset NPUTSYS.HEX
unload NPUTSYS.HEX 0000
del NPUTSYS.COM > nul
copy NPUTSYS.HEX ..\SYS > nul
