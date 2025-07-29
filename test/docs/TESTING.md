# Clarity ESP32 Testing Methodology

Comprehensive testing strategy and methodology for the Clarity ESP32 digital gauge system trigger optimization.

## Testing Philosophy

This test suite follows a **behavior-driven development (BDD)** approach, focusing on validating system behavior against documented scenarios rather than implementation details. The methodology emphasizes:

- **Scenario-based testing**: All tests implement real-world usage scenarios
- **Hardware abstraction**: Mock hardware eliminates ESP32 dependencies
- **Performance validation**: Response time and memory usage are first-class concerns
- **Integration focus**: System-level behavior validation over unit isolation

## Test Strategy Overview

### 1. Hierarchical Test Structure

```
System Integration Tests (End-to-End)
├── Component Integration Tests
│   ├── Trigger System Tests (Unit)
│   ├── Panel Manager Tests (Unit)
│   └── Sensor Tests (Unit)
└── Mock Hardware Abstraction Layer
```

### 2. Test Categories by Purpose

#### **Functional Tests** (80% of test suite)
- Validate correct behavior for all documented scenarios
- Verify state transitions and system responses
- Ensure trigger priority and FIFO logic implementation

#### **Performance Tests** (15% of test suite)  
- Response time measurement under various loads
- Memory usage validation and leak detection
- Stress testing with high-frequency events

#### **Edge Case Tests** (5% of test suite)
- Boundary condition validation
- Error recovery and fault tolerance
- Invalid input handling

### 3. Coverage Metrics

#### **Scenario Coverage**: 100%
All scenarios from `docs/scenarios.md` are implemented:
- System Startup: S1.1-S1.5 (5 scenarios)
- Single Triggers: S2.1-S2.4 (4 scenarios)  
- Multiple Triggers: S3.1-S3.5 (5 scenarios)
- Edge Cases: S4.1-S4.5 (5 scenarios)
- Performance: S5.1-S5.3 (3 scenarios)

#### **Component Coverage**: 100%
- Trigger system (all GPIO pins and logic paths)
- Panel manager (all panel types and transitions)
- Sensor system (ADC channels and conversion logic)
- Integration points (trigger→panel, sensor→display)

#### **State Coverage**: 100%
- All valid system states (panel + theme combinations)
- All trigger activation combinations
- All restoration chain scenarios

## Mock Hardware Architecture

### Design Principles

1. **Behavioral Fidelity**: Mock hardware reproduces real ESP32 behavior
2. **State Isolation**: Each test starts with clean mock state
3. **Deterministic Results**: No random or time-dependent behavior
4. **Performance Simulation**: Timing characteristics match real hardware

### MockHardware Implementation

```cpp
class MockHardware {
    // GPIO simulation with 40-pin array matching ESP32
    static bool mock_gpio_states[40];
    
    // ADC simulation with 12-bit resolution (0-4095)
    static uint16_t mock_adc_readings[40];
    
public:
    // State management
    static void reset();                    // Clean slate for each test
    
    // GPIO simulation  
    static void setGpioState(uint8_t pin, bool state);
    static bool getGpioState(uint8_t pin);
    
    // ADC simulation
    static void simulateAdcReading(uint8_t pin, uint16_t value);
    static uint16_t getAdcReading(uint8_t pin);
};
```

### Hardware Mapping

| Component | Real Hardware | Mock Hardware |
|-----------|---------------|---------------|
| Key Present | GPIO 25 | MockHardware pin 25 |
| Key Not Present | GPIO 26 | MockHardware pin 26 |
| Lock State | GPIO 27 | MockHardware pin 27 |
| Lights State | GPIO 28 | MockHardware pin 28 |
| Oil Pressure | ADC1_CH6 (GPIO 34) | MockHardware ADC 34 |
| Oil Temperature | ADC1_CH7 (GPIO 35) | MockHardware ADC 35 |

## Test Execution Methodology

### 1. Test Lifecycle

#### Setup Phase
```cpp
void setUp(void) {
    MockHardware::reset();           // Clean hardware state
    // Initialize test-specific state
}
```

