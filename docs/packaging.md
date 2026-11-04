# Paketierung und spätere Store-Schritte

Aktueller öffentlicher Name: **Model Building Projects**. Alle sechs Oberflächensprachen bleiben
verfügbar. Neue Installationen verwenden die unterstützte Systemsprache mit Englisch
als Fallback. Die technischen Kennungen und Datenverzeichnisse bleiben unverändert.
macOS erlaubt Zugriff auf ausgewählte Dateien und Ordner mit Lese-/Schreibrechten.
Die Export-Ordnerwahl gewährt Zugriff auch für temporäre Nachbardateien.
Native macOS-Sandbox und ein physischer exFAT-Datenträger benötigen eigene Abnahme.


Veröffentlichungsreihenfolge: **zuerst Microsoft Store, später Mac App Store**
für Mac-Computer. Jetzt werden lokale Testpakete vorbereitet. Es gibt weder
Store-Einreichung noch Veröffentlichung oder behauptete Store-Freigabe.

## Gemeinsame Grundlage

Qt 6.8.3 wird dynamisch unter der gewählten kostenfreien LGPL-Basis eingebunden.
Core, Gui, Widgets, Sql, Plattform-Plugins und QSQLITE stammen aus demselben SDK.
Das JPEG-Plugin wird ausdrücklich beigefügt und geprüft. PNG ist im verwendeten
Qt Gui eingebaut. Sechs eingebettete App-Kataloge und fünf Qt-Basisübersetzungen, Komponentenhinweise,
Lizenztexte und SDK-SPDX-Metadaten gehören zum Paket. Qt Test wird nicht ausgeliefert.
Eine separate Qt-Installation oder Entwicklerwerkzeuge sind nicht erforderlich.

Vor Paketierung Formatter, Compilerwarnungen, statische Analyse und Tests prüfen.
Die Presets `dev`, `release` und `asan` verwenden getrennte Build-Verzeichnisse.
Lieferungen enthalten Version, Betriebssystem und Architektur im Namen.
Die Skripte überschreiben vorhandene Lieferverzeichnisse und Archive nicht.
Noch ungeteilte Prüfpakete können für einen neuen Versuch separat archiviert werden.

Lizenztexte und Quellenherkunft stehen unter `licenses`. Der SHA-256 des
unveränderten offiziellen Qt-6.8.3-Quellarchivs wurde lokal nachgerechnet.
Vor öffentlicher Auslieferung müssen passende Quellen und Bauhinweise zuverlässig
bereitgestellt werden. Bibliotheksaustausch und Untersuchung geänderter
LGPL-Bibliotheken dürfen nicht durch App-Bedingungen ausgeschlossen werden.
Die konkrete Lizenzkonformität von Store-Paketierung, Signierung und Vertrieb
bleibt vor einer Einreichung gesondert zu prüfen. Es wurde nichts gekauft.

## Ergänzendes Linux-Testpaket

```bash
source scripts/env.sh
cmake --preset release
cmake --build --preset release --parallel 4
ctest --preset release
python3 scripts/package_linux.py --qt "$MODELLBAUPROJEKT_QT_ROOT"
```

Falls `libxcb-cursor.so.0` nicht systemweit verfügbar ist, zusätzlich
`--cursor-library /pfad/libxcb-cursor.so.0` und
`--cursor-license /pfad/zum/copyright` angeben. `--stage-only` erzeugt ein
Prüfverzeichnis unter `build/staging`, ohne eine Lieferung zu archivieren.

Das Paket enthält eigene Qt-Bibliotheken, ICU 73, QSQLITE, JPEG-Plugin,
XCB-Plattform und GTK3-Plattformthema. Übliche Ubuntu-24.04-Desktopbibliotheken
wie glibc, GTK3, X11, Fontconfig und OpenGL bleiben Systemvoraussetzungen.
Es ist ein Linux-x64-Testpaket, kein universelles Linux- oder Store-Angebot.

```bash
QT_QPA_PLATFORMTHEME=gtk3 ./build/releases/modellbauprojekt-0.1.0-linux-x64/ModellbauProjekt.sh --data-dir /tmp/modellbauprojekt-pakettest
```

Die Paketierung kontrolliert dynamische Bibliotheksauflösung und erzeugt
SHA-256-Prüfsummen. Für die Abnahme das Paket aus dem Quellbaum herauskopieren
und mit bereinigten Qt-, QML- und Bibliothekspfaden starten. Die Testskripte
kontrollieren zusätzlich `/proc/<pid>/maps`. Qt, ICU, QSQLITE und JPEG müssen
aus dem Paket stammen. Ein Versionsaufruf ist kein ausreichender Test.

```bash
DISPLAY=:0 python3 scripts/gui_smoke.py --binary /absoluter/pfad/ModellbauProjekt.sh --output build/manual/package-visible --clean-environment
```

