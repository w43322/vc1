# -GhidraHome, else VC1_GHIDRA_HOME / GHIDRA_INSTALL_DIR (tools/workspace.ps1).
param([string]$GhidraHome)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'workspace.ps1')
$GhidraHome = Get-Vc1GhidraHome $GhidraHome
$repoRoot = Split-Path $PSScriptRoot -Parent
$projectDir = Join-Path $repoRoot 'ghidra'
$exePath = Join-Path $repoRoot 'Virtual-City_Win_EN_Full\VirtualCity.Unwrapped.exe'
if (Test-Path (Join-Path $projectDir 'VirtualCity.gpr')) {
    throw 'VirtualCity already exists. Open it instead of importing over it.'
}
if (!(Test-Path $exePath)) { throw "Missing input: $exePath" }
$expected = 'bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695'
if ((Get-FileHash -LiteralPath $exePath -Algorithm SHA256).Hash.ToLower() -ne $expected) {
    throw 'Original SHA256 mismatch'
}
# Initial binary analysis only. Import recovered names with import-ghidra.ps1.
New-Item -ItemType Directory -Force $projectDir | Out-Null
& (Join-Path $GhidraHome 'support\analyzeHeadless.bat') $projectDir VirtualCity `
    -import $exePath `
    -log (Join-Path $projectDir 'analyze-headless.log')
if ($LASTEXITCODE -ne 0) { throw "Ghidra import failed: $LASTEXITCODE" }
