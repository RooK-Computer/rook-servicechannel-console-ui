# RooK Console UI - Konzept

Status: In Arbeit

## Zweck des Dokuments

Dieses Dokument konkretisiert die lokale RooK-UI fuer die Konsole so weit, dass daraus spaeter ein belastbarer Implementierungsplan fuer dieses Repository abgeleitet werden kann.

Es beschreibt:

* feste Rahmenbedingungen aus der uebergeordneten Spezifikation,
* den geplanten Nutzerfluss durch die UI,
* offene Produkt- und UX-Entscheidungen,
* spaeter zu stabilisierende Detailregeln fuer Darstellung, Eingabe und Fehlerfaelle.

## Quellen und Einordnung

Dieses Dokument baut aktuell auf folgenden Artefakten auf:

* `spec/docs/architecture/servicechannel-concept.md`
* `spec/implementation/01-rook-ui-status.md`
* `spec/implementation/10-komponentenuebergreifender-entwicklungsplan.md`
* spaeter bei Bedarf: `spec/openapi/01-ui-agent-local-ipc.openapi.yaml`

## Bereits feststehende Rahmenbedingungen

Die folgenden Punkte gelten derzeit als gesetzt:

* Die UI ist die **lokale** Bedienoberflaeche auf der Konsole.
* Die UI ist fuer **Darstellung und Eingabe** zustaendig, nicht fuer Systemoperationen.
* Die UI spricht ausschliesslich mit dem **lokalen RooK Agent**.
* Der RooK Agent ist die **verbindliche Quelle fuer Laufzeitstatus**.
* Die UI laeuft **fullscreen direkt unter Linux**, ohne X oder Wayland.
* Die UI muss im regulaeren Betrieb **vollstaendig per Gamepad** bedienbar sein.
* Fuer WLAN-Zugangsdaten wird ein **On-Screen-Keyboard** benoetigt.
* Die UI muss mindestens folgende Zustaende sichtbar machen:
  * WLAN
  * VPN
  * Support-Modus
  * Support-PIN

## Abgrenzung

Nicht Aufgabe der UI:

* WLAN direkt konfigurieren oder verwalten
* VPN direkt starten oder beenden
* mit dem zentralen Backend sprechen
* sicherheits- oder systemnahe Entscheidungen eigenstaendig treffen

Diese Verantwortung liegt beim RooK Agent bzw. bei den nachgelagerten Systemkomponenten.

## Zielbild der lokalen UI

Die UI soll vor Ort in wenigen, klaren Schritten durch den Support-Ablauf fuehren:

1. den Zweck des Features auf einem Begruessungsbildschirm erklaeren,
2. nach dem Einstieg den aktuellen Sitzungszustand pruefen,
3. bei Bedarf den eigentlichen Verbindungsaufbau schrittweise fuehren,
4. den ausgehandelten PIN klar sichtbar anzeigen,
5. waehrend der aktiven Sitzung einen einfachen Statusblick bieten,
6. das Trennen der Serviceverbindung erlauben und den Rueckbau sichtbar machen.

## Vorlaeufiger Hauptfluss

Der folgende Ablauf ist aktuell die bevorzugte Arbeitsrichtung und muss noch weiter konkretisiert werden:

1. **Begruessungsbildschirm**
   * Titel des Screens: **"RooK Service"**
   * textlastige Einfuehrung in Zweck und Nutzen des RooK-Servicekanals
   * nach dem Titel beginnt direkt der eigentliche Langtext, ohne separate Unterzeile
   * der Erklaertext muss **scrollbar** sein
   * der konkrete Text wird spaeter ausgearbeitet
   * fuer diesen Text ist im Projekt eine **separate Textdatei** vorzusehen, damit das Team ihn leicht nachtraeglich aendern kann
   * es gibt eine Option **"nicht mehr anzeigen"**
   * es gibt einen klaren **"Weiter"**-Button
   * es gibt einen **"Beenden"**-Button
2. **Session-Pruefung nach dem Einstieg**
   * nach **"Weiter"** prueft die Anwendung, ob bereits eine laufende Service-Session existiert
   * bei vorhandener laufender Session geht es direkt zum Statusbildschirm
   * ohne laufende Session startet der Einrichtungsdurchlauf
3. **Gefuehrter Verbindungsaufbau**
   * zuerst wird geprueft, ob bereits eine WLAN-Verbindung besteht
   * falls eine WLAN-Verbindung besteht, wird die WLAN-Einrichtung uebersprungen
   * falls keine WLAN-Verbindung besteht, wird ein WLAN-Einrichtungsflow durchlaufen
   * anschliessend wird der VPN-Verbindungsaufbau versucht
   * bei erfolgreicher VPN-Verbindung wird eine Service-Session gestartet
   * danach geht es zum Statusbildschirm
4. **Statusbildschirm / aktive Sitzung**
   * der Screen traegt den Titel **"RooK Service"**
   * PIN prominent und aus Distanz lesbar anzeigen
   * ueber der PIN steht der Labeltext **"Service-PIN"**
   * der Labeltext ist klar sichtbar, aber deutlich kleiner als die PIN
   * die PIN ist das dominante Hauptelement des Bildschirms
   * die PIN bleibt **rein typografisch** und bekommt **keinen eigenen Kasten**
   * neben der PIN werden **keine weiteren Laufzeitstatus-Texte** dauerhaft angezeigt
   * der Bildschirm enthaelt eine **Trennen**-Aktion zum sauberen Beenden der aktiven Service-Session
   * falls gerade eine aktive Service-Session mit sichtbarer PIN besteht, fuehrt **"Trennen"** zunaechst in einen Bestaetigungsdialog
   * die **Trennen**-Aktion baut den temporaeren Support-Zustand ab
   * der Bildschirm enthaelt eine **Beenden**-Aktion zum Schliessen der UI
5. **Beenden / Cleanup**
   * Support-Modus beenden
   * Rueckbau des temporaeren Zustands anzeigen
   * wieder zum Begruessungsbildschirm zurueckkehren

## Konkretisierte Einstiegslogik

Der Einstieg in die Anwendung folgt derzeit diesem Modell:

1. Beim Start erscheint zunaechst der **Willkommensbildschirm**.
2. Dort wird der Servicechannel in Textform erklaert.
3. Der Textbereich muss scrollbar sein, damit auch laengere spaetere Produkttexte darstellbar sind.
4. Die Scrollbarkeit soll sichtbar und erklaert sein:
   * durch eine **sichtbare Scrollleiste**
   * durch den Hinweistext **"Mit Stick oder Schultertasten scrollen"**
5. Auf dem Bildschirm gibt es:
   * eine klare Checkbox mit der Beschriftung **"Beim naechsten Start nicht mehr anzeigen"**
   * einen klaren **"Weiter"**-Button
   * einen **"Beenden"**-Button
