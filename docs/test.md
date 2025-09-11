# Clarity Testing Guide

This document provides comprehensive instructions for testing the Clarity digital gauge system using the Wokwi ESP32 simulator. The testing framework includes both automated integration testing and manual debugging capabilities.

## Testing Environments Overview

Clarity provides two distinct testing environments, each optimized for different use cases:

| Environment | Purpose | Log Level | Location | Build Target |
|-------------|---------|-----------|----------|--------------|
| **Integration Testing** | Automated CI/CD validation | INFO only | `test/wokwi/` | `test-wokwi` |
| **Manual Debug Testing** | Interactive development | VERBOSE (all levels) | `test/manual/` | `debug-local` |

### Integration Testing Environment
- ✅ **Clean log output** - Only `[I]`, `[W]`, `[E]`, and custom `[T]` messages
- ✅ **Automated validation** - Pattern-based test verification
- ✅ **CI/CD ready** - 5-minute timeout with pass/fail detection
- ✅ **Performance focus** - Optimized for timing measurements with `log_t()` function

### Manual Debug Testing Environment  
- ✅ **Full verbose logging** - All `[V]`, `[D]`, `[I]`, `[W]`, `[E]`, and `[T]` messages
- ✅ **Interactive control** - No timeout, manual hardware simulation
- ✅ **Development focused** - Complete debug information for troubleshooting
- ✅ **Performance tracking** - Custom `log_t()` messages alongside full debug output

## Test Structure

### Integration Testing Files (`test/wokwi/`)
```
test/wokwi/
├── diagram.json           # Hardware configuration
├── wokwi.toml            # Test-wokwi firmware configuration  
├── integration-test.yaml # Complete test specification
├── wokwi_run.sh          # Simple bash integration test runner
└── integration_test.py   # Advanced Python test runner with progress tracking
```

### Manual Debug Testing Files (`test/manual/`)
```
test/manual/
├── diagram.json          # Hardware configuration (identical to integration)
├── wokwi.toml           # Debug-local firmware configuration
└── wokwi_debug.sh       # Manual debug testing script
```

### Build Environments
- **`test-wokwi`**: Clean integration testing (CORE_DEBUG_LEVEL=3)
- **`debug-local`**: Full debug testing (CORE_DEBUG_LEVEL=5)

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
# Build for integration testing (clean logs)
pio run -e test-wokwi
ls .pio/build/test-wokwi/firmware.bin

# Build for manual debugging (verbose logs)  
pio run -e debug-local
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

## Running Tests

### Integration Testing (Automated)

#### Method 1: Simple Shell Script (Recommended)

The easiest way to run the complete integration test:

```bash
# Navigate to integration test directory
cd test/wokwi

# Run the automated integration test
./wokwi_run.sh
```

**Features:**
- Automatically builds `test-wokwi` firmware if needed
- Finds wokwi-cli in common locations
- Runs 5-minute test with colored output
- Shows real-time serial monitor output with clean logging (INFO level only)

**Expected Output:**
```
=== Clarity Integration Test Runner ===
Found firmware: ../../.pio/build/test-wokwi/firmware.bin
Using wokwi-cli: /home/user/bin/wokwi-cli
Starting Wokwi simulation with timeout: 300000ms

[Clean serial output showing only [I], [W], [E], and [T] messages...]
[T] SplashPanel loaded successfully
[T] OemOilPanel loaded successfully
[T] KeyPresentActivate() - Loading KEY panel

=== Integration Test Completed Successfully ===
```

#### Method 2: Advanced Python Runner

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

### Manual Debug Testing (Interactive)

For interactive development and debugging with full verbose output:

#### Manual Debug Session

```bash
# Navigate to manual testing directory
cd test/manual

# Start interactive debug session (no timeout)
./wokwi_debug.sh

# Or with optional timeout
TIMEOUT=300000 ./wokwi_debug.sh
```

**Features:**
- Full verbose logging - All `[V]`, `[D]`, `[I]`, `[W]`, `[E]`, and `[T]` messages
- No timeout by default - Manual control over session
- Interactive hardware simulation - Full control over buttons, switches, potentiometers
- Complete debug information - Perfect for development and troubleshooting

**Expected Output:**
```
=== Clarity Manual Debug Testing ===
Full verbose logging enabled for debugging
Found firmware: ../../.pio/build/debug-local/firmware.bin

=== Hardware Setup ===
Action Button (btn1):  GPIO 32 - Main user input
Debug Button (btn2):   GPIO 34 - Debug error trigger
[...hardware mapping details...]

Serial output (all log levels):
[V] Verbose debug messages
[D] Debug information  
[I] General information
[W] Warnings
[E] Errors
[T] Timing/performance messages ← Your custom log_t() function
```

#### Method 3: Direct Wokwi CLI

For manual control and custom parameters in either environment:

