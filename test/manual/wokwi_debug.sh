#!/bin/bash

# Clarity Manual Debug Testing with Wokwi
# Full verbose logging for manual testing and debugging

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Clarity Manual Debug Testing ===${NC}"
echo -e "${YELLOW}Full verbose logging enabled for debugging${NC}"

# Check if we're in the right directory
if [ ! -f "wokwi.toml" ]; then
    echo -e "${RED}Error: wokwi.toml not found. Run this script from test/manual directory.${NC}"
    exit 1
fi

# Check if firmware exists
FIRMWARE_PATH="../../.pio/build/debug-local/firmware.bin"
if [ ! -f "$FIRMWARE_PATH" ]; then
    echo -e "${YELLOW}Warning: Firmware not found at $FIRMWARE_PATH${NC}"
    echo -e "${YELLOW}Building firmware first...${NC}"
    cd ../../
    pio run -e debug-local
    cd test/manual/
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

# Manual testing mode - no timeout by default
TIMEOUT=${TIMEOUT:-0}

if [ "$TIMEOUT" -eq 0 ]; then
    echo -e "${GREEN}Starting Wokwi simulation in manual mode (no timeout)${NC}"
    echo -e "${YELLOW}Press Ctrl+C to stop the simulation${NC}"
    echo -e "${BLUE}Full verbose logging is enabled - you'll see [V], [D], [I], [W], [E], and [T] messages${NC}"
else
    echo -e "${GREEN}Starting Wokwi simulation with timeout: ${TIMEOUT}ms${NC}"
fi

echo ""
echo -e "${BLUE}=== Hardware Setup ===${NC}"
echo -e "${YELLOW}Action Button (btn1):  GPIO 32 - Main user input${NC}"
echo -e "${YELLOW}Debug Button (btn2):   GPIO 34 - Debug error trigger${NC}"  
echo -e "${YELLOW}Pressure Pot (pot1):   GPIO 36 - Oil pressure sensor${NC}"
echo -e "${YELLOW}Temperature Pot (pot2): GPIO 39 - Oil temperature sensor${NC}"
echo -e "${YELLOW}DIP Switch (sw1):      GPIO 25,26,27,33 - Trigger controls${NC}"
echo -e "${YELLOW}  Position 1: Key Present (GPIO 25)${NC}"
echo -e "${YELLOW}  Position 2: Key Not Present (GPIO 26)${NC}"
echo -e "${YELLOW}  Position 3: Lock (GPIO 27)${NC}"
echo -e "${YELLOW}  Position 4: Lights (GPIO 33)${NC}"
echo ""

# Run the simulation
echo -e "${GREEN}Command: $WOKWI_CLI$([ "$TIMEOUT" -ne 0 ] && echo " --timeout $TIMEOUT")${NC}"
echo -e "${BLUE}Serial output (all log levels):${NC}"
echo ""

# Execute wokwi-cli
if [ "$TIMEOUT" -eq 0 ]; then
    # No timeout - manual mode
    if $WOKWI_CLI; then
        echo ""
        echo -e "${GREEN}=== Manual Debug Session Completed ===${NC}"
        exit 0
    else
        echo ""
        echo -e "${RED}=== Debug Session Terminated ===${NC}"
        exit 1
    fi
else
    # With timeout
    if $WOKWI_CLI --timeout $TIMEOUT; then
        echo ""
        echo -e "${GREEN}=== Manual Debug Session Completed ===${NC}"
        exit 0
    else
        echo ""
        echo -e "${RED}=== Debug Session Failed or Timed Out ===${NC}"
        exit 1
    fi
fi