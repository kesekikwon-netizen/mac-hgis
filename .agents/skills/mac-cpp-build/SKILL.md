---
name: mac-cpp-build
description: Professional C++20 build and packaging skill for macOS using CMake, AppleClang, Ninja, ccache, and macOS .app bundles with @rpath.
---

# Mac C++ Build & Bundle Engineering Skill

## Overview
This skill provides complete procedures and best practices for building high-performance modern C++20 desktop applications on macOS (Apple Silicon `arm64`), structuring CMake targets, configuring `ccache` acceleration, and producing valid macOS `.app` bundles.

## CMake Best Practices on macOS
1. **Toolchain & Compiler Detection**:
   ```cmake
   if(APPLE)
     set(CMAKE_CXX_STANDARD 20)
     set(CMAKE_CXX_STANDARD_REQUIRED ON)
     add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
     # Export visibility
     set(CMAKE_CXX_VISIBILITY_PRESET hidden)
     set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
   endif()
   ```

2. **Apple Silicon & Architecture**:
   - Default to native `arm64` on Apple Silicon.
   - Support Universal 2 (`arm64;x86_64`) when packaging for distribution.

3. **macOS App Bundle (`.app`) Configuration**:
   ```cmake
   add_executable(ka-hgis MACOSX_BUNDLE
     ${SOURCES}
     ${RESOURCES}
   )
   set_target_properties(ka-hgis PROPERTIES
     MACOSX_BUNDLE_BUNDLE_NAME "HGIS"
     MACOSX_BUNDLE_GUI_IDENTIFIER "kr.hgis.ka-hgis"
     MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
     MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
     MACOSX_BUNDLE_COPYRIGHT "Copyright © 2026 HGIS Team"
   )
   ```

4. **Dynamic Linking & `@rpath`**:
   To ensure the application can find frameworks and external dylibs without breaking macOS security:
   ```cmake
   set_target_properties(ka-hgis PROPERTIES
     BUILD_WITH_INSTALL_RPATH TRUE
     INSTALL_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks;/Applications/QGIS.app/Contents/Frameworks"
   )
   ```

5. **Compilation Acceleration**:
   - Use `ccache` via `find_program(CCACHE_PROGRAM ccache)`.
   - Set `CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM}`.
   - Use `Ninja` generator for multi-threaded compilation.
