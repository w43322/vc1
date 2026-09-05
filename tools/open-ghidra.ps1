param([string]$GhidraHome = 'D:\ghidra_12.1.2_PUBLIC')
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$projectFile = Join-Path $repoRoot 'ghidra\VirtualCity.gpr'
if (!(Test-Path $projectFile)) { throw 'Run init-ghidra.ps1 first.' }
Start-Process -FilePath (Join-Path $GhidraHome 'ghidraRun.bat') `
    -ArgumentList ('"' + $projectFile + '"') -WindowStyle Hidden
