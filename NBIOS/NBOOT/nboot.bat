echo off
cpm ..\z80asm NBOOT/f,NBOOT/h
rem hexreset NBOOT.HEX
unload NBOOT 0000
del NBOOT.COM > nul
copy NBOOT.HEX ..\..\SYS > nul
