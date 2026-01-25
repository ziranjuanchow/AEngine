@echo off
setlocal

:: Set up VS Environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 call "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

:: Create logs directory
if not exist logs mkdir logs

:: Configure CMake (Only if needed)
if not exist build (
    echo [Configuring CMake...]
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake -GNinja > logs\configure.log 2>&1
    if errorlevel 1 (
        echo [Error] CMake configuration failed. Check logs\configure.log
        exit /b 1
    )
)

:: Build
echo [Building...]
cmake --build build > logs\build.log 2>&1
if errorlevel 1 (
    echo [Error] Build failed. See logs\build.log for details.
    type logs\build.log
    exit /b 1
)

echo [Build Success]
endlocal
