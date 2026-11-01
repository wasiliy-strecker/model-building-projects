# Komponenten und Lizenzhinweise

Model Building Projects verwendet Qt 6.8.3 unter der GNU Lesser General Public License,
Version 3 (LGPL-3.0). Qt Core, Gui, Widgets und Sql werden dynamisch eingebunden.
Der QSQLITE-Treiber wird als dynamisches Plugin ausgeliefert. Qt Test wird nur
für Entwicklungsprüfungen verwendet.

Qt ist Copyright The Qt Company Ltd. und weiterer Beitragender.
Lizenzinformationen: https://www.qt.io/development/open-source-lgpl-obligations
Quellcode der eingesetzten Qt-Basis:
https://download.qt.io/archive/qt/6.8/6.8.3/submodules/qtbase-everywhere-src-6.8.3.tar.xz

Die Dateien im Ordner licenses enthalten die Lizenztexte und Hinweise zu
mitgelieferten Fremdkomponenten. Zur Veröffentlichung müssen der zu den
ausgelieferten Bibliotheken gehörende Quellcode und die nötigen Bauhinweise
verfügbar gemacht werden. Das lokale Testpaket ist noch keine Store-Freigabe.

Die Rechte zum Ändern, Ersetzen und erneuten Einbinden der LGPL-Bibliotheken
sowie zur Untersuchung solcher Änderungen werden nicht eingeschränkt.
Eine spätere zusätzliche Nutzungsvereinbarung darf diese Rechte nicht aufheben.

SQLite ist gemeinfrei. Weitere Komponenten innerhalb von Qt, beispielsweise
Unicode-Daten, PCRE2, HarfBuzz, FreeType, libpng und zlib, haben eigene Hinweise.
Die Paketierung übernimmt die mit dem konkreten SDK ausgelieferten Metadaten
und dokumentiert zusätzlich gebündelte Laufzeitbibliotheken.

Eigener App-Code: Copyright 2026 Wasiliy Strecker. Eine gesonderte öffentliche
Quellcodelizenz ist bisher nicht festgelegt. Diese Aussage ändert keine Rechte
an Qt oder anderen Fremdkomponenten.

Die Bildverarbeitung benutzt Qt Gui und das dynamische Qt-JPEG-Plugin.
Keine weitere Bildbibliothek wird direkt von der App eingebunden.
Im tatsächlich verwendeten Qt-6.8.3-Linux-SDK weisen die SPDX-Dateien
libpng 1.6.47, libjpeg-turbo 3.1.0 und SQLite 3.49.1 aus.
Die jeweiligen Lizenztexte und Attributionen liegen unter
`licenses/qtbase-6.8.3/src/3rdparty`. PNG-Unterstützung liegt in Qt Gui,
JPEG-Unterstützung im separat beigelegten Imageformat-Plugin.

Das Linux-Testpaket enthält ICU 73.2, libxcb-cursor und das Qt-GTK3-Plattformthema.
Die systemseitige GTK3-Laufzeit und übliche Linux-Desktopbibliotheken bleiben
Voraussetzungen. Die mitgelieferten SDK-SPDX-Dateien dokumentieren den konkreten
Build und seine Fremdkomponenten. Native Windows- und macOS-Pakete benötigen
ihre eigenen Abhängigkeitsinventare. Eine SDK-Lizenzalternative allein ist
keine Aussage über die Konformität einer späteren Store-Auslieferung.
