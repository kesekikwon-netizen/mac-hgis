# AGENTS.md - ka-hgis macOS Native Development Guide

Welcome to **ka-hgis** (Korean Archaeology HGIS) macOS Porting & Native Development.
This document defines project conventions, architecture, tooling, and specialized agent roles.

---

## 1. Project Overview & Identity
- **Name**: `ka-hgis` (v0.3+)
- **Target Platform**: macOS (Apple Silicon `arm64` & Universal, macOS 14+)
- **Language**: C++20 (AppleClang)
- **Frameworks**:
  - **Qt 6**: UI framework, Widgets, Painting, Event loop, Multimedia, Network, SVG
  - **QGIS 4.x Libraries**: `qgis_core`, `qgis_gui`, `qgis_analysis`, `qgis_3d` (`/Applications/QGIS.app/Contents/Frameworks`)
  - **GDAL 3.x / PROJ / GEOS**: Geodata processing, Coordinate Transformations (EPSG:5179, 5186, 5187)
- **Output Artifact**: macOS Native Application Bundle (`ka-hgis.app`)

---

## 2. Directory Architecture
```
ka-hgis/
├── CMakeLists.txt              # Cross-platform CMake (AppleClang on Mac, MSVC on Win)
├── CMakePresets.json           # Presets for Ninja, Clang, VS2022
├── src/
│   ├── app/                    # Application Entry, MainWindow, Ribbon, Splash, CrashGuard
│   └── core/                   # Core business logic (SurveyFactory, ChecklistEngine, LayerOps, etc.)
├── data/
│   ├── rules/                  # 18-rule excavation drawing checklist JSON
│   ├── schemas/                # ka_hgis_layers YAML schemas
│   └── topographic/            # DXF/DWG/SHP symbols & templates
├── templates/layouts/          # QGIS Print Layout templates (.qpt)
├── tests/                      # CTest unit tests (test_checklist, test_workflow)
├── scripts/                    # Build, launch, package scripts (Mac .sh & Win .ps1)
└── .agents/                    # Antigravity Superpower Skills, Hooks & Configs
```

---

## 3. macOS C++ Development Standards
1. **No MSVC Specifics in Platform-Agnostic Code**:
   - Never use `__declspec(dllimport)` or `/utf-8` directly in C++ files.
   - Use standard C++20 features.
   - Guard any OS-specific calls with `#if defined(Q_OS_MAC)` or `#if defined(Q_OS_WIN)`.
2. **File Paths & Separators**:
   - Never hardcode backslashes `\` or Windows drive letters (`C:`, `D:`).
   - Use `QDir::cleanPath()`, `QDir::separator()`, or standard `/`.
   - Store user data in standard macOS locations via `QStandardPaths::AppDataLocation` (`~/Library/Application Support/ka-hgis`).
3. **macOS UI/UX Guidelines**:
   - Use `QKeySequence::Save` (auto `Cmd+S` on Mac, `Ctrl+S` on Windows), `QKeySequence::Quit` (`Cmd+Q`), `QKeySequence::Export` (`Cmd+E`).
   - Enable Retina / High-DPI support (`AA_EnableHighDpiScaling` or Qt6 default).
   - Ensure macOS global menu bar integration when appropriate.
4. **Credential Security**:
   - Replace Windows DPAPI (`wincrypt.h`, `CryptProtectData`) with macOS Keychain API (`Security.framework`) or cross-platform safe store.

---

## 4. Specialized Expert Subagents
When executing tasks in this codebase, delegate to or invoke the following specialized subagents:

| Subagent | Role | Responsibilities |
|---|---|---|
| `mac_cmake_builder` | CMake & Build Architect | `CMakeLists.txt`, AppleClang flags, ccache, Ninja, `.app` bundle, `@rpath` setup |
| `qgis_sdk_integrator` | QGIS & GIS SDK Integrator | QGIS 4.x header tree, `/Applications/QGIS.app` dylibs, GDAL/PROJ linking, dynamic loader paths |
| `mac_native_adapter` | macOS Platform Specialist | Mac Keychain (`KaSecretStore`), POSIX file locking (`SurveyDurability`), Retina UI, Cmd shortcuts |
| `mac_gis_tester` | Quality & CI Engineer | CTest execution, headless smoke-quit, 18-rule checklist verification, GitHub Actions macOS CI |

---

## 5. Build & Run Procedures
```bash
# Configure with Ninja + ccache + Clang
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DKA_HGIS_BUILD_TESTS=ON

# Build
cmake --build build

# Run unit tests
ctest --test-dir build --output-on-failure

# Launch the macOS application
open build/ka-hgis.app
```

---

## 6. Continuous Auto Commit & Push Policy (매 단계 자동 커밋 및 푸시)
- **Autonomous Auto Commit & Push**:
  - 모든 기능 구현, 버그 수정, 설정 변경 및 리팩토링의 각 단계가 완료될 때마다 반드시 자체 빌드(`cmake --build build`)와 테스트(`ctest`) 검증을 거친 후, **자율적으로 `git commit` 및 `git push origin main`을 즉시 실행**한다.
  - 사용자에게 커밋/푸시 여부를 묻지 않고 완료 시점마다 자동으로 동기화하여 변경 사항이 유실되지 않도록 보장한다.
  - 커밋 메시지는 Conventional Commits 표준(`feat:`, `fix:`, `refactor:`, `test:`, `docs:`)을 준수한다.

