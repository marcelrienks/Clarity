# Clarity Test Suite Documentation

## Overview

The Clarity project has a comprehensive unit test suite covering all major components of the ESP32-based digital gauge system. The test infrastructure uses Unity testing framework with PlatformIO's native test runner.

## Test Suite Statistics

- **Total Tests**: 249 individual test cases across 21 test files
- **Test Coverage**: All major system components
- **Framework**: Unity Testing Framework via PlatformIO
- **Platform**: Native (cross-platform compilation for testing)
- **Status**: ✅ All tests properly configured and executable

## Test Categories

### Core System Tests (21 test files)

| Category | Files | Test Count | Description |
|----------|-------|------------|-------------|
| **Sensors** | 5 files | ~75 tests | Hardware sensor interfaces and data processing |
| **Managers** | 4 files | ~60 tests | System management and coordination logic |
| **Providers** | 2 files | ~30 tests | Hardware abstraction layer implementations |
| **Factories** | 3 files | ~45 tests | Object creation and dependency injection |
| **Components** | 2 files | ~25 tests | UI component interfaces and behavior |
| **Panels** | 2 files | ~20 tests | Display panel management and rendering |
| **System** | 1 file | ~8 tests | Core system services and containers |
| **Utilities** | 2 files | ~11 tests | Helper functions and timing utilities |

### Individual Test Files

```
test/
├── test_key_sensor.cpp              # Key presence detection logic
├── test_light_sensor.cpp            # Ambient light sensing
├── test_lock_sensor.cpp             # Lock state detection
├── test_oil_pressure_sensor.cpp     # Oil pressure monitoring
├── test_oil_temperature_sensor.cpp  # Oil temperature monitoring
├── test_panel_manager.cpp           # Panel lifecycle management
├── test_preference_manager.cpp      # Settings and configuration
├── test_style_manager.cpp           # UI styling and themes
├── test_trigger_manager.cpp         # Event triggering system
├── test_gpio_provider.cpp           # GPIO hardware abstraction
├── test_lvgl_display_provider.cpp   # Display driver interface
├── test_manager_factory.cpp         # Manager object creation
├── test_ui_factory.cpp              # UI component factories
├── test_ui_factory_simplified.cpp   # Simplified UI factories
├── test_component_interfaces.cpp    # Component behavior contracts
├── test_component_interfaces_standalone.cpp # Isolated component tests
├── test_panel_interfaces.cpp        # Panel behavior contracts
├── test_panel_interfaces_standalone.cpp # Isolated panel tests
├── test_service_container.cpp       # Dependency injection container
├── test_ticker.cpp                  # Timing and task scheduling
└── test_simple_ticker.cpp           # Basic timing utilities
```

## Running Tests

### Single Command - Run All Tests

```bash
pio test -e test-all
```

This command will:
- Execute all 21 test files sequentially
- Run each test file as an individual Unity executable
- Report results for all 249 test cases
- Generate coverage information
- Provide comprehensive pass/fail summary

### Test Environment Configuration

The `test-all` environment is configured in `platformio.ini`:

```ini
[env:test-all]
platform = native
build_flags = 
    -std=gnu++17
    -D UNIT_TESTING
    -D CLARITY_DEBUG
    -D UNITY_INCLUDE_DOUBLE
    -I include
    -I test
    -I test/mocks
    -I test/utilities
    --coverage
    -fprofile-arcs
    -ftest-coverage
    -g3
    -O0
    -lgcov
lib_deps = 
    throwtheswitch/Unity @ ^2.5.2
test_framework = unity
test_build_src = yes
test_filter = test_*
```

### Alternative Test Commands

```bash
# Run tests with verbose output
pio test -e test-all -v

# Run tests with maximum verbosity for debugging
pio test -e test-all -vvv

# Run tests and generate coverage report
pio test -e test-all && python scripts/coverage.py
```

## Test Architecture

### Unity Test Structure

Each test file follows the standard Unity testing pattern:

