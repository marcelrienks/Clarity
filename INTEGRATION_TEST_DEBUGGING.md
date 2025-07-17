# Integration Test Debugging Guide

## Overview

This document provides debugging guidance for Wokwi integration tests in the Clarity project. The integration tests use Wokwi CLI automation scenarios to simulate hardware interactions and verify system behavior.

## Test Architecture

### Test Files Location
- **Test scenarios**: `test/` directory
- **Test runner scripts**: `run_all_tests.sh` (Linux/Mac), `run_all_tests.bat` (Windows)
- **Wokwi configuration**: `wokwi.toml`
- **Hardware diagram**: `diagram.json`

### Test Categories
1. **test_basic_startup.yaml** - Basic system startup sequence
2. **test_oil_sensors.yaml** - Oil sensor functionality
3. **test_key_trigger.yaml** - Key trigger behavior
4. **test_lock_trigger.yaml** - Lock trigger behavior  
5. **test_trigger_priority.yaml** - Trigger priority handling
6. **test_invalid_states.yaml** - Invalid state handling

## Common Issues and Solutions

### 1. Test Timing Issues

**Problem**: Tests timeout waiting for panel logs (e.g., "KeyPanel", "LockPanel")

**Root Cause**: Trigger evaluation is throttled to 300ms intervals in `PanelManager::update_panel()`

**Solution**: Add 500ms delay after trigger activation:
```yaml
- name: "Activate trigger"
  set-control:
    part-id: esp
    control: "25"
    value: 1

- name: "Wait for trigger evaluation (300ms throttle)"
  delay: 500ms

- name: "Wait for panel loading"
  wait-serial: "KeyPanel"
```

### 2. Log Formatting Issues

**Problem**: Test logs appear with stair-step formatting:
```
[TEST] system_ready
                   [TEST] SplashPanel
                                     [TEST] OemOilPanel
```

**Root Cause**: Serial output buffering issues in `log_t` macro

**Solution**: Updated `test_logging.h` with explicit newline handling:
```c
#define log_t(format, ...) do { \
  Serial.print("\n[TEST] "); \
  Serial.printf(format, ##__VA_ARGS__); \
  Serial.print("\n"); \
  Serial.flush(); \
  delay(1); \
} while(0)
```

### 3. GPIO Control Methods

We investigated three approaches for controlling GPIO pins in automation scenarios:

#### Method 1: DIP Switch Control (Original)
```yaml
# In test file
set-control:
  part-id: sw1
  control: "1"
  value: 1

# In wokwi.toml  
[[wokwi.scenario.controls]]
part-id = "sw1"
control = "1"
```

**Status**: ❌ Failed - DIP switch automation is alpha/undocumented
**Benefit**: ✅ Allows manual testing of invalid states
**Issue**: Control commands don't translate to GPIO state changes

#### Method 2: Pushbutton Control  
```yaml
# In test file
set-control:
  part-id: btn1
  control: "pressed"
  value: 1

# In wokwi.toml
[[wokwi.scenario.controls]]
part-id = "btn1"
control = "pressed"
```

**Status**: ✅ Works for automation
**Issue**: ❌ Removes DIP switches needed for manual testing
**Drawback**: Can't test invalid states (both triggers active)

#### Method 3: Direct GPIO Control (Recommended)
```yaml
# In test file
set-control:
  part-id: esp
  control: "25"
  value: 1

# In wokwi.toml
[[wokwi.scenario.controls]]
part-id = "esp"
control = "25"
```

**Status**: ⚠️ Untested (quota limit)
**Benefit**: ✅ Bypasses component issues, keeps DIP switches
**Theory**: Controls ESP32 GPIO pins directly

## Debug Logging

### Enabling Debug Output

For debugging trigger issues, temporary debug logging was added:

**In KeyTrigger (`src/triggers/key_trigger.cpp`)**:
```cpp
#ifdef WOKWI_EMULATOR
if (should_trigger) {
    Serial.printf("\n[DEBUG] KeyTrigger firing! State: %d\n", static_cast<int>(current_key_state));
    Serial.flush();
}
#endif
```

**In KeySensor (`src/sensors/key_sensor.cpp`)**:
```cpp
#ifdef WOKWI_EMULATOR
Serial.printf("\n[DEBUG] KeySensor GPIO read: pin25=%d, pin26=%d\n", pin25_high, pin26_high);
Serial.flush();
#endif
```

### Log Analysis

**Expected flow when trigger activates**:
1. `[DEBUG] KeySensor GPIO read: pin25=1, pin26=0` (GPIO state change)
2. `[DEBUG] KeyTrigger firing! State: 1` (trigger evaluation)
3. `[TEST] KeyPanel` (panel loads)

