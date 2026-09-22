@echo off
REM Builds everything (Release), runs the analysis tests, and installs the
REM VJ Analyzer VST3 where Ableton Live 10.1+ looks for VST3 plug-ins.
REM Needs CMake + Visual Studio 2022 (already used for this project).
setlocal
set "ROOT=%~dp0"

echo === AnalysisCore + tests ===
cmake -S "%ROOT%analysis" -B "%ROOT%analysis\build" || goto :fail
cmake --build "%ROOT%analysis\build" --config Release || goto :fail
"%ROOT%analysis\build\Release\analysis_tests.exe" || goto :fail

echo === VJ Engine ===
cmake -S "%ROOT%engine" -B "%ROOT%engine\build" || goto :fail
cmake --build "%ROOT%engine\build" --config Release || goto :fail

echo === VJ Analyzer (VST3) ===
cmake -S "%ROOT%plugin" -B "%ROOT%plugin\build" || goto :fail
cmake --build "%ROOT%plugin\build" --config Release || goto :fail

echo === Installing VST3 ===
set "VST3_SRC=%ROOT%plugin\build\VJAnalyzer_artefacts\Release\VST3\VJ Analyzer.vst3"
set "VST3_DST=%CommonProgramFiles%\VST3\VJ Analyzer.vst3"
tasklist | find /i "Ableton Live" >nul && echo NOTE: close Ableton Live first if the copy fails (the plug-in file is in use).
xcopy "%VST3_SRC%" "%VST3_DST%" /E /I /Y >nul || goto :fail
echo Installed to %VST3_DST%
echo In Live: Preferences ^> Plug-ins ^> enable "Use VST3 Plug-in System Folders", then Rescan.
pause
exit /b 0

:fail
echo.
echo BUILD FAILED - see the messages above.
pause
exit /b 1
