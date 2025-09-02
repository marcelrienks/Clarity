# Realistic Testing Strategy

This document provides a practical testing approach that works within the documented platform limitations of PlatformIO and ESP32 constraints.

## Platform Limitations (Documented)

### PlatformIO Unity Framework Issues
- **Build Filters Don't Work**: Unity test framework ignores build filters, attempts to link all test files together
- **Nested Directory Limitation**: Test files within nested directories are not found, only tests in root test directory are run
- **Multiple Test Files**: Cannot have multiple test files as they will try to link together and cause conflicts

### Wokwi Emulator Limitations  
- **Square Display**: Uses square ILI9341 instead of round GC9A01 display
- **Horizontal Inversion**: Images render inverted horizontally
- **Limited Hardware Simulation**: Cannot fully simulate all automotive input conditions

### ESP32 Memory Constraints
- **Limited RAM**: ~250KB available after OTA partitioning affects test complexity
- **Single Core**: LVGL operations cannot be interrupted, limiting test scenarios
- **Flash Storage**: Tests must fit within flash constraints for embedded execution

## Realistic Testing Approach

### 1. Single Test File Strategy

Given PlatformIO Unity limitations, use a single comprehensive test file:

**Structure**:
```
test/
└── test_clarity_system.cpp    # Single comprehensive test file
```

**Content Organization**:
```cpp
// test/test_clarity_system.cpp
#include <unity.h>

// Test categories grouped within single file
void test_sensor_functionality() { /* GPIO sensor tests */ }
void test_interrupt_system() { /* Interrupt registration/execution */ } 
void test_panel_management() { /* Panel creation/switching */ }
void test_memory_management() { /* Memory usage validation */ }
void test_error_handling() { /* Error system integration */ }

void setup() {
    UNITY_BEGIN();
    
    // GPIO/Sensor Tests
    RUN_TEST(test_sensor_functionality);
    
    // Interrupt System Tests  
    RUN_TEST(test_interrupt_system);
    
    // Panel Management Tests
    RUN_TEST(test_panel_management);
    
    // Memory Tests
    RUN_TEST(test_memory_management);
    
    // Error Handling Tests
    RUN_TEST(test_error_handling);
    
    UNITY_END();
}

void loop() {}
```

### 2. Mock-Based Testing Strategy

**Hardware Abstraction Testing**:
- Create mock implementations of provider interfaces
- Test business logic without hardware dependencies
- Validate interface contracts and error handling

**Mock Provider Implementation**:
```cpp
class MockGpioProvider : public IGpioProvider {
private:
    std::map<int, bool> pinStates_;
    
public:
    void SetPinState(int pin, bool state) { pinStates_[pin] = state; }
    bool DigitalRead(int pin) override { return pinStates_[pin]; }
    void PinMode(int pin, uint8_t mode) override { /* mock */ }
};
```

### 3. Component-Level Testing

**Individual Component Validation**:
- Test sensor change detection logic
- Test interrupt registration and execution  
- Test panel creation and lifecycle
- Test manager singleton behavior

**Example Test Structure**:
```cpp
void test_sensor_change_detection() {
    MockGpioProvider mockGpio;
    KeyPresentSensor sensor(&mockGpio);
    
    // Test initial state (no change)
    mockGpio.SetPinState(25, false);
    TEST_ASSERT_FALSE(sensor.HasStateChanged());
    
    // Test state change detection
    mockGpio.SetPinState(25, true);
    TEST_ASSERT_TRUE(sensor.HasStateChanged());
    
    // Test no change on subsequent reads
    TEST_ASSERT_FALSE(sensor.HasStateChanged());
}
```

### 4. Integration Testing with Wokwi

**Wokwi-Specific Testing**:
- Focus on system integration testing
- Test user input flows (button sequences)
- Validate display updates (within emulator limitations)
- Test timing-sensitive operations

**Wokwi Test Setup**:
```
1. Configure DIP switches for GPIO inputs
2. Use potentiometers for ADC sensor simulation  
3. Monitor serial output for system state validation
4. Test button timing detection (short/long press)
```

**Wokwi Test Scenarios**:
- Startup sequence validation
- Panel switching via GPIO state changes
- Button navigation testing
- Error condition simulation
- Memory usage monitoring via serial output

### 5. Memory Profiling Strategy

**ESP32 Memory Monitoring**:
```cpp
void test_memory_usage() {
    size_t initial_free = ESP.getFreeHeap();
    size_t initial_largest = ESP.getMaxAllocHeap();
    
    // Create system components
    auto interrupt_manager = InterruptManager::Instance();
    
    size_t after_system = ESP.getFreeHeap();
    
    // Validate memory usage within expected bounds
    size_t system_memory = initial_free - after_system;
    TEST_ASSERT_LESS_THAN(5000, system_memory); // Less than 5KB for core system
    
    Serial.printf("System Memory Usage: %d bytes\n", system_memory);
    Serial.printf("Free Heap: %d bytes\n", after_system);
    Serial.printf("Largest Free Block: %d bytes\n", ESP.getMaxAllocHeap());
}
```