Der Helfer setzt das GTK3-Thema für den nativen Dateidialog, verwendet eigene
Fenster-IDs und prüft den fokussierten Prozess vor Tastatureingaben. Auf dem
gemeinsamen Desktop nimmt er über den gesamten Ablauf die Sperre
`/tmp/app-factory-cpp-visible-ui.lock`. Es muss ein X11-Fenstermanager laufen.
Ein eigener X11-Desktop mit Fenstermanager kann unabhängig verwendet werden.
Die Abnahme nutzt nur generierte Bilder und ein temporäres Datenverzeichnis.

## Windows x64 und Microsoft Store zuerst

Voraussetzungen sind Visual Studio 2022 mit x64-C++-Werkzeugen und Windows-SDK,
CMake, Ninja und Qt 6.8.3 MSVC 2022 x64. Alle nativen Schritte sind auf diesem
Linux-Rechner vorbereitet, aber noch nicht unter Windows ausgeführt.

```powershell
$env:MODELLBAUPROJEKT_QT_ROOT = 'C:\Qt\6.8.3\msvc2022_64'
.\scripts\package_windows.ps1 -Format zip
```

Das Skript baut und testet, verwendet `windeployqt`, übernimmt QSQLITE und JPEG
explizit und kopiert die VC143-CRT-DLLs aus `VCToolsRedistDir` app-lokal.
Es prüft die benötigten Laufzeitdateien und liefert die eingebetteten Übersetzungen,
Lizenztexte, Dokumentation und Prüfsummen mit. Die Microsoft-Redistributables
müssen bei der nativen Distributionsprüfung ebenfalls berücksichtigt werden.

MSIX benötigt die echten Identitätswerte des registrierten Produkts:

```powershell
.\scripts\package_windows.ps1 -Format msix -IdentityName '<Partner-Center-Paketname>' -Publisher '<Partner-Center-Publisher>' -PublisherDisplayName '<Anzeigename>'
```

Keine erfundenen Store- oder Publisher-Werte. Die interne Kennung
`de.appfabrik.modellbauprojekt` ist kein Ersatz für die Partner-Center-Identität.
Die Vorlage verwendet `Windows.FullTrustApplication` und `runFullTrust`.
`makeappx.exe` stammt aus dem Windows-SDK. Für ZIP und MSIX ist keine
MSI-Upgrade-GUID erforderlich. Das Skript signiert nicht und lädt nichts hoch.

Vor Store-Einreichung bleiben Produktregistrierung, echte Identität,
Lizenzprüfung der konkreten Distribution, native Installation und Updates,
Bilddecoder, Systemdialoge, Tastatur, Datenpfade, Metadaten, Datenschutzangaben,
Signierung und Microsoft-Zertifizierung offen. Lokales Sideloading benötigt
einen gesonderten Testsignierungsweg. Keine Schlüssel oder Zertifikate committen.

## macOS und späterer Mac App Store

Vorgesehen sind Xcode 15 oder neuer und das Qt-6.8.3-macOS-SDK.
Deployment-Target ist macOS 12. Intel und Apple Silicon benötigen jeweils
native Builds und Abnahmen. Diese wurden hier nicht ausgeführt.

```bash
export MODELLBAUPROJEKT_QT_ROOT=/absoluter/pfad/Qt/6.8.3/macos
bash scripts/package_macos.sh
```

Das Skript erzeugt `ModellbauProjekt.app` mit Bundle-ID
`de.appfabrik.modellbauprojekt`, bündelt Qt mit `macdeployqt`, kontrolliert
Cocoa, QSQLITE und JPEG und erstellt eine DMG. `-appstore-compliant` lässt
Qt-Komponenten mit privaten APIs aus. Diese Option garantiert keine Store-Eignung.
Die Ad-hoc-Signatur erlaubt lokale Tests und ist keine Distributionssignatur.

Die Entitlements-Vorlage enthält App Sandbox sowie
`com.apple.security.files.user-selected.read-only`. Die spätere Sandbox soll
nur den einmaligen lesenden Import der ausdrücklich ausgewählten Datei erlauben.
Die App speichert eigene Bildbytes in SQLite und benötigt keinen dauerhaften
Zugriff, keine Security-Scoped Bookmarks und keine Änderung der Quelldatei.
Das Verhalten von Qt, dem nativen Dialog und dem asynchronen Import ist auf dem
signierten Sandbox-Bundle nativ zu prüfen. Die Vorlage allein weist es nicht nach.

QStandardPaths liegt in der Sandbox im App-Container, typischerweise unter
`~/Library/Containers/de.appfabrik.modellbauprojekt/Data/Library/Application Support/`.
Den vollständigen Pfad mit „Über die App“ nativ ermitteln. `--data-dir` muss
bei Sandbox-Prüfungen einen zugänglichen Containerpfad verwenden.

