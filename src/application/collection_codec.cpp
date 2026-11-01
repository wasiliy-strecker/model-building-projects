#include "application/collection_codec.h"
#include "application/photo_importer.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <cmath>
namespace modelbuild {
namespace {
[[noreturn]] void invalid() {
  throw TransferError(
      QCoreApplication::translate("App", "Die Sammlungsdatei enthält ungültige oder zu große "
                                         "Daten. Die Sammlung bleibt unverändert."));
}
void check(const ProjectDraft& draft) {
  if (validateDraft(draft) != draft)
    invalid();
  for (const auto& text : {draft.name, draft.manufacturer, draft.location, draft.note}) {
    if (!text.isValidUtf16() || text.size() > 1000000)
      invalid();
  }
  if (!draft.photo.isEmpty())
    (void)decodePhoto(draft.photo);
}
} // namespace
QJsonObject encodeEntry(const ProjectDraft& draft) {
  check(draft);
  QJsonObject object{{QStringLiteral("name"), draft.name},
                     {QStringLiteral("manufacturer"), draft.manufacturer},
                     {QStringLiteral("location"), draft.location},
                     {QStringLiteral("note"), draft.note},
                     {QStringLiteral("photoBase64"),
                      draft.photo.isEmpty()
                          ? QJsonValue(QJsonValue::Null)
                          : QJsonValue(QString::fromLatin1(draft.photo.toBase64()))}};
  object.insert(QStringLiteral("status"), storedStatus(draft.status));
  return object;
}
ProjectDraft decodeEntry(const QJsonObject& object) {
  if (object.size() != 6)
    invalid();
  for (const auto& field : {QStringLiteral("name"), QStringLiteral("manufacturer"),
                            QStringLiteral("location"), QStringLiteral("note")})
    if (!object.value(field).isString())
      invalid();
  ProjectDraft draft;
  draft.name = object.value(QStringLiteral("name")).toString();
  draft.manufacturer = object.value(QStringLiteral("manufacturer")).toString();
  draft.location = object.value(QStringLiteral("location")).toString();
  draft.note = object.value(QStringLiteral("note")).toString();
  const auto photo = object.value(QStringLiteral("photoBase64"));
  if (!photo.isNull()) {
    if (!photo.isString())
      invalid();
    const auto encoded = photo.toString().toLatin1();
    if (encoded.isEmpty() || encoded.size() > ((maxPhotoBytes + 2) / 3) * 4)
      invalid();
    const auto decoded =
        QByteArray::fromBase64Encoding(encoded, QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded || decoded.decoded.toBase64() != encoded)
      invalid();
    draft.photo = decoded.decoded;
  }
  if (!object.value(QStringLiteral("status")).isString())
    invalid();
  draft.status = statusFromStorage(object.value(QStringLiteral("status")).toString());
  check(draft);
  return draft;
}
QByteArray entryFingerprint(const ProjectDraft& draft) {
  auto object = encodeEntry(draft);
  return QCryptographicHash::hash(QJsonDocument(object).toJson(QJsonDocument::Compact),
                                  QCryptographicHash::Sha256);
}
} // namespace modelbuild
