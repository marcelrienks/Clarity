@echo off
echo Setting up test environment...

REM Create necessary directories
if not exist "build" mkdir build
if not exist "build\test" mkdir build\test

REM Clean previous builds
if exist ".pio\build\test" rmdir /s /q .pio\build\test

REM Build and run tests
pio test -e test -v
