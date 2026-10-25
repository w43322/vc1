@echo off
rem Web build: the SDL3 target compiled to WebAssembly with Emscripten.
rem Output: build\web\bin\index.html, VirtualCity.js, VirtualCity.wasm.
rem Emscripten is the emsdk at VC1_EMSDK (default build\emsdk, "emsdk install
rem latest"); SDL3 / SDL3_mixer come from VC1_SDL3_SOURCE_DIR and
rem VC1_SDL3_MIXER_SOURCE_DIR when set, else they are downloaded like the
rem SDL3 build's. Extra arguments go to "cmake --build".
setlocal
chcp 65001 >nul
set "REPO_ROOT=%~dp0.."
if not defined VC1_EMSDK set "VC1_EMSDK=%REPO_ROOT%\build\emsdk"
if not exist "%VC1_EMSDK%\emsdk_env.bat" (
  echo No emsdk at %VC1_EMSDK%. Install one there or set VC1_EMSDK:
  echo   git clone https://github.com/emscripten-core/emsdk.git build\emsdk
  echo   build\emsdk\emsdk install latest ^&^& build\emsdk\emsdk activate latest
  exit /b 1
)
call "%VC1_EMSDK%\emsdk_env.bat" >nul
if errorlevel 1 exit /b 1

rem CMake 3.16+ and Ninja: the ones on PATH, else Visual Studio's.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
  for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -all -products * -property installationPath`) do (
    if exist "%%i\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "VS_CMAKE_DIR=%%i\Common7\IDE\CommonExtensions\Microsoft\CMake"
  )
)
if not defined VC1_CMAKE if exist "%VS_CMAKE_DIR%\CMake\bin\cmake.exe" set "VC1_CMAKE=%VS_CMAKE_DIR%\CMake\bin\cmake.exe"
if not defined VC1_CMAKE set "VC1_CMAKE=cmake"
where ninja >nul 2>nul
if errorlevel 1 if exist "%VS_CMAKE_DIR%\Ninja\ninja.exe" set "PATH=%VS_CMAKE_DIR%\Ninja;%PATH%"

set VC1_SDL_ARGS=
if defined VC1_SDL3_SOURCE_DIR set VC1_SDL_ARGS="-DFETCHCONTENT_SOURCE_DIR_SDL3=%VC1_SDL3_SOURCE_DIR%"
if defined VC1_SDL3_MIXER_SOURCE_DIR set VC1_SDL_ARGS=%VC1_SDL_ARGS% "-DFETCHCONTENT_SOURCE_DIR_SDL3_MIXER=%VC1_SDL3_MIXER_SOURCE_DIR%"
"%VC1_CMAKE%" -S "%REPO_ROOT%" -B "%REPO_ROOT%\build\web" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
  "-DCMAKE_TOOLCHAIN_FILE=%EMSDK%\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake" %VC1_SDL_ARGS%
if errorlevel 1 exit /b 1
"%VC1_CMAKE%" --build "%REPO_ROOT%\build\web" %*
exit /b %ERRORLEVEL%
