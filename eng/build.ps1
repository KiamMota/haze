$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"

$CpuCount = (Get-CimInstance Win32_Processor).NumberOfLogicalProcessors

Write-Host "==> Configuring Haze..."
cmake -S $Root -B $BuildDir -G "MinGW Makefiles"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed."
}

Write-Host ""
Write-Host "Found: $CpuCount CPU cores/logical processors"
Write-Host ""
Write-Host "==> Building Haze..."
Write-Host "    Compiling in parallel using $CpuCount CPU logical processors..."

cmake --build $BuildDir --parallel $CpuCount

if ($LASTEXITCODE -ne 0) {
    throw "Haze build failed."
}

$Executable = Join-Path $BuildDir "haze.exe"

if (-not (Test-Path $Executable)) {
    throw "Build completed, but haze.exe was not found at: $Executable"
}

Write-Host ""
Write-Host "==> Build successful"
Write-Host "    Found: $Executable"
