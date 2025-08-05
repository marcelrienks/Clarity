# Testing Documentation

## Overview

The Clarity project includes comprehensive unit tests for core logic, sensors, managers, components, and integration testing. The test suite uses Unity testing framework and runs on the native platform without requiring ESP32 hardware.

## Testing Framework

- **Framework**: Unity Testing Framework v2.6.0
- **Platform**: Native (cross-platform testing without ESP32 hardware)
- **Mocking**: Custom LVGL and hardware mocks for embedded abstractions
- **Test Runner**: PlatformIO with Unity integration

## PlatformIO and Unity Limitations

### Single Test File Constraint

Due to a limitation with PlatformIO and Unity integration, the testing framework cannot properly handle multiple separate test files when using build filters. This creates linking conflicts where multiple test files try to link together, causing compilation failures.

**Workaround Implemented**: All tests are consolidated into a single `test_all.cpp` file with compilation conditionals to enable different test suites:

```cpp
#ifdef TEST_SENSORS_ONLY
    // Only sensor tests compile
#endif

#ifdef TEST_MANAGERS_CORE_ONLY  
    // Only core manager tests compile
#endif

#ifdef TEST_COMPONENTS_ONLY
    // Only component tests compile
#endif
```

### Nested Directory Limitation

PlatformIO has an additional limitation where test files within nested directories are not automatically discovered and only tests in the root test directory are executed.

**Workaround**: All test code is consolidated in the root `/test/` directory with a single test file approach.

## Test Environments

The project defines multiple test environments to work around the single-file limitation:

### Available Test Environments

1. **test-all**: Runs complete test suite
2. **test-sensors**: Runs only sensor-related tests
3. **test-managers-core**: Runs core manager tests
4. **test-components**: Runs component tests
5. **test-integration**: Runs integration tests
6. **test-infrastructure**: Runs infrastructure tests

### Environment Configuration

Each test environment uses compilation conditionals to include only relevant test code:

```ini
[env:test-sensors]
platform = native
build_flags = 
    -D TEST_SENSORS_ONLY
    -D UNITY_INCLUDE_DOUBLE
    -I include
    -I test
    -I test/mocks
    -I test/utilities
    -D LVGL_MOCK
```

## Running Tests

### Execute All Tests
```bash
pio.exe test -e test-all
```

### Execute Specific Test Suites
```bash
# Sensor tests only
pio.exe test -e test-sensors

# Manager tests only  
pio.exe test -e test-managers-core

# Component tests only
pio.exe test -e test-components

# Integration tests only
pio.exe test -e test-integration

# Infrastructure tests only
pio.exe test -e test-infrastructure
```

### Execute Tests with Verbose Output
```bash
pio.exe test -e test-all --verbose
```

### Execute Tests with Specific Filter
```bash
pio.exe test -e test-all --filter="test_sensor*"
```

## Test Structure

### Test Organization

Tests are organized by functional areas within the single `test_all.cpp` file:

- **Sensor Tests**: Input validation, ADC conversion, state detection
- **Manager Tests**: Configuration, panel switching, style management
- **Component Tests**: UI rendering, data binding, visual updates
- **Integration Tests**: End-to-end workflows, service interactions
- **Infrastructure Tests**: Service container, factories, utilities

### Mock System

The test suite includes comprehensive mocks for embedded dependencies:

#### Hardware Mocks
- `mock_gpio_provider.h/cpp`: GPIO operations without hardware
- `arduino_mock.h`: Arduino framework functions
- `esp32-hal-log.h`: ESP32 logging system

#### Display System Mocks  
- `lvgl_mock.h`: LVGL graphics library
- `mock_display_provider.h`: Display abstraction layer

#### Test Utilities
- `test_helpers.h/cpp`: Common test utilities and fixtures
- `unity_config.h`: Unity framework configuration

## Test Coverage

### Core Areas Tested

1. **Timing/Ticker Logic**
   - Frame timing calculations
   - Dynamic delay handling
   - Performance optimization

2. **Sensor Logic**
   - Value change detection
   - ADC conversions and scaling
   - Key state determination
   - Oil pressure/temperature monitoring

3. **Configuration Management**
   - Settings persistence
   - Validation and defaults
   - Preference loading/saving

4. **Panel Management**
   - Panel lifecycle (init → load → update)
   - Panel switching logic
   - Trigger-based transitions

5. **Component Rendering**
   - UI element creation and updates
   - Theme switching
   - Visual state management

### Test Results Format

Typical test execution output:
```
Running test-all...
12 test cases: 12 succeeded in 00:00:08.063
Memory usage: 85% of 32KB
```

## Debugging Tests

### Debug Build Flags

Test environments include debug flags for troubleshooting:
```
-g3          # Maximum debug information
-O0          # No optimization for debugging
-D CLARITY_DEBUG  # Enable application debug output
```

### Running Individual Tests

To debug specific test functions, use Unity's test runner with filters:
```bash
pio.exe test -e test-all --filter="RUN_TEST(test_sensor_oil_pressure)"
```

### Memory Debugging

Tests run with memory debugging enabled to catch allocation issues:
```
-D UNITY_INCLUDE_DOUBLE  # Enable floating point assertions
```

## Continuous Integration

Tests are integrated with GitHub Actions for automated validation:

- **Trigger**: Every pull request and push to main
- **Matrix**: Cross-platform testing (Linux, Windows, macOS)
- **Coverage**: All test environments executed
- **Reporting**: Test results and coverage reports

See `.github/workflows/test.yml` for CI configuration details.

## Adding New Tests

### Guidelines for New Test Code

1. **Add to test_all.cpp**: All test code must be in the single test file
2. **Use Compilation Conditionals**: Wrap tests in appropriate `#ifdef` blocks
3. **Follow Naming Convention**: Use `test_<component>_<functionality>` format
4. **Include Setup/Teardown**: Use Unity's `setUp()` and `tearDown()` functions
5. **Mock Dependencies**: Use existing mocks or extend them for new hardware dependencies

### Example Test Addition

```cpp
#ifdef TEST_SENSORS_ONLY
void test_new_sensor_functionality() {
    // Setup
    setUp();
    
    // Test logic
    TEST_ASSERT_EQUAL(expected_value, actual_value);
    
    // Teardown
    tearDown();
}
#endif
```

This testing approach ensures comprehensive coverage while working within PlatformIO's constraints, providing fast feedback on core functionality without requiring ESP32 hardware.