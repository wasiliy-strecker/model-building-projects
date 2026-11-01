# Prüfbericht

## Sammlungstransfer und gemeinsame Oberfläche, 1. November 2026

Model Building Projects übernimmt die Verbesserungen von Board Game Shelf bis Commit `11245ea`.
Buttons sind neu angeordnet. Alle Tabellenspalten bieten auf- und absteigende
Sortierung, ohne die ausgewählte Eintrags-ID zu verlieren. Info enthält drei
getrennte Offline-Fenster mit roten Schließen-Buttons. Der öffentliche englische
Name bleibt in allen sechs Sprachen gleich. Die Systemsprache beim Erststart
mit Englisch als Fallback ist ausdrücklich bestätigt. Datenpfade, App-Kennungen,
Datenbankschema und gespeicherte Sprachwahlen bleiben erhalten.

JSON-Dateien sichern alle gespeicherten Inhalte einschließlich Originalfotos.
Ein konsistenter Lesesnapshot enthält auch die in der Tabelle absichtlich nicht
geladenen Foto-BLOBs. Import ergänzt fehlende Exemplare atomar, berücksichtigt
die Anzahl identischer Exemplare und verändert keine vorhandenen Einträge.
Der Worker importiert den bestätigten Snapshot,
auch wenn die Quelldatei zwischenzeitlich geändert wird.

Die vollständigen 13 CTest-Suiten bestehen in Development, Release und mit
AddressSanitizer, LeakSanitizer und UndefinedBehaviorSanitizer. Formatter,
strikte Compilerwarnungen, clang-tidy, Ruff, Shell- und XML-Prüfung bestehen.
Die vorgebauten Qt-Bibliotheken sind selbst nicht mit Sanitizern instrumentiert.
Nach den letzten Layoutkorrekturen wurden die betroffenen UI-, Übersetzungs-
und Dialogsuiten in allen drei Konfigurationen zusätzlich erfolgreich geprüft.

Native X11-Prüfungen auf eigenen Xvfb-Bildschirmen verwenden ausschließlich
synthetische Daten. Geprüft sind alle sechs Sprachen, kleine Fenster,
Tastaturbedienung der Info-Menüs, rote Schließen-Buttons, Sortierung mit
beibehaltener Auswahl, vollständiger Sammlungstransfer und die vorhandenen
Foto-/Projektabläufe mit Neustart ohne Bildquelle.

Auch die eigenständigen Linux-Pakete mit eigener Qt-Laufzeit bestehen den
vollständigen Bedienablauf. Native GTK-Dateidialoge prüfen Export bei aktiver
Suche, Importabbruch, bestätigten Import mit Fotos, wiederholten Import ohne
zusätzliche Duplikate und Neustart. Bei Checklisten werden zusätzlich kollidierende
Schrittkennungen geprüft. Laufzeitnachweise kontrollieren die tatsächlich geladenen
Qt-, SQLite- und JPEG-Bibliotheken. Die Pakete werden ausschließlich als Ordner
mit `--stage-only` erzeugt. Vorherige Testpakete bleiben erhalten.

Die Tests simulieren fehlende Hardlinks, späte Dateikollisionen und Schreibfehler.
Neue Dateien werden dann exklusiv angelegt. Der Kopierweg ist nicht atomar und
kann bei hartem Prozessabbruch eine unvollständige neue Datei hinterlassen.
Bestehende Dateien werden ausschließlich nach Bestätigung und erneuter Prüfung
atomar ersetzt. App-Daten und Dateiverweise sind als Exportziel geschützt.

Native Windows- und macOS-Ausführung, Apple-Sandbox, Signierung und ein physischer
exFAT-Datenträger bleiben offen. Linux-Prüfungen sind keine Store-Freigabe.

Prüfbefehle und lokale Nachweise:

```bash
bash scripts/check.sh
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 bash scripts/check.sh asan
cmake --build --preset release --parallel 4
ctest --preset release
```

