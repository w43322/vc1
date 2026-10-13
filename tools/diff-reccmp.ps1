# Verbose asm diff for one or more original addresses, without reccmp's PDB
# parsing noise. Usage: tools/diff-reccmp.ps1 0x462900 [0x4a8de0 ...]
param([Parameter(Mandatory = $true, ValueFromRemainingArguments = $true)][string[]]$Address)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $root 'build/vc9'
$exe = Join-Path $root '.venv/Scripts/reccmp-reccmp.exe'
if (!(Test-Path -LiteralPath "$build/reccmp-build.yml")) { throw 'Run tools/build-vc9.cmd first' }

$oldUtf8 = $env:PYTHONUTF8
$env:PYTHONUTF8 = '1'
Push-Location $build
try {
    foreach ($a in $Address) {
        $ErrorActionPreference = 'Continue'
        $lines = & $exe --target VC1 --verbose $a --no-color 2>&1 | ForEach-Object { "$_" }
        $ErrorActionPreference = 'Stop'
        $lines | Where-Object {
            $_ -notmatch '^\[INFO\]' -and
            $_ -notmatch 'Dropped duplicate address' -and
            $_ -notmatch 'collides with previous staged match'
        } | ForEach-Object { Write-Host $_ }
    }
}
finally {
    Pop-Location
    $env:PYTHONUTF8 = $oldUtf8
}
