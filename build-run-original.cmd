@echo off
rem Builds the reference build (VC9, tools\build-vc9.cmd) and runs it from the
rem local game copy as shipped: Virtual-City_Win_EN_Full, which holds
rem VirtualCity.bar (the game mounts *.bar from its working directory).
setlocal
set "ROOT=%~dp0"
call "%ROOT%tools\build-vc9.cmd"
if errorlevel 1 exit /b 1
start "Virtual City (VC9)" /D "%ROOT%Virtual-City_Win_EN_Full" "%ROOT%build\vc9\bin\VirtualCity.Unwrapped.exe"
