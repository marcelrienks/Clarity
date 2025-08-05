# Test 4.2: Key Not Present Panel Switching

## Objective
Test key not present trigger functionality and panel switching with red key icon display.

## Test Setup
- **Pot1 (Pressure)**: Set to ~150 (low reading for reference)
- **Pot2 (Temperature)**: Set to ~250 (low-medium reading for reference)
- **DIP Switches**: All OFF initially (00000000)
- **Key Control**: DIP Switch #2 (Key Not Present trigger)

## Expected Behavior
1. **Initial State**:
   - System starts normally with oil panel
   - Dual gauges showing pressure/temperature readings
   - Day theme active (white scale ticks and icons)

2. **Key Not Present Activation**:
   - Toggle DIP Switch #2 ON (position 2 = 01000000)
   - **Panel Switch**: Oil panel should be replaced by Key panel
   - **Key Icon**: Red key icon should be displayed
   - **State Indication**: Visual indication that key is NOT present

3. **Key Not Present Deactivation**:
   - Toggle DIP Switch #2 OFF (back to 00000000)
   - **Panel Switch**: Key panel should be replaced by Oil panel
   - Return to dual gauge display
   - Oil panel should reload and display current sensor values

## Interactive Test Steps
1. **Start Simulation**: Wait for oil panel to fully load
2. **Note Initial Readings**: Observe pressure/temperature gauge positions (~1.5 Bar, ~25°C)
3. **Activate Key Not Present**:
   - Click DIP Switch #2 to ON position
   - **Verify**: Panel switches from Oil to Key
   - **Verify**: RED key icon is displayed (not present state)
   - **Verify**: Icon color clearly indicates negative/absent state
4. **Deactivate Key Not Present**:
   - Click DIP Switch #2 to OFF position
   - **Verify**: Panel switches back to Oil
   - **Verify**: Gauges reload with current sensor readings
5. **Repeat**: Toggle multiple times to verify consistency

## Pass Criteria
- [ ] Initial oil panel loads normally with low sensor readings
- [ ] DIP Switch #2 ON triggers immediate panel switch to Key panel
- [ ] Key panel displays RED key icon (not present state)
- [ ] Red color clearly indicates negative/warning state
- [ ] Key panel maintains current theme (day/night)
- [ ] DIP Switch #2 OFF triggers return to Oil panel
- [ ] Oil panel reloads with current sensor values
- [ ] Panel switching is smooth without errors
- [ ] Serial monitor shows trigger state changes

## Expected Serial Output
```
[INFO] Trigger key_not_present: INACTIVE -> ACTIVE
[INFO] Executing panel action: Load KeyPanel
[DEBUG] Creating and loading panel: KeyPanel (trigger-driven: yes)
[INFO] Trigger key_not_present: ACTIVE -> INACTIVE
[INFO] No other panel triggers active - restoring to default: OemOilPanel
```

## Visual Verification Points
- **Red Key Icon**: Must be clearly visible and RED colored
- **Color Distinction**: Red should be clearly distinguishable from green (present state)
- **Panel Transitions**: Should be clean without display artifacts
- **Theme Consistency**: Key panel should respect current theme (day/night)
- **Icon Clarity**: Red key icon should be properly rendered and centered

## Priority Level
This test validates **Priority 0** trigger behavior, same as key present trigger.

## Comparison with Key Present
| Aspect | Key Present (Test 4.1) | Key Not Present (Test 4.2) |
|--------|------------------------|----------------------------|
| Trigger | DIP Switch #1 | DIP Switch #2 |
| Icon Color | GREEN | RED |
| State | Positive/Present | Negative/Absent |
| Priority | 0 (Highest) | 0 (Highest) |
| Panel | KeyPanel | KeyPanel |

## Notes
- Red color indicates negative/warning state (key missing)
- Same panel (KeyPanel) used for both present/not present states
- Icon color is the primary differentiator between key states
- Both key triggers have equal priority (0) - highest in system
- Test verifies visual feedback for security-related key absence