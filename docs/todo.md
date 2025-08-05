# TODO.md

This document tracks outstanding tasks, improvements, and feature requests for the Clarity project.

## High Priority

### Wokwi Integration Testing Plan

#### Current Wokwi Setup Analysis
- [x] Wokwi configuration uses ESP32 DevKit C v4 with ILI9341 display (square 320x240) 
- [x] Hardware mapping:
  - **Analog Sensors**: Pot1 ’ VP (GPIO36/Oil Pressure), Pot2 ’ VN (GPIO39/Oil Temperature)  
  - **Digital Triggers**: DIP Switch pins ’ GPIO 25/26/27/33 (Key Present/Not Present/Lock/Lights)
  - **Display**: ILI9341 ’ SPI pins (different from target GC9A01 round display)
- [x] Firmware path: `../../.pio/build/debug-local/firmware.bin` with GDB debugging support

#### Wokwi Integration Test Scenarios (Based on scenario.md)

**Test Suite 1: Basic System Startup**
- [ ] **Test 1.1**: Default startup sequence
  - Verify splash screen animation with day theme
  - Confirm transition to oil panel with default sensor values
  - Validate needle positioning and scale rendering

**Test Suite 2: Oil Monitoring Panel Tests**  
- [ ] **Test 2.1**: Oil panel with sensor data
  - Set Pot1 (pressure) and Pot2 (temperature) to halfway positions
  - Verify gauge needle animations and positioning
  - Test day theme styling (white scale ticks and icons)
- [ ] **Test 2.2**: Oil sensor value changes
  - Dynamically adjust potentiometer values during runtime
  - Verify smooth needle transitions and accurate readings
  - Test boundary conditions (min/max sensor values)

**Test Suite 3: Theme Switching Tests**
- [ ] **Test 3.1**: Day/Night theme transitions
  - Toggle DIP switch pin 4 (GPIO33/Lights) during oil panel display
  - Verify theme change without panel reload (red’white scale ticks)
  - Confirm theme persistence across panel switches
- [ ] **Test 3.2**: Startup with night theme active
  - Begin simulation with Lights trigger HIGH
  - Verify splash screen and oil panel use night theme from start

**Test Suite 4: Key Panel Integration Tests**
- [ ] **Test 4.1**: Key present scenario
  - Toggle DIP switch pin 1 (GPIO25/Key Present) HIGH during runtime
  - Verify panel switch to key panel with green key icon
  - Test return to oil panel when trigger goes LOW
- [ ] **Test 4.2**: Key not present scenario  
  - Toggle DIP switch pin 2 (GPIO26/Key Not Present) HIGH
  - Verify panel switch to key panel with red key icon
  - Test priority over oil panel display
- [ ] **Test 4.3**: Invalid key state handling
  - Set both Key Present AND Key Not Present triggers HIGH simultaneously
  - Verify system handles conflicting states gracefully
  - Document expected behavior for invalid states

**Test Suite 5: Lock Panel Integration Tests**
- [ ] **Test 5.1**: Lock activation during runtime
  - Toggle DIP switch pin 3 (GPIO27/Lock) HIGH while in oil panel
  - Verify immediate switch to lock panel display
  - Test return to previous panel when lock deactivates
- [ ] **Test 5.2**: Lock priority testing
  - Activate lock trigger while key panel is displayed
  - Verify lock panel takes precedence
  - Test lock deactivation returns to appropriate panel based on other triggers

**Test Suite 6: Complex Multi-Trigger Scenarios (Major Scenario)**
- [ ] **Test 6.1**: Complete system integration test
  - Implement the full scenario from scenario.md:
    1. Start with oil sensors at halfway ’ Oil panel loads
    2. Lights trigger HIGH ’ Theme changes to night (no reload)
    3. Lock trigger HIGH ’ Lock panel loads  
    4. Key not present HIGH ’ Key panel (red icon)
    5. Key present HIGH ’ Lock panel (invalid state handling)
    6. Key not present LOW ’ Key panel (green icon)
    7. Key present LOW ’ Lock panel
    8. Lock trigger LOW ’ Oil panel with night theme
    9. Lights trigger LOW ’ Theme changes to day
