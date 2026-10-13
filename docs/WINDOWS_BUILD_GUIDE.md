# ka-hgis (Strata) Windows 빌드 설명서 및 크로스 플랫폼 아키텍처 설계도

> **대상**: Windows 10/11 (64-bit)  
> **언어 및 표준**: C++20 (MSVC 2022 v143)  
> **핵심 프레임워크**: Qt 6.x, QGIS 4.x (`qgis-dev`), GDAL 3.x, PROJ, GEOS  
> **빌드 시스템**: CMake 3.21+ / Visual Studio 2022 / Ninja  

---

## 1. 크로스 플랫폼 아키텍처 설계도 (macOS & Windows Dual-Platform)

`ka-hgis`는 단일 C++20 코드베이스로 **macOS(Apple Silicon Native)**와 **Windows(x64 Native)**를 모두 지원하도록 설계되었습니다.

```
                      [ 단일 C++20 소스 트리 ]
                                 │
         ┌───────────────────────┴───────────────────────┐
         ▼                                               ▼
   [ macOS Native ]                               [ Windows Native ]
• 컴파일러: AppleClang (C++20)                  • 컴파일러: MSVC 2022 (v143 / C++20)
• GUI/엔진: Qt 6 (Homebrew)                     • GUI/엔진: Qt 6 (OSGeo4W)
• GIS 엔진: QGIS.app Frameworks                  • GIS 엔진: OSGeo4W qgis-dev (DLLs)
• 보안 금고: Apple Keychain API                 • 보안 금고: Windows DPAPI (wincrypt)
• 배포 형태: ka-hgis.app (애플리케이션 번들)       • 배포 형태: ka-hgis.exe + 포터블 런타임
```

### 1.1 플랫폼 추상화 핵심 구성요소
| 구분 | macOS 구현 | Windows 구현 | 공통 인터페이스 |
|---|---|---|---|
| **계정/키 암호화** | `KaSecretStoreMac.cpp` (Keychain API) | `KaSecretStoreWin.cpp` (DPAPI `CryptProtectData`) | `KaSecretStore.h` |
| **파일 잠금 및 내구성** | `fcntl(F_FULLFSYNC)` / POSIX lock | Win32 `LockFileEx` / `FlushFileBuffers` | `SurveyDurability.h` |
| **GIS 런타임 탐지** | `KaPortableRuntimeMac.cpp` (`/Applications/QGIS.app`) | `KaPortableRuntime.cpp` (`C:\OSGeo4W`) | `KaPortableRuntime.h` |
| **한글/공백 경로 대응** | POSIX UTF-8 표준 경로 | ASCII 정션 링크 (`C:\Users\Public\ka-hgis\rt-*`) | `KaPortableRuntime.cpp` |
| **실행/테스트 서브시스템**| `.app` 번들 / 콘솔 바이너리 | `WIN32_EXECUTABLE FALSE` (콘솔 QTest) | `CMakeLists.txt` |

---

## 2. Windows 빌드 환경 요구 사항

### 2.1 필수 소프트웨어
1. **운영체제**: Windows 10 (버전 21H2 이상) 또는 Windows 11 (64-bit 전용)
2. **Visual Studio 2022**:
   - 버전: 17.4 이상 (Community, Professional, Enterprise 또는 Build Tools)
   - 필수 워크로드: **C++를 사용한 데스크톱 개발 (Desktop development with C++)**
   - 포함 구성요소: `MSVC v143 - VS 2022 C++ x64/x86 빌드 도구`, `최신 Windows 10/11 SDK`
3. **CMake**: 3.21 이상 (`PATH` 환경 변수 등록 권장)
4. **Git for Windows**: 2.40 이상
5. **OSGeo4W (QGIS 4.x 개발자 SDK)**:
   - 권장 설치 경로: `C:\OSGeo4W` (또는 `D:\OSGeo4W`)
   - 필수 패키지 5종:
     * `qgis-dev` (QGIS 코어, GUI, 분석 라이브러리 및 헤더)
     * `qt6-devel` (Qt 6.x 라이브러리 및 MOC/UIC/RCC 도구)
     * `gdal-dev-devel` (GDAL 3.x 개발 라이브러리)
     * `sqlite3-devel` (SpatiaLite 및 속성 저장소)
     * `pdal-dev` (지형 포인트 클라우드 처리 라이브러리)

---

## 3. 원클릭 자동 설치 및 빌드 (가장 권장하는 방법)

관리자 권한으로 PowerShell을 열고 아래 명령어를 순서대로 실행하면 모든 의존성 설치부터 빌드, 검증까지 자동으로 완료됩니다.

### Step 1: 의존성 패키지 자동 설치 (최초 1회)
```powershell
# 관리자 권한 PowerShell 실행
cd <프로젝트_경로>\hgis

# CMake, VS Build Tools, OSGeo4W 패키지 자동 다운로드 및 설치
.\scripts\install-deps.ps1
```

