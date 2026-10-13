#!/usr/bin/env bash
set -euo pipefail

# ka-hgis macOS Native Build Script
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${PROJECT_ROOT}"

echo "=========================================="
echo "  ka-hgis macOS Native Build"
echo "=========================================="

# 1. Check tools
for tool in cmake ninja ccache; do
  if ! command -v "${tool}" >/dev/null 2>&1; then
    echo "Error: Required tool '${tool}' is not installed."
    echo "Install with: brew install ${tool}"
    exit 1
  fi
done

# 2. Check QGIS
if [ ! -d "/Applications/QGIS.app" ]; then
  echo "Warning: /Applications/QGIS.app not found. QGIS frameworks will be searched in standard locations."
fi

# 3. Configure
BUILD_DIR="${PROJECT_ROOT}/build"
echo "Configuring CMake into ${BUILD_DIR}..."
cmake -B "${BUILD_DIR}" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DKA_HGIS_BUILD_TESTS=ON \
  "$@"

# 4. Compile
echo "Building targets with Ninja..."
cmake --build "${BUILD_DIR}" --config Release

# 5. Populate app bundle frameworks for macOS dynamic linking
if [ -d "${BUILD_DIR}/ka-hgis.app/Contents" ]; then
  FRAMEWORKS_DIR="${BUILD_DIR}/ka-hgis.app/Contents/Frameworks"
  PLUGINS_DIR="${BUILD_DIR}/ka-hgis.app/Contents/PlugIns"
  RESOURCES_DIR="${BUILD_DIR}/ka-hgis.app/Contents/Resources"
  MACOS_DIR="${BUILD_DIR}/ka-hgis.app/Contents/MacOS"

  mkdir -p "${FRAMEWORKS_DIR}" "${PLUGINS_DIR}" "${RESOURCES_DIR}"

  for lib in Core Gui Widgets Xml PrintSupport Network Sql Concurrent; do
    if [ -f "/opt/homebrew/opt/qtbase/lib/Qt${lib}.framework/Versions/A/Qt${lib}" ]; then
      ln -sf "/opt/homebrew/opt/qtbase/lib/Qt${lib}.framework/Versions/A/Qt${lib}" "${FRAMEWORKS_DIR}/libQt6${lib}.dylib"
      ln -sf "/opt/homebrew/opt/qtbase/lib/Qt${lib}.framework/Versions/A/Qt${lib}" "${FRAMEWORKS_DIR}/libQt6${lib}.6.dylib"
    fi
  done
  if [ -f "/opt/homebrew/opt/qtsvg/lib/QtSvg.framework/Versions/A/QtSvg" ]; then
    ln -sf "/opt/homebrew/opt/qtsvg/lib/QtSvg.framework/Versions/A/QtSvg" "${FRAMEWORKS_DIR}/libQt6Svg.dylib"
    ln -sf "/opt/homebrew/opt/qtsvg/lib/QtSvg.framework/Versions/A/QtSvg" "${FRAMEWORKS_DIR}/libQt6Svg.6.dylib"
  fi
  if [ -f "/opt/homebrew/opt/qtmultimedia/lib/QtMultimedia.framework/Versions/A/QtMultimedia" ]; then
    ln -sf "/opt/homebrew/opt/qtmultimedia/lib/QtMultimedia.framework/Versions/A/QtMultimedia" "${FRAMEWORKS_DIR}/libQt6Multimedia.dylib"
    ln -sf "/opt/homebrew/opt/qtmultimedia/lib/QtMultimedia.framework/Versions/A/QtMultimedia" "${FRAMEWORKS_DIR}/libQt6Multimedia.6.dylib"
  fi
  if [ -f "/opt/homebrew/opt/qt5compat/lib/QtCore5Compat.framework/Versions/A/QtCore5Compat" ]; then
    ln -sf "/opt/homebrew/opt/qt5compat/lib/QtCore5Compat.framework/Versions/A/QtCore5Compat" "${FRAMEWORKS_DIR}/libQt6Core5Compat.6.11.1.dylib"
  fi

  # Symlink Qt plugins into app bundle
  for p in platforms styles imageformats iconengines multimedia tls; do
    if [ -d "/opt/homebrew/share/qt/plugins/${p}" ]; then
      ln -sfn "/opt/homebrew/share/qt/plugins/${p}" "${PLUGINS_DIR}/${p}"
    fi
  done

  # Symlink data and templates into bundle Resources and MacOS dirs
  ln -sfn "${PROJECT_ROOT}/data" "${RESOURCES_DIR}/data"
  ln -sfn "${PROJECT_ROOT}/templates" "${RESOURCES_DIR}/templates"
  ln -sfn "${PROJECT_ROOT}/data" "${MACOS_DIR}/data"
  ln -sfn "${PROJECT_ROOT}/templates" "${MACOS_DIR}/templates"
fi

echo "=========================================="
echo "  Build completed successfully!"
echo "  App bundle: ${BUILD_DIR}/ka-hgis.app"
echo "=========================================="
