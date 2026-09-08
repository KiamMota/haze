#!/usr/bin/env bash

set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build"

CPU_COUNT="$(nproc 2>/dev/null || sysctl -n hw.logicalcpu)"

echo "==> Configuring Haze..."
cmake -S "$ROOT" -B "$BUILD_DIR"

echo ""
echo "Found: $CPU_COUNT CPU logical processors"
echo ""
echo "==> Building Haze..."
echo "    Compiling in parallel using $CPU_COUNT CPU cores/logical processors..."

cmake --build "$BUILD_DIR" --parallel "$CPU_COUNT"

EXECUTABLE="$BUILD_DIR/haze"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Build completed, but haze was not found at: $EXECUTABLE" >&2
    exit 1
fi

echo ""
echo "==> Build successful"
echo "    Found: $EXECUTABLE"
