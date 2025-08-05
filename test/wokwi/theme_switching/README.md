# Test 3.1: Day/Night Theme Switching

## Objective
Verify theme switching functionality triggered by lights sensor without panel reload.

## Test Setup
- **Pot1 (Pressure)**: Set to ~300 (some visible reading)
- **Pot2 (Temperature)**: Set to ~400 (some visible reading)  
- **DIP Switches**: All OFF initially (00000000)
- **Theme Control**: DIP Switch #4 (Lights trigger)

## Expected Behavior
1. **Initial State**: 
   - System starts with day theme (white scale ticks and icons)
   - Oil panel displays with sensor readings
   - Needles positioned according to potentiometer values

2. **Night Mode Activation**:
   - Toggle DIP Switch #4 ON (position 4 = 00001000)
   - **CRITICAL**: Panel should NOT reload/restart
   - Theme immediately switches to night mode
   - Scale ticks and icons change to red color
   - Needles remain in same positions (no re-animation)

3. **Day Mode Restoration**:
   - Toggle DIP Switch #4 OFF (back to 00000000)
   - Theme immediately switches back to day mode
   - Scale ticks and icons change to white color
   - Needles remain in same positions

## Interactive Test Steps
1. **Start Simulation**: Wait for oil panel to fully load
2. **Observe Initial State**: Note white scale ticks/icons (day theme)
3. **Activate Night Mode**: 
   - Click DIP Switch #4 to ON position
   - **Verify**: NO panel reload occurs
   - **Verify**: Colors change to red (night theme)
4. **Deactivate Night Mode**:
   - Click DIP Switch #4 to OFF position  
   - **Verify**: Colors change back to white (day theme)
5. **Repeat**: Toggle multiple times to verify consistency

## Pass Criteria
- [ ] Initial oil panel loads with day theme (white ticks/icons)
- [ ] DIP Switch #4 ON triggers immediate theme change to night (red)
- [ ] NO panel reload occurs during theme change
- [ ] Needle positions remain unchanged during theme switch
- [ ] DIP Switch #4 OFF restores day theme (white)
- [ ] Theme changes are instantaneous (no delay/animation)
- [ ] Serial monitor shows theme change messages
- [ ] Multiple theme toggles work consistently

## Expected Serial Output
```
[INFO] Trigger lights_state: INACTIVE -> ACTIVE
[DEBUG] Switching application theme to: Night
[INFO] Trigger lights_state: ACTIVE -> INACTIVE  
[DEBUG] Switching application theme to: Day
```

## Critical Test Points
- **No Panel Reload**: This is the key difference from panel triggers
- **Color Changes Only**: Only scale/icon colors should change
- **Needle Preservation**: Gauge needle positions must remain stable
- **Immediate Response**: Theme changes should be instantaneous

## Notes
- This test specifically validates that theme triggers behave differently from panel triggers
- The oil panel should continue running without interruption during theme changes
- Visual verification is crucial for color change confirmation