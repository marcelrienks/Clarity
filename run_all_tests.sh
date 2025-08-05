#!/bin/bash

echo "================================================================================"
echo "                    Clarity Comprehensive Test Suite Runner"
echo "================================================================================"
echo "This script runs all test suites individually to work within Unity's limitations"
echo "Total: 101 tests across 5 test suites"
echo "================================================================================"
echo

FAILED=0
TOTAL_TESTS=0

echo "[1/5] Running Sensor Tests (21 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-sensors; then
    echo "SUCCESS: Sensor tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 21))
else
    echo "ERROR: Sensor tests failed!"
    FAILED=1
fi
echo

echo "[2/5] Running Manager Core Tests (15 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-managers-core; then
    echo "SUCCESS: Manager core tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 15))
else
    echo "ERROR: Manager core tests failed!"
    FAILED=1
fi
echo


echo "[3/5] Running Component Tests (24 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-components; then
    echo "SUCCESS: Component tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 24))
else
    echo "ERROR: Component tests failed!"
    FAILED=1
fi
echo

echo "[4/5] Running Integration Tests (20 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-integration; then
    echo "SUCCESS: Integration tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 20))
else
    echo "ERROR: Integration tests failed!"
    FAILED=1
fi
echo

echo "[5/5] Running Infrastructure Tests (21 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-infrastructure; then
    echo "SUCCESS: Infrastructure tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 21))
else
    echo "ERROR: Infrastructure tests failed!"
    FAILED=1
fi
echo

echo "================================================================================"
echo "                             TEST SUITE SUMMARY"
echo "================================================================================"
if [ $FAILED -eq 0 ]; then
    echo ">>> ALL WORKING TEST SUITES PASSED! <<<"
    echo "Total tests executed: $TOTAL_TESTS/101"
    echo
    echo "Sensor Tests           [PASS] - 21 tests"
    echo "Manager Core Tests    [PASS] - 15 tests"
    echo "Component Tests        [PASS] - 24 tests"
    echo "Integration Tests      [PASS] - 20 tests"
    echo "Infrastructure Tests   [PASS] - 21 tests"
    echo
    echo "SUCCESS: All 101 tests passed!"
    exit 0
else
    echo ">>> SOME TEST SUITES FAILED! <<<"
    echo "Check the output above for failed test details."
    echo
    echo "Individual test commands:"
    echo "  pio test -e test-sensors        (21 tests)"
    echo "  pio test -e test-managers-core      (15 tests)"
    echo "  pio test -e test-components     (24 tests)"
    echo "  pio test -e test-integration    (20 tests)"
    echo "  pio test -e test-infrastructure (21 tests)"
    exit 1
fi