6. Nach Aktivierung von **"Weiter"** wird der aktuelle Sitzungszustand beim Agent geprueft.
7. Falls bereits eine laufende Service-Session existiert, ueberspringt die UI den Einrichtungsdurchlauf und wechselt direkt in den **Statusbildschirm**.
8. Falls keine laufende Service-Session existiert, startet die UI den **Service-Session-Einrichtungsdurchlauf**.

Die Auswahl **"nicht mehr anzeigen"** soll dauerhaft gespeichert werden:

* in einer Datei unter **`.config`** im Homeverzeichnis des aktuellen Users
* sie gilt dauerhaft, bis sie spaeter explizit zurueckgesetzt wird
* in der ersten Version gibt es **keine UI-Funktion zum Zuruecksetzen**
* ein Zuruecksetzen erfolgt vorerst nur **manuell ueber die Konfigurationsdatei**

## Konkretisierter Einrichtungsworkflow

Der Einrichtungsworkflow folgt aktuell diesem Ablauf:

1. Zu Beginn wird geprueft, ob bereits eine **WLAN-Verbindung** besteht.
2. Falls bereits WLAN besteht, wird die Eingabe von WLAN-Verbindungsdaten vollstaendig uebersprungen.
3. In diesem Fall versucht die UI direkt den **VPN-Verbindungsaufbau**.
4. Falls noch **kein WLAN** besteht, fuehrt die UI durch einen separaten WLAN-Einrichtungsflow.
5. Der WLAN-Einrichtungsflow besteht mindestens aus folgenden Screens:
   * Screen mit **verfuegbaren WLAN-Netzen**
   * Screen zur **Passworteingabe**
   * **Wartedialogscreen** fuer die Einrichtung der WLAN-Verbindung
6. Nach erfolgreicher WLAN-Einrichtung wird der **VPN-Verbindungsaufbau** gestartet.
7. Fuer den VPN-Aufbau gibt es einen eigenen **Wartedialogscreen**.
8. Wenn die VPN-Verbindung innerhalb einer gewissen Zeit zustande kommt, wird eine **Service-Session gestartet**.
9. Nach erfolgreichem Start der Service-Session wechselt die UI in den **Statusbildschirm**.
10. Wenn der VPN-Verbindungsaufbau nach einiger Zeit **nicht zustande kommt**, erscheint ein eigener Fehler- bzw. Statusscreen.
11. Auf diesem Screen wird der Nutzer gefragt, ob er:
   * **abbrechen und beenden** moechte oder
   * das **WLAN wechseln** moechte
12. Mit der Option **WLAN wechseln** kehrt der Nutzer in den WLAN-Einrichtungsflow zurueck und der WLAN-Scan wird neu gestartet.
13. Mit der Option **abbrechen und beenden** wird der laufende Einrichtungsversuch verlassen und die UI geschlossen.

## Vorlaeufige Screenliste fuer den Einrichtungsflow

Fuer den Service-Session-Einrichtungsdurchlauf sind derzeit mindestens diese Screens vorgesehen:

1. Screen mit verfuegbaren **WLAN-Netzen**
2. Screen zur **WLAN-Passworteingabe**
3. **Wartedialogscreen** fuer die WLAN-Einrichtung
4. **Wartedialogscreen** fuer den VPN-Verbindungsaufbau
5. Screen fuer den Fall, dass der **VPN-Aufbau nicht rechtzeitig zustande kommt**

## Paketierung und Zielsystemintegration

Fuer den Zielbetrieb auf der Konsole gelten fuer die RooK UI jetzt folgende Festlegungen:

* die UI wird als **Debian-Paket mit `nfpm`** ausgeliefert
* die Paketierung wird in **zwei getrennte Pakete** aufgeteilt:
  * **`rook-console-ui`** fuer Binary, Laufzeitressourcen und den oeffentlichen Launcher
  * **`rook-console-integration`** fuer RetroPie-/EmulationStation-Integration
* das UI-Paket enthaelt:
  * das UI-Binary
  * alle benoetigten Laufzeitressourcen wie Texte, Fonts und sonstige Assets
  * den oeffentlichen Starter **`/usr/bin/rook-ui`**
* die paketierten UI-Ressourcen liegen unter **`/usr/share/rook-console-ui/resources`**
* das Integrationspaket enthaelt:
  * das EmulationStation-Startskript **`Service.sh`**
  * die Installationslogik fuer die EmulationStation-Anbindung
  * die Default-Konfiguration fuer den Pfad zur systemweiten EmulationStation-Konfiguration
  * ein konservatives RooK-Theme-Snippet fuer die Systemgrafik

Fuer die EmulationStation-Integration gilt:

* die Integration arbeitet gegen die **systemweite EmulationStation-Konfiguration des Zielimages**
* wenn dort noch kein System **`RooK`** vorhanden ist, wird es bei der Installation angelegt
* das Integrationspaket legt dabei einen Systemeintrag mit:
  * technischem Namen **`rook`**
  * Anzeigenamen **`RooK`**
  * paketiertem ROM-/Launcher-Pfad unter **`/usr/share/rook-console-ui/emulationstation/roms`**
  * Startkommando **`/bin/sh %ROM%`**
* der sichtbare Startpunkt innerhalb dieses Systems heisst **`Service`**
* die Systemgrafik fuer `RooK` verwendet das paketierte Logo **`rook_logo_v1-0-0_name_bw.svg`**
* standardmaessig wird dafuer ein RooK-Theme-Snippet unter **`/etc/emulationstation/themes/carbon/rook/theme.xml`** angelegt
* wenn bereits ein System **`RooK`** existiert, werden **nur fehlende Eintraege ergaenzt**
* bereits vorhandene RooK-/Service-Eintraege werden **nicht automatisch normalisiert oder ueberschrieben**
* ein bereits vorhandenes RooK-Theme-Snippet wird ebenfalls **nicht automatisch ueberschrieben**

## WLAN-Netzauswahl

Fuer die erste Version der WLAN-Netzauswahl gilt:

* Titel des Screens: **"Mit WLAN verbinden"**
* der WLAN-Scan wird als **laufender Prozess** behandelt
* nach dem Start des Scans koennen nach und nach weitere Netze erscheinen
* der Scan endet erst wirklich, wenn der Nutzer sich fuer ein WLAN entscheidet
* die Liste der verfuegbaren Netze wird **alphabetisch nach SSID** sortiert
* neue Netze werden waehrend des laufenden Scans **live alphabetisch einsortiert**
* die aktuell fokussierte Auswahl muss dabei stabil bleiben und darf sich durch neu eingefuegte Eintraege nicht verschieben
* ein aktuell aktives WLAN wird **nicht gesondert markiert**
* die WLAN-Liste besitzt eine sichtbare Aktion **"Zurueck"**
* **"Zurueck"** fuehrt zum vorherigen Screen, wenn ein sinnvoller Ruecksprungpunkt im Stack vorhanden ist
* falls kein sinnvoller Ruecksprungpunkt vorhanden ist, beendet **"Zurueck"** die App
* wenn der WLAN-Scan keine Netze liefert, zeigt die UI einen **Leerstaat**
* der Hinweistext des Leerstaats lautet: **"Keine WLAN-Netze gefunden"**
* dieser Leerstaat bietet nur **"Abbrechen und Beenden"**
* wenn zwischen Scan-Start und erstem gefundenen WLAN **mehr als 30 Sekunden** vergehen:
  * wird der laufende Scan automatisch beendet
  * und automatisch neu gestartet

