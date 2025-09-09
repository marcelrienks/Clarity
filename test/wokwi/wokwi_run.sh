#!/bin/bash
#
# Secure Wokwi Test Runner for PlatformIO Integration
# This script safely loads the Wokwi token from environment and runs the test
#

# Source bash profile to get environment variables
source ~/.bashrc

# Check if token is available
if [ -z "$WOKWI_CLI_TOKEN" ]; then
    echo "Error: WOKWI_CLI_TOKEN environment variable not set."
    echo "Add to your ~/.bashrc:"
    echo "export WOKWI_CLI_TOKEN=\"your-token-here\""
    echo ""
    echo "Get your token at: https://wokwi.com/dashboard/ci"
    exit 1
fi

# Change to the wokwi test directory
cd "$(dirname "$0")"

# Run wokwi-cli with the token from environment
exec /home/marcelr/bin/wokwi-cli --timeout 35000