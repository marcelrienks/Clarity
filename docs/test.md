# Clarity Integration Testing

This document provides comprehensive instructions for running the Clarity digital gauge system integration tests using the Wokwi ESP32 simulator.

## Overview

The integration test validates the complete Clarity system functionality by executing the Primary Integration Test Scenario defined in `docs/scenarios.md`. It covers all 6 panels, 5 interrupts, theme switching, sensor interactions, error handling, and configuration management in a realistic user interaction flow.

## Test Structure

### Test Files Location
```
test/wokwi/
├── diagram.json           # Hardware configuration
├── wokwi.toml            # Wokwi simulator settings  
├── integration-test.yaml # Complete test specification
├── wokwi_run.sh          # Simple bash test runner
└── integration_test.py   # Advanced Python test runner
```

### Hardware Configuration

The test uses a comprehensive hardware setup simulating real automotive gauge scenarios:

- **ESP32 DevKit-C v4**: Main microcontroller
- **ILI9341 Display**: 240x240 simulation (note: actual target is round GC9A01)
- **Action Button (btn1)**: GPIO 32 - Main user input button
- **Debug Button (btn2)**: GPIO 34 - Debug error trigger activation
- **Pressure Potentiometer (pot1)**: GPIO 36 - Oil pressure sensor simulation
- **Temperature Potentiometer (pot2)**: GPIO 39 - Oil temperature sensor simulation
- **4-Position DIP Switch (sw1)**: Trigger simulation controls
  - Position 1 (GPIO 25): Key Present trigger
  - Position 2 (GPIO 26): Key Not Present trigger  
  - Position 3 (GPIO 27): Lock trigger
  - Position 4 (GPIO 33): Lights trigger

## Prerequisites

### 1. Build Environment Setup
Ensure you have PlatformIO installed and the project builds successfully:
```bash
# Test build
pio run -e debug-local

# Check that firmware is created
ls .pio/build/debug-local/firmware.bin
```

### 2. Wokwi CLI Installation
Install the Wokwi CLI tool from one of these sources:

**Option 1: Direct Download**
```bash
# Download and install to ~/bin/
curl -L https://github.com/wokwi/wokwi-cli/releases/latest/download/wokwi-cli-linux -o ~/bin/wokwi-cli
chmod +x ~/bin/wokwi-cli
```

**Option 2: NPM Installation**
```bash
npm install -g @wokwi/cli
```

**Option 3: Check existing installation**
```bash
# Test if already installed
wokwi-cli --version
```

### 3. Python Requirements (for advanced runner)
```bash
# Python 3.6+ required
python3 --version

# No additional packages needed - uses only standard library
```

## Running the Integration Test

### Method 1: Simple Shell Script (Recommended)

The easiest way to run the complete integration test:

```bash
# Navigate to test directory
cd test/wokwi

# Run the integration test
./wokwi_run.sh
```

**Features:**
- Automatically builds firmware if needed
- Finds wokwi-cli in common locations
- Runs 5-minute test with colored output
- Shows real-time serial monitor output

**Expected Output:**
```
=== Clarity Integration Test Runner ===
Found firmware: ../../.pio/build/debug-local/firmware.bin
Using wokwi-cli: /home/user/bin/wokwi-cli
Starting Wokwi simulation with timeout: 300000ms

[Serial output showing test progression...]

=== Integration Test Completed Successfully ===
```

### Method 2: Advanced Python Runner

For detailed test phase tracking and analysis:

```bash
# Navigate to test directory
cd test/wokwi

# Run with Python test runner
python3 integration_test.py
```

**Features:**
- Real-time serial pattern recognition
- Test phase completion tracking
- Detailed test summary report
- Automatic test progression analysis

**Expected Output:**
```
🧪 Clarity Integration Test
========================================
✅ Using existing firmware: ../../.pio/build/debug-local/firmware.bin
🚀 Starting Wokwi simulation (timeout: 300000ms)...

📡 Serial Monitor Output:
==================================================
[Real-time serial output with test progression...]

==================================================
📊 Test Phase Summary:
  ✅ Startup
  ✅ Sensor Interaction  
  ✅ Theme Trigger
  ✅ Error System
  ✅ Configuration

📈 Overall Progress: 5/5 phases completed
✅ Integration test completed successfully!
```

### Method 3: Direct Wokwi CLI

For manual control and custom parameters:

