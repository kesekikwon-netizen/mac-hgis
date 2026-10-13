---
name: macos-native-porting
description: Porting Windows-specific C++ code to macOS native APIs, including Keychain integration, POSIX file durability, Retina display optimization, and Mac UI conventions.
---

# macOS Native Porting & Platform Adaptation Skill

## Overview
This skill guides replacing Windows-specific APIs (`windows.h`, `wincrypt.h`, DPAPI, file locks) with macOS native APIs, ensuring full compatibility, security, and Mac user experience.

## 1. Replacing Windows DPAPI with macOS Keychain
In `KaSecretStore.cpp`:
- Windows: Uses `CryptProtectData` / `CryptUnprotectData` (`wincrypt.h`).
- macOS Native: Use macOS `Security.framework` (`SecItemAdd`, `SecItemCopyMatching`) or an encrypted local credential store protected by user session.

```cpp
#if defined(Q_OS_WIN)
  // Windows DPAPI implementation
#elif defined(Q_OS_MAC)
  // macOS Keychain implementation via Security.framework
#else
  // Portable fallback
#endif
```

## 2. File Durability & POSIX Atomic Saves
In `SurveyDurability.cpp` and `SurveyStorage.cpp`:
- Windows: Uses `CreateFileW`, `FILE_FLAG_WRITE_THROUGH`, `FlushFileBuffers`.
- macOS: Use POSIX `open()`, `write()`, `fsync()` or `fcntl(fd, F_FULLFSYNC)`.
- macOS `F_FULLFSYNC` guarantees disk flushing to NVMe storage.

## 3. macOS UI & Keyboard Shortcuts
- Use `QKeySequence::Save` (evaluates to `Cmd+S` on Mac, `Ctrl+S` on Windows).
- Use `QKeySequence::Quit` (evaluates to `Cmd+Q`).
- Set `QApplication::setQuitOnLastWindowClosed(true)`.
- Configure `setUnifiedTitleAndToolBarOnMac(true)` where appropriate for native modern macOS titlebars.
