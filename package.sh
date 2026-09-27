#!/bin/bash
# Build the GUI and produce a self-contained portable folder (no source).
#
#   ./package.sh              release
#   ./package.sh Debug        debug
#
# Result: build/dist/CPMToolsQt6-v0.1-preview-win64/
#
# For the combined release archive (runnable build + source) use:
#   powershell -ExecutionPolicy Bypass -File package_release.ps1
set -e

cd "$(dirname "$0")" || exit 1
export PATH=/usr/bin:/ucrt64/bin:$PATH

BUILD_TYPE="${1:-Release}"

# The deployment steps run as POST_BUILD commands on CPMToolsQt6, so a normal
# build already leaves a self-contained build/ directory where the executable
# sits alongside the Qt and MinGW runtime DLLs.
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCPM_PORTABLE=ON
cmake --build build --parallel

# Stage the distribution folder and zip it.
cmake --build build --target portable

echo
ZIP="build/dist/CPMToolsQt6-v0.1-preview-win64.zip"
if [ -f "$ZIP" ]; then
    echo "Portable package: $ZIP"
    echo "  $(du -h "$ZIP" | cut -f1) - unzip anywhere and run CPMToolsQt6.exe"
else
    echo "error: expected $ZIP was not produced" >&2
    exit 1
fi
