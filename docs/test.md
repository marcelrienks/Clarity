# Clarity Testing Guide

This document provides comprehensive instructions for testing the Clarity digital gauge system using the Wokwi ESP32 simulator. The testing framework includes both automated integration testing and manual debugging capabilities.

## Testing Environments Overview

Clarity provides two distinct testing environments, each optimized for different use cases:

| Environment | Purpose | Log Level | Location | Build Target |
|-------------|---------|-----------|----------|--------------|
| **Integration Testing** | Automated CI/CD validation | TEST_LOGS_ONLY | `test/wokwi/` | `test-wokwi` |
| **Manual Debug Testing** | Interactive development | VERBOSE (all levels) | `test/manual/` | `debug-local` |

### Integration Testing Environment
- ✅ **Ultra-clean log output** - ONLY `[T]` test messages (all other logs suppressed)
- ✅ **Automated test patterns** - Validates specific functionality sequences
- ✅ **2-minute timeout** - Prevents hanging in CI environments
- ✅ **Test phase tracking** - Progress monitoring with pass/fail detection

### Manual Debug Testing Environment  
- ✅ **Full verbose logging** - All `[V]`, `[D]`, `[I]`, `[W]`, `[E]`, and `[T]` messages
- ✅ **Interactive control** - No timeout, manual hardware simulation
- ✅ **Development focused** - Complete debug information for troubleshooting
- ✅ **Performance tracking** - Custom `log_t()` messages alongside full debug output

## 📋 Build Commands

**Important:** Always run PlatformIO build commands from the project root directory, not from test subdirectories.

### Manual Debug Environment:
```bash
# From project root: Build for manual testing (verbose logging)
cd /path/to/Clarity
pio run -e debug-local

# Then run manual debug session
cd test/manual
./wokwi_debug.sh
```

### Integration Test Environment:  
```bash
# From project root: Build for integration testing (TEST_LOGS_ONLY mode)
cd /path/to/Clarity
pio run -e test-wokwi

# Then run automated integration tests
cd test/wokwi
./wokwi_test_automation.sh
```

## Test Structure

### Integration Testing Files (`test/wokwi/`)
```
test/wokwi/
├── diagram_automated.json        # Hardware configuration with push buttons
├── wokwi_automated.toml         # Test-wokwi firmware configuration  
├── scenario_automated.yaml      # Complete automated test specification
├── wokwi_test_automation.sh     # Cross-platform automated test runner
└── wokwi.toml                   # Manual testing configuration
```

### Manual Debug Testing Files (`test/manual/`)
```
test/manual/
├── diagram.json          # Hardware configuration with DIP switches
├── wokwi.toml           # Debug-local firmware configuration
└── wokwi_debug.sh       # Manual debug testing script
```

### Build Environments
- **`test-wokwi`**: Ultra-clean integration testing (CORE_DEBUG_LEVEL=0 + TEST_LOGS_ONLY)
- **`debug-local`**: Full debug testing (CORE_DEBUG_LEVEL=5)

### Hardware Configuration

#### Integration Testing (Automated)
- **ESP32 DevKit-C v4**: Main microcontroller
- **ILI9341 Display**: 240x240 simulation (note: actual target is round GC9A01)
- **Action Button (btn1)**: GPIO 32 - Main user input button
- **Debug Button (btn2)**: GPIO 34 - Debug error trigger activation
- **Push Button Triggers**: Replace DIP switches for full automation
  - **trigger_btn1 (Blue)**: GPIO 25 - Key Present
  - **trigger_btn2 (Yellow)**: GPIO 26 - Key Not Present
  - **trigger_btn3 (Orange)**: GPIO 27 - Lock Engaged
  - **trigger_btn4 (Purple)**: GPIO 33 - Lights On

#### Manual Debug Testing (Interactive)
- **ESP32 DevKit-C v4**: Main microcontroller
- **ILI9341 Display**: 240x240 simulation
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
# Build for integration testing (TEST_LOGS_ONLY)
pio run -e test-wokwi
ls .pio/build/test-wokwi/firmware.bin

# Build for manual debugging (verbose logs)  
pio run -e debug-local
ls .pio/build/debug-local/firmware.bin
```

### 2. Wokwi CLI Installation
Install the Wokwi CLI tool from one of these sources:

**Option 1: Direct Download (macOS)**
```bash
# Download and install to ~/bin/
curl -L https://github.com/wokwi/wokwi-cli/releases/latest/download/wokwi-cli-macos -o ~/bin/wokwi-cli
chmod +x ~/bin/wokwi-cli
```

**Option 2: Direct Download (Linux/WSL2)**
```bash
# Download and install to ~/bin/
curl -L https://github.com/wokwi/wokwi-cli/releases/latest/download/wokwi-cli-linux -o ~/bin/wokwi-cli
chmod +x ~/bin/wokwi-cli
```

**Option 3: NPM Installation**
```bash
npm install -g @wokwi/cli
```

**Option 4: Check existing installation**
```bash
# Test if already installed
wokwi-cli --version
```

## Running Tests

### Integration Testing (Automated)

#### Method 1: Cross-Platform Shell Script (Recommended)

The automated integration test with push button controls:

```bash
# Navigate to integration test directory
cd test/wokwi

