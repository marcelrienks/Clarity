# Architecture Decision Record - Interrupt System Evolution

## Decision Summary

**Decision**: Adopt dual-function interrupt architecture (v3.0) over separate activation/deactivation interrupts (v2.0)

**Date**: August 31, 2025

**Status**: Proposed (not yet implemented)

**Context**: Evolution from complex evaluation/execution system to simplified single-purpose interrupts

## Problem Statement

The interrupt system has evolved through multiple iterations:

1. **Original System**: Complex evaluation/execution separation with effects-based switching
2. **v1.0 Simplified**: Single-purpose interrupts with direct execution
3. **v2.0 Dual Interrupts**: Separate interrupts for activate/deactivate states
4. **v3.0 Dual Functions**: Single interrupt with activate/deactivate function pairs

The core challenge: How to handle sensor state changes (HIGH/LOW, ON/OFF) with proper override logic while maintaining simplicity.

## Architecture Comparison

### v2.0 Approach: Separate Interrupts
```cpp
// Two separate interrupt objects per sensor
Interrupt lockEngagedInterrupt = {
    .id = "lock_engaged",
    .triggerState = SensorState::ACTIVE,
    .canBeOverridden = false,
    .execute = &LoadLockPanel
};

Interrupt lockDisengagedInterrupt = {
    .id = "lock_disengaged", 
    .triggerState = SensorState::INACTIVE,
    .canBeOverridden = true,
    .execute = &CheckRestoration
};
```

**Problems with v2.0:**
1. **Type Complexity**: Need separate interrupt types for activation vs deactivation
2. **Confusing Semantics**: When is "lock_disengaged" interrupt considered "active"?
3. **Object Proliferation**: Double the number of interrupt objects
4. **Override Confusion**: Do deactivation interrupts participate in override logic?

### v3.0 Approach: Dual Functions
```cpp
// Single interrupt object per sensor with dual functions
Interrupt lockInterrupt = {
    .id = "lock",
    .activateFunc = &LoadLockPanel,          // Called when GPIO goes HIGH
    .deactivateFunc = &CheckRestoration,     // Called when GPIO goes LOW
    .canBeOverriddenOnActivate = false,      // Only activation can be overridden
    .sensor = &lockSensor
};
```

**Benefits of v3.0:**
1. **Natural Grouping**: Related functions stay together conceptually
2. **Clear Semantics**: Only `activateFunc` sets `isActive = true`
3. **Simple Override Logic**: Only activation participates in override resolution
4. **1:1 Mapping**: One interrupt per sensor, clean mental model
5. **No Type Complexity**: Single interrupt struct handles both states

## Decision Rationale

### Primary Reasons for v3.0

#### 1. **Eliminates Semantic Confusion**
**Problem**: In v2.0, when is a "deactivation" interrupt considered "active"?
- If `lock_disengaged` interrupt is "active" when lock is disengaged (normal operation), then most deactivation interrupts would be active most of the time
- This makes the `isActive` flag meaningless for override logic

**Solution**: In v3.0, only `activateFunc` can set `isActive = true`. Deactivation never sets active state.

#### 2. **Natural Conceptual Model**
**Problem**: Sensors have states (HIGH/LOW), not separate entities
**Solution**: One interrupt per sensor matches the natural 1:1 relationship

#### 3. **Simplified Override Logic**
**Problem**: Do deactivation interrupts participate in override logic? How?
**Solution**: Only activation can be overridden (makes logical sense). Deactivation is always straightforward.

#### 4. **Reduced Complexity**
**Problem**: v2.0 requires managing twice as many interrupt objects
**Solution**: v3.0 has half the objects with cleaner organization

### Technical Implementation Benefits

```cpp
// v2.0: Complex state matching
if (sensor->GetCurrentState() == interrupt.triggerState) {
    // Which interrupts should execute? How do they interact?
    ExecuteInterrupt(interrupt);
}

// v3.0: Clear state handling  
if (sensor->HasStateChanged()) {
    if (sensor->GetCurrentState() == SensorState::ACTIVE) {
        HandleActivation(interrupt);  // Check overrides
    } else {
        interrupt.ExecuteDeactivate(); // Always execute, no overrides
    }
}
```

### Override Logic Clarity

#### v2.0 Override Questions:
- Can deactivation interrupts be overridden?
- When are deactivation interrupts "active"?
- How do activation/deactivation interrupts interact?

