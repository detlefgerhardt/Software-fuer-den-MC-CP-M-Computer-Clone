# Thema Diskettenformate
Bei den Diskettenformaten gibt es beim den CP/M-Rechner und natürlich auch beim MC CP/M-Computer eine babylonische Verwirrung. Es gibt nicht nur viele unterschiedliche Formate für 8 Zoll (Maxi-disk), 5 1/2 Zoll (Mini-Disk) und 3,5 Zoll (gab es historisch bei den CP/M-Rechnern nicht), sondern auch noch die Unterscheidung zwischen dem physikalischen Format und dem logischen Format von CP/M.
Hier mal ein kurzer Überblick, welche Formate beim hier entwickelten NMON (Monitor) und NBIOS (CP/M-BIOS) an welcher Stelle verwendet werden.

1. Beim Booten eines Datenträgers liest der Monitor eine physikalischen Boot-Sektor vom Datenträger
2. Dieser Boot-Sektor enthält den Boot-Loader, der über den Monitor die physikalischen Sektoren mit dem CP/M-Betriebssystem lädt
3. Daraufhin übernimmen BDOS und BIOS und greifen über logische Sektoren auf das Laufwerk zu.
4. Das Formatprogramm NFORM (und das Original UFORMAT) arheiten mit physikalischen Sektoren direkt auf der Controller-Hardware.
5. Das Tool NPUTSYS zum Schreiben des CP/M-Systems + BIOS auf einen Diskette, verwendet den Monitor und schreibt physikalische Sektoren.
6. Das Tool NPUTBOOT zum Schreiben des Bootloaders auf eine Diskette, verwendet den Monitor und schreibt den Boot-Sektor als physikalischen Sektor.
7. Das Tool NPUTS-CF zum Schreiben des CP/M-Systems + BIOS auf ein Compact-Flash-Volumne verwendet das BIOS und schreibt logische Sektoren.
8. Das Tool NPUTB-CF zum Schreiben des Bootloaders auf ein Compact-Flash-Volumne verwendet den Monitor und schreibt den Boot-Sektor als physikalischen Sektor.
9. Das Tool DISKCOPY verwendet aus Performance-Gründen den Monitor und zum Lesen und Schreiben von physikalische Sektoren.
