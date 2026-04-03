# AGENTS.md

## Zweck dieses Repos

Dieses Repository ist das Zuhause der **RooK Console UI** als eigenstaendige Komponente im groesseren Servicechannel-Projekt.

Solange die Implementierung noch nicht begonnen hat, liegt der Schwerpunkt hier auf:

* dem Ausarbeiten eines belastbaren UI-Konzepts,
* dem Dokumentieren offener Produkt- und UX-Entscheidungen,
* dem Ableiten eines spaeteren Implementierungsplans.

## Zentrale Einstiegsstellen

Bei inhaltlicher Arbeit an diesem Repo in der Regel in dieser Reihenfolge lesen:

1. `docs/rook-ui-konzept.md`
2. `spec/docs/architecture/servicechannel-concept.md`
3. `spec/implementation/01-rook-ui-status.md`
4. `spec/implementation/11-integrationsbefunde-und-folgearbeiten.md`

Bei Fragen zum lokalen UI-Agent-Vertrag zusaetzlich:

* `spec/openapi/01-ui-agent-local-ipc.openapi.yaml`

## Arbeitsregeln fuer dieses Repo

* Dieses Repo darf UI-Verhalten detaillierter ausarbeiten als die uebergeordnete Spezifikation, aber es darf den dokumentierten Systemgrenzen nicht widersprechen.
* **Stabile Entscheidungen** fuer das lokale UI in `docs/rook-ui-konzept.md` festhalten.
* **Noch offene Punkte** ebenfalls dort sammeln, klar als offen markiert.
* Systemweite oder vertragsrelevante Aenderungen nicht nur lokal dokumentieren; dann muessen auch die passenden Dateien unter `spec/` angepasst werden.
* Keine Implementierungsdetails als beschlossen dokumentieren, solange sie noch Produkt- oder UX-Fragen offenlassen.

## Erwartete Systemgrenzen

Fuer die RooK Console UI gelten aktuell folgende harte Grenzen aus der Spezifikation:

* Fullscreen unter Linux ohne X oder Wayland
* Bedienung primaer per Gamepad
* On-Screen-Keyboard fuer WLAN-Zugangsdaten
* keine direkte Systemlogik in der UI
* Kommunikation ausschliesslich mit dem lokalen RooK Agent

## Dokumentationsstil

* Bevorzugte Sprache in diesem Repo: **Deutsch**
* Produkt- und UX-Entscheidungen moeglichst konkret formulieren
* Wenn Annahmen getroffen werden, diese explizit als Annahmen kennzeichnen

