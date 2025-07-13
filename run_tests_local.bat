@echo off
REM Clarity Digital Gauge System - Complete Test Suite Runner (Windows)
REM Runs all tests: Unity unit tests, Wokwi integration tests, and build verification

echo.
echo 🧪 Clarity Digital Gauge System - Complete Test Suite
echo ====================================================

REM Check prerequisites
echo 📋 Checking prerequisites...

REM Check PlatformIO
where pio.exe >nul 2>nul
if %errorlevel% neq 0 (
    echo ✗ PlatformIO not found. Please install PlatformIO first.
    pause
    exit /b 1
)
echo ✓ PlatformIO found

REM Check Wokwi CLI
if not exist "wokwi-cli.exe" (
    if not exist "wokwi-cli" (
        echo ⚠ Wokwi CLI not found in current directory
        echo ⚠ Integration tests will be skipped
        set SKIP_WOKWI=true
    ) else (
        echo ✓ Wokwi CLI found
        set SKIP_WOKWI=false
    )
) else (
    echo ✓ Wokwi CLI found
    set SKIP_WOKWI=false
)

REM Check WOKWI_CLI_TOKEN
if "%WOKWI_CLI_TOKEN%"=="" (
    if "%SKIP_WOKWI%"=="false" (
        echo ⚠ WOKWI_CLI_TOKEN environment variable not set
        echo ⚠ Integration tests will be skipped
        set SKIP_WOKWI=true
    )
)

echo.

REM 1. Run Unity Unit Tests
echo 🔬 Running Unity Unit Tests...
echo ------------------------------
pio.exe test -e test --verbose
if %errorlevel% neq 0 (
    echo ✗ Unit tests failed
    pause
    exit /b 1
)
echo ✓ Unit tests completed
echo.

REM 2. Build verification for all environments
echo 🔨 Running Build Verification...
echo --------------------------------

echo Building environment: debug-local
pio.exe run -e debug-local
if %errorlevel% neq 0 (
    echo ✗ Build failed for debug-local
    pause
    exit /b 1
)
pio.exe run -e debug-local --target size
echo ✓ Build verification for debug-local completed

echo Building environment: debug-upload
pio.exe run -e debug-upload
if %errorlevel% neq 0 (
    echo ✗ Build failed for debug-upload
    pause
    exit /b 1
)
pio.exe run -e debug-upload --target size
echo ✓ Build verification for debug-upload completed

echo Building environment: release
pio.exe run -e release
if %errorlevel% neq 0 (
    echo ✗ Build failed for release
    pause
    exit /b 1
)
pio.exe run -e release --target size
echo ✓ Build verification for release completed
echo.

REM 3. Run Wokwi Integration Tests (if available)
if "%SKIP_WOKWI%"=="false" (
    echo 🌐 Running Wokwi Integration Tests...
    echo ------------------------------------
    
    echo Building firmware for integration testing...
    pio.exe run -e debug-local
    if %errorlevel% neq 0 (
        echo ✗ Firmware build failed
        pause
        exit /b 1
    )
    echo ✓ Firmware built for integration testing
    
    echo Starting Wokwi simulation...
    if exist "wokwi-cli.exe" (
        wokwi-cli.exe test --scenario test_scenarios.yaml --timeout 120000
    ) else (
        wokwi-cli test --scenario test_scenarios.yaml --timeout 120000
    )
    if %errorlevel% neq 0 (
        echo ✗ Integration tests failed
        pause
        exit /b 1
    )
    echo ✓ Integration tests completed
) else (
    echo ⚠ Skipping Wokwi Integration Tests (CLI not available or token not set)
)

echo.
echo 🎉 All tests completed successfully!
echo ==================================
echo ✅ Unity unit tests: PASSED
echo ✅ Build verification: PASSED
if "%SKIP_WOKWI%"=="false" (
    echo ✅ Wokwi integration tests: PASSED
) else (
    echo ⚠️ Wokwi integration tests: SKIPPED
)
echo.
echo 📊 Test Summary:
echo - Unit tests verify business logic and sensor functionality
echo - Build verification ensures all environments compile correctly
if "%SKIP_WOKWI%"=="false" (
    echo - Integration tests validate complete hardware workflows
)
echo.
echo Ready for deployment! 🚀
echo.
pause