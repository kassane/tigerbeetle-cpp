@echo off
setlocal enabledelayedexpansion

set ZIG=%~1
set ZIG_BUILD_TYPE=%~2
set CLIENT=%~3

echo.
echo running client...
%ZIG% build run %ZIG_BUILD_TYPE% -- %CLIENT%
echo.
%ZIG% uninstall

endlocal