#### Execution Phase
```cpp
void test_scenario_implementation(void) {
    TriggerScenarioTest test;
    test.SetupScenario("Descriptive name");
    
    // Apply scenario events
    auto events = TestScenarios::specificScenario();
    test.ApplyTriggerSequence(events);
    
    // Validate expected state
    test.ValidateExpectedState(ExpectedStates::EXPECTED_STATE);
}
```

#### Validation Phase
- **State Verification**: Panel, theme, and trigger states
- **Performance Metrics**: Response time and memory usage
- **Error Detection**: Assertion failures and timeout conditions

#### Teardown Phase
```cpp
void tearDown(void) {
    // Cleanup resources
    // Log test results
}
```

### 2. Scenario-Based Test Design

#### Event-Driven Testing
Tests simulate real-world trigger events:
```cpp
struct TriggerEvent {
    const char* triggerId;      // "key_present", "lock_state", etc.
    bool pinState;              // true = active, false = inactive
    uint32_t timestamp;         // Relative timing (ms)
};
```

#### State Validation Framework
```cpp
struct ExpectedState {
    const char* expectedPanel;   // "OemOilPanel", "KeyPanel", etc.
    const char* expectedTheme;   // "Day", "Night"
    std::vector<const char*> activeTriggers;  // List of active triggers
};
```

## Performance Testing Methodology

### Response Time Measurement
```cpp
void measureResponseTime(std::function<void()> operation) {
    auto start = std::chrono::high_resolution_clock::now();
    operation();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // Store/validate duration
}
```

### Memory Usage Tracking
- **Static Analysis**: Compile-time memory footprint
- **Dynamic Monitoring**: Runtime heap usage patterns  
- **Leak Detection**: Memory usage over extended test runs
- **Stack Analysis**: Maximum stack depth during operations

### Stress Testing Parameters
- **High Frequency Events**: 100+ trigger events per second
- **Extended Operation**: 1000+ state transitions
- **Rapid Switching**: Panel changes every 10ms
- **Memory Pressure**: Large iteration counts

## Quality Assurance Process

### Test Development Guidelines

#### Test Naming Convention
- **Function names**: `test_S<section>_<number>_<description>`
- **Example**: `test_S3_1_priority_override_key_over_lock`
- **Scenario reference**: Must map to `docs/scenarios.md`

#### Test Structure Requirements
- **Setup**: Clear scenario description and initial state
- **Execution**: Event sequence application
- **Validation**: Expected state verification
- **Cleanup**: Resource deallocation

#### Assertion Strategy
- **Specific assertions**: Use targeted test macros
- **Clear failure messages**: Include context in assertion messages
- **State validation**: Verify all relevant system state components

## Common Test Failures & Solutions

### Mock Hardware State Issues
**Symptom**: Unexpected trigger states
**Solution**: Verify `MockHardware::reset()` in `setUp()`

### Timing-Related Failures
**Symptom**: Inconsistent test results
**Solution**: Use deterministic timing, avoid real-time dependencies

### State Validation Failures
**Symptom**: Expected state mismatch
**Solution**: Add intermediate state checks and detailed logging

## Performance Baselines

### Response Time Targets
- **Trigger processing**: < 1ms
- **Panel switching**: < 5ms
- **Sensor reading**: < 2ms
- **Complete scenario**: < 10ms

### Memory Usage Targets
- **Static memory**: < 50KB
- **Dynamic memory**: < 10KB peak
- **Memory leaks**: 0 bytes over 1000 iterations
- **Stack usage**: < 2KB maximum depth

## Test Environment Setup

### Prerequisites
```bash
# Install dependencies
sudo apt-get install libunity-dev build-essential

# Verify compiler
g++ --version  # Minimum 7.0 for C++17

# Install PlatformIO
pip install platformio
```

### Quick Validation
```bash
cd test && ./run_quick_tests.sh
```

## References

- **Scenario Documentation**: `../docs/scenarios.md`
- **Test Coverage**: See README.md in this directory
- **Unity Framework**: https://github.com/ThrowTheSwitch/Unity
- **PlatformIO Testing**: https://docs.platformio.org/en/latest/advanced/unit-testing/