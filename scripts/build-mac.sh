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

echo "=========================================="
echo "  Build completed successfully!"
echo "  App bundle: ${BUILD_DIR}/ka-hgis.app"
echo "=========================================="
