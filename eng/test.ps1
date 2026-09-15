#!/usr/bin/env pwsh

$ErrorActionPreference = "Stop"

$ROOT = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$BUILD_DIR = Join-Path $ROOT "build"

$CPU_COUNT = [Environment]::ProcessorCount

Write-Host "==> Configuring Haze (with tests)..."
cmake -S $ROOT -B $BUILD_DIR

Write-Host ""
Write-Host "==> Building tests..."
cmake --build $BUILD_DIR --parallel $CPU_COUNT

Write-Host ""
Write-Host "==> Running tests..."
ctest --test-dir $BUILD_DIR --output-on-failure