Apple-Entwickleridentität, Team, Provisioning, Signierung aller Bestandteile,
konkrete LGPL-Konformität, Sandbox, Einreichungspaket und Review bleiben offen.
Notarisierte DMG-Direktdistribution ist ein anderer Weg und ersetzt diese
Mac-App-Store-Prüfung nicht. Es werden keine privaten Schlüssel mitgeliefert.

## Native Abnahme

Auf jedem Zielsystem ohne Qt-SDK sichtbar prüfen: Start, alle vier Textfelder,
Unicode-Suche, Fotoimport mit JPEG und PNG, Abbruch des Systemdialogs, Ersatz
und Entfernung, alle Statuswechsel in beide Richtungen, reine Entwurfsänderungen,
Speicherfehler mit Wiederholung, unabhängige gleiche Namen, Löschen und Neustart.
Erneut starten, nachdem die Fotoquelle entfernt wurde. Foto und gespeicherter
Status müssen erhalten sein. Kleine Fenster und vergrößerte Schrift kontrollieren.
Installation und Updates müssen die Daten erhalten.

Der Linux-Nachweis ergänzt diese Prüfungen. Er ersetzt weder eine native
Windows- noch eine macOS-Abnahme. In diesem Repository ist kein automatischer
Upload, Release-Workflow oder Store-Publishing eingerichtet.

## Offizielle Quellen

Am 27. Oktober 2026 geprüft. Vor Veröffentlichung erneut prüfen.

[Qt-LGPL-Pflichten](https://www.qt.io/development/open-source-lgpl-obligations),
[Qt-Bildformate](https://doc.qt.io/qt-6.8/qtimageformats-index.html),
[Windows-Deployment](https://doc.qt.io/qt-6.8/windows-deployment.html),
[macOS-Deployment](https://doc.qt.io/qt-6.8/macos-deployment.html),
[MSIX-Komponenten](https://learn.microsoft.com/en-us/windows/msix/desktop/desktop-to-uwp-manual-conversion)
und [Apple: lesender Dateizugriff nach Auswahl](https://developer.apple.com/documentation/bundleresources/entitlements/com.apple.security.files.user-selected.read-only).

## Internationale Vorschau

Die aktuelle Vorschau wird als Ordner ohne ZIP geliefert. Für die Linux-Abnahme
`package_linux.py --stage-only --output-dir build/staging/international` verwenden.
Die sechs App-Sprachen und Qt-Basisübersetzungen liegen im Binary. Eigene native
Systemdialogtitel sind lokalisiert, OS-Schaltflächen folgen gegebenenfalls der
Systemsprache. Die endgültigen Windows-Aufnahmen, Manifest-Sprachressourcen,
Store-Identität, Installation, Update und native Abnahme folgen separat.
Siehe [Internationalisierung](internationalization.md).

## Paketprüfungen gegen Transporter 90255 und 90277, 4. November 2026

`scripts/macos_bundle.py` prüft die Kennungen in allen enthaltenen `Info.plist`-
Dateien einschließlich eingebetteter Bundles. Ungültige Kennungen, insbesondere
Unterstriche, stoppen die Paketierung. Registrierte App-IDs und fremde
Framework-Kennungen werden nicht automatisch umgeschrieben.

`prepare` ergänzt ausschließlich in der erzeugten `.app`-Kopie die Leserechte
für alle Benutzer sowie die Ausführungsrechte für Verzeichnisse und bereits
als ausführbar markierte Dateien. Daten erhalten keine Ausführungsrechte und
es werden keine zusätzlichen Schreibrechte vergeben. Ein bereits eingebettetes
Provisionierungsprofil wird damit lesbar; das private Original außerhalb der
App bleibt unverändert. Interne Framework-Symlinks bleiben erhalten, externe
oder defekte Links werden abgewiesen. Plist-, Bibliotheks- und Signaturbytes
bleiben unverändert.

Die gezielten Skript-Prüfungen benötigen weder Maven/CMake noch ein SDK,
Signierungszertifikate, Netzwerk oder einen App-Build:

```bash
python3 -B scripts/test_macos_bundle.py
```

Ein vorhandenes Bundle kann ohne Änderungen geprüft werden:

```bash
python3 -B scripts/macos_bundle.py inspect --app "/pfad/zur/App.app"
```

Diese Korrektur ist ausschließlich Quellcode-Vorbereitung. Native Paketprüfung,
Distributionssignierung und Transporter-Validierung folgen beim späteren Release.

`package_macos.sh` führt `prepare` nach dem Kopieren aller Ressourcen vor
`codesign` aus. Nach dem Signieren prüft `inspect` erneut, bevor das DMG
entsteht. Diese Qt-App enthält keine Temurin-Runtime; der konkrete Java-
Runtime-ID-Generator ist hier nicht vorhanden. Der Paketweg bleibt eine
lokale DMG-Lieferung, keine vollständige Mac-App-Store-Auslieferung.
