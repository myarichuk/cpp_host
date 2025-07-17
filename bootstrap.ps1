$ErrorActionPreference = "Stop"

$vcpkgDir = "external/vcpkg"
$toolchainFile = "$vcpkgDir\scripts\buildsystems\vcpkg.cmake"

if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    Write-Host ">>> Installing Python..."
    if (Get-Command winget -ErrorAction SilentlyContinue) {
        winget install --id Python.Python.3 -e --source winget
    }
    else {
        Write-Host "Python is required but winget is not available. Please install Python manually." -ForegroundColor Red
        exit 1
    }
}

if (-not (Get-Command conan -ErrorAction SilentlyContinue)) {
    Write-Host ">>> Installing Conan..."
    python -m pip install --user conan
}

Write-Host ">>> Bootstrapping vcpkg..."

# Clone if needed
if (-not (Test-Path "$vcpkgDir\.git")) {
    git clone https://github.com/microsoft/vcpkg.git $vcpkgDir
    Push-Location $vcpkgDir
    git submodule update --init --recursive
    Pop-Location
}

# Bootstrap vcpkg
& "$vcpkgDir\bootstrap-vcpkg.bat"

# Set env var for editor support
$env:VCPKG_ROOT = Resolve-Path "$vcpkgDir"

Write-Host ">>> Configuring CMake..."
$toolchainFile = Resolve-Path "$vcpkgDir\scripts\buildsystems\vcpkg.cmake"

cmake -B build -S . `
    -DCMAKE_BUILD_TYPE=Debug `
    -DCMAKE_TOOLCHAIN_FILE="$($toolchainFile)" `
    -DGH_BUILD_TESTS=ON

Write-Host ">>> Building..."
cmake --build build --parallel

Write-Host ">>> Done."
