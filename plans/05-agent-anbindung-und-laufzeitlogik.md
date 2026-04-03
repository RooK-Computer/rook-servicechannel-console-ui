# Plan 05 - Agent-Anbindung und Laufzeitlogik

Status: ready

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
