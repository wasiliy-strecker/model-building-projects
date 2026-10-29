# ModellbauProjekt

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
