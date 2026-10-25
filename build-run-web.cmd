@echo off
rem Builds the web build (tools\build-web.cmd; it packages the local game copy
rem when there is one) and serves it at http://localhost:8000/ until this
rem window is closed. Set VC1_WEB_PORT for another port.
setlocal
set "ROOT=%~dp0"
if not defined VC1_WEB_PORT set VC1_WEB_PORT=8000
call "%ROOT%tools\build-web.cmd"
if errorlevel 1 exit /b 1
set "PYTHON=python"
if exist "%ROOT%.venv\Scripts\python.exe" set "PYTHON=%ROOT%.venv\Scripts\python.exe"
rem Open the page once the server below is listening.
start "" /B cmd /c "ping -n 3 127.0.0.1 >nul & explorer http://localhost:%VC1_WEB_PORT%/"
"%PYTHON%" -m http.server %VC1_WEB_PORT% --bind 127.0.0.1 --directory "%ROOT%build\web\bin"
