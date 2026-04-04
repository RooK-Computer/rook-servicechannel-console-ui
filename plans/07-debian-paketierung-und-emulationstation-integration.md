# Plan 07 - Debian-Paketierung und EmulationStation-Integration

Status: done

## Ziel

Die RooK UI als Debian-Paket mit `nfpm` auslieferbar machen und die Integration in RetroPie / EmulationStation fuer das Zielsystem sauber mitliefern.

## Ergebnis nach Abschluss

* es gibt einen verifizierbaren `nfpm`-Buildpfad fuer die UI-Paketierung
* die UI wird als separates Laufzeitpaket mit Binary, Launcher und Assets paketiert
* die EmulationStation-Anbindung liegt in einem separaten Integrationspaket
* die Installation ergaenzt ein fehlendes System `RooK` idempotent in der systemweiten EmulationStation-Konfiguration
* das System `RooK` enthaelt einen Startpunkt `Service`, der die UI startet

## Umsetzungsergebnis

1. **Paketgrenzen festgezogen**
   * `rook-console-ui` liefert Binary, Launcher und alle benoetigten Laufzeitressourcen
   * `rook-console-integration` liefert EmulationStation-Integration, Default-Konfiguration und Installationslogik
2. **Paketbuild eingefuehrt**
   * `Makefile` bietet jetzt `package`, `package-ui`, `package-integration` und `package-inspect`
   * wenn lokal kein `nfpm` vorhanden ist, nutzt der Build `go run ... nfpm`
3. **Paketierte Laufzeitpfade abgesichert**
   * die App kann Ressourcen ueber `ROOK_UI_RESOURCE_ROOT` bzw. den paketierten Standardpfad finden
   * der oeffentliche Launcher setzt den paketierten Ressourcenpfad explizit
4. **EmulationStation-Integration umgesetzt**
    * das Integrationspaket fuehrt ein idempotentes Maintainer-Skript aus
    * ein fehlendes `RooK`-System wird in der systemweiten EmulationStation-Konfiguration angelegt
    * vorhandene RooK-/Service-Eintraege werden nicht ueberschrieben
    * fuer das System `RooK` wird ein konservatives Theme-Snippet mit dem paketierten RooK-Logo angelegt
5. **Dokumentation nachgezogen**
   * lokales Konzept, Plan-Index, README und Implementierungsstatus spiegeln die neue Paketierungs- und Integrationslogik
