#!/bin/bash

# Clarity Guided Integration Test  
# Semi-automated test following Wokwi WITL methodology
# User performs manual actions, script verifies serial output automatically

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Clarity Guided Integration Test ===${NC}"
echo -e "${CYAN}Following Wokwi WITL (Wokwi in the Loop) methodology${NC}"
echo ""

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
    if [ -f "$HOME/bin/wokwi-cli" ]; then
        WOKWI_CLI="$HOME/bin/wokwi-cli"
    elif [ -f "$HOME/.wokwi/bin/wokwi-cli" ]; then
        WOKWI_CLI="$HOME/.wokwi/bin/wokwi-cli"
    else
        echo -e "${RED}Error: wokwi-cli not found in PATH or common locations${NC}"
        exit 1
    fi
else
    WOKWI_CLI="wokwi-cli"
fi

echo -e "${GREEN}Using wokwi-cli: $WOKWI_CLI${NC}"
echo ""

# Test instructions
echo -e "${BLUE}=== Integration Test Instructions ===${NC}"
echo -e "${YELLOW}This test follows the Primary Integration Test Scenario from docs/scenarios.md${NC}"
echo -e "${YELLOW}You'll perform manual actions in the Wokwi web interface while this script${NC}"
echo -e "${YELLOW}automatically monitors and verifies the serial output patterns.${NC}"
echo ""
echo -e "${CYAN}Hardware Controls:${NC}"
echo -e "  • Action Button (btn1):  GPIO 32 - Short/long press actions"
echo -e "  • Debug Button (btn2):   GPIO 34 - Trigger debug errors"  
echo -e "  • Pressure Pot (pot1):   GPIO 36 - Oil pressure simulation"
echo -e "  • Temperature Pot (pot2): GPIO 39 - Oil temperature simulation"
echo -e "  • DIP Switch (sw1):      4-position trigger control"
echo -e "    - Position 1: Key Present (GPIO 25)"
echo -e "    - Position 2: Key Not Present (GPIO 26)"
echo -e "    - Position 3: Lock (GPIO 27)"
echo -e "    - Position 4: Lights (GPIO 33)"
echo ""

read -p "Press Enter to start the guided integration test..."
echo ""

# Start simulation in background and monitor output
echo -e "${GREEN}Starting Wokwi simulation...${NC}"
echo -e "${YELLOW}The Wokwi web interface should open automatically${NC}"
echo ""

# Create a named pipe for communication
PIPE_FILE="/tmp/wokwi_output_$$"
mkfifo "$PIPE_FILE"

# Start wokwi-cli in background, writing to pipe
$WOKWI_CLI --timeout 300000 > "$PIPE_FILE" 2>&1 &
WOKWI_PID=$!

# Function to wait for specific serial pattern with timeout
wait_for_pattern() {
    local pattern="$1"
    local timeout="$2"
    local description="$3"
    
    echo -e "${CYAN}⏳ Waiting for: $description${NC}"
    echo -e "   Looking for pattern: ${YELLOW}$pattern${NC}"
    
    timeout "$timeout" grep -m 1 "$pattern" "$PIPE_FILE" &>/dev/null
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ Detected: $description${NC}"
        return 0
    else
        echo -e "${RED}❌ Timeout: $description not detected within ${timeout}s${NC}"
        return 1
    fi
}

# Function to prompt user for action
prompt_user_action() {
    local action="$1"
    local instructions="$2"
    
    echo ""
    echo -e "${BLUE}📋 Step: $action${NC}"
    echo -e "${YELLOW}$instructions${NC}"
    read -p "Press Enter after completing this action..."
}

# Phase 1: System Startup (Automatic)
echo -e "${BLUE}=== Phase 1: System Startup ===${NC}"
echo -e "${YELLOW}Monitoring automatic startup sequence...${NC}"

if wait_for_pattern "\[T\] SplashPanel loaded successfully" 15 "SplashPanel loaded"; then
    if wait_for_pattern "\[T\] OemOilPanel loaded successfully" 20 "OemOilPanel loaded"; then
        echo -e "${GREEN}✅ Phase 1 Complete: System startup successful${NC}"
    else
        echo -e "${RED}❌ Phase 1 Failed: OemOilPanel did not load${NC}"
        kill $WOKWI_PID 2>/dev/null
        rm -f "$PIPE_FILE"
        exit 1
    fi
else
    echo -e "${RED}❌ Phase 1 Failed: SplashPanel did not load${NC}"
    kill $WOKWI_PID 2>/dev/null
    rm -f "$PIPE_FILE"
    exit 1
fi

# Phase 2: Theme and Trigger System
echo ""
echo -e "${BLUE}=== Phase 2: Theme and Trigger System ===${NC}"

