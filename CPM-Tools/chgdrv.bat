set name="CHGDRV"

CPM CCZ /SX %name% > %name%.cerr
CPM M80 =%name%/Z/L > %name%.merr"
CPM L80 %name%/N,XXXMAIN,%name%,LIB/S/E/Y > %name%.lerr

MICERR %name%.cerr %name%.merr %name%.lerr
