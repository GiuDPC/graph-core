#!/usr/bin/env bash
# package.sh — Release build + package for distribution
set -euo pipefail
PROJ_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DIST_DIR="$PROJ_DIR/dist"
JOBS="$(nproc)"

rm -rf "$DIST_DIR" "$PROJ_DIR/build-release"
mkdir -p "$DIST_DIR"

echo "=== Building Release ==="
cmake -B "$PROJ_DIR/build-release" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -S "$PROJ_DIR"
cmake --build "$PROJ_DIR/build-release" -j"$JOBS"

echo "=== Packaging Linux ==="
mkdir -p "$DIST_DIR/graph-core-linux"
cp "$PROJ_DIR/build-release/graph-core" "$DIST_DIR/graph-core-linux/"
[ -f "$PROJ_DIR/graph-core-logo.png" ] && cp "$PROJ_DIR/graph-core-logo.png" "$DIST_DIR/graph-core-linux/"
[ -d "$PROJ_DIR/recursos" ] && cp -r "$PROJ_DIR/recursos" "$DIST_DIR/graph-core-linux/"
[ -d "$PROJ_DIR/muestras" ] && [ "$(ls -A "$PROJ_DIR/muestras")" ] && cp -r "$PROJ_DIR/muestras" "$DIST_DIR/graph-core-linux/"
cd "$DIST_DIR" && tar -czf "graph-core-linux-x86_64.tar.gz" "graph-core-linux/"
echo "  dist/graph-core-linux-x86_64.tar.gz"

# Cross-compile Windows if MinGW available
if command -v x86_64-w64-mingw32-g++ &>/dev/null; then
    echo "=== Packaging Windows ==="
    cmake -B "$PROJ_DIR/build-windows" \
        -DCMAKE_TOOLCHAIN_FILE="$PROJ_DIR/cmake/toolchain-mingw.cmake" \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -S "$PROJ_DIR"
    cmake --build "$PROJ_DIR/build-windows" -j"$JOBS"
    mkdir -p "$DIST_DIR/graph-core-windows"
    cp "$PROJ_DIR/build-windows/graph-core.exe" "$DIST_DIR/graph-core-windows/"
    [ -f "$PROJ_DIR/graph-core-logo.png" ] && cp "$PROJ_DIR/graph-core-logo.png" "$DIST_DIR/graph-core-windows/"
    [ -d "$PROJ_DIR/recursos" ] && cp -r "$PROJ_DIR/recursos" "$DIST_DIR/graph-core-windows/"
    [ -d "$PROJ_DIR/muestras" ] && [ "$(ls -A "$PROJ_DIR/muestras")" ] && cp -r "$PROJ_DIR/muestras" "$DIST_DIR/graph-core-windows/"
    cd "$DIST_DIR" && zip -rq "graph-core-windows-x64.zip" "graph-core-windows/"
    echo "  dist/graph-core-windows-x64.zip"
else
    echo "  [SKIP] Windows — MinGW not found"
fi

echo ""
echo "=== Packages ==="
ls -lh "$DIST_DIR"/*.tar.gz "$DIST_DIR"/*.zip 2>/dev/null | awk '{print "  " $NF " (" $5 ")"}'
