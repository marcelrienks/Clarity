@echo off
echo Running quick tests...
REM Create build directory if it doesn't exist
if not exist "..\build" mkdir "..\build"
REM Set compiler flags
set CXX=C:\msys64\ucrt64\bin\g++.exe
set UNITY_PATH=.pio\libdeps\test\Unity\src
set INCLUDE_FLAGS=-I. -I..\include -I..\test -I..\test\unit\utilities -I..\test\mocks -I..\.pio\libdeps\test\Unity\src -DUNIT_TESTING
set CPP_FLAGS=-std=c++17 %INCLUDE_FLAGS%
echo Compiling test files...
REM Compile test files
%CXX% %CPP_FLAGS% ^
    ..\.pio\libdeps\test\Unity\src\unity.c ^
    mocks\mock_hardware.cpp ^
    test_main.cpp ^
    -o ..\build\quick_tests.exe
REM Run the tests if compilation succeeded
if %ERRORLEVEL% EQU 0 (
    echo Running tests...
    ..\build\quick_tests.exe
) else (
    echo Compilation failed
    exit /b 1
)