Vollständige Protokolle liegen unter `build/port-verified-dev.log`,
`build/port-verified-asan.log` und `build/port-release-check.log`.
Die abschließenden UI-Prüfungen stehen in `build/port-final-*.log`.
Native Belege und Aufnahmen liegen in `build/port-final-native/`,
`build/port-package-journey/` und `build/port-final-package-transfer/`.
Die abschließenden Testpakete liegen in `build/port-package-final/`.


## Internationalisierung, 29. Oktober 2026

Release und ASan/UBSan bestehen mit 136 fachlichen Qt-Testausführungen
plus 14 Initialisierungs- und Abschlussaufrufen. Alle acht CTest-Suiten
einschließlich Katalogprüfung bestehen. Sechs Kataloge enthalten je 119 Meldungen.
Formatter, Compilerwarnungen, clang-tidy, Ruff und Shell-Syntax sind geprüft.

Die gebündelte Release-Laufzeit besteht zwölf echte Prozesse auf einem eigenen
Xvfb-Display. Jede Sprache prüft Unicode-CRUD, Fotos, alle Statuswechsel,
Verwerfen, SQLite-Sperren, Sprachspeicherfehler und Neustart. 24 Aufnahmen
mit 1920 × 1080 Pixeln zeigen die laufende Anwendung. Die Bilder wurden visuell
geprüft. Der bisherige deutsche GUI-Gesamtablauf besteht zusätzlich mit
nativer GTK3-Fotoauswahl und 14 Nachweisbildern.

Nachweise liegen in `build/international-release-check.log`,
`build/international-asan-check.log`, `build/international-captures-final-v2/`
und `build/international-legacy-gui-final-v2/`. Die getestete Laufzeit liegt in
`build/staging/international-20261029-v2/modellbauprojekt-0.1.0-linux-x64/`.
Quell- und Laufzeit-Hashes binden die Aufnahmen an den geprüften Stand.
Die Ordnerlieferung enthält sechs Store-Textfassungen und eigenes Demo-Bildmaterial.
Windows-Abnahme und Store-Einreichung sind weiterhin offen.

## Historischer Erststand

Stand: 27. Oktober 2026, ModellbauProjekt 0.1.0.

## Umgebung und automatische Prüfungen

Ubuntu 24.04.4 LTS x64, GCC 13.3.0, Qt 6.8.3, CMake 4.4.4, Ninja 1.11.1,
clang-format 23.1.3 und clang-tidy 22.1.8. Qt ist ein vorgebautes dynamisches
SDK. Der eigene Code verwendet C++20. Qt Test meldet für das SDK dessen
ursprünglichen GCC-10.3.1-Build. Die Anwendung wurde hier mit GCC 13.3.0 gebaut
und zusammen mit diesen Binärdateien tatsächlich ausgeführt.

```bash
bash scripts/check.sh
cmake --preset release
cmake --build --preset release --parallel 4
ctest --preset release
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 bash scripts/check.sh asan
```

**84 fachliche Testausführungen bestehen in sechs CTest-Suiten.** Qt Test zählt
zusätzlich zwölf erfolgreiche Initialisierungs- und Abschlussaufrufe.
Keine Fehler und keine übersprungenen Fälle auf dieser Linux-Umgebung.

| Suite | Fälle | Schwerpunkte |
| --- | ---: | --- |
| domain | 16 | Unicode-Leerraum, Texte, Notiz, vier Suchfelder, Normalisierung, Sortierung, Testpfade |
| repository | 16 | CRUD, IDs, Duplikate, Wiederöffnen, SQL-Texte, Sperren, Rollback, Schema- und Dateifehler |
| dialogs | 14 | Echte Dialoge, Pflichtfeld, Enter, Speicherkürzel, alle Abbruchwege, Löschschutz, Wiederholung, Thread-Lebenszyklus |
| photo | 19 | JPEG/PNG, Inhalt statt Endung, beschädigte Daten, Byte-/Seiten-/Pixelgrenzen, wachsender Stream, EXIF, BLOB-Persistenz |
| status | 11 | Mapping, Standard, alle Übergänge, ungültige Werte, Neustart, gemeinsame atomare Änderung, Status-Verwerfschutz |
| photo_dialogs | 8 | Reine Fotoänderungen, Dateidialogabbruch, vollständiges Zurücksetzen, veraltete Vorschau, zerstörte Fenster, fehlerhaftes gespeichertes Bild |

