$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $root 'build/vc9'
$compare = Join-Path $root '.venv/Scripts/reccmp-reccmp.exe'
if (!(Test-Path -LiteralPath $compare)) { throw 'Run tools/setup-reccmp.ps1 first' }
if (!(Test-Path -LiteralPath "$build/reccmp-build.yml")) { throw 'Run tools/build-vc9.cmd first' }
$oldUtf8 = $env:PYTHONUTF8
$env:PYTHONUTF8 = '1'
Push-Location $build
try {
    & "$root/.venv/Scripts/reccmp-decomplint.exe" --module VC1 --warnfail "$root/src" "$root/third_party/squirrel223"
    if ($LASTEXITCODE) { throw 'Source marker validation failed' }
    $reportDir = Join-Path $build 'reports'
    New-Item -ItemType Directory -Force -Path $reportDir | Out-Null
    $compareArgs = @('--target', 'VC1', '--json', "$reportDir/VC1.json", '--html', "$reportDir/VC1.html", '--no-color')
    & $compare @compareArgs
    if ($LASTEXITCODE) { throw 'reccmp comparison failed' }
} finally {
    Pop-Location
    $env:PYTHONUTF8 = $oldUtf8
}
