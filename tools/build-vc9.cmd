@echo off
setlocal
set "REPO_ROOT=%~dp0.."
if not defined VC9_ROOT set "VC9_ROOT=%ProgramFiles(x86)%\Microsoft Visual Studio 9.0"
if not exist "%VC9_ROOT%\Common7\Tools\vsvars32.bat" (
  echo VC9 not found. Set VC9_ROOT to the Visual Studio 9.0 installation directory.
  exit /b 1
)
set INCLUDE=
set LIB=
set LIBPATH=
call "%VC9_ROOT%\Common7\Tools\vsvars32.bat"
if errorlevel 1 exit /b 1
rem Prevent ambient compiler/linker options leaking into reference builds.
set CL=
set _CL_=
set LINK=
if not "%~1"=="" (
  echo Usage: tools\build-vc9.cmd
  exit /b 1
)
where cl
where link
cmake -S "%REPO_ROOT%" -B "%REPO_ROOT%\build\vc9" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DVC1_USE_SDL3=OFF
if errorlevel 1 exit /b 1
cmake --build "%REPO_ROOT%\build\vc9"
exit /b %ERRORLEVEL%
