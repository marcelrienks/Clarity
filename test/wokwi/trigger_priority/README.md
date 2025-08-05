# Test 6.2: Trigger Priority Validation

## Objective
Systematically test trigger priority handling and validate panel switching follows documented priority order under various trigger combinations.

## Test Setup
- **Pot1 (Pressure)**: Set to ~256 (lower reading for reference)
- **Pot2 (Temperature)**: Set to ~384 (lower-medium reading for reference)
- **DIP Switches**: All OFF initially (00000000)
- **Focus**: Systematic trigger priority testing

## Trigger Priority Reference

### Panel Triggers (Higher Priority = Shown)
- **Priority 0 (Highest)**: Key Present (DIP #1), Key Not Present (DIP #2)
- **Priority 1 (Medium)**: Lock State (DIP #3)
- **Priority ∞ (Lowest)**: Oil Panel (Default - no triggers)

### Theme Triggers (Independent)
- **Theme Control**: Lights (DIP #4) - Does NOT affect panel priority

## Systematic Priority Tests

### Test 6.2.1: Single Trigger Priority Validation

#### Individual Trigger Testing
1. **Oil Panel Baseline** (00000000)
   - **Expected**: Oil panel with gauge readings (~2.5 Bar, ~38°C)
   - **Verify**: Default panel behavior

2. **Lock Priority** (00100000 - DIP #3 ON)
   - **Expected**: Lock panel
   - **Verify**: Priority 1 overrides oil panel

3. **Key Not Present Priority** (01000000 - DIP #2 ON, #3 OFF)
   - **Expected**: Key panel (red icon)
   - **Verify**: Priority 0 overrides oil panel

4. **Key Present Priority** (10000000 - DIP #1 ON, #2 OFF)
   - **Expected**: Key panel (green icon)  
   - **Verify**: Priority 0 overrides oil panel

### Test 6.2.2: Multi-Trigger Priority Resolution

#### Two-Trigger Combinations
5. **Key Present + Lock** (10100000 - DIP #1 + #3 ON)
   - **Expected**: Key panel (green) - Priority 0 > Priority 1
   - **Verify**: Higher priority wins

6. **Key Not Present + Lock** (01100000 - DIP #2 + #3 ON)
   - **Expected**: Key panel (red) - Priority 0 > Priority 1
   - **Verify**: Higher priority wins

7. **Both Key Triggers** (11000000 - DIP #1 + #2 ON)
   - **Expected**: Lock panel OR Key panel (conflict resolution)
   - **Verify**: System handles invalid state gracefully

#### Three-Trigger Combinations  
8. **All Panel Triggers** (11100000 - DIP #1 + #2 + #3 ON)
   - **Expected**: Highest priority active (Key panel or Lock panel)
   - **Verify**: Priority system remains functional under maximum load

### Test 6.2.3: Theme Independence Validation

#### Theme + Panel Combinations
9. **Lock + Night Theme** (00101000 - DIP #3 + #4 ON)
   - **Expected**: Lock panel with night theme (red styling)
   - **Verify**: Theme doesn't affect panel priority

10. **Key Present + Night Theme** (10001000 - DIP #1 + #4 ON)
    - **Expected**: Key panel (green) with night theme
    - **Verify**: Theme applied independently of panel

### Test 6.2.4: Sequential Priority Changes

#### Priority Escalation Testing
11. **Oil → Lock → Key** Sequence:
    - Start: 00000000 (Oil panel)
    - Step 1: 00100000 (Lock panel) - Verify switch
    - Step 2: 10100000 (Key panel) - Verify higher priority takes over

12. **Priority Deactivation** Sequence:
    - Start: 10100000 (Key + Lock active, Key showing)
    - Step 1: 00100000 (Key OFF → Lock panel) - Verify fallback
    - Step 2: 00000000 (Lock OFF → Oil panel) - Verify default return

## Detailed Test Procedure

### Phase 1: Baseline and Individual Priorities
1. **Start Simulation**: Wait for oil panel (00000000)
2. **Test Lock Priority**: DIP #3 ON → Verify Lock panel
3. **Test Key Not Present**: DIP #3 OFF, #2 ON → Verify Key panel (red)
4. **Test Key Present**: DIP #2 OFF, #1 ON → Verify Key panel (green)
5. **Return to Baseline**: All OFF → Verify Oil panel

### Phase 2: Conflict Resolution
6. **Two Keys Conflict**: DIP #1 + #2 ON → Document behavior
7. **Key + Lock**: DIP #1 + #3 ON → Verify Key wins (Priority 0 > 1)
8. **All Triggers**: DIP #1 + #2 + #3 ON → Document resolution

### Phase 3: Theme Independence  
9. **Add Theme**: DIP #4 ON with various panel combinations
10. **Verify Independence**: Theme changes don't affect panel priority
11. **Remove Theme**: DIP #4 OFF → Verify panels unchanged

### Phase 4: Dynamic Priority Changes
12. **Sequential Activation**: Build up triggers one by one
13. **Sequential Deactivation**: Remove triggers and verify fallback
14. **Rapid Changes**: Quick priority switches to test stability

## Expected Panel Priority Behavior

### Priority Resolution Matrix
| Key Present | Key Not Present | Lock | Expected Panel | Notes |
|-------------|-----------------|------|----------------|-------|
| OFF | OFF | OFF | Oil | Default state |
| OFF | OFF | ON | Lock | Priority 1 |
| OFF | ON | OFF | Key (Red) | Priority 0 |
| ON | OFF | OFF | Key (Green) | Priority 0 |
| OFF | ON | ON | Key (Red) | Priority 0 > 1 |
| ON | OFF | ON | Key (Green) | Priority 0 > 1 |
| ON | ON | OFF | Key/Lock* | Invalid state - document behavior |
| ON | ON | ON | Key/Lock* | Invalid state - document behavior |

*Invalid states require system-specific conflict resolution

## Pass Criteria

### Priority Logic Validation
- [ ] Individual triggers activate correct panels
- [ ] Higher priority triggers override lower priority
- [ ] Priority 0 (Key) always overrides Priority 1 (Lock)
- [ ] Oil panel appears only when no triggers active

### Conflict Resolution
- [ ] Invalid key states handled gracefully (no crashes)
- [ ] System maintains stable state during conflicts
- [ ] Documented behavior for simultaneous key triggers
- [ ] Recovery possible from all conflict states

### Theme Independence
- [ ] Theme changes don't affect panel priority decisions
- [ ] All panels display correctly in both day and night themes
- [ ] Theme state persists across panel switches
- [ ] Theme + panel combinations work correctly

### Dynamic Behavior
- [ ] Sequential priority changes handled smoothly
- [ ] Fallback behavior works when higher priorities deactivate
- [ ] No race conditions during rapid trigger changes
- [ ] System maintains consistent state throughout test

## Expected Serial Output Patterns

### Priority Changes
```
[INFO] Trigger lock_state: INACTIVE -> ACTIVE
[INFO] Set active panel trigger: lock_state (priority 1)
[INFO] Trigger key_present: INACTIVE -> ACTIVE  
[INFO] Set active panel trigger: key_present (priority 0)
[INFO] Executing panel action: Load KeyPanel
```

### Conflict Detection (if implemented)
```
[WARN] Invalid trigger state: Both key_present and key_not_present active
[INFO] Resolving conflict: [resolution strategy]
```

## Critical Validation Points

### Priority System Integrity
- **Consistent Behavior**: Same trigger combinations always produce same results
- **No Priority Inversion**: Lower priority never overrides higher priority
- **Clean Transitions**: Panel switches occur smoothly without artifacts

### Edge Case Handling
- **Invalid States**: System remains stable during conflicts
- **Rapid Changes**: Priority resolution works under timing pressure
- **State Recovery**: System recovers from any trigger combination

## Documentation Requirements

### Test Results Documentation
- **Priority Matrix**: Document actual behavior for all combinations
- **Conflict Resolution**: Record how invalid states are handled
- **Edge Cases**: Note any unexpected behaviors or limitations
- **Performance**: Record response times for priority changes

## Notes
- This test validates the core trigger management system
- Essential for understanding system behavior under complex scenarios
- Results inform user documentation and system design decisions
- Identifies any gaps in priority logic implementation