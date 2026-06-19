#!/usr/bin/env bash
# ============================================================================
# graph-core — Build & Package Script
# Compiles for Linux (native) and Windows (cross-compilation via MinGW)
# ============================================================================
set -euo pipefail

PROJ_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$PROJ_DIR/dist"
JOBS="$(nproc)"

RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

info()  { echo -e "${CYAN}[INFO]${NC} $*"; }
ok()    { echo -e "${GREEN}[OK]${NC} $*"; }
fail()  { echo -e "${RED}[FAIL]${NC} $*"; exit 1; }

# ── Clean dist and builds ───────────────────────────────────────────────────
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"
rm -rf "$PROJ_DIR/build-linux" "$PROJ_DIR/build-windows"

# ── Build Linux ─────────────────────────────────────────────────────────────
info "Building for Linux..."
cmake -B "$PROJ_DIR/build-linux" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=OFF \
  -S "$PROJ_DIR" \
  > /dev/null 2>&1

cmake --build "$PROJ_DIR/build-linux" -j"$JOBS" 2>&1 | tail -3

if [ ! -f "$PROJ_DIR/build-linux/graph-core" ]; then
  fail "Linux build failed — graph-core binary not found"
fi

ok "Linux binary ready"

# Package Linux
mkdir -p "$DIST_DIR/graph-core-linux"
cp "$PROJ_DIR/build-linux/graph-core" "$DIST_DIR/graph-core-linux/"
cp "$PROJ_DIR/graph-core-logo.png" "$DIST_DIR/graph-core-linux/"
cp -r "$PROJ_DIR/recursos" "$DIST_DIR/graph-core-linux/"

# Copy sample graphs if they exist
if [ -d "$PROJ_DIR/muestras" ] && [ "$(ls -A "$PROJ_DIR/muestras")" ]; then
  cp -r "$PROJ_DIR/muestras" "$DIST_DIR/graph-core-linux/"
fi

cd "$DIST_DIR"
tar -czf "graph-core-linux-x86_64.tar.gz" "graph-core-linux/"
ok "Linux package: dist/graph-core-linux-x86_64.tar.gz"

# ── Build Windows (cross-compilation) ──────────────────────────────────────
if command -v x86_64-w64-mingw32-g++ &> /dev/null; then
  info "Building for Windows (MinGW cross-compilation)..."
  cmake -B "$PROJ_DIR/build-windows" \
    -DCMAKE_TOOLCHAIN_FILE="$PROJ_DIR/mingw-toolchain.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -S "$PROJ_DIR" \
    > /dev/null 2>&1

  cmake --build "$PROJ_DIR/build-windows" -j"$JOBS" 2>&1 | tail -3

  if [ ! -f "$PROJ_DIR/build-windows/graph-core.exe" ]; then
    fail "Windows build failed — graph-core.exe not found"
  fi

  ok "Windows binary ready"

  # Package Windows
  mkdir -p "$DIST_DIR/graph-core-windows"
  cp "$PROJ_DIR/build-windows/graph-core.exe" "$DIST_DIR/graph-core-windows/"
  cp "$PROJ_DIR/graph-core-logo.png" "$DIST_DIR/graph-core-windows/"
  cp -r "$PROJ_DIR/recursos" "$DIST_DIR/graph-core-windows/"

  if [ -d "$PROJ_DIR/muestras" ] && [ "$(ls -A "$PROJ_DIR/muestras")" ]; then
    cp -r "$PROJ_DIR/muestras" "$DIST_DIR/graph-core-windows/"
  fi

  cd "$DIST_DIR"
  zip -rq "graph-core-windows-x64.zip" "graph-core-windows/"
  ok "Windows package: dist/graph-core-windows-x64.zip"
else
  echo -e "${RED}[SKIP]${NC} MinGW not found — skipping Windows build"
  echo "  Install: sudo pacman -S mingw-w64-gcc"
fi

# ── Summary ────────────────────────────────────────────────────────────────
echo ""
echo -e "${BOLD}${GREEN}Build complete!${NC}"
echo ""
echo "Packages in dist/:"
ls -lh "$DIST_DIR"/*.tar.gz "$DIST_DIR"/*.zip 2>/dev/null | awk '{print "  " $NF " (" $5 ")"}'
echo ""
echo "To distribute:"
echo "  Linux  → Send graph-core-linux-x86_64.tar.gz — extract and run ./graph-core"
echo "  Windows → Send graph-core-windows-x64.zip — extract and run graph-core.exe"
