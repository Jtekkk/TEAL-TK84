@echo off
rem ---------------------------------------------------------------------------
rem  TEAL TK84 - build the plugin and compile the Windows installer .exe
rem
rem  Requirements:
rem    - CMake + a C++ toolchain (Build Tools for Visual Studio)
rem    - Inno Setup 6      https://jrsoftware.org/isdl.php
rem
rem  Usage (from anywhere):
rem    installer\build-installer.bat
rem
rem  Output:
rem    installer\Output\TEAL_TK84-1.0.0-Windows.exe
rem ---------------------------------------------------------------------------
setlocal

rem Repo root is the parent of this script's folder.
set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%.."
set "ROOT=%CD%"

if "%BUILD_DIR%"==""  set "BUILD_DIR=build"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Release"

echo ==^> Building TEAL TK84 (%BUILD_TYPE%)...
cmake -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 goto :fail
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% --parallel
if errorlevel 1 goto :fail

rem Locate the Inno Setup compiler (ISCC).
set "ISCC="
where iscc >nul 2>nul && set "ISCC=iscc"
if "%ISCC%"=="" if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" set "ISCC=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
if "%ISCC%"=="" if exist "%ProgramFiles%\Inno Setup 6\ISCC.exe"      set "ISCC=%ProgramFiles%\Inno Setup 6\ISCC.exe"

if "%ISCC%"=="" (
    echo.
    echo ERROR: Inno Setup 6 (ISCC.exe) was not found.
    echo        Install it from https://jrsoftware.org/isdl.php and re-run,
    echo        or compile installer\TEAL_TK84.iss manually.
    goto :fail
)

echo ==^> Compiling installer with Inno Setup...
"%ISCC%" "%SCRIPT_DIR%TEAL_TK84.iss"
if errorlevel 1 goto :fail

echo.
echo ==^> Done. Installer is in: %SCRIPT_DIR%Output\
popd
endlocal
exit /b 0

:fail
echo.
echo ==^> Build failed.
popd
endlocal
exit /b 1
