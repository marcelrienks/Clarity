#!/bin/bash

# Clarity Digital Gauge System - Complete Test Suite Runner
# Runs all tests: Unity unit tests, Wokwi integration tests, and build verification

set -e  # Exit on any error

echo "🧪 Clarity Digital Gauge System - Complete Test Suite"
echo "===================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print status
print_status() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Check prerequisites
echo "📋 Checking prerequisites..."

# Check PlatformIO
if ! command -v pio &> /dev/null; then
    print_error "PlatformIO not found. Please install PlatformIO first."
    exit 1
fi
print_status "PlatformIO found"

# Check Wokwi CLI
if [ ! -f "./wokwi-cli" ]; then
    print_warning "Wokwi CLI not found in current directory"
    print_warning "Integration tests will be skipped"
    SKIP_WOKWI=true
else
    print_status "Wokwi CLI found"
    SKIP_WOKWI=false
fi

# Check WOKWI_CLI_TOKEN
if [ -z "$WOKWI_CLI_TOKEN" ] && [ "$SKIP_WOKWI" = false ]; then
    print_warning "WOKWI_CLI_TOKEN environment variable not set"
    print_warning "Integration tests will be skipped"
    print_warning "Set token with: export WOKWI_CLI_TOKEN=<your_wokwi_token>"
    SKIP_WOKWI=true
fi

echo ""

# 1. Run Unity Unit Tests
echo "🔬 Running Unity Unit Tests..."
echo "------------------------------"
pio test -e unit-test --verbose
print_status "Unit tests completed"
echo ""

# 2. Build verification for all environments
echo "🔨 Running Build Verification..."
echo "--------------------------------"

environments=("debug-local" "debug-upload" "release" "integration-test")
for env in "${environments[@]}"; do
    echo "Building environment: $env"
    pio run -e $env
    pio run -e $env --target size
    print_status "Build verification for $env completed"
done
echo ""

# 3. Run Wokwi Integration Tests (if available)
if [ "$SKIP_WOKWI" = false ]; then
    echo "🌐 Running Wokwi Integration Tests..."
    echo "------------------------------------"
    
    # Build integration-test environment specifically for integration tests
    echo "Building integration-test environment for Wokwi..."
    pio run -e integration-test
    
    # Temporarily update wokwi.toml to use integration-test build
    sed -i.bak 's|debug-local|integration-test|g' wokwi.toml
    
    # Run Wokwi simulation
    echo "Starting Wokwi simulation..."
    
    scenarios=("test/test_basic_startup.yaml" "test/test_oil_sensors.yaml" "test/test_key_trigger.yaml" "test/test_lock_trigger.yaml" "test/test_trigger_priority.yaml" "test/test_invalid_states.yaml" "test/test_theme_trigger.yaml")
    
    for scenario in "${scenarios[@]}"; do
        echo "Running scenario: $scenario"
        ./wokwi-cli . --scenario $scenario --timeout 60000
        if [ $? -ne 0 ]; then
            print_error "Integration test failed: $scenario"
            exit 1
        fi
        print_status "Scenario $scenario completed"
    done
    
    # Restore original wokwi.toml
    mv wokwi.toml.bak wokwi.toml
    
    print_status "All integration tests completed"
else
    print_warning "Skipping Wokwi Integration Tests (CLI not available or token not set)"
fi

echo ""
echo "🎉 All tests completed successfully!"
echo "=================================="
echo "✅ Unity unit tests: PASSED"
echo "✅ Build verification: PASSED"
if [ "$SKIP_WOKWI" = false ]; then
    echo "✅ Wokwi integration tests: PASSED"
else
    echo "⚠️  Wokwi integration tests: SKIPPED"
fi
echo ""
echo "📊 Test Summary:"
echo "- Unit tests verify business logic and sensor functionality"
echo "- Build verification ensures all environments compile correctly"
if [ "$SKIP_WOKWI" = false ]; then
    echo "- Integration tests validate complete hardware workflows"
fi
echo ""
echo "Ready for deployment! 🚀"