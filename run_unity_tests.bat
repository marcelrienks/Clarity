@echo off
echo ================================================================================
echo                    Clarity Comprehensive Test Suite Runner
echo ================================================================================
echo This script runs all test suites individually to work within Unity's limitations
echo Total: 101 tests across 5 test suites
echo ================================================================================
echo.

set FAILED=0
set TOTAL_TESTS=0

echo [1/5] Running Sensor Tests (21 tests)...
echo --------------------------------------------------------------------------------
pio test -e test-sensors
if %ERRORLEVEL% neq 0 (
    echo ERROR: Sensor tests failed!
    set FAILED=1
) else (
    echo SUCCESS: Sensor tests passed!
    set /a TOTAL_TESTS+=21
)
echo.

echo [2/5] Running Manager Core Tests (15 tests)...
echo --------------------------------------------------------------------------------  
pio test -e test-managers-core
if %ERRORLEVEL% neq 0 (
    echo ERROR: Manager core tests failed!
    set FAILED=1
) else (
    echo SUCCESS: Manager core tests passed!
    set /a TOTAL_TESTS+=15
)
echo.


echo [3/5] Running Component Tests (24 tests)...
echo --------------------------------------------------------------------------------
pio test -e test-components
if %ERRORLEVEL% neq 0 (
    echo ERROR: Component tests failed!
    set FAILED=1
) else (
    echo SUCCESS: Component tests passed!
    set /a TOTAL_TESTS+=24
)
echo.

echo [4/5] Running Integration Tests (20 tests)...
echo --------------------------------------------------------------------------------
pio test -e test-integration
if %ERRORLEVEL% neq 0 (
    echo ERROR: Integration tests failed!
    set FAILED=1
) else (
    echo SUCCESS: Integration tests passed!
    set /a TOTAL_TESTS+=20
)
echo.

echo [5/5] Running Infrastructure Tests (21 tests)...
echo --------------------------------------------------------------------------------
pio test -e test-infrastructure  
if %ERRORLEVEL% neq 0 (
    echo ERROR: Infrastructure tests failed!
    set FAILED=1
) else (
    echo SUCCESS: Infrastructure tests passed!
    set /a TOTAL_TESTS+=21
)
echo.

echo ================================================================================
echo                             TEST SUITE SUMMARY
echo ================================================================================
if %FAILED% equ 0 (
    echo ^>^>^> ALL WORKING TEST SUITES PASSED! ^<^<^<
    echo Total tests executed: %TOTAL_TESTS%/101
    echo.
    echo Sensor Tests           [PASS] - 21 tests
    echo Manager Core Tests    [PASS] - 15 tests
    echo Component Tests        [PASS] - 24 tests
    echo Integration Tests      [PASS] - 20 tests
    echo Infrastructure Tests   [PASS] - 21 tests
    echo.
    echo SUCCESS: All 101 tests passed!
    exit /b 0
) else (
    echo ^>^>^> SOME TEST SUITES FAILED! ^<^<^<
    echo Check the output above for failed test details.
    echo.
    echo Individual test commands:
    echo   pio test -e test-sensors        ^(21 tests^)
    echo   pio test -e test-managers-core      ^(15 tests^)
    echo   pio test -e test-components     ^(24 tests^)
    echo   pio test -e test-integration    ^(20 tests^)
    echo   pio test -e test-infrastructure ^(21 tests^)
    exit /b 1
)