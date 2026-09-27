#!/bin/bash
# Configure and build the CP/M tools GUI + CLI harness.
#
#   ./build.sh              release build
#   ./build.sh Debug        debug build
#   ./build.sh Release clean
#
# Requires the MSYS2 UCRT64 toolchain (see README-qt.md).
set -e

cd "$(dirname "$0")" || exit 1

# Use the UCRT64 toolchain that ships with the MSYS2 base install.
export PATH=/usr/bin:/ucrt64/bin:$PATH

BUILD_TYPE="${1:-Release}"
DO_CLEAN="$2"

if [ "$DO_CLEAN" = "clean" ]; then
    rm -rf build
fi

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build build --parallel

echo
echo "Build complete:"
echo "  GUI : ./build/CPMToolsQt6.exe"
echo "  CLI : ./build/cpmcli.exe"
echo ""
echo "Release archive : powershell -File package_release.ps1"