```bash
# Navigate to test directory
cd test/wokwi

# Run with custom timeout (default: 300000ms = 5 minutes)
wokwi-cli --timeout 300000

# Run with shorter timeout for quick testing
wokwi-cli --timeout 60000

# Run indefinitely (Ctrl+C to stop)
wokwi-cli
```

## Test Phases and Expected Behavior

The integration test follows 7 main phases covering all aspects of the Primary Integration Test Scenario:

### Phase 1: System Startup
- **Duration**: ~15 seconds
- **Expected**: Splash panel animation → Oil panel with animated gauges
- **Key Serial Patterns**:
  ```
  Starting Clarity service initialization
  SplashPanel loaded successfully
  OemOilPanel loaded successfully
  ```

### Phase 2: Sensor Interaction
- **Duration**: ~5 seconds
- **Expected**: Potentiometer adjustments cause gauge animations
- **Key Serial Patterns**:
  ```
  Pressure reading changed from ADC
  Temperature reading changed from ADC
  Gauge animation smooth update
  ```

### Phase 3: Theme and Trigger System
- **Duration**: ~15 seconds
- **Expected**: Night theme → Lock panel → Key panels → Restoration chain
- **Key Serial Patterns**:
  ```
  LightsOnActivate() - Setting NIGHT theme
  LockEngagedActivate() - Loading LOCK panel
  KeyNotPresentActivate() - Loading KEY panel
  KeyPresentActivate() - Loading KEY panel
  ```

### Phase 4: Error System Integration
- **Duration**: ~10 seconds
- **Expected**: Error panel → Navigate errors → Clear all errors
- **Key Serial Patterns**:
  ```
  ErrorOccurredActivate() - Loading ERROR panel
  ErrorManager: 3 errors reported
  Error navigation - cycling through errors
  Error resolution - all errors cleared
  ```

### Phase 5: Trigger Chain Restoration
- **Duration**: ~5 seconds
- **Expected**: Return to Oil panel when all triggers deactivated
- **Key Serial Patterns**:
  ```
  No blocking interrupts - restoring to 'OemOilPanel'
  ```

### Phase 6: Theme Restoration
- **Duration**: ~3 seconds  
- **Expected**: Return to day theme
- **Key Serial Patterns**:
  ```
  LightsOffActivate() - Setting DAY theme
  Theme changed to Day
  ```

### Phase 7: Configuration System
- **Duration**: ~20 seconds
- **Expected**: Config navigation → Theme settings → Apply night theme → Exit
- **Key Serial Patterns**:
  ```
  Config panel navigation started
  Theme settings submenu accessed
  Night theme applied via configuration
  Config exit - returning to oil panel
  ```

## Success Criteria

### ✅ Core Functionality
- [ ] All 6 panels display correctly (Splash, Oil, Key, Lock, Error, Config)
- [ ] All 5 interrupts function with correct priorities
- [ ] Theme switching works (Day/Night) 
- [ ] Button actions work (short vs long press)
- [ ] Sensor animations run smoothly
- [ ] Configuration system navigates properly

### ✅ Integration Points  
- [ ] Panel priorities: Error/Key (CRITICAL) > Lock (IMPORTANT) > Oil (default)
- [ ] Trigger restoration logic works correctly
- [ ] Theme persists across panel switches
- [ ] Animations don't block other operations
- [ ] Error system overrides all other panels
- [ ] Config changes apply immediately

### ✅ Performance
- [ ] No memory leaks or crashes
- [ ] System remains responsive throughout test
- [ ] Smooth gauge animations
- [ ] Consistent frame rate maintained

## Troubleshooting

### Build Issues

**Problem**: Firmware build fails
```bash
# Clean and rebuild
pio run -e debug-local --target clean
pio run -e debug-local

# Check for missing dependencies
pio lib list
```

**Problem**: Wrong build environment
```bash
# Ensure using debug-local environment (fastest build)
pio run -e debug-local

# Available environments:
# - debug-local: Fast build for testing
# - debug-upload: With inverted colors for waveshare
# - release: Optimized build
```

### Wokwi CLI Issues

**Problem**: wokwi-cli not found
```bash
# Check if installed
which wokwi-cli

# Install if missing
curl -L https://github.com/wokwi/wokwi-cli/releases/latest/download/wokwi-cli-linux -o ~/bin/wokwi-cli
chmod +x ~/bin/wokwi-cli

# Add to PATH if needed
echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

**Problem**: Simulation timeout
```bash
# Increase timeout for slower systems
TIMEOUT=600000 ./wokwi_run.sh

