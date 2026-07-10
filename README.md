# Software-fuer-den-MC-CP-M-Computer-Clone
Software fuer den MC CP/M-Computer Clone

**Diese Repository befindet sich noch im Aufbau. Die Programme sind teilweise noch unvollständig und ungetestet!**

Die Software in diesem Repository enthält Software für den MC CP/M Copmuter Clone von Ulrich Haumann. Die Software basiert sowohl auf der Originalsoftware von Rolf-Dieter Klein als auch auf Änderungen und neuen Programmen von Ulrich Haumann.


**https://github.com/uli-pi/MC-CPM-Computer-Clone**

Ziel dieses Projektes ist es, die Software des MC CP/M Computer-Clones möglichst gut zu dokumentieren und weiter zu entwickeln.
Der Monitor soll kompatibel bleiben und die Kompatibilität des BIOS ist von CP/M vorgegeben.

Geplante Erweiterungen sind:
- Optimierung, unnötige Routinen entfernen
- Unterstützung weiterer Diskettenformate und eine einfacher Implementierung neuer Formate
- Die Hardwareunterstützung auf die aktuelle Hardware des MC CP/M-Clones reduzieren, um die Software zu vereinfachen. Wer erweiterte Kompatibilität benötigt, kann auf die Originalsoftware zurückgreifen).
- Unterstützung der IDE/CF-Karte gleichzeitig zusammen mit Diskettenlaufwerken
- Vereinheitlichung der Tools - zum Beispiel für die Nutzung von Disketten und IDE/CF-Laufwerke

Dieses Repository gliedert sich in (noch in Arbeit):
- NMON - der Monitor für den CP/M-Computer
- NBIOS - das angepasste BIOS, die Schnittstelle zwischen den Monitor, der Hardware und CP/M
- NBIOS-CF - angepasstes BIOS und Tools für das IDE/CF-Interface (noch nicht funktionsfähig)
- CPM-Tools - Projektspezifische Tools unter CP/M
- CPM-Standardsoftware - Standard CP/M-Programme, die für Entwicklung und die Systemeinrichtung benötigt werden.
- Windows-Tools - Windows-Programme die für Entwicklung des CP/M-Computer verwendet werden.

Achtung. Die Versionierung und die Namen der Dateien wurden geändert, um eine klaren Schnitt zwischen der OriginalSoftare und dieser Software machen.

Das CP/M wurde auf ein 58K-System geändert. Das CP/M beginnt bei Adresse CC00h und das BIOS bei Adresse E200h. Das war notwendig um ausreichend Platz für BIOS-Erweiterung und BIOS-Experimente zu schaffen.

