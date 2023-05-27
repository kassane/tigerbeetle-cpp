@echo off
setlocal enabledelayedexpansion

REM Define colors for console output
set COLOR_RED=^[[1;31m
set COLOR_END=^[[0m

REM Function to handle errors and cleanup
:onerror
if "!ERRORLEVEL!"=="0" (
    del running.log
    echo Done!!
) else (
    echo %COLOR_RED%
    echo Error running with tigerbeetle
    echo %COLOR_END%
    type running.log
)

taskkill /f /im tigerbeetle.exe > nul 2>&1

REM Be careful to use a running-specific filename so that we don't erase a real data file
set "FILE=%CD%\0_0.tigerbeetle"
if exist "%FILE%" (
    del "%FILE%"
)

zig-out\bin\tigerbeetle.exe format --cluster=0 --replica=0 --replica-count=1 "%FILE%" > running.log 2>&1
echo Starting replica 0
start /B zig-out\bin\tigerbeetle.exe start --addresses=3001 "%FILE%" > running.log 2>&1

echo.
echo running client...
..\..\tb_cpp.exe

echo.

if exist "%FILE%" (
    del "%FILE%"
)
