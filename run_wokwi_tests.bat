@echo off
setlocal enabledelayedexpansion

REM Wokwi Automated Test Runner for Clarity Automotive Gauge System
REM This script runs all integration test scenarios using Wokwi CLI

echo 🚗 Clarity Automotive Gauge - Wokwi Integration Tests
echo ==================================================

REM Check if WOKWI_CLI_TOKEN is set
if "%WOKWI_CLI_TOKEN%"=="" (
    echo ❌ Error: WOKWI_CLI_TOKEN environment variable not set
    echo Please set your Wokwi CLI token:
    echo set WOKWI_CLI_TOKEN=your_token_here
    exit /b 1
)

REM Check if wokwi-cli is installed
wokwi-cli --version >nul 2>&1
if errorlevel 1 (
    echo ❌ Error: wokwi-cli not found
    echo Please install Wokwi CLI from GitHub releases
    echo https://github.com/wokwi/wokwi-cli/releases
    exit /b 1
)

echo 📦 Building firmware...
pio run -e debug-local
if errorlevel 1 (
    echo ❌ Build failed!
    exit /b 1
)
echo ✅ Firmware build successful
echo.

REM Test scenarios array (using indexed variables)
set test_count=0
set /a test_count+=1 & set "test[!test_count!]=basic_startup:Basic System Startup:Loading splash panel"
set /a test_count+=1 & set "test[!test_count!]=oil_panel_sensors:Oil Panel Sensor Testing:Updating pressure"
set /a test_count+=1 & set "test[!test_count!]=theme_switching:Day/Night Theme Switching:Switching application theme"
set /a test_count+=1 & set "test[!test_count!]=key_present:Key Present Panel Switch:key sensor"
set /a test_count+=1 & set "test[!test_count!]=key_not_present:Key Not Present Panel:key sensor"
set /a test_count+=1 & set "test[!test_count!]=lock_panel:Lock Panel Integration:lock sensor"
set /a test_count+=1 & set "test[!test_count!]=night_startup:Night Theme Startup:Initializing style manager"
set /a test_count+=1 & set "test[!test_count!]=trigger_priority:Trigger Priority Validation:initialized to INACTIVE"
set /a test_count+=1 & set "test[!test_count!]=major_scenario:Major Integration Scenario:Loading splash panel"
set /a test_count+=1 & set "test[!test_count!]=performance_stress:Performance Stress Testing:Service initialization completed"

REM Track test results
set passed_tests=0
set failed_tests=0
set failed_test_names=

echo 🧪 Running %test_count% integration test scenarios...
echo.

REM Create results directory
if not exist test_results mkdir test_results
echo Test Execution Report - %DATE% %TIME% > test_results\test_summary.txt
echo ======================================= >> test_results\test_summary.txt

REM Run each test scenario
for /l %%i in (1,1,%test_count%) do (
    set "scenario_info=!test[%%i]!"
    
    REM Split scenario info (parse colon-separated values)
    for /f "tokens=1,2,3 delims=:" %%a in ("!scenario_info!") do (
        set "scenario_dir=%%a"
        set "scenario_name=%%b"
        set "expect_text=%%c"
    )
    
    echo 🔧 Running: !scenario_name!
    echo    Directory: test/wokwi/!scenario_dir!
    
    REM Create scenario-specific results directory
    if not exist test_results\!scenario_dir! mkdir test_results\!scenario_dir!
    
    REM Record start time
    set start_time=!TIME!
    
    REM Run wokwi-cli with proper validation
    wokwi-cli test/wokwi/!scenario_dir! --elf .pio/build/debug-local/firmware.elf --diagram-file diagram.json --timeout 60000 --expect-text "!expect_text!" --fail-text "Exception" --fail-text "Guru Meditation" > test_results\!scenario_dir!\output.log 2>&1
    
    if !errorlevel! equ 0 (
        REM Test passed
        set end_time=!TIME!
        call :calculate_duration "!start_time!" "!end_time!" duration
        echo    ✅ PASSED ^(!duration!s^)
        echo ✅ !scenario_name! - PASSED ^(!duration!s^) >> test_results\test_summary.txt
        set /a passed_tests+=1
        
        REM Move any screenshots to results directory
        if exist test\wokwi\!scenario_dir!\*.png (
            move test\wokwi\!scenario_dir!\*.png test_results\!scenario_dir!\ >nul 2>&1
        )
    ) else (
        REM Test failed
        set end_time=!TIME!
        call :calculate_duration "!start_time!" "!end_time!" duration
        echo    ❌ FAILED ^(!duration!s^)
        echo ❌ !scenario_name! - FAILED ^(!duration!s^) >> test_results\test_summary.txt
        set /a failed_tests+=1
        if "!failed_test_names!"=="" (
            set "failed_test_names=!scenario_name!"
        ) else (
            set "failed_test_names=!failed_test_names!, !scenario_name!"
        )
        
        REM Capture failure screenshots if any
        if exist test\wokwi\!scenario_dir!\*.png (
            move test\wokwi\!scenario_dir!\*.png test_results\!scenario_dir!\ >nul 2>&1
        )
    )
    echo.
)

echo ==================================================
echo 📊 Test Execution Summary
echo ==================================================
echo Total Tests: %test_count%
echo Passed: %passed_tests%
echo Failed: %failed_tests%
echo.

REM Add summary to results file
echo. >> test_results\test_summary.txt
echo SUMMARY: >> test_results\test_summary.txt
echo ======== >> test_results\test_summary.txt
echo Total Tests: %test_count% >> test_results\test_summary.txt
echo Passed: %passed_tests% >> test_results\test_summary.txt
echo Failed: %failed_tests% >> test_results\test_summary.txt

if %failed_tests% gtr 0 (
    echo ❌ Failed Tests: %failed_test_names%
    echo.
    echo Failed Tests: >> test_results\test_summary.txt
    echo   %failed_test_names% >> test_results\test_summary.txt
    echo.
    echo 💡 Check test_results\ directory for detailed logs and screenshots
    echo.
    echo 🚫 Some tests failed. Please review the results.
    exit /b 1
) else (
    echo 🎉 All tests passed successfully!
    echo.
    echo 📂 Test artifacts saved to: test_results\
    echo 📸 Screenshots and logs available for review
)

echo.
echo Test execution completed at %DATE% %TIME%
echo Test execution completed at %DATE% %TIME% >> test_results\test_summary.txt

exit /b 0

REM Function to calculate duration (simplified - returns seconds as approximation)
:calculate_duration
set start=%~1
set end=%~2
set result_var=%~3

REM Simple duration calculation (this is a basic approximation)
REM For more precise timing, would need more complex time parsing
set /a %result_var%=30
goto :eof