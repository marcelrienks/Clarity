@echo off
setlocal enabledelayedexpansion

rem Enhanced Wokwi Test Runner with Comprehensive Validation
rem Uses YAML scenario files for detailed testing where available

echo 🚗 Clarity Automotive Gauge - Complete Wokwi Integration Tests
echo =============================================================

rem Check if WOKWI_CLI_TOKEN is set
if "%WOKWI_CLI_TOKEN%"=="" (
    echo ❌ Error: WOKWI_CLI_TOKEN environment variable not set
    echo Please set your Wokwi CLI token:
    echo set WOKWI_CLI_TOKEN="your_token_here"
    exit /b 1
)

rem Check if wokwi-cli is installed
where wokwi-cli >nul 2>nul
if errorlevel 1 (
    echo ❌ Error: wokwi-cli not found
    echo Please install Wokwi CLI:
    echo curl -L https://wokwi.com/ci/install.sh ^| sh
    exit /b 1
)

echo 📦 Building firmware...
pio.exe run -e debug-local
if errorlevel 1 (
    echo ❌ Build failed!
    exit /b 1
)
echo ✅ Firmware build successful
echo.

rem Complete test scenarios with YAML automation
set "test_scenarios[0]=basic_startup:Basic System Startup:basic_startup.test.yaml:Loading OEM oil panel"
set "test_scenarios[1]=oil_panel_sensors:Oil Panel Sensor Testing:oil_panel_sensors.test.yaml:Updating pressure"
set "test_scenarios[2]=theme_switching:Day/Night Theme Switching:theme_switching.test.yaml:Loading OEM oil panel"
set "test_scenarios[3]=night_startup:Night Theme Startup:night_startup.test.yaml:Loading OEM oil panel"
set "test_scenarios[4]=key_present:Key Present Panel Switch:key_present.test.yaml:Loading OEM oil panel"
set "test_scenarios[5]=key_not_present:Key Not Present Panel Switch:key_not_present.test.yaml:Loading OEM oil panel"
set "test_scenarios[6]=lock_panel:Lock Panel Integration:lock_panel.test.yaml:Loading OEM oil panel"
set "test_scenarios[7]=startup_triggers:Startup Triggers Validation:startup_triggers.test.yaml:Loading OEM oil panel"
set "test_scenarios[8]=trigger_priority:Trigger Priority Validation:trigger_priority.test.yaml:Loading OEM oil panel"
set "test_scenarios[9]=major_scenario:Major Integration Scenario:major_scenario.test.yaml:Loading OEM oil panel"

rem Track test results
set total_tests=10
set passed_tests=0
set failed_tests=0
set "failed_test_names="

echo 🧪 Running %total_tests% comprehensive integration test scenarios...
echo.

rem Create results directory
if not exist test_results mkdir test_results
echo Complete Test Execution Report - %date% %time% > test_results\test_summary.txt
echo ============================================= >> test_results\test_summary.txt

rem Run each test scenario
for /L %%i in (0,1,9) do (
    rem Parse scenario info
    for /f "tokens=1-4 delims=:" %%a in ("!test_scenarios[%%i]!") do (
        set "scenario_id=%%a"
        set "scenario_name=%%b"
        set "yaml_file=%%c"
        set "expect_text=%%d"
    )
    
    echo 🔧 Running: !scenario_name!
    echo    Test file: !yaml_file!
    
    rem Create scenario-specific results directory
    if not exist test_results\!scenario_id! mkdir test_results\!scenario_id!
    
    rem Record start time
    set start_time=!time!
    
    rem Build wokwi-cli command
    set "cmd=wokwi-cli test/wokwi --elf .pio/build/debug-local/firmware.elf --timeout 60000"
    
    rem Add YAML scenario if available
    if exist "test\wokwi\!yaml_file!" (
        echo    📋 Using enhanced YAML: !yaml_file!
        set "cmd=!cmd! --scenario !yaml_file!"
    ) else (
        echo    📝 Using basic validation: !expect_text!
        set "cmd=!cmd! --expect-text "!expect_text!" --fail-text "Exception" --fail-text "Guru Meditation""
    )
    
    rem Run the test
    !cmd! > "test_results\!scenario_id!\output.log" 2>&1
    if !errorlevel! equ 0 (
        echo    ✅ PASSED
        echo ✅ !scenario_name! - PASSED >> test_results\test_summary.txt
        set /a passed_tests+=1
        
        rem Move any screenshots to results directory
        if exist test\wokwi\*.png (
            move test\wokwi\*.png test_results\!scenario_id!\ >nul 2>&1
        )
    ) else (
        echo    ❌ FAILED
        echo ❌ !scenario_name! - FAILED >> test_results\test_summary.txt
        if "!failed_test_names!"=="" (
            set "failed_test_names=!scenario_name!"
        ) else (
            set "failed_test_names=!failed_test_names!;!scenario_name!"
        )
        set /a failed_tests+=1
        
        rem Capture failure screenshots if any
        if exist test\wokwi\*.png (
            move test\wokwi\*.png test_results\!scenario_id!\ >nul 2>&1
        )
    )
    echo.
)

echo =============================================================
echo 📊 Enhanced Test Execution Summary
echo =============================================================
echo Total Tests: %total_tests%
echo Passed: %passed_tests% ✅
echo Failed: %failed_tests% ❌
echo.

rem Add summary to results file
echo. >> test_results\test_summary.txt
echo COMPLETE SUMMARY: >> test_results\test_summary.txt
echo ================= >> test_results\test_summary.txt
echo Total Tests: %total_tests% >> test_results\test_summary.txt
echo Passed: %passed_tests% >> test_results\test_summary.txt
echo Failed: %failed_tests% >> test_results\test_summary.txt

if %failed_tests% gtr 0 (
    echo ❌ Failed Tests:
    echo Failed Tests: >> test_results\test_summary.txt
    for %%f in ("%failed_test_names:;=" "%") do (
        echo    • %%~f
        echo   • %%~f >> test_results\test_summary.txt
    )
    echo.
    echo 💡 Check test_results\ directory for detailed logs and screenshots
    echo.
    echo 🚫 Some tests failed. Please review the results.
    exit /b 1
) else (
    echo 🎉 All comprehensive tests passed successfully!
    echo.
    echo 📂 Test artifacts saved to: test_results\
    echo 📸 Screenshots and logs available for review
)

echo.
echo Test execution completed at %date% %time%
echo Test execution completed at %date% %time% >> test_results\test_summary.txt