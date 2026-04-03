# Plan 04 - Setup-Flow und Produkt-Screens

Status: rejected

## Ziel

Die sichtbaren Hauptscreens der Anwendung umsetzen: Begruessung, WLAN-Liste, Passwort-Screen, Warte-Screens, Fehler-Screens und Status-Screen.

## Ergebnis nach Abschluss

* alle produktiven Hauptscreens existieren
* die Microcopy aus dem Konzept ist uebernommen
* der Screenflow ist im Preview-Modus reviewbar

## Arbeitspakete

1. **Welcome- und Status-Screen**
   * scrollbarer Willkommenstext
   * Checkbox fuer "Beim naechsten Start nicht mehr anzeigen"
   * Status-Screen mit PIN, Trennen und Beenden
   * Trennen-Bestaetigungsdialog als Overlay
2. **WLAN-Screens**
   * WLAN-Liste mit Leerstaat
   * Passwort-Screen mit SSID und Passwortfeld
   * sichtbare Aktionen fuer Verbinden und Zurueck
3. **Warte- und Fehler-Screens**
   * WLAN-Warte-Screen
   * VPN-Warte-Screen
   * WLAN-Fehler-Screen
   * VPN-Fehler-Screen
4. **Keyboard-Screen**
   * QWERTZ-Layout
   * Shift, Alt, Caps Lock
   * B-Verhalten gemaess Konzept

## Hinweise fuer spaetere Umsetzung

* in diesem Plan duerfen Screens zunaechst noch mit Preview-/Mock-Daten arbeiten
* der eigentliche Laufzeitstatus kommt erst mit Plan 05 voll dazu

## Abschlusskriterien

* alle im Konzept geforderten Hauptscreens sind vorhanden
* jeder dieser Screens ist direkt previewbar
* wiederkehrende Screen-Muster nutzen gemeinsame Komponenten

## Rejection-Notiz

Die bisherige Erfuellung dieses Plans gilt als **nicht akzeptiert**.

Begruendung:

* die Hauptscreens wurden **nicht** als echte Screen-Module unter `screens/` umgesetzt
* der Preview-Modus lief ueber einen separaten Stub-/Scenario-Pfad statt ueber dieselbe Screen-Implementierung wie der Normalbetrieb
* damit wurde die eigentliche Architekturabsicht dieses Plans verfehlt

Die dabei entstandenen Vorarbeiten bleiben teilweise nutzbar:

* Microcopy
* Welcome-Text-Ressourcen
* einzelne Daten- und Interaktionsannahmen aus dem Preview-Pfad

Sie zaehlen aber **nicht** als Erfuellung dieses Plans.

Der Ersatzpfad ist `plans/04b-echte-screens-und-preview-gleichlauf.md`.
