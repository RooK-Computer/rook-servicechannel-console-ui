# Plan 03 - Navigation, Fokus und Standardkomponenten

Status: done

## Ziel

Die wiederverwendbare Interaktionsbasis schaffen: Intent-System, Ruecksprung-Stack, Fokuslogik und gemeinsame Komponenten fuer Listen, Dialoge und Aktionen.

## Ergebnis nach Abschluss

* Intent-Navigation ist technisch umgesetzt
* Ruecksprungverhalten ist zentral definiert
* Fokus- und Scrollverhalten ist fuer Listen, Dialoge und Grid-artige Screens nutzbar
* gemeinsame UI-Komponenten reduzieren Screen-spezifischen Sondercode

## Arbeitspakete

1. **Intent- und Stack-System**
   * `NavigateTo`, `CloseApp`, `NoOp` technisch modellieren
   * Stack fuer Rueckspruenge in der App-Shell verwalten
   * Parameter-Weitergabe pro Screen sauber definieren
2. **Fokus-Engine**
   * D-Pad-Fokus, Stick-Scrollen und Shoulder-Scrollen konsistent abbilden
   * Fokus fuer Listen, Button-Reihen, Dialoge und Grid-Strukturen vereinheitlichen
3. **Standardkomponenten**
   * Buttons und Button-Zeilen
   * Scroll-Container
   * Listen- und Leerstaat-Bausteine
   * Warte- und Fehlerdialog-Grundmuster
4. **Interaktionsregeln**
   * Bestaetigen, Zurueck, Abbrechen und Escape-Verhalten zentral abbilden
   * Overlay-Dialoge innerhalb des aktuellen Screens verankern

## Hinweise fuer spaetere Umsetzung

* Komponenten navigieren nicht selbst
* Screens liefern nur Actions/Intents an die App-Shell

## Umsetzungsnotiz

Plan 03 ist ueber einen terminalbasierten Interaktionspfad umgesetzt:

* zentrales Intent-Modell mit `NavigateTo`, `CloseApp` und `NoOp`
* app-weiter Ruecksprung-Stack fuer Preview-Navigation
* zentrale Tastenzuordnung fuer Fokusbewegung, Bestaetigen, Zurueck und Beenden
* gemeinsame Komponentenmodelle fuer Liste, Action-Row und Dialog
* gemeinsame Fokuslogik fuer Listeneintraege, Aktionsreihen und Dialogaktionen

## Abschlusskriterien

* mindestens ein Listen-Screen und ein Dialog-Screen nutzen dieselben Fokus- und Aktionsmechaniken
* Rueckspruenge funktionieren ohne Screen-spezifische Sonderlogik
