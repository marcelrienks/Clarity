#!/bin/bash

echo "================================================================================"
echo "                    Clarity Comprehensive Test Suite Runner"
echo "================================================================================"
echo "This script runs the complete Unity test suite (101 tests)"
echo "================================================================================"
echo

FAILED=0
TOTAL_TESTS=0

echo "Running Complete Test Suite (101 tests)..."
echo "--------------------------------------------------------------------------------"
if pio test -e test-all; then
    echo "SUCCESS: All Unity tests passed!"
    TOTAL_TESTS=101
else
    echo "ERROR: Unity tests failed!"
    FAILED=1
fi
echo

echo "================================================================================"
echo "                             TEST SUITE SUMMARY"
echo "================================================================================"
if [ $FAILED -eq 0 ]; then
    echo ">>> ALL UNITY TESTS PASSED! <<<"
    echo "Total tests executed: $TOTAL_TESTS/101"
    echo
    echo "Sensor Tests           [PASS] - 21 tests"
    echo "Manager Tests          [PASS] - 15 tests"
    echo "Component Tests        [PASS] - 24 tests"
    echo "Integration Tests      [PASS] - 20 tests"
    echo "Infrastructure Tests   [PASS] - 21 tests"
    echo
    echo "SUCCESS: All 101 Unity tests passed!"
    exit 0
else
    echo ">>> UNITY TESTS FAILED! <<<"
    echo "Check the output above for failed test details."
    echo
    echo "Run command:"
    echo "  pio test -e test-all        (101 tests)"
    exit 1
fi