# Run the fully automated integration test
./wokwi_test_automation.sh
```

**Features:**
- Cross-platform detection (WSL2, macOS, Linux)
- Automatically finds wokwi-cli in common locations
- Push button automation for 100% hands-off testing
- Shows ONLY `[T]` test logs (all other output suppressed)
- Comprehensive scenario testing matching scenarios.md

**Expected Output (TEST_LOGS_ONLY):**
```
========================================
 Clarity Wokwi Test Automation         
========================================

Platform: macos
Wokwi CLI: /Users/username/bin/wokwi-cli

Starting Clarity Automated Integration Test
Test Environment: Wokwi Emulator with Push Button Automation

[T] SplashPanel loaded successfully
[T] OemOilPanel loaded successfully
[T] LightsOnActivate() - Setting NIGHT theme
[T] LockPanel loaded successfully
[T] KeyPanel loaded successfully
[T] Key icon color set to RED (key not present)
[T] Key icon color set to GREEN (key present)
[T] ErrorPanel loaded successfully
← ONLY test timing messages shown, all other logs suppressed
```

### Manual Debug Testing (Interactive)

For interactive development and debugging with full verbose output:

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

## Test Phases and Expected Behavior

The integration test follows the complete scenario from scenarios.md covering all 20 steps:

### System Startup (Steps 1-2)
- **Duration**: ~15 seconds
- **Expected**: Splash panel animation → Oil panel with animated gauges
- **Key Serial Patterns**:
  ```
  [T] SplashPanel loaded successfully
  [T] OemOilPanel loaded successfully
  ```

### Theme and Panel System (Steps 3-8)
- **Duration**: ~20 seconds
- **Expected**: Night theme → Lock panel → Key panels → Priority restoration
- **Key Serial Patterns**:
  ```
  [T] LightsOnActivate() - Setting NIGHT theme
  [T] LockPanel loaded successfully
  [T] KeyPanel loaded successfully
  [T] Key icon color set to RED (key not present)
  [T] Key icon color set to GREEN (key present)
  ```

### Error System Integration (Steps 9-11)
- **Duration**: ~10 seconds
- **Expected**: Error panel → Navigate errors → Clear all errors → Restore
- **Key Serial Patterns**:
  ```
  [T] ErrorPanel loaded successfully
  [T] ShortPressActivate() - Executing short press action
  [T] LongPressActivate() - Executing long press action
  ```

### Complete System Restoration (Steps 12-14)
- **Duration**: ~8 seconds
- **Expected**: Return to Oil panel → Day theme → Config access
- **Key Serial Patterns**:
  ```
  [T] No blocking interrupts - restoring
  [T] OemOilPanel loaded successfully
  [T] LightsOffActivate() - Setting DAY theme
  ```

## Success Criteria

### ✅ Core Functionality
- [ ] All 6 panels display correctly (Splash, Oil, Key, Lock, Error, Config)
- [ ] All 5 interrupts function with correct priorities
- [ ] Theme switching works (Day/Night) 
- [ ] Button actions work (short vs long press)
- [ ] Panel loading logs appear correctly
- [ ] Icon color changes are logged

### ✅ Integration Points  
- [ ] Panel priorities: Key (CRITICAL) > Lock (IMPORTANT) > Oil (default)
- [ ] Trigger restoration logic works correctly
- [ ] Theme persists across panel switches
- [ ] Error system overrides all other panels
- [ ] Icon color changes without panel reloads
- [ ] All automated scenario steps execute in order

### ✅ Performance
- [ ] No memory leaks or crashes
- [ ] System remains responsive throughout test
- [ ] All test logs appear in correct sequence
- [ ] Consistent timing measurements

## log_t() Function Integration

Both testing environments support the custom `log_t()` function with different behaviors:

### TEST_LOGS_ONLY Mode (test-wokwi)
```cpp
#include "utilities/logging.h"  // Include the logging header

log_t("Panel loaded successfully");        // Shows: [T] Panel loaded successfully
log_t("Operation took %d ms", duration);   // Shows: [T] Operation took 150 ms