## Warte-Screens

Die Warte-Screens fuer WLAN-Einrichtung und VPN-Aufbau sollen bewusst knapp gehalten sein.

Der aktuelle Zielzustand ist:

* Warte-Screens zeigen grundsaetzlich immer **Titel plus Statustext**
* ein **animierter Spinner**
* ein **kurzer Statustext**
* Titel des WLAN-Warte-Screens: **"Mit WLAN verbinden"**
* fuer den WLAN-Warte-Screen lautet dieser Text: **"Verbindung zum WLAN wird hergestellt"**
* Titel des VPN-Warte-Screens: **"Sichere Verbindung"**
* fuer den VPN-Warte-Screen lautet dieser Text: **"Sichere Verbindung wird aufgebaut"**

## VPN-Fehlerscreen

Der VPN-Fehlerscreen soll die Situation bewusst knapp und sachlich formulieren.

Der aktuelle Zielzustand ist:

* Titel des Screens: **"Verbindung fehlgeschlagen"**
* kurze Meldung: **"Verbindung konnte nicht aufgebaut werden"**
* keine stark technische oder diagnostische Formulierung in der Primaerdarstellung
* Wechsel auf diesen Screen nach **60 Sekunden** ohne erfolgreichen VPN-Aufbau

## Fehler-Screens als Grundmuster

Fehler-Screens sollen moeglichst einheitlich aufgebaut sein.

Der aktuelle Zielzustand ist:

* Fehler-Screens zeigen grundsaetzlich immer **Titel plus Fehlermeldung**
* kurze sachliche Fehlermeldung
* keine technische Primaerdarstellung
* klare Folgeentscheidung fuer den Nutzer
* sofern kein sinnvollerer Rueckweg existiert, ist **"Abbrechen und Beenden"** Teil des Grundmusters

Dieses Grundmuster gilt nicht nur fuer den VPN-Fehlerfall, sondern auch fuer fehlgeschlagenen WLAN-Aufbau.

## WLAN-Fehlerscreen

Fuer den fehlgeschlagenen WLAN-Aufbau soll ebenfalls ein eigener Fehler-Screen verwendet werden.

Der aktuelle Zielzustand ist:

* Titel des Screens: **"WLAN-Verbindung fehlgeschlagen"**
* gleicher Grundstil wie beim VPN-Fehlerscreen
* Wechsel auf diesen Screen nach **30 Sekunden** ohne erfolgreichen WLAN-Aufbau
* kurze Meldung: **"Verbindung zum WLAN konnte nicht hergestellt werden"**
* Aktionen:
  * **"Zurueck zur WLAN-Liste"**
  * **"Abbrechen und Beenden"**

## Aus Spezifikation bereits ableitbare Mindest-Statussicht

Die UI soll fuer Laufzeitstatus keine eigenen Annahmen erfinden, sondern sich an den bereits vorhandenen Spezifikationsartefakten orientieren.

Der derzeit belastbare Mindeststand aus `spec/schemas/local-ipc/01-ui-agent-local-ipc-message-catalog.md`,
`spec/openapi/01-ui-agent-local-ipc.openapi.yaml` und
`spec/models/events/01-ui-agent-local-ipc-events.md` ist:

* `GetStatus` liefert mindestens:
  * `supportActive`
  * `supportState`
  * `wifiState`
  * `vpnState`
* zusaetzlich liefert der Status:
  * `anyWifiActive`
  * `supportWifiActive`
  * optional `activeWifiConnection`
* `wifiState` beschreibt den vom RooK-Agent gefuehrten **Support-WLAN-Zustand**
* damit kann die UI zwischen allgemeiner WLAN-Konnektivitaet des Hosts und dem RooK-Support-WLAN unterscheiden

Aus den Event-Spezifikationen ist derzeit mindestens belastbar:

* `WifiConnectionStateChanged` mit `state = connected | disconnected`
* `VpnStateChanged` mit `state = connected | disconnected`
* `SupportStateChanged` mit `state = idle | online | online+vpnup | servicemode`
* `PinAssigned` mit mindestens `pin`
* `ErrorRaised` mit mindestens `code` und `message`

Fehlende Details bleiben offen und sollen erst erweitert werden, wenn sie in `spec/` konkretisiert sind.

## Architektur-Guardrails fuer die Implementierung

Damit die Anwendung spaeter leicht erweitert werden kann, soll die Implementierung nicht screenweise "zusammenwachsen", sondern von Beginn an mit klaren Schichten aufgebaut werden.

Die Zielrichtung fuer die Architektur ist:

* **App-Shell**
  * Start, SDL-Lebenszyklus, Main Loop, Konfiguration, Umschaltung zwischen Normalbetrieb und Preview-/Testmodus
  * Umschaltung zwischen Zieldevice-Rendering und Entwicklungs-Rendering
  * SDL2 bleibt die gemeinsame Plattformschicht fuer **KMS/DRM auf dem Zielgeraet** und **X/Wayland im Entwicklungsbetrieb**
  * Preview und Normalbetrieb starten dieselbe Screen-Registry; sie unterscheiden sich nur in Datenquelle, Start-Intent und Laufzeitumgebung
* **Navigation und Flow-Steuerung**
  * die konkrete Entscheidung ueber den **naechsten Screen** liegt primaer beim aktuellen Screen bzw. dessen Screen-Logik
  * wiederverwendbare Komponenten wie Fehler-Screens oder Dialoge navigieren **nicht selbst**
  * stattdessen liefern sie nur wohldefinierte Actions/Ereignisse an den umgebenden Screen zurueck
  * als Leitmodell wird ein **Intent-Konzept aehnlich Android** verwendet:
    * ein Screen erzeugt einen Intent fuer den naechsten Schritt
    * der Intent enthaelt das Ziel sowie optionale Parameter
    * die **App-Shell** fuehrt diesen Intent aus
  * fuer definierte Rueckspruenge wird ein **app-weiter Stack** aus Screens bzw. Start-Intents gepflegt
* **Screen-Modelle statt verstreuter UI-Logik**
  * jeder Screen bekommt ein klar abgegrenztes View-Model bzw. Screen-Model
  * Screens rendern auf Basis dieses Modells und loesen nur wohldefinierte Actions aus
