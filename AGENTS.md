# AGENTS.md

## Zweck dieses Repos

Dieses Repository ist das Zuhause der **RooK Console UI** als eigenstaendige Komponente im groesseren Servicechannel-Projekt.

Die Konzeptphase ist weitgehend abgeschlossen, und die Implementierung hat mit **Plan 01** bereits begonnen.

Der aktuelle Schwerpunkt dieses Repos liegt jetzt auf:

* der Abarbeitung der lokalen Implementierungsplaene unter `plans/`,
* dem Spiegeln des Komponentenfortschritts nach `spec/implementation/01-rook-ui-status.md`,
* dem Nachziehen fachlicher Entscheidungen in `docs/rook-ui-konzept.md`, falls waehrend der Umsetzung neue Produktfragen auftauchen.

## Zentrale Einstiegsstellen

Bei inhaltlicher Arbeit an diesem Repo in der Regel in dieser Reihenfolge lesen:

1. `docs/rook-ui-konzept.md`
2. `plans/00-plan-index.md`
3. den ersten Plan mit Status `ready` oder `in_progress`
4. `spec/docs/architecture/servicechannel-concept.md`
5. `spec/implementation/01-rook-ui-status.md`
6. `spec/implementation/11-integrationsbefunde-und-folgearbeiten.md`

Bei Fragen zum lokalen UI-Agent-Vertrag zusaetzlich:

* `spec/openapi/01-ui-agent-local-ipc.openapi.yaml`

## Arbeitsregeln fuer dieses Repo

* Dieses Repo darf UI-Verhalten detaillierter ausarbeiten als die uebergeordnete Spezifikation, aber es darf den dokumentierten Systemgrenzen nicht widersprechen.
* **Stabile Entscheidungen** fuer das lokale UI in `docs/rook-ui-konzept.md` festhalten.
* **Noch offene Produktpunkte** ebenfalls dort sammeln, klar als offen markiert.
* **Implementierungsreihenfolge und Fortschritt** im Ordner `plans/` pflegen, nicht im Konzeptdokument.
* Relevante Fortschrittsaenderungen der Komponente immer auch in `spec/implementation/01-rook-ui-status.md` spiegeln.
* Systemweite oder vertragsrelevante Aenderungen nicht nur lokal dokumentieren; dann muessen auch die passenden Dateien unter `spec/` angepasst werden.
* Keine Implementierungsdetails als beschlossen dokumentieren, solange sie noch Produkt- oder UX-Fragen offenlassen.
* Wenn fuer einen Plan benoetigte **Dev-Pakete** fehlen, ist das ein **Blocker**. In diesem Fall stoppen, den Blocker dokumentieren und **nicht** mit einem Ersatzpfad den Plan als erledigt markieren.
* RmlUi ist fuer die weitere Arbeit **als projektlokale Abhaengigkeit** vorgesehen und soll nicht als `apt`-Paket vorausgesetzt werden.
* Der Preview-Modus muss fuer produktive Hauptscreens **denselben Screen-Code** wie der Normalbetrieb ausfuehren. Preview-Stubs oder separate Preview-Screens zaehlen nicht als Planerfuellung.

## Arbeitsweise mit dem Ordner `plans/`

* `plans/00-plan-index.md` ist die kanonische Uebersicht ueber Reihenfolge und Status.
* Jeder Plan hat einen eigenen Statuskopf (`done`, `in_progress`, `ready`, `blocked`, `rejected`).
* Immer den **ersten** Plan mit Status `ready` bearbeiten, sofern nicht bereits ein anderer Plan `in_progress` ist.
* Nach Abschluss eines Plans:
  * Status im Plan selbst aktualisieren
  * Status in `plans/00-plan-index.md` aktualisieren
  * `spec/implementation/01-rook-ui-status.md` mit dem neuen Komponentenzustand aktualisieren
  * den naechsten Plan auf `ready` setzen
  * dann fuer menschliches Review anhalten und **nicht automatisch** den naechsten Plan beginnen
* Wenn ein Plan wegen falscher Annahmen oder fehlender Voraussetzungen nicht akzeptiert werden kann, bekommt er den Status **`rejected`** und es wird ein klarer Ersatzpfad angelegt.
* Wenn bei der Umsetzung neue fachliche Produktfragen auftauchen, gehoeren sie zurueck nach `docs/rook-ui-konzept.md`.

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
