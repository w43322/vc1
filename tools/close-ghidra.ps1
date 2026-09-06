$ErrorActionPreference = 'Stop'
$sockDir = Join-Path $env:LOCALAPPDATA 'Temp'
$pids = @()
Get-ChildItem -Path $sockDir -Directory -Filter 'ghidra-mcp-*' -ErrorAction SilentlyContinue |
    ForEach-Object {
        Get-ChildItem $_.FullName -Filter 'ghidra-*.sock' -ErrorAction SilentlyContinue |
            ForEach-Object {
                if ($_.BaseName -match '^ghidra-(\d+)$') { $pids += [int]$Matches[1] }
            }
    }
$pids += @(Get-CimInstance Win32_Process -Filter "Name = 'javaw.exe' OR Name = 'java.exe'" -ErrorAction SilentlyContinue |
    Where-Object { $_.CommandLine -match 'ghidra' } |
    Select-Object -ExpandProperty ProcessId)
$pids = $pids | Sort-Object -Unique
if (!$pids) {
    Write-Output 'ghidra-not-running'
    exit 0
}
foreach ($procId in $pids) {
    Stop-Process -Id $procId -Force -ErrorAction SilentlyContinue
}
$deadline = (Get-Date).AddSeconds(60)
while ((Get-Date) -lt $deadline) {
    $alive = $false
    foreach ($procId in $pids) {
        if (Get-Process -Id $procId -ErrorAction SilentlyContinue) { $alive = $true }
    }
    if (!$alive) {
        Write-Output 'ghidra-exited'
        exit 0
    }
    Start-Sleep -Seconds 1
}
throw 'Ghidra did not exit within 60s'
