# Wokwi Integration Test Plan

**Test Name**: Clarity Full System Integration Test  
**Platform**: Wokwi ESP32 Simulator  
**Purpose**: Validate complete system functionality following the primary integration test scenario

## Test Overview

This comprehensive integration test exercises the entire Clarity digital gauge system, validating all panels, triggers, animations, and user interactions as defined in the Primary Integration Test Scenario in `scenarios.md`. The test follows a realistic user interaction flow that demonstrates system startup, sensor interactions, trigger priorities, error handling, and configuration management.

## Prerequisites

### Hardware Configuration
Hardware configuration is defined in `test/wokwi/diagram.json` which includes:
- ESP32 development board
- Action button (GPIO 32)
- Trigger buttons for Key, Lock, Lights, and Debug Error
- Potentiometers for Pressure and Temperature sensors

### Build and Run
Build configuration is specified in `test/wokwi/wokwi.toml`. To execute the test:

```bash
# Build test firmware
pio run -e debug-local

# Run integration test using wokwi-cli with configuration from toml file
# (The toml file contains paths to firmware.bin and firmware.elf)
```

## Test Phases

Based on the Primary Integration Test Scenario from `scenarios.md`:

### Phase 1: System Startup
1. **System Start**
   - Verify: Splash panel loads and animates with day theme
   - Serial: "SplashPanel loaded successfully"

2. **Oil Panel Initial Load**
   - Verify: Oil panel loads automatically after splash animation
   - Verify: Pressure and Temperature scales animate to preset start values
   - Serial: "OemOilPanel loaded successfully"

### Phase 2: Real-time Sensor Interaction
3. **Dynamic Sensor Changes**
   - Action: Adjust pressure potentiometer
   - Verify: Pressure gauge animates smoothly to reflect new value
   - Action: Adjust temperature potentiometer
   - Verify: Temperature gauge animates smoothly to reflect new value
   - Verify: Both gauges respond dynamically with continuous animation

### Phase 3: Theme and Trigger System
4. **Lights Trigger - Theme Change (STYLE Trigger)**
   - Action: Activate lights trigger
   - Verify: Night theme applied instantly to Oil Panel without reload
   - Verify: Oil panel continues displaying with night theme
   - Serial: "Theme changed to Night"

5. **Lock Trigger (IMPORTANT Priority)**
   - Action: Activate lock trigger
   - Verify: Lock panel loads (IMPORTANT priority)
   - Verify: Night theme maintained on Lock panel
   - Serial: "Lock trigger activated"

6. **Key Not Present Trigger (CRITICAL Priority)**
   - Action: Activate key not present trigger
   - Verify: Key panel loads with red key icon (CRITICAL priority overrides lock)
   - Verify: Key panel displays with night theme
   - Serial: "KeyNotPresentSensor state changed"

7. **Key Present Trigger Transition (CRITICAL Priority)**
   - Action: Activate key present trigger
   - Verify: Key panel reloads with green key icon
   - Verify: Key not present trigger automatically deactivates
   - Serial: "KeyPresentSensor state changed"

8. **Key State Reversal and Panel Persistence**
   - Action: Deactivate key not present trigger
   - Verify: Key panel maintains green key icon (key present still active)

9. **Priority-Based Restoration Chain**
   - Action: Deactivate key present trigger
   - Verify: Lock panel loads (restoration to next highest priority)
   - Verify: Lock panel displays with night theme

### Phase 4: Error System Integration
10. **Debug Error Trigger with Multiple Errors**
    - Action: Activate debug error trigger
    - Verify: Error panel loads with 3 reported errors (CRITICAL priority)
    - Verify: Error panel displays error list with night theme
    - Serial: "Error trigger activated"

11. **Error Navigation**
    - Action: Short press action button
    - Verify: Cycle through one error message
    - Verify: Error panel advances to next error, marking current as viewed

12. **Error Resolution and Restoration**
    - Action: Long press action button
    - Verify: Clear all errors and exit Error panel
    - Verify: Lock panel loads (next highest priority active trigger)

### Phase 5: Complete Trigger Chain Restoration
13. **Lock Trigger Deactivation**
    - Action: Deactivate lock trigger
    - Verify: Oil panel loads with night theme
    - Verify: Pressure and Temperature scales animate to preset values

### Phase 6: Theme Restoration
14. **Lights Trigger Deactivation**
    - Action: Deactivate lights trigger
    - Verify: Day theme applied to Oil Panel without reload
    - Serial: "Theme changed to Day"

### Phase 7: Configuration System
15. **Configuration Access**
    - Action: Long press action button on Oil panel
    - Verify: Config panel loads with main preference settings
    - Verify: Config panel displays with day theme

16. **Configuration Navigation to Theme Settings**
    - Action: Short press repeatedly until theme settings reached
    - Verify: Cycles through main menu options
    - Verify: Theme Settings option highlighted

17. **Theme Submenu Access**
    - Action: Long press action button
    - Verify: Theme option settings displayed (submenu)

18. **Theme Selection Navigation**
    - Action: Short press navigation until Night selected
    - Verify: Cycles between Day Theme and Night Theme options
    - Verify: Night option highlighted

19. **Theme Setting Application**
    - Action: Long press action button
    - Verify: Apply night theme and return to main preference settings
    - Verify: Config panel displays main menu with night theme applied

20. **Configuration Exit Navigation**
    - Action: Short press repeatedly until Exit reached
    - Verify: Cycles through main menu options until Exit highlighted

21. **Return to Oil Panel**
    - Action: Long press action button
    - Verify: Oil panel loads with applied night theme
    - Verify: Pressure and Temperature scales animate to preset values

## Implementation

### Test Automation Files Created

