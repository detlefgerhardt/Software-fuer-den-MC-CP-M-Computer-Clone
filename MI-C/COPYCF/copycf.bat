set name="COPYCF"
set name2="COPYCFLL"

CPM CCZ /SX %name% > %name%.cerr
CPM M80 =%name%/Z/L > %name%.merr"
CPM M80 =%name2%/Z/L > %name2%.merr
CPM L80 %name%/N,XXXMAIN,%name%,%name2%,LIBDG/S,LIB/S/E/Y > %name%.lerr

MICERR %name%.cerr %name%.merr %name2%.merr %name%.lerr

COPY %name%.COM \Retro\CPM\RunCPM\F\0
