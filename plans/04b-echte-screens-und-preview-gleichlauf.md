# Plan 04b - Echte Screens und Preview-Gleichlauf

Status: blocked

## Ziel

Die vorgesehenen Hauptscreens als echte Module unter `screens/` umsetzen und sicherstellen, dass Preview und Normalbetrieb exakt dieselbe Screen-Implementierung ausfuehren.

## Voraussetzungen

Dieser Plan darf erst beginnen, wenn die gemeinsame SDL-/RmlUi-Laufzeitbasis aus `plans/02b-sdl2-und-rmlui-grafikintegration.md` steht.

## Ergebnis nach Abschluss

* Welcome-, WLAN-, Passwort-, Warte-, Fehler- und Status-Screens existieren als echte Screen-Module unter `screens/`
* dieselben Screen-Module laufen im Preview und im Normalbetrieb
* Preview-Szenarien liefern nur Daten und Startzustand, aber keinen separaten UI-Codepfad
* die in `docs/rook-ui-konzept.md` definierte Microcopy und Screen-Struktur ist in diese Module uebernommen

## Arbeitspakete

1. **Screen-Registry auf echte Screen-Module umstellen**
   * stabile Screen-IDs auf konkrete Screen-Klassen bzw. Screen-Module mappen
   * keine produktiven Hauptscreens mehr nur ueber Preview-Stub-Modelle bereitstellen
2. **Einzelne Hauptscreens in `screens/` umsetzen**
   * Welcome- und Status-Screen
   * WLAN-Liste und Passwort-/Keyboard-Screen
   * WLAN-/VPN-Warte-Screens
   * WLAN-/VPN-Fehler-Screens
3. **Preview-Gleichlauf sicherstellen**
   * `rook-ui --preview <screen-id>` startet dieselben Screen-Module wie der Normalbetrieb
   * Preview-Szenarien injecten nur Daten, Flags und Startparameter
4. **Wiederverwendbare Komponenten sauber anbinden**
   * Dialoge, Listen, Action-Reihen, Scroll-Container und Keyboard-Mechaniken aus gemeinsamen Komponenten aufbauen
   * keine Screen-duplizierenden Preview-Sonderbauten einfuehren

## Abschlusskriterien

* der Ordner `screens/` enthaelt die produktiven Hauptscreens
* `rook-ui --preview welcome` und `rook-ui --preview status` laufen ueber dieselben Screen-Module wie der Normalbetrieb
* die Architektur nutzt keine separaten Preview-Screens fuer produktive Hauptscreens
