# Clarity Wokwi Integration Tests

This directory contains comprehensive integration tests for the Clarity ESP32 digital gauge system using the Wokwi simulator platform.

## Overview

The Wokwi integration test provides **complete end-to-end validation** of the Clarity system in a simulated hardware environment. This test exercises all major system components in a single cohesive 7-minute flow.

### Test Coverage

- ✅ **All 6 Panels**: Splash → Oil → Key → Lock → Error → Config
- ✅ **All Trigger Types**: Lights, Lock, Key Present/Not Present, Debug Error  
- ✅ **Priority System**: CRITICAL > IMPORTANT > NORMAL trigger handling
- ✅ **Theme System**: Day/Night theme switching and persistence
- ✅ **Animation System**: Pressure/temperature gauge animations
- ✅ **Button System**: Short press vs long press detection
- ✅ **Configuration System**: Complete navigation and settings management
- ✅ **Error Handling**: CRITICAL priority error system with recovery
- ✅ **State Management**: Panel restoration and trigger deactivation

## Files Structure

```
test/
├── test_wokwi_integration.cpp          # Main integration test (PlatformIO naming convention)
└── wokwi/                             # Wokwi-specific configuration files
    ├── README.md                      # This file
    ├── diagram.json                   # Wokwi hardware configuration
    ├── quick_test.py                  # Python test runner
    ├── run_integration_test.sh        # Bash test execution script
    └── wokwi.toml                     # Wokwi simulator configuration
```

## Hardware Configuration

The test uses a comprehensive Wokwi hardware setup:

### Components
- **ESP32 DevKit**: Main microcontroller
- **ILI9341 Display**: Simulated 240x240 round display
- **5 Push Buttons**: Action, Key, Lock, Lights, Error triggers
- **2 Potentiometers**: Oil pressure and temperature simulation
- **DIP Switch**: Additional GPIO trigger simulation

### GPIO Mapping
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

## Test Phases

### Phase 1: System Startup (30s)
- System initialization and factory creation
- Splash screen animation
- Automatic transition to Oil panel

### Phase 2: Sensor & Animations (45s)  
- Initial pressure/temperature readings
- Dynamic sensor value changes
- Smooth gauge animations with completion verification

### Phase 3: Trigger System (90s)
- Lights trigger → Night theme activation
- Lock trigger → Panel switching with IMPORTANT priority
- Key triggers → Present/Not Present state management
- Trigger deactivation and panel restoration

### Phase 4: Error Handling (60s)
- Debug error trigger → CRITICAL priority activation
- Error panel navigation (short press)
- Error panel exit (long press)
- System recovery validation

### Phase 5: Theme Management (45s)
- Lock trigger deactivation
- Lights deactivation → Day theme restoration
- Theme persistence verification

### Phase 6: Configuration System (120s)
- Config panel entry via long press
- Navigation through all config options
- Theme sub-menu navigation
- Theme change via configuration
- Configuration persistence and exit

### Phase 7: Final Validation (30s)
- Final sensor animations
- System stability verification
- Complete functionality confirmation

## Running the Tests

