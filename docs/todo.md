# TODO:
## 1. Architecture Documentation

Create an architecture md document in the docs directory and link it in this README under the existing **Architecture** section.

### Include:
- **Patterns**
  - MVP Relationships
  - Layer Structure
- **Roles and Responsibilities**
- **Logic Flow**
- **Dynamic Panel Switching**
  - Based on sensor input
- **Trigger System Interrupts**
- **Panels**
- **Components**
- **Hardware Configuration**
  - GPIO Pins
  - Display/LCD Interface (SPI)
  - Sensor Inputs (Digital + Analog)

---

## 2. Scenario Documentation

Create a scenario md document in the docs directory and link it in this README under the existing **Scenario** section.

### Major Scenario (Full System Test)

> App starts with pressure and temperature values set to halfway  
> → Splash animates with day theme (white text)  
> → Oil panel loads with day theme (white scale ticks and icon)  
> → Oil panel needles animate  
> → Lights trigger high  
> → Oil panel does NOT reload, theme changes to night (red scale ticks and icon)  
> → Lock trigger high  
> → Lock panel loads  
> → Key not present trigger high  
> → Key panel loads (present = false → red icon)  
> → Key present trigger high  
> → Lock panel loads (both key present and key not present true = invalid state)  
> → Key not present trigger low  
> → Key panel loads (present = true → green icon, key present trigger still high)  
> → Key present trigger low  
> → Lock panel loads (lock trigger still high)  
> → Lock trigger low  
> → Oil panel loads with night theme (red scale ticks and icon, lights trigger still active)  
> → Oil panel needles animate  
> → Lights trigger low  
> → Oil panel does NOT reload, theme changes to day (white scale ticks and icon)

### Individual Test Scenarios

- App starts → Splash animates with day theme (white text)
- App starts with pressure and temperature values set to halfway  
  → Splash animates with day theme (white text)  
  → Oil panel loads with day theme (white scale ticks and icon)  
  → Oil panel needles animate
- App starts  
  → Splash animates with day theme (white text)  
  → Oil panel loads with day theme  
  → Key present trigger high → Key panel loads (green icon)  
  → Key present trigger low → Oil panel loads
- App starts with key present trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Key panel loads (green icon)  
  → Key present trigger low → Oil panel loads
- App starts  
  → Splash animates with day theme  
  → Oil panel loads  
  → Key not present trigger high → Key panel loads (red icon)  
  → Key not present trigger low → Oil panel loads
- App starts with key not present trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Key panel loads (red icon)  
  → Key not present trigger low → Oil panel loads
- App starts  
  → Splash animates  
  → Oil panel loads  
  → Lock trigger high → Lock panel loads  
  → Lock trigger low → Oil panel loads
- App starts with lock trigger high  
  → Splash animates  
  → Oil panel does NOT load  
  → Lock panel loads  
  → Lock trigger low → Oil panel loads
- App starts  
  → Splash animates  
  → Oil panel loads  
  → Lights trigger high → Theme changes to night (no reload)  
  → Lights trigger low → Theme changes to day (no reload)
- App starts with lights trigger high  
  → Splash animates with night theme (red text)  
  → Oil panel loads with night theme  
  → Lights trigger low → Theme changes to day (no reload)

---

## 3. Unit Tests

**Status**: ✅ Basic framework implemented (12 tests), ❌ Comprehensive coverage pending

Current implementation provides basic unit testing foundation, but comprehensive testing requires significant infrastructure investment.

### Current Implementation
- **12 basic logic tests** for core algorithms
- **Unity framework** integration with PlatformIO
- **Basic mocks** for Arduino/ESP32 functions
- **Test runner** with coverage reporting setup

### Comprehensive Testing Requirements

To achieve full code coverage for **27 source files** (~80-120 test cases):

#### 1. Extensive Mock Infrastructure
**ESP32/Arduino Ecosystem Mocks**:
- Complete `Arduino.h` mock with all hardware functions
- `esp32-hal-log.h` mock for logging macros
- `Preferences.h` mock for NVS storage simulation
- `nvs_flash.h` mock for flash operations
- ADC/GPIO hardware simulation with state management

**LVGL UI Library Mocks** (Most Complex):
- `lv_obj_t` and UI object hierarchy
- `lv_style_t` and styling system
- `lv_color_t` and color management
- Layout and positioning system (`lv_coord_t`, `lv_align_t`)
- Event handling and timer system
- Screen and widget lifecycle management

**ArduinoJson Mocks**:
- Complete `JsonDocument` implementation
- Serialization/deserialization functions
- Error handling and validation

#### 2. Dedicated Test Infrastructure
**Mock Service Implementations**:
```cpp
// Complete interface implementations for testing
class MockPanelService : public IPanelService
class MockStyleService : public IStyleService  
class MockTriggerService : public ITriggerService
class MockDisplayProvider : public IDisplayProvider
```

**Test Fixtures and Utilities**:
- Sensor state simulation framework
- GPIO pin state management
- Configuration persistence testing
- Memory leak detection
- Performance benchmarking utilities

**Build System Integration**:
- Separate test-only source filtering
- Coverage report generation (gcov/lcov)
- CI/CD integration with test results
- Automated regression testing

#### 3. Test Categories Implementation
**Manager Tests** (4 files, ~20 tests):
- PreferenceManager: Config persistence, JSON handling, NVS operations
- StyleManager: Theme management, LVGL integration, style inheritance
- PanelManager: Panel lifecycle, switching logic, state management  
- TriggerManager: Event handling, priority management, callback execution

**Sensor Tests** (5 files, ~25 tests):
- All sensors: GPIO interaction, value change detection, calibration
- Analog sensors: ADC conversion, filtering, boundary conditions
- Digital sensors: State machine logic, debouncing, error states

**Component Tests** (6 files, ~30 tests):
- UI components: LVGL integration, rendering, state updates
- Layout management, event handling, theme application

**System Tests** (8+ files, ~25 tests):
- ServiceContainer: Dependency injection, lifecycle management
- Providers: Hardware abstraction, interface compliance
- Factories: Object creation, configuration injection

#### 4. Advanced Testing Features
**Hardware-in-the-Loop Testing**:
- ESP32 simulator integration
- Real GPIO state validation  
- Timing-sensitive operation testing

**Integration Test Framework**:
- Full scenario execution (from point 2)
- End-to-end workflow validation
- Performance and memory profiling

**Coverage and Quality Metrics**:
- Line coverage: Target 85%+
- Branch coverage: Target 80%+
- Function coverage: Target 95%+
- Mutation testing for test quality validation

### Implementation Effort Estimate
- **Mock Infrastructure**: 2-3 weeks full-time development
- **Test Cases**: 3-4 weeks comprehensive test implementation  
- **CI/CD Integration**: 1 week automation setup
- **Documentation**: 1 week test documentation and maintenance guides

### Architecture Validation
✅ **Confirmed**: The codebase architecture excellently supports comprehensive testing through:
- Dependency injection pattern enabling easy mocking
- Interface abstractions decoupling hardware dependencies  
- Service container managing component lifecycle
- MVP pattern separating business logic from UI concerns

The 12 current tests demonstrate the framework works; scaling to comprehensive coverage requires the infrastructure investment outlined above.

---

## 4. Integration Tests

Create integration tests to validate full scenario flows.  
> Is it possible to do so to prove scenarios?

---

## 5. Wokwi Tests

Create Wokwi-based simulations for the documented scenarios.

## 6. General

* Night theme (triggered by lights) is not changing the background of the screen to fully black, only the central pivot is turning full black
