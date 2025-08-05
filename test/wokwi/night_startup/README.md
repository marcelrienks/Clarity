# Test 3.2: Startup with Night Theme Active

## Objective
Verify system startup behavior when lights trigger is active from boot, ensuring night theme is applied from the beginning.

## Test Setup
- **Pot1 (Pressure)**: Set to ~400 (moderate reading for visibility)
- **Pot2 (Temperature)**: Set to ~500 (moderate reading for visibility)
- **DIP Switches**: Switch #4 ON from start (00001000) - Lights trigger active
- **Initial State**: Night theme should be active from boot

## Expected Behavior

### Boot Sequence with Night Theme
1. **ESP32 Boot**: Standard hardware initialization
2. **Trigger Detection**: System detects lights trigger HIGH at startup
3. **Theme Initialization**: Night theme applied during system init
4. **Splash Screen**: Should display with RED text (night theme)
5. **Oil Panel Loading**: Should load with night theme (red scale ticks and icons)
6. **Gauge Animation**: Needles animate to sensor positions with night styling

### Key Differences from Day Startup
- **Splash Text**: RED instead of white
- **Oil Icons**: RED instead of white  
- **Scale Ticks**: RED instead of white
- **Theme State**: Night theme active from first screen

## Interactive Test Steps

### Phase 1: Initial Boot Verification
1. **Start Simulation**: Begin with DIP #4 already ON (lights active)
2. **Splash Screen Check**:
   - **Verify**: Splash text appears in RED (night theme)
   - **Compare**: Should be noticeably different from white day theme
   - **Timing**: Normal splash animation duration (~3 seconds)

### Phase 2: Oil Panel Night Theme Validation  
3. **Oil Panel Loading**:
   - **Verify**: Automatic transition from splash to oil panel
   - **Verify**: Oil icons are RED (night theme)
   - **Verify**: Scale ticks are RED (night theme)
   - **Verify**: Needles animate to ~4 Bar pressure, ~50°C temperature

### Phase 3: Theme Consistency Testing
4. **Theme Toggle Test**:
   - **Action**: Toggle DIP #4 OFF → Should switch to day theme
   - **Verify**: Colors change to white (scale ticks, icons)
   - **Verify**: NO panel reload occurs
5. **Theme Restoration**:
   - **Action**: Toggle DIP #4 ON → Should restore night theme
   - **Verify**: Colors change back to red
   - **Verify**: Panel continues running without interruption

## Pass Criteria

### Startup Verification
- [ ] System boots successfully with lights trigger HIGH
- [ ] Splash screen displays with RED text (night theme)
- [ ] Splash animation completes normally (~3 seconds)
- [ ] Automatic transition to oil panel occurs

### Night Theme Application
- [ ] Oil panel loads with night theme styling
- [ ] Oil icons are clearly RED colored
- [ ] Scale ticks are clearly RED colored  
- [ ] Gauge needles position correctly (~4 Bar, ~50°C)
- [ ] Night theme is consistent across all visual elements

### Theme Toggle Functionality
- [ ] DIP #4 OFF switches to day theme (white colors)
- [ ] NO panel reload occurs during theme change
- [ ] DIP #4 ON restores night theme (red colors)
- [ ] Theme changes are immediate and complete

### System Integration
- [ ] Serial monitor shows lights trigger detected at startup
- [ ] No errors or warnings during night theme initialization
- [ ] System operates normally in night theme mode
- [ ] Theme state persists correctly throughout test

## Expected Serial Output

### Startup Sequence
```
[DEBUG] Initializing trigger states from current sensor states...
[DEBUG] Trigger lights_state initialized to ACTIVE based on GPIO state
[DEBUG] Initializing style manager with theme: Night
[DEBUG] Switching application theme to: Night  
[INFO] Using config default panel: OemOilPanel
```

### Theme Toggle Testing
```
[INFO] Trigger lights_state: ACTIVE -> INACTIVE
[DEBUG] Switching application theme to: Day
[INFO] Trigger lights_state: INACTIVE -> ACTIVE  
[DEBUG] Switching application theme to: Night
```

## Visual Verification Points

### Critical Visual Checks
- **Splash Screen**: RED text clearly visible (not white)
- **Oil Icons**: RED oil can icons (not white)  
- **Scale Ticks**: RED tick marks around gauges (not white)
- **Gauge Needles**: Positioned at ~4 Bar and ~50°C
- **Theme Consistency**: All UI elements use night theme colors

### Comparison Reference
Use the `basic_startup` test as a comparison reference:
- **Day Startup**: White splash text, white scale ticks/icons
- **Night Startup**: RED splash text, red scale ticks/icons

## Critical Test Points

### Theme Initialization Priority
- **Startup Detection**: Lights trigger must be detected during system init
- **Theme Application**: Night theme must be applied BEFORE first UI render
- **Consistency**: All UI components must respect the initial theme state

### State Management
- **Persistence**: Theme state should persist correctly from boot
- **Toggle Functionality**: Theme switching should work normally after night startup
- **Priority Logic**: Lights trigger should maintain proper priority behavior

## Debugging Notes

### Common Issues
- **Wrong Colors**: If splash/UI shows white instead of red, theme not applied at startup
- **Delayed Theme**: If theme switches after splash, initialization timing issue
- **Toggle Problems**: If theme toggle doesn't work, trigger state management issue

### Verification Methods
- **Visual Comparison**: Side-by-side with day startup test
- **Serial Logs**: Confirm trigger detection and theme initialization
- **Color Accuracy**: Ensure red is clearly distinguishable from white

## Notes
- This test validates proper theme initialization timing
- Demonstrates system correctly reads trigger states at boot
- Verifies theme consistency from first UI render
- Tests edge case of non-default theme at startup
- Important for real-world scenarios where lights may be on when system powers up