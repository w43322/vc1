# Machine-specific paths for the tools; dot-source this file.
# Each path is the script's parameter when given, else an environment
# variable, else the default below. Set the variables once per user, e.g.
#   setx VC1_GHIDRA_HOME "C:\tools\ghidra_12.1.2_PUBLIC"
#   setx VC1_GHIDRA_MCP_DIR "C:\tools\ghidra-mcp"

function Resolve-Vc1Path([string]$Value, [string[]]$EnvNames, [string]$Default) {
    if ($Value) { return $Value }
    foreach ($name in $EnvNames) {
        $fromEnv = [Environment]::GetEnvironmentVariable($name)
        if ($fromEnv) { return $fromEnv }
    }
    return $Default
}

# Ghidra 12.x install (contains ghidraRun.bat and support\analyzeHeadless.bat).
function Get-Vc1GhidraHome([string]$Value) {
    $path = Resolve-Vc1Path $Value @('VC1_GHIDRA_HOME', 'GHIDRA_INSTALL_DIR') 'D:\ghidra_12.1.2_PUBLIC'
    if (!(Test-Path -LiteralPath (Join-Path $path 'ghidraRun.bat'))) {
        throw "No Ghidra install at $path; pass -GhidraHome or set VC1_GHIDRA_HOME."
    }
    return $path
}

# ghidra-mcp checkout (the bridge-mcp-ghidra uv project used by .mcp.json).
function Get-Vc1GhidraMcpDir([string]$Value) {
    return Resolve-Vc1Path $Value @('VC1_GHIDRA_MCP_DIR') 'D:\ghidra-mcp'
}
