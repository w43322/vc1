# -GhidraHome, else VC1_GHIDRA_HOME / GHIDRA_INSTALL_DIR (tools/workspace.ps1).
param([string]$GhidraHome)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'workspace.ps1')
$GhidraHome = Get-Vc1GhidraHome $GhidraHome
$repoRoot = Split-Path $PSScriptRoot -Parent
$projectFile = Join-Path $repoRoot 'ghidra\VirtualCity.gpr'
if (!(Test-Path $projectFile)) { throw 'Run init-ghidra.ps1 first.' }
Start-Process -FilePath (Join-Path $GhidraHome 'ghidraRun.bat') `
    -ArgumentList ('"' + $projectFile + '"') -WindowStyle Hidden
