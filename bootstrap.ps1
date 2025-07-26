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
        python -m pip install --user --upgrade conan

        $userBase = $(python -m site --user-base)
        $scriptDir = Join-Path $userBase "Scripts"
        if ($env:PATH -notlike "*$scriptDir*") {
            $env:PATH = "$scriptDir;$env:PATH"
        }
    }
}

function Ensure-BoostDI {
    $boostDiPath = "external/boost-di"
    if (-not (Test-Path $boostDiPath)) {
        Write-Host ">>> Cloning Boost.DI..."
        git clone https://github.com/boost-ext/di.git $boostDiPath
    } else {
        Write-Host ">>> Boost.DI already exists. Skipping clone."
    }
}

if ($Clean) {
    Write-Host ">>> Cleaning build directory..."
    Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
}

# bootstrap vcpkg
if (-not $SkipBootstrap) {
    Write-Host ">>> Checking vcpkg..."

    $isSubmodule = (Test-Path '.gitmodules') -and (Test-Path "$VcpkgDir\.git")
    if ($isSubmodule) {
        Write-Host ">>> vcpkg appears to be a submodule. Updating..."
        git submodule update --init --recursive
    } elseif (-not (Test-Path $VcpkgDir)) {
        Write-Host ">>> Cloning vcpkg..."
        git clone https://github.com/microsoft/vcpkg.git $VcpkgDir
        Push-Location $VcpkgDir
        git submodule update --init --recursive
        Pop-Location
    }

    if (-not (Test-Path "$VcpkgDir/vcpkg.exe")) {
        if (Test-Path "$VcpkgDir\bootstrap-vcpkg.bat") {
            Write-Host ">>> Bootstrapping vcpkg..."
            & "$VcpkgDir\bootstrap-vcpkg.bat" -disableMetrics
        } else {
            Write-Error "vcpkg bootstrap script not found in $VcpkgDir"
            exit 1
        }
    } else {
        Write-Host ">>> vcpkg binary already exists. Skipping bootstrap."
    }
}

$env:VCPKG_ROOT = (Resolve-Path $VcpkgDir)
Install-Conan

# do we use CMakePresets.json
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
