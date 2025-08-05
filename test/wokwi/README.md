# Wokwi Integration Testing Suite

This directory contains comprehensive integration tests for the Clarity digital gauge system using Wokwi emulation.

## Test Overview

The Wokwi integration tests validate complete system behavior including sensor readings, trigger management, panel switching, theme changes, and hardware interactions in an emulated environment.

## Hardware Emulation Mapping

### ESP32 DevKit C v4
- **Display**: ILI9341 320x240 (square vs target GC9A01 round)
- **Analog Sensors**: 
  - Pot1 → VP (GPIO36) → Oil Pressure Sensor
  - Pot2 → VN (GPIO39) → Oil Temperature Sensor
- **Digital Triggers**:
  - DIP Switch #1 → GPIO25 → Key Present
  - DIP Switch #2 → GPIO26 → Key Not Present  
  - DIP Switch #3 → GPIO27 → Lock State
  - DIP Switch #4 → GPIO33 → Lights (Theme Control)

### Potentiometer Value Mapping
- **0-1023 Range**: Maps to full sensor ranges
- **Pressure**: 0 = 0 Bar, 1023 = 10 Bar
- **Temperature**: 0 = 0°C, 1023 = 120°C
- **Halfway (~512)**: ~5 Bar pressure, ~60°C temperature

## Test Scenarios

### Phase 1: Individual Component Testing

#### [basic_startup](basic_startup/)
**Objective**: Verify default startup sequence and oil panel loading
- Tests splash screen animation with day theme
- Validates transition to oil panel with minimum sensor values
- Confirms basic system initialization

#### [oil_panel_sensors](oil_panel_sensors/) 
**Objective**: Test oil panel with sensor data and gauge animations
- Tests gauge needle animations with halfway sensor values
- Validates real-time sensor response to potentiometer changes
- Verifies accurate ADC to display value mapping

#### [theme_switching](theme_switching/)
**Objective**: Validate day/night theme switching without panel reload
- Tests lights trigger (DIP Switch #4) functionality
- Verifies theme changes occur without panel interruption
- Validates color changes (white ↔ red) for scale ticks and icons

### Phase 2: Panel Switching Tests

#### [key_present](key_present/)
**Objective**: Test key present trigger and green key icon display  
- Tests key present trigger (DIP Switch #1) functionality
- Validates panel switching from Oil → Key → Oil
- Verifies green key icon display for present state

#### [key_not_present](key_not_present/)
**Objective**: Test key not present trigger and red key icon display
- Tests key not present trigger (DIP Switch #2) functionality  
- Validates red key icon display for not present state
- Tests trigger priority over oil panel

### Phase 3: System Integration

#### [major_scenario](major_scenario/)
**Objective**: Complete system integration test (from scenario.md)
- Executes the comprehensive 9-step test sequence
- Tests all triggers, panel switching, and theme changes
- Validates trigger priorities and conflict resolution
- Tests theme persistence across panel switches

## Running Tests

### Prerequisites
1. **Build Firmware**: Ensure `debug-local` environment is built
   ```bash
   pio run -e debug-local
   ```

2. **Wokwi Access**: Tests require Wokwi simulator access
   - Online: [wokwi.com](https://wokwi.com)
   - VS Code: Wokwi extension

### Individual Test Execution
1. Navigate to desired test directory (e.g., `basic_startup/`)
2. Open `diagram.json` in Wokwi (uses shared `wokwi.toml` from root directory)
3. Follow test steps in the `README.md`
4. Verify pass criteria and expected behaviors

### Test Execution Order
**Recommended sequence for comprehensive validation:**
1. `basic_startup` - Verify basic functionality
2. `oil_panel_sensors` - Test sensor integration  
3. `theme_switching` - Validate theme system
4. `key_present` - Test key panel functionality
5. `key_not_present` - Test key absence functionality
6. `major_scenario` - Complete system integration

## Expected Results

### Serial Monitor Output
Each test includes expected serial output patterns for verification:
- Trigger state changes
- Panel switching messages  
- Theme change notifications
- Sensor reading updates

### Visual Verification  
Key visual checkpoints:
- **Splash Screen**: White text (day) or red text (night)
- **Oil Panel**: Gauge needles, scale colors, oil icons
- **Key Panel**: Green (present) or red (not present) key icons
- **Lock Panel**: Lock state indication
- **Theme Changes**: Immediate color transitions

## Known Limitations

### Display Differences
- **Wokwi**: Square ILI9341 (320x240) vs target round GC9A01 (240x240)
- **Orientation**: Wokwi displays may render horizontally inverted
- **Impact**: Visual layout may differ from actual hardware

### Emulation Constraints  
- **Timing**: Wokwi timing may differ from real ESP32 performance
- **Interrupts**: GPIO interrupt behavior may vary in emulation
- **ADC**: Potentiometer ADC values may not perfectly match real sensors

## Debugging

### Serial Monitor
- Enable serial monitor in Wokwi for detailed logging
- Debug builds include extensive logging for troubleshooting
- Monitor trigger state changes and panel transitions

### GDB Debugging
- Single GDB server configuration (port 3333) shared across all tests
- Use with Wokwi's debugging features for step-through analysis
- ELF files provided for symbol debugging

### Common Issues
- **Display Artifacts**: Usually caused by rapid trigger switching
- **Stuck Animations**: May indicate timing issues or animation conflicts
- **Wrong Colors**: Check theme state and DIP switch positions

## Test Documentation Format

Each test directory contains:
- `diagram.json` - Wokwi hardware configuration
- `README.md` - Detailed test procedures and pass criteria

**Note**: All tests share the single `wokwi.toml` configuration file in the root `wokwi/` directory.

## Contributing

When adding new test scenarios:
1. Follow the established directory structure
2. Include comprehensive test documentation
3. Specify clear pass/fail criteria
4. Document expected serial output patterns
5. Update this main README with new test descriptions