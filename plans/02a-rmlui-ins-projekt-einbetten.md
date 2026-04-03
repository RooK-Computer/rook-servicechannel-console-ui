# Plan 02a - RmlUi ins Projekt einbetten

Status: done

## Ziel

RmlUi als projektlokale Drittanbieterabhaengigkeit einbetten, damit die spaetere grafische UI-Integration nicht von einer Systeminstallation per `apt` abhaengt.

## Hintergrund

RmlUi laesst sich in der Zielumgebung nicht verlaesslich als Dev-Paket aus den Paketquellen beziehen.

Deshalb wird RmlUi vor der eigentlichen Grafikintegration als fester Bestandteil dieses Repos vorbereitet.

## Ergebnis nach Abschluss

* eine feste, im Repo dokumentierte RmlUi-Quelle ist eingebunden
* CMake kann RmlUi aus dem Projekt selbst heraus bauen oder einbinden
* die spaetere SDL2-/Render-Integration in Plan 02b kann auf diese eingebettete Basis aufsetzen
* Lizenz- und Update-Hinweise fuer die eingebettete Abhaengigkeit sind lokal dokumentiert

## Arbeitspakete

1. **Einbettungsstrategie festziehen**
   * Ordnerstruktur fuer eingebettete Third-Party-Abhaengigkeiten festlegen
   * festhalten, ob RmlUi als Git-Submodule, Vendor-Snapshot oder andere projektlokale Quelle eingebunden wird
2. **Build-Integration vorbereiten**
   * CMake so erweitern, dass RmlUi aus dem eingebetteten Quellstand eingebunden werden kann
   * Zieldefinitionen, Includes und sichtbare Build-Optionen fuer die Folgeplaene vorbereiten
3. **Lizenz- und Pflegepfad festhalten**
   * Herkunft, Version/Pinning und Update-Pfad dokumentieren
   * benoetigte Lizenzdateien oder Referenzen fuer das Repo einplanen
4. **Uebergang zu Plan 02b sichern**
   * klar dokumentieren, welche technischen Annahmen Plan 02b ab hier uebernehmen darf
   * Plan 02b von projektlokalem RmlUi statt von Systempaketen ausgehen lassen

## Abschlusskriterien

* es gibt einen klar benannten projektlokalen Ort fuer RmlUi
* der Buildpfad fuer RmlUi ist in CMake vorgesehen
* `plans/02b-sdl2-und-rmlui-grafikintegration.md` setzt auf eingebettetes RmlUi auf
* die weitere Grafikintegration ist fuer RmlUi nicht mehr von `apt`-Paketen abhaengig

## Umsetzungsnotiz

Plan 02a wurde ueber einen projektlokalen Git-Submodule-Pfad umgesetzt:

* Pfad: `third_party/rmlui`
* Upstream: `https://github.com/mikke89/RmlUi.git`
* Pin: Release `6.2` (`2230d1a6e8e0848ed87a5761e2a5160b2a175ba4`)
* Initialisierung fuer frische Checkouts: `make deps`

Die Top-Level-CMake-Konfiguration bindet RmlUi jetzt ueber `add_subdirectory(...)` aus dem Repo selbst ein.
