$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $root 'build/vc9'
$bin = Join-Path $root '.venv/Scripts'
if (!(Test-Path -LiteralPath "$bin/reccmp-reccmp.exe")) { throw 'Run tools/setup-reccmp.ps1 first' }
if (!(Test-Path -LiteralPath "$build/reccmp-build.yml")) { throw 'Run tools/build-vc9.cmd first' }

function Invoke-ReccmpTool {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [string]$LogFile,
        [switch]$AllowFailure
    )
    $exe = Join-Path $bin $Name
    Write-Host ">>> $Name $($Arguments -join ' ')"
    $prevEap = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $output = & $exe @Arguments 2>&1
        $code = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $prevEap
    }
    $lines = @($output | ForEach-Object { "$_" })
    if ($LogFile) {
        $lines | Out-File -FilePath $LogFile -Encoding utf8
    }
    $lines | ForEach-Object { Write-Host $_ }
    if ($code -and -not $AllowFailure) {
        throw "$Name failed with exit $code"
    }
    if ($code -and $AllowFailure) {
        Write-Host "$Name exited $code (expected while layouts still differ)"
    }
}

$oldUtf8 = $env:PYTHONUTF8
$env:PYTHONUTF8 = '1'
Push-Location $build
try {
    Invoke-ReccmpTool -Name 'reccmp-decomplint.exe' -Arguments @('--target', 'VC1', '--warnfail', "$root/src", "$root/third_party/squirrel223")

    $reportDir = Join-Path $build 'reports'
    New-Item -ItemType Directory -Force -Path $reportDir | Out-Null
    $json = Join-Path $reportDir 'VC1.json'
    $prev = Join-Path $reportDir 'VC1.prev.json'
    Invoke-ReccmpTool -Name 'reccmp-reccmp.exe' -Arguments @(
        '--target', 'VC1',
        '--json', $json,
        '--html', (Join-Path $reportDir 'VC1.html'),
        '--svg', (Join-Path $reportDir 'VC1.svg'),
        '--no-color',
        '--print-rec-addr'
    )

    if (Test-Path -LiteralPath $prev) {
        Invoke-ReccmpTool -Name 'reccmp-aggregate.exe' -Arguments @(
            '--diff', $prev, $json,
            '--no-color'
        ) -LogFile (Join-Path $reportDir 'VC1.diff.txt') -AllowFailure
    }
    Copy-Item -LiteralPath $json -Destination $prev -Force

    Invoke-ReccmpTool -Name 'reccmp-datacmp.exe' -Arguments @(
        '--target', 'VC1', '--all', '--no-color', '--print-rec-addr'
    ) -LogFile (Join-Path $reportDir 'VC1-data.txt') -AllowFailure

    Invoke-ReccmpTool -Name 'reccmp-vtable.exe' -Arguments @(
        '--target', 'VC1', '--verbose', '--no-color'
    ) -LogFile (Join-Path $reportDir 'VC1-vtable.txt') -AllowFailure

    Invoke-ReccmpTool -Name 'reccmp-roadmap.exe' -Arguments @(
        '--target', 'VC1',
        '--csv', (Join-Path $reportDir 'VC1-roadmap.csv')
    ) -LogFile (Join-Path $reportDir 'VC1-roadmap.txt') -AllowFailure
}
finally {
    Pop-Location
    $env:PYTHONUTF8 = $oldUtf8
}
exit 0
