# Write the local .mcp.json (git-ignored) from config/mcp.json.template.
# The ghidra-mcp checkout is -GhidraMcpDir, else VC1_GHIDRA_MCP_DIR, else the
# default in tools/workspace.ps1. An existing different .mcp.json is kept
# unless -Force is given.
param([string]$GhidraMcpDir, [switch]$Force)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'workspace.ps1')
$root = Split-Path $PSScriptRoot -Parent
$GhidraMcpDir = Get-Vc1GhidraMcpDir $GhidraMcpDir
if (!(Test-Path -LiteralPath (Join-Path $GhidraMcpDir 'pyproject.toml'))) {
    Write-Warning "No ghidra-mcp checkout at $GhidraMcpDir (pyproject.toml missing); writing the path anyway."
}
$template = [IO.File]::ReadAllText((Join-Path $root 'config/mcp.json.template'))
# ConvertTo-Json yields the path as an escaped JSON string, quotes included.
$text = $template.Replace('"@GHIDRA_MCP_DIR@"', (ConvertTo-Json $GhidraMcpDir))
$target = Join-Path $root '.mcp.json'
if ((Test-Path -LiteralPath $target) -and !$Force) {
    if ([IO.File]::ReadAllText($target) -ne $text) {
        Write-Host "Existing .mcp.json differs and was kept; rerun with -Force to replace it."
        exit 0
    }
}
[IO.File]::WriteAllText($target, $text, (New-Object Text.UTF8Encoding $false))
Write-Host "Wrote .mcp.json (ghidra-mcp at $GhidraMcpDir)."
