REM build LIBDG.REL

REM compile c modules
CPM CCZ /SX CONIO > conio.cerr
CPM M80 =CONIO/Z/L > conio.merr
CPM CCZ /SX CGETS > fgets.cerr
CPM M80 =CGETS/Z/L > fgets.merr
CPM CCZ /SX CPRINTF > cprintf.cerr
CPM M80 =CPRINTF/Z/L > cprintf.merr
CPM CCZ /SX RANDOM > random.cerr
CPM M80 =RANDOM/Z/L > random.merr
CPM CCZ /SX TOHEX > tohex.cerr
CPM M80 =TOHEX/Z/L > tohex.merr
CPM CCZ /SX STRUPR > strupr.cerr
CPM M80 =STRUPR/Z/L > strupr.merr
CPM CCZ /SX STRLWR > strlwr.cerr
CPM M80 =STRLWR/Z/L > strlwr.merr
CPM CCZ /SX PHYDRV > phydrv.cerr
CPM M80 =PHYDRV/Z/L > phydrv.merr

REM assemble assembler modules
CPM M80 =MEMSET/Z/L > memset.merr
CPM M80 =MEMCPY/Z/L > memcpy.merr
CPM M80 =OUTP/Z/L > outp.merr
CPM M80 =INP/Z/L > inp.merr
CPM M80 =SIOA/Z/L > sioa.merr
CPM M80 =SIOB/Z/L > siob.merr

REM build library from modules
CPM LIB80 LIBDG=PHYDRV,MEMSET,MEMCPY,OUTP,INP,CPRINTF,CONIO,CGETS,SIOA,SIOB,RANDOM,TOHEX,STRUPR,STRLWR/E > libdg.lerr

MICERR conio.cerr conio.merr fgets.cerr fgets.merr cprintf.cerr cprintf.merr random.cerr random.merr tohex.cerr tohex.merr

MICERR strupr.cerr strupr.merr strlwr.cerr strlwr.merr phydrv.cerr phydrv.merr memset.merr memcpy.merr outp.merr inp.merr sioa.merr siob.merr

MICERR libdg.lerr

COPY LIBDG.REL ..
