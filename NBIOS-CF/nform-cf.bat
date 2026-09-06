echo off > nul
cpm z80asm NFORM-CF/f,NFORM-CF/h
hexreset NFORM-CF.HEX
copy NFORM-CF.HEX ..\SYS
copy NFORM-CF.COM ..\SYS
