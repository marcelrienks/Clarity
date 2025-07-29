@echo off
echo Running all unit tests (without Wokwi)...

REM Create build directory if it doesn't exist
if not exist "..\build" mkdir "..\build"
if not exist "..\build\test" mkdir "..\build\test"

REM Set compiler paths
set CXX=C:\msys64\ucrt64\bin\g++.exe

REM Set include paths
set INCLUDES=-I. -I..\include -I..\test -I..\test\mocks -I..\test\unit -I..\test\integration -I..\test\unit\utilities -I..\test\unit\scenarios -I..\test\unit\sensors -I..\test\unit\managers -I..\test\unit\device -I..\test\unit\components -I.pio\libdeps\test\Unity\src

REM Set compiler flags
set CXXFLAGS=-std=c++17 -DUNIT_TESTING

REM List all source files
echo Compiling test files...

%CXX% %CXXFLAGS% %INCLUDES% ^
    .pio\libdeps\test\Unity\src\unity.c ^
    test\mocks\mock_managers.cpp ^
    test\mocks\mock_system.cpp ^
    test\mocks\mock_utilities.cpp ^
    test\unit\utilities\test_utilities.cpp ^
    test\unit\sensors\test_sensors.cpp ^
    test\unit\sensors\test_oil_sensors.cpp ^
    test\unit\scenarios\test_startup_scenarios.cpp ^
    test\unit\scenarios\test_single_trigger_scenarios.cpp ^
    test\unit\scenarios\test_performance_scenarios.cpp ^
    test\unit\scenarios\test_multiple_trigger_scenarios.cpp ^
    test\unit\scenarios\test_edge_case_scenarios.cpp ^
    test\unit\managers\test_panel_manager.cpp ^
    test\unit\managers\test_style_manager.cpp ^
    test\unit\managers\test_trigger_system.cpp ^
    test\test_main.cpp ^
    -o build\test\test_runner.exe

REM Run the tests if compilation succeeded
if %ERRORLEVEL% EQU 0 (
    echo Running tests...
    build\test\test_runner.exe
) else (
    echo Compilation failed
    exit /b 1
)
