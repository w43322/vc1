@echo off
rem Publish the web build (build\web\bin, tools\build-web.cmd) to Cloudflare
rem Pages with wrangler, run through the emsdk's Node. The project is
rem VC1_PAGES_PROJECT (default virtual-city), created on first use as a Pages
rem project (--force: wrangler would otherwise make it a Worker). wrangler
rem signs in with CLOUDFLARE_API_TOKEN, else the first line of
rem cloudflare-token.txt at the repository root (git-ignored), else
rem "wrangler login" in the browser. Extra arguments go to "wrangler pages
rem deploy".
setlocal
set "REPO_ROOT=%~dp0.."
if not defined VC1_EMSDK set "VC1_EMSDK=%REPO_ROOT%\build\emsdk"
if not defined VC1_PAGES_PROJECT set "VC1_PAGES_PROJECT=virtual-city"
if not exist "%REPO_ROOT%\build\web\bin\game.data.json" (
  echo No web build in build\web\bin; run tools\build-web.cmd first.
  exit /b 1
)
if not defined CLOUDFLARE_API_TOKEN if exist "%REPO_ROOT%\cloudflare-token.txt" (
  set /p CLOUDFLARE_API_TOKEN=<"%REPO_ROOT%\cloudflare-token.txt"
)
if exist "%VC1_EMSDK%\emsdk_env.bat" call "%VC1_EMSDK%\emsdk_env.bat" >nul
call npx --yes wrangler@4 pages project list | findstr /r /c:"[ ]%VC1_PAGES_PROJECT%[ ]" >nul
if errorlevel 1 (
  call npx --yes wrangler@4 pages project create "%VC1_PAGES_PROJECT%" --production-branch main --force
  if errorlevel 1 exit /b 1
)
call npx --yes wrangler@4 pages deploy "%REPO_ROOT%\build\web\bin" --project-name "%VC1_PAGES_PROJECT%" --branch main --commit-dirty=true %*
exit /b %ERRORLEVEL%