The integration test has been implemented with the following files:

1. **`test/wokwi/integration-test.yaml`** - Complete test automation specification
   - 7 test phases covering all 21 steps from the Primary Integration Test Scenario
   - Hardware mapping for all buttons, switches, and potentiometers
   - Serial pattern monitoring for test validation
   - Success criteria and performance metrics

2. **`test/wokwi/wokwi_run.sh`** - Simple bash script test runner
   - Automatic firmware building if needed
   - Wokwi-cli discovery and execution
   - 5-minute test timeout with colored output

3. **`test/wokwi/integration_test.py`** - Advanced Python test runner
   - Real-time serial monitoring and phase tracking
   - Automatic test progression analysis
   - Test summary with completion status

### Running the Integration Test

**Option 1: Simple Shell Script**
```bash
cd test/wokwi
./wokwi_run.sh
```

**Option 2: Advanced Python Runner**
```bash
cd test/wokwi
python3 integration_test.py
```

**Option 3: Direct Wokwi-CLI**
```bash
cd test/wokwi
wokwi-cli --timeout 300000
```

### Test Hardware Configuration

The test uses the existing `diagram.json` hardware setup:
- **Action Button (btn1)**: GPIO 32 - Main user input
- **Debug Button (btn2)**: GPIO 34 - Error trigger activation  
- **Pressure Pot (pot1)**: GPIO 36 - Oil pressure simulation
- **Temperature Pot (pot2)**: GPIO 39 - Oil temperature simulation
- **DIP Switch (sw1)**: 4-position switch for trigger simulation
  - Pin 1 (GPIO 25): Key Present trigger
  - Pin 2 (GPIO 26): Key Not Present trigger
  - Pin 3 (GPIO 27): Lock trigger
  - Pin 4 (GPIO 33): Lights trigger

## Wokwi Automation Scenario

The test automation has been implemented in `test/wokwi/integration-test.yaml` following the Primary Integration Test Scenario from `scenarios.md`. The automation includes:

### Key Automation Points:
- **Sensor Interactions**: Adjust pressure and temperature potentiometers to verify gauge animations
- **Trigger Sequence**: Lights → Lock → Key Not Present → Key Present → Key deactivation → Lock restoration
- **Error System**: Activate debug error with 3 errors, navigate with short press, exit with long press
- **Configuration Flow**: Access config, navigate to theme settings, apply night theme, exit
- **Theme Validation**: Verify instant theme changes without panel reloads
- **Panel Transitions**: Validate priority-based panel loading and restoration chain

### Expected Serial Monitoring:
The automation should monitor for key serial messages that indicate successful progression through each phase of the test scenario.

## Success Criteria

### Core Functionality ✅
- [ ] All 6 panels display correctly (Splash, Oil, Key, Lock, Error, Config)
- [ ] All 5 interrupts function with correct priorities
- [ ] Theme switching works (Day/Night)
- [ ] Button actions work (short vs long press)
- [ ] Sensor animations run smoothly
- [ ] Configuration system navigates properly

### Integration Points ✅
- [ ] Panel priorities: Error/Key (CRITICAL) > Lock (IMPORTANT) > Oil (default)
- [ ] Trigger restoration logic works correctly
- [ ] Theme persists across panel switches
- [ ] Animations don't block other operations
- [ ] Error system overrides all other panels
- [ ] Config changes apply immediately

### Performance ✅
- [ ] No memory leaks or crashes
- [ ] System remains responsive throughout test
- [ ] Smooth gauge animations
- [ ] Consistent frame rate maintained

## Expected Serial Patterns

Based on actual logging implementation in the codebase:

```
[System Startup]
Starting Clarity service initialization with dual factory pattern...
ProviderFactory created successfully
DeviceProvider created successfully
GpioProvider created successfully
DisplayProvider created successfully
ManagerFactory created successfully with provider factory
StyleManager configured for direct preference reading

[Panel Loading Triggers]
KeyPresentActivate() - Loading KEY panel
KeyNotPresentActivate() - Loading KEY panel
LockEngagedActivate() - Loading LOCK panel
ErrorOccurredActivate() - Loading ERROR panel
Saved user panel 'OemOilPanel' for restoration

[Theme Changes (STYLE Triggers)]
LightsOnActivate() - Setting NIGHT theme
LightsOffActivate() - Setting DAY theme

[Priority-Based Restoration]
LockDisengagedActivate() - Checking for restoration
ErrorClearedActivate() - Checking for restoration
No blocking interrupts - restoring to 'OemOilPanel'
Cannot restore - blocking interrupts still active

[Button Actions]
ShortPressActivate() - Executing short press action
LongPressActivate() - Executing long press action
Short press on OIL panel - cycling to next panel
Long press on OIL panel - loading config panel

[Error System Integration]
Error trigger detected - loading ErrorPanel
Error trigger cleared - resetting error panel state
ErrorManager: 3 errors reported
Error navigation - cycling through errors
Error resolution - all errors cleared

[Sensor Interactions]
Pressure reading changed from ADC
Temperature reading changed from ADC
Gauge animation smooth update
Sensor state change detected

[Configuration System]
Config panel navigation started
Theme settings submenu accessed
Night theme applied via configuration
Config exit - returning to oil panel
```

## Usage

1. **Run test to validate system functionality**: Execute the complete Primary Integration Test Scenario
2. **Monitor serial output**: Check for expected patterns indicating successful progression
3. **Verify all phases complete**: Ensure startup, sensor interactions, triggers, errors, and configuration all function correctly

This integration test provides comprehensive validation of the complete Clarity system following a realistic user interaction flow as defined in the Primary Integration Test Scenario from `scenarios.md`.