* **Ports und Adapter fuer Aussenwelt**
  * der Agent-Zugriff haengt hinter einer Interface-Schicht
  * die echte IPC-Implementierung und Test-/Preview-Implementierungen muessen austauschbar sein
* **Wiederverwendbare UI-Bausteine**
  * Buttons, Button-Zeilen, Scroll-Container, Listen, Leerstaaten, Warte-Dialoge, Fehler-Screens und Dialoge sollen als gemeinsame Komponenten gebaut werden
* **Design-Tokens**
  * Abstaende, Typografie, Farben, Fokus-Stile und Groessen nicht pro Screen neu definieren

## Empfohlene Code-Struktur

Die genaue Ordnerstruktur ist spaeter festzuziehen, aber das technische Zielbild ist bereits klar:

* `app/`
  * Programmstart, Main Loop, Screen-Registry, Navigation per Intent-Ausfuehrung, Moduswahl, Auswahl des Render-Backends
  * haelt globale UI-Zustaende wie Theme, Preview-Modus und App-Context
  * nutzt eine Registry mit stabilen **String-Screen-IDs**
* `domain/` oder `flow/`
  * UI-seitige Ablaufregeln und Zustandsuebersetzungen aus Agent-Status in Screen-Modelle
  * vermittelt zwischen Screen-Logik und Agent-/Service-Ports
* `screens/`
  * je Screen eine klar abgegrenzte Einheit aus Logik und Darstellung
  * alle produktiven Hauptscreens werden als **echte Screen-Module in `screens/`** implementiert
  * Screens geben Folgeaktionen als **Intent** an die App-Shell zurueck
  * innerhalb eines Screens sind **Logik** und **View** klar getrennt, bleiben aber pro Screen zusammen organisiert
  * kleine modale Dialoge werden innerhalb des aktuellen Screens als Overlay modelliert
  * der Preview-Modus instanziiert **dieselben Screen-Module** wie der Normalbetrieb; zusaetzliche Preview-Screens oder Stub-Screens sind nicht vorgesehen
* `components/`
  * wiederverwendbare UI-Bausteine
  * eher generische Praesentations- und Interaktionsbausteine mit moeglichst wenig eigener Logik
* `services/` oder `ports/`
  * Schnittstellen fuer Agent, Preview-Daten, Persistenz von UI-Einstellungen
  * werden von Screens nicht direkt, sondern ueber Flow-/Service-Logik genutzt
* `adapters/`
  * konkrete Implementierungen fuer IPC, Dateizugriff und Preview-Szenarien
* `theme/`
  * Farben, Typografie, Abstaende, Fokus- und Layoutkonstanten
* `context/` oder Teil der App-Shell
  * kleiner globaler App-Context fuer wenige zentrale UI-Zustaende

## Wiederverwendungs-Guardrails

Zur Vermeidung von dupliziertem Screen-Code sollen folgende Regeln gelten:

* keine direkten Agent-Aufrufe aus Render-Code
* keine Screen-spezifischen Sonderimplementierungen fuer Standarddialoge, wenn das gleiche Muster bereits existiert
* Fehler-Screens, Warte-Screens und einfache Entscheidungsdialoge werden aus gemeinsamen Basiskomponenten aufgebaut
* der WLAN-Listen-Screen und das On-Screen-Keyboard sollen auf gemeinsamen Fokus- und Listen-/Grid-Mechaniken aufsetzen
* Design- und Eingaberegeln werden an zentraler Stelle definiert, nicht pro Screen neu
* wiederverwendbare Komponenten duerfen **keine eigene Navigation** fest verdrahten
* stattdessen meldet die Komponente nur eine Aktion, und der umgebende Screen entscheidet ueber den naechsten Screen
* der Screen selbst fuehrt die Navigation nicht direkt aus, sondern gibt einen **Intent** an die App-Shell zurueck
* wiederverwendbare Komponenten bleiben moeglichst **praesentational** und melden Events nach oben
* globale UI-Zustaende werden zentral in App-Shell bzw. kleinem App-Context gehalten
* kleine Bestaetigungsdialoge und modale Entscheidungen werden bevorzugt als Overlay innerhalb des aktuellen Screens modelliert
* Screens sprechen Agent-Funktionalitaet nicht direkt an, sondern ueber einen dazwischenliegenden Flow-/Service-Layer

## Design-Guardrails

Neben der technischen Architektur braucht die UI auch eine konsistente visuelle Leitplanke.

Vor der Implementierung muessen deshalb auch die grundlegenden Design-Guardrails festgezogen werden, insbesondere:

* Farbwelt und Kontrastprinzipien
* Typografie und Font-Auswahl
* Groessenverhaeltnisse fuer Titel, Fliesstext, Buttons und PIN
* Abstands- und Rasterlogik
* Fokusdarstellung fuer Gamepad-Navigation
* allgemeine Designsprache:
  * ruhig vs. technisch
  * sachlich vs. freundlich
  * flach vs. stark konturiert
* Regeln fuer Icons, Spinner, Scrollleisten und Dialoge
* verbindliche Design-Tokens fuer Farben, Typografie, Abstaende, Radien und Linienstaerken

Die bislang festgezogene gestalterische Richtung ist:

* visuelle Orientierung an der **8-Bit-/Retro-Spiel-Aesthetik** des Gesamtprojekts
* moegliche Referenz: **RetroPie / EmulationStation**
* trotzdem keine verspielte oder ueberladene Richtung
* die UI soll weiterhin **sachlich, funktional und ruhig** wirken
* die Farben **Lila `#9000ff`** und **Gelb `#ffdc00`** sind verbindliche Marken- bzw. Leittfarben der UI
* **Gelb `#ffdc00`** soll als **Hintergrundfarbe** verwendet werden
* das Gelb ist als **dominanter Hintergrund fast ueberall** vorgesehen
* als Open-Source-Schriftart wird durchgaengig **JetBrains Mono** verwendet
* fokussierte Elemente werden mit einer **kraeftigen lila Kontur** hervorgehoben
* die Formensprache ist **kantig, klar konturiert und retro-kompatibel**
* Standardtext und Standardlinien werden in **sehr dunklem Schwarz oder Dunkelgrau** dargestellt
* Icons und Spinner sind **pixelig/retro-inspiriert**
* dabei duerfen sie etwas kreativer und spielnäher wirken, solange sie klar lesbar bleiben
* Layouts folgen einem **klaren groben Raster** mit **grosszuegigen Abstaenden**
* Hauptbuttons sind **gefuellte Flaechen** mit **starker Kontur** und klarer Lesbarkeit
* Primaeraktionen erscheinen als **dunkle Buttons**
* **Lila `#9000ff`** wird primaer fuer **Fokus und starke Hervorhebung** verwendet, nicht als Standard-Buttonflaeche
* Dialoge und Fehlerboxen erscheinen als **klar abgegrenzte dunkle Panels** mit **starker Kontur**
* Scrollleisten sind **sichtbar und klar konturiert**, aber nicht lauter als der eigentliche Inhalt
* Ecken sind **klar kantig**; Radien gibt es **nicht oder nur minimal**
* Animationen duerfen **deutlich spieliger und auffaelliger** sein als das statische Layout
* diese spieligere Animation darf auch bei **Screenwechseln** und **groesseren Zustandswechseln** sichtbar werden
* der dominante gelbe Hintergrund darf eine **leichte retroartige Textur oder ein Pattern** erhalten
* Typografiehierarchie:
  * **sehr grosse PIN**
  * **grosse Buttons**
  * **gut lesbarer Fliesstext**
  * moeglichst **wenig kleine Schrift**