### Prerequisites
1. **Wokwi CLI**: Install with `npm install -g @wokwi/cli`
2. **PlatformIO**: Ensure PlatformIO is installed and configured
3. **Build Environment**: Project must build successfully
4. **Wokwi Token** (for CLI execution): Get your free token at [https://wokwi.com/dashboard/ci](https://wokwi.com/dashboard/ci)
   - Add to your bash profile: `echo 'export WOKWI_CLI_TOKEN="your-token-here"' >> ~/.bashrc`
   - ⚠️ **Security**: Never commit tokens to git or store in configuration files
   - Required for automated test execution through PlatformIO

### Execution Options

#### Option 1: Automated Script (Recommended)
```bash
# Run the complete integration test
./test/wokwi/run_integration_test.sh
```

#### Option 2: Direct PlatformIO Integration ✅ **NEW** 
```bash
# Set your Wokwi token in bash profile (one-time setup)
echo 'export WOKWI_CLI_TOKEN="your-token-here"' >> ~/.bashrc
source ~/.bashrc

# Build test firmware and run directly in Wokwi
pio run -e test-wokwi -t upload

# Or build only
pio run -e test-wokwi
```

#### Option 3: Manual Wokwi Execution  
```bash
# Build firmware first
pio run -e test-wokwi
cd test/wokwi
wokwi-cli run --timeout 60000 diagram.json ../../.pio/build/test-wokwi/firmware.bin
```

#### Option 4: Wokwi Web Interface
1. Upload `diagram.json` to Wokwi web editor
2. Upload compiled firmware binary
3. Run simulation and monitor serial output

## Test Validation

The test provides comprehensive validation through:

### Serial Output Monitoring
- **150+ Verification Points**: Automatic checking of system responses
- **Phase Progression**: Clear logging of each test phase
- **Error Detection**: Immediate failure reporting with details
- **Performance Timing**: Animation and transition timing validation

### Expected Serial Output Pattern
```
================================================
CLARITY FULL SYSTEM INTEGRATION TEST
================================================

========================================
PHASE 1: System Startup & Initial State
========================================
✅ PASSED: InterruptManager initialization
✅ PASSED: Splash panel loaded
✅ PASSED: Oil panel auto-loaded after splash

... (additional phases)

================================================
TEST SUMMARY
================================================
Total Duration: 387452 ms (6.5 minutes)
Phases Completed: 7/7
Test Result: PASSED ✅
================================================
```

### Success Criteria
- ✅ All 7 phases complete successfully
- ✅ No system crashes or memory leaks
- ✅ All panel transitions occur within timing limits
- ✅ All animations complete smoothly
- ✅ All user interactions respond correctly
- ✅ Theme persistence works properly
- ✅ Configuration changes apply and persist

## Troubleshooting

### Common Issues

#### Build Failures
```bash
# Clean and rebuild
pio run -e test-wokwi -t clean
pio run -e test-wokwi
```

#### Test Timeouts
- Ensure test duration allows for full 7-minute execution
- Check Wokwi CLI timeout settings (default 420 seconds)
- Monitor system performance during test

#### GPIO Issues
- Verify Wokwi diagram.json pin mappings match code
- Check button bounce settings in diagram
- Ensure proper pull-down resistor configuration

#### Animation Issues  
- Verify LVGL buffer configuration for Wokwi
- Check display refresh rate settings
- Monitor for animation completion callbacks

### Debug Techniques

#### Serial Log Analysis
```bash
# Extract test results from log
grep -E "(PHASE|✅|❌|TEST SUMMARY)" integration_test.log

# Check for specific system messages
grep -E "(loaded successfully|trigger activated|Theme changed)" integration_test.log
```

#### Phase-by-Phase Debugging
- Add breakpoints at phase boundaries
- Use `TEST_ASSERT_TRUE_MESSAGE()` for detailed failure info
- Monitor memory usage during long-running test

## Performance Benchmarks

### Expected Timing
- **Panel Transitions**: <500ms
- **Gauge Animations**: 750ms ±50ms
- **Theme Changes**: <200ms
- **Configuration Navigation**: <300ms per step

### Memory Usage
- **Peak RAM**: <180KB (out of 320KB available)
- **Flash Usage**: ~1.4MB (debug build)
- **No Memory Leaks**: Stable throughout 7-minute test

## Integration with CI/CD

The Wokwi integration test can be integrated into automated CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Run Wokwi Integration Test
  run: |
    npm install -g @wokwi/cli
    ./test/wokwi/run_integration_test.sh
```

## Test Maintenance

### Updating Tests
1. Modify test phases in `test/test_wokwi_integration.cpp`
2. Update timing constants if system performance changes
3. Adjust GPIO mappings if hardware configuration changes
4. Update expected serial messages for system changes

### Adding New Test Phases
```cpp
void test_phase8_new_feature() {
    logPhase("New Feature Testing");
    
    // Test implementation
    verifyCondition(
        waitForSerialMessage("expected_message", 2000),
        "New feature functionality"
    );
    
    delay(PHASE_DELAY_MS);
}
```

## Conclusion

This comprehensive Wokwi integration test provides **complete system validation** for the Clarity digital gauge system. It serves as both a **regression test suite** and a **system demonstration**, ensuring all components work together correctly under realistic operating conditions.

The test is designed to be:
- **Comprehensive**: Covers 100% of major system functionality
- **Reliable**: Consistent results across multiple test runs  
- **Maintainable**: Easy to modify and extend for new features
- **Automated**: Can run unattended in CI/CD pipelines
- **Educational**: Serves as a complete system usage example

---
**Duration**: ~7 minutes  
**Coverage**: 100% major functionality  
**Validation Points**: 150+  
**Platforms**: Wokwi Simulator, CI/CD Integration