# Test Naming Standards - Phase 3 Organization

## Overview
This document defines standardized naming conventions for the Clarity ESP32 test suite to improve maintainability and reduce cross-file dependencies.

## File Naming Conventions

### Test Files
- **Unit Tests:** `test_<category>_<component>.cpp`
  - `test_sensor_key.cpp`
  - `test_manager_preference.cpp`
  - `test_provider_gpio.cpp`

- **Integration Tests:** `test_integration_<scenario>.cpp`
  - `test_integration_power_cycle.cpp`
  - `test_integration_startup_scenarios.cpp`

- **Performance Tests:** `test_performance_<area>.cpp`
  - `test_performance_sensors.cpp`
  - `test_performance_core_logic.cpp`

### Support Files
- **Mock Files:** `mock_<interface>.h`
- **Fixture Files:** `fixture_<category>.h`
- **Utility Files:** `util_<purpose>.h`

## Function Naming Conventions

### Test Functions
Following the pattern: `test_<category>_<component>_<specific_behavior>`

#### Core Logic Tests
```cpp
void test_core_timing_calculation()
void test_core_adc_conversion_accuracy()
void test_core_key_state_determination()
void test_core_config_validation()
```

#### Sensor Tests
```cpp
void test_sensor_key_initialization()
void test_sensor_key_state_transitions()
void test_sensor_key_error_conditions()
void test_sensor_key_performance_benchmark()

void test_sensor_oil_pressure_reading_accuracy()
void test_sensor_oil_pressure_boundary_conditions()
void test_sensor_oil_temperature_conversion_logic()
```

#### Manager Tests
```cpp
void test_manager_preference_initialization()
void test_manager_preference_json_serialization()
void test_manager_preference_error_recovery()
void test_manager_preference_concurrent_access()

void test_manager_panel_lifecycle()
void test_manager_panel_switching_logic()
void test_manager_style_theme_persistence()
```

#### Provider Tests
```cpp
void test_provider_gpio_pin_configuration()
void test_provider_gpio_digital_operations()
void test_provider_gpio_analog_operations()

void test_provider_display_initialization()
void test_provider_display_screen_management()
```

#### Factory Tests
```cpp
void test_factory_manager_dependency_injection()
void test_factory_manager_error_handling()
void test_factory_ui_component_creation()
```

#### Integration Tests
```cpp
void test_integration_power_cycle_recovery()
void test_integration_memory_pressure_handling()
void test_integration_concurrent_trigger_bursts()
void test_integration_theme_persistence_stress()
```

#### Performance Tests
```cpp
void test_performance_adc_conversion_benchmark()
void test_performance_sensor_state_detection()
void test_performance_config_operations()
void test_performance_memory_usage()
```

### Test Suite Runner Functions
```cpp
void runCoreLo gicTests()
void runSensorTests()
void runManagerTests()
void runProviderTests()
void runFactoryTests()
void runIntegrationTests()
void runPerformanceTests()
```

### Setup/Teardown Functions
```cpp
void setUp_<category>_<component>()
void tearDown_<category>_<component>()

// Examples:
void setUp_sensor_key()
void tearDown_sensor_key()
void setUp_manager_preference()
void tearDown_manager_preference()
```

## Variable Naming Conventions

### Test Fixtures
```cpp
std::unique_ptr<SensorTestFixture> sensorFixture;
std::unique_ptr<ManagerTestFixture> managerFixture;
std::unique_ptr<IntegrationTestFixture> integrationFixture;
```

### Mock Objects
```cpp
MockGpioProvider* mockGpio;
MockDisplayProvider* mockDisplay;
MockPreferenceService* mockPreferences;
MockStyleService* mockStyle;
```

### Test Objects
```cpp
KeySensor* keyUnderTest;
PreferenceManager* prefManagerUnderTest;
PanelManager* panelManagerUnderTest;
```

## Constant Naming Conventions

### Test Categories
```cpp
namespace TestCategories {
    constexpr const char* CORE_LOGIC = "CoreLogic";
    constexpr const char* SENSOR = "Sensor";
    constexpr const char* MANAGER = "Manager";
    constexpr const char* PROVIDER = "Provider";
    constexpr const char* FACTORY = "Factory";
    constexpr const char* INTEGRATION = "Integration";
    constexpr const char* PERFORMANCE = "Performance";
}
```

