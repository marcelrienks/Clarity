# Clarity Wokwi Test Suite

## Quick Start

### Using the Test Script (Recommended)

**Basic Test** (5 phases, ~5 seconds):
```bash
./test/wokwi/run_test.sh basic
```

**Full Test Suite** (comprehensive testing, ~2 minutes):
```bash
./test/wokwi/run_test.sh full
```

### Manual Commands

**Basic Test**:
```bash
pio run -e test-wokwi-basic && cd test/wokwi && cp wokwi-basic.toml wokwi.toml && wokwi-cli --timeout 10000
```

**Full Test Suite**:
```bash
pio run -e test-wokwi-full && cd test/wokwi && cp wokwi-full.toml wokwi.toml && wokwi-cli --timeout 120000
```

Note: Run all commands from the project root directory.

## When to Use Each Test

### Basic Test (`basic`)
**Use when:**
- Quick hardware validation during development
- CI/CD pipeline for pull request validation
- Verifying GPIO pin functionality
- Testing button/sensor simulation
- Fast feedback loop during coding

**What it tests:**
- GPIO pin initialization
- Button press simulation (all 4 buttons)
- Analog sensor simulation (pressure/temperature)
- Timing validation (rapid button presses)
- Long press detection accuracy

**Duration:** ~5 seconds
**Phases:** 5

### Full Test Suite (`full`)
**Use when:**
- Pre-release system validation
- Complete integration testing
- Manual testing before deployment
- Validating major feature changes
- End-to-end system verification

**What it tests:**
- Complete 7-phase system integration
- Panel lifecycle management (Splash, Oil, Key, Lock, Error, Config)
- Trigger system with priorities (CRITICAL > IMPORTANT > NORMAL)
- Theme switching (Day/Night modes)
- Animation system validation
- Error handling and recovery
- Configuration management
- User interaction flows
- Sensor data processing

**Duration:** ~2 minutes  
**Phases:** 7

## Command Usage Guide

### For Development
```bash
# Quick validation while coding
./test/wokwi/run_test.sh basic

# Build only (no simulation)
pio run -e test-wokwi-basic
```

### For Integration Testing
```bash
# Complete system validation
./test/wokwi/run_test.sh full

# Manual testing with extended timeout
pio run -e test-wokwi-full && cd test/wokwi && cp wokwi-full.toml wokwi.toml && wokwi-cli --timeout 180000
```

### For CI/CD Pipelines
```bash
# GitHub Actions / automated testing
./test/wokwi/run_test.sh basic

# Release validation (if time permits)
./test/wokwi/run_test.sh full
```

## GitHub Actions Integration

The project includes automated testing via GitHub Actions:

### **Automated Test Triggers:**

**Basic Test** (runs on every commit):
- ✅ Push to `main`, `develop`, feature branches
- ✅ ~30 seconds execution time
- ✅ Quick hardware validation feedback

**Full Test** (runs on pull requests):
- ✅ Pull requests to `main`/`develop` 
- ✅ ~3 minutes execution time  
- ✅ Complete system integration validation
- ✅ Automatic PR comments with results

**Release Tests** (runs on releases):
- ✅ Push to `release/*` branches
- ✅ Version tags (`v*.*.*`)
- ✅ Both basic and full tests
- ✅ Pre-release validation

### **Setup Requirements:**
1. **Wokwi Token**: Add `WOKWI_CLI_TOKEN` secret to GitHub repository
   - Get token from: https://wokwi.com/dashboard/ci
   - Repository Settings → Secrets and variables → Actions
2. **Branch Protection**: Require status checks for merge protection

### **Workflow Files:**
- `.github/workflows/wokwi-basic-test.yml` - Basic tests
- `.github/workflows/wokwi-full-test.yml` - Full tests  
- `.github/workflows/wokwi-release-test.yml` - Release validation
- `.github/workflows/README.md` - Detailed workflow setup guide

### For Debugging
```bash
# Build with verbose output
pio run -e test-wokwi-full -v

# Run simulation with screenshot capture
pio run -e test-wokwi-full && cd test/wokwi && cp wokwi-full.toml wokwi.toml && wokwi-cli --screenshot-part lcd1 --screenshot-time 5000 --timeout 10000
```

## Prerequisites

1. **Wokwi Token**: Must be set as environment variable
   - Get token from: https://wokwi.com/dashboard/ci
   - Should already be in your `~/.zshrc` file
   - Open new terminal or run: `source ~/.zshrc`

2. **PlatformIO**: Must be installed and configured

## Test Coverage & Hardware Configuration

### Hardware Components (Wokwi Simulation)
- **ESP32 DevKit**: Main microcontroller
- **ILI9341 Display**: Simulated 240x240 round display  
- **5 Push Buttons**: Action, Key, Lock, Lights, Error triggers
- **2 Potentiometers**: Oil pressure and temperature simulation
- **DIP Switch**: Additional GPIO trigger simulation

