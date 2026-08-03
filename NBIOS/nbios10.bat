echo off > nul
cpm z80asm NBIOS10/f,NBIOS10/h
rem COPY NBIOS10.HEX ..\BIOSTST
hexreset NBIOS10.HEX
del NBIOS10.COM
