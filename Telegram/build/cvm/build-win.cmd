@echo off
rem Build the CloudVeil Messenger Release app (x64) -> out\Release\CloudVeilMessenger.exe
rem /m:1 /p:CL_MPCount=4 limits parallelism: a 16 GB machine runs out of
rem compiler heap with the defaults. Raise them on machines with more memory.
setlocal
call "%~dp0env-win.cmd" || exit /b 1
cd /d "%~dp0..\..\..\out"
cmake --build . --config Release --target Telegram -- /m:1 /p:CL_MPCount=4 /v:minimal
echo build exit %ERRORLEVEL%
