echo off > nul
cpm z80asm NPUTS-CF/f,NPUTS-CF/h
rem hexreset NPUTS-CF.HEX
unload NPUTS-CF.HEX 0000
del nputs-cf.com
