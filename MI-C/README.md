# Programme für den MI-C Compiler 3.18I

Dieser Ordner enthält eine Reihe von Programmen und Tools für CP/M, die mit dem MI-C Compiler geschrieben wurden. Die Tools benötigen in der Regel das angepasste NBIOS und sind nicht mit anderen BIOS-Versionen lauffähig. Das betrifft vor allem die erweiterte Laufwerksparameter-Tabelle.

Ebenfalls enthalten ist eine eigene Library, die das arbeiten mit dem C-Compiler und den MC CP/M-Computer BIOS erleichtet.

Der MI-C Compiler ist ein deutsches Produkt von 1983 und der erste C-Compiler mit dem ich ab 1984 gearbeitet habe. Er implementiert den K&R (Kernighan & Ritchie) Standard von 1978. Der alte Standard unterscheidet sich stark von den neueren Standard (zum Beispiel ANSI C von 1989). Eine Funktion-Deklaration sieht zum Beispiel so aus:

```
  // <- this is NOT allowed
  function(p1, p2, c)
    char *p1, *p2;
    int c
  {
    int a, b, c;
    ...
  }
```
Alle Variablen müssen zu Beginn einer Funktion deklariert werden. "void" gibt es noch nicht (alle Funktionen geben per Default einen int-Wert zurück) und // ist als Kommentar nicht erlaubt.
Aber selbst der alte C Standard hat bereits einen 32 Bit Long Integer, so dass man in vielen Fällen um die langsame Fließkommaarithmetik herumkommt. Außerdem sind die Dateizugriffe gepuffert und damit sehr schnell und unabhängig von den festen 128-Byte Blockgröße von CP/M. Beides sehr große Vorteile und der Grund, warum ich den MI-C-Compiler dem Turbo-Pascal 3.0 vorziehe.
Der MI-C Compiler erzeugt sehr kompakten Z80 Assembler-Code (optional auch 8080), der mit M80 und L80 assmebliert und gelinkt werden kann. Es ist sehr einfach. C und Assemblercode zu mischen. Die Standard-Bibliothek ist sehr limiert, weswegen ich sie stückweise um neue Funktion erweitere, zum Beispiel um die CONIO-Funktionen, die man von ANSI-C gewohnt ist.
Eine große Einschränkung ist, dass beim  M80 nur die ersten 6 Zeichen eines Labels signifikant sind, so dass diese Einschränkung auch für Variablen und Funktionen in C gilt. Das ist sehr gewöhnungsbedürftig.

Ich verwendete den MI-C-Compiler damals (1984) auf einem Sharp MZ80B mit CP/M 2.2. Keine großem Projekte, lediglich einige Tools und ROM-Code für Z80-basierte Embeded-Projekte.

**Wie für das gesamte Repository gilt auc hier: Vieles ist noch unfertig und im Fluss. Es gibt aktuell noch viele Änderungem und die meisten Programme sind nur rudimentär getestet.**

Die Projekte liegen in Unterverzeichnissen. Zum compilieren müssen sie in das übergeordnete MI-C Verzeichnis kopiert werden. Zum Compilieren in der Windows-Console einfach die zugehörige Batch-Datei ausführen.

Hier eine Liste der Projekte (nicht immer aktuell):

- **DISCOPY** kopiert Disketten von einem Laufwerk auf ein anderes, wobei über BIOS-Funktionen geprüft wird, ob die Formatdefinitionen der Laufwerke idebtisch sind
- **KEYCODES** zeigt die Tastencodes (Eingaben) der Terminals an (zum Beispiel um die Belegung von Sondertasten zu ermitteln)
- **LIBDG** ist die eigene Library mit vielen zusätzlichen und zum Teil systemspezifischen Funktion
- **NFORM** ist ein komplett neu geschriebens Formatierprogramm
- **NFORMCF** ist ein neues Formatierprogramm für IDE/CF-Laufwerke. Im wesentlich werden nur die Sektoren mit E5h beschrieben, also eigentlich eher ein Löschprogramm
- **NXDISK** übertragt komplette Disketten als Image per XModem-Protokoll auf den PC (und wieder zurück). Das kann zur Sicherung von Disketten verwendet werden. Oder zum Kopieren, wenn nur ein Laufwerk zu Verfügung steht. Es werden die Standard-Console (serielle Schnittstelle des SYS-Boards) und die beiden SIO-Port des OUT-Boards unterstützt. Die übertragung über das OUT-Board funktioniert mit bis zu 37800 bit/s.
- **SETFMTS** erlaubt das umdefinieren der Laufwerkparameter für einzelne Laufwerke zur Laufzeit
- **SHOWFMTS** zeigt die aktuellen Laufwerksparameter aller Laufwerke inkl. des physikalischen Formats.
- **VT** Zeigt eine VT-Animation auf der Konsole an
- **VTSHOW** Zeigt alle VT100-Animation auf den angegebenen Laufwerken un zufälliger Reihenfolge an

Die folgende Programme sind Testtools, die ja nach Test im Quelltext angepasst werden. Sie sind nicht universell verwendbar:

- **BEBLOCK** wurde zum Testen des Blocking/Deblocking Algorithmus des BIOS verwendet.
- **DISKTEST** wird zum Testen der Laufdwerksroutinen verwendet.
- **SERIAL** Ist ein Testprogramm für diediverse Schnittstellen.
- **SKEWTEST** Gewindigkeittests zur Skew/Interleave-Bestimmung.
- **XMODEM** Ist ein Testprogramm für die XModem-Übertragung.

Generell teste ich die Programm, indem ich sie benutze. Ich habe weder die Zeit noch die Lust für systematische Tests und das Schreiben von Unit-Tests. Dies ist ein Hobby-Projekt, das mit vor allem Spaß machen soll. ;-)
