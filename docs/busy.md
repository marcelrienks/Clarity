# Next Steps: Comprehensive Test Suite Implementation

## Current Status ✅

**Basic Test Framework**: Successfully implemented and running
- ✅ 12 basic tests passing in all environments (coverage, integration, performance, memory)
- ✅ Unity test framework properly integrated with LVGL mocks
- ✅ Mock service architecture functional
- ✅ Path resolution and type conflicts resolved

## Remaining Work: 108 Comprehensive Tests

**Target**: Get all 108 comprehensive tests in `test/unit/` compiling and running

### Test File Inventory (15 files, 108 tests total)

**Manager Tests** (4 files, ~38 tests):
- `test/unit/managers/test_preference_manager.cpp` - 14 tests ⚠️ **Interface mismatches**
- `test/unit/managers/test_panel_manager.cpp` - 8 tests ⚠️ **Constructor/interface conflicts** 
- `test/unit/managers/test_style_manager.cpp` - 9 tests ⚠️ **Missing methods**
- `test/unit/managers/test_trigger_manager.cpp` - 7 tests ⚠️ **Hardware dependencies**

**Sensor Tests** (5 files, ~47 tests):
- `test/unit/sensors/test_key_sensor.cpp` - 16 tests ✅ **Fixed Reading types**
- `test/unit/sensors/test_lock_sensor.cpp` - 10 tests ✅ **Fixed Reading types**
- `test/unit/sensors/test_light_sensor.cpp` - 7 tests ✅ **Fixed Reading types**
- `test/unit/sensors/test_oil_pressure_sensor.cpp` - 9 tests ✅ **Fixed Reading types**
- `test/unit/sensors/test_oil_temperature_sensor.cpp` - 5 tests ✅ **Fixed Reading types**

**System/Utility Tests** (6 files, ~23 tests):
- `test/unit/providers/test_gpio_provider.cpp` - 8 tests ⚠️ **Hardware mocking needed**
- `test/unit/system/test_service_container.cpp` - 7 tests ⚠️ **DI framework conflicts**
- `test/unit/utilities/test_ticker.cpp` - 6 tests ⚠️ **Timing dependencies**
- `test/unit/utilities/test_simple_ticker.cpp` - 4 tests ⚠️ **Timing dependencies**
- `test/unit/sensors/test_sensor_logic.cpp` - 6 tests ⚠️ **Logic validation**
- `test/unit/managers/test_config_logic.cpp` - 8 tests ⚠️ **Config validation**

## Implementation Plan

### Phase 1: Manager Interface Updates (Priority: HIGH)

**1. PreferenceManager Constructor Fix**
```cpp
// Current issue: Tests expect constructor with ServiceContainer
// Reality: PreferenceManager() takes no parameters
// Fix: Update test setup to use default constructor + dependency injection
```

**2. PanelManager Interface Alignment**
```cpp  
// Current issue: Tests use addPanel(), hasPanel(), loadPanel()
// Reality: Uses createAndLoadPanel(), getCurrentPanel(), updatePanel()
// Fix: Update test methods to match IPanelService interface
```

**3. StyleManager Method Updates**
```cpp
// Current issue: Tests use toggleTheme(), getCurrentTheme()
// Reality: Uses set_theme(), get_colours()
// Fix: Update test methods to match actual API
```

### Phase 2: Hardware Abstraction Layer (Priority: MEDIUM)

**1. GPIO Provider Mocking**
- Complete MockGpioProvider with all pin operations
- Add interrupt simulation capabilities
- Handle ADC reading simulation

**2. Sensor Hardware Dependencies**
```cpp
// Current issues:
// - Missing adc_attenuation_t type definitions
// - ESP32 logging functions (log_v, log_d, log_i) not available
// - Hardware-specific initialization calls

// Solutions:
// - Add complete ESP32 mock definitions in test/mocks/esp32-hal-log.h
// - Mock ADC configuration functions  
// - Implement sensor value simulation framework
```

### Phase 3: Service Container Integration (Priority: MEDIUM)

**1. Dependency Injection Framework**
- Update ServiceContainer tests to handle current DI patterns
- Fix service registration and retrieval in test fixtures
- Ensure proper service lifecycle management

