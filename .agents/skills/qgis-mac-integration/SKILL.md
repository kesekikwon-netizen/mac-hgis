---
name: qgis-mac-integration
description: Integration procedures for linking QGIS 4.x libraries (libqgis_core, libqgis_gui) on macOS, managing GDAL/PROJ dependencies, and resolving runtime paths.
---

# QGIS macOS Integration Skill

## Overview
This skill guides linking against QGIS 4.x shared libraries (`.dylib`) on macOS, managing geospatial dependencies (GDAL, PROJ, GEOS), and setting up runtime environment variables.

## Key QGIS Libraries & Paths on macOS
In standard macOS installations (`/Applications/QGIS.app/Contents/Frameworks/`):
- `libqgis_core.dylib`: Core GIS data structures, vector/raster layers, symbology, expressions, layouts.
- `libqgis_gui.dylib`: Map canvas, rubber bands, layer tree view, layout designer view, map tools.
- `libqgis_analysis.dylib`: Vector/raster algorithms, TIN interpolation, hillshade analysis.
- `libqgis_3d.dylib`: 3D map views and terrain rendering.

## Header Files (SDK)
When building against QGIS on macOS:
1. Provide the QGIS C++ header directory tree (e.g. `include/qgis/qgs*.h`).
2. Include paths must include:
   - `${QGIS_INCLUDE_DIR}`
   - `${GDAL_INCLUDE_DIR}` (`/opt/homebrew/include`)
   - `${Qt6_INCLUDE_DIRS}`

## Runtime Environment Variables
When launching the standalone C++ app on macOS, ensure:
```bash
export QGIS_PREFIX_PATH="/Applications/QGIS.app/Contents/MacOS"
export GDAL_DATA="/opt/homebrew/share/gdal"
export PROJ_LIB="/opt/homebrew/share/proj"
```
Or initialize programmatically inside `KaApplication::initQgis()`:
```cpp
#ifdef Q_OS_MAC
QgsApplication::setPrefixPath("/Applications/QGIS.app/Contents/MacOS", true);
#endif
```
