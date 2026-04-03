# Plan 01 - Projektgrundlage und App-Shell

Status: done

## Ziel

Eine lauffaehige technische Basis fuer die UI schaffen: Buildsystem, Abhaengigkeiten, Ordnerstruktur, App-Shell und Umschaltung zwischen Produktiv- und Preview-Modus.

## Ergebnis nach Abschluss

* C++-Projekt mit Build-Konfiguration und sauberer Abhaengigkeitsdefinition
* Grundstruktur gemaess Konzept (`app/`, `screens/`, `components/`, `services/` oder `ports/`, `adapters/`, `theme/`)
* minimale App-Shell mit Main Loop, Konfiguration und Screen-Start
* klarer Einstieg fuer Normalbetrieb und `--preview <screen-id>`

## Arbeitspakete

1. **Build und Projektstruktur**
   * Buildsystem festziehen und Repository-Struktur anlegen
   * SDL2- und RmlUi-Abhaengigkeiten sauber einbinden
   * Entwicklungs- und Zielgeraete-Buildpfad beruecksichtigen
2. **App-Shell**
   * Programmstart, Main Loop und Shutdown-Verhalten aufsetzen
   * App-Context fuer globale UI-Zustaende vorbereiten
   * Umschaltung zwischen Normalbetrieb und Preview-Modus anlegen
3. **Screen-Registry-Grundlage**
   * stabile String-Screen-IDs einfuehren
   * Start eines ersten Platzhalter-Screens ueber Registry ermoeglichen
4. **Konfigurations- und Dateizugriff vorbereiten**
   * Pfade fuer UI-Konfiguration und spaetere Persistenz festlegen
   * Laden von lokalen Ressourcen vorbereiten

## Hinweise fuer spaetere Umsetzung

* Noch keine echte Agent-Anbindung in diesem Plan
* Noch keine vollstaendigen Produkt-Screens
* Fokus auf tragfaehigem Fundament statt schneller Feature-Dichte

## Abschlusskriterien

* App startet reproduzierbar
* Preview-Aufruf und Normalstart sind technisch getrennt, aber fuehren durch dieselbe App-Shell
* mindestens ein registrierter Test-/Platzhalter-Screen ist renderbar
