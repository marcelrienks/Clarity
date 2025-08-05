# Test Suite 5: Lock Panel Integration Tests

## Test 5.1: Lock Activation During Runtime

### Objective
Test lock trigger functionality and panel switching with lock icon display during runtime.

## Test Setup
- **Pot1 (Pressure)**: Set to ~350 (moderate reading for reference)
- **Pot2 (Temperature)**: Set to ~450 (moderate reading for reference)
- **DIP Switches**: All OFF initially (00000000)
- **Lock Control**: DIP Switch #3 (Lock trigger)

## Expected Behavior

### Initial State
- System starts normally with oil panel
- Dual gauges showing pressure/temperature readings (~3.5 Bar, ~45°C)
- Day theme active (white scale ticks and icons)

### Lock Activation
- Toggle DIP Switch #3 ON (position 3 = 00100000)
- **Panel Switch**: Oil panel should be replaced by Lock panel
- **Lock Icon**: Lock icon should be displayed indicating engaged state
- **Priority**: Lock trigger should take precedence over oil panel

### Lock Deactivation  
- Toggle DIP Switch #3 OFF (back to 00000000)
- **Panel Switch**: Lock panel should be replaced by Oil panel
- Return to dual gauge display
- Oil panel should reload and display current sensor values

## Interactive Test Steps

1. **Start Simulation**: Wait for oil panel to fully load
2. **Note Initial Readings**: Observe pressure/temperature gauge positions
3. **Activate Lock**:
   - Click DIP Switch #3 to ON position
   - **Verify**: Panel switches from Oil to Lock immediately
   - **Verify**: Lock icon is displayed clearly
   - **Verify**: Current theme is maintained (day/night)
4. **Deactivate Lock**:
   - Click DIP Switch #3 to OFF position
   - **Verify**: Panel switches back to Oil
   - **Verify**: Gauges reload with current sensor readings
5. **Repeat**: Toggle multiple times to verify consistency

## Test 5.2: Lock Priority Testing

### Objective
Verify lock panel takes precedence over key panel and returns to appropriate panel.

### Test Steps
1. **Setup**: Start with oil panel loaded
2. **Activate Key Present**: DIP Switch #1 ON → Key panel (green icon)
3. **Activate Lock**: DIP Switch #3 ON → Should switch to Lock panel (priority 1 > key priority 0)
4. **Deactivate Lock**: DIP Switch #3 OFF → Should return to Key panel (key still active)
5. **Deactivate Key**: DIP Switch #1 OFF → Should return to Oil panel

### Expected Priority Behavior
- **Lock Trigger Active**: Always shows Lock panel regardless of other triggers
- **Lock Deactivated**: Returns to highest priority active trigger
- **No Active Triggers**: Returns to default Oil panel

## Pass Criteria

- [ ] Initial oil panel loads normally with moderate sensor readings
- [ ] DIP Switch #3 ON triggers immediate panel switch to Lock panel  
- [ ] Lock panel displays lock icon clearly with current theme
- [ ] Lock panel maintains current theme (day/night)
- [ ] DIP Switch #3 OFF triggers return to Oil panel
- [ ] Oil panel reloads with current sensor values
- [ ] Lock trigger takes precedence over key triggers when both active
- [ ] Lock deactivation returns to next highest priority trigger
- [ ] Panel switching is smooth without errors or artifacts
- [ ] Serial monitor shows lock trigger state changes

## Expected Serial Output

```
[INFO] Trigger lock_state: INACTIVE -> ACTIVE
[INFO] Set active panel trigger: lock_state (priority 1)
[INFO] Executing panel action: Load LockPanel
[DEBUG] Creating and loading panel: LockPanel (trigger-driven: yes)
[INFO] Trigger lock_state: ACTIVE -> INACTIVE
[INFO] Removed panel trigger lock_state, new active: [next highest or none]
```

## Visual Verification Points
- **Lock Icon**: Must be clearly visible and properly rendered
- **Panel Transitions**: Should be clean without display artifacts
- **Theme Consistency**: Lock panel should respect current theme
- **Icon Clarity**: Lock icon should be properly centered on display

## Priority Level
This test validates **Priority 1** trigger behavior according to the trigger system.

## Critical Test Points
- **Immediate Switching**: Lock activation should immediately switch panels
- **Priority Override**: Lock should take precedence over key triggers
- **Return Logic**: Lock deactivation should return to appropriate panel based on remaining active triggers
- **State Persistence**: System should maintain trigger states correctly

## Notes
- Lock trigger has priority 1 (medium priority) - higher than oil panel but lower than key triggers
- Panel switching should be immediate (< 1 second)
- Lock panel indicates security/restricted state
- Test verifies both activation and deactivation scenarios
- Priority testing validates the trigger management system behavior