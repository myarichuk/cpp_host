#!/usr/bin/env bash
set -euo pipefail

# -- Configuration --------------------------------------------------------------
VCPKG_DIR="external/vcpkg"
TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"
BUILD_DIR="build"
BUILD_TYPE="Debug"

detect_os() {
    case "$(uname -s)" in
        Linux)
            if ! command -v apt-get >/dev/null 2>&1; then
                echo "Unsupported Linux distribution: apt-get not found" >&2
                exit 1
            fi
            OS_TYPE=linux
            ;;
        Darwin)
            if ! command -v brew >/dev/null 2>&1; then
                echo "Homebrew is required but not found. Install from https://brew.sh/" >&2
                exit 1
            fi
            OS_TYPE=macos
            ;;
        *)
            echo "Unsupported operating system: $(uname -s)" >&2
            exit 1
            ;;
    esac
}

install_packages() {
    case "$OS_TYPE" in
        linux)
            sudo apt-get update
            sudo apt-get install -y git build-essential cmake curl pkg-config unzip tar
            ;;
        macos)
            brew update
            brew install git cmake curl
            ;;
    esac
}

realpath_f() {
    if command -v realpath >/dev/null 2>&1; then
        realpath "$1"
    else
        python3 - "$1" <<'EOF'
import os,sys
print(os.path.realpath(sys.argv[1]))
EOF
    fi
}

detect_os
install_packages

echo ">>> Bootstrapping vcpkg..."

# Clone vcpkg if it isn't already there
if [ ! -d "$VCPKG_DIR/.git" ]; then
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
    ( cd "$VCPKG_DIR" && git submodule update --init --recursive )
fi

# Build the vcpkg executable (only needed the first time)
"$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics

# Export VCPKG_ROOT so IDEs/CMake-presets can discover it
VCPKG_ROOT="$(realpath_f "$VCPKG_DIR")"
export VCPKG_ROOT

echo ">>> Configuring CMake..."
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DGH_BUILD_TESTS=ON

echo ">>> Building..."
cmake --build "$BUILD_DIR" --parallel

echo ">>> Done."
