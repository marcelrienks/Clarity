# Integration Test Implementation Strategy

**Related Documentation:**
- **[Test Scenarios](test.md)** - Test requirements and scenarios
- **[Architecture](architecture.md)** - System architecture overview
- **[Requirements](requirements.md)** - Functional and non-functional requirements
- **[Application Flow](diagrams/application-flow.md)** - Runtime processing flow

## Executive Summary

This document outlines the comprehensive strategy for implementing automated integration tests for the Clarity ESP32-based digital gauge system. The approach transforms the current C++-based Unity tests into a hybrid testing architecture that leverages Wokwi's YAML automation scenarios for hardware simulation while maintaining Unity framework for complex assertions and state validation.

## Table of Contents

1. [Current State Analysis](#current-state-analysis)
2. [Proposed Architecture](#proposed-architecture)
3. [Implementation Components](#implementation-components)
4. [Test Orchestration](#test-orchestration)
5. [CI/CD Integration](#cicd-integration)
6. [Implementation Roadmap](#implementation-roadmap)
7. [Success Metrics](#success-metrics)

## Current State Analysis

### Existing Test Infrastructure

The current implementation provides two test suites:

- **Basic Test**: 5 phases, ~5 seconds runtime (hardware validation focus)
- **Full Test**: 7 phases, ~2 minutes runtime (complete system integration)
- **Framework**: Unity C++ test framework with manual hardware simulation
- **Execution**: Single Wokwi session with sequential phase execution

### Identified Gaps

1. **Tight Coupling**: Tests are tightly coupled to C++ implementation code
2. **Limited Separation**: No clear separation between hardware simulation and business logic validation
3. **Reusability Issues**: Limited reusability of test scenarios across different test suites
4. **Basic Validation**: Serial output validation limited to simple string matching
5. **No Visual Testing**: Lacks UI/display validation capabilities
6. **Timing Fragility**: Timing dependencies make tests prone to intermittent failures

## Proposed Architecture

### Hybrid Testing Approach

The new architecture maintains the **single-session, sequential execution** model while introducing a clear separation of concerns:

```
┌─────────────────────────────────────────┐
│         Test Orchestration Layer        │
├─────────────────────────────────────────┤
│  YAML Scenarios  │  Unity Assertions    │
│  (Hardware Sim)  │  (Logic Validation)  │
├──────────────────┴──────────────────────┤
│         Serial Communication            │
│         Protocol & Markers              │
├─────────────────────────────────────────┤
│         Wokwi Simulation Engine         │
└─────────────────────────────────────────┘
```

### Key Design Principles

1. **Single Session Execution**: All 7 phases run consecutively in one Wokwi simulation
2. **State Persistence**: System state carries through all phases without reset
3. **Hardware Continuity**: Hardware states maintained across phase transitions
4. **Modular Scenarios**: Individual phase scenarios combined into master orchestrator
5. **Clear Separation**: Hardware simulation (YAML) vs business logic (Unity)

## Implementation Components

### Directory Structure

```
test/wokwi/
├── scenarios/
│   ├── full-integration-test.yaml    # Master orchestrator
│   ├── basic-hardware-test.yaml      # Quick validation
│   ├── phase1-startup.yaml           # Individual phase scenarios
│   ├── phase2-sensors.yaml
│   ├── phase3-triggers.yaml
│   ├── phase4-errors.yaml
│   ├── phase5-themes.yaml
│   ├── phase6-config.yaml
│   └── phase7-validation.yaml
├── helpers/
│   ├── hardware_simulator.h          # Hardware abstraction
│   ├── serial_protocol.h             # Communication protocol
│   ├── timing_utils.h                # Timing and synchronization
│   └── state_validator.h             # State machine validation
├── tests/
│   ├── test_basic_hardware.cpp       # Basic test implementation
│   ├── test_full_integration.cpp     # Full test implementation
│   └── test_framework.cpp            # Common test utilities
└── configs/
    ├── wokwi-basic.toml              # Basic test configuration
    ├── wokwi-full.toml               # Full test configuration
    └── diagram.json                   # Hardware component definitions
```

### Serial Communication Protocol

A structured protocol enables synchronization between YAML scenarios and Unity tests:

```cpp
// test/wokwi/helpers/serial_protocol.h
class TestSerialProtocol {
public:
    // Phase markers
    static void emitPhaseStart(int phase);
    static void emitPhaseComplete(int phase);
    
    // Event markers
    static void emitButtonEvent(const char* button);
    static void emitSensorChange(const char* sensor, int value);
    static void emitPanelLoad(const char* panel);
    static void emitThemeChange(const char* theme);
    static void emitError(const char* error);
    
    // Structured output for complex validations
    static void emitJson(const char* event, JsonDocument& data);
};
```

**Marker Format**: `[MARKER_NAME:optional_data]`

Examples:
- `[PHASE_1_START]`
- `[BUTTON_ACTION_PRESSED]`
- `[PANEL_LOADED:OilPanel]`
- `[SENSOR_CHANGED:pressure:2047]`

### Hardware Simulation Helpers

Abstraction layer for hardware interaction:

```cpp
// test/wokwi/helpers/hardware_simulator.h
class HardwareSimulator {
public:
    // Button simulation
    static void simulateButtonPress(int pin, int duration_ms);
    static void simulateButtonSequence(int pin, int count, int interval_ms);
    
    // Sensor simulation
    static void setSensorValue(int pin, int value);
    static void sweepSensor(int pin, int start, int end, int step_ms);
    
    // Validation
    static bool validateGPIOConfiguration();
    static bool validatePinMode(int pin, int expected_mode);
};
```

### State Validation Framework

Ensures system state consistency across phases:

```cpp
// test/wokwi/helpers/state_validator.h
class StateValidator {
    struct SystemState {
        const char* current_panel;
        const char* theme;
        bool triggers[5];      // key, lock, lights, error states
        int sensor_values[2];  // pressure, temperature
        unsigned long phase_duration_ms;
    };
    
    static void captureState(SystemState& state);
    static void validateStateTransition(
        const SystemState& before, 
        const SystemState& after,
        const char* event
    );
    static void assertValidState(const SystemState& state);
};
```

## Test Orchestration

### Master YAML Orchestrator

The master orchestrator ensures all phases run sequentially in a single session:

```yaml
# test/wokwi/scenarios/full-integration-test.yaml
name: 'Clarity Full System Integration Test'
version: 1
author: 'Clarity Test Suite'
description: 'Complete 7-phase integration test running in single session'

steps:
  # Test initialization
  - wait-serial: '[TEST_START]'
  - delay: 100ms
  
  # Phase 1: System Startup & Initial State (30s)
  - wait-serial: '[PHASE_1_START]'
  - include: phase1-startup.yaml
  - wait-serial: '[PHASE_1_COMPLETE]'
  
  # Phase 2: Sensor Data & Animations (45s)
  # Continues immediately - no reset
  - wait-serial: '[PHASE_2_START]'
  - include: phase2-sensors.yaml
  - wait-serial: '[PHASE_2_COMPLETE]'
  
  # Phase 3: Trigger System Testing (90s)
  # Builds on existing state from Phase 2
  - wait-serial: '[PHASE_3_START]'
  - include: phase3-triggers.yaml
  - wait-serial: '[PHASE_3_COMPLETE]'
  
  # Phase 4: Error Handling System (60s)
  # Error system interacts with current triggers
  - wait-serial: '[PHASE_4_START]'
  - include: phase4-errors.yaml
  - wait-serial: '[PHASE_4_COMPLETE]'
  
  # Phase 5: Trigger Deactivation & Themes (45s)
  # Tests restoration with accumulated state
  - wait-serial: '[PHASE_5_START]'
  - include: phase5-themes.yaml
  - wait-serial: '[PHASE_5_COMPLETE]'
  
  # Phase 6: Configuration System (120s)
  # Config changes affect persistent system
  - wait-serial: '[PHASE_6_START]'
  - include: phase6-config.yaml
  - wait-serial: '[PHASE_6_COMPLETE]'
  
  # Phase 7: Final System Validation (30s)
  # Validates complete system after all changes
  - wait-serial: '[PHASE_7_START]'
  - include: phase7-validation.yaml
  - wait-serial: '[PHASE_7_COMPLETE]'
  
  # Test completion - only after all phases
  - wait-serial: '[ALL_TESTS_PASSED]'
```

### Unity Test Implementation

The C++ test maintains context across all phases:

```cpp
// test/wokwi/test_full_integration.cpp
class FullIntegrationTest {
private:
    TestContext context;  // Persistent across all phases
    
public:
    void test_full_system_integration() {
        // Initialize test
        Serial.println("[TEST_START]");
        context.test_start_ms = millis();
        
        // Phase 1: System Startup (system persists after)
        runPhase1_SystemStartup();
        TEST_ASSERT_EQUAL_STRING("OilPanel", context.current_panel);
        TEST_ASSERT_EQUAL_STRING("Day", context.current_theme);
        
        // Phase 2: Sensor Animations (continues with Oil panel)
        runPhase2_SensorAnimations();
        TEST_ASSERT_TRUE(context.animations_completed);
        TEST_ASSERT_EQUAL(2047, context.last_pressure_value);
        
        // Phase 3: Trigger System (affects existing state)
        runPhase3_TriggerSystem();
        TEST_ASSERT_EQUAL_STRING("KeyPanel", context.current_panel);
        TEST_ASSERT_TRUE(context.triggers.key_active);
        TEST_ASSERT_TRUE(context.triggers.lock_active);
        
        // Phase 4: Error Handling (on top of triggers)
        runPhase4_ErrorHandling();
        TEST_ASSERT_EQUAL_STRING("ErrorPanel", context.current_panel);
        TEST_ASSERT_TRUE(context.errors_acknowledged);
        
        // Phase 5: Theme Changes (with restoration)
        runPhase5_ThemeChanges();
        TEST_ASSERT_EQUAL_STRING("OilPanel", context.current_panel);
        TEST_ASSERT_EQUAL_STRING("Night", context.current_theme);
        
        // Phase 6: Configuration (from current state)
        runPhase6_Configuration();
        TEST_ASSERT_TRUE(context.config_applied);
        TEST_ASSERT_EQUAL_STRING("Night", context.saved_theme);
        
        // Phase 7: Final Validation (complete system check)
        runPhase7_FinalValidation();
        TEST_ASSERT_TRUE(context.all_systems_operational);
        TEST_ASSERT_LESS_THAN(450000, millis() - context.test_start_ms);
        
        // Test passed - single session complete
        Serial.println("[ALL_TESTS_PASSED]");
    }
    
private:
    void runPhase1_SystemStartup() {
        Serial.println("[PHASE_1_START]");
        context.phase_start_ms[0] = millis();
        
        // Validate splash panel loads
        TEST_ASSERT_TRUE(waitForSerial("Splash panel loaded", 1000));
        
        // YAML simulates button press to skip animation
        // Unity validates the response
        TEST_ASSERT_TRUE(waitForSerial("Oil panel loaded", 5000));
        
        context.current_panel = "OilPanel";
        Serial.println("[PHASE_1_COMPLETE]");
    }
    
    // Phases 2-7 continue similarly...
};
```

### Individual Phase Scenarios

Each phase has its own YAML scenario that can be included:

#### Phase 1: Startup & Initial State
```yaml
# phase1-startup.yaml
name: 'Phase 1 - System Startup'
steps:
  # Wait for splash panel
  - wait-serial: 'Splash panel loaded'
  - delay: 3000ms  # Let animation start
  
  # Skip animation with short press
  - set-control:
      part-id: btn_action
      control: pressed
      value: 1
  - delay: 100ms
  - set-control:
      part-id: btn_action
      control: pressed
      value: 0
  
  # Confirm oil panel loads
  - wait-serial: 'Oil panel loaded'
  - wait-serial: 'Pressure animation started'
  - wait-serial: 'Temperature animation started'
```

#### Phase 3: Trigger System Testing
```yaml
# phase3-triggers.yaml
name: 'Phase 3 - Trigger System'
steps:
  # Test NORMAL priority (lights for theme)
  - set-control:
      part-id: btn_lights
      control: pressed
      value: 1
  - wait-serial: 'Theme changed to Night'
  
  # Test IMPORTANT priority (lock panel)
  - set-control:
      part-id: btn_lock
      control: pressed
      value: 1
  - wait-serial: 'Lock panel loaded'
  
  # Test CRITICAL priority (key panel)
  - set-control:
      part-id: btn_key
      control: pressed
      value: 1
  - wait-serial: 'Key panel loaded'
  
  # Test priority restoration
  - set-control:
      part-id: btn_key
      control: pressed
      value: 0
  - wait-serial: 'Lock panel restored'  # Lock still active
```

## CI/CD Integration

### GitHub Actions Workflow

```yaml
# .github/workflows/integration-tests.yml
name: Integration Tests

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  basic-test:
    name: Basic Hardware Validation
    runs-on: ubuntu-latest
    timeout-minutes: 5
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup PlatformIO
        uses: actions/setup-python@v4
        with:
          python-version: '3.10'
      - run: pip install platformio
      
      - name: Build Test Firmware
        run: pio run -e test-wokwi-basic
      
      - name: Run Basic Hardware Test
        uses: wokwi/wokwi-ci-action@v1
        with:
          token: ${{ secrets.WOKWI_CLI_TOKEN }}
          path: test/wokwi
          timeout: 10000
          scenario: scenarios/basic-hardware-test.yaml
          expect_text: '[ALL_TESTS_PASSED]'
          
  full-integration-test:
    name: Full System Integration (7 Phases)
    runs-on: ubuntu-latest
    timeout-minutes: 10
    if: github.event_name == 'pull_request'
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup PlatformIO
        uses: actions/setup-python@v4
        with:
          python-version: '3.10'
      - run: pip install platformio
      
      - name: Build Full Test Firmware
        run: pio run -e test-wokwi-full
      
      - name: Run Full Integration Test (Single Session)
        uses: wokwi/wokwi-ci-action@v1
        with:
          token: ${{ secrets.WOKWI_CLI_TOKEN }}
          path: test/wokwi
          timeout: 150000  # 2.5 minutes for entire 7-phase test
          scenario: scenarios/full-integration-test.yaml
          expect_text: '[ALL_TESTS_PASSED]'
          serial_log_file: test-output.log
          
      - name: Parse Test Results
        if: always()
        run: |
          echo "## Test Phase Summary" >> $GITHUB_STEP_SUMMARY
          grep "PHASE_.*_COMPLETE" test/wokwi/test-output.log | while read line; do
            echo "- ✅ $line" >> $GITHUB_STEP_SUMMARY
          done
          
      - name: Upload Test Artifacts
        if: always()
        uses: actions/upload-artifact@v3
        with:
          name: integration-test-results
          path: |
            test/wokwi/test-output.log
            test/wokwi/screenshots/*.png
```

### PlatformIO Configuration

```ini
# platformio.ini additions
[env:test-wokwi-basic]
extends = env:test-wokwi
build_flags = 
    ${env:test-wokwi.build_flags}
    -D TEST_BASIC_ONLY
    -D TEST_TIMEOUT_MS=10000
    -D ENABLE_TEST_MARKERS

[env:test-wokwi-full]
extends = env:test-wokwi
build_flags = 
    ${env:test-wokwi.build_flags}
    -D TEST_FULL_SUITE
    -D TEST_TIMEOUT_MS=120000
    -D ENABLE_TEST_MARKERS
    -D ENABLE_STATE_VALIDATION
    -D ENABLE_TIMING_CHECKS

[env:test-wokwi-performance]
extends = env:test-wokwi
build_flags = 
    ${env:test-wokwi.build_flags}
    -D TEST_PERFORMANCE
    -D MEASURE_TIMING
    -D STRICT_TIMING_CHECKS
    -D ENABLE_PROFILING
```

## Advanced Testing Features

### Visual Validation

Screenshot capture at critical points:

```yaml
# visual-validation.yaml
steps:
  - wait-serial: 'Oil panel loaded'
  - screenshot:
      part-id: lcd1
      filename: oil-panel-day.png
  - delay: 100ms
  
  - set-control:
      part-id: btn_lights
      control: pressed
      value: 1
  - wait-serial: 'Theme changed to Night'
  - screenshot:
      part-id: lcd1
      filename: oil-panel-night.png
```

### Performance Testing

Timing assertions for critical operations:

```cpp
void test_performance_metrics() {
    // Panel transition timing
    TestTiming::startMeasurement("panel_transition");
    triggerHandler->ActivateTrigger("lock");
    TEST_ASSERT_TRUE(waitForPanelLoad("Lock", 500));
    TestTiming::endMeasurement("panel_transition");
    TestTiming::assertTiming("panel_transition", 300, 50); // 300ms ±50ms
    
    // Animation performance
    TestTiming::startMeasurement("gauge_animation");
    oilPanel->StartPressureAnimation(0, 100);
    TEST_ASSERT_TRUE(waitForAnimationComplete(1000));
    TestTiming::endMeasurement("gauge_animation");
    TestTiming::assertTiming("gauge_animation", 750, 50); // 750ms ±50ms
}
```

### Memory Monitoring

Track memory usage across phases:

```cpp
void monitorMemoryUsage() {
    size_t free_heap_start = esp_get_free_heap_size();
    
    // Run phase
    runPhase();
    
    size_t free_heap_end = esp_get_free_heap_size();
    int heap_delta = free_heap_start - free_heap_end;
    
    // Assert no memory leaks
    TEST_ASSERT_LESS_THAN(100, abs(heap_delta));
}
```

## Implementation Roadmap

### Phase 1: Foundation (Week 1)
- [ ] Create serial protocol helper classes
- [ ] Implement basic YAML scenarios for hardware simulation
- [ ] Refactor existing Unity tests to emit markers
- [ ] Set up initial GitHub Actions workflow

### Phase 2: Core Testing (Week 2)
- [ ] Develop all 7 phase YAML scenarios
- [ ] Implement state validation framework
- [ ] Add timing and performance assertions
- [ ] Create test data fixtures and helpers

### Phase 3: Advanced Features (Week 3)
- [ ] Add visual validation with screenshot comparison
- [ ] Implement memory monitoring and leak detection
- [ ] Create parallel test execution for independent scenarios
- [ ] Add code coverage reporting integration

### Phase 4: Documentation & Deployment (Week 4)
- [ ] Write comprehensive test writing guidelines
- [ ] Document troubleshooting procedures
- [ ] Create test result dashboard
- [ ] Deploy to CI/CD pipeline

## Success Metrics

### Performance Targets
- **Basic Test Execution**: < 30 seconds
- **Full Test Execution**: < 2 minutes
- **CI/CD Feedback Time**: < 5 minutes from commit
- **Test Reliability**: < 1% flaky test rate

### Coverage Goals
- **Critical Path Coverage**: 100%
- **Overall Code Coverage**: > 80%
- **Hardware Simulation Coverage**: 100% of GPIO/ADC
- **State Transition Coverage**: 100% of defined transitions

### Maintainability Metrics
- **New Test Creation Time**: < 30 minutes
- **Test Debugging Time**: < 1 hour average
- **False Positive Rate**: < 0.5%
- **Test Documentation Coverage**: 100%

## Benefits

1. **Separation of Concerns**: Clear division between hardware simulation (YAML) and business logic (Unity)
2. **Single Session Testing**: Maintains realistic system behavior with state persistence
3. **Reusability**: Modular scenarios can be combined for different test configurations
4. **Maintainability**: Hardware changes don't require C++ code modifications
5. **Scalability**: Easy to add new test scenarios and phases
6. **CI/CD Integration**: Native support for GitHub Actions and other CI platforms
7. **Debugging**: Clear serial markers and structured logging for troubleshooting
8. **Performance**: Ability to run tests in parallel when appropriate
9. **Coverage**: Comprehensive validation including visual, timing, and state checks

## Conclusion

This integration test strategy provides a robust, maintainable, and scalable testing framework for the Clarity system. By maintaining the single-session, sequential execution model while introducing modern testing practices, we ensure both test fidelity and development efficiency. The hybrid approach leverages the best of both Wokwi's simulation capabilities and Unity's assertion framework, providing confidence in system behavior across all operational scenarios.