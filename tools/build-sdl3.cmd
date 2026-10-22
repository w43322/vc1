@echo off
setlocal
chcp 65001 >nul
set VSLANG=1033
set "REPO_ROOT=%~dp0.."
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo Install modern Visual Studio C++ build tools first.
  exit /b 1
)
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VC1_PORT_VS=%%i"
if not defined VC1_PORT_VS exit /b 1
set INCLUDE=
set LIB=
set LIBPATH=
call "%VC1_PORT_VS%\VC\Auxiliary\Build\vcvars32.bat"
if errorlevel 1 exit /b 1
set CL=
set _CL_=
set LINK=
if not defined VC1_CMAKE if exist "%VC1_PORT_VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "VC1_CMAKE=%VC1_PORT_VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if not defined VC1_CMAKE if exist "%REPO_ROOT%\build\cmake-runtime\cmake\data\bin\cmake.exe" set "VC1_CMAKE=%REPO_ROOT%\build\cmake-runtime\cmake\data\bin\cmake.exe"
if not defined VC1_CMAKE set "VC1_CMAKE=cmake"
set VC1_MIXER_ARG=
if defined VC1_SDL3_MIXER_SOURCE_DIR set VC1_MIXER_ARG="-DFETCHCONTENT_SOURCE_DIR_SDL3_MIXER=%VC1_SDL3_MIXER_SOURCE_DIR%"
if defined VC1_SDL3_SOURCE_DIR (
  "%VC1_CMAKE%" -S "%REPO_ROOT%" -B "%REPO_ROOT%\build\sdl3" -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DVC1_USE_SDL3=ON "-DFETCHCONTENT_SOURCE_DIR_SDL3=%VC1_SDL3_SOURCE_DIR%" %VC1_MIXER_ARG%
) else (
  "%VC1_CMAKE%" -S "%REPO_ROOT%" -B "%REPO_ROOT%\build\sdl3" -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DVC1_USE_SDL3=ON %VC1_MIXER_ARG%
)
if errorlevel 1 exit /b 1
"%VC1_CMAKE%" --build "%REPO_ROOT%\build\sdl3" %*
exit /b %ERRORLEVEL%
