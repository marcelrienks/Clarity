#!/bin/bash
# Clarity Wokwi Integration Test Runner
# This script builds and runs the full system integration test in Wokwi

echo "================================================"
echo "Clarity Full System Integration Test"
echo "================================================"
echo "Platform: Wokwi ESP32 Simulator"
echo "Duration: ~7 minutes"
echo "================================================"
echo ""

# Check if Wokwi CLI is installed
if ! command -v wokwi-cli &> /dev/null; then
    echo "❌ Error: Wokwi CLI not found"
    echo "Install with: npm install -g @wokwi/cli"
    exit 1
fi

# Clean previous builds
echo "🧹 Cleaning previous builds..."
pio run -e debug-local -t clean

# Build the test firmware
echo "🔨 Building test firmware..."
pio test -e test-wokwi --without-uploading --without-testing

if [ $? -ne 0 ]; then
    echo "❌ Build failed"
    exit 1
fi

echo "✅ Build successful"
echo ""

# Run the test in Wokwi
echo "🚀 Starting Wokwi simulation..."
echo "================================================"
echo ""

cd test/wokwi

# Start Wokwi with timeout (7 minutes + buffer)
timeout 450 wokwi-cli run \
    --timeout 420000 \
    --serial-log-file integration_test.log \
    --diagram diagram.json \
    ../../.pio/build/test-wokwi/firmware.bin

TEST_RESULT=$?

echo ""
echo "================================================"
echo "Test Results"
echo "================================================"

# Check test results
if [ -f integration_test.log ]; then
    echo "📋 Test Log Summary:"
    echo "-------------------"
    
    # Extract key test results
    grep -E "(PHASE|✅|❌|TEST SUMMARY|Test Result)" integration_test.log
    
    echo ""
    echo "Full log saved to: test/wokwi/integration_test.log"
    
    # Check if test passed
    if grep -q "Test Result: PASSED" integration_test.log; then
        echo ""
        echo "🎉 Integration Test PASSED!"
        exit 0
    else
        echo ""
        echo "❌ Integration Test FAILED"
        echo "Check integration_test.log for details"
        exit 1
    fi
else
    echo "❌ No test log generated"
    exit 1
fi