# Testing Guide for Clarity Gauge System

This document describes the comprehensive testing strategy implemented for the Clarity ESP32-based digital gauge system.

## Testing Architecture

The project implements a **hybrid testing approach** combining:

1. **Wokwi Integration Tests** - Hardware-in-loop testing with emulated ESP32
2. **Unity Unit Tests** - Business logic testing without hardware dependencies  
3. **Build Verification** - Multi-environment compilation testing

## Quick Start

### Running All Tests Locally

```bash
# Run Wokwi integration tests (requires Wokwi CLI)
wokwi-cli . --scenario test_scenarios.yaml

# Run unit tests
pio test -e test

# Build verification for all environments
pio run -e debug-local
pio run -e debug-upload  
pio run -e release
```

### Prerequisites

1. **PlatformIO Core**: `pip install platformio`
2. **Wokwi CLI**: Install from [Wokwi docs](https://docs.wokwi.com/wokwi-ci/getting-started)
3. **Wokwi Token**: Required for CI/CD (set `WOKWI_CLI_TOKEN` environment variable)

## Integration Tests (Wokwi)

### Test Scenarios Coverage

The `test_scenarios.yaml` file defines comprehensive integration tests:

#### **System Startup Tests**
- Boot sequence validation
- Panel initialization
- Display rendering verification

#### **Sensor Integration Tests**
- Oil pressure sensor readings (pot1: 0-100%)
- Oil temperature sensor readings (pot2: 0-100%)
- Sensor error handling and edge cases
- Threshold validation (low/normal/high ranges)

#### **Panel Switching Tests**
- Key trigger activation (switch 1, pin 1)
- Lock trigger activation (switch 1, pin 2) 
- Panel restoration logic (switch 1, pin 3)
- Rapid switching scenarios

#### **Hardware Simulation Mapping**
```
pot1 (GPIO VP)    → Oil Pressure Sensor (0-100 PSI)
pot2 (GPIO VN)    → Oil Temperature Sensor (70-250°F)
sw1 pin 1 (GPIO 25) → Key Trigger
sw1 pin 2 (GPIO 26) → Lock Trigger  
sw1 pin 3 (GPIO 27) → Restoration Trigger
```

### Running Integration Tests

```bash
# Single test run
wokwi-cli . --scenario test_scenarios.yaml

# Specific scenario
wokwi-cli . --scenario test_scenarios.yaml --test "Oil Pressure Sensor Low Reading"

# With custom timeout
wokwi-cli . --scenario test_scenarios.yaml --timeout 45000
```

## Unit Tests (Unity Framework)

### Test Coverage

#### **test_preference_manager.cpp**
- Configuration serialization/deserialization
- JSON handling with ArduinoJson
- Default values validation
- Widget location structures
- Reading variant type testing

#### **test_sensor_logic.cpp**  
- Sensor threshold algorithms
- Warning and critical range detection
- Error state handling
- Oil pressure/temperature specific ranges
- Mock sensor implementations

#### **test_interrupt_manager.cpp**
- Trigger priority evaluation algorithms
- Panel switching logic
- Restoration mechanism
- Multi-trigger scenarios
- Edge case handling

### Running Unit Tests

```bash
# Run all unit tests
pio test -e test --verbose

# Run specific test file
pio test -e test --filter "test_preference_manager"

# Debug mode with detailed output
pio test -e test -v --verbose
```

## CI/CD Integration

### GitHub Actions Workflow

The `.github/workflows/test.yml` provides automated testing:

```yaml
jobs:
  wokwi-integration-tests:   # Hardware-in-loop with Wokwi
  unit-tests:                # Unity framework tests
  build-verification:        # Multi-environment builds
```

### Required Secrets

Set in GitHub repository settings:
- `WOKWI_CLI_TOKEN`: Your Wokwi CI token from [wokwi.com](https://wokwi.com)

### Workflow Triggers

- **Push**: `main`, `develop` branches
- **Pull Request**: `main` branch
- **Manual**: Via GitHub Actions UI

## Test Development Guidelines

### Adding Wokwi Integration Tests

1. **Define Hardware Interaction**:
   ```yaml
   - name: "Test new sensor"
     set-pot:
       pot: "pot1" 
       value: 0.75
     expect-text: "Expected Output"
   ```

2. **Test Panel Transitions**:
   ```yaml  
   - name: "Test panel switch"
     set-switch:
       switch: "sw1"
       pin: 1
       value: true
     wait-for-text: "New Panel"
   ```

3. **Verify System Behavior**:
   ```yaml
   - name: "Verify stability"
     wait: 2000
     expect-text: "System Ready"
   ```

### Adding Unit Tests

1. **Create Test File**: `test/test_[component].cpp`

2. **Include Testing Headers**:
   ```cpp
   #ifdef UNIT_TESTING
   #include <unity.h>
   #include "utilities/types.h"
   ```

3. **Implement Mock Classes**:
   ```cpp
   class MockSensor {
       // Mock implementation for testing
   };
   ```

4. **Write Test Functions**:
   ```cpp
   void test_sensor_behavior(void) {
       MockSensor sensor;
       // Test logic
       TEST_ASSERT_EQUAL(expected, actual);
   }
   ```

## Testing Best Practices

### Integration Testing
- **Test Real Workflows**: Complete user scenarios from sensor input to display
- **Hardware Edge Cases**: Extreme sensor values, rapid state changes
- **System Integration**: Panel transitions, trigger priorities, restoration
- **Error Conditions**: Sensor failures, invalid inputs, timing issues

### Unit Testing  
- **Business Logic Focus**: Algorithms, calculations, state management
- **Mock External Dependencies**: Hardware APIs, LVGL, ESP32 features
- **Edge Cases**: Boundary conditions, error states, invalid inputs
- **Fast Execution**: Tests should run quickly without hardware delays

### Continuous Integration
- **Fail Fast**: Tests should detect issues early in development
- **Comprehensive Coverage**: Both integration and unit test perspectives
- **Build Verification**: Ensure all target environments compile successfully
- **Artifact Collection**: Preserve firmware binaries and test reports

## Troubleshooting

### Common Issues

#### **Wokwi Tests Fail**
- Verify `WOKWI_CLI_TOKEN` is set correctly
- Check firmware builds successfully: `pio run -e debug-local`
- Validate `diagram.json` hardware connections
- Increase timeout values for slow operations

#### **Unit Tests Fail**  
- Ensure `#ifdef UNIT_TESTING` guards are present
- Check Unity framework dependency: `throwtheswitch/Unity @ ^2.5.2`
- Verify mock implementations match real interfaces
- Run tests individually to isolate failures

#### **Build Failures**
- Clean build environment: `pio run --target clean`
- Update PlatformIO: `pio upgrade`
- Check library dependencies in `platformio.ini`
- Verify include paths and compilation flags

### Debug Commands

```bash
# Verbose build output
pio run -e debug-local -v

# Test with debug output  
pio test -e test -v --verbose

# Check program size and memory usage
pio run --target size

# Clean and rebuild
pio run --target clean && pio run
```

## Performance Considerations

### Test Execution Times
- **Wokwi Integration**: ~2-5 minutes (hardware simulation overhead)
- **Unity Unit Tests**: ~10-30 seconds (pure logic testing)
- **Build Verification**: ~1-3 minutes per environment

### Optimization Tips
- **Parallel Execution**: Run multiple test jobs concurrently in CI
- **Caching**: Cache PlatformIO dependencies in CI workflows
- **Selective Testing**: Run full suite on main branch, subset on feature branches
- **Early Termination**: Fail fast on first test failure for rapid feedback

## Contributing

When adding new features:

1. **Add Integration Tests**: Cover the complete user workflow in `test_scenarios.yaml`
2. **Add Unit Tests**: Test business logic in dedicated `test/test_*.cpp` files  
3. **Update Documentation**: Document new test scenarios and expected behavior
4. **Verify CI**: Ensure all tests pass in GitHub Actions before merging

For questions or issues with testing, please refer to:
- [PlatformIO Testing Documentation](https://docs.platformio.org/en/latest/advanced/unit-testing/)
- [Wokwi CI Documentation](https://docs.wokwi.com/wokwi-ci/)
- [Unity Testing Framework](http://www.throwtheswitch.org/unity)