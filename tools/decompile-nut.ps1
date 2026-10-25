# Decompile every compiled .nut script of the game into readable Squirrel.
# Output: Virtual-City_Win_EN_Full/nut-src/ (same tree as VirtualCity/).
# The game folder is not git-ignored so Cursor Grep/Read can see the output;
# never stage anything under it.
# Decompiler: the vendor/NutCracker submodule (https://github.com/w43322/NutCracker,
# branch sq2-vc1, pinned commit), built into build/NutCracker when missing.
param([switch]$Rebuild)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$src = Join-Path $repo 'vendor\NutCracker'
$project = Join-Path $src 'nutcracker\nutcracker.vcxproj'
$outDir = Join-Path $repo 'build\NutCracker'
$nc = Join-Path $outDir 'nutcracker.exe'

if (-not (Test-Path $project)) {
    & git -C $repo submodule update --init -- vendor/NutCracker
    if ($LASTEXITCODE -ne 0) { throw 'git submodule update failed' }
}
if ($Rebuild -or -not (Test-Path $nc)) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $msbuild = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    if (-not $msbuild) { throw 'MSBuild with the x86/x64 C++ tools not found (Visual Studio 2022)' }
    $sdk = (Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\Include" -Directory |
        Sort-Object Name | Select-Object -Last 1).Name
    # The project targets SDK 8.1 / v140_xp; override with whatever is installed.
    # Output and intermediates go to build/, keeping the submodule clean.
    & $msbuild $project /p:Configuration=Release /p:Platform=Win32 `
        /p:WindowsTargetPlatformVersion=$sdk /p:PlatformToolset=v143 `
        "/p:OutDir=$outDir/" "/p:IntDir=$outDir/obj/" /m /v:minimal
    if ($LASTEXITCODE -ne 0) { throw "NutCracker build failed" }
}

$root = Join-Path $repo 'Virtual-City_Win_EN_Full\VirtualCity'
$out = Join-Path $repo 'Virtual-City_Win_EN_Full\nut-src'
$ok = 0
$copied = 0
Get-ChildItem $root -Recurse -Filter *.nut | ForEach-Object {
    $rel = $_.FullName.Substring($root.Length + 1)
    $dst = Join-Path $out $rel
    New-Item -ItemType Directory -Force (Split-Path $dst) | Out-Null
    $text = & $nc $_.FullName 2>&1 | Out-String
    if ($LASTEXITCODE -eq 0) {
        [IO.File]::WriteAllText($dst, $text)
        $ok++
    }
    else {
        # A few .nut files are plain-text source already (e.g. res/RetailerCode.nut).
        Copy-Item $_.FullName $dst -Force
        $copied++
    }
}
"decompiled $ok, copied as text $copied -> $out"
