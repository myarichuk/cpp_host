$ErrorActionPreference = "Stop"

param(
    [string]$VcpkgDir = "external/vcpkg",
    [string]$BuildDir = "build",
    [string]$BuildType = "Debug",
    [switch]$NoTests,
    [switch]$Clean,
    [switch]$SkipBootstrap,
    [int]$Jobs = [Environment]::ProcessorCount,
    [switch]$Verbose
)

if ($Verbose) { $VerbosePreference = "Continue" }

function Install-Conan {
    if (-not (Get-Command conan -ErrorAction SilentlyContinue)) {
        Write-Host ">>> Installing Conan..."
        python -m pip install --user conan
    }
}

# Clean build directory if requested
if ($Clean) {
    Write-Host ">>> Cleaning build directory..."
    Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
}

# Bootstrap vcpkg
if (-not $SkipBootstrap) {
    Write-Host ">>> Bootstrapping vcpkg..."
    if (-not (Test-Path "$VcpkgDir\.git")) {
        git clone --depth 1 https://github.com/microsoft/vcpkg.git $VcpkgDir
        Push-Location $VcpkgDir
        git submodule update --init --recursive --depth 1
        Pop-Location
    }
    if (-not (Test-Path "$VcpkgDir/vcpkg.exe")) {
        & "$VcpkgDir\bootstrap-vcpkg.bat" -disableMetrics
    }
}

$env:VCPKG_ROOT = (Resolve-Path $VcpkgDir)
Install-Conan

# Determine whether to use CMakePresets.json
$usePresets = Test-Path "CMakePresets.json"

if ($usePresets) {
    Write-Host ">>> Detected CMakePresets.json. Using presets."
    cmake --preset $BuildType | Out-Null
    cmake --build --preset $BuildType --parallel $Jobs
} else {
    Write-Host ">>> Configuring CMake..."
    $toolchain = Join-Path $env:VCPKG_ROOT "scripts/buildsystems/vcpkg.cmake"
    cmake -S . -B $BuildDir `
        -DCMAKE_BUILD_TYPE=$BuildType `
        -DCMAKE_TOOLCHAIN_FILE=$toolchain `
        -DGH_BUILD_TESTS=$(if ($NoTests) {"OFF"} else {"ON"}) `
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    Write-Host ">>> Building..."
    cmake --build $BuildDir --parallel $Jobs
}

Write-Host "✔ Build complete."
