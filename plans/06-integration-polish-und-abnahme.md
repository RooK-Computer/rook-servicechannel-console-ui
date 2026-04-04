# Plan 06 - Integration, Polish und Abnahme

Status: done

## Ziel

Die UI zu einem konsistenten, reviewbaren Produktstand fuehren: End-to-End-Integration, Feinschliff, finale Preview-Abdeckung und Abnahmevorbereitung.

## Ergebnis nach Abschluss

* Screenflow, Preview und Laufzeitpfade sind zusammengefuehrt
* visuelle und textliche Details sind konsistent
* der Implementierungsstand ist fuer Review und Weiterarbeit sauber dokumentiert

## Arbeitspakete

1. **End-to-End-Abgleich**
   * Flows gegen Konzept pruefen
   * Rueckspruenge, Dialoge und Fehlerpfade angleichen
2. **Preview-Vervollstaendigung**
   * fuer alle produktiven Hauptscreens reproduzierbare Preview-Zustaende hinterlegen
   * Reviewability fuer Design und Produktabnahme absichern
3. **Inhalt und Feinschliff**
   * finalen Willkommenstext einpflegen
   * sichtbare Aktionsreihenfolgen finalisieren
   * spaetere Lokalisierbarkeit sauber vorbereiten
4. **Abnahme und Dokumentation**
   * Planstatus aktualisieren
   * offene Restthemen dokumentieren
   * Uebergang in laufende Implementierungsarbeit oder Wartung vorbereiten

## Umsetzungsergebnis

* Default-Previews fuer die produktiven Hauptscreens sind jetzt mit reviewbaren Beispielparametern hinterlegt
* der Terminal-Fallback folgt im Normalbetrieb jetzt demselben interaktiven Laufzeitpfad wie der grafische Host, statt nur einen statischen Einzel-Screen zu rendern
* der kontrollierte Fehlerabbruch bei fehlendem Agent-/Socket-Zugriff ist damit auch im Diagnosepfad konsistent
* der positive Agent-Startpfad wurde gegen einen lokalen Test-Agenten verprobt; ein aktiver Service-Zustand fuehrt reproduzierbar direkt in den Status-Screen mit PIN
* sichtbare Restdetails wie tote Footer-Hinweise in produktiven Hauptscreens sind entfernt

## Abschlusskriterien

* die Anwendung entspricht dem Konzept in Verhalten und sichtbarer Sprache
* der Plan-Index zeigt einen konsistenten Endstand
