#!/usr/bin/env bash
# clean.sh — Remove all build directories
set -euo pipefail
PROJ_DIR="$(cd "$(dirname "$0")/.." && pwd)"
rm -rf "$PROJ_DIR/build-linux" "$PROJ_DIR/build-release" "$PROJ_DIR/build-windows" "$PROJ_DIR/dist"
echo "Cleaned."