## Erster Typografie-Vorschlag fuer 1080p-TV-Ausgabe

Als erste Arbeitsbasis fuer eine Anzeige auf einem Wohnzimmer-Fernseher in **1920x1080** wird folgende Groessenordnung vorgeschlagen:

* Screen-Titel: **56 px**
* Dialogtitel: **44 px**
* Fliesstext / Haupttext: **32 px**
* Hilfstexte / kleine Hinweise: **24 px**
* Button-Beschriftungen: **36 px**
* Label ueber der PIN: **40 px**
* PIN-Ziffern: **240 px**

Diese Werte sind als erste Arbeitsbasis fuer Implementierung und Preview gedacht und koennen bei der Umsetzung noch feinjustiert werden.

## Preview- und Testmodus ohne laufenden Agent

Die UI soll gezielt auch **ohne laufenden RooK Agent** reviewbar und entwickelbar sein.

Der dafuer vorgesehene Entwicklungsmodus soll folgende Faehigkeiten haben:

* gezielter Start einzelner Screens
* reproduzierbare Review-Zustaende fuer Design- und Produktabnahmen
* keine Abhaengigkeit von echtem WLAN-, VPN- oder Session-Lifecycle
* keine Agent-Interaktion
* keine ablaufenden Timer oder automatischen Screenwechsel
* Animationen sollen trotzdem laufen

## Render-Backends fuer Produktiv- und Entwicklungsmodus

Die UI benoetigt zwei klar getrennte Laufzeitpfade fuer Rendering und Fenstersystem:

Dabei gilt:

* **SDL2** ist die gemeinsame Plattformschicht fuer beide Modi
* darauf werden getrennte Render-/System-Backends fuer **Entwicklung** und **Zielgeraet** angebunden
* diese Backends muessen mit **RmlUi** zusammenspielen, ohne die UI-Schicht selbst zu verzweigen

* **Zielgeraet / Produktivbetrieb**
  * direkte Nutzung des Linux-Renderpfads ueber **SDL2 mit KMS/DRM**
  * kein X
  * kein Wayland
* **Entwicklung und Testing**
  * Nutzung der vorhandenen Fenster-/Display-Integration unter **Wayland oder X**
  * dieser Pfad dient der lokalen Entwicklung, dem Review einzelner Screens und dem Testen von Animationen

Der Preview-Modus soll bewusst einfach aufrufbar sein.

Mindestanforderung:

* beim Start wird die **ID des Screens als Argument** uebergeben
* der angegebene Screen wird direkt angezeigt
* ein Druck auf **Escape** beendet die Anwendung wieder
* pro Screen gibt es in der ersten Version genau **einen Default-Preview-Zustand**
* bevorzugte CLI-Form: **`rook-ui --preview <screen-id>`**

Die konkrete CLI-Syntax kann spaeter noch verfeinert werden; fachlich ist aber gesetzt:

* es muss eine **Screen-Registry** geben
* die UI muss ohne echten Agent mit **Preview-Daten** startbar sein
* der Preview-Modus zeigt einen einzelnen Screen gezielt an
* Preview-Screens laufen ohne echte Zeitablauflogik, aber mit aktiven Animationen
* Preview-Daten werden in der ersten Version **fest im Code** ueber eine **Scenario-Registry fuer echte Screens** definiert
* alle produktiven **Hauptscreens** muessen direkt im Preview-Modus startbar sein
* der Preview-Modus darf **keinen zweiten UI-Codepfad** fuer dieselben Screens aufbauen

## Architektur fuer Preview-Daten

Damit Preview und Produktivbetrieb denselben UI-Code nutzen, soll die Datenversorgung austauschbar aufgebaut werden.

Bevorzugtes Modell:

* gemeinsames Interface fuer die UI-Datenquelle
* eine Implementierung gegen echte Agent-IPC
* eine Implementierung gegen **fest im Code definierte Preview-Szenarien** fuer gezielt startbare Screens

Dadurch kann derselbe Screen entweder:

* mit echtem Agent-Status
* oder mit einem fest vorgegebenen Review-Zustand ohne Agent und ohne laufende Timer

gestartet werden, ohne dass pro Screen ein zweiter Sonderpfad gebaut werden muss.

Konkret bedeutet das:

* `rook-ui --preview <screen-id>` startet die normale SDL-basierte App-Shell
* die Screen-ID wird gegen dieselbe Screen-Registry wie im Produktivbetrieb aufgeloest
* der Unterschied liegt nur in den injizierten Preview-Daten bzw. im Startzustand
* Renderpfad, Fokuslogik, Screen-Komposition und Screen-Code bleiben identisch

## Layout-Strategie

Fuer die Layout-Entwicklung soll nach aktuellem Stand **keine eigene Layout-Engine** gebaut werden.

Die festgezogene Richtung ist:

* eine vorhandene Layout-Loesung einsetzen
* eigene Arbeit auf Screen-Komposition, Fokuslogik, Gamepad-Navigation und Projekt-spezifische Komponenten konzentrieren
* **RmlUi** ist die **Primaerbibliothek** fuer Layout und UI-Dokumente

Aktuell zeichnen sich diese Optionen ab:

### 1. RmlUi als festgelegte Primaerloesung

RmlUi ist eine C++-UI-Bibliothek mit HTML-/CSS-aehnlichem Ansatz, eigener Layout-Engine, Eventsystem und leichter Integration in Echtzeit- bzw. Fullscreen-Anwendungen.

Dafuer spricht im Kontext dieses Projekts:

* Layouts und Screen-Strukturen bleiben vergleichsweise leicht lesbar und pflegbar
* Styling und Wiederverwendung koennen zentral organisiert werden
* die Bibliothek ist eher fuer echte Produktoberflaechen als fuer reine Debug-Tools gedacht
* sie passt grundsaetzlich zu C++- und Echtzeit-/SDL-nahen Einsatzszenarien
* sie bringt eine klare Renderer-Abstraktion mit und laesst sich damit an Produktiv- und Entwicklungs-Backends anbinden
* Markup-, Styling- und Hot-Reload-Potenzial passen gut zum Preview- und Review-Workflow

