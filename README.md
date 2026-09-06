# Virtual City 1

目标：`Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe`  
SHA-256：`bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695`

```powershell
.\tools\setup-reccmp.ps1
.\tools\open-ghidra.ps1
.\tools\build-vc9.cmd
.\tools\compare-reccmp.ps1
.\tools\close-ghidra.ps1
.\tools\import-ghidra.ps1
```

`import-ghidra.ps1` 把已匹配的 PDB 名字写回 Ghidra。headless 导入前先关掉 Ghidra GUI。