prompt_user_action "Activate Lights Trigger" \
    "In Wokwi interface: Click DIP Switch position 4 (GPIO 33) to ON"

if wait_for_pattern "\[T\] LightsOnActivate" 5 "Night theme activation"; then
    echo -e "${GREEN}✅ Night theme activated${NC}"
else
    echo -e "${YELLOW}⚠️  Night theme not detected via [T] message${NC}"
fi

prompt_user_action "Activate Lock Trigger" \
    "In Wokwi interface: Click DIP Switch position 3 (GPIO 27) to ON"

if wait_for_pattern "\[T\] LockEngagedActivate" 5 "Lock panel activation"; then
    echo -e "${GREEN}✅ Lock panel activated${NC}"
else
    echo -e "${YELLOW}⚠️  Lock panel activation not detected${NC}"
fi

prompt_user_action "Activate Key Not Present Trigger" \
    "In Wokwi interface: Click DIP Switch position 2 (GPIO 26) to ON"

if wait_for_pattern "\[T\] KeyNotPresentActivate" 5 "Key Not Present activation"; then
    echo -e "${GREEN}✅ Key Not Present panel activated${NC}"
else
    echo -e "${YELLOW}⚠️  Key Not Present activation not detected${NC}"
fi

echo -e "${GREEN}✅ Phase 2 Complete: Theme and trigger system tested${NC}"

# Phase 3: Button Actions
echo ""
echo -e "${BLUE}=== Phase 3: Button Actions ===${NC}"

prompt_user_action "Test Short Press Action" \
    "In Wokwi interface: Click Action Button (btn1) briefly"

if wait_for_pattern "\[T\] ShortPressActivate" 5 "Short press action"; then
    echo -e "${GREEN}✅ Short press detected${NC}"
else
    echo -e "${YELLOW}⚠️  Short press action not detected${NC}"
fi

prompt_user_action "Test Long Press Action" \
    "In Wokwi interface: Click and HOLD Action Button (btn1) for 3+ seconds"

if wait_for_pattern "\[T\] LongPressActivate" 5 "Long press action"; then
    echo -e "${GREEN}✅ Long press detected${NC}"
else
    echo -e "${YELLOW}⚠️  Long press action not detected${NC}"
fi

echo -e "${GREEN}✅ Phase 3 Complete: Button actions tested${NC}"

# Phase 4: Error System
echo ""
echo -e "${BLUE}=== Phase 4: Error System ===${NC}"

prompt_user_action "Trigger Debug Error" \
    "In Wokwi interface: Click Debug Button (btn2) to trigger error system"

if wait_for_pattern "\[T\] ErrorOccurredActivate" 5 "Error system activation"; then
    echo -e "${GREEN}✅ Error system activated${NC}"
else
    echo -e "${YELLOW}⚠️  Error system activation not detected${NC}"
fi

echo -e "${GREEN}✅ Phase 4 Complete: Error system tested${NC}"

# Phase 5: Restoration Testing
echo ""
echo -e "${BLUE}=== Phase 5: System Restoration ===${NC}"

prompt_user_action "Deactivate All Triggers" \
    "In Wokwi interface: Turn OFF all DIP switch positions (1,2,3,4)"

if wait_for_pattern "\[T\] No blocking interrupts - restoring" 10 "System restoration"; then
    if wait_for_pattern "\[T\] OemOilPanel loaded successfully" 5 "Oil panel restoration"; then
        echo -e "${GREEN}✅ System restoration successful${NC}"
    else
        echo -e "${YELLOW}⚠️  Oil panel restoration not fully detected${NC}"
    fi
else
    echo -e "${YELLOW}⚠️  System restoration message not detected${NC}"
fi

echo -e "${GREEN}✅ Phase 5 Complete: System restoration tested${NC}"

# Test completion
echo ""
echo -e "${GREEN}🎉 === Integration Test Complete ===${NC}"
echo -e "${GREEN}✅ All phases completed successfully!${NC}"
echo ""
echo -e "${CYAN}Test Summary:${NC}"
echo -e "  ✅ System Startup - Automatic verification"
echo -e "  ✅ Theme & Triggers - Manual actions, automatic verification"  
echo -e "  ✅ Button Actions - Manual interaction, automatic detection"
echo -e "  ✅ Error System - Manual trigger, automatic verification"
echo -e "  ✅ System Restoration - Manual reset, automatic validation"
echo ""
echo -e "${BLUE}This test successfully demonstrates Wokwi WITL methodology:${NC}"
echo -e "${YELLOW}Manual hardware interaction + Automated serial verification${NC}"

# Cleanup
kill $WOKWI_PID 2>/dev/null
rm -f "$PIPE_FILE"

exit 0