### 2. Yoga nur als Fallback-Option

Yoga ist eine performante embeddable Flexbox-Layout-Engine fuer C++.

Dagegen spricht als Primaerwahl fuer dieses Projekt:

* Yoga loest nur das Layoutproblem
* Widgets, Fokus, Styling, Textkomposition und Interaktion muessten weitgehend selbst gebaut werden

Yoga bleibt damit eher eine technische Fallback-Option, falls eine hoeherwertige UI-Schicht verworfen wird.

### 3. Dear ImGui nicht als Primaer-UI fuer das Produkt

Dear ImGui ist stark fuer Entwickler-Tools, Debug-UIs und schnelle Iteration ausgelegt.

Fuer dieses Projekt ist es nach aktuellem Stand **nicht** die bevorzugte Basis fuer die Endnutzeroberflaeche, weil:

* die Bibliothek sich selbst klar eher als Tool-/Debug-UI positioniert
* wir eine langlebige, ruhige Produktoberflaeche statt einer typischen Tool-Oberflaeche brauchen

Ein Einsatz fuer interne Debug- oder Review-Werkzeuge bleibt davon unberuehrt.

## On-Screen-Keyboard-Strategie

Fuer das On-Screen-Keyboard ist nach aktuellem Stand **keine offensichtliche reife Standardbibliothek** erkennbar, die direkt zu diesem Projekt passt.

Deshalb ist die derzeit bevorzugte Richtung:

* das On-Screen-Keyboard als **eigene UI-Komponente** zu bauen
* dabei aber **nicht** als Sonderfall ausserhalb der restlichen UI
* stattdessen auf denselben Fokus-, Grid-, Button- und Layout-Bausteinen wie der Rest der Anwendung

Damit bleibt das Keyboard:

* gamepad-gerecht
* QWERTZ-/Shift-/Alt-/Caps-Lock-faehig
* im Preview-Modus einzeln reviewbar
* spaeter layoutseitig austausch- oder erweiterbar

## Noch zu konkretisierende UX-Bereiche

Fuer das Produktkonzept sind nur noch wenige Punkte offen:

### 1. Inhalt und spaetere Erweiterungen

* konkreter Inhalt des Willkommenstextes
* konkrete Lokalisierungsstrategie fuer spaetere Sprachversionen

### 2. Feinschliff in der sichtbaren UI

* genaue Reihenfolge sichtbarer Aktionen auf Screens und Dialogen

## Im Implementierungsplan weiter auszuarbeiten

Die folgenden Punkte gelten nicht mehr als offene Produktentscheidung, sondern werden im naechsten Schritt im Implementierungsplan technisch konkretisiert:

* genaue Stack-Regeln fuer Rueckspruenge und Intent-Verlauf
* genaue Form der Intent-Parameter pro Screen
* genaue Fokuslogik, insbesondere fuer Keyboard-Grid und dynamische Listen
* genaue technische Ausgestaltung der fest im Code definierten Preview-Registry
* genaue Trennung zwischen Screen-Logik, Komponentenbibliothek und Agent-Adapter
* konkreter interner Zuschnitt einzelner Screen-Module
* konkrete Form, in der Komponenten Actions an den Screen zurueckmelden
* wie Screen-Logik an Agent-Port bzw. Flow-/Service-Layer angebunden wird

## Offene Entscheidungen

Die folgenden Punkte sind noch nicht entschieden und werden in den naechsten Gespraechen geklaert:

* konkrete Lokalisierungsstrategie fuer spaetere Sprachversionen
* konkreter Inhalt des Willkommenstextes

## Entscheidungslog

1. Erste feste Produktentscheidung:
   * primaere Zielaufloesung: **1920x1080**
   * Orientierung: **Querformat**
   * **keine** verbindliche Fallback-Aufloesung
2. Bevorzugte UI-Grundform:
   * **hybrider Aufbau**
   * textlastiger Begruessungsbildschirm
   * gefuehrter Schritt-fuer-Schritt-Flow fuer den Aufbau der Service-Verbindung
   * separater Statusbildschirm fuer aktive Verbindung mit PIN-Anzeige und Trennen-Aktion
3. Eingabe-Hardware:
   * geplant wird mit einem **vollen Standard-Controller**
   * Shoulder-Buttons und Trigger duerfen als regulaere Eingaben genutzt werden
4. Grundlegendes Button-Schema:
   * **A** = bestaetigen / weiter
   * **B** = zurueck / abbrechen
   * **X** = Sonderaktion
   * fuer **Beenden** wird **kein fester Face-Button** reserviert
   * **X** bleibt in der ersten Version bewusst **unbelegt**
   * eine spaetere Nutzung bleibt als Erweiterungsreserve offen
5. Fokus- und Scroll-Navigation:
   * **D-Pad** bewegt den Fokus
   * **Analogsticks** scrollen
   * **Shoulder-Buttons** scrollen ebenfalls
6. Sprache der ersten Produktfassung:
   * **Deutsch zuerst**
   * spaetere Lokalisierbarkeit soll mitgedacht werden
7. Passwortdarstellung:
   * waehrend der Eingabe **sichtbar**
   * nach Abschluss der Eingabe **nirgendwo mehr sichtbar**
8. Einstiegslogik:
   * es gibt einen **Willkommensbildschirm** mit erklaerendem, scrollbar dargestelltem Text
   * der Bildschirm enthaelt die Checkbox **"Beim naechsten Start nicht mehr anzeigen"** sowie **"Weiter"** und **"Beenden"**
   * nach **"Weiter"** erfolgt eine **Session-Pruefung**
   * mit laufender Service-Session geht es direkt in den **Statusbildschirm**
   * ohne laufende Service-Session startet der **Einrichtungsdurchlauf**
   * der Willkommenstext selbst soll aus einer **separaten Textdatei** kommen
9. Statusbildschirm:
   * Titel des Screens: **"RooK Service"**
   * die **PIN** ist gross und visuell dominant
   * ueber der PIN steht **"Service-PIN"**
   * der Labeltext ist **klar sichtbar**, aber **deutlich kleiner** als die PIN
   * die PIN bleibt **rein typografisch** ohne eigenen Kasten
   * der Bildschirm bietet die Aktionen **"Trennen"** und **"Beenden"**
   * neben der PIN gibt es **keine weiteren dauerhaften Laufzeitstatus-Texte**
10. Semantik von **"Beenden"**:
   * **"Beenden"** schliesst nur die UI
   * eine aktive Service-Session bleibt dabei unberuehrt
11. Semantik von **"Trennen"**:
   * **"Trennen"** beendet die aktive Service-Session sauber
   * **"Trennen"** baut den temporaeren Support-Zustand ab
