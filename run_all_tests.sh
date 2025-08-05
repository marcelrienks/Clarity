#!/bin/bash

echo "================================================================================"
echo "                    Clarity Comprehensive Test Suite Runner"
echo "================================================================================"
echo "This script runs all test suites individually to work within Unity's limitations"
echo "Total: 113 tests across 5 test suites"
echo "================================================================================"
echo

FAILED=0
TOTAL_TESTS=0

echo "[PHASE 1] Running Sensor Tests (21 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-sensors; then
    echo "SUCCESS: Sensor tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 21))
else
    echo "ERROR: Sensor tests failed!"
    FAILED=1
fi
echo

echo "[PHASE 2A] Running Manager Core Tests (15 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-managers-core; then
    echo "SUCCESS: Manager core tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 15))
else
    echo "ERROR: Manager core tests failed!"
    FAILED=1
fi
echo

echo "[PHASE 2B] Running Manager Extended Tests (13 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-managers-extended; then
    echo "SUCCESS: Manager extended tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 13))
else
    echo "ERROR: Manager extended tests failed!"
    FAILED=1
fi
echo

echo "[PHASE 3] Running Component Tests (23 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-components; then
    echo "SUCCESS: Component tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 23))
else
    echo "ERROR: Component tests failed!"
    FAILED=1
fi
echo

echo "[PHASE 4] Running Integration Tests (20 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-integration; then
    echo "SUCCESS: Integration tests passed!"
    TOTAL_TESTS=$((TOTAL_TESTS + 20))
else
    echo "ERROR: Integration tests failed!"
    FAILED=1
fi
echo

echo "[PHASE 5] Running Infrastructure Tests (21 tests)..."
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
    echo ">>> ALL TEST SUITES PASSED! <<<"
    echo "Total tests executed: $TOTAL_TESTS/113"
    echo
    echo "Phase 1: Sensor Tests           [PASS] - 21 tests"
    echo "Phase 2A: Manager Core Tests    [PASS] - 15 tests"
    echo "Phase 2B: Manager Extended Tests [PASS] - 13 tests"
    echo "Phase 3: Component Tests        [PASS] - 23 tests"
    echo "Phase 4: Integration Tests      [PASS] - 20 tests"
    echo "Phase 5: Infrastructure Tests   [PASS] - 21 tests"
    echo
    echo "SUCCESS: All 113 tests passed across all system layers!"
    exit 0
else
    echo ">>> SOME TEST SUITES FAILED! <<<"
    echo "Check the output above for failed test details."
    echo
    echo "Individual test commands:"
    echo "  pio test -e test-sensors        (21 tests)"
    echo "  pio test -e test-managers-core      (15 tests)"
    echo "  pio test -e test-managers-extended (13 tests)"
    echo "  pio test -e test-components     (23 tests)"
    echo "  pio test -e test-integration    (20 tests)"
    echo "  pio test -e test-infrastructure (21 tests)"
    exit 1
fi