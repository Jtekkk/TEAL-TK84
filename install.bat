@echo off
rem ---------------------------------------------------------------------------
rem  TEAL TK84 - build ^& install script (Windows)
rem
rem  Configures the project, builds a Release, and copies the plugin into the
rem  standard user plugin folders (handled by COPY_PLUGIN_AFTER_BUILD in CMake).
rem
rem  Usage:
rem    install.bat                       fetch JUCE automatically, build ^& install
rem    set JUCE_DIR=C:\JUCE ^& install.bat  use a local JUCE checkout
rem ---------------------------------------------------------------------------
setlocal

cd /d "%~dp0"

if "%BUILD_DIR%"==""  set "BUILD_DIR=build"
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Release"

echo ==^> TEAL TK84 install
echo     build dir:  %BUILD_DIR%
echo     build type: %BUILD_TYPE%

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: cmake is required but was not found on PATH.
    exit /b 1
)

set "CMAKE_ARGS=-B "%BUILD_DIR%""
if not "%JUCE_DIR%"=="" (
    echo     JUCE_DIR:   %JUCE_DIR%
    set "CMAKE_ARGS=%CMAKE_ARGS% -DJUCE_DIR="%JUCE_DIR%""
)

echo ==^> Configuring...
cmake %CMAKE_ARGS%
if errorlevel 1 exit /b 1

echo ==^> Building (this fetches JUCE on first run and may take a while)...
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% --parallel
if errorlevel 1 exit /b 1

rem Explicitly install the built VST3 to the per-user VST3 folder (idempotent,
rem works even when the build was already up to date).
set "ART=%BUILD_DIR%\TEAL_TK84_artefacts\%BUILD_TYPE%"
set "VST3_DEST=%LOCALAPPDATA%\Programs\Common\VST3"

echo.
echo ==^> Installing...
if exist "%ART%\VST3\TEAL TK84.vst3" (
    if not exist "%VST3_DEST%" mkdir "%VST3_DEST%"
    robocopy "%ART%\VST3\TEAL TK84.vst3" "%VST3_DEST%\TEAL TK84.vst3" /E /NFL /NDL /NJH /NJS /NC /NS >nul
    echo     VST3 -^> %VST3_DEST%
)

echo.
echo ==^> Done. Built artefacts are in: %ART%\
echo     Rescan plugins in your DAW to pick up TEAL TK84.

endlocal