# Or run with custom timeout
wokwi-cli --timeout 600000
```

### Test Execution Issues

**Problem**: Test appears stuck
- Check serial output for error messages
- Some phases take longer (Configuration: ~20 seconds)
- Press Ctrl+C to interrupt if needed

**Problem**: Missing test phases
- Check that all DIP switches are in OFF position at start
- Verify button connections in diagram.json
- Monitor serial patterns for phase completion

**Problem**: Serial output too fast
```bash
# Use Python runner for better tracking
python3 integration_test.py

# Or redirect output to file
./wokwi_run.sh > test_output.log 2>&1
```

### Hardware Simulation Issues

**Problem**: Button presses not registered
- Wokwi UI: Click buttons in the browser simulator
- Check GPIO connections in diagram.json
- Verify button bounce settings

**Problem**: Potentiometer changes not detected
- Wokwi UI: Drag potentiometer knobs to change values
- ADC readings should appear in serial output
- Range: 0-4095 (12-bit ADC)

**Problem**: DIP switch not working
- Wokwi UI: Click individual switch positions
- Switches control GPIO triggers (25, 26, 27, 33)
- Ensure switches start in OFF position

## Test Customization

### Modifying Test Duration

Edit `wokwi_run.sh` or `integration_test.py`:
```bash
# Change default timeout (milliseconds)
TIMEOUT=180000  # 3 minutes instead of 5
```

### Adding Custom Test Steps

Edit `test/wokwi/integration-test.yaml`:
```yaml
# Add new test phase
- name: "Custom Test Phase"
  steps:
    - name: "Custom Step"
      action: "wait_for_serial"
      pattern: "Custom Pattern"
      timeout: 5000
```

### Monitoring Specific Patterns

Edit `integration_test.py` patterns dictionary:
```python
patterns = {
    "custom_phase": [
        "Custom pattern 1",
        "Custom pattern 2"
    ]
}
```

## Integration with CI/CD

### GitHub Actions Example
```yaml
name: Integration Test
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install PlatformIO
        run: pip install platformio
      - name: Install Wokwi CLI
        run: |
          curl -L https://github.com/wokwi/wokwi-cli/releases/latest/download/wokwi-cli-linux -o /usr/local/bin/wokwi-cli
          chmod +x /usr/local/bin/wokwi-cli
      - name: Run Integration Test
        run: |
          cd test/wokwi
          timeout 600 ./wokwi_run.sh
```

### Local Pre-commit Hook
```bash
#!/bin/sh
# .git/hooks/pre-commit
echo "Running integration test..."
cd test/wokwi
timeout 300 ./wokwi_run.sh || exit 1
echo "Integration test passed!"
```

## Performance Testing Notes

### Memory Usage Monitoring
```bash
# Add to serial monitoring patterns
"Free heap:"
"Largest free block:"
"PSRAM free:"
```

### Timing Analysis
```bash
# Monitor specific operations
"Panel load time:"
"Animation frame time:"
"Interrupt processing time:"
```

### Load Testing
```bash
# Rapid trigger activation
for i in {1..10}; do
  # Simulate rapid button presses
  echo "Rapid test cycle $i"
done
```

## Test Results Documentation

### Expected Test Duration
- **Full Integration Test**: ~90 seconds actual execution
- **With 5-minute timeout**: Allows for slow system performance
- **Phase Breakdown**:
  - Startup: 15s
  - Sensor Interaction: 5s  
  - Theme/Trigger System: 15s
  - Error System: 10s
  - Trigger Restoration: 5s
  - Theme Restoration: 3s
  - Configuration System: 20s
  - Buffer time: ~17s

### Memory Usage Expectations
- **Startup heap**: ~200KB free
- **During animations**: ~180KB free minimum
- **Panel transitions**: Brief memory allocation spikes
- **Error states**: Additional ~5KB for error queue

### Performance Benchmarks
- **Panel load time**: <500ms
- **Theme switch time**: <100ms (instant)
- **Button response time**: <100ms
- **Gauge animation**: Smooth 60fps target
- **Interrupt processing**: <10ms per trigger

This integration test provides comprehensive validation of the Clarity system before and after optimization work, ensuring all functionality remains intact while performance improvements are implemented.