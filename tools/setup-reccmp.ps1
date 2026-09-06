param([string]$Python = 'python', [string]$Original)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
if (!$Original) { $Original = Join-Path $root 'Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe' }
$Original = (Resolve-Path -LiteralPath $Original).Path
$expected = 'bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695'
if ((Get-FileHash -LiteralPath $Original -Algorithm SHA256).Hash.ToLower() -ne $expected) { throw 'Original SHA256 mismatch' }
$revision = (Get-Content -LiteralPath "$PSScriptRoot/reccmp-revision.txt" -Raw).Trim()
$checkout = Join-Path $root 'vendor/reccmp'
if (!(Test-Path -LiteralPath $checkout)) {
    & git clone https://github.com/isledecomp/reccmp.git $checkout
    if ($LASTEXITCODE) { throw 'git clone failed' }
    & git -C $checkout checkout --detach $revision
    if ($LASTEXITCODE) { throw 'git checkout failed' }
}
$head = & git -C $checkout rev-parse HEAD
if ($LASTEXITCODE -or $head -ne $revision) { throw 'Unexpected reccmp revision; preserve local work and review it first' }
$venvPython = Join-Path $root '.venv/Scripts/python.exe'
if (!(Test-Path -LiteralPath $venvPython)) {
    & $Python -m venv (Join-Path $root '.venv')
    if ($LASTEXITCODE) { throw 'venv creation failed (use Python 3.10+)' }
}
& $venvPython -m pip install -r "$PSScriptRoot/requirements-reccmp.txt"
if ($LASTEXITCODE) { throw 'Dependency installation failed' }
& $venvPython -m pip install --no-deps -e $checkout
if ($LASTEXITCODE) { throw 'reccmp installation failed' }
$userConfig = Join-Path $root 'reccmp-user.yml'
if (!(Test-Path -LiteralPath $userConfig)) {
    $quoted = $Original.Replace('\','/').Replace("'", "''")
    $yaml = "targets:`n  VC1:`n    path: '$quoted'`n"
    [IO.File]::WriteAllText($userConfig, $yaml, (New-Object Text.UTF8Encoding $false))
} else { Write-Host 'Existing reccmp-user.yml preserved; check its paths if the checkout moved.' }
Write-Host "reccmp ready at $revision (local modifications, if any, are preserved)."
