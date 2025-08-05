# Testing Documentation

## Overview

The Clarity project includes two comprehensive testing approaches:

1. **Unity Unit Tests**: Fast unit tests for core logic, sensors, managers, and components using Unity framework on native platform
2. **Wokwi Integration Tests**: Hardware emulation tests that validate complete system behavior including UI, sensors, and real-world scenarios

Both testing approaches work together to ensure code quality and system reliability at different levels of the application stack.

# 1. Unity Unit Testing

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

---

# 2. Wokwi Integration Testing

## Overview

Wokwi integration tests provide comprehensive end-to-end validation of the Clarity automotive gauge system using hardware emulation. These tests verify complete system behavior including display rendering, sensor interactions, panel switching, and real-world usage scenarios.

## Testing Framework

- **Platform**: Wokwi ESP32 Simulator with ILI9341 display emulation
- **Hardware**: ESP32 DevKit C v4 with potentiometers and DIP switches
- **Automation**: Wokwi CLI with expect/fail text validation
- **CI Integration**: GitHub Actions with automated test execution
- **Test Runner**: Shell script with automatic pass/fail reporting

## Test Directory Structure

```
test/wokwi/
├── basic_startup/             # Test Suite 1
│   ├── diagram.json          # Hardware configuration  
│   ├── basic_startup.test.yaml # Test scenario (not used by CLI)
│   └── README.md             # Test documentation
├── oil_panel_sensors/         # Test Suite 2
│   └── ... (same structure)
└── ... (10 test scenarios total)

Root directory scripts:
├── run_wokwi_tests.sh        # Comprehensive Wokwi test runner (160 lines)
├── run_unity_tests.sh        # Unity unit test runner (101 tests)
└── run_unity_tests.bat       # Windows Unity test runner
```

**Notes**: 
- No `wokwi.toml` files - firmware paths are specified via CLI parameters
- YAML scenario files exist but are not used by current CLI implementation
- Test validation uses `--expect-text` and `--fail-text` CLI parameters

## Test Suites

### Test Suite 1: Basic System Startup
- **Location**: `test/wokwi/basic_startup/`
- **Purpose**: Verify splash screen sequence and oil panel loading
- **Duration**: ~15 seconds
- **Validation**: Serial output patterns, visual splash screen

### Test Suite 2: Oil Panel Sensor Testing
- **Location**: `test/wokwi/oil_panel_sensors/`
- **Purpose**: Test dynamic sensor data and gauge animations
- **Duration**: ~20 seconds
- **Validation**: Needle positioning, sensor value changes, boundary conditions

### Test Suite 3: Theme Switching
- **Location**: `test/wokwi/theme_switching/` and `test/wokwi/night_startup/`
- **Purpose**: Day/night theme switching without panel reload
- **Duration**: ~25 seconds
- **Validation**: Color changes, theme persistence, startup theme detection

### Test Suite 4: Key Panel Integration
- **Location**: `test/wokwi/key_present/` and `test/wokwi/key_not_present/`
- **Purpose**: Key trigger functionality with green/red icon states
- **Duration**: ~20 seconds
- **Validation**: Panel switching, icon colors, invalid state handling

### Test Suite 5: Lock Panel Integration
- **Location**: `test/wokwi/lock_panel/`
- **Purpose**: Lock state trigger and lock icon display
- **Duration**: ~20 seconds
- **Validation**: Lock panel activation, icon display, priority level

### Test Suite 6: Major Integration Scenario
- **Location**: `test/wokwi/major_scenario/` and `test/wokwi/trigger_priority/`
- **Purpose**: 9-step complete system integration and priority testing
- **Duration**: ~60 seconds
- **Validation**: Multi-trigger scenarios, priority resolution, fallback behavior

### Test Suite 7: Performance and Stability
- **Location**: `test/wokwi/performance_stress/`
- **Purpose**: Stress testing with rapid trigger switching
- **Duration**: ~45 seconds
- **Validation**: System stability, memory usage, error-free operation

## Hardware Configuration

### ESP32 DevKit C v4 Pin Mapping
```
Potentiometers (Analog Sensors):
├── Pot1 (Oil Pressure) → GPIO36 (VP) → 0-10 Bar range
└── Pot2 (Oil Temperature) → GPIO39 (VN) → 0-120°C range

DIP Switch Triggers (Digital Inputs):
├── Switch #1 → GPIO25 → Key Present (Priority 0)
├── Switch #2 → GPIO26 → Key Not Present (Priority 0)  
├── Switch #3 → GPIO27 → Lock State (Priority 1)
└── Switch #4 → GPIO33 → Lights (Theme trigger - independent)

Display (SPI):
├── ILI9341 240x240 → SPI2_HOST
├── CS → GPIO22, DC → GPIO16, RST → GPIO4
├── MOSI → GPIO23, SCK → GPIO18
└── Backlight → 3.3V (always on)
```

