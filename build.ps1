#Requires -Version 7.0
<#
.SYNOPSIS
    AEngine Build Script - PowerShell Version
.DESCRIPTION
    Builds the AEngine project using Visual Studio 2026 and vcpkg.
    Handles environment setup, CMake configuration, build, and tests.
.PARAMETER Install
    Run vcpkg install before building
.PARAMETER ForceInstall
    Force reinstall all vcpkg dependencies (clears vcpkg_installed)
.PARAMETER Clean
    Only clean the build directory, don't rebuild
.PARAMETER NoBuild
    Configure only, skip the build step
.PARAMETER NoTest
    Skip running tests after build
.PARAMETER ShowOutput
    Show detailed output from CMake and build tools
.EXAMPLE
    .\build.ps1
    .\build.ps1 -Install
    .\build.ps1 -ForceInstall -ShowOutput
#>

[CmdletBinding()]
param(
    [switch]$Install,
    [switch]$ForceInstall,
    [switch]$Clean,
    [switch]$NoBuild,
    [switch]$NoTest,
    [switch]$ShowOutput,
    [string]$VcpkgRoot
)

# ============================================================================
# Configuration
# ============================================================================
$ErrorActionPreference = "Stop"

$resolvedVcpkgRoot = if ($VcpkgRoot) {
    $VcpkgRoot
}
elseif ($env:AENGINE_VCPKG_ROOT) {
    $env:AENGINE_VCPKG_ROOT
}
elseif ($env:VCPKG_ROOT) {
    $env:VCPKG_ROOT
}
else {
    "F:\Temp\vcpkg"
}

$Config = @{
    VcpkgRoot  = $resolvedVcpkgRoot
    VSVersion  = "18"
    VSEditions = @("Professional", "Enterprise", "Community")
    BuildDir   = "build"
    OutDir     = "out"
    LogDir     = "logs"
    ShowOutput = $ShowOutput.IsPresent
}

# ============================================================================
# Helper Functions
# ============================================================================

function Write-Status {
    param([string]$Message, [string]$Type = "Info")
    $prefix = switch ($Type) {
        "Info" { "[AEngine]" }
        "Success" { "[AEngine] +" }
        "Warning" { "[AEngine] !" }
        "Error" { "[AEngine] X" }
        default { "[AEngine]" }
    }
    $color = switch ($Type) {
        "Info" { "Cyan" }
        "Success" { "Green" }
        "Warning" { "Yellow" }
        "Error" { "Red" }
        default { "White" }
    }
    Write-Host "$prefix $Message" -ForegroundColor $color
}

function Test-VcpkgInstalled {
    $vcpkgExe = Join-Path $Config.VcpkgRoot "vcpkg.exe"
    if (-not (Test-Path $vcpkgExe)) {
        Write-Status "vcpkg.exe not found at $($Config.VcpkgRoot)" -Type Error
        return $false
    }

    # Manifest mode writes dependency state under <vcpkgRoot>\buildtrees.
    # Validate write access early so users get a clear error message.
    $buildtreesDir = Join-Path $Config.VcpkgRoot "buildtrees"
    if (-not (Test-Path $buildtreesDir)) {
        Write-Status "vcpkg buildtrees directory not found at $buildtreesDir" -Type Error
        return $false
    }
    $probeFile = Join-Path $buildtreesDir ".aengine_write_probe"
    try {
        Set-Content -Path $probeFile -Value "probe" -Encoding ASCII -ErrorAction Stop
        Remove-Item $probeFile -Force -ErrorAction SilentlyContinue
    }
    catch {
        Write-Status "No write permission for $buildtreesDir" -Type Error
        Write-Status "Use -VcpkgRoot <path> or set AENGINE_VCPKG_ROOT/VCPKG_ROOT to a writable vcpkg clone." -Type Warning
        return $false
    }

    Write-Status "Found vcpkg at $($Config.VcpkgRoot)"
    return $true
}