**2. Mock Service Improvements**
- Complete MockTriggerService implementation
- Add missing ITriggerService methods
- Implement trigger state simulation

### Phase 4: Timing and Logic Tests (Priority: LOW)

**1. Ticker/Timer Abstraction**
- Abstract time dependencies for deterministic testing
- Implement MockTimeProvider for consistent timing tests
- Handle async callback simulation

**2. Configuration Logic Validation**
- Implement comprehensive config validation tests
- Add edge case handling for malformed configurations
- Test persistence and recovery scenarios

## Technical Implementation Steps

### Step 1: Enable Comprehensive Tests (IMMEDIATE)

```ini
# In platformio.ini [env:test-coverage]
build_src_filter = 
	+<*>
	+<../test/test_all.cpp>
	+<../test/unit>          # Enable comprehensive tests
	+<../test/mocks>
	+<../test/utilities>
	-<../src/components>     # Exclude hardware-dependent components
	-<../src/providers>      # Exclude hardware providers  
	-<../src/factories>      # Exclude factory classes
	-<../test/wokwi>
	-<../test/integration>
```

### Step 2: Systematic Interface Fixes

```cpp
// Add to test_all.cpp (uncomment when ready)
// Comprehensive Test Suites (108 tests total)
runPreferenceManagerTests();    // 14 tests - FIX CONSTRUCTOR
runTriggerManagerTests();       // 8 tests - FIX HARDWARE DEPS  
runPanelManagerTests();         // 7 tests - FIX INTERFACE
runStyleManagerTests();         // 9 tests - FIX METHODS
runKeySensorTests();           // 16 tests - READY ✅
runLockSensorTests();          // 10 tests - READY ✅  
runLightSensorTests();         // 7 tests - READY ✅
runOilPressureSensorTests();   // 9 tests - READY ✅
runOilTemperatureSensorTests(); // 9 tests - READY ✅
runGpioProviderTests();        // 8 tests - FIX HARDWARE MOCKING
runServiceContainerTests();    // 7 tests - FIX DI FRAMEWORK
runTickerTests();              // 6 tests - FIX TIMING DEPS
runSimpleTickerTests();        // 4 tests - FIX TIMING DEPS  
runSensorLogicTests();         // 6 tests - FIX LOGIC VALIDATION
runConfigLogicTests();         // 8 tests - FIX CONFIG VALIDATION
```

### Step 3: Missing Dependencies

**Add complete ESP32 mocking:**
```cpp
// test/mocks/esp32-hal-log.h - MISSING
#define log_v(format, ...) printf("[V] " format "\n", ##__VA_ARGS__)
#define log_d(format, ...) printf("[D] " format "\n", ##__VA_ARGS__)  
#define log_i(format, ...) printf("[I] " format "\n", ##__VA_ARGS__)
#define log_w(format, ...) printf("[W] " format "\n", ##__VA_ARGS__)
#define log_e(format, ...) printf("[E] " format "\n", ##__VA_ARGS__)

typedef enum {
    ADC_ATTEN_DB_0   = 0,
    ADC_ATTEN_DB_2_5 = 1, 
    ADC_ATTEN_DB_6   = 2,
    ADC_ATTEN_DB_11  = 3
} adc_attenuation_t;
```

## Expected Timeline

- **Phase 1** (Manager fixes): 2-3 hours
- **Phase 2** (Hardware abstraction): 3-4 hours  
- **Phase 3** (Service container): 2 hours
- **Phase 4** (Timing/logic): 1-2 hours

**Total Estimated Effort**: 8-11 hours for complete 108-test suite

## Success Criteria

```bash
# Target outcome:
pio test -e test-coverage
# Expected: 120 test cases: 120 succeeded (12 basic + 108 comprehensive)

python3 scripts/run_tests_with_coverage.py
# Expected: All test environments PASS with comprehensive coverage report
```

## Notes

- **Sensor tests are largely ready** - Reading type conflicts already resolved
- **Manager tests need the most work** - Interface mismatches are significant  
- **Hardware mocking is the key blocker** - ESP32 dependencies must be abstracted
- **Current foundation is solid** - Basic framework provides excellent starting point

The comprehensive test suite represents significant value for code quality and confidence, but requires systematic interface alignment work.