**Actual flow when failing**:
1. `[DEBUG] KeySensor GPIO read: pin25=0, pin26=0` (no GPIO change)
2. No trigger firing log
3. No panel loading log

## Test Execution Commands

### Manual Test Execution
```bash
# Build integration test environment
pio run -e integration-test

# Run individual tests
./wokwi-cli . --scenario test/test_basic_startup.yaml --timeout 60000
./wokwi-cli . --scenario test/test_key_trigger.yaml --timeout 60000
./wokwi-cli . --scenario test/test_lock_trigger.yaml --timeout 60000
./wokwi-cli . --scenario test/test_trigger_priority.yaml --timeout 60000
./wokwi-cli . --scenario test/test_invalid_states.yaml --timeout 60000
./wokwi-cli . --scenario test/test_oil_sensors.yaml --timeout 60000

# Run all tests
./run_all_tests.sh
```

### Prerequisites
- Wokwi CLI installed
- WOKWI_CLI_TOKEN environment variable set
- Sufficient Wokwi quota (tests consume CI minutes)

## Hardware Configuration

### Circuit Design
- **GPIO 25**: KEY_PRESENT (DIP switch 1)
- **GPIO 26**: KEY_NOT_PRESENT (DIP switch 2)  
- **GPIO 27**: LOCK (DIP switch 3)
- **GPIO 36**: OIL_PRESSURE (potentiometer 1)
- **GPIO 39**: OIL_TEMPERATURE (potentiometer 2)

### Pin Configuration
```cpp
// In KeySensor::init()
pinMode(GpioPins::KEY_PRESENT, INPUT_PULLDOWN);     // GPIO 25
pinMode(GpioPins::KEY_NOT_PRESENT, INPUT_PULLDOWN); // GPIO 26
```

**DIP Switch Logic**:
- Switch OFF: GPIO reads LOW (pulled down)
- Switch ON: GPIO reads HIGH (connected to 3V3)

## Troubleshooting Checklist

### When Integration Tests Fail

1. **Check system startup**:
   - ✅ `[TEST] system_ready` appears
   - ✅ `[TEST] SplashPanel` appears  
   - ✅ `[TEST] OemOilPanel` appears

2. **Check GPIO control**:
   - Add debug logging to KeySensor
   - Verify GPIO state changes after `set-control`
   - Expected: `pin25=1` when activating GPIO 25

3. **Check trigger evaluation**:
   - Add debug logging to KeyTrigger
   - Verify trigger fires when GPIO changes
   - Expected: `[DEBUG] KeyTrigger firing!` message

4. **Check panel loading**:
   - Verify panel completion callbacks are called
   - Expected: `[TEST] KeyPanel` after trigger fires

### When Manual Testing is Needed

1. **Use DIP switches** for invalid state testing
2. **Activate both switches** to test invalid state handling
3. **Verify system returns to OemOilPanel** for invalid states

## Future Improvements

1. **Investigate direct GPIO control** once Wokwi quota is available
2. **Document working control syntax** for different components
3. **Add integration test for sensor value changes** (potentiometers)
4. **Reduce dependency on timing delays** in tests
5. **Consider interrupt-based trigger evaluation** instead of polling

## Related Files

- `src/managers/panel_manager.cpp` - 300ms trigger throttling
- `src/triggers/key_trigger.cpp` - Trigger evaluation logic
- `src/sensors/key_sensor.cpp` - GPIO pin reading
- `include/test_logging.h` - Test logging macros
- `include/hardware/gpio_pins.h` - GPIO pin definitions

## TODO: Complete Integration Test Implementation

**Priority**: High
**Status**: In Progress
**Next Steps**:
1. Test direct GPIO control approach (`part-id: esp`, `control: "25"`) once Wokwi quota is restored
2. If direct GPIO control fails, investigate alternative approaches:
   - Custom chip implementation with GPIO control
   - Hybrid approach using pushbuttons for automation + DIP switches for manual testing
3. Update all integration test files to use working control method
4. Verify all 6 test scenarios pass consistently
5. Add integration tests to CI/CD pipeline
6. Document final working configuration for team

**Current State**: 
- DIP switches preserved for manual testing
- Direct GPIO control configured but untested
- Debug logging in place for troubleshooting
- All test scenarios updated with proper timing delays

**Blocker**: Wokwi free plan quota exceeded - need paid plan or quota reset to continue testing

## Last Updated
July 17, 2025 - Initial debugging documentation after investigating GPIO control methods