function Find-VSInstallPath {
    foreach ($edition in $Config.VSEditions) {
        $vsPath = "C:\Program Files\Microsoft Visual Studio\$($Config.VSVersion)\$edition"
        if (Test-Path $vsPath) {
            Write-Status "Found VS 2026 ($edition)"
            return $vsPath
        }
    }
    Write-Status "Could not find Visual Studio 2026" -Type Error
    return $null
}

function Initialize-VSEnvironment {
    param([string]$VSInstallPath)
    
    Write-Status "Activating VS 2026 Environment..."
    
    # Use vcvars64.bat (more reliable than Launch-VsDevShell.ps1)
    $vcvarsPath = Join-Path $VSInstallPath "VC\Auxiliary\Build\vcvars64.bat"
    
    if (-not (Test-Path $vcvarsPath)) {
        Write-Status "vcvars64.bat not found at $vcvarsPath" -Type Error
        return $false
    }
    
    # Create temporary batch file
    $tempBatch = Join-Path $env:TEMP "aengine_vsenv_$(Get-Random).bat"
    $tempOutput = Join-Path $env:TEMP "aengine_vsenv_$(Get-Random).txt"
    
    try {
        # Write batch script
        @"
@echo off
call "$vcvarsPath" >nul 2>&1
if errorlevel 1 exit /b 1
set >"$tempOutput"
"@ | Set-Content -Path $tempBatch -Encoding ASCII
        
        # Execute batch file
        $process = Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$tempBatch`"" -Wait -PassThru -NoNewWindow
        
        if ($process.ExitCode -ne 0) {
            Write-Status "vcvars64.bat execution failed" -Type Error
            return $false
        }
        
        if (-not (Test-Path $tempOutput)) {
            Write-Status "Failed to capture environment variables" -Type Error
            return $false
        }
        
        # Parse and apply environment variables
        $envLines = Get-Content $tempOutput -Encoding OEM
        foreach ($line in $envLines) {
            if ($line -match '^([^=]+)=(.*)$') {
                $name = $matches[1]
                $value = $matches[2]
                # Skip some special variables that shouldn't be overwritten
                if ($name -notin @('PROMPT', 'TEMP', 'TMP')) {
                    [Environment]::SetEnvironmentVariable($name, $value, "Process")
                }
            }
        }
        
        # Verify environment was set by checking for cl.exe
        $clPath = Get-Command cl.exe -ErrorAction SilentlyContinue
        if (-not $clPath) {
            Write-Status "VS environment not properly initialized (cl.exe not found)" -Type Error
            return $false
        }
        
        Write-Status "VS environment initialized" -Type Success
        return $true
    }
    catch {
        Write-Status "Failed to initialize VS environment: $_" -Type Error
        return $false
    }
    finally {
        # Cleanup temp files
        Remove-Item $tempBatch -Force -ErrorAction SilentlyContinue
        Remove-Item $tempOutput -Force -ErrorAction SilentlyContinue
    }
}

function Stop-AEngineProcesses {
    $processes = @("AEngine", "AEngineTests")
    
    foreach ($proc in $processes) {
        Get-Process -Name $proc -ErrorAction SilentlyContinue | ForEach-Object {
            Write-Status "Stopping process: $($_.Name) (PID: $($_.Id))" -Type Warning
            $_ | Stop-Process -Force
        }
    }
    
    # Give processes time to fully terminate
    Start-Sleep -Milliseconds 500
}

function Stop-StaleBuildProcesses {
    param([string]$ProjectRoot, [string]$BuildDir)

    $escapedProjectRoot = [Regex]::Escape($ProjectRoot)
    $escapedBuildDir = [Regex]::Escape((Join-Path $ProjectRoot $BuildDir))
    $targets = @("cmake.exe", "ninja.exe", "ctest.exe")

    try {
        $candidates = Get-CimInstance Win32_Process -ErrorAction Stop | Where-Object {
            $_.Name -in $targets -and
            $_.CommandLine -and
            ($_.CommandLine -match $escapedProjectRoot -or $_.CommandLine -match $escapedBuildDir)
        }
    }
    catch {
        # Some constrained environments don't expose Win32_Process.
        return
    }

    foreach ($proc in $candidates) {
        Write-Status "Stopping stale build process: $($proc.Name) (PID: $($proc.ProcessId))" -Type Warning
        Stop-Process -Id $proc.ProcessId -Force -ErrorAction SilentlyContinue
    }

    if ($candidates.Count -gt 0) {
        Start-Sleep -Milliseconds 500
    }
}

function Remove-BuildDirectory {
    param([string]$Path)
    
    if (-not (Test-Path $Path)) {
        return $true
    }
    
    Write-Status "Removing $Path..."
    
    # Method 1: PowerShell Remove-Item
    try {
        Remove-Item -Path $Path -Recurse -Force -ErrorAction Stop
        return $true
    }
    catch {
        Write-Status "PowerShell cleanup failed, trying cmd.exe..." -Type Warning
    }
    
    Start-Sleep -Seconds 1
    
    # Method 2: cmd.exe rd (sometimes works when PowerShell doesn't)
    cmd /c "rd /s /q `"$Path`"" 2>&1 | Out-Null
    if (-not (Test-Path $Path)) {
        return $true
    }
    
    # Method 3: robocopy empty-folder trick (very aggressive)
    Write-Status "Trying robocopy cleanup method..." -Type Warning
    $emptyDir = Join-Path $env:TEMP "AEngine_Empty_$(Get-Random)"
    try {
        New-Item -ItemType Directory -Path $emptyDir | Out-Null
        robocopy $emptyDir $Path /MIR /NFL /NDL /NJH /NJS /nc /ns /np 2>&1 | Out-Null
        Remove-Item $Path -Recurse -Force -ErrorAction SilentlyContinue
        if (-not (Test-Path $Path)) {
            return $true
        }
    }
    catch {
        # Continue to error
    }
    finally {
        Remove-Item $emptyDir -Force -ErrorAction SilentlyContinue
    }
    
    # If all methods fail, give user guidance
    Write-Status "Failed to remove $Path - directory is locked" -Type Error
    Write-Status "Please close any applications that might be using this directory:" -Type Warning
    Write-Status "  - Visual Studio / VS Code" -Type Warning
    Write-Status "  - File Explorer windows" -Type Warning
    Write-Status "  - Any terminal sessions in that directory" -Type Warning
    return $false
}

function Invoke-VcpkgInstall {
    param([switch]$Force)
    
    $vcpkgExe = Join-Path $Config.VcpkgRoot "vcpkg.exe"
    
    if ($Force) {
        Write-Status "Force reinstalling dependencies..."
        $vcpkgInstalled = "vcpkg_installed"
        if (Test-Path $vcpkgInstalled) {
            Remove-Item -Path $vcpkgInstalled -Recurse -Force
        }
    }
    else {
        Write-Status "Installing dependencies via vcpkg..."
    }
    
    if ($Config.ShowOutput) {
        & $vcpkgExe install --triplet=x64-windows
    }
    else {
        & $vcpkgExe install --triplet=x64-windows 2>&1 | Out-Null
    }
    
    if ($LASTEXITCODE -ne 0) {
        Write-Status "vcpkg install failed (exit code: $LASTEXITCODE)" -Type Error
        return $false
    }
    
    Write-Status "Dependencies installed" -Type Success
    return $true
}

function Invoke-CMakeConfigure {
    Write-Status "Configuring CMake..."
    
    $vcpkgCMake = Join-Path $Config.VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
    $buildDir = $Config.BuildDir
    
    # Check for Ninja
    $ninjaPath = Get-Command ninja -ErrorAction SilentlyContinue
    
    $cmakeArgs = @(
        "-B", $buildDir
        "-S", "."
        "-DCMAKE_TOOLCHAIN_FILE=$vcpkgCMake"
    )
    
    if ($ninjaPath) {
        Write-Status "Using Ninja generator..."
        $cmakeArgs += @("-G", "Ninja")
    }
    else {
        Write-Status "Ninja not found, using default VS generator..."
    }
    
    if ($Config.ShowOutput) {
        cmake @cmakeArgs
    }
    else {
        cmake @cmakeArgs 2>&1 | Out-Null
    }
    
    if ($LASTEXITCODE -ne 0) {
        Write-Status "CMake configuration failed (exit code: $LASTEXITCODE)" -Type Error
        return $false
    }
    
    Write-Status "CMake configuration complete" -Type Success
    return $true
}

function Invoke-Build {
    Write-Status "Building Debug configuration..."
    
    $buildDir = $Config.BuildDir
    
    if ($Config.ShowOutput) {
        cmake --build $buildDir --config Debug
    }
    else {
        cmake --build $buildDir --config Debug 2>&1 | Out-Null
    }
    
    if ($LASTEXITCODE -ne 0) {
        Write-Status "Build failed (exit code: $LASTEXITCODE)" -Type Error
        return $false
    }
    
    Write-Status "Build complete" -Type Success
    return $true
}

function Invoke-Tests {
    Write-Status "Running tests..."
    
    # Ensure log directory exists
    $logDir = $Config.LogDir
    if (-not (Test-Path $logDir)) {
        New-Item -ItemType Directory -Path $logDir | Out-Null
    }
    
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $logFile = Join-Path $logDir "tests_$timestamp.log"
    
    Write-Status "Logging test results to $logFile"
    
    Push-Location $Config.BuildDir
    try {
        $testOutput = ctest -C Debug --output-on-failure 2>&1
        $testOutput | Out-File -FilePath "..\$logFile" -Encoding utf8
        
        # Always show test output
        $testOutput | ForEach-Object { Write-Host $_ }
        
        if ($LASTEXITCODE -ne 0) {
            Write-Status "Some tests failed" -Type Warning
            return $false
        }
        
        Write-Status "All tests passed" -Type Success
        return $true
    }
    finally {
        Pop-Location
    }
}

# ============================================================================
# Main Script
# ============================================================================

function Main {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "       AEngine Build System v2.0       " -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    
    # Step 1: Validate vcpkg
    if (-not (Test-VcpkgInstalled)) {
        return 1
    }
    
    # Step 2: Find and initialize VS environment
    $vsInstallPath = Find-VSInstallPath
    if (-not $vsInstallPath) {
        return 1
    }
    
    if (-not (Initialize-VSEnvironment -VSInstallPath $vsInstallPath)) {
        return 1
    }
    
    # Step 3: Stop running processes
    Stop-AEngineProcesses
    Stop-StaleBuildProcesses -ProjectRoot (Resolve-Path ".").Path -BuildDir $Config.BuildDir
    
    # Step 4: Clean build directories (best effort)
    Write-Status "Cleaning build cache..."
    if (-not (Remove-BuildDirectory -Path $Config.BuildDir)) {
        Write-Status "Continuing with incremental build (directory locked)" -Type Warning
    }
    Remove-BuildDirectory -Path $Config.OutDir | Out-Null
    
    # If clean-only mode, exit here
    if ($Clean) {
        Write-Status "Clean complete" -Type Success
        return 0
    }
    
    # Step 5: vcpkg install (optional)
    if ($Install -or $ForceInstall) {
        if (-not (Invoke-VcpkgInstall -Force:$ForceInstall)) {
            return 1
        }
    }
    
    # Step 6: CMake configure
    if (-not (Invoke-CMakeConfigure)) {
        return 1
    }
    
    # Step 7: Build (unless skipped)
    if (-not $NoBuild) {
        if (-not (Invoke-Build)) {
            return 1
        }
    }
    
    # Step 8: Run tests (unless skipped)
    if (-not $NoTest -and -not $NoBuild) {
        Invoke-Tests | Out-Null  # Don't fail on test failures
    }
    
    Write-Host ""
    Write-Status "Build and Test Complete!" -Type Success
    Write-Host ""
    
    return 0
}

# Run main and exit with appropriate code
$exitCode = Main
exit $exitCode
