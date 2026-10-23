# Thema Diskettenformate beim MC CP/M Computer

Bei den Diskettenformaten gibt es beim den CP/M-Rechner und natürlich auch beim MC CP/M-Computer eine babylonische Verwirrung. Es gibt nicht nur viele unterschiedliche Formate für 8 Zoll (Maxi-disk), 5 1/2 Zoll (Mini-Disk) und 3,5 Zoll (gab es historisch bei den CP/M-Rechnern nicht), sondern auch noch die Unterscheidung zwischen dem physikalischen Format und dem logischen Format von CP/M.

Usprünglich verwendete CP/M (CP/M 1.4) 8 Zoll Laufwerke mit 128-Byte-Sektoren. Unter CP/M 2.2 waren dann auch andere Laufwerke mit abweichenden Formaten und Sektorgrößen möglich. CP/M und das BDOS blieben aber immer bei der festen Sektorgröße von 128 Byte. Die Umrechnung (inkl. Blocking und Deblocking) muss das BIOS übernehmen, das für jeden CP/M-Rechner und jeden Diskettentyp individuell angepasst wurde.
Bei 800KB NKC Format, das NMON und NBIOS standardmäßig als Boot-Disketten-Format verwenden, ist das physikalische Format: 1024 Bytes/Sektor, 5 Sektoren/Track, 80 Tracks/Seite und 2 Diskettenseiten.
Das entsprechende logische CP/M-Format ist: 128 Bytes/Sektor, 40 Sektoren/Track, 160 Tracks/Disk.
Weiterhin ist zu beachten, dass beim Monitor die physikalischen Sektoren bei 1 beginnen (1..n) und beim BIOS die logischen Sektoren bei 0 (0..n-1).

Hier mal ein kurzer Überblick, welche Format-Arten beim dem entwickelten NMON (Monitor) und NBIOS (CP/M-BIOS) an welcher Stelle verwendet werden. Es ist geplant, den physikalischen bzw. logischen Zugriff so weit wie möglich zu vereinheitlichen, zum Beispiel bei dem Compact-Flash-Laufwerken.

1. Beim Booten eines Datenträgers liest der Monitor einen physikalischen Boot-Sektor vom Datenträger.
2. Dieser Boot-Sektor enthält den Boot-Loader, der über den Monitor die physikalischen Sektoren mit dem CP/M-Betriebssystem lädt
3. Daraufhin übernimmen BDOS und BIOS und greifen über logische Sektoren auf das Laufwerk zu.
4. Das Formatprogramm NFORM (und das Original UFORMAT) arheiten mit physikalischen Sektoren direkt auf der Controller-Hardware.
5. Das Tool NPUTSYS zum Schreiben des CP/M-Systems + BIOS auf einen Diskette, verwendet den Monitor und schreibt physikalische Sektoren.
6. Das Tool NPUTBOOT zum Schreiben des Bootloaders auf eine Diskette, verwendet den Monitor und schreibt den Boot-Sektor als physikalischen Sektor.
7. Das Tool NPUTS-CF zum Schreiben des CP/M-Systems + BIOS auf ein Compact-Flash-Volumne verwendet das BIOS und schreibt logische Sektoren.
8. Das Tool NPUTB-CF zum Schreiben des Bootloaders auf ein Compact-Flash-Volumne verwendet den Monitor und schreibt den Boot-Sektor als physikalischen Sektor.
9. Das Tool DISKCOPY verwendet aus Performance-Gründen den Monitor und zum Lesen und Schreiben von physikalische Sektoren.