// All other log levels (log_e, log_w, log_i, log_d, log_v) are suppressed
log_i("This will not appear");             // Suppressed
log_d("This will not appear");             // Suppressed
```

### Full Debug Mode (debug-local)
```cpp
log_t("Panel loaded successfully");        // Shows: [T] Panel loaded successfully
log_i("General information");              // Shows: [I] General information
log_d("Debug details");                    // Shows: [D] Debug details
// All log levels work normally
```

### Expected Output Examples
**Integration Testing (TEST_LOGS_ONLY):**
```
[T] SplashPanel loaded successfully
[T] OemOilPanel loaded successfully
[T] KeyPanel loaded successfully
[T] Key icon color set to GREEN (key present)
[T] LockPanel loaded successfully
[T] ErrorPanel loaded successfully
```

**Manual Debug Testing (Full Logging):**
```
[V] Verbose debug messages
[D] Debug information
[I] General information
[W] Warnings
[E] Errors
[T] Timing/performance messages
```

### Benefits
- ⏱️ **Performance measurement** - Easy timing of critical operations
- 🔍 **Optimization tracking** - Before/after performance comparisons  
- 🏷️ **Filtered logging** - Easy to grep/filter for timing data
- 📊 **Clean CI output** - Only essential test logs in automated testing
- 🧪 **Test assertions** - Perfect for automated test validation

## Troubleshooting

### Build Issues

**Problem**: Firmware build fails
```bash
# Clean and rebuild from project root
cd /path/to/Clarity
pio run -e debug-local --target clean
pio run -e debug-local

# Check for missing dependencies
pio lib list
```

**Problem**: Wrong build environment
```bash
# For integration testing (TEST_LOGS_ONLY)
pio run -e test-wokwi

# For manual debugging (verbose logs)
pio run -e debug-local

# Available testing environments:
# - test-wokwi: Ultra-clean integration testing (TEST_LOGS_ONLY)
# - debug-local: Full debug testing (CORE_DEBUG_LEVEL=5)
```

**Problem**: PlatformIO command not found from test directory
```bash
# Always run from project root
cd /path/to/Clarity  # NOT from test/wokwi or test/manual
pio run -e test-wokwi
```

### Wokwi CLI Issues

**Problem**: wokwi-cli not found
```bash
# Check if installed
which wokwi-cli

# Install for macOS
curl -L https://github.com/wokwi/wokwi-cli/releases/latest/download/wokwi-cli-macos -o ~/bin/wokwi-cli
chmod +x ~/bin/wokwi-cli

# Install for Linux/WSL2
curl -L https://github.com/wokwi/wokwi-cli/releases/latest/download/wokwi-cli-linux -o ~/bin/wokwi-cli
chmod +x ~/bin/wokwi-cli

# Add to PATH if needed
echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

**Problem**: Cross-platform path issues
The automation script automatically detects platform and finds wokwi-cli in common locations:
- macOS: `/Users/username/bin/wokwi-cli`, `/usr/local/bin/wokwi-cli`
- WSL2: `/home/username/bin/wokwi-cli`, `/usr/local/bin/wokwi-cli`
- Linux: `/home/username/bin/wokwi-cli`, `/usr/local/bin/wokwi-cli`

### Test Execution Issues

**Problem**: Test hangs waiting for interrupt callbacks
This is expected behavior in Wokwi simulation - GPIO changes are polled rather than generating actual interrupts. The TEST_LOGS_ONLY feature is still working correctly.

**Problem**: Only seeing [T] logs in integration test
This is the intended behavior! TEST_LOGS_ONLY mode suppresses all other log levels for clean CI/CD output.

**Problem**: Missing test phases  
- Check that all push buttons start in released position
- Verify automation scenario matches scenarios.md order
- Monitor for timeout issues

## Development Workflow

### For Feature Development
1. **Code Changes** → Build with `pio run -e debug-local` (from project root)
2. **Manual Testing** → Use `cd test/manual && ./wokwi_debug.sh`
3. **Interactive Debugging** → Full verbose output + manual hardware control
4. **Performance Measurement** → Add `log_t()` calls for timing critical sections
5. **Fix Issues** → Repeat until stable

### For CI/CD Integration  
1. **Code Complete** → Build with `pio run -e test-wokwi` (from project root)
2. **Integration Testing** → Use `cd test/wokwi && ./wokwi_test_automation.sh`
3. **Automated Validation** → Ultra-clean output + automated pass/fail + `log_t()` timing
4. **Deploy** → If all tests pass

### Quick Commands Reference
```bash
# Manual Debug Testing (Full verbose output)
cd /path/to/Clarity
pio run -e debug-local
cd test/manual
./wokwi_debug.sh

# Integration Testing (TEST_LOGS_ONLY output)  
cd /path/to/Clarity
pio run -e test-wokwi
cd test/wokwi
./wokwi_test_automation.sh
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
      - name: Build Test Environment
        run: pio run -e test-wokwi
      - name: Run Integration Test
        run: |
          cd test/wokwi
          timeout 180 ./wokwi_test_automation.sh
```

This comprehensive testing framework provides both detailed debugging capabilities for development and ultra-clean automated testing for CI/CD, ensuring all functionality remains intact while providing clear, actionable test results.