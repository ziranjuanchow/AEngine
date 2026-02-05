@echo off
setlocal

echo [AEngine] Setting up build environment for Visual Studio 2026...

:: --- Configuration ---
set "VCPKG_ROOT=F:\Temp\vcpkg"
set "VCPKG_CMAKE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
:: ---------------------

:: 1. Validate vcpkg
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo [Error] vcpkg.exe not found at %VCPKG_ROOT%
    pause
    exit /b 1
)

echo [AEngine] Found vcpkg at %VCPKG_ROOT%

:: 2. Find Visual Studio 2026 Developer Command Prompt
set "VS_DEV_CMD="

:: Try VS2026 / VS v18 paths
if exist "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_DEV_CMD=C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_DEV_CMD=C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VS_DEV_CMD=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
)

if "%VS_DEV_CMD%"=="" (
    echo [Error] Could not find Visual Studio 2026 vcvars64.bat. 
    echo Please check your VS 2026 installation path.
    pause
    exit /b 1
)

echo [AEngine] Activating VS 2026 Environment: %VS_DEV_CMD%
call "%VS_DEV_CMD%"

:: 3. Setup PATH to prioritize VS tools and CMake
:: This helps avoid MSYS2 interference
echo [AEngine] Cleaning build cache...
if exist build rd /s /q build
if exist out rd /s /q out

echo [AEngine] Configuring CMake...
:: We use -G "Ninja" if available, otherwise let CMake decide the best VS generator
:: VS 2026 usually comes with Ninja.
where ninja >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [AEngine] Using Ninja generator...
    cmake -B build -S . -G "Ninja" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_CMAKE%"
) else (
    echo [AEngine] Ninja not found, letting CMake choose the best Visual Studio generator...
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_CMAKE%"
)

if %ERRORLEVEL% NEQ 0 (
    echo [Error] CMake configuration failed.
    pause
    exit /b %ERRORLEVEL%
)

echo [AEngine] Building Debug configuration...
cmake --build build --config Debug

if %ERRORLEVEL% NEQ 0 (
    echo [Error] Build failed.
    pause
    exit /b %ERRORLEVEL%
)

echo [AEngine] Running Tests...

cd build



if not exist ..\logs mkdir ..\logs



:: Get current timestamp (YYYYMMDD_HHMMSS)

for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value') do set datetime=%%I

set TIMESTAMP=%datetime:~0,8%_%datetime:~8,6%



echo [AEngine] Logging test results to logs\tests_%TIMESTAMP%.log

ctest -C Debug --output-on-failure > ..\logs\tests_%TIMESTAMP%.log 2>&1



:: Also print to console

type ..\logs\tests_%TIMESTAMP%.log



echo [AEngine] Build and Test Complete!

pause