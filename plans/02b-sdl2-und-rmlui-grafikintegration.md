# Plan 02b - SDL2- und grafische RmlUi-Integration

Status: ready

## Ziel

Die bislang fehlende echte grafische Integration nachziehen: SDL2 fuer Fenster-/Render-Kontext und die bereits projektlokal eingebettete RmlUi-Basis fuer die tatsaechliche UI-Darstellung.

## Voraussetzungen

Dieser Plan darf nur bearbeitet werden, wenn die benoetigten Voraussetzungen erfuellt sind.

Erwartete Voraussetzungen:

* `plans/02a-rmlui-ins-projekt-einbetten.md` ist abgeschlossen
* SDL2-Entwicklungspakete sind im Environment verfuegbar

Wenn diese Voraussetzungen spaeter wieder nicht erfuellt sind:

* ist das ein **echter Blocker**
* es wird **kein** terminalbasierter oder anderer Ersatz als Planerfuellung gewertet
* der Nutzer installiert die benoetigten SDL2-Pakete nach

## Ergebnis nach Abschluss

* SDL2 ist als gemeinsame Runtime-/Plattformschicht fuer **KMS/DRM** und **X/Wayland** eingebunden
* RmlUi rendert die echte UI grafisch statt ueber den Terminal-Fallback
* Preview laeuft sichtbar grafisch ueber **denselben App-/Screen-Codepfad**
* die bestehende Screen-, Fokus- und Navigationslogik bleibt erhalten und wird nur an den echten Renderpfad angebunden

## Arbeitspakete

1. **Voraussetzungen verifizieren**
   * Abschluss von Plan 02a sicherstellen
   * SDL2-Dev-Pakete im Environment pruefen
   * nur bei Vollstaendigkeit mit dem Plan fortfahren
2. **CMake und Linkage**
   * `find_package` fuer SDL2 gegen die real verfuegbaren Pakete verifizieren
   * eingebettetes RmlUi sauber an die App anbinden
   * Includes, Linktargets und Buildpfade fuer SDL2/RmlUi sauber festziehen
3. **Gemeinsame SDL-Laufzeitpfade herstellen**
   * SDL-Lebenszyklus, Eventloop und Renderzyklus als gemeinsame Basis aufbauen
   * Entwicklungsbetrieb unter X/Wayland und Zielbetrieb ueber KMS/DRM sauber an dieselbe UI-Schicht anbinden
4. **Preview ohne Sonderpfad herstellen**
   * `rook-ui --preview <screen-id>` ueber dieselbe App-Shell und dieselbe Screen-Registry starten
   * Preview darf nur Datenquelle und Startzustand austauschen, nicht aber eigene Screen-Stubs oder eine zweite UI-Implementierung verwenden
5. **Grafischen Host fuer spaetere Screens vorbereiten**
   * die Render-/Document-Host-Struktur so vorbereiten, dass echte Screen-Module aus `screens/` spaeter direkt daran andocken

## Abschlusskriterien

* `rook-ui --preview welcome` nutzt dieselbe SDL-/RmlUi-App-Shell wie der Normalbetrieb
* Entwicklungsbetrieb unter X/Wayland und Zielpfad ueber KMS/DRM sind beide in der Architektur vorgesehen
* der Terminal-Fallback gilt danach nur noch als Not- oder Diagnosepfad, nicht als Erfuellung dieses Plans
