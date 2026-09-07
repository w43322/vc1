param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Offset
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $root 'build/vc9'
$exe = Join-Path $root '.venv/Scripts/reccmp-stackcmp.exe'
if (!(Test-Path -LiteralPath $exe)) { throw 'Run tools/setup-reccmp.ps1 first' }
if (!(Test-Path -LiteralPath "$build/reccmp-build.yml")) { throw 'Run tools/build-vc9.cmd first' }
$oldUtf8 = $env:PYTHONUTF8
$env:PYTHONUTF8 = '1'
Push-Location $build
try {
    & $exe --target VC1 $Offset
    if ($LASTEXITCODE) { throw "reccmp-stackcmp failed with exit $LASTEXITCODE" }
}
finally {
    Pop-Location
    $env:PYTHONUTF8 = $oldUtf8
}