- [ ] **Test 6.2**: Trigger priority validation
  - Test various combinations of simultaneous triggers
  - Verify panel switching follows documented priority order
  - Document any edge cases or unexpected behaviors

**Test Suite 7: Performance and Stability Tests**
- [ ] **Test 7.1**: Rapid trigger switching
  - Rapidly toggle multiple triggers in sequence
  - Monitor for memory leaks, crashes, or display artifacts
  - Verify smooth transitions under stress conditions
- [ ] **Test 7.2**: Long-running stability test  
  - Run simulation for extended periods with periodic trigger changes
  - Monitor memory usage and system stability
  - Test sensor value changes over time

#### Wokwi Test Implementation Strategy

**Phase 1: Individual Component Testing**
- [ ] Create separate wokwi scenarios for each trigger type
- [ ] Validate sensor reading accuracy and ADC conversion
- [ ] Test individual panel rendering and animations

**Phase 2: Integration Testing**
- [ ] Implement multi-trigger scenario testing
- [ ] Validate trigger priority and panel switching logic
- [ ] Test theme switching across all panels

**Phase 3: System Validation**
- [ ] Run complete scenario.md test cases in wokwi
- [ ] Compare emulated behavior with unit test expectations  
- [ ] Document any discrepancies between emulated and target hardware

**Phase 4: Automated Testing Integration**
- [ ] Investigate wokwi CLI for automated test execution
- [ ] Consider integrating wokwi tests into CI/CD pipeline
- [ ] Create test result validation and reporting

#### Hardware Mapping Documentation
- [ ] **Test Harness Setup Guide**: Document DIP switch positions for each trigger
- [ ] **Sensor Simulation Guide**: Document potentiometer ranges and expected gauge responses
- [ ] **Debug Workflow**: Document GDB debugging setup and common debugging scenarios

### Known Issues
- [ ] Fix wokwi display limitation: Square display renders content inverted horizontally (mentioned in CLAUDE.md)
- [ ] Display mismatch: Target hardware uses round GC9A01 display vs wokwi's square ILI9341
- [ ] PlatformIO Unity test limitations:
  - [ ] Build filters not working with multiple test files (linking conflicts)
  - [ ] Nested directory test files not discovered (only root `/test/` works)

### Testing Infrastructure
- [ ] Expand integration test coverage for complete system scenarios
- [ ] Add performance benchmarking for LVGL rendering operations
- [ ] Create hardware-in-the-loop test setup for physical device validation

## Medium Priority

### Architecture Enhancements
- [ ] Consider implementing dependency injection container improvements
- [ ] Evaluate adding event-driven architecture for sensor state changes
- [ ] Review memory usage optimization for constrained ESP32 environment

### UI/UX Improvements
- [ ] Implement smooth transitions between panels (fade/slide animations)
- [ ] Add visual feedback for sensor connection status
- [ ] Consider adding configuration mode for sensor calibration

### Documentation
- [ ] Add comprehensive API documentation for all interfaces
- [ ] Create detailed hardware setup guide with wiring diagrams
- [ ] Document performance characteristics and memory usage

## Low Priority

### Feature Requests
- [ ] Add data logging capability (SD card or external storage)
- [ ] Implement wireless configuration via WiFi access point
- [ ] Add support for additional sensor types (CAN bus, etc.)
- [ ] Consider adding diagnostic mode for troubleshooting

### Code Quality
- [ ] Run static analysis tools (clang-tidy, cppcheck)
- [ ] Add code coverage reporting to CI pipeline
- [ ] Consider refactoring large functions for better maintainability

### Build System
- [ ] Investigate PlatformIO library dependency optimization
- [ ] Add automated firmware signing for production builds
- [ ] Create automated release pipeline with GitHub Actions

## Completed
- [x] Implement MVP architecture with clear separation of concerns
- [x] Create comprehensive unit test suite (100 tests)
- [x] Set up CI/CD pipeline with GitHub Actions
- [x] Implement dynamic panel switching based on sensor triggers
- [x] Add theme switching (day/night modes)
- [x] Analyze existing wokwi configuration and hardware mapping
- [x] Plan comprehensive wokwi integration test scenarios based on documented scenarios