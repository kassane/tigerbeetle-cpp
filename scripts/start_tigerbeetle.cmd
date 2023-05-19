@echo off

REM Run tigerbeetle start in the background
start /B "" "./zig-out/bin/tigerbeetle" start %*

REM Save the process ID to a file for later use
echo %errorlevel% > tigerbeetle.pid

REM Read the process ID from the saved file
set /p pid=<tigerbeetle.pid

timeout 5

REM Kill the tigerbeetle start process
taskkill /PID %pid%