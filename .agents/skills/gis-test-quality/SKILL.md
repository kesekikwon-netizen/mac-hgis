---
name: gis-test-quality
description: Comprehensive testing and validation procedures for ka-hgis on macOS, covering CTest unit tests, headless smoke testing, and GitHub Actions CI automation.
---

# GIS Test & Quality Assurance Skill

## Overview
This skill outlines how to run and verify tests for `ka-hgis` on macOS, ensuring that geometry checks, 18-rule excavation drawing verification, and survey data persistence remain defect-free.

## 1. Running Unit Tests via CTest
The project contains unit and workflow test suites (`ka_hgis_tests`, `ka_workflow_tests`):
```bash
# Run all tests with verbose failure output
ctest --test-dir build -C Release --output-on-failure
```

## 2. Headless Smoke Test
To verify the application launches and exits cleanly without hanging or crashing on macOS:
```bash
./build/ka-hgis.app/Contents/MacOS/ka-hgis --smoke-quit
```
Expected: Exit code 0, no memory leaks or unhandled signals.

## 3. GitHub Actions macOS Runner
In `.github/workflows/ci.yml`:
- Use `macos-latest` (Apple Silicon M-series runner).
- Install dependencies: `brew install cmake ninja ccache qt gdal`.
- Run sanity checks, configuration, build, and CTest.
