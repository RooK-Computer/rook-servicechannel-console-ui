# RooK Console UI - Implementierungsplan-Index

Status: Aktiv

## Zweck

Dieser Ordner zerlegt das UI-Konzept in nacheinander abarbeitbare Implementierungsplaene.

Zukuenftige Agents sollen hier sofort erkennen:

* welche Vorarbeit bereits abgeschlossen ist,
* welcher Plan als naechstes bearbeitet werden soll,
* welche Plaene noch blockiert sind,
* welche Datei nach Abschluss eines Plans aktualisiert werden muss.

## Statusmodell

Verwendete Statuswerte:

* `done` - Plan komplett umgesetzt
* `in_progress` - Plan wird aktuell bearbeitet
* `ready` - kann als naechstes begonnen werden
* `blocked` - haengt von vorherigen Plaenen ab
* `rejected` - in der bisherigen Form nicht akzeptiert; erfordert Ersatz oder Neuansatz

## Aktueller Einstiegspunkt

* Fachliche Grundlage: `docs/rook-ui-konzept.md`
* Naechster auszufuehrender Plan: keiner; die lokale Planreihe ist abgearbeitet und wartet auf menschliche Abnahme

## Planreihenfolge

| Reihenfolge | Datei | Ziel | Status | Abhaengigkeiten |
| --- | --- | --- | --- | --- |
| Konzept | `docs/rook-ui-konzept.md` | Fachliche und UX-Grundlage | `done` | - |
| 01 | `plans/01-projektgrundlage-und-app-shell.md` | Build, Ordnerstruktur, App-Shell, Laufzeitmodi | `done` | Konzept |
| 02 | `plans/02-rendering-theme-und-preview.md` | RmlUi/SDL-Rendering, Theme-Tokens, Preview-Modus | `rejected` | 01 |
| 02a | `plans/02a-rmlui-ins-projekt-einbetten.md` | RmlUi als projektlokale Drittanbieterabhaengigkeit einbetten | `done` | 01 |
| 02b | `plans/02b-sdl2-und-rmlui-grafikintegration.md` | echte grafische SDL2-/RmlUi-Integration nachziehen | `done` | 01, 02a |
| 03 | `plans/03-navigation-fokus-und-standardkomponenten.md` | Navigation, Intent-System, Fokuslogik, Basiskomponenten | `done` | 01 |
| 04 | `plans/04-setup-flow-und-produkt-screens.md` | Welcome-, Setup-, Fehler- und Status-Screens | `rejected` | 03 |
| 04b | `plans/04b-echte-screens-und-preview-gleichlauf.md` | produktive Hauptscreens als echte `screens/`-Module mit Preview-Gleichlauf | `done` | 02b, 03 |
| 05 | `plans/05-agent-anbindung-und-laufzeitlogik.md` | Agent-Port, Status-Mapping, Persistenz, echte Laufzeitpfade | `done` | 01, 02a, 02b, 03, 04b |
| 06 | `plans/06-integration-polish-und-abnahme.md` | End-to-End-Integration, Texte, Preview-Vervollstaendigung, Abnahme | `done` | 02a, 02b, 04b, 05 |
| 07 | `plans/07-debian-paketierung-und-emulationstation-integration.md` | Debian-Paketierung mit nfpm sowie RetroPie-/EmulationStation-Integration | `done` | 06 |

## Pflege-Regeln

Wenn ein Plan bearbeitet wird:

1. Status in dieser Datei auf `in_progress` setzen.
2. Status im betroffenen Plan-Dokument ebenfalls anpassen.
3. Nach Abschluss den Plan auf `done` setzen.
4. Den naechsten unblocked Plan auf `ready` setzen.
5. Den komponentenspezifischen Status unter `spec/implementation/01-rook-ui-status.md` mitziehen.
6. `AGENTS.md` nur anpassen, wenn sich der empfohlene Einstieg oder die Planstruktur aendert.
7. Nach Abschluss eines Plans **nicht automatisch** mit dem naechsten Plan weitermachen, sondern fuer Review anhalten.
8. Fehlende Dev-Pakete fuer einen Plan gelten als **Blocker** und duerfen nicht durch Ersatzimplementierungen als "done" ersetzt werden.
