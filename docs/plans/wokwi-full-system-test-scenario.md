# Wokwi Full System Integration Test Scenario

**Test Name**: Complete Clarity System Integration Test  
**Platform**: Wokwi ESP32 Simulator  
**Duration**: ~5-7 minutes  
**Purpose**: Validate complete system functionality including all panels, triggers, animations, and user interactions

## Test Overview

This comprehensive test scenario exercises the entire Clarity digital gauge system in a single cohesive flow, validating all major components:
- Panel lifecycle management (init → load → update → animations)
- Interrupt system (triggers, actions, priority handling)  
- UI system (LVGL rendering, themes, animations)
- User interactions (button presses, navigation)
- Error handling and recovery
- Configuration management

## Prerequisites

### Hardware Setup (Wokwi)
```json
{
  "version": 1,
  "author": "Clarity Integration Test",
  "editor": "wokwi",
  "parts": [
    { 
      "type": "wokwi-esp32-devkit-v1",
      "id": "esp32",
      "top": 0,
      "left": 0
    },
    {
      "type": "wokwi-pushbutton", 
      "id": "btn-action",
      "top": 150,
      "left": 200,
      "attrs": { "color": "blue" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "btn-key-present", 
      "top": 200,
      "left": 200,
      "attrs": { "color": "green" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "btn-lock",
      "top": 250, 
      "left": 200,
      "attrs": { "color": "yellow" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "btn-lights",
      "top": 300,
      "left": 200, 
      "attrs": { "color": "white" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "btn-debug-error",
      "top": 350,
      "left": 200,
      "attrs": { "color": "red" }
    },
    {
      "type": "wokwi-potentiometer",
      "id": "pot-oil-pressure",
      "top": 100,
      "left": 400,
      "attrs": { "value": "2048" }
    },
    {
      "type": "wokwi-potentiometer", 
      "id": "pot-oil-temperature",
      "top": 200,
      "left": 400,
      "attrs": { "value": "1024" }
    }
  ],
  "connections": [
    ["esp32:GPIO32", "btn-action:1.l", "blue", []],
    ["esp32:GPIO25", "btn-key-present:1.l", "green", []],
    ["esp32:GPIO26", "btn-lock:1.l", "yellow", []],
    ["esp32:GPIO27", "btn-lights:1.l", "white", []],
    ["esp32:GPIO34", "btn-debug-error:1.l", "red", []],
    ["esp32:GPIO35", "pot-oil-pressure:SIG", "orange", []],
    ["esp32:GPIO36", "pot-oil-temperature:SIG", "purple", []],
    ["btn-action:2.r", "esp32:GND", "black", []],
    ["btn-key-present:2.r", "esp32:GND", "black", []],
    ["btn-lock:2.r", "esp32:GND", "black", []],
    ["btn-lights:2.r", "esp32:GND", "black", []],
    ["btn-debug-error:2.r", "esp32:GND", "black", []]
  ]
}
```

### Build Configuration
```bash
# Use debug-local environment for optimal Wokwi compatibility
pio run -e debug-local --target upload
```

## Complete Test Scenario

### Phase 1: System Startup & Initial State
**Duration**: 30 seconds  
**Expected Behavior**: System initialization with splash screen

1. **Power On ESP32** 
   - ✅ **Verify**: Serial output shows system initialization
   - ✅ **Verify**: Factory creation and provider initialization logs
   - ✅ **Verify**: InterruptManager initialization with all handlers

2. **Splash Screen Animation** (0-3 seconds)
   - ✅ **Verify**: Display shows Clarity branding/logo
   - ✅ **Verify**: Smooth splash screen animation
   - ✅ **Verify**: Serial: "SplashPanel loaded successfully"
   - ✅ **Verify**: Automatic transition to Oil panel after splash duration

3. **Oil Panel Initial Load** (3-5 seconds)
   - ✅ **Verify**: Oil pressure gauge visible (left side)
   - ✅ **Verify**: Oil temperature gauge visible (right side) 
   - ✅ **Verify**: Serial: "OemOilPanel loaded successfully"
   - ✅ **Verify**: Day theme active (white background)