### Step 2: 자동 환경 검사 및 빌드·테스트
```powershell
# 환경 점검(Git/CMake/VS2022/OSGeo4W) -> CMake 설정 -> 빌드 -> CTest -> 스모크 테스트
.\scripts\bootstrap-dev-pc.ps1
```
> 정상 완료 시 `build\Release\ka-hgis.exe` 실행 파일이 생성되며, 183개 자동화 테스트가 수행됩니다.

---

## 4. 수동 빌드 절차 (PowerShell CLI)

터미널에서 직접 빌드 명령어를 제어하고자 할 때 사용하는 절차입니다.

### Step 1: 환경 변수 로드
```powershell
# OSGeo4W, Qt6, QGIS 라이브러리 경로를 현재 세션 PATH에 자동 주입
. .\scripts\dev-env.ps1
```

### Step 2: CMake 프로젝트 생성 (Visual Studio 2022 솔루션)
```powershell
# CMakePresets.json의 'vs' 프리셋을 사용하여 64비트 솔루션 생성
cmake --preset vs

# 또는 직접 옵션을 지정하는 경우:
cmake -B build -G "Visual Studio 17 2022" -A x64 `
  -DOSGEO4W_ROOT="$env:OSGEO4W_ROOT" `
  -DKA_HGIS_BUILD_TESTS=ON
```

### Step 3: 컴파일 및 링크 (Release 모드)
```powershell
# 멀티코어 병렬 빌드 실행
cmake --build build --config Release --parallel
```

### Step 4: 단위 테스트 및 회귀 검증 (CTest)
```powershell
ctest --test-dir build -C Release --output-on-failure --parallel
```

### Step 5: 실행
```powershell
.\scripts\run-ka-hgis.ps1
```

---

## 5. Visual Studio 2022 IDE에서 개발하기

1. **Visual Studio 2022 실행**
2. **[파일] -> [열기] -> [폴더...]** 선택 후 `hgis` 저장소 루트 폴더 선택
3. Visual Studio가 `CMakeLists.txt`와 `CMakePresets.json`을 자동으로 감지합니다.
4. 상단 구성 드롭다운에서 **`Visual Studio 2022 (기본 빌드)`** 및 **`x64-Release`** 선택
5. **[빌드] -> [모두 빌드]** (`Ctrl+Shift+B`) 실행
6. 시작 항목을 `ka-hgis.exe`로 지정 후 `F5` 키를 눌러 디버깅 시작

---

## 6. 독립형 포터블 패키지 제작 (OSGeo4W 없는 PC 배포용)

개발 PC에서 빌드된 실행 파일과 필요한 런타임 DLL, GDAL/PROJ 데이터베이스, Qt 플러그인을 단일 폴더로 압축/패키징합니다.

```powershell
# 포터블 배포 번들 생성 (dist\ka-hgis-portable 폴더 생성)
.\scripts\make-portable.ps1
```

### 배포 번들 구조:
```
dist/ka-hgis-portable/
├── ka-hgis.exe                 # 메인 실행 파일
├── start.bat                   # 원클릭 실행 스크립트 (환경 자동 매핑)
├── apps/
│   ├── qgis-dev/               # qgis_core.dll, qgis_gui.dll 등
│   ├── Qt6/                    # Qt6Core.dll, Qt6Widgets.dll, 플러그인
│   └── gdal-dev/               # gdal.dll 및 gdal 데이터
├── share/proj/                 # proj.db 좌표계 변환 데이터베이스
└── config/                     # 사용자 설정 및 오프라인 캐시
```
- 대상 PC에서는 별도의 설치 프로그램이나 관리자 권한 없이 `start.bat`을 실행하기만 하면 즉시 작동합니다.

---

## 7. 주요 문제 해결 (Windows Troubleshooting)

### 1) 한글 윈도우 계정명 또는 경로 오류
- **현상**: `No dynamic QGIS data provider plugins found` 또는 `srs.db does not exist!` 오류가 발생하며 지도가 뜨지 않음.
- **원인**: QGIS 4.x 코어가 비-ASCII(한글) 경로를 일부 레거시 C API로 처리하여 발생하는 문제.
- **해결책**: `ka-hgis` 내부의 `KaPortableRuntime`이 공용 폴더(`C:\Users\Public\ka-hgis\rt-*`)에 영문 ASCII 정션(Junction)을 자동 생성하여 투명하게 연결하므로 안심하고 사용하시면 됩니다.

### 2) DLL 누락 오류 (`pdal-devcpp210.dll` 등)
- **현상**: 빌드 후 실행 시 특정 DLL이 없다는 윈도우 시스템 경고창 발생.
- **해결책**: 반드시 `. .\scripts\dev-env.ps1` 또는 `.\scripts\run-ka-hgis.ps1`을 통해 실행해야 OSGeo4W의 bin 디렉터리들이 올바른 우선순위로 세팅됩니다.

### 3) Visual Studio LTO 내부 컴파일러 오류 (C2.dll ICE)
- **현상**: MSVC에서 `/GL` (전체 프로그램 최적화) 링크 중 `c2.dll` 내부 충돌 발생.
- **해결책**: `CMakeLists.txt`에 `if(MSVC) set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE OFF)`로 보호 조치가 이미 반영되어 있어 정상 빌드됩니다.
