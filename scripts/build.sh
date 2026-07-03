#!/usr/bin/env bash
# build.sh — Developer build with tests
set -euo pipefail
PROJ_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$PROJ_DIR/build-linux}"
JOBS="$(nproc)"
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -S "$PROJ_DIR"
cmake --build "$BUILD_DIR" -j"$JOBS"
ctest --test-dir "$BUILD_DIR" --output-on-failure
