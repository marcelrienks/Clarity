#!/bin/bash
#
# Wokwi Test Runner Script
# Supports both basic and full test modes
# Usage: ./run_test.sh [basic|full]
#

# Set test mode (default to basic)
TEST_MODE=${1:-basic}

# Set project root (script is in test/wokwi, project root is ../..)
PROJECT_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
WOKWI_DIR="$PROJECT_ROOT/test/wokwi"

# Determine PlatformIO command
if command -v pio &> /dev/null; then
    PIO_CMD="pio"
elif [ -f "$HOME/.platformio/penv/bin/pio" ]; then
    PIO_CMD="$HOME/.platformio/penv/bin/pio"
else
    echo "Error: PlatformIO (pio) not found in PATH or $HOME/.platformio/penv/bin/"
    exit 1
fi

# Determine wokwi-cli command
if command -v wokwi-cli &> /dev/null; then
    WOKWI_CMD="wokwi-cli"
elif [ -f "$HOME/.wokwi/bin/wokwi-cli" ]; then
    WOKWI_CMD="$HOME/.wokwi/bin/wokwi-cli"
elif [ -f "$HOME/bin/wokwi-cli" ]; then
    WOKWI_CMD="$HOME/bin/wokwi-cli"
else
    echo "Error: wokwi-cli not found in PATH or known locations"
    exit 1
fi

# Check for Wokwi token
if [ -z "$WOKWI_CLI_TOKEN" ]; then
    echo "Error: WOKWI_CLI_TOKEN environment variable not set."
    echo "Get your token at: https://wokwi.com/dashboard/ci"
    exit 1
fi

# Set test parameters based on mode
case $TEST_MODE in
    basic)
        ENV_NAME="test-wokwi-basic"
        CONFIG_FILE="wokwi-basic.toml"
        TIMEOUT=10000
        echo "Running BASIC test (5 phases, ~5 seconds)..."
        ;;
    full)
        ENV_NAME="test-wokwi-full"
        CONFIG_FILE="wokwi-full.toml"
        TIMEOUT=120000
        echo "Running FULL test (7 phases, ~2 minutes)..."
        ;;
    *)
        echo "Usage: $0 [basic|full]"
        echo "  basic - Quick hardware validation test (~5 seconds)"
        echo "  full  - Complete system integration test (~2 minutes)"
        exit 1
        ;;
esac

# Change to project root
cd "$PROJECT_ROOT" || exit 1

# Build the firmware
echo "Building firmware for $TEST_MODE test..."
$PIO_CMD run -e "$ENV_NAME" || {
    echo "Build failed!"
    exit 1
}

# Change to wokwi directory
cd "$WOKWI_DIR" || exit 1

# Copy the appropriate config file
echo "Setting up Wokwi configuration..."
cp "$CONFIG_FILE" wokwi.toml || {
    echo "Failed to copy Wokwi configuration!"
    exit 1
}

# Run the test
echo "Starting Wokwi simulation (timeout: $((TIMEOUT/1000)) seconds)..."
OUTPUT_FILE="/tmp/wokwi_output_$$"
$WOKWI_CMD --timeout "$TIMEOUT" | tee "$OUTPUT_FILE"

# Capture exit code
EXIT_CODE=$?

# Check if test actually passed by looking for the result message
if grep -q "WOKWI_TEST_RESULT: PASSED" "$OUTPUT_FILE"; then
    echo "✅ Test PASSED"
    rm -f "$OUTPUT_FILE"
    exit 0
elif grep -q "WOKWI_TEST_RESULT: FAILED" "$OUTPUT_FILE"; then
    echo "❌ Test FAILED"
    rm -f "$OUTPUT_FILE"
    exit 1
elif [ $EXIT_CODE -eq 42 ]; then
    # Exit code 42 is timeout - check if we saw a successful completion before timeout
    if grep -q "Test execution completed" "$OUTPUT_FILE" && grep -q "PASSED" "$OUTPUT_FILE"; then
        echo "✅ Test PASSED (completed before timeout)"
        rm -f "$OUTPUT_FILE"
        exit 0
    else
        echo "❌ Test FAILED (timeout without completion)"
        rm -f "$OUTPUT_FILE"
        exit 1
    fi
else
    echo "❌ Test FAILED (exit code: $EXIT_CODE)"
    rm -f "$OUTPUT_FILE"
    exit $EXIT_CODE
fi