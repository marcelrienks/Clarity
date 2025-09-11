#!/bin/bash

# Clarity Wokwi Integration Test Runner
# Simple script to execute the integration test with wokwi-cli

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Clarity Integration Test Runner ===${NC}"

# Check if we're in the right directory
if [ ! -f "wokwi.toml" ]; then
    echo -e "${RED}Error: wokwi.toml not found. Run this script from test/wokwi directory.${NC}"
    exit 1
fi

# Check if firmware exists
FIRMWARE_PATH="../../.pio/build/debug-local/firmware.bin"
if [ ! -f "$FIRMWARE_PATH" ]; then
    echo -e "${YELLOW}Warning: Firmware not found at $FIRMWARE_PATH${NC}"
    echo -e "${YELLOW}Building firmware first...${NC}"
    cd ../../
    pio run -e debug-local
    cd test/wokwi/
    if [ ! -f "$FIRMWARE_PATH" ]; then
        echo -e "${RED}Error: Failed to build firmware${NC}"
        exit 1
    fi
fi

echo -e "${GREEN}Found firmware: $FIRMWARE_PATH${NC}"

# Check if wokwi-cli is available
if ! command -v wokwi-cli &> /dev/null; then
    # Try user's home bin directory (common wokwi-cli install location)
    if [ -f "$HOME/bin/wokwi-cli" ]; then
        WOKWI_CLI="$HOME/bin/wokwi-cli"
    elif [ -f "$HOME/.wokwi/bin/wokwi-cli" ]; then
        WOKWI_CLI="$HOME/.wokwi/bin/wokwi-cli"
    else
        echo -e "${RED}Error: wokwi-cli not found in PATH or common locations${NC}"
        echo -e "${YELLOW}Please install wokwi-cli or add it to your PATH${NC}"
        exit 1
    fi
else
    WOKWI_CLI="wokwi-cli"
fi

echo -e "${GREEN}Using wokwi-cli: $WOKWI_CLI${NC}"

# Set timeout (5 minutes for full integration test)
TIMEOUT=${TIMEOUT:-300000}

echo -e "${GREEN}Starting Wokwi simulation with timeout: ${TIMEOUT}ms${NC}"
echo -e "${YELLOW}Integration test will run for up to 5 minutes...${NC}"
echo ""

# Run the simulation
echo -e "${GREEN}Command: $WOKWI_CLI --timeout $TIMEOUT${NC}"
echo -e "${YELLOW}Watching serial output for test patterns...${NC}"
echo ""

# Execute wokwi-cli with timeout
if $WOKWI_CLI --timeout $TIMEOUT; then
    echo ""
    echo -e "${GREEN}=== Integration Test Completed Successfully ===${NC}"
    echo -e "${GREEN}Check the serial output above for test progression${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}=== Integration Test Failed or Timed Out ===${NC}"
    echo -e "${YELLOW}Check the serial output above for errors or incomplete test phases${NC}"
    exit 1
fi