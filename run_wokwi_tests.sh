#!/bin/bash

# Enhanced Wokwi Test Runner with Comprehensive Validation
# Uses YAML scenario files for detailed testing where available

echo "🚗 Clarity Automotive Gauge - Complete Wokwi Integration Tests"
echo "============================================================="

# Check if WOKWI_CLI_TOKEN is set
if [ -z "$WOKWI_CLI_TOKEN" ]; then
    echo "❌ Error: WOKWI_CLI_TOKEN environment variable not set"
    echo "Please set your Wokwi CLI token:"
    echo "export WOKWI_CLI_TOKEN=\"your_token_here\""
    exit 1
fi

# Check if wokwi-cli is installed
if ! command -v wokwi-cli &> /dev/null; then
    echo "❌ Error: wokwi-cli not found"
    echo "Please install Wokwi CLI:"
    echo "curl -L https://wokwi.com/ci/install.sh | sh"
    exit 1
fi

echo "📦 Building firmware..."
if ! pio run -e debug-local; then
    echo "❌ Build failed!"
    exit 1
fi
echo "✅ Firmware build successful"
echo ""

# Complete test scenarios with YAML automation
test_scenarios=(
    "basic_startup:Basic System Startup:basic_startup.test.yaml:Loading OEM oil panel"
    "oil_panel_sensors:Oil Panel Sensor Testing:oil_panel_sensors.test.yaml:Updating pressure"
    "theme_switching:Day/Night Theme Switching:theme_switching.test.yaml:Loading OEM oil panel"
    "night_startup:Night Theme Startup:night_startup.test.yaml:Loading OEM oil panel"
    "key_present:Key Present Panel Switch:key_present.test.yaml:Loading OEM oil panel"
    "key_not_present:Key Not Present Panel Switch:key_not_present.test.yaml:Loading OEM oil panel"
    "lock_panel:Lock Panel Integration:lock_panel.test.yaml:Loading OEM oil panel"
    "startup_triggers:Startup Triggers Validation:startup_triggers.test.yaml:Loading OEM oil panel"
    "startup_triggers_simple:Simple Startup Triggers:startup_triggers_simple.test.yaml:Loading OEM oil panel"
    "trigger_priority:Trigger Priority Validation:trigger_priority.test.yaml:Loading OEM oil panel"
    "major_scenario:Major Integration Scenario:major_scenario.test.yaml:Loading OEM oil panel"
)

# Track test results
total_tests=${#test_scenarios[@]}
passed_tests=0
failed_tests=0
failed_test_names=()

echo "🧪 Running $total_tests comprehensive integration test scenarios..."
echo ""

# Create results directory
mkdir -p test_results
echo "Complete Test Execution Report - $(date)" > test_results/test_summary.txt
echo "=============================================" >> test_results/test_summary.txt

# Run each test scenario
for scenario_info in "${test_scenarios[@]}"; do
    # Split scenario info: id:name:yaml:expect
    scenario_id="${scenario_info%%:*}"
    temp="${scenario_info#*:}"
    scenario_name="${temp%%:*}"
    temp="${temp#*:}"
    yaml_file="${temp%%:*}"
    expect_text="${temp#*:}"
    
    echo "🔧 Running: $scenario_name"
    echo "   Test file: $yaml_file"
    
    # Create scenario-specific results directory
    mkdir -p test_results/$scenario_id
    
    # Record start time
    start_time=$(date +%s)
    
    # Build wokwi-cli command
    cmd="wokwi-cli test/wokwi --elf .pio/build/debug-local/firmware.elf --timeout 60000"
    
    # Add YAML scenario if available
    if [ -n "$yaml_file" ] && [ -f "test/wokwi/$yaml_file" ]; then
        echo "   📋 Using enhanced YAML: $yaml_file"
        cmd="$cmd --scenario $yaml_file"
    else
        echo "   📝 Using basic validation: $expect_text"
        cmd="$cmd --expect-text \"$expect_text\" --fail-text \"Exception\" --fail-text \"Guru Meditation\""
    fi
    
    # Run the test
    if eval "$cmd" > "test_results/$scenario_id/output.log" 2>&1; then
        end_time=$(date +%s)
        duration=$((end_time - start_time))
        
        echo "   ✅ PASSED (${duration}s)"
        echo "✅ $scenario_name - PASSED (${duration}s)" >> test_results/test_summary.txt
        ((passed_tests++))
        
        # Move any screenshots to results directory
        if compgen -G "test/wokwi/*.png" > /dev/null; then
            mv test/wokwi/*.png test_results/$scenario_id/
        fi
        
    else
        end_time=$(date +%s)
        duration=$((end_time - start_time))
        
        echo "   ❌ FAILED (${duration}s)"
        echo "❌ $scenario_name - FAILED (${duration}s)" >> test_results/test_summary.txt
        failed_test_names+=("$scenario_name")
        ((failed_tests++))
        
        # Capture failure screenshots if any
        if compgen -G "test/wokwi/*.png" > /dev/null; then
            mv test/wokwi/*.png test_results/$scenario_id/
        fi
    fi
    echo ""
done

echo "============================================================="
echo "📊 Test Execution Summary"
echo "============================================================="
echo "Total Tests: $total_tests"
echo "Passed: $passed_tests ✅"
echo "Failed: $failed_tests ❌"
echo ""

# Add summary to results file
echo "" >> test_results/test_summary.txt
echo "COMPLETE SUMMARY:" >> test_results/test_summary.txt
echo "=================" >> test_results/test_summary.txt
echo "Total Tests: $total_tests" >> test_results/test_summary.txt
echo "Passed: $passed_tests" >> test_results/test_summary.txt
echo "Failed: $failed_tests" >> test_results/test_summary.txt

if [ $failed_tests -gt 0 ]; then
    echo "❌ Failed Tests:"
    for failed_test in "${failed_test_names[@]}"; do
        echo "   • $failed_test"
    done
    echo ""
    echo "Failed Tests:" >> test_results/test_summary.txt
    for failed_test in "${failed_test_names[@]}"; do
        echo "  • $failed_test" >> test_results/test_summary.txt
    done
    
    echo "💡 Check test_results/ directory for detailed logs and screenshots"
    echo ""
    echo "🚫 Some tests failed. Please review the results."
    exit 1
else
    echo "🎉 All comprehensive tests passed successfully!"
    echo ""
    echo "📂 Test artifacts saved to: test_results/"
    echo "📸 Screenshots and logs available for review"
fi

echo ""
echo "Test execution completed at $(date)"
echo "Test execution completed at $(date)" >> test_results/test_summary.txt