#!/bin/bash

# Wokwi Automated Test Runner for Clarity Automotive Gauge System
# This script runs all integration test scenarios using Wokwi CLI

# Note: No 'set -e' to allow script to continue if individual tests fail

echo "🚗 Clarity Automotive Gauge - Wokwi Integration Tests"
echo "=================================================="

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if WOKWI_CLI_TOKEN is set
if [ -z "$WOKWI_CLI_TOKEN" ]; then
    echo -e "${RED}❌ Error: WOKWI_CLI_TOKEN environment variable not set${NC}"
    echo "Please set your Wokwi CLI token:"
    echo "export WOKWI_CLI_TOKEN=\"your_token_here\""
    exit 1
fi

# Check if wokwi-cli is installed
if ! command -v wokwi-cli &> /dev/null; then
    echo -e "${RED}❌ Error: wokwi-cli not found${NC}"
    echo "Please install Wokwi CLI:"
    echo "curl -L https://wokwi.com/ci/install.sh | sh"
    exit 1
fi

echo -e "${BLUE}📦 Building firmware...${NC}"
if ! pio run -e debug-local; then
    echo -e "${RED}❌ Build failed!${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Firmware build successful${NC}"
echo ""

# Test scenarios to run with their expected validation text
test_scenarios=(
    "basic_startup:Basic System Startup:Loading splash panel"
    "oil_panel_sensors:Oil Panel Sensor Testing:Updating pressure" 
    "theme_switching:Day/Night Theme Switching:Switching application theme"
    "key_present:Key Present Panel Switch:key sensor"
    "key_not_present:Key Not Present Panel:key sensor"
    "lock_panel:Lock Panel Integration:lock sensor"
    "night_startup:Night Theme Startup:Initializing style manager"
    "trigger_priority:Trigger Priority Validation:initialized to INACTIVE"
    "major_scenario:Major Integration Scenario:Loading splash panel"
    "performance_stress:Performance Stress Testing:Service initialization completed"
)

# Track test results
total_tests=${#test_scenarios[@]}
passed_tests=0
failed_tests=0
failed_test_names=()

echo -e "${BLUE}🧪 Running $total_tests integration test scenarios...${NC}"
echo ""

# Create results directory
mkdir -p test_results
echo "Test Execution Report - $(date)" > test_results/test_summary.txt
echo "=======================================" >> test_results/test_summary.txt

# Run each test scenario  
for scenario_info in "${test_scenarios[@]}"; do
    # Split scenario info using parameter expansion (more reliable)
    scenario_dir="${scenario_info%%:*}"
    temp="${scenario_info#*:}"
    scenario_name="${temp%%:*}"
    expect_text="${temp#*:}"
    
    echo -e "${YELLOW}🔧 Running: $scenario_name${NC}"
    echo "   Directory: test/wokwi/$scenario_dir"
    
    # Create scenario-specific results directory
    mkdir -p test_results/$scenario_dir
    
    # Run the test with timeout and capture output using shared config
    start_time=$(date +%s)
    
    # Run wokwi-cli with proper validation using expect-text and fail-text
    if timeout 120s wokwi-cli test/wokwi/$scenario_dir \
        --elf .pio/build/debug-local/firmware.elf \
        --diagram-file diagram.json \
        --timeout 60000 \
        --expect-text "$expect_text" \
        --fail-text "Exception" \
        --fail-text "Guru Meditation" > "test_results/$scenario_dir/output.log" 2>&1; then
    
        end_time=$(date +%s)
        duration=$((end_time - start_time))
        
        echo -e "${GREEN}   ✅ PASSED${NC} (${duration}s)"
        echo "✅ $scenario_name - PASSED (${duration}s)" >> test_results/test_summary.txt
        ((passed_tests++))
        
        # Move any screenshots to results directory from test directory
        if compgen -G "test/wokwi/$scenario_dir/*.png" > /dev/null; then
            mv test/wokwi/$scenario_dir/*.png test_results/$scenario_dir/
        fi
        
    else
        end_time=$(date +%s)
        duration=$((end_time - start_time))
        
        echo -e "${RED}   ❌ FAILED${NC} (${duration}s)"
        echo "❌ $scenario_name - FAILED (${duration}s)" >> test_results/test_summary.txt
        failed_test_names+=("$scenario_name")
        ((failed_tests++))
        
        # Capture failure screenshots if any  
        if compgen -G "test/wokwi/$scenario_dir/*.png" > /dev/null; then
            mv test/wokwi/$scenario_dir/*.png test_results/$scenario_dir/
        fi
    fi
    echo ""
done

echo "=================================================="
echo -e "${BLUE}📊 Test Execution Summary${NC}"
echo "=================================================="
echo -e "Total Tests: $total_tests"
echo -e "${GREEN}Passed: $passed_tests${NC}"
echo -e "${RED}Failed: $failed_tests${NC}"
echo ""

# Add summary to results file
echo "" >> test_results/test_summary.txt
echo "SUMMARY:" >> test_results/test_summary.txt
echo "========" >> test_results/test_summary.txt
echo "Total Tests: $total_tests" >> test_results/test_summary.txt
echo "Passed: $passed_tests" >> test_results/test_summary.txt
echo "Failed: $failed_tests" >> test_results/test_summary.txt

if [ $failed_tests -gt 0 ]; then
    echo -e "${RED}❌ Failed Tests:${NC}"
    for failed_test in "${failed_test_names[@]}"; do
        echo -e "${RED}   • $failed_test${NC}"
    done
    echo ""
    echo "Failed Tests:" >> test_results/test_summary.txt
    for failed_test in "${failed_test_names[@]}"; do
        echo "  • $failed_test" >> test_results/test_summary.txt
    done
    
    echo -e "${YELLOW}💡 Check test_results/ directory for detailed logs and screenshots${NC}"
    echo ""
    echo -e "${RED}🚫 Some tests failed. Please review the results.${NC}"
    exit 1
else
    echo -e "${GREEN}🎉 All tests passed successfully!${NC}"
    echo ""
    echo -e "${BLUE}📂 Test artifacts saved to: test_results/${NC}"
    echo -e "${BLUE}📸 Screenshots and logs available for review${NC}"
fi

echo ""
echo "Test execution completed at $(date)"
echo "Test execution completed at $(date)" >> test_results/test_summary.txt