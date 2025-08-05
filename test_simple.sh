#!/bin/bash

echo "Simple test runner..."

test_scenarios=(
    "basic_startup:Basic System Startup:Loading splash panel"
    "oil_panel_sensors:Oil Panel Sensor Testing:Updating pressure"
)

mkdir -p test_results

for scenario_info in "${test_scenarios[@]}"; do
    OLD_IFS="$IFS"
    IFS=':' read -r scenario_dir scenario_name expect_text <<< "$scenario_info"
    IFS="$OLD_IFS"
    
    echo "🔧 Running: $scenario_name"
    echo "   Directory: test/wokwi/$scenario_dir"
    
    start_time=$(date +%s)
    
    if timeout 60s wokwi-cli test/wokwi/$scenario_dir \
        --elf .pio/build/debug-local/firmware.elf \
        --diagram-file diagram.json \
        --timeout 30000 \
        --expect-text "$expect_text" \
        --fail-text "Exception" > "test_results/${scenario_dir}_output.log" 2>&1; then
        
        end_time=$(date +%s)
        duration=$((end_time - start_time))
        echo "   ✅ PASSED (${duration}s)"
    else
        end_time=$(date +%s)
        duration=$((end_time - start_time))
        echo "   ❌ FAILED (${duration}s)"
    fi
    echo ""
done

echo "Test run completed."