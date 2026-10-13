#include "KaSecretStoreInternal.h"

#if defined(__APPLE__)
#define __STDC_WANT_LIB_EXT1__ 1
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonRandom.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <string.h>
#include <unistd.h>

namespace KaSecretStoreInternal {

static QByteArray getOrCreateMasterKey() {
  const char* serviceStr = "ka-hgis";
  const char* accountStr = "secret-store-v1";

  CFStringRef service = CFStringCreateWithCString(nullptr, serviceStr, kCFStringEncodingUTF8);
  CFStringRef account = CFStringCreateWithCString(nullptr, accountStr, kCFStringEncodingUTF8);

  const void* queryKeys[] = {kSecClass, kSecAttrService, kSecAttrAccount, kSecReturnData, kSecMatchLimit};
  const void* queryValues[] = {kSecClassGenericPassword, service, account, kCFBooleanTrue, kSecMatchLimitOne};
  CFDictionaryRef query = CFDictionaryCreate(nullptr, queryKeys, queryValues, 5, &kCFTypeDictionaryKeyCallBacks,
                                             &kCFTypeDictionaryValueCallBacks);

  CFTypeRef result = nullptr;
  OSStatus status = SecItemCopyMatching(query, &result);
  CFRelease(query);

  if (status == errSecSuccess && result) {
    CFDataRef data = static_cast<CFDataRef>(result);
    const CFIndex len = CFDataGetLength(data);
    const UInt8* ptr = CFDataGetBytePtr(data);
    QByteArray key(reinterpret_cast<const char*>(ptr), static_cast<int>(len));
    CFRelease(result);
    CFRelease(service);
    CFRelease(account);
    if (key.size() == 32) return key;
  }

  QByteArray newKey(32, Qt::Uninitialized);
  arc4random_buf(newKey.data(), static_cast<size_t>(newKey.size()));

  CFDataRef keyData = CFDataCreate(nullptr, reinterpret_cast<const UInt8*>(newKey.constData()), newKey.size());
  const void* addKeys[] = {kSecClass, kSecAttrService, kSecAttrAccount, kSecAttrAccessible, kSecValueData};
  const void* addValues[] = {kSecClassGenericPassword, service, account, kSecAttrAccessibleAfterFirstUnlock, keyData};
  CFDictionaryRef addDict = CFDictionaryCreate(nullptr, addKeys, addValues, 5, &kCFTypeDictionaryKeyCallBacks,
                                               &kCFTypeDictionaryValueCallBacks);
  status = SecItemAdd(addDict, nullptr);
  CFRelease(addDict);
  CFRelease(keyData);
  CFRelease(service);
  CFRelease(account);

  if (status == errSecSuccess) {
    return newKey;
  }

  char host[256] = {};
  gethostname(host, sizeof(host));
  const char* user = getenv("USER");
  QByteArray seed = QByteArray(serviceStr) + QByteArray(accountStr) + QByteArray(user ? user : "") + QByteArray(host);
  unsigned char hash[CC_SHA256_DIGEST_LENGTH];
  CC_SHA256(seed.constData(), static_cast<CC_LONG>(seed.size()), hash);
  return QByteArray(reinterpret_cast<const char*>(hash), 32);
}

bool platformProtectBytes(const QByteArray& plain, QByteArray* out) {
  if (!out || plain.isEmpty()) return false;
  out->clear();

  const QByteArray key = getOrCreateMasterKey();
  if (key.size() != 32) return false;

  QByteArray iv(kCCBlockSizeAES128, Qt::Uninitialized);
  arc4random_buf(iv.data(), static_cast<size_t>(iv.size()));

  const size_t bufSize = plain.size() + kCCBlockSizeAES128;
  QByteArray encrypted(static_cast<int>(bufSize), Qt::Uninitialized);
  size_t bytesMoved = 0;

  const CCCryptorStatus cryptStatus =
      CCCrypt(kCCEncrypt, kCCAlgorithmAES, kCCOptionPKCS7Padding, key.constData(), kCCKeySizeAES256, iv.constData(),
              plain.constData(), static_cast<size_t>(plain.size()), encrypted.data(), bufSize, &bytesMoved);

  if (cryptStatus != kCCSuccess) return false;
  encrypted.resize(static_cast<int>(bytesMoved));

  QByteArray payload = iv + encrypted;
  *out = payload.toBase64();
  return !out->isEmpty();
}

bool platformUnprotectBytes(const QByteArray& base64, QByteArray* out) {
  if (!out) return false;
  out->clear();

  const QByteArray payload = QByteArray::fromBase64(base64);
  if (payload.size() <= kCCBlockSizeAES128) return false;

  const QByteArray key = getOrCreateMasterKey();
  if (key.size() != 32) return false;

  const QByteArray iv = payload.left(kCCBlockSizeAES128);
  const QByteArray encrypted = payload.mid(kCCBlockSizeAES128);

  const size_t bufSize = encrypted.size() + kCCBlockSizeAES128;
  QByteArray decrypted(static_cast<int>(bufSize), Qt::Uninitialized);
  size_t bytesMoved = 0;

  const CCCryptorStatus cryptStatus =
      CCCrypt(kCCDecrypt, kCCAlgorithmAES, kCCOptionPKCS7Padding, key.constData(), kCCKeySizeAES256, iv.constData(),
              encrypted.constData(), static_cast<size_t>(encrypted.size()), decrypted.data(), bufSize, &bytesMoved);

  if (cryptStatus != kCCSuccess) return false;
  decrypted.resize(static_cast<int>(bytesMoved));
  *out = decrypted;
  return true;
}

}  // namespace KaSecretStoreInternal

#endif
