# Plan 02 - Rendering, Theme und Preview

Status: rejected

## Ziel

Die UI visuell und technisch renderfaehig machen: SDL2/RmlUi-Integration, Theme-Tokens, Render-Backends und produktionsnaher Preview-Modus.

## Ergebnis nach Abschluss

* Render- und Preview-Schicht laeuft ueber die App-Shell
* Theme/Tokens fuer Farben, Typografie, Abstaende und Fokusstil sind zentral definiert
* Preview-Modus zeigt Screens gezielt per ID
* Escape beendet die Preview

## Arbeitspakete

1. **RmlUi-Integration**
   * Renderpfad fuer SDL2 und RmlUi herstellen
   * Ressourcensystem fuer Layouts, Fonts und Styles definieren
2. **Theme-System**
   * Farben, Typografie, Abstaende, Linienstaerken und Fokusstil als zentrale Tokens ablegen
   * ersten Typografie-Vorschlag aus dem Konzept uebernehmen
3. **Render-Backends**
   * Entwicklungsmodus fuer X/Wayland
   * Zielpfad fuer KMS/DRM sauber vorbereiten
4. **Preview-Modus**
   * `rook-ui --preview <screen-id>` an App-Shell anbinden
   * Screen-Preview-Registry in erster technischer Form einfuehren
   * Default-Preview-Zustand pro Screen vorsehen

## Hinweise fuer spaetere Umsetzung

* Preview bleibt ohne Agent-Interaktion und ohne automatische Screenwechsel
* Animationen muessen auch im Preview-Modus laufen

## Abschlusskriterien

* Theme zentral statt screenweise definiert
* mindestens ein echter UI-Screen ist im Preview-Modus sichtbar
* Escape beendet die Preview konsistent

## Rejection-Notiz

Dieser Plan gilt in seiner bisherigen Umsetzung als **nicht akzeptiert**.

Begruendung:

* die eigentliche SDL2-/RmlUi-Grafikintegration wurde **nicht** geliefert
* es gab **keine echte grafische Arbeit** im Sinne dieses Plans
* stattdessen wurde ein terminalbasierter Fallback-Renderer aufgebaut

Die dabei entstandenen Vorarbeiten bleiben technisch nuetzlich:

* Theme-Tokens
* Preview-Registry
* Backend-Erkennung

Sie zaehlen aber **nicht** als Erfuellung dieses Plans.

Der Ersatzpfad ist `plans/02b-sdl2-und-rmlui-grafikintegration.md`.
