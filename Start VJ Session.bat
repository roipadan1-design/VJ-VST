@echo off
REM One-shot launcher: starts the VJ Engine render process and Ableton Live
REM together, so you don't have to run the engine .exe by hand every time.
setlocal

set "ENGINE_ROOT=%~dp0engine\build\VJEngine_artefacts"
set "ENGINE_EXE="

if exist "%ENGINE_ROOT%\Release\VJ Engine.exe" set "ENGINE_EXE=%ENGINE_ROOT%\Release\VJ Engine.exe"
if not defined ENGINE_EXE if exist "%ENGINE_ROOT%\Debug\VJ Engine.exe" set "ENGINE_EXE=%ENGINE_ROOT%\Debug\VJ Engine.exe"

if not defined ENGINE_EXE (
    echo Could not find "VJ Engine.exe" under %ENGINE_ROOT%.
    echo Build the engine project first ^(engine\build\VJEngine.sln^), then re-run this script.
    pause
    exit /b 1
)

echo Starting VJ Engine: %ENGINE_EXE%
start "" "%ENGINE_EXE%"

set "LIVE_EXE=C:\ProgramData\Ableton\Live 10 Suite\Program\Ableton Live 10 Suite.exe"

if exist "%LIVE_EXE%" (
    echo Starting Ableton Live...
    start "" "%LIVE_EXE%"
) else (
    echo Ableton Live not found at expected path - start it manually:
    echo   %LIVE_EXE%
)

endlocal
