# Model Building Projects

Eigenständige C++20-App mit Qt 6.8.3 Widgets, CMake und SQLite über QSQLITE.
Ergänzend gelten die Workspace-Regeln. Andere Apps bleiben unverändert.
Stufe 3 enthält vier Textfelder, ein optionales JPEG-/PNG-Foto und genau die
Statuswerte Geplant, Angefangen und Fertig. Keine weiteren Hauptfunktionen.

## Architektur und Schutz

`domain` enthält Werte und Validierung, `application` den Repository-Vertrag
und Fotoimport, `persistence` ausschließlich SQL, `ui` die Qt-Oberfläche.
Ein Worker besitzt seine SQL-Verbindung und führt SQL und Bilddecodierung aus.
Widgets und QPixmap bleiben im GUI-Thread. IDs identifizieren Projekte.
Statusspeicherung verwendet explizite englische Schlüssel, niemals Ordinalzahlen.
Texte, Foto und Status bilden einen gemeinsamen atomaren Entwurf.
Fehler erhalten ihn. Jeder Abbruchweg verwendet denselben Verwerfschutz.
Fotos werden begrenzt eingelesen und vollständig geprüft. Grenzen sind 10 MiB,
6000 Pixel pro Seite und 16 Millionen Pixel. Quelldateien niemals verändern.
Tests benutzen nur temporäre Datenbanken und synthetische Bilder.

## Sprachen und Vorschauen

Sechs app-lokale Kataloge für de-DE, en-US, es-ES, fr-FR, it-IT und pt-BR
übersetzen alle eigenen UI-Meldungen. Neue Texte im App-Katalog ergänzen.
Sprachwahl gilt ab Neustart. Statuscodes bleiben englisch und unverändert.
GitHub-Namen und Metadaten bleiben englisch. Vorschauen ausschließlich als
Ordner ohne ZIP oder anderes Lieferarchiv liefern. Echte Fenster auf eigenen
virtuellen Displays prüfen. Siehe `docs/internationalization.md`.

## Prüfen und abschließen

```bash
source scripts/env.sh
find src tests -type f \( -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 clang-format -i
bash scripts/check.sh
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 bash scripts/check.sh asan
bash scripts/run.sh --data-dir /absoluter/isolierter/testpfad
```

`.tools/env.sh` ist optional, lokal und ignoriert. Keine Abhängigkeit auf eine
andere Portfolio-App einführen. Eigenes Git-Repository und eigene Paketkennung.
Zusätzlich echte Fenster und ein eigenständiges Paket einschließlich Import,
Statuswechseln, Fehlern und Neustart prüfen. Grenzen ehrlich dokumentieren.
Nach erfolgreichen Prüfungen lokal committen und Workspace-Changelog ergänzen.
Keine Nutzerdaten, Build-Artefakte oder Secrets committen. Kein Push.
Veröffentlichungsreihenfolge: zuerst Microsoft Store, danach Mac App Store.
Native Abnahmen, Signierung und konkrete Store-Lizenzkonformität bleiben separat.

## Gemeinsame C++-Verbesserungen, 1. November 2026

Der öffentliche Produktname ist **Model Building Projects** in allen sechs Sprachen.
Bestehende App-Kennungen, Datenpfade und SQLite-Schemata beibehalten.
Der Nutzer bestätigt für alle fünf C++-Apps die unterstützte Systemsprache beim
Erststart mit Englisch als Fallback. Explizite Sprachwahlen bleiben erhalten.
Dies ersetzt die allgemeine Englisch-Vorgabe ausschließlich für diese C++-Reihe.

Info enthält Über die App, Datenschutz und Lizenzen mit roten Schließen-Buttons.
Die Original-Lizenztexte werden offline eingebettet. Sammlungstransfer per JSON
ist in dieser Stufe ausdrücklich beauftragt, einschließlich aller vorhandenen
Fotos und gegebenenfalls Status und Checklisten. Kein Austausch zwischen Apps.
Import ergänzt fehlende Exemplare atomar. Vorhandene Einträge bleiben erhalten.
Dateien verwenden das eigene Format in Version 1. Höchstens 10.000 Einträge,
256 MiB und 10.000 Schritte je Projekt. Fotos behalten die bisherigen Grenzen.

Auf macOS wird zuerst der Exportordner nativ gewählt, dann der Dateiname.
Nur ausgewählte Dateien und Ordner erhalten Lese-/Schreibzugriff. Neue Dateien
werden ohne Überschreiben veröffentlicht. Ohne Hardlinks darf eine Datei exklusiv
neu angelegt und kopiert werden. Nach hartem Abbruch kann dabei eine unvollständige
neue Datei zurückbleiben. Bestehende Ziele werden ausschließlich nach Bestätigung
und erneuter Prüfung atomar ersetzt.

## Farbauswahl ab Stufe 2, 1. November 2026

Auf ausdrücklichen Nutzerauftrag wird die optionale Farbauswahl aus Yarn Box
auf alle vier C++-Apps ab Collection Showcase übertragen. Die Begrenzung auf
die bisherigen Hauptfunktionen bleibt bestehen. Farben gehören zum atomaren
Entwurf und zum JSON-Transfer. Schema 2 ergänzt das Farbfeld nach vollständiger
Prüfung und mit atomarer Migration von Schema 1. App-IDs, Datenpfade und
vorhandene Inhalte bleiben erhalten. Neue Werte verwenden #RRGGBB oder
#RRGGBBAA. Alte Exporte ohne Farbe bleiben lesbar. Alle sechs Sprachen abdecken.
