echo off > nul
cpm z80asm NPUTB-CF/f,NPUTB-CF/h
hexreset NPUTB-CF.HEX
del NPUTS-CF.COM
copy NPUTS-CF.HEX ..\SYS