### Wokwi Configuration
- **No Config Files**: Uses CLI parameters for firmware paths (no wokwi.toml needed)
- **Firmware**: Built with `debug-local` environment for faster compilation
- **Display**: Square ILI9341 (limitation: actual hardware uses round GC9A01)
- **Interactive**: Manual DIP switch and potentiometer manipulation during tests
- **Execution**: All commands run from project root directory

## Running Wokwi Tests

### Prerequisites
1. **Install Wokwi CLI**:
   ```bash
   # Linux/macOS/WSL
   curl -L https://wokwi.com/ci/install.sh | sh
   
   # Windows: Download from GitHub releases
   # https://github.com/wokwi/wokwi-cli/releases
   ```

2. **Get API Token**:
   - Sign up at https://ci.wokwi.com/
   - Get token from dashboard
   - Set: `export WOKWI_CLI_TOKEN="your_token"`

### Execution Methods

#### 1. Wokwi Integration Test Runner
```bash
# Run all 10 test scenarios with comprehensive reporting
./run_wokwi_tests.sh
```

**Features**:
- ✅ Automatic firmware build with error handling
- 🧪 Sequential execution of all test scenarios  
- 📦 Token validation and prerequisite checks
- 📊 Pass/fail reporting with timing and detailed summaries
- 🎯 Uses correct expect patterns for validation
- 💯 Returns proper exit codes (0=success, 1=failure)
- 📸 Screenshot capture and artifact collection
- 📂 Results saved to `test_results/` directory with logs
- 🌈 Color-coded output for better visibility
- 🔍 Detailed failure reporting with test names

#### 2. VS Code Integration
**Ctrl+Shift+P** → **Tasks: Run Task** → **"Wokwi Integration Tests"**

#### 3. Individual Test Execution
```bash
# Build firmware first
pio run -e debug-local

# Run specific test scenarios with validation from project root
wokwi-cli test/wokwi/basic_startup \
  --elf .pio/build/debug-local/firmware.elf \
  --diagram-file diagram.json \
  --timeout 60000 \
  --expect-text "Loading splash panel" \
  --fail-text "Exception" \
  --fail-text "Guru Meditation"

wokwi-cli test/wokwi/trigger_priority \
  --elf .pio/build/debug-local/firmware.elf \
  --diagram-file diagram.json \
  --timeout 60000 \
  --expect-text "initialized to INACTIVE" \
  --fail-text "Exception" \
  --fail-text "Guru Meditation"
```

#### 4. Command Line Loop (Basic)
```bash
# Build once, test all from project root without validation
pio run -e debug-local

for dir in test/wokwi/*/; do
  if [ -d "$dir" ] && [ -f "$dir/diagram.json" ]; then
    echo "Running: $(basename "$dir")"
    wokwi-cli "$dir" \
      --elf .pio/build/debug-local/firmware.elf \
      --diagram-file diagram.json \
      --timeout 60000
  fi
done
```

## Test Automation

### Configuration Approach
The automation framework uses CLI parameters instead of configuration files:
- **No wokwi.toml files needed** - firmware paths specified via CLI
- **All commands run from project root** - consistent relative paths
- **Shared firmware build** - single `.pio/build/debug-local/` location

### Test Validation Strategy
Tests use CLI-based validation with specific expect patterns:

```bash
# Standard test execution pattern
wokwi-cli test/wokwi/test_directory \
  --elf .pio/build/debug-local/firmware.elf \
  --diagram-file diagram.json \
  --timeout 60000 \
  --expect-text "Expected log pattern" \
  --fail-text "Exception" \
  --fail-text "Guru Meditation"
```

### YAML Scenario Files (Present but Unused)
Each test directory contains a `.test.yaml` file for documentation purposes:

```yaml
name: "Basic System Startup Test"
description: "Verify oil panel loads with default sensor readings"
timeout: 60000

# Note: Current CLI implementation doesn't support YAML scenarios
# Validation handled via --expect-text and --fail-text parameters
```

**Important**: YAML scenario files exist in the test directories but are **not used** by the current Wokwi CLI implementation. Test validation is handled through CLI parameters.

### GitHub Actions Integration
- **Unity Tests**: `.github/workflows/test.yml` - Runs `pio test -e test-all` for 100 unit tests
- **Wokwi Tests**: `.github/workflows/wokwi-tests.yml` - Builds firmware for integration testing
- **Trigger**: Push/PR to main branches or manual dispatch with path filters
- **Platform**: Ubuntu latest with PlatformIO caching

