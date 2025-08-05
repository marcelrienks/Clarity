#\!/bin/bash

echo "🚗 Clarity Automotive Gauge - Wokwi Integration Tests"
echo "=================================================="

# Build firmware first
echo "📦 Building firmware..."
pio run -e debug-local
if [ $? -ne 0 ]; then 
    echo "❌ Build failed\!"
    exit 1
fi
echo "✅ Firmware build successful"
echo ""

# Test results tracking
PASSED=0
FAILED=0
TOTAL=0

# Function to run a single test
run_test() {
    local test_dir="$1"
    local test_name="$2" 
    local expect_text="$3"
    
    TOTAL=$((TOTAL + 1))
    echo "🔧 Running: $test_name"
    echo "   Directory: test/wokwi/$test_dir"
    
    if wokwi-cli test/wokwi/$test_dir \
        --elf .pio/build/debug-local/firmware.elf \
        --diagram-file diagram.json \
        --timeout 60000 \
        --expect-text "$expect_text" \
        --fail-text "Exception" \
        --fail-text "Guru Meditation" > /dev/null 2>&1; then
        echo "   ✅ PASSED"
        PASSED=$((PASSED + 1))
    else
        echo "   ❌ FAILED"
        FAILED=$((FAILED + 1))
    fi
    echo ""
}

# Run all tests
run_test "basic_startup" "Basic System Startup" "Loading splash panel"
run_test "oil_panel_sensors" "Oil Panel Sensor Testing" "Updating pressure"

# Final summary
echo "=================================================="
echo "📊 Test Execution Summary"
echo "=================================================="
echo "Total Tests: $TOTAL"
echo "Passed: $PASSED ✅"
echo "Failed: $FAILED ❌"
echo ""

if [ $FAILED -eq 0 ]; then
    echo "🎉 All tests passed successfully\!"
    exit 0
else
    echo "💥 Some tests failed. Check the output above."
    exit 1
fi