#### v3.0 Override Answers:
- Only activation can be overridden
- Only activation sets `isActive = true`  
- Clean separation: activation uses override logic, deactivation uses simple execution

## Real-World Scenarios

### Scenario: Key Present → Lock Engaged → Lock Disengaged

#### v2.0 Behavior (Confusing):
```
1. key_present_activate.isActive = true
2. lock_engaged tries to activate but blocked
3. lock_disengaged executes... but is it "active"?
```

#### v3.0 Behavior (Clear):
```
1. keyInterrupt.ExecuteActivate() → isActive = true
2. lockInterrupt activation blocked by override logic
3. lockInterrupt.ExecuteDeactivate() → simple restoration check
```

## Code Organization Impact

### v2.0: Scattered Related Logic
```cpp
// Functions for same sensor spread across different interrupts
&LoadLockPanel;      // In lock_engaged interrupt
&CheckRestoration;   // In lock_disengaged interrupt
```

### v3.0: Grouped Related Logic
```cpp
// Functions for same sensor grouped together
Interrupt lockInterrupt = {
    .activateFunc = &LoadLockPanel,
    .deactivateFunc = &CheckRestoration  // Related functions together
};
```

## Performance Considerations

### Memory Usage
- **v2.0**: 2 interrupt objects × N sensors = 2N objects
- **v3.0**: 1 interrupt object × N sensors = N objects
- **Overhead**: v3.0 has 2 function pointers vs v2.0's 1 per object
- **Net Result**: Similar memory usage, better cache locality

### Processing Overhead
- **v2.0**: Must match sensor state to correct interrupt
- **v3.0**: Direct state-to-function mapping
- **Result**: v3.0 is slightly more efficient

## Migration Impact

### From Current Simplified System
Both v2.0 and v3.0 require similar migration effort from the currently implemented simplified system, but v3.0 results in cleaner final architecture.

### Future Extensibility
v3.0 makes adding new sensors trivial:
```cpp
// Adding new sensor requires only one interrupt object
Interrupt newSensorInterrupt = {
    .activateFunc = &HandleNewSensorActive,
    .deactivateFunc = &HandleNewSensorInactive,
    .sensor = &newSensor
};
```

## Decision Factors Summary

| Factor | v2.0 Score | v3.0 Score | Winner |
|--------|------------|------------|--------|
| Conceptual Clarity | 6/10 | 9/10 | v3.0 |
| Override Logic Simplicity | 5/10 | 9/10 | v3.0 |
| Code Organization | 6/10 | 8/10 | v3.0 |
| Memory Usage | 7/10 | 8/10 | v3.0 |
| Implementation Effort | 7/10 | 7/10 | Tie |
| Future Extensibility | 6/10 | 9/10 | v3.0 |

## Risks and Mitigations

### Risks
1. **Function Pointer Overhead**: Two function pointers per interrupt vs one
2. **Implementation Complexity**: Need to handle dual-function dispatch
3. **Null Function Handling**: Some interrupts might only need activation or deactivation

### Mitigations
1. **Memory Impact**: Minimal - similar overall memory usage due to fewer objects
2. **Dispatch Logic**: Simple if/else based on sensor state
3. **Null Checks**: Standard null pointer checks before function calls

## Future Considerations

### Extensibility
v3.0 architecture naturally supports:
- Sensors with only activation (events)
- Sensors with only deactivation (cleanup)
- Full dual-state sensors (most common)

### Testing
v3.0 simplifies testing:
- Test one interrupt object with both functions
- Clear override scenarios
- Predictable state management

## Conclusion

**The dual-function approach (v3.0) is superior** because it eliminates the fundamental semantic confusion of v2.0 while providing cleaner organization and simpler override logic.

The key insight is that **sensors naturally have two states, but interrupts should be single logical entities** that handle both state transitions for their associated sensor.

This decision prioritizes:
1. **Conceptual Clarity** over implementation simplicity
2. **Long-term Maintainability** over short-term convenience  
3. **Natural Modeling** over technical optimization

## Implementation Recommendation

Proceed with v3.0 dual-function architecture for any future interrupt system redesign. The current simplified system (v1.0) works adequately for immediate needs, but v3.0 represents the ideal target architecture for long-term evolution.

---

**Decision Made By**: Architecture Review  
**Stakeholders**: System Architecture, Interrupt Management, Future Maintainers  
**Review Date**: To be scheduled when implementation is considered