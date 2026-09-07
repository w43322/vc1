param([string]$GhidraHome = 'D:\ghidra_12.1.2_PUBLIC')
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$projectDir = Join-Path $repoRoot 'ghidra'
$exePath = Join-Path $repoRoot 'Virtual-City_Win_EN_Full\VirtualCity.Unwrapped.exe'
if (Test-Path (Join-Path $projectDir 'VirtualCity.gpr')) {
    throw 'VirtualCity already exists. Open it instead of importing over it.'
}
if (!(Test-Path $exePath)) { throw "Missing input: $exePath" }
New-Item -ItemType Directory -Force $projectDir | Out-Null
& (Join-Path $GhidraHome 'support\analyzeHeadless.bat') $projectDir VirtualCity `
    -import $exePath -scriptPath $PSScriptRoot -postScript ReplayAnalysis.java `
    -log (Join-Path $projectDir 'analyze-headless.log')
if ($LASTEXITCODE -ne 0) { throw "Ghidra import failed: $LASTEXITCODE" }