Temporäre SQLite-Dateien prüfen gleiche Namen mit unabhängigen IDs und Fotos.
Ein erneutes Öffnen lädt alle drei gespeicherten Statuswerte. Unbekannte Werte
werden ausdrücklich gemeldet, auch beim Start. Die SQLite-CHECKs weisen
ungültigen Status und NULL zurück. Fotos werden als Bytes gespeichert,
Listenabfragen enthalten nur deren Kennzeichnung.

Eine zweite echte SQLite-Verbindung erzeugt Schreib- und Commit-Sperren.
Beim Commitfehler sind im echten Editor Name, Bild und Status geändert.
Der Test liest den vollständigen alten Datenbankstand unabhängig zurück,
prüft den erhaltenen Entwurf und speichert nach Freigabe erfolgreich erneut.
Reine Statuswechsel erhalten die unveränderten Texte und Bildbytes.

Die Dialogtests prüfen Abbrechen, Escape und Fensterschließen mit
„Weiterbearbeiten“ als Standard- und Escape-Aktion. Das Hauptfenster schützt
auch einen ausschließlich geänderten Status. Gültige und vollständig
zurückgesetzte Entwürfe schließen ohne unnötige Rückfrage. Ein Foto-Lesefehler
öffnet keinen Editor mit versehentlich leerem Bild.

Verzögerte Detailabfragen prüfen schnellen Auswahlwechsel, eine ansprechbare
GUI und zerstörte Fenster. Ergebnisse einer früheren Auswahl überschreiben
weder das neue Foto noch den Platzhalter. Ein während des Imports zerstörter
Editor bekommt keinen späten Callback. Ein frischer Editor bleibt leer.

Formatter, strenge Compilerwarnungen als Fehler und clang-tidy sind ohne
Befunde. AddressSanitizer, LeakSanitizer und UndefinedBehaviorSanitizer bestehen
ohne Befunde und ohne Unterdrückungsdateien. Die vorgebauten Qt-, SQLite- und
Decoderbibliotheken sind selbst nicht instrumentiert. Dieser Nachweis deckt
deren Interna daher nicht im selben Umfang wie den eigenen Anwendungscode ab.

Ruff und Python-Formatprüfung, `bash -n`, CMake-Preset-JSON und XML/Plist-Syntax
wurden ebenfalls geprüft. PowerShell, Windows-Ressourcencompiler und native
Apple-Werkzeuge konnten hier nicht ausgeführt werden.

## Sichtbare Abnahme

Die Entwicklungsoberfläche wurde auf dem Linux-Desktop angesehen. Der
vollständige Ablauf wird mit echten Fenstern auf einem eigenen sichtbaren
Xephyr-X11-Bildschirm mit Openbox als Fenstermanager durchgeführt. GTK3 stellte
den nativen Dateidialog. Das Testwerkzeug Openbox wurde ausschließlich in die
ignorierte `.tools`-Umgebung entpackt. Es gehört nicht zur App oder zum Paket.

```bash
DISPLAY=:1 python3 scripts/gui_smoke.py --binary build/release/modellbauprojekt --output build/manual/release-visible
```

Der Helfer verwendet eigene Prozess- und Fenster-IDs, prüft den Fokus vor
Eingaben und erzeugt ausschließlich synthetische Bilder und eine temporäre
Datenbank. Er benötigt Python 3, xdotool, xwininfo, xprop und ImageMagick.
Bei `DISPLAY=:0` hält er die gemeinsame Desktop-Sperre über den ganzen Ablauf.
Ein bloßer X11-Server ohne Fenstermanager ist für den nativen GTK-Fokuswechsel
kein geeigneter Abnahmedesktop. Der Test weist darauf ausdrücklich hin.

