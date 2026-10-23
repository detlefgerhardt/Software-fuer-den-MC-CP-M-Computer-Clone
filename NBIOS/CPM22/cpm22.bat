echo off
cpm ..\z80asm CPM22/f,CPM22/h
hexreset CPM22.HEX
copy CPM22.COM CPM22.BIN > nul
del CPM22.COM > nul
copy CPM22.HEX ..\..\SYS > nul

