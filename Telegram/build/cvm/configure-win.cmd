@echo off
rem Configure CloudVeil Messenger (x64 Release).
rem Reads "API_ID API_HASH" as ONE line from stdin so the keys never appear on
rem a command line (cmd's "set /p" reads every piped line into one variable).
rem   echo ID HASH| Telegram\build\cvm\configure-win.cmd
setlocal
set /p CVM_KEYS=
for /f "tokens=1,2" %%a in ("%CVM_KEYS%") do (
    set "API_ID=%%a"
    set "API_HASH=%%b"
)
set CVM_KEYS=
if "%API_ID%"=="" (echo missing API id & exit /b 1)
if "%API_HASH%"=="" (echo missing API hash & exit /b 1)
call "%~dp0env-win.cmd" || exit /b 1
cd /d "%~dp0..\.."
call configure.bat x64 -D TDESKTOP_API_ID=%API_ID% -D TDESKTOP_API_HASH=%API_HASH% -D DESKTOP_APP_DISABLE_AUTOUPDATE=ON -D DESKTOP_APP_DISABLE_CRASH_REPORTS=ON
echo configure exit %ERRORLEVEL%
