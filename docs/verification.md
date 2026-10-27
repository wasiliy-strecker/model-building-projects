# Prüfbericht

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
