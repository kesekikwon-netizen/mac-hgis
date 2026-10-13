#pragma once

#include "KaPortableRuntime.h"
#include <QDir>

#if defined(Q_OS_MAC)
namespace KaPortableRuntimeMac {
void discoverMac(KaPortablePaths& out, const QDir& app);
bool applyMacEnvironment();
}  // namespace KaPortableRuntimeMac
#endif
