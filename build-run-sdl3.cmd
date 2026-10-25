@echo off
rem Builds the SDL3 build (tools\build-sdl3.cmd) and runs it from the local
rem game copy as shipped: Virtual-City_Win_EN_Full, which holds
rem VirtualCity.bar (the game mounts *.bar from its working directory).
setlocal
set "ROOT=%~dp0"
call "%ROOT%tools\build-sdl3.cmd"
if errorlevel 1 exit /b 1
start "Virtual City (SDL3)" /D "%ROOT%Virtual-City_Win_EN_Full" "%ROOT%build\sdl3\bin\VirtualCity.SDL3.exe"
