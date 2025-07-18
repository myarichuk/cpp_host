#!/usr/bin/env bash
set -euo pipefail

# ----------------------------------------
# Defaults
# ----------------------------------------
VCPKG_DIR="external/vcpkg"
BUILD_DIR="build"
BUILD_TYPE="Debug"
BUILD_TESTS=ON
CLEAN_BUILD=false
NO_VCPKG_BOOTSTRAP=false
OS_TYPE=""
CONCURRENCY=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
VERBOSE=false

# Colors
GREEN="\033[0;32m"
CYAN="\033[0;36m"
RESET="\033[0m"

log() {
  echo -e "${CYAN}>>>${RESET} $*"
}

log_success() {
  echo -e "${GREEN}✔${RESET} $*"
}

usage() {
  cat <<EOF
Usage: $(basename "$0") [options]

Options:
  -h, --help              Show this help and exit
  --vcpkg-dir DIR         Where to clone/build vcpkg (default: $VCPKG_DIR)
  --build-dir DIR         CMake build dir (default: $BUILD_DIR)
  --build-type TYPE       CMake build type (default: $BUILD_TYPE)
  --no-tests              Disable building tests (default: ON)
  --clean                 Remove build dir before configuring
  --skip-bootstrap        Assume vcpkg is already bootstrapped
  --jobs N                Parallel build jobs (default: auto = $CONCURRENCY)
  --verbose               Enable verbose script output
EOF
  exit 1
}

# ----------------------------------------
# Parse CLI
# ----------------------------------------
while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help) usage ;;
    --vcpkg-dir) VCPKG_DIR="$2"; shift 2 ;;
    --build-dir) BUILD_DIR="$2"; shift 2 ;;
    --build-type) BUILD_TYPE="$2"; shift 2 ;;
    --no-tests) BUILD_TESTS=OFF; shift ;;
    --clean) CLEAN_BUILD=true; shift ;;
    --skip-bootstrap) NO_VCPKG_BOOTSTRAP=true; shift ;;
    --jobs) CONCURRENCY="$2"; shift 2 ;;
    --verbose) VERBOSE=true; shift ;;
    *) echo "Unknown option: $1"; usage ;;
  esac
done

if [[ "$VERBOSE" == true ]]; then
  set -x
fi

echo -e "${CYAN}=== Configuration ===${RESET}"
echo "VCPKG_DIR:     $VCPKG_DIR"
echo "BUILD_DIR:     $BUILD_DIR"
echo "BUILD_TYPE:    $BUILD_TYPE"
echo "Build tests:   $BUILD_TESTS"
echo "Clean build:   $CLEAN_BUILD"
echo "Skip vcpkg bs: $NO_VCPKG_BOOTSTRAP"
echo "Jobs:          $CONCURRENCY"
echo

# ----------------------------------------
# OS Detection
# ----------------------------------------
detect_os() {
  local uname_s
  uname_s="$(uname -s)"
  case "$uname_s" in
    Linux)
      if grep -qi microsoft /proc/version 2>/dev/null; then
        log "Detected WSL"
      fi
      if command -v apt-get &>/dev/null; then
        OS_TYPE=linux_apt
      elif command -v dnf &>/dev/null; then
        OS_TYPE=linux_dnf
      else
        echo "Unsupported Linux distro. Only apt or dnf supported." >&2
        exit 1
      fi
      ;;
    Darwin)
      OS_TYPE=macos
      ;;
    *)
      echo "Unsupported OS: $uname_s" >&2
      exit 1
      ;;
  esac
}

# ----------------------------------------
# Install packages
# ----------------------------------------
install_packages() {
  case "$OS_TYPE" in
    linux_apt)
      sudo apt-get update -qq
      sudo apt-get install -y git build-essential cmake curl pkg-config unzip tar python3 python3-pip
      ;;
    linux_dnf)
      sudo dnf install -y git cmake curl make gcc-c++ pkgconf-pkg-config unzip tar python3 python3-pip
      ;;
    macos)
      brew update
      brew install git cmake curl python autoconf automake libtool pkg-config ninja
      ;;
  esac
}

# ----------------------------------------
# Python & Conan
# ----------------------------------------
install_conan() {
  if ! command -v conan &>/dev/null; then
    log "Installing Conan via pip3..."
    if command -v pip3 &>/dev/null; then
      pip3 install --upgrade --user conan
      export PATH="$HOME/.local/bin:$PATH"
    else
      echo "pip3 not found. Please install Python3/pip first." >&2
      exit 1
    fi
  fi
}

# ----------------------------------------
# realpath fallback
# ----------------------------------------
realpath_f() {
  if command -v realpath &>/dev/null; then
    realpath "$1"
  else
    python3 - <<EOF
import os,sys
print(os.path.realpath(sys.argv[1]))
EOF
  fi
}

# ----------------------------------------
# Main flow
# ----------------------------------------
detect_os
install_packages
install_conan

[[ "$CLEAN_BUILD" == true ]] && log "Cleaning build dir..." && rm -rf "$BUILD_DIR"

# Bootstrap vcpkg
if [[ "$NO_VCPKG_BOOTSTRAP" == false ]]; then
  log "Bootstrapping vcpkg..."
  if [[ ! -d "$VCPKG_DIR/.git" ]]; then
    git clone --depth=1 https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
    (cd "$VCPKG_DIR" && git submodule update --init --recursive --depth=1)
  fi
  if [[ ! -x "$VCPKG_DIR/vcpkg" ]]; then
    "$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics
  else
    log "vcpkg already bootstrapped. Skipping."
  fi
fi

VCPKG_ROOT="$(realpath_f "$VCPKG_DIR")"
export VCPKG_ROOT

# ----------------------------------------
# CMake Preset or Manual
# ----------------------------------------
if [[ -f "CMakePresets.json" ]]; then
  log "Detected CMakePresets.json. Using presets."
  cmake --preset "$BUILD_TYPE"
  cmake --build --preset "$BUILD_TYPE" --parallel "$CONCURRENCY"
else
  log "Configuring CMake manually..."
  cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DGH_BUILD_TESTS="$BUILD_TESTS" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

  log "Building (jobs=$CONCURRENCY)..."
  cmake --build "$BUILD_DIR" --parallel "$CONCURRENCY"
fi

log_success "Build complete."