```cpp
#include <unity.h>
#include "test_fixtures.h"
// ... other includes

void setUp(void) {
    // Setup code before each test
}

void tearDown(void) {
    // Cleanup code after each test
}

void test_specific_functionality() {
    // Individual test implementation
    TEST_ASSERT_EQUAL(expected, actual);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_specific_functionality);
    // ... more tests
    
    return UNITY_END();
}
```

### Mock Infrastructure

Tests use comprehensive mock implementations located in `test/mocks/`:

- **Arduino.h**: Arduino framework mocking
- **mock_gpio_provider.h**: GPIO hardware abstraction
- **mock_services.h**: System service mocking
- **mock_implementations.cpp**: Mock object implementations
- **lvgl.h**: LVGL graphics library mocking

### Test Utilities

Common test utilities in `test/utilities/`:

- **test_common.h**: Shared test functions and macros
- **test_fixtures.h**: Reusable test setup patterns
- **test_interface.h**: Common test interface definitions

## Expected Results

When running `pio test -e test-all`, you should see:

```
Collected 21 tests

Processing test_key_sensor.cpp in test-all environment
✓ test_key_sensor_init                    [PASSED]
✓ test_key_sensor_key_present_state       [PASSED]
✓ test_key_sensor_key_not_present_state   [PASSED]
... (more tests)

Processing test_light_sensor.cpp in test-all environment
✓ test_light_sensor_initialization        [PASSED]
✓ test_light_sensor_threshold_detection   [PASSED]
... (more tests)

[Continues for all 21 test files]

=================================== SUMMARY ====================================
Environment    Test           Status    Duration
-------------  -------------  --------  ------------
test-all       21 files       PASSED    00:XX:XX.XXX
================ 249 test cases: 249 succeeded in 00:XX:XX.XXX ================
```

## Continuous Integration

### GitHub Actions Integration

```yaml
name: Unit Tests
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-python@v4
        with:
          python-version: '3.x'
      - name: Install PlatformIO
        run: pip install platformio
      - name: Run Unit Tests
        run: pio test -e test-all
      - name: Generate Coverage Report
        run: python scripts/coverage.py
```

### Local Development Workflow

```bash
# 1. Make code changes
# 2. Run all tests
pio test -e test-all

# 3. If tests pass, commit changes
git add .
git commit -m "Your changes"

# 4. Push to trigger CI
git push
```

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure all dependencies are installed
   ```bash
   pio pkg install -e test-all
   ```

2. **Test Failures**: Check mock implementations are up to date
   ```bash
   # Verify test environment builds correctly
   pio run -e test-all
   ```

3. **Coverage Issues**: Ensure gcov is available
   ```bash
   # Install coverage tools
   sudo apt-get install gcov lcov  # Linux
   brew install gcov               # macOS
   ```

### Debug Individual Tests

To debug a specific test file:

```bash
# Move other test files temporarily
mkdir test/temp
mv test/test_*.cpp test/temp/
mv test/temp/test_specific_file.cpp test/

# Run single test
pio test -e test-all

# Restore all tests
mv test/temp/*.cpp test/
rmdir test/temp
```

## Test Maintenance

### Adding New Tests

1. Create new test file following naming convention: `test_<component>.cpp`
2. Follow Unity test structure with setUp(), tearDown(), and main()
3. Include necessary mocks and fixtures
4. Add tests to verify component functionality
5. Run `pio test -e test-all` to verify integration

### Updating Existing Tests

1. Maintain test isolation - each test should be independent
2. Update mocks when interfaces change
3. Ensure setUp() and tearDown() properly initialize/cleanup
4. Verify all tests pass after changes

## Summary

The Clarity test suite provides comprehensive coverage of all system components with a simple, unified execution model. The single command `pio test -e test-all` runs all 249 tests across 21 files, providing confidence in system reliability and facilitating continuous integration workflows.

**Key Benefits:**
- ✅ Single command execution
- ✅ Comprehensive coverage (249 tests)
- ✅ Fast feedback loop
- ✅ CI/CD integration ready
- ✅ Isolated, maintainable test structure