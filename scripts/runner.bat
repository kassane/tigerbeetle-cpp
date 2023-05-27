@echo off
setlocal enabledelayedexpansion

if "%~1" equ ":main" (
    shift /1
    goto main
)

cmd /d /c "%~f0" :main %*
set ZIG_RESULT=%ERRORLEVEL%
taskkill /F /IM zig-out\bin\tigerbeetle.exe >nul

if !ZIG_RESULT! equ 0 (
    del /f client.log
) else (
    echo.
    echo Error running client
    type client.log
)

echo.
exit /b

:main

echo Initializing replica 0
set ZIG_FILE=0_0.tigerbeetle
if exist "!ZIG_FILE!" DEL /F "!ZIG_FILE!"
    zig-out\bin\tigerbeetle.exe format --cluster=0 --replica=0 --replica-count=1 !ZIG_FILE! > client.log 2>&1 || exit /b


echo Starting replica 0
start /B "tigerbeetle_0" zig-out\bin\tigerbeetle.exe start --addresses=3001 !ZIG_FILE! > client.log 2>&1

echo.
echo client running...
..\..\tb_cpp.exe
exit /b %errorlevel%