Ein separater sichtbarer Qt-Test benutzt Schriftgröße 18 und 400 × 340 Pixel.
Die Schaltflächen bleiben erreichbar, das Formular scrollt. Der Screenshot
liegt unter `docs/screenshots/grosse-schrift.png`.

```bash
MODELLBAUPROJEKT_TEST_SCREENSHOT="$PWD/build/manual/grosse-schrift.png" DISPLAY=:1 build/dev/dialogs_tests largeFontSmallDialogKeepsActionsReachable
```

Der vollständige Release-Ablauf besteht. Die Screenshots wurden angesehen und
SQLite unabhängig nach den Aktionen gelesen. Das Ergebnis liegt unter
`build/manual/release-visible/result.json`.

| Sichtbarer Ablauf | Ergebnis |
| --- | --- |
| Leere Liste und Pflichtfeldfehler | Verständliche Hinweise, kein versehentlicher Eintrag |
| Native GTK-Auswahl abbrechen, PNG hinzufügen und JPEG ersetzen | Entwurf erhalten, tatsächliche Bildbytes korrekt gespeichert |
| Mehrzeilige Notiz und Umlautsuche | Alle vier Texte korrekt, Enter fügt eine Zeile ein |
| Suche ohne Treffer | Eigener Leerzustand, Suche lässt sich leeren |
| Reine Statusänderung und sichere Vorauswahl | Weiterbearbeiten erhält die Auswahl, Verwerfen schreibt nichts |
| Gleichzeitige Änderung von Text, Foto und Status bei echter Sperre | Alter Stand bleibt, vollständiger Entwurf lässt sich erneut speichern |
| Fertig zurück auf Geplant und Angefangen | Alle Änderungen persistieren, Foto bleibt bytegenau erhalten |
| Zwei gleichnamige Projekte | Unabhängige IDs, Fotos und Statuswerte |
| Prozessneustart nach Entfernung beider Bildquellen | Beide Fotos und Statuswerte weiterhin vorhanden |
| Foto entfernen | Nur das ausgewählte Projekt verliert sein Foto, Status bleibt |
| Löschabbruch und zweimal bestätigtes Löschen | Nur die gewählte ID wird entfernt, am Ende wieder leere Liste |

## Eigenständiges Linux-Paket

Ein Paket mit eigener Qt-Laufzeit wurde unter `build/staging` erzeugt und in
sein eigenes temporäres Verzeichnis außerhalb des Projekts kopiert.
Der vollständige oben beschriebene sichtbare Ablauf besteht auch dort.
`build/manual/package-visible/result.json` meldet Erfolg.

```bash
DISPLAY=:1 python3 scripts/gui_smoke.py --binary /absoluter/pfad/ModellbauProjekt.sh --output build/manual/package-visible --clean-environment
```

Für den Paketprozess enthielt PATH nur `/usr/bin:/bin`. Externe Qt-, QML-, SDK-
und Bibliothekspfade wurden entfernt. Der Launcher verwendet ausschließlich
seine eigenen Qt- und Pluginpfade. Native GTK-Auswahl, PNG-Import, JPEG-Ersatz,
Statuswechsel in beide Richtungen, SQLite, Fehlerwiederholung, Löschung und
Prozessneustart wurden ausgeführt. Beide Bildquelldateien waren vor dem
Neustart entfernt. Die gespeicherten Statuswerte und Bildbytes blieben erhalten.

`loaded-libraries.json` kontrolliert die tatsächlichen Prozessabbildungen.
Qt, ICU, QSQLITE, JPEG-Plugin und Qt-GTK3-Plattformthema stammen aus dem
Paketverzeichnis. Systemseitiges GTK3 und übliche Linux-Bibliotheken bleiben
Voraussetzungen. Die Windows- und macOS-Abnahme wird dadurch nicht ersetzt.