### Phase 2: Sensor Data & Animations  
**Duration**: 45 seconds  
**Expected Behavior**: Dynamic sensor readings with smooth animations

4. **Pressure/Temperature Animations** (5-15 seconds)
   - ✅ **Verify**: Pressure needle animates to initial position (~2 Bar equivalent)
   - ✅ **Verify**: Temperature needle animates to initial position (~40°C equivalent)
   - ✅ **Verify**: Serial: Animation completion callbacks
   - ✅ **Verify**: UI state returns to IDLE after animations complete

5. **Dynamic Sensor Value Changes** (15-25 seconds)
   - **Action**: Adjust oil pressure potentiometer (pot-oil-pressure) from 2048 to 3500
   - ✅ **Verify**: Pressure needle smoothly animates to new position
   - ✅ **Verify**: Serial: "Pressure reading changed to X Bar"
   - **Action**: Adjust oil temperature potentiometer (pot-oil-temperature) from 1024 to 2500  
   - ✅ **Verify**: Temperature needle smoothly animates to new position
   - ✅ **Verify**: Both animations can run simultaneously without conflicts

### Phase 3: Trigger System Testing
**Duration**: 90 seconds  
**Expected Behavior**: Priority-based trigger system with panel switching

6. **Lights Trigger (Night Theme)** (25-30 seconds)
   - **Action**: Press and hold `btn-lights` (GPIO 27)
   - ✅ **Verify**: Theme changes from Day to Night (background turns red)
   - ✅ **Verify**: Oil gauges update with night theme colors
   - ✅ **Verify**: Serial: "Theme changed to Night" 
   - ✅ **Verify**: Oil panel remains active (no panel change)

7. **Lock Trigger (IMPORTANT Priority)** (30-40 seconds)
   - **Action**: Press and hold `btn-lock` (GPIO 26)
   - ✅ **Verify**: Panel switches to Lock panel immediately
   - ✅ **Verify**: Lock icon displayed in center of screen
   - ✅ **Verify**: Serial: "Lock trigger activated - loading lock panel"
   - ✅ **Verify**: Night theme maintained on lock panel

8. **Key Not Present Trigger (IMPORTANT Priority)** (40-50 seconds)
   - **Action**: Press and hold `btn-key-present` to simulate key removal
   - ✅ **Verify**: Panel switches to Key panel with RED key icon
   - ✅ **Verify**: Serial: "KeyNotPresentSensor state changed: NOT_PRESENT -> PRESENT"
   - ✅ **Verify**: Red key icon indicates key not present state
   - ✅ **Verify**: Night theme maintained

9. **Key Present Trigger (IMPORTANT Priority)** (50-60 seconds)
   - **Action**: Release `btn-key-present` to simulate key insertion
   - ✅ **Verify**: Key icon changes to GREEN (key present)
   - ✅ **Verify**: Serial: "KeyPresentSensor state changed: NOT_PRESENT -> PRESENT" 
   - ✅ **Verify**: Panel remains on Key panel but icon color changes
   - ✅ **Verify**: Green key icon indicates key present state

10. **Key Not Present Deactivation** (60-70 seconds)
    - **Action**: Verify key not present trigger is inactive
    - ✅ **Verify**: Green key panel remains active
    - ✅ **Verify**: No automatic panel changes
    - ✅ **Verify**: System maintains current state correctly

11. **Key Present Deactivation + Lock Restoration** (70-80 seconds)
    - **Action**: Press and hold `btn-key-present` again (deactivate key present)
    - ✅ **Verify**: Panel switches back to Lock panel (trigger restoration)
    - ✅ **Verify**: Serial: "Trigger restoration: returning to lock panel"
    - ✅ **Verify**: Lock icon displayed with night theme

### Phase 4: Error Handling System
**Duration**: 60 seconds  
**Expected Behavior**: CRITICAL priority error system with navigation

