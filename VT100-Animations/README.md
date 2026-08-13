# VT100 Animationen für den MC CP/M Computer
Eine Auswahl der besten VT100 Animationen von http://artscene.textfiles.com/vt100/ und zwei Viewer-Programme für den MC CP/M-Computer.

**VT.COM** ist ein Viewer für einzelen VT100-Files. Natürlich kann man die Files auch mit **pip con:=file.vt** ausgeben. Aber die einzelnen Files erwarten zum Teil unterschiedliche Terminaleinstellungen, zum Beispiel beim Zeileende. Der Viewer behebt das Problem und schaltet vor dem Starten der Animation den Cursor ab.

**VTSHOW.COM** zeigt VT100-Files in zufälliger Reihenfolge als Endlos-Demo an. Die Files können auf mehrere Laufwerke verteilt werden und für den Zufallsgenerator ist ein Seed anzugeben. Zum Beispiel: **VTSHOW A B F 100**. Wenn man keine Laufwerke angibt, wird das aktuelle Laufwerk verwendet.
