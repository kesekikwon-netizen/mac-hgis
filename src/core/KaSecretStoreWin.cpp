#include "KaSecretStoreInternal.h"

#if defined(Q_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <wincrypt.h>
#include <windows.h>

namespace KaSecretStoreInternal {

bool platformProtectBytes(const QByteArray& plain, QByteArray* out) {
  if (!out) return false;
  out->clear();
  DATA_BLOB input;
  input.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(plain.constData()));
  input.cbData = static_cast<DWORD>(plain.size());
  DATA_BLOB entropy;
  entropy.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(kEntropy));
  entropy.cbData = static_cast<DWORD>(sizeof(kEntropy) - 1);
  DATA_BLOB output{};
  const BOOL ok = CryptProtectData(&input, L"ka-hgis", &entropy, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output);
  if (!ok || !output.pbData || output.cbData == 0) return false;
  *out = QByteArray(reinterpret_cast<const char*>(output.pbData), static_cast<int>(output.cbData)).toBase64();
  LocalFree(output.pbData);
  return !out->isEmpty();
}

bool platformUnprotectBytes(const QByteArray& base64, QByteArray* out) {
  if (!out) return false;
  out->clear();
  const QByteArray blob = QByteArray::fromBase64(base64);
  if (blob.isEmpty()) return false;
  DATA_BLOB input;
  input.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(blob.constData()));
  input.cbData = static_cast<DWORD>(blob.size());
  DATA_BLOB entropy;
  entropy.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(kEntropy));
  entropy.cbData = static_cast<DWORD>(sizeof(kEntropy) - 1);
  DATA_BLOB output{};
  const BOOL ok = CryptUnprotectData(&input, nullptr, &entropy, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output);
  if (!ok || !output.pbData) return false;
  *out = QByteArray(reinterpret_cast<const char*>(output.pbData), static_cast<int>(output.cbData));
  SecureZeroMemory(output.pbData, output.cbData);
  LocalFree(output.pbData);
  return true;
}

}  // namespace KaSecretStoreInternal

#endif
