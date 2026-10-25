# Full reccmp pass: decomplint, asmcmp (JSON/HTML/SVG), aggregate diffs, datacmp,
# vtable, roadmap, progress. Full tool output goes to build/vc9/reports/*.txt;
# the console gets a summary. Only decomplint and asmcmp fail the script.
#
#   -Full           also echo every tool's full output
#   -ResetBaseline  make this run the round baseline (VC1.base.json); do it
#                   after a commit so "since baseline" covers the whole round
#   -UpdateStatus   copy the progress SVG to docs/status.svg (shown in README)
param([switch]$Full, [switch]$ResetBaseline, [switch]$UpdateStatus)

$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$build = Join-Path $root 'build/vc9'
$bin = Join-Path $root '.venv/Scripts'
if (!(Test-Path -LiteralPath "$bin/reccmp-reccmp.exe")) { throw 'Run tools/setup-reccmp.ps1 first' }
if (!(Test-Path -LiteralPath "$build/reccmp-build.yml")) { throw 'Run tools/build-vc9.cmd first' }

$summaryPattern = '^\[(ERROR|WARNING)\]|Ambiguous match|collides with|not found|Traceback|Error:'

function Invoke-ReccmpTool {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [Parameter(Mandatory = $true)][string]$LogFile,
        [switch]$AllowFailure,
        [switch]$ShowAll
    )
    $exe = Join-Path $bin $Name
    Write-Host ">>> $Name -> $(Split-Path $LogFile -Leaf)"
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
    $lines | Out-File -FilePath $LogFile -Encoding utf8
    if ($Full -or $ShowAll) {
        $lines | ForEach-Object { Write-Host $_ }
    }
    else {
        $lines | Where-Object { $_ -match $summaryPattern } | Sort-Object -Unique |
            ForEach-Object { Write-Host "    $_" }
    }
    if ($code -and -not $AllowFailure) {
        throw "$Name failed with exit $code (see $LogFile)"
    }
    if ($code -and $AllowFailure) {
        Write-Host "    $Name exited $code (expected while layouts still differ)"
    }
}

$oldUtf8 = $env:PYTHONUTF8
$env:PYTHONUTF8 = '1'
Push-Location $build
try {
    $reportDir = Join-Path $build 'reports'
    New-Item -ItemType Directory -Force -Path $reportDir | Out-Null

    Invoke-ReccmpTool -Name 'reccmp-decomplint.exe' -Arguments @(
        '--target', 'VC1', '--warnfail', "$root/src", "$root/third_party/squirrel223"
    ) -LogFile (Join-Path $reportDir 'VC1-decomplint.txt') -ShowAll

    $json = Join-Path $reportDir 'VC1.json'
    $prev = Join-Path $reportDir 'VC1.prev.json'
    $base = Join-Path $reportDir 'VC1.base.json'
    Invoke-ReccmpTool -Name 'reccmp-reccmp.exe' -Arguments @(
        '--target', 'VC1',
        '--json', $json,
        '--html', (Join-Path $reportDir 'VC1.html'),
        '--svg', (Join-Path $reportDir 'VC1.svg'),
        '--no-color',
        '--print-rec-addr'
    ) -LogFile (Join-Path $reportDir 'VC1-asmcmp.txt')

    if (Test-Path -LiteralPath $prev) {
        Write-Host '--- since previous run'
        Invoke-ReccmpTool -Name 'reccmp-aggregate.exe' -Arguments @(
            '--diff', $prev, $json, '--no-color'
        ) -LogFile (Join-Path $reportDir 'VC1.diff.txt') -AllowFailure -ShowAll
    }
    Copy-Item -LiteralPath $json -Destination $prev -Force
    if ($UpdateStatus) {
        # reccmp's template sizes the title for short names like LEGO1.DLL;
        # shrink it so "VirtualCity.Unwrapped.exe" fits the badge.
        $svg = [IO.File]::ReadAllText((Join-Path $reportDir 'VC1.svg'))
        $svg = $svg.Replace('font-size:12.7px', 'font-size:8.4px')
        [IO.File]::WriteAllText((Join-Path $root 'docs/status.svg'), $svg, (New-Object Text.UTF8Encoding $false))
        Write-Host '--- docs/status.svg updated'
    }

    if ($ResetBaseline -or !(Test-Path -LiteralPath $base)) {
        Copy-Item -LiteralPath $json -Destination $base -Force
        Write-Host '--- baseline reset to this run'
    }
    else {
        Write-Host '--- since round baseline'
        Invoke-ReccmpTool -Name 'reccmp-aggregate.exe' -Arguments @(
            '--diff', $base, $json, '--no-color'
        ) -LogFile (Join-Path $reportDir 'VC1.base-diff.txt') -AllowFailure -ShowAll
    }

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

    Write-Host '--- progress'
    & (Join-Path $bin 'python.exe') (Join-Path $PSScriptRoot 'report-progress.py')
}
finally {
    Pop-Location
    $env:PYTHONUTF8 = $oldUtf8
}
exit 0