```bash
# Integration testing directory
cd test/wokwi
wokwi-cli --timeout 300000    # Clean logs, automated patterns

# Manual debugging directory  
cd test/manual
wokwi-cli                     # Full logs, no timeout
```

## Test Phases and Expected Behavior

The integration test follows 7 main phases covering all aspects of the Primary Integration Test Scenario:

### Phase 1: System Startup
- **Duration**: ~15 seconds
- **Expected**: Splash panel animation → Oil panel with animated gauges
- **Key Serial Patterns**:
  ```
  Starting Clarity service initialization
  [T] SplashPanel loaded successfully
  [T] OemOilPanel loaded successfully
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
  [T] LightsOnActivate() - Setting NIGHT theme
  [T] LockEngagedActivate() - Loading LOCK panel
  [T] KeyNotPresentActivate() - Loading KEY panel
  [T] KeyPresentActivate() - Loading KEY panel
  ```

### Phase 4: Error System Integration
- **Duration**: ~10 seconds
- **Expected**: Error panel → Navigate errors → Clear all errors
- **Key Serial Patterns**:
  ```
  [T] ErrorOccurredActivate() - Loading ERROR panel
  ErrorManager: 3 errors reported
  [T] ShortPressActivate() - Executing short press action
  [T] LongPressActivate() - Executing long press action
  ```

### Phase 5: Trigger Chain Restoration
- **Duration**: ~5 seconds
- **Expected**: Return to Oil panel when all triggers deactivated
- **Key Serial Patterns**:
  ```
  [T] No blocking interrupts - restoring to 'OemOilPanel'
  [T] OemOilPanel loaded successfully
  ```

### Phase 6: Theme Restoration
- **Duration**: ~3 seconds  
- **Expected**: Return to day theme
- **Key Serial Patterns**:
  ```
  [T] LightsOffActivate() - Setting DAY theme
  Theme changed to Day
  ```

### Phase 7: Configuration System
- **Duration**: ~20 seconds
- **Expected**: Config navigation → Theme settings → Apply night theme → Exit
- **Key Serial Patterns**:
  ```
  [T] LongPressActivate() - Executing long press action
  Config panel loaded successfully
  Theme configuration updated
  [T] OemOilPanel loaded successfully
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
# For integration testing (clean logs)
pio run -e test-wokwi

# For manual debugging (verbose logs)
pio run -e debug-local

# Available testing environments:
# - test-wokwi: Clean integration testing (CORE_DEBUG_LEVEL=3)
# - debug-local: Full debug testing (CORE_DEBUG_LEVEL=5)
# - debug-upload: With inverted colors for waveshare (CORE_DEBUG_LEVEL=3)
# - release: Optimized production build (CORE_DEBUG_LEVEL=0)
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

## log_t() Function Integration

Both testing environments now support the custom `log_t()` function for timing and performance measurements:

### Usage in Code
```cpp
#include "utilities/logging.h"  // Include the logging header

// Timing measurements
log_t("Panel loaded successfully");
log_t("Operation completed in %d ms", duration);  
log_t("Free heap: %d bytes", ESP.getFreeHeap());
log_t("Service initialization completed in %lu ms", end_time - start_time);
```

### Expected Output
```
[T] Panel loaded successfully
[T] Operation completed in 150 ms
[T] Free heap: 180432 bytes
[T] Service initialization completed in 89 ms
```

### Benefits
- ⏱️ **Performance measurement** - Easy timing of critical operations
- 🔍 **Optimization tracking** - Before/after performance comparisons  
- 🏷️ **Filtered logging** - Easy to grep/filter for timing data
- 📊 **Metrics collection** - Automated performance monitoring in CI/CD

## Development Workflow

### For Feature Development
1. **Code Changes** → Build with `debug-local` environment
2. **Manual Testing** → Use `cd test/manual && ./wokwi_debug.sh`
3. **Interactive Debugging** → Full verbose output + manual hardware control
4. **Performance Measurement** → Add `log_t()` calls for timing critical sections
5. **Fix Issues** → Repeat until stable

### For CI/CD Integration  
1. **Code Complete** → Build with `test-wokwi` environment
2. **Integration Testing** → Use `cd test/wokwi && ./wokwi_run.sh`
3. **Automated Validation** → Clean output + automated pass/fail + `log_t()` timing
4. **Deploy** → If all tests pass

### Quick Commands Reference
```bash
# Manual Debug Testing (Full verbose output)
cd test/manual && ./wokwi_debug.sh

# Integration Testing (Clean output)  
cd test/wokwi && ./wokwi_run.sh

# Build specific environments
pio run -e debug-local    # Manual testing
pio run -e test-wokwi     # Integration testing
```

This dual testing framework provides comprehensive validation of the Clarity system before and after optimization work, ensuring all functionality remains intact while performance improvements are implemented and measured.