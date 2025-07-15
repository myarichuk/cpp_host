#!/usr/bin/env bash
set -euo pipefail

# -- Configuration --------------------------------------------------------------
VCPKG_DIR="external/vcpkg"
TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"
BUILD_DIR="build"
BUILD_TYPE="Debug"

echo ">>> Bootstrapping vcpkg..."

# Clone vcpkg if it isn't already there
if [ ! -d "$VCPKG_DIR/.git" ]; then
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
    ( cd "$VCPKG_DIR" && git submodule update --init --recursive )
fi

# Build the vcpkg executable (only needed the first time)
"$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics

# Export VCPKG_ROOT so IDEs/CMake-presets can discover it
export VCPKG_ROOT="$(realpath "$VCPKG_DIR")"

echo ">>> Configuring CMake..."
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DGH_BUILD_TESTS=ON

echo ">>> Building..."
cmake --build "$BUILD_DIR" --parallel

echo ">>> Done."
