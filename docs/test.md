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

## Test Environment

The project uses a single test environment that runs the complete test suite:

### Available Test Environment

1. **test-all**: Runs complete test suite (100 tests)

### Environment Configuration

The test environment runs all 100 tests using compilation conditionals:

```ini
[env:test-all]
platform = native
build_flags = 
    -D TEST_ALL_SUITES
    -D UNITY_INCLUDE_DOUBLE
    -D UNIT_TESTING
    -D CLARITY_DEBUG
    -I include
    -I test
    -I test/mocks
    -I test/utilities
    -D LVGL_MOCK
    -g3
    -O0
lib_deps = 
    throwtheswitch/Unity @ ^2.6.0
test_framework = unity
test_filter = test_all
```

## Running Tests

### Execute All Tests
```bash
pio test -e test-all
```

### Execute Tests with Verbose Output
```bash
pio test -e test-all --verbose
```

All 100 tests run as a single comprehensive suite covering:
- **Sensor Tests (21)**: Oil pressure/temperature, key state, lock state, light sensors
- **Manager Tests (15)**: Trigger manager, panel manager, service mocks
- **Component Tests (23)**: OEM oil components, UI components, branding components
- **Integration Tests (20)**: End-to-end scenarios, system integration
- **Infrastructure Tests (21)**: Device layer, factories, utilities, main application

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
=== Clarity Complete Test Suite (All Tests) ===
Running ALL 100 tests across all layers...

--- Sensor Tests (21) ---
--- Manager Core Tests (15) ---
--- Component Tests (23) ---
--- Integration Tests (20) ---
--- Infrastructure Tests (21) ---

=== Complete Test Suite Finished ===
Total: 100 tests executed
100 Tests 0 Failures 0 Ignored
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

Tests are run as a complete suite. Individual test debugging can be done by temporarily modifying the `test_all.cpp` file to comment out unwanted test sections or by examining the detailed test output.

### Memory Debugging

Tests run with memory debugging enabled to catch allocation issues:
```
-D UNITY_INCLUDE_DOUBLE  # Enable floating point assertions
```

## Continuous Integration

Tests are integrated with GitHub Actions for automated validation:

- **Trigger**: Every pull request and push to main/develop branches
- **Platform**: Ubuntu latest
- **Test Execution**: Complete 100-test suite
- **Build Verification**: Release firmware build and size check

The CI workflow runs two jobs in parallel:
1. **Test Job**: Executes all 100 tests with verbose output
2. **Build Job**: Verifies release firmware compilation and reports program size

See `.github/workflows/test.yml` for CI configuration details.

## Adding New Tests

### Guidelines for New Test Code

1. **Add to test_all.cpp**: All test code must be in the single test file
2. **Use TEST_ALL_SUITES**: Tests run under the `#ifdef TEST_ALL_SUITES` compilation block
3. **Follow Naming Convention**: Use `test_<component>_<functionality>` format
4. **Include Setup/Teardown**: Use Unity's `setUp()` and `tearDown()` functions
5. **Mock Dependencies**: Use existing mocks or extend them for new hardware dependencies
6. **Add to RUN_TEST**: Include the new test in the appropriate section's `RUN_TEST()` calls

### Example Test Addition

```cpp
void test_new_sensor_functionality() {
    // Setup
    setUp();
    
    // Test logic
    TEST_ASSERT_EQUAL(expected_value, actual_value);
    
    // Teardown
    tearDown();
}

// Add to the appropriate section in main():
#elif defined(TEST_ALL_SUITES)
    // ... existing tests ...
    printf("--- Sensor Tests (21) ---\n");
    RUN_TEST(test_oil_pressure_sensor_initialization);
    // ... other sensor tests ...
    RUN_TEST(test_new_sensor_functionality);  // Add here
```

This testing approach ensures comprehensive coverage while working within PlatformIO's constraints, providing fast feedback on core functionality without requiring ESP32 hardware.