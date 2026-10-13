#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_PATH="${PROJECT_ROOT}/build/ka-hgis.app"
BIN_PATH="${APP_PATH}/Contents/MacOS/ka-hgis"

if [ ! -f "${BIN_PATH}" ]; then
  # Fallback if binary is built directly into build/ka-hgis
  if [ -f "${PROJECT_ROOT}/build/ka-hgis" ]; then
    BIN_PATH="${PROJECT_ROOT}/build/ka-hgis"
  else
    echo "Error: Binary not found at ${BIN_PATH} or ${PROJECT_ROOT}/build/ka-hgis"
    echo "Please build first using ./scripts/build-mac.sh"
    exit 1
  fi
fi

# Ensure GDAL and PROJ data paths are exported
export GDAL_DATA="${GDAL_DATA:-/opt/homebrew/share/gdal}"
export PROJ_LIB="${PROJ_LIB:-/opt/homebrew/share/proj}"
export QGIS_PREFIX_PATH="${QGIS_PREFIX_PATH:-/Applications/QGIS.app/Contents/MacOS}"

if [ "${1:-}" = "--smoke-quit" ]; then
  echo "Running headless smoke-quit..."
  "${BIN_PATH}" --smoke-quit
  echo "Smoke test passed!"
elif [ -d "${APP_PATH}" ]; then
  echo "Opening ${APP_PATH}..."
  open "${APP_PATH}"
else
  "${BIN_PATH}" "$@"
fi