Das Lieferarchiv liegt unter
`build/releases/modellbauprojekt-0.1.0-linux-x64.tar.gz`, mit SHA-256-Datei.
Es verwendet dieselben geprüften Binärdateien und ergänzt die fertige
Dokumentation einschließlich der angesehenen Screenshots. Die Binärdateien
werden gegen das außerhalb des Projekts geprüfte Paket bytegenau verglichen.

## Datenpfad

QStandardPaths wurde mit der tatsächlichen App-Kennung ohne Öffnen einer
Datenbank abgefragt. Auf diesem Linux-Rechner liefert Qt
`/home/unknown/.local/share/de.appfabrik.modellbauprojekt`.
Sämtliche App- und Pakettests verwenden trotzdem ausschließlich `--data-dir`
oder QTemporaryDir. Normale Benutzerdaten wurden nicht geöffnet.

## Offene native Zielsysteme

Windows-x64-EXE/ZIP und MSIX-Vorlage sowie macOS-App-Bundle/DMG für Intel und
Apple Silicon sind vorbereitet. Sie wurden auf diesem Linux-Rechner nicht
nativ gebaut oder bedient. Native Datei- und Rechteprüfungen, Tastenkürzel,
Skalierung, macOS-Sandbox und einmaliger lesender Fotoimport benötigen dort
eine gesonderte Abnahme. Installation und Updates sind ebenfalls offen.

Signierung, konkrete Store-Lizenzprüfung, Identitäten und Store-Schritte
stehen aus. Zuerst Microsoft Store, später Mac App Store. Es wurde weder
gepusht noch veröffentlicht. Linux ersetzt diese nativen Abnahmen nicht.

## Farbauswahl pro Eintrag, 1. November 2026

Der Farbwähler wie in Yarn Box ist pro Eintrag optional. Palette, eigene Farben,
Transparenz, Entfernen, Abbrechen, reine Farbänderungen und die Tab-Reihenfolge
sind in sechs Sprachen geprüft. Tabelle, Suche und Sortierung berücksichtigen
Farbkachel und Hexcode. Foto und Status bleiben erhalten.

Alle 14 CTest-Suiten bestehen im Entwicklungsbuild und mit AddressSanitizer
und UndefinedBehaviorSanitizer. clang-format, Compilerwarnungen als Fehler und
die statischen Prüfungen bestehen. Nach der Fokuskorrektur wurden die beiden
betroffenen UI-Dateien zusätzlich mit clang-tidy geprüft. Die Tests umfassen
RGB/RGBA, echte Schema-1-Fixtures, Migration mit unveränderten IDs und Inhalten,
Rollback und Wiederholung eines gesperrten Umstiegs, atomare Schreibfehler,
Altimporte ohne Farbe und idempotenten JSON-Transfer mit Farbunterschieden.

`color_tests` und `collection_features_tests` bestehen zusätzlich mit xcb und
GTK3 auf einem eigenen sichtbaren Xephyr-Display. Das eigenständige Paket
unter `build/manual/color-package-final/modellbauprojekt-0.1.0-linux-x64/`
besteht `scripts/color_gui_smoke.py` und `scripts/gui_smoke.py` mit bereinigter
Entwicklungsumgebung. Die Farbprüfung öffnet den nativen Picker mit Tab und
Leertaste, bestätigt Weiß, bricht eine weitere Auswahl ab, entfernt die Farbe
und prüft beide Zustände nach zwei Neustarts. Die bisherigen Abläufe prüfen
außerdem Foto und Status, Fehler, Löschen und Neustart ohne Bildquelle.
Die veralteten Klickpositionen der Verwerf- und Löschdialoge im Prüfskript sind
an die bereits vorhandene Button-Reihenfolge angepasst. Tastatureingaben sind
auf die eigenen Prozessfenster begrenzt.

Nachweise liegen unter `build/manual/colors/`, `build/manual/color-package-native/`
und `build/manual/color-package-workflow/`. Tests verwenden ausschließlich
synthetische Daten und temporäre Sammlungen. Native Windows- und macOS-Abnahmen
bleiben offen. Keine Store-Veröffentlichung und kein Lieferarchiv.
