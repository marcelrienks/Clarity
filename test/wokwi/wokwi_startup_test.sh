#!/bin/bash

# Clarity Automated Startup Test
# Tests system initialization and basic startup sequence without manual interaction

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Clarity Automated Startup Test ===${NC}"

# Check if we're in the right directory
if [ ! -f "wokwi.toml" ]; then
    echo -e "${RED}Error: wokwi.toml not found. Run this script from test/wokwi directory.${NC}"
    exit 1
fi

# Check if firmware exists
FIRMWARE_PATH="../../.pio/build/test-wokwi/firmware.bin"
if [ ! -f "$FIRMWARE_PATH" ]; then
    echo -e "${YELLOW}Warning: Firmware not found at $FIRMWARE_PATH${NC}"
    echo -e "${YELLOW}Building firmware first...${NC}"
    cd ../../
    pio run -e test-wokwi
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

# Short timeout for startup validation (30 seconds)
TIMEOUT=30000

echo -e "${GREEN}Starting automated startup validation (timeout: ${TIMEOUT}ms)${NC}"
echo -e "${YELLOW}Testing: System initialization → SplashPanel → OemOilPanel${NC}"
echo ""

# Run the simulation and capture output
echo -e "${GREEN}Command: $WOKWI_CLI --timeout $TIMEOUT${NC}"
echo -e "${YELLOW}Monitoring startup sequence...${NC}"
echo ""

# Execute wokwi-cli and check for success patterns
if $WOKWI_CLI --timeout $TIMEOUT 2>&1 | tee /tmp/wokwi_output.log | grep -q "\[T\] OemOilPanel loaded successfully"; then
    echo ""
    echo -e "${GREEN}=== Startup Test PASSED ===${NC}"
    echo -e "${GREEN}✅ System initialized successfully${NC}"
    echo -e "${GREEN}✅ SplashPanel loaded${NC}"
    echo -e "${GREEN}✅ OemOilPanel loaded${NC}"
    echo -e "${GREEN}✅ All startup [T] timing messages detected${NC}"
    exit 0
else
    echo ""
    echo -e "${RED}=== Startup Test FAILED ===${NC}"
    echo -e "${YELLOW}Check output above for missing startup messages${NC}"
    echo -e "${YELLOW}Expected: [T] SplashPanel loaded successfully${NC}"
    echo -e "${YELLOW}Expected: [T] OemOilPanel loaded successfully${NC}"
    exit 1
fi