12. **Debug Error Trigger (CRITICAL Priority)** (80-90 seconds)
    - **Action**: Press `btn-debug-error` (GPIO 34) 
    - ✅ **Verify**: Panel immediately switches to Error panel
    - ✅ **Verify**: Error list displayed with at least one error entry
    - ✅ **Verify**: Serial: "CRITICAL error trigger activated"
    - ✅ **Verify**: Night theme maintained on error panel

13. **Error Panel Navigation - Short Press** (90-100 seconds)
    - **Action**: Short press `btn-action` (GPIO 32, 500-1500ms)
    - ✅ **Verify**: Error panel cycles to next error in list
    - ✅ **Verify**: Serial: "Short press action - cycling to next error"
    - ✅ **Verify**: Scrollable error display updates

14. **Error Panel Navigation - Long Press Exit** (100-110 seconds)
    - **Action**: Long press `btn-action` (GPIO 32, >1500ms)
    - ✅ **Verify**: Panel switches back to Lock panel (restoration)
    - ✅ **Verify**: Serial: "Long press action - exiting error panel"
    - ✅ **Verify**: Error panel properly cleaned up

### Phase 5: Trigger Deactivation & Theme Changes
**Duration**: 45 seconds  
**Expected Behavior**: Proper trigger cleanup and theme transitions

15. **Lock Trigger Deactivation** (110-125 seconds)
    - **Action**: Release `btn-lock` (GPIO 26)
    - ✅ **Verify**: Panel switches to Oil panel (default restoration)
    - ✅ **Verify**: Serial: "Lock trigger deactivated - restoring oil panel"
    - ✅ **Verify**: Oil gauges display with night theme
    - ✅ **Verify**: Pressure/temperature animations resume

16. **Lights Trigger Deactivation (Day Theme)** (125-135 seconds)  
    - **Action**: Release `btn-lights` (GPIO 27)
    - ✅ **Verify**: Theme changes from Night to Day (background to white)
    - ✅ **Verify**: Oil gauges update with day theme colors immediately
    - ✅ **Verify**: Serial: "Theme changed to Day"
    - ✅ **Verify**: Panel remains on Oil panel

### Phase 6: Configuration System Testing
**Duration**: 120 seconds  
**Expected Behavior**: Complete configuration navigation and theme management

17. **Enter Configuration Panel** (135-145 seconds)
    - **Action**: Long press `btn-action` (>1500ms) while on Oil panel
    - ✅ **Verify**: Panel switches to Config panel
    - ✅ **Verify**: Configuration menu displayed with options
    - ✅ **Verify**: Serial: "Long press action - loading config panel"
    - ✅ **Verify**: Day theme maintained

18. **Navigate Config Options** (145-160 seconds)
    - **Action**: Short press `btn-action` repeatedly (5-8 times)
    - ✅ **Verify**: Config menu cycles through options:
      - Theme Settings
      - Sensor Settings  
      - Display Settings
      - System Settings
      - Exit
    - ✅ **Verify**: Visual highlighting of selected option
    - ✅ **Verify**: Serial logs for each option selection

19. **Enter Theme Sub-Settings** (160-170 seconds)
    - **Action**: Navigate to "Theme Settings" and long press `btn-action`
    - ✅ **Verify**: Theme sub-menu opens
    - ✅ **Verify**: Current theme highlighted (Day)
    - ✅ **Verify**: Available theme options visible
    - ✅ **Verify**: Serial: "Entering theme configuration"

20. **Change Theme in Config** (170-180 seconds)
    - **Action**: Short press `btn-action` to cycle to Night theme
    - ✅ **Verify**: Night theme option becomes highlighted
    - **Action**: Long press `btn-action` to confirm selection
    - ✅ **Verify**: Theme immediately changes to Night (red background)
    - ✅ **Verify**: Returns to main config menu with night theme applied
    - ✅ **Verify**: Serial: "Theme changed to Night via configuration"

21. **Navigate to Exit Configuration** (180-200 seconds)
    - **Action**: Short press `btn-action` repeatedly until "Exit" is selected
    - ✅ **Verify**: Menu cycles through all options back to Exit
    - ✅ **Verify**: Exit option becomes highlighted
    - ✅ **Verify**: Consistent night theme throughout navigation

