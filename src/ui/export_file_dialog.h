#pragma once

#include <QString>

class QWidget;
class QDir;

namespace modelbuild {
QString exportFilename(const QString& entered);
QString chooseExportFile(QWidget* parent, const QDir& directory, const QString& suggestedName,
                         bool selectDirectory);
} // namespace modelbuild
