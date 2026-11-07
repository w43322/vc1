@echo off
rem Publish the web build (build\web\bin, tools\build-web.cmd) to Cloudflare
rem Pages with wrangler, run through the emsdk's Node. The project is
rem VC1_PAGES_PROJECT (default virtual-city). wrangler signs in with
rem CLOUDFLARE_API_TOKEN when set, else "wrangler login" in the browser.
rem Extra arguments go to "wrangler pages deploy".
setlocal
set "REPO_ROOT=%~dp0.."
if not defined VC1_EMSDK set "VC1_EMSDK=%REPO_ROOT%\build\emsdk"
if not defined VC1_PAGES_PROJECT set "VC1_PAGES_PROJECT=virtual-city"
if not exist "%REPO_ROOT%\build\web\bin\game.data.json" (
  echo No web build in build\web\bin; run tools\build-web.cmd first.
  exit /b 1
)
if exist "%VC1_EMSDK%\emsdk_env.bat" call "%VC1_EMSDK%\emsdk_env.bat" >nul
call npx --yes wrangler@4 pages deploy "%REPO_ROOT%\build\web\bin" --project-name "%VC1_PAGES_PROJECT%" --branch main %*
exit /b %ERRORLEVEL%
