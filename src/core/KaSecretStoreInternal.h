#pragma once

#include <QByteArray>

namespace KaSecretStoreInternal {
constexpr char kEntropy[] = "ka-hgis-account-v1";
bool platformProtectBytes(const QByteArray& plain, QByteArray* out);
bool platformUnprotectBytes(const QByteArray& base64, QByteArray* out);
}  // namespace KaSecretStoreInternal
