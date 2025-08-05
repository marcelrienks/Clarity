# Test 4.1: Key Present Panel Switching

## Objective
Test key present trigger functionality and panel switching with green key icon display.

## Test Setup
- **Pot1 (Pressure)**: Set to ~200 (some reading for reference)
- **Pot2 (Temperature)**: Set to ~300 (some reading for reference)
- **DIP Switches**: All OFF initially (00000000)
- **Key Control**: DIP Switch #1 (Key Present trigger)

## Expected Behavior
1. **Initial State**:
   - System starts normally with oil panel
   - Dual gauges showing pressure/temperature readings
   - Day theme active

2. **Key Present Activation**:
   - Toggle DIP Switch #1 ON (position 1 = 10000000)
   - **Panel Switch**: Oil panel should be replaced by Key panel
   - **Key Icon**: Green key icon should be displayed
   - **State Indication**: Visual indication that key is present

3. **Key Present Deactivation**:
   - Toggle DIP Switch #1 OFF (back to 00000000)
   - **Panel Switch**: Key panel should be replaced by Oil panel
   - Return to dual gauge display
   - Oil panel should reload and display current sensor values

## Interactive Test Steps
1. **Start Simulation**: Wait for oil panel to fully load
2. **Note Initial Readings**: Observe pressure/temperature gauge positions
3. **Activate Key Present**:
   - Click DIP Switch #1 to ON position
   - **Verify**: Panel switches from Oil to Key
   - **Verify**: Green key icon is displayed
   - **Verify**: No other panels interfere
4. **Deactivate Key Present**:
   - Click DIP Switch #1 to OFF position
   - **Verify**: Panel switches back to Oil
   - **Verify**: Gauges reload with current sensor readings
5. **Repeat**: Toggle multiple times to verify consistency

## Pass Criteria
- [ ] Initial oil panel loads normally
- [ ] DIP Switch #1 ON triggers immediate panel switch to Key panel
- [ ] Key panel displays GREEN key icon (present state)
- [ ] Key panel maintains current theme (day/night)
- [ ] DIP Switch #1 OFF triggers return to Oil panel
- [ ] Oil panel reloads with current sensor values
- [ ] Panel switching is smooth without errors
- [ ] Serial monitor shows trigger state changes

## Expected Serial Output
```
[INFO] Trigger key_present: INACTIVE -> ACTIVE
[INFO] Executing panel action: Load KeyPanel
[DEBUG] Creating and loading panel: KeyPanel (trigger-driven: yes)
[INFO] Trigger key_present: ACTIVE -> INACTIVE
[INFO] No other panel triggers active - restoring to default: OemOilPanel
```

## Visual Verification Points
- **Green Key Icon**: Must be clearly visible and GREEN colored
- **Panel Transitions**: Should be clean without display artifacts
- **Theme Consistency**: Key panel should respect current theme
- **Icon Centering**: Key icon should be properly centered on display

## Priority Level
This test validates **Priority 0** trigger behavior according to the trigger system.

## Test 4.3: Invalid Key State Handling

### Objective
Test system behavior when both key present and key not present triggers are active simultaneously (invalid state).

### Invalid State Test Procedure
1. **Setup**: Start with oil panel (all switches OFF)
2. **Activate Key Present**: DIP #1 ON → Key panel (green icon)
3. **Create Invalid State**: DIP #2 ON (both key triggers active)
   - **Document**: Which panel is displayed (Key or Lock or other)
   - **Verify**: System doesn't crash or become unresponsive
   - **Monitor**: Serial output for error messages or warnings
4. **Resolve Invalid State**: 
   - Option A: DIP #1 OFF (leave only key not present) 
   - Option B: DIP #2 OFF (leave only key present)
   - **Verify**: System returns to valid state correctly

### Expected Invalid State Behavior
The system should handle this gracefully through one of these approaches:
- **Fallback to Lock Panel**: System may default to lock panel during conflicts
- **Priority Resolution**: One key trigger takes precedence over the other
- **Last Valid State**: System maintains previous valid panel until conflict resolves
- **Error State**: System shows error indication but remains stable

### Invalid State Pass Criteria
- [ ] **No Crashes**: System continues running during invalid state
- [ ] **Stable Display**: Panel remains visible and functional
- [ ] **Graceful Recovery**: System recovers when invalid state is resolved
- [ ] **Serial Logging**: Appropriate messages logged for invalid state
- [ ] **Deterministic Behavior**: Same invalid state always produces same result

### Expected Serial Output for Invalid State
```
[INFO] Trigger key_present: INACTIVE -> ACTIVE
[INFO] Trigger key_not_present: INACTIVE -> ACTIVE  
[WARN] Invalid key state: Both key_present and key_not_present active
[INFO] Conflict resolution: [system-specific behavior]
```

### Documentation Requirements
- **Record Behavior**: Document exactly what happens during invalid state
- **Recovery Method**: Note how system recovers from invalid state
- **Performance Impact**: Check if invalid state affects system performance
- **User Experience**: Evaluate if behavior is reasonable for end users

## Notes
- Key present has highest priority among panel triggers
- Panel switching should be immediate (< 1 second)
- Green color indicates positive/present state
- Test verifies round-trip functionality (oil → key → oil)
- Invalid state testing ensures system robustness
- Conflict resolution behavior should be documented for user manual