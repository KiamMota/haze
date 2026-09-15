#!/usr/bin/env bash
# eng/test.sh

set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build"

CPU_COUNT="$(nproc 2>/dev/null || sysctl -n hw.logicalcpu)"

echo "==> Configuring Haze (with tests)..."
cmake -S "$ROOT" -B "$BUILD_DIR"

echo ""
echo "==> Building tests..."
cmake --build "$BUILD_DIR" --parallel "$CPU_COUNT"

echo ""
echo "==> Running tests..."
ctest --test-dir "$BUILD_DIR" --output-on-failure
