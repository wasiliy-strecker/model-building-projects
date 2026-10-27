#pragma once

#include <QWidget>

namespace modelbuild {
bool confirmDiscard(QWidget* parent);
bool confirmDelete(QWidget* parent, const QString& name);
} // namespace modelbuild