### GPIO Pin Mapping
```cpp
// Button inputs
#define BTN_ACTION    32  // Main action button (short/long press)
#define BTN_KEY       25  // Key present/not present simulation
#define BTN_LOCK      26  // Lock trigger simulation
#define BTN_LIGHTS    27  // Lights trigger (day/night theme)
#define BTN_ERROR     34  // Debug error trigger

// Analog inputs
#define POT_PRESSURE  36  // Oil pressure sensor simulation (VP)
#define POT_TEMP      39  // Oil temperature sensor simulation (VN)
```

## What the Tests Do

### Basic Test (5 phases, ~5 seconds):

1. **Hardware Initialization** - GPIO pin setup and pin state verification
2. **Button Simulation** - Tests all 4 buttons (action, key, lock, lights)
3. **Analog Sensor Simulation** - Tests pressure/temperature potentiometers with ADC validation
4. **Timing Validation** - Rapid button press performance (<2000ms for 5 presses)
5. **Long Press Validation** - Button press duration accuracy (±100ms tolerance)

### Full Test (7 phases, ~2 minutes):

1. **System Startup & Initial State** (30s) - System initialization, splash screen, oil panel loading
2. **Sensor Data & Animations** (45s) - Pressure/temperature animations, dynamic sensor changes  
3. **Trigger System Testing** (90s) - All trigger priorities, panel switching, theme changes
4. **Error Handling System** (60s) - Critical error trigger, navigation, panel restoration
5. **Trigger Deactivation & Theme Changes** (45s) - Lock deactivation, theme switching
6. **Configuration System Testing** (120s) - Config navigation, theme settings, persistence
7. **Final System Validation** (30s) - Dual sensor animations, system responsiveness

### Test Coverage Summary:
- ✅ **All 6 Panels**: Splash → Oil → Key → Lock → Error → Config
- ✅ **All Trigger Types**: Lights, Lock, Key Present/Not Present, Debug Error
- ✅ **Priority System**: CRITICAL > IMPORTANT > NORMAL trigger handling  
- ✅ **Theme System**: Day/Night theme switching and persistence
- ✅ **Animation System**: Pressure/temperature gauge animations
- ✅ **Button System**: Short press vs long press detection
- ✅ **Configuration System**: Complete navigation and settings management
- ✅ **Error Handling**: CRITICAL priority error system with recovery
- ✅ **State Management**: Panel restoration and trigger deactivation

## Expected Output

```
================================================
CLARITY WOKWI HARDWARE SIMULATION TEST
================================================
Test Duration: ~30 seconds
Total Phases: 5
================================================

✅ PASSED: GPIO pins initialized successfully
✅ PASSED: Action button press simulation completed
✅ PASSED: Key button press simulation completed
✅ PASSED: Lock button press simulation completed
✅ PASSED: Lights button press simulation completed
✅ PASSED: Pressure sensor value set successfully
✅ PASSED: Temperature sensor value set successfully
✅ PASSED: Rapid button simulation completed within timing requirements
✅ PASSED: Long press duration accuracy validated

================================================
TEST SUMMARY
================================================
Total Duration: 5282 ms (5.3 seconds)
Phases Completed: 5/5
Test Result: PASSED ✅
================================================
```

## Alternative Commands

Using the wrapper script:
```bash
./test/wokwi/wokwi_run.sh
```

Build only (no simulation):
```bash
~/.platformio/penv/bin/pio run -e test-wokwi
```

## Troubleshooting

### Common Issues

**Token Error**: If you get "Missing WOKWI_CLI_TOKEN", run:
```bash
source ~/.zshrc
```

**Build Errors**: Clean and rebuild:
```bash
pio run -e test-wokwi-basic -t clean && pio run -e test-wokwi-basic
pio run -e test-wokwi-full -t clean && pio run -e test-wokwi-full
```

**Test Timeouts**: 
- Basic test should complete in ~5 seconds (10s timeout)
- Full test should complete in ~90 seconds (120s timeout)
- Check for infinite loops or blocking operations in test code

**GPIO/Hardware Issues**:
- Verify Wokwi diagram.json pin mappings match code definitions
- Check button bounce settings in diagram
- Ensure proper pull-down resistor configuration

**Animation Issues**:
- Verify LVGL buffer configuration for Wokwi
- Check display refresh rate settings  
- Monitor for animation completion callbacks

**Serial Output Issues**:
- Ensure baud rate matches (115200)
- Check for proper Serial.begin() initialization
- Monitor for expected test result messages

### Debug Techniques

**Extract Test Results**:
```bash
# Filter test output for key information
grep -E "(PHASE|✅|❌|TEST SUMMARY)" test_output.log

# Check for specific system messages  
grep -E "(loaded successfully|trigger activated|Theme changed)" test_output.log
```

**Performance Monitoring**:
- Panel transitions should occur in <500ms
- Gauge animations should complete in 750ms ±50ms
- Theme changes should occur in <200ms
- Memory usage should remain stable throughout test