**Memory Leak Detection**:
```cpp
void test_memory_stability() {
    size_t baseline = ESP.getFreeHeap();
    
    // Perform multiple panel switches
    for(int i = 0; i < 10; i++) {
        PanelManager::Instance().CreateAndLoadPanel(PanelNames::OIL, false);
        PanelManager::Instance().CreateAndLoadPanel(PanelNames::KEY, false);  
        delay(100); // Allow cleanup
    }
    
    size_t after_cycles = ESP.getFreeHeap();
    
    // Memory should be stable (within small tolerance for cleanup delays)
    TEST_ASSERT_INT_WITHIN(100, baseline, after_cycles);
}
```

### 6. Error Condition Testing

**Error System Validation**:
```cpp
void test_error_handling() {
    ErrorManager& errorMgr = ErrorManager::Instance();
    
    // Test error reporting
    errorMgr.ReportError(ErrorLevel::ERROR, "TEST", "Test error condition");
    
    // Validate error was recorded
    TEST_ASSERT_TRUE(errorMgr.HasErrors());
    
    // Test error panel activation (if implemented)
    // ... panel switching validation
    
    // Test error clearing
    errorMgr.ClearErrors();
    TEST_ASSERT_FALSE(errorMgr.HasErrors());
}
```

### 7. Performance Testing

**Response Time Validation**:
```cpp
void test_button_responsiveness() {
    unsigned long start_time = millis();
    
    // Simulate button press
    // ... trigger button sensor
    
    // Measure response time
    while(!panel_switched && (millis() - start_time) < 200) {
        // Process interrupt system
        InterruptManager::Instance().Process();
        delay(1);
    }
    
    unsigned long response_time = millis() - start_time;
    TEST_ASSERT_LESS_THAN(100, response_time); // Less than 100ms response
}
```

## Practical Test Execution

### 1. Development Workflow
```bash
# Run tests locally
pio test -e debug-local

# Monitor memory usage
pio device monitor --baud 115200

# Run with Wokwi emulation  
pio test -e wokwi
```

### 2. Continuous Integration Approach
**Limited CI**: Given platform constraints, focus on:
- Code compilation validation
- Static analysis (cppcheck, clang-tidy)
- Documentation consistency checks
- Basic unit test execution

### 3. Manual Testing Checklist

**Hardware Integration Testing** (when hardware available):
- [ ] All GPIO pins respond correctly
- [ ] Display shows correct visuals on round GC9A01
- [ ] Button timing detection accurate
- [ ] Automotive power supply compatibility
- [ ] Temperature/pressure sensor accuracy
- [ ] Long-term stability testing

**System Integration Testing**:
- [ ] Startup sequence completes successfully
- [ ] All panels load and display correctly  
- [ ] Panel switching works via GPIO inputs
- [ ] Button navigation functions
- [ ] Theme switching responds to light sensor
- [ ] Error conditions are handled gracefully
- [ ] System recovers from error states

## Test Data Validation

### Memory Usage Validation
Track and validate the memory estimates provided in the requirements:

**Expected Measurements**:
- Core system: 1,000-1,100 bytes
- LVGL buffers: 28.8KB-57.6KB (configurable)
- Total system: 30-59KB

**Validation Approach**:
```cpp
void validate_memory_estimates() {
    // Measure before system init
    size_t pre_init = ESP.getFreeHeap();
    
    // Initialize full system
    InitializeSystemComponents();
    
    // Measure after system init
    size_t post_init = ESP.getFreeHeap(); 
    
    size_t system_usage = pre_init - post_init;
    
    Serial.printf("Actual System Memory Usage: %d bytes\n", system_usage);
    
    // Validate against documented estimates
    TEST_ASSERT_INT_WITHIN(5000, 35000, system_usage); // 30-40KB estimate range
}
```

## Testing Limitations and Workarounds

### Limitations We Accept
1. **Single Test File**: Cannot have multiple test files due to PlatformIO Unity issues
2. **Emulator Differences**: Wokwi display differences require visual validation on real hardware
3. **Limited CI**: Cannot do full hardware-in-loop testing in CI environment

### Practical Workarounds
1. **Well-Organized Single Test File**: Use clear test grouping and naming
2. **Mock-Heavy Testing**: Abstract hardware dependencies for reliable testing
3. **Serial Output Validation**: Use serial monitoring for system state validation
4. **Manual Hardware Validation**: Maintain manual testing checklist for hardware-specific testing

## Test Coverage Goals

**Achievable Coverage**:
- **Core Logic**: 80%+ coverage of interrupt system, panel management, sensor logic
- **Hardware Interfaces**: Mock-based testing of all provider interfaces  
- **Error Handling**: All error paths tested with mock failures
- **Memory Management**: Memory usage and stability validation
- **Integration**: End-to-end workflow testing via Wokwi

**Limited Coverage** (Hardware Required):
- Physical GPIO behavior validation
- Actual display output verification on round GC9A01
- Real automotive sensor integration
- Power supply and environmental testing

## Conclusion

This testing strategy acknowledges platform limitations while providing comprehensive testing within realistic constraints. The approach focuses on:

1. **Testable Architecture**: Using mocks and interfaces for reliable testing
2. **Single File Strategy**: Working within PlatformIO Unity constraints  
3. **Memory Validation**: Proving the documented memory usage claims
4. **Integration Focus**: Testing complete workflows rather than isolated units
5. **Manual Validation**: Clear checklist for hardware-dependent testing

This provides a practical path forward for validating system functionality without getting blocked by platform limitations.