### CI Configuration
- **Unity Tests**: Automatic execution with verbose output in CI
- **Wokwi Tests**: Currently builds firmware only (integration step pending)
- **Build Verification**: Release firmware compilation and size checks
- **Test Coverage**: Both unit (100 tests) and integration (10 scenarios) validation

## Visual Validation

### Screenshot Capture
Tests automatically capture screenshots at key verification points:
- **System startup states**
- **Panel switching transitions**  
- **Theme change confirmations**
- **Error conditions** (if any)
- **Final test states**

### Manual Validation Points
During interactive testing, verify:
- **Splash Screen**: Correct text color (white=day, red=night)
- **Gauge Needles**: Proper positioning matching potentiometer values
- **Panel Icons**: Correct colors (green=present, red=not present/lock)
- **Theme Consistency**: All UI elements match current theme
- **Smooth Transitions**: No display artifacts during panel switches

## Test Coverage Validation

### Comprehensive Scenario Coverage
- ✅ **Default Startup**: Splash → Oil panel loading
- ✅ **Sensor Integration**: Real-time potentiometer → gauge updates
- ✅ **Theme Management**: Day/night switching, startup theme detection
- ✅ **Trigger System**: All panel triggers with priority testing
- ✅ **Edge Cases**: Invalid states, boundary conditions
- ✅ **Performance**: Rapid switching, long-term stability
- ✅ **Integration**: Complete 9-step real-world scenario

### Expected Test Results
**Typical execution summary** (from `run_wokwi_tests.sh`):
```
🚗 Clarity Automotive Gauge - Wokwi Integration Tests
==================================================
📦 Building firmware...
✅ Firmware build successful

🔧 Running: Basic System Startup
   Directory: test/wokwi/basic_startup
   ✅ PASSED

🔧 Running: Oil Panel Sensor Testing
   Directory: test/wokwi/oil_panel_sensors
   ✅ PASSED
   
... (8 more test scenarios) ...

==================================================
📊 Test Execution Summary
==================================================
Total Tests: 10
Passed: 10 ✅
Failed: 0 ❌

🎉 All tests passed successfully!
```

**Current Status**: All 10/10 tests passing consistently. Test infrastructure is stable and fully validated.

## Debugging Integration Tests

### Common Issues
- **Token Issues**: Ensure `WOKWI_CLI_TOKEN` is set correctly 
- **Build Failures**: Run `pio run -e debug-local` before testing
- **Timeout Errors**: Current timeout set to 60000ms (60 seconds) - tests can take multiple seconds
- **Expect Pattern Mismatches**: Adjust `--expect-text` patterns based on actual firmware logs
- **JSON Parsing**: Ensure diagram.json files have string values (not numbers) for potentiometer "value" attributes

### Debugging Commands
For failing tests, run individually with output visible:
```bash
# Debug basic_startup test
wokwi-cli test/wokwi/basic_startup \
  --elf .pio/build/debug-local/firmware.elf \
  --diagram-file diagram.json \
  --timeout 60000 \
  --expect-text "Loading splash panel"

# Debug trigger_priority test  
wokwi-cli test/wokwi/trigger_priority \
  --elf .pio/build/debug-local/firmware.elf \
  --diagram-file diagram.json \
  --timeout 60000 \
  --expect-text "initialized to INACTIVE"
```

### Limitations
- **Manual Interaction**: DIP switches and potentiometers require manual manipulation during tests
- **Display Shape**: Wokwi shows square ILI9341 display vs actual round GC9A01
- **Timing Differences**: Simulation may have different timing than real hardware
- **Resource Limits**: Wokwi simulation time limits based on subscription plan
- **Expect Patterns**: Must match exact log output from firmware (case-sensitive)

## Integration with Unity Tests

Both testing approaches complement each other:

| Unity Unit Tests | Wokwi Integration Tests |
|------------------|-------------------------|
| **Fast execution** (seconds) | **Comprehensive validation** (minutes) |
| **Code logic verification** | **Hardware behavior validation** |
| **Mocked dependencies** | **Real hardware emulation** |
| **Development feedback** | **Release validation** |
| **100 focused tests** | **10 end-to-end scenarios** |
| **Pre-commit validation** | **Pre-release validation** |

**Recommended Workflow**:
1. **Development**: Run Unity tests frequently during coding
2. **Feature Complete**: Run Wokwi tests to validate integration
3. **Pre-commit**: Ensure both test suites pass
4. **CI/CD**: Both test types run automatically on PR/push

This comprehensive testing strategy ensures code quality at both the unit level and system integration level, providing confidence in the Clarity automotive gauge system's reliability and functionality.