$ErrorActionPreference = "Stop"

$vcpkgDir = "external/vcpkg"
$toolchainFile = "$vcpkgDir\scripts\buildsystems\vcpkg.cmake"

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
