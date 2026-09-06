param([string]$GhidraHome = 'D:\ghidra_12.1.2_PUBLIC')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $root 'build/vc9'
$importer = Join-Path $root '.venv/Scripts/reccmp-ghidra-import.exe'
$projectFile = Join-Path $root 'ghidra/VirtualCity.gpr'
if (!(Test-Path -LiteralPath $importer)) { throw 'Run tools/setup-reccmp.ps1 first' }
if (!(Test-Path -LiteralPath "$build/reccmp-build.yml")) { throw 'Run tools/build-vc9.cmd first' }
if (!(Test-Path -LiteralPath $projectFile)) { throw 'Run tools/init-ghidra.ps1 first' }
if (!(Test-Path -LiteralPath $GhidraHome)) { throw "Missing Ghidra install: $GhidraHome" }

# GUI importer looks next to reccmp-project.yml; headless only needs the copy in build/vc9.
Copy-Item -LiteralPath "$build/reccmp-build.yml" -Destination "$root/reccmp-build.yml" -Force

$oldUtf8 = $env:PYTHONUTF8
$oldGhidra = $env:GHIDRA_INSTALL_DIR
$env:PYTHONUTF8 = '1'
$env:GHIDRA_INSTALL_DIR = $GhidraHome
Push-Location $build
try {
    & $importer --target VC1 --local-project-name VirtualCity --local-project-dir "$root/ghidra" --file /VirtualCity.Unwrapped.exe
    if ($LASTEXITCODE) { throw "reccmp-ghidra-import failed: $LASTEXITCODE" }
} finally {
    Pop-Location
    $env:PYTHONUTF8 = $oldUtf8
    $env:GHIDRA_INSTALL_DIR = $oldGhidra
}
