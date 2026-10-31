REM LIBDG.REL *dg* 30.10.2026
REM build LIBDG.REL
REM
REM revision history
REM 12.10.2026 *dg* - Start of history
REM                 - VT100 added
REM                 - STRICMP added
REM 14.10.2026 *dg* - WAITCH added
REM 16.10.2026 *dg* - hextou() in TOHEX added
REM 22.10.2026 *dg* - hexdmp() in HEXDMP added
REM 29.10.2026 *dg* - getlng() in GETLNG added
REM                 - getint() in GETINT added
REM

REM compile c modules
call mic CONIO
call mic CGETS
call mic CPRINTF
call mic RANDOM
call mic HEXDMP
call mic TOHEX
call mic STRUPR
call mic STRLWR 
call mic PHYDRV
call mic VT100
call mic STRICMP

REM assemble assembler modules
CALL asm MEMSET
CALL asm MEMCPY
CALL asm MEMCMP
CALL asm STRICMP
CALL asm GETLNG
CALL asm GETINT
CALL asm OUTP
CALL asm INP
CALL asm SIOA
CALL asm SIOB
CALL asm WAITCH

REM build library from modules
CPM LIB80 LIB1=PHYDRV,VT100,WAITCH,HEXDMP/E > libdg1.lerr
CPM LIB80 LIB2=MEMSET,MEMCPY,MEMCMP,STRICMP,GETLNG,GETINT,TOHEX/E > libdg2.lerr
CPM LIB80 LIB3=STRUPR,STRLWR,RANDOM,CPRINTF,CGETS,CONIO/E > libdg3.lerr
CPM LIB80 LIB4=SIOA,SIOB,OUTP,INP/E > libdg4.lerr
CPM LIB80 LIBDG=LIB1,LIB2,LIB3,LIB4/E > libdg.lerr
DEL LIB?.REL

rem CPM LIB80 LIBDG=PHYDRV,MEMSET,MEMCPY,MEMCMP,STRICMP,GETLNG,GETINT,VT100,CPRINTF,HEXDMP,SIOA,SIOB,TOHEX/E > libdg1.lerr
rem CPM LIB80 LIBDG=STRUPR,STRLWR,RANDOM,WAITCH,OUTP,INP,CONIO,CGETS,LIBDG/E > libdg2.lerr

MICERR -flibdg.mic

COPY LIBDG.REL ..