12. Einrichtungsworkflow:
   * zuerst wird auf bestehendes WLAN geprueft
   * bestehendes WLAN ueberspringt die WLAN-Einrichtung
   * danach wird der VPN-Aufbau versucht
   * bei erfolgreichem VPN-Aufbau wird eine Service-Session gestartet
   * anschliessend wechselt die UI in den Statusbildschirm
13. Erforderliche Setup-Screens:
   * WLAN-Netzauswahl
   * WLAN-Passworteingabe
   * Wartedialog fuer WLAN-Einrichtung
   * Wartedialog fuer VPN-Aufbau
   * eigener Screen fuer VPN-Timeout oder fehlgeschlagenen VPN-Aufbau
14. VPN-Fehlerverzweigung:
    * wenn das VPN nach einiger Zeit nicht zustande kommt, wird ein eigener Screen gezeigt
    * dort gibt es die Optionen **"Abbrechen und Beenden"** und **"WLAN wechseln"**
    * **"Abbrechen und Beenden"** schliesst die UI
    * **"WLAN wechseln"** startet den WLAN-Scan neu und fuehrt zur WLAN-Liste zurueck
15. Warte-Screens:
   * zeigen einen **animierten Spinner**
   * zeigen einen **kurzen Statustext**
16. VPN-Fehlerscreen:
   * formuliert den Fehler **sachlich kurz**
   * verwendet in der Primaerdarstellung **keine technische Detailsprache**
   * erscheint nach **60 Sekunden** ohne erfolgreichen VPN-Aufbau
17. Fehler-Screens:
   * folgen einem **einheitlichen Grundmuster**
   * dieses Muster gilt auch fuer **WLAN-Fehler**
   * **"Abbrechen und Beenden"** ist das generelle Fehler-Screen-Muster, sofern kein sinnvollerer Rueckweg existiert
18. WLAN-Fehler:
   * verwendet ebenfalls einen eigenen Fehler-Screen
   * erscheint nach **30 Sekunden** ohne erfolgreichen WLAN-Aufbau
   * bietet **"Zurueck zur WLAN-Liste"** und **"Abbrechen und Beenden"**
19. Persistenz von **"nicht mehr anzeigen"**:
   * wird **dauerhaft** gespeichert
   * Speicherung erfolgt in einer Datei unter **`.config`** im Homeverzeichnis des Users
   * Zuruecksetzen in der ersten Version **nicht in der UI**
   * Zuruecksetzen vorerst nur **manuell ueber die Konfigurationsdatei**
20. Scroll-Hinweise auf dem Willkommensbildschirm:
   * Scrollbarkeit wird durch eine **sichtbare Scrollleiste** gezeigt
   * zusaetzlich gibt es den Hinweistext **"Mit Stick oder Schultertasten scrollen"**
21. Bestaetigung fuer **"Trennen"**:
   * nur noetig, wenn gerade eine **aktive Service-Session mit sichtbarer PIN** besteht
   * Titel des Dialogs: **"Service-Verbindung trennen"**
   * Haupttext des Dialogs: **"Service-Verbindung wirklich trennen?"**
   * Aktionen des Dialogs: **"Trennen"** und **"Zurueck"**
22. On-Screen-Keyboard:
   * Titel des Screens: **"WLAN-Passwort eingeben"**
   * startet mit **deutschem QWERTZ-Layout**
   * soll spaeter **austausch- oder wechselbar** aufgebaut sein
   * ist in der ersten Version ein **volles Layout**
   * enthaelt **Shift**, **Alt** und **Caps Lock** als anwaehlbare Tasten
   * diese Tasten aendern die sichtbare Belegung
   * die ausgewaehlte **SSID** wird deutlich oberhalb der Eingabe angezeigt
   * der Passwort-Screen hat eine eigene anwaehlbare Aktion **"Verbinden"**
   * der Passwort-Screen hat zusaetzlich eine sichtbare Aktion **"Zurueck"**
   * das Keyboard enthaelt eine sichtbare **Backspace-Taste mit Symbol statt Textbeschriftung**
   * Aktionsbeschriftungen wie **"Verbinden"** und **"Zurueck"** sind **nicht fest in der Keyboard-Library verdrahtet**
    * sie werden von aussen an den Keyboard-Screen bzw. die Keyboard-Komponente uebergeben
    * auf dem Passwort-Screen loescht **B** das vorherige Zeichen
    * nur bei leerer Eingabe fuehrt **B** den normalen Ruecksprung des aktuellen Screens aus
    * wenn dabei kein sinnvoller Ruecksprungpunkt im Stack vorhanden ist, beendet **B** die App
    * diese Bedienregel wird optisch kenntlich gemacht
   * das Keyboard bekommt eine **eigene visuelle Identitaet**
   * es soll langfristig als **separate Library** weiterpflegbar und in mehreren Projekten nutzbar sein
   * es bleibt farblich bei **Gelb und Lila**
   * es soll **schlicht, klar und wenig verspielt** wirken
   * es bleibt trotzdem erkennbar im **8-Bit-Look**
   * es erscheint im UI als **vollwertiger eigener Screen**
23. Statusquelle:
   * eine laufende **Service-Session** wird ausschliesslich ueber den vom RooK Agent gelieferten Status erkannt
   * auch **WLAN**- und **VPN**-Zustand werden ausschliesslich ueber den vom RooK Agent gelieferten Status erkannt
24. Bereits in `spec/` verankerter Mindeststatus:
   * `GetStatus` liefert mindestens `supportActive`, `supportState`, `wifiState`, `vpnState`
   * zusaetzlich `anyWifiActive`, `supportWifiActive` und optional `activeWifiConnection`
   * Event-seitig sind mindestens `WifiConnectionStateChanged`, `VpnStateChanged`, `SupportStateChanged`, `PinAssigned` und `ErrorRaised` relevant
25. WLAN-Netzauswahl:
   * Titel: **"Mit WLAN verbinden"**
   * WLAN-Scan ist ein **laufender Prozess**
   * neue Netze koennen nach und nach erscheinen
   * der Scan endet erst mit der WLAN-Auswahl des Nutzers
   * verfuegbare Netze werden in der ersten Version **alphabetisch nach SSID** sortiert
   * neue Netze werden **live alphabetisch einsortiert**
   * die aktuelle Auswahl bleibt dabei stabil
   * ein aktuell aktives WLAN wird **nicht besonders markiert**
   * es gibt eine sichtbare Aktion **"Zurueck"**
   * **"Zurueck"** fuehrt zum vorherigen Screen aus dem Stack, sonst beendet es die App
   * bei leerem Scan gibt es einen Leerstaat mit **"Abbrechen und Beenden"**
   * wenn nach **30 Sekunden** noch kein erstes WLAN gefunden wurde, wird der Scan automatisch neu gestartet
26. Agent-/IPC-Verlust:
   * wenn der Kontakt zum RooK Agent waehrend des Betriebs verloren geht, beendet sich die UI **sofort**
