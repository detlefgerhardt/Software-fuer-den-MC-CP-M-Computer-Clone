echo off
cpm ..\z80asm CPM22/f,CPM22/h
rem hexreset CPM22.HEX
unload CPM22.HEX 0000
copy CPM22.COM CPM22.BIN > nul
del CPM22.COM > nul
copy CPM22.HEX ..\..\SYS > nul

