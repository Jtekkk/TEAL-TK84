#!/usr/bin/env bash
#
# TEAL TK84 — build & install script (macOS / Linux)
#
# Configures the project, builds a Release, and copies the plugin into the
# standard user plugin folders (handled by COPY_PLUGIN_AFTER_BUILD in CMake).
#
# Usage:
#   ./install.sh                 # fetch JUCE automatically, build & install
#   JUCE_DIR=/path/to/JUCE ./install.sh   # use a local JUCE checkout
#   BUILD_DIR=out ./install.sh   # override the build directory (default: build)
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

echo "==> TEAL TK84 install"
echo "    source:     $SCRIPT_DIR"
echo "    build dir:  $BUILD_DIR"
echo "    build type: $BUILD_TYPE"

if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake is required but was not found on PATH." >&2
    exit 1
fi

# Assemble configure arguments.
CMAKE_ARGS=(-B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE")

# Prefer Ninja when available for faster builds.
if command -v ninja >/dev/null 2>&1; then
    CMAKE_ARGS+=(-G Ninja)
fi

# Honour a local JUCE checkout if provided.
if [[ -n "${JUCE_DIR:-}" ]]; then
    echo "    JUCE_DIR:   $JUCE_DIR"
    CMAKE_ARGS+=(-DJUCE_DIR="$JUCE_DIR")
fi

echo "==> Configuring..."
cmake "${CMAKE_ARGS[@]}"

echo "==> Building (this fetches JUCE on first run and may take a while)..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" --parallel

# Explicitly install the built artefacts to the user plugin folders. This is
# idempotent and works even when the build was already up to date (in which
# case CMake's COPY_PLUGIN_AFTER_BUILD step does not re-run).
ART="$BUILD_DIR/TEAL_TK84_artefacts/$BUILD_TYPE"

case "$(uname -s)" in
    Darwin)
        VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3"
        AU_DEST="$HOME/Library/Audio/Plug-Ins/Components"
        ;;
    *)
        VST3_DEST="$HOME/.vst3"
        AU_DEST=""
        ;;
esac

echo ""
echo "==> Installing..."
if compgen -G "$ART/VST3/"*.vst3 > /dev/null; then
    mkdir -p "$VST3_DEST"
    cp -R "$ART/VST3/"*.vst3 "$VST3_DEST/"
    echo "    VST3 -> $VST3_DEST"
fi
if [[ -n "$AU_DEST" ]] && compgen -G "$ART/AU/"*.component > /dev/null; then
    mkdir -p "$AU_DEST"
    cp -R "$ART/AU/"*.component "$AU_DEST/"
    echo "    AU   -> $AU_DEST"
fi

echo ""
echo "==> Done. Built artefacts are in: $ART/"
echo "    Rescan plugins in your DAW to pick up TEAL TK84."
