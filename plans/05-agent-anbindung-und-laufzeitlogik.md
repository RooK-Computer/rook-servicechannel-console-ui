# Plan 05 - Agent-Anbindung und Laufzeitlogik

Status: done

## Ziel

Die UI mit der echten Aussenwelt verbinden: Agent-Port, IPC-Anbindung, Status-Mapping, Persistenz und reale Ablaufentscheidungen.

## Ergebnis nach Abschluss

* UI liest ihren Laufzeitstatus ausschliesslich ueber den RooK Agent
* Screenflow reagiert auf echten WLAN-, VPN- und Support-Status
* Persistenz fuer "nicht mehr anzeigen" ist vorhanden

## Arbeitspakete

1. **Agent-Port und Adapter**
   * Port fuer `GetStatus` und relevante Events definieren
   * konkrete IPC-Adapter anschliessen
2. **Status-Mapping**
   * `supportState`, `wifiState`, `vpnState`, `PinAssigned` und Fehlerereignisse in UI-Modelle uebersetzen
   * Session-Pruefung beim Start abbilden
3. **Laufzeit-Flow**
   * WLAN vorhanden -> VPN-Versuch
   * kein WLAN -> Setup-Flow
   * VPN-Fehler -> WLAN wechseln oder Abbrechen
   * Agent-/IPC-Verlust -> UI beendet sich
4. **Persistenz**
   * `.config`-Datei fuer UI-Einstellungen anbinden
   * "nicht mehr anzeigen" lesen und schreiben

## Hinweise fuer spaetere Umsetzung

* keine parallelen Wahrheiten neben dem Agent-Status einfuehren
* Fehlerbehandlung sichtbar, aber nicht technisch ueberladen halten

## Abschlusskriterien

* Startpfad und Screenflow folgen echtem Agent-Status
* die wichtigsten Agent-Ereignisse fuehren zu sichtbaren UI-Updates
* Persistenz der Welcome-Option funktioniert

## Umsetzungsstand

`05` ist jetzt umgesetzt:

* ein UI-seitiger Agent-Port und ein Unix-Domain-Socket-Adapter sind vorhanden
* der Produktpfad loest den Agent-Socket gemaess aktueller Spezifikation ueber `/etc/default/rook-agent` und `ROOK_AGENT_SOCKET_PATH` auf; fuer ungepackte Entwicklungsstarts gilt der User-Config-Fallback
* der produktive Startpfad liest Welcome-Persistenz und echten Agent-Status statt Mockwerten
* bei bestehendem WLAN kann der Flow direkt in den Support-/VPN-Pfad verzweigen
* Status-, WLAN-, Wait- und Fehler-Screens werden im Normalbetrieb mit Runtime-Daten aus Agent und Settings gespeist
* die Welcome-Option wird ueber `settings.json` unter `.config` gelesen und geschrieben
* der Produktpfad beendet sich kontrolliert, wenn der Agent-/IPC-Zugriff fehlt
