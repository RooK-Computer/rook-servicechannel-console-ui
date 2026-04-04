# Plan 04b - Echte Screens und Preview-Gleichlauf

Status: done

## Ziel

Die vorgesehenen Hauptscreens als echte Module unter `screens/` umsetzen und sicherstellen, dass Preview und Normalbetrieb exakt dieselbe Screen-Implementierung ausfuehren.

## Voraussetzungen

Dieser Plan darf erst beginnen, wenn die gemeinsame SDL-/RmlUi-Laufzeitbasis aus `plans/02b-sdl2-und-rmlui-grafikintegration.md` steht.

## Ergebnis nach Abschluss

* Welcome-, WLAN-, Passwort-, Warte-, Fehler- und Status-Screens existieren als echte Screen-Module unter `screens/`
* dieselben Screen-Module laufen im Preview und im Normalbetrieb
* Preview-Szenarien liefern nur Daten und Startzustand, aber keinen separaten UI-Codepfad
* die in `docs/rook-ui-konzept.md` definierte Microcopy und Screen-Struktur ist in diese Module uebernommen
* die grafische UI laedt projektlokale `JetBrains Mono`-Schriftdateien aus `resources/fonts`
* WLAN- und VPN-Warte-Screens sind auf nicht-interaktive Wartebildschirme mit animiertem Spinner zurueckgefuehrt
* der Start aus Repo-Root und aus `build/` nutzt denselben Ressourcenpfad und kippt nicht mehr wegen des Working Directories in den Text-Fallback

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

## Review-Abschluss

Der Plan ist nach den nachtraeglichen Review-Korrekturen fuer Font, Warte-Screens, Spinner, Welcome-Layout, Status-PIN, Footer-Bereinigung, Scrollleisten und Screen-Listing jetzt akzeptiert.

Nachtraegliche 04b-Nacharbeit:

* der Passwort-Screen ist jetzt nicht mehr nur ein Text-Stub, sondern enthaelt ein echtes On-Screen-Keyboard mit QWERTZ-/Shift-/Alt-/Caps-Logik
* die Bedienregel fuer **B** ist umgesetzt: loeschen bei vorhandener Eingabe, normaler Ruecksprung nur bei leerem Passwort
* fuer Preview und Screen-Listing existiert zusaetzlich die Alias-ID `password`, waehrend `keyboard` aus dem Konzept weiterhin gueltig bleibt
