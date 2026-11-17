@echo off
rem CloudVeil Messenger build environment: Python 3 first on PATH and
rem VS 2022 x64 with the MSVC 14.44 toolset (required by tdesktop 7.4.x).
rem Override CVM_PYTHON / CVM_VCVARS if your install paths differ.
if not defined CVM_PYTHON set "CVM_PYTHON=%LOCALAPPDATA%\Programs\Python\Python313"
if not defined CVM_VCVARS set "CVM_VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "PATH=%CVM_PYTHON%;%CVM_PYTHON%\Scripts;%PATH%"
call "%CVM_VCVARS%" -vcvars_ver=14.44 >nul
if errorlevel 1 (echo vcvars64 failed & exit /b 1)