### Performance Thresholds
```cpp
namespace PerformanceThresholds {
    constexpr uint32_t ADC_CONVERSION_MAX_MS = 1000;
    constexpr uint32_t SENSOR_STATE_DETECTION_MAX_MS = 2000;
    constexpr uint32_t CONFIG_OPERATIONS_MAX_MS = 2000;
    constexpr uint32_t MEMORY_LEAK_TOLERANCE_BYTES = 1024;
}
```

### Test Data
```cpp
namespace TestData {
    constexpr const char* VALID_PANEL_NAMES[] = {"OIL", "KEY", "LOCK"};
    constexpr const char* VALID_THEMES[] = {"DAY", "NIGHT"};
    constexpr uint16_t ADC_MIN_VALUE = 0;
    constexpr uint16_t ADC_MAX_VALUE = 4095;
}
```

## Macro Naming Conventions

### Test Assertion Macros
```cpp
#define TEST_ASSERT_VALID_STATE(state, valid_states...)
#define TEST_ASSERT_PERFORMANCE_THRESHOLD(operation, max_ms)
#define TEST_ASSERT_MEMORY_STABLE(operation)
#define TEST_ASSERT_NO_CROSS_FILE_DEPENDENCIES()
```

### Test Pattern Macros
```cpp
#define SENSOR_TEST_PATTERN(sensor_type, test_name)
#define MANAGER_TEST_PATTERN(manager_type, test_name)
#define PERFORMANCE_TEST_PATTERN(category, test_name, threshold_ms)
```

## File Organization Standards

### Directory Structure
```
test/
├── core/                    # Core logic tests
│   ├── test_core_timing.cpp
│   ├── test_core_algorithms.cpp
│   └── test_core_performance.cpp
├── sensors/                 # Sensor tests
│   ├── test_sensor_key.cpp
│   ├── test_sensor_oil.cpp
│   └── test_sensor_performance.cpp
├── managers/                # Manager tests
│   ├── test_manager_preference.cpp
│   ├── test_manager_panel.cpp
│   └── test_manager_style.cpp
├── providers/               # Provider tests
│   ├── test_provider_gpio.cpp
│   └── test_provider_display.cpp
├── factories/               # Factory tests
│   ├── test_factory_manager.cpp
│   └── test_factory_ui.cpp
├── integration/             # Integration tests
│   ├── test_integration_scenarios.cpp
│   └── test_integration_performance.cpp
├── utilities/               # Test utilities
│   ├── test_common.h
│   ├── test_interface.h
│   ├── fixture_sensor.h
│   ├── fixture_manager.h
│   └── mock_implementations.cpp
└── test_suite_organized.cpp # Main organized test runner
```

### Include Standards
```cpp
// Standard order of includes
#include <unity.h>                    // Unity framework
#include "utilities/test_common.h"    // Common test utilities
#include "utilities/test_interface.h" // Standardized interfaces
#include "test_fixtures.h"            // Test fixtures
#include "mock_implementations.h"     // Mock objects
#include "component_under_test.h"     // Component being tested
```

## Benefits of Standardized Naming

1. **Predictable Structure:** Developers can easily locate and understand tests
2. **Reduced Dependencies:** Clear separation of concerns and interfaces
3. **Better Maintainability:** Consistent patterns make modifications easier
4. **Improved Readability:** Self-documenting test names and organization
5. **Automated Tooling:** Enables better test discovery and reporting
6. **Scalability:** Structure supports growth without complexity increase

## Migration Guidelines

When updating existing tests to follow these standards:

1. **Rename functions** to follow the `test_<category>_<component>_<behavior>` pattern
2. **Reorganize files** into the standardized directory structure
3. **Update includes** to use standardized utilities and interfaces
4. **Replace custom fixtures** with standardized test fixtures
5. **Standardize mock usage** across all test files
6. **Add performance thresholds** using the standardized macros

This standardization significantly improves the maintainability and organization of the test suite while eliminating cross-file dependencies.