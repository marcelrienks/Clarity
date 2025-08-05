# Test 6.1: Complete System Integration Test (Major Scenario)

## Objective
Execute the comprehensive test scenario from scenario.md that validates all major system components and their interactions.

## Test Setup
- **Pot1 (Pressure)**: Set to ~512 (halfway position)
- **Pot2 (Temperature)**: Set to ~512 (halfway position)  
- **DIP Switches**: All OFF initially (00000000)
- **Required**: Manual DIP switch manipulation during test

## Complete Test Sequence

### Step 1: Initial Startup
**Expected**: App starts with pressure and temperature values set to halfway
- **Verify**: Splash animates with day theme (white text)
- **Verify**: Oil panel loads with day theme (white scale ticks and icon)  
- **Verify**: Oil panel needles animate to halfway positions

### Step 2: Night Theme Activation  
**Action**: Toggle DIP Switch #4 ON (Lights trigger HIGH)
- **Expected**: Oil panel does NOT reload, theme changes to night (red scale ticks and icon)
- **Verify**: Panel continues running, only colors change
- **Verify**: Needles remain in position

### Step 3: Lock Panel Activation
**Action**: Toggle DIP Switch #3 ON (Lock trigger HIGH)  
- **Expected**: Lock panel loads
- **Verify**: Panel switches from Oil to Lock
- **Verify**: Lock icon displayed with night theme

### Step 4: Key Not Present
**Action**: Toggle DIP Switch #2 ON (Key not present trigger HIGH)
- **Expected**: Key panel loads (present = false → red icon)
- **Verify**: Panel switches from Lock to Key  
- **Verify**: RED key icon displayed (not present state)

### Step 5: Invalid Key State
**Action**: Toggle DIP Switch #1 ON (Key present trigger HIGH)
- **Expected**: Lock panel loads (both key present and key not present true = invalid state)
- **Verify**: System handles conflicting key states gracefully
- **Verify**: Falls back to Lock panel (higher priority)

### Step 6: Key Present State
**Action**: Toggle DIP Switch #2 OFF (Key not present trigger LOW)
- **Expected**: Key panel loads (present = true → green icon, key present trigger still high)
- **Verify**: Panel switches to Key with GREEN icon
- **Verify**: Only key present trigger remains active

### Step 7: Return to Lock
**Action**: Toggle DIP Switch #1 OFF (Key present trigger LOW)  
- **Expected**: Lock panel loads (lock trigger still high)
- **Verify**: Panel switches back to Lock
- **Verify**: Lock trigger takes precedence

### Step 8: Return to Oil Panel
**Action**: Toggle DIP Switch #3 OFF (Lock trigger LOW)
- **Expected**: Oil panel loads with night theme (red scale ticks and icon, lights trigger still active)
- **Verify**: Panel switches back to Oil  
- **Verify**: Night theme maintained (lights still active)
- **Verify**: Oil panel needles animate to current sensor values

### Step 9: Day Theme Restoration
**Action**: Toggle DIP Switch #4 OFF (Lights trigger LOW)
- **Expected**: Oil panel does NOT reload, theme changes to day (white scale ticks and icon)
- **Verify**: Only theme changes, no panel reload
- **Verify**: Day theme restored (white colors)

## Pass Criteria
- [ ] All 9 steps execute in sequence without errors
- [ ] Panel switching follows documented priority order  
- [ ] Theme changes occur without panel reloads
- [ ] Key state conflicts handled gracefully
- [ ] Oil panel needles maintain/restore positions correctly
- [ ] Serial monitor shows all expected trigger changes
- [ ] No display artifacts or stuck states
- [ ] System returns to stable oil panel state

## Expected Serial Output Pattern
```
[INFO] Oil pressure/temperature readings established
[INFO] Trigger lights_state: INACTIVE -> ACTIVE (Theme: Night)
[INFO] Trigger lock_state: INACTIVE -> ACTIVE (Panel: Lock)
[INFO] Trigger key_not_present: INACTIVE -> ACTIVE (Panel: Key)  
[INFO] Trigger key_present: INACTIVE -> ACTIVE (Panel: Lock - conflict resolution)
[INFO] Trigger key_not_present: ACTIVE -> INACTIVE (Panel: Key - green)
[INFO] Trigger key_present: ACTIVE -> INACTIVE (Panel: Lock)
[INFO] Trigger lock_state: ACTIVE -> INACTIVE (Panel: Oil)
[INFO] Trigger lights_state: ACTIVE -> INACTIVE (Theme: Day)
```

## Critical Validation Points
1. **Theme vs Panel Behavior**: Theme triggers should never reload panels
2. **Priority Handling**: Key triggers (priority 0) override lock (priority 1)  
3. **Conflict Resolution**: Invalid key states should fall back gracefully
4. **State Persistence**: Oil readings should persist through panel switches
5. **Animation Continuity**: Gauge animations should be smooth and complete

## Timing Considerations
- Allow 2-3 seconds between steps for animations to complete
- Observe each panel transition carefully for visual verification
- Monitor serial output for trigger state confirmations

## Notes
This is the most comprehensive test scenario, validating the complete system integration including all trigger types, priority handling, theme switching, and panel management.