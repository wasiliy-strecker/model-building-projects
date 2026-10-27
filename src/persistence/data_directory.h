#pragma once

#include <QString>

namespace modelbuild {
QString defaultDataDirectory();
QString resolveDataDirectory(const QString& overridePath, bool explicitlyProvided);
} // namespace modelbuild
