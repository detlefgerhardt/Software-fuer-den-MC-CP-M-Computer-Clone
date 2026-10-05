# Software-fuer-den-MC-CP-M-Computer-Clone
Software fuer den MC CP/M-Computer Clone

**Dieses Repository befindet sich noch im Aufbau. Die Programme sind teilweise noch unvollständig und ungetestet!**

Dieses Repository enthält Software für den MC CP/M Computer Nachbau von Ulrich Haumann. Die Software basiert sowohl auf der Originalsoftware von Rolf-Dieter Klein als auch auf Änderungen und neuen Programmen von Ulrich Haumann.


**https://github.com/uli-pi/MC-CPM-Computer-Clone**

Ziel dieses Projektes ist es, die Software des MC CP/M Computer-Clones möglichst gut zu dokumentieren und weiter zu entwickeln.
Der Monitor soll kompatibel bleiben und die Kompatibilität des BIOS ist von CP/M vorgegeben. Teile des Monitors und des BIOS, die nicht für CP/M benötigt werden, wurden bereits weitgehend entfernt.

Geplante Erweiterungen:
- Optimierung, unnötige Routinen entfernen (im BIOS weitgehend erledigt, im Monitor ist noch einiges zu tun)
- Unterstützung weiterer Diskettenformate und eine einfacher Implementierung neuer Formate (weitgehend erledigt inkl. der dynmaischen Umkonfiguration der Laufwerksdaten zur Laufzwit=
- Die Hardwareunterstützung auf die aktuelle Hardware des MC CP/M-Clones reduzieren, um die Software zu vereinfachen. Wer erweiterte Kompatibilität benötigt, kann auf die Originalsoftware zurückgreifen).
- Unterstützung der IDE/CF-Karte gleichzeitig zusammen mit Diskettenlaufwerken (erledigt)
- Vereinheitlichung der Tools - zum Beispiel für die Nutzung von Disketten und IDE/CF-Laufwerke (hier kann noch weiter optimiert werden. Bisher gibt es noch eigene Tools zum Einrichten von IDE/CF-Laufwerken)

Dieses Repository enthält folgende Teilprojekte (noch in Arbeit, Liste nicht immer aktuell):
- **NMON** - der Monitor für den MC CP/M-Computer, geändert und erweitert, basierendem auf dem Original-Monitor von Rolf-Dieter Klein
- **NBIOS** - das angepasste BIOS, die Schnittstelle zwischen dem Monitor, der Hardware und CP/M (ebenfalls basierenden auf dem Original-BIOS von RDK)
- **NBIOS-CF** - angepasstes BIOS und Tools für das IDE/CF-Interface
- **NFORM** ist das originale in Z80 geschrieben Formatierprogramm für den MC CP/M-Computer. Da es lediglich den Monitor vorrausetzt, kann es ohne CP/M direkt im Monitor ausgeführt werden.
- **MI-C** - in MI-C programmierte Programme und Tools, um Teil abgestimmt auf das NBIOS und nur mit diesem lauffähig
- **DEV-Tools** - Entwicklungstools für CP/M und Windows (Assembler, MI-C-Compiler, eigene Tools)
- **CPM-Standardsoftware** - Standard CP/M-Programme, die für Entwicklung und die Systemeinrichtung benötigt werde oder nützlich sind.
- **VT100-Animationen** - Ausgewählte VT-Animationen inkl. Anzeige-Tools.

Achtung. Die Versionierung und die Namen der Dateien wurden gegenüber den Originalen geändert, um eine klaren Schnitt zwischen der Originalsoftare und den Programmen dieses Projektes zu haben und Verwechselungen zu vermeiden.

Das CP/M wurde auf ein 58K-System geändert. Das CP/M beginnt bei Adresse CC00h und das BIOS bei Adresse E200h. Das war notwendig um ausreichend Platz für BIOS-Erweiterung, Debuggung und Experimente zu schaffen.

