echo off > nul
cpm ..\z80asm NGET/f,NGET/h
rem hexreset NGET.HEX
unload NGET.HEX 0000
copy NGET.HEX ..\..\SYS > nul