27. Reboot-Semantik:
   * nach Reboot ist fuer die UI **kein eigener Sonderfall** vorgesehen
   * die UI prueft beim Start immer den aktuellen Agent-Status und entscheidet daraus
   * wenn der Agent nach Reboot unerwartet weiter eine laufende Session meldet, ist das als **Agent-Thema** zu behandeln
28. Architektur-Guardrails:
   * die UI soll in klar getrennte Schichten fuer App-Shell, Flow-Steuerung, Screens, Komponenten und Adapter aufgebaut werden
   * Agent-Zugriff und Preview-Daten muessen hinter austauschbaren Interfaces liegen
   * wiederkehrende Screen-Muster werden als gemeinsame Komponenten gebaut
   * die Entscheidung ueber den **naechsten Screen** liegt primaer beim aktuellen Screen
   * wiederverwendbare Komponenten navigieren nicht selbst, sondern liefern nur Actions/Ereignisse zurueck
   * Screens geben Folgeaktionen als **Intent** an die App-Shell zurueck
   * Intents koennen Zielscreen und Parameter transportieren
    * minimale Intent-Menge in der ersten Version:
      * `NavigateTo(screen-id, params)`
      * `CloseApp`
      * `NoOp`
    * `params` werden in der ersten Version als **generische Key-Value-Map** modelliert
    * fuer Rueckspruenge wird ein **app-weiter Stack** aus Screens bzw. Start-Intents gepflegt
   * pro Screen sind **Logik** und **View** getrennt, bleiben aber zusammen organisiert
   * wiederverwendbare Komponenten bleiben moeglichst **praesentational** und melden Events nach oben
   * globale UI-Zustaende liegen zentral in App-Shell bzw. kleinem App-Context
   * kleine modale Dialoge werden bevorzugt als **Overlay innerhalb des aktuellen Screens** modelliert
   * Screen-IDs sind stabile **String-IDs** wie `welcome`, `status`, `wifi-list`, `keyboard`
   * Screens greifen **nicht direkt** auf Agent-Ports zu
   * Agent-Anbindung laeuft ueber einen dazwischenliegenden **Flow-/Service-Layer**
29. Design-Guardrails:
   * visuelle Regeln fuer Farben, Typografie, Abstaende, Fokusstil und allgemeine Designsprache muessen vor der Implementierung festgezogen werden
   * gestalterische Grundrichtung: **8-Bit-/Retro-Spiel-Aesthetik**
   * Referenzrahmen: **RetroPie / EmulationStation**
    * trotz Retro-Anmutung soll die UI **sachlich und funktional** bleiben
    * verbindliche Leittfarben: **`#9000ff`** und **`#ffdc00`**
    * **`#ffdc00`** wird als **Hintergrundfarbe** eingesetzt
    * das Gelb ist als **dominanter Hintergrund fast ueberall** vorgesehen
    * durchgaengige Schriftart: **JetBrains Mono**
    * Fokusdarstellung: **kraeftige lila Kontur** mit klarer Kontrastwirkung
    * Formensprache: **kantig, klar konturiert und retro-kompatibel**
   * Standardtext und Standardlinien: **sehr dunkles Schwarz oder Dunkelgrau**
   * Icons und Spinner: **pixelig/retro-inspiriert**, gern leicht spielnah-kreativ, aber klar lesbar
   * Raster und Abstaende: **grosszuegig** und auf einem **klaren groben Raster**
   * Hauptbuttons: **gefuellte Flaechen** mit **starker Kontur** und klarer Lesbarkeit
   * Primaeraktionen: **dunkle Buttons**
   * **Lila** dient primaer **Fokus und Hervorhebung**, nicht der Standardfuellung von Buttons
   * Dialoge und Fehlerboxen: **klar abgegrenzte dunkle Panels** mit **starker Kontur**
   * Scrollleisten: **sichtbar und klar konturiert**, aber nicht lauter als der Inhalt
   * Ecken und Radien: **klar kantig**, **keine oder nur minimale Radien**
    * Animationen duerfen **deutlich spieliger und auffaelliger** sein als das statische Layout
    * das gilt auch fuer **Screenwechsel** und **groessere Zustandswechsel**
    * der gelbe Hintergrund darf eine **leichte retroartige Textur oder ein Pattern** tragen
    * Typografiehierarchie: **sehr grosse PIN**, **grosse Buttons**, **gut lesbarer Fliesstext**, moeglichst **wenig kleine Schrift**
    * erster Groessenvorschlag fuer 1080p-TV:
      * Screen-Titel: **56 px**
      * Dialogtitel: **44 px**
      * Fliesstext / Haupttext: **32 px**
      * Hilfstexte: **24 px**
      * Button-Beschriftungen: **36 px**
      * PIN-Label: **40 px**
      * PIN-Ziffern: **240 px**
30. Preview- und Testmodus:
   * einzelne Screens muessen ohne laufenden Agent gezielt startbar sein
   * dafuer soll es eine Screen-Registry und reproduzierbare Preview-Daten geben
   * Produktiv- und Preview-Modus sollen denselben UI-Code verwenden
   * der Screen wird ueber seine **ID als Startargument** ausgewaehlt
   * im Preview-Modus gibt es **keine Agent-Interaktion**
   * im Preview-Modus gibt es **keine ablaufenden Timer**
   * **Animationen** laufen trotzdem
   * **Escape** beendet die Anwendung
   * Preview-Daten kommen in der ersten Version aus einer **fest im Code definierten Screen-/Scenario-Registry**
   * pro Screen gibt es in der ersten Version genau **einen Default-Preview-Zustand**
   * alle produktiven **Hauptscreens** sind direkt previewbar
   * bevorzugte CLI-Form: **`rook-ui --preview <screen-id>`**
31. Render-Pfade:
   * **SDL2** ist die gemeinsame Plattformschicht
   * daran werden getrennte Entwicklungs- und Zielgeraete-Backends fuer **RmlUi** angebunden
   * auf dem Zielgeraet wird **SDL2 ueber KMS/DRM** ohne X und ohne Wayland genutzt
   * in Entwicklung und Test darf die Integration ueber **Wayland oder X** laufen
32. Layout-Strategie:
   * **keine eigene Layout-Engine**
   * **RmlUi** ist die festgelegte **Primaerbibliothek**
   * **Yoga** bleibt eine moegliche technische Fallback-Option
   * **Dear ImGui** ist nicht die bevorzugte Basis fuer die Endnutzeroberflaeche
33. On-Screen-Keyboard-Strategie:
   * das Keyboard soll als **eigene UI-Komponente** gebaut werden
   * es soll auf denselben gemeinsamen UI-Bausteinen wie der Rest der Anwendung aufsetzen

Weitere Entscheidungen werden hier mit kurzer Begruendung eingetragen und anschliessend in die betreffenden Abschnitte uebernommen.
