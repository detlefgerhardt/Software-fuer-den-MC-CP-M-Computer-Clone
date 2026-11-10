echo off > nul
cpm z80asm NFORM-CF/f,NFORM-CF/h
rem hexreset NFORM-CF.HEX
unload NFORM-CF.HEX 0000
copy NFORM-CF.HEX ..\SYS
copy NFORM-CF.COM ..\SYS