22. **Exit Configuration Panel** (200-210 seconds)  
    - **Action**: Long press `btn-action` on Exit option
    - ✅ **Verify**: Panel switches back to Oil panel
    - ✅ **Verify**: Night theme maintained on Oil panel
    - ✅ **Verify**: Serial: "Exiting configuration - returning to oil panel"
    - ✅ **Verify**: Configuration properly saved and applied

### Phase 7: Final System Validation
**Duration**: 30 seconds  
**Expected Behavior**: Complete system functionality confirmation

23. **Final Pressure/Temperature Animations** (210-240 seconds)
    - **Action**: Adjust both potentiometers to different values
    - ✅ **Verify**: Both gauges animate smoothly to new positions
    - ✅ **Verify**: Night theme colors maintained throughout animations
    - ✅ **Verify**: Dual animations work without conflicts
    - ✅ **Verify**: System remains responsive to all inputs

## Success Criteria

### Core Functionality ✅
- [ ] All 6 panels load and display correctly (Splash, Oil, Key, Lock, Error, Config)
- [ ] All 4 trigger types function with correct priorities (CRITICAL > IMPORTANT > NORMAL)
- [ ] Button actions work reliably (short press vs long press detection)
- [ ] Theme system switches properly between Day/Night modes
- [ ] Animations run smoothly without blocking other operations

### Performance Metrics ✅
- [ ] Panel transitions occur within 500ms
- [ ] Animations complete within expected duration (750ms for gauge animations)
- [ ] No memory leaks or system crashes during full test cycle
- [ ] Responsive user input throughout entire test sequence
- [ ] Consistent frame rate and smooth UI rendering

### Integration Validation ✅  
- [ ] Factory systems create all components successfully
- [ ] Interrupt system handles concurrent triggers correctly
- [ ] Sensor data flows properly to display components
- [ ] Error system integrates with trigger priorities
- [ ] Configuration changes persist and apply immediately
- [ ] Panel restoration logic works after trigger deactivation

## Expected Serial Output Patterns

```
[Boot Sequence]
ManagerFactory: Initializing providers...
InterruptManager initialized successfully
SplashPanel loaded successfully
OemOilPanel loaded successfully

[Animations]  
Pressure reading changed to X Bar
Temperature reading changed to Y C
Animation completion callback

[Triggers]
Theme changed to Night
Lock trigger activated - loading lock panel
KeyNotPresentSensor state changed
CRITICAL error trigger activated

[User Actions]
Short press action detected
Long press action detected  
Config panel navigation

[System State]
UI state: IDLE
Trigger restoration: returning to oil panel
```

## Test Execution Notes

### Timing Considerations
- Allow sufficient time between actions for animations to complete
- Monitor serial output to confirm each phase completion
- Adjust potentiometer values gradually for smooth animations

### Visual Verification Points
- Panel transitions should be instantaneous (no flickering)
- Gauge animations should be smooth and consistent
- Theme changes should apply to all UI elements immediately
- Error panel should display readable error information

### Common Issues & Troubleshooting
- **Issue**: Panel doesn't switch → Check trigger timing and serial logs
- **Issue**: Animations stutter → Verify LVGL buffer configuration  
- **Issue**: Theme doesn't apply → Check StyleManager initialization
- **Issue**: Button not responding → Verify GPIO pin mappings and debouncing

## Post-Test Validation

After completing the full test scenario:

1. **System State Check**
   - Verify system returns to stable oil panel state
   - Confirm all triggers are properly deactivated
   - Check memory usage remains stable

2. **Log Analysis**
   - Review complete serial output for errors or warnings
   - Verify all expected log messages appeared
   - Confirm no memory leaks or resource issues

3. **Configuration Persistence**
   - Power cycle the system
   - Verify theme setting persists after restart
   - Confirm all user preferences maintained

This comprehensive test scenario validates the complete Clarity system functionality in a single cohesive flow, ensuring all major components work together correctly and providing confidence in the overall system integration.

---
**Test Duration**: ~7 minutes  
**Coverage**: 100% of major system functionality  
**Automation Potential**: High (can be scripted with Wokwi API)