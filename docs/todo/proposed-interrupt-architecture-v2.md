# Proposed Interrupt Architecture v2.0

## Executive Summary

This document outlines a proposed refined interrupt architecture that further simplifies the system by establishing clear single responsibilities for sensors and interrupts, with a sophisticated override mechanism for proper restoration behavior.

## Core Principles

### 1. Sensor Single Responsibility
**Sensors have ONE responsibility**: Track GPIO pin state changes.

```cpp
class BaseSensor {
private:
    SensorState previousState_ = SensorState::INACTIVE;
    SensorState currentState_ = SensorState::INACTIVE;
    
public:
    bool HasStateChanged() {
        SensorState newState = ReadCurrentState(); // Implementation specific
        
        if (previousState_ != newState) {
            previousState_ = currentState_;
            currentState_ = newState;
            return true;
        }
        return false;
    }
    
    SensorState GetCurrentState() const { return currentState_; }
    SensorState GetPreviousState() const { return previousState_; }
    
protected:
    virtual SensorState ReadCurrentState() = 0; // GPIO read implementation
};

enum class SensorState {
    INACTIVE = 0,  // GPIO LOW / false / off
    ACTIVE = 1     // GPIO HIGH / true / on
};
```

### 2. Interrupt Single Responsibility
**Interrupts have ONE responsibility**: Execute an injected function when triggered.

```cpp
struct Interrupt {
    const char* id;                          // Unique identifier
    Priority priority;                       // Execution priority
    void (*execute)(void* context);          // Injected function
    void* context;                           // Execution context
    
    // State association
    BaseSensor* sensor;                      // Associated sensor (1:1)
    SensorState triggerState;                // ACTIVE or INACTIVE
    
    // Override behavior
    bool canBeOverridden;                    // Can other interrupts override this?
    bool isActive;                           // Currently active flag
    
    // Helper methods
    bool ShouldTrigger() const {
        return sensor && sensor->GetCurrentState() == triggerState;
    }
    
    void SetActive(bool active) { isActive = active; }
    bool IsActive() const { return isActive; }
};
```

### 3. Sensor-Interrupt Relationship
- **One sensor can be associated with multiple interrupts** (activate + deactivate)
- **One interrupt can only be associated with one sensor** (1:1 mapping)
- **Each interrupt maps to exactly one sensor state** (ACTIVE or INACTIVE)

## Architecture Mappings

### Example: Lock Sensor
```cpp
// One sensor
LockSensor lockSensor;

// Two interrupts - one for each state
Interrupt lockEngagedInterrupt = {
    .id = "lock_engaged",
    .priority = Priority::IMPORTANT,
    .execute = &LoadLockPanel,
    .context = &panelManager,
    .sensor = &lockSensor,
    .triggerState = SensorState::ACTIVE,     // GPIO HIGH
    .canBeOverridden = false,                // Cannot be overridden
    .isActive = false
};

Interrupt lockDisengagedInterrupt = {
    .id = "lock_disengaged", 
    .priority = Priority::IMPORTANT,
    .execute = &CheckRestoration,
    .context = nullptr,
    .sensor = &lockSensor,
    .triggerState = SensorState::INACTIVE,   // GPIO LOW
    .canBeOverridden = true,                 // Can be overridden
    .isActive = false
};
```

### Example: Lights Sensor
```cpp
// One sensor
LightsSensor lightsSensor;

// Two interrupts - one for each state
Interrupt lightsOnInterrupt = {
    .id = "lights_on",
    .priority = Priority::NORMAL,
    .execute = &SetNightTheme,
    .context = &styleManager,
    .sensor = &lightsSensor,
    .triggerState = SensorState::ACTIVE,     // GPIO HIGH (lights on)
    .canBeOverridden = true,                 // Can be overridden
    .isActive = false
};

Interrupt lightsOffInterrupt = {
    .id = "lights_off",
    .priority = Priority::NORMAL, 
    .execute = &SetDayTheme,
    .context = &styleManager,
    .sensor = &lightsSensor,
    .triggerState = SensorState::INACTIVE,   // GPIO LOW (lights off)
    .canBeOverridden = false,                // Cannot be overridden (theme restoration)
    .isActive = false
};
```

## Override Logic Implementation

### Processing Flow
1. **Sensor State Change Detection**: Check all sensors for state changes
2. **Interrupt Identification**: Find interrupts that should trigger based on new sensor state
3. **Override Resolution**: Apply override logic before execution
4. **Execution**: Execute the final resolved interrupt

### Override Resolution Algorithm
```cpp
class InterruptManager {
public:
    void ProcessSensorStateChanges() {
        for (auto* sensor : sensors_) {
            if (sensor->HasStateChanged()) {
                ProcessSensorChange(*sensor);
            }
        }
    }
    
private:
    void ProcessSensorChange(const BaseSensor& sensor) {
        // Find interrupts that should trigger for this sensor's new state
        std::vector<Interrupt*> candidateInterrupts;
        for (auto& interrupt : interrupts_) {
            if (interrupt.sensor == &sensor && 
                interrupt.ShouldTrigger()) {
                candidateInterrupts.push_back(&interrupt);
            }
        }
        
        // Apply override logic
        Interrupt* interruptToExecute = ResolveOverrides(candidateInterrupts);
        
        if (interruptToExecute) {
            ExecuteInterrupt(*interruptToExecute);
        }
    }
    
    Interrupt* ResolveOverrides(const std::vector<Interrupt*>& candidates) {
        if (candidates.empty()) return nullptr;
        if (candidates.size() == 1) return candidates[0];
        
        // Check for active non-overridable interrupts
        for (auto* activeInterrupt : GetActiveInterrupts()) {
            if (!activeInterrupt->canBeOverridden) {
                // Non-overridable interrupt is active
                // Find if any candidate can activate this interrupt instead
                for (auto* candidate : candidates) {
                    if (candidate->triggerState == SensorState::ACTIVE && 
                        SameEffectAs(*candidate, *activeInterrupt)) {
                        return activeInterrupt; // Re-execute the non-overridable
                    }
                }
                return nullptr; // Block execution
            }
        }
        
        // No blocking interrupts, execute highest priority candidate
        return *std::max_element(candidates.begin(), candidates.end(),
            [](const Interrupt* a, const Interrupt* b) {
                return static_cast<int>(a->priority) > static_cast<int>(b->priority);
            });
    }
};
```

## Concrete Example Scenarios

### Scenario 1: Key Present → Lock Engaged → Lock Disengaged
1. **Key Present Activates**: Load KEY panel, set `keyPresentInterrupt.isActive = true`
2. **Lock Engaged**: Try to load LOCK panel, but `keyPresentInterrupt.canBeOverridden = false`
   - Result: Re-execute Key Present (stay on KEY panel)
3. **Lock Disengaged**: Check overrides, Key Present still active and non-overridable
   - Result: Re-execute Key Present (stay on KEY panel)

### Scenario 2: Lights On → Lock Engaged → Lock Disengaged  
1. **Lights On**: Set NIGHT theme, set `lightsOnInterrupt.isActive = true`
2. **Lock Engaged**: Load LOCK panel (overrides lights), set `lockEngagedInterrupt.isActive = true`
3. **Lock Disengaged**: Check overrides, Lights On is active but `canBeOverridden = true`
   - Result: Execute restoration logic → restore to saved panel or re-apply NIGHT theme

### Scenario 3: Normal Operation → Error Occurs → Error Clears
1. **Normal State**: User browsing OIL panel
2. **Error Occurs**: Load ERROR panel, set `errorInterrupt.isActive = true`, `canBeOverridden = false`
3. **User presses button**: Button interrupt blocked by non-overridable error
4. **Error Clears**: Restore to saved OIL panel

## Interrupt Configurations

| Sensor | Interrupt | Trigger State | Can Be Overridden | Effect |
|--------|-----------|---------------|-------------------|--------|
| KeyPresent | key_present_activate | ACTIVE | false | Load KEY panel |
| KeyPresent | key_present_deactivate | INACTIVE | true | Check restoration |
| KeyNotPresent | key_not_present_activate | ACTIVE | false | Load KEY panel |
| KeyNotPresent | key_not_present_deactivate | INACTIVE | true | Check restoration |
| Lock | lock_engaged | ACTIVE | false | Load LOCK panel |
| Lock | lock_disengaged | INACTIVE | true | Check restoration |
| Lights | lights_on | ACTIVE | true | Set NIGHT theme |
| Lights | lights_off | INACTIVE | false | Set DAY theme |
| Error | error_occurred | ACTIVE | false | Load ERROR panel |
| Error | error_cleared | INACTIVE | true | Check restoration |
| Button | short_press | ACTIVE | true | Cycle panels |
| Button | long_press | ACTIVE | true | Toggle theme |

## State Management Rules

### Active Flag Management
```cpp
void ExecuteInterrupt(Interrupt& interrupt) {
    // Set this interrupt as active
    interrupt.SetActive(true);
    
    // If this interrupt loads a panel, deactivate others that load panels
    if (LoadsPanel(interrupt)) {
        DeactivateConflictingInterrupts(interrupt);
    }
    
    // Execute the injected function
    interrupt.execute(interrupt.context);
}

void DeactivateConflictingInterrupts(const Interrupt& newInterrupt) {
    for (auto& interrupt : interrupts_) {
        if (&interrupt != &newInterrupt && 
            LoadsPanel(interrupt) && 
            interrupt.IsActive()) {
            interrupt.SetActive(false);
        }
    }
}
```

### Critical Issue: Inactive State Management

⚠️ **INCOMPLETE DESIGN ISSUE**: The current proposal has a fundamental problem with inactive state management.

**Problem**: If we set `isActive = true` for deactivation interrupts (INACTIVE state), then:
- `lights_off` interrupt would be active when lights are off (normal operation)  
- `lock_disengaged` interrupt would be active when lock is disengaged (normal operation)
- `error_cleared` interrupt would be active when no errors exist (normal operation)

This means most "deactivate" interrupts would be active during normal operation, which defeats the purpose of the active flag for override logic.

**Proposed Solutions** (requires further analysis):

1. **Separate Active Tracking**: 
   - `isActive` only applies to "activation" interrupts
   - Deactivation interrupts never set `isActive = true`
   - Override logic only considers activation interrupts

2. **State-Specific Override Logic**:
   - Different override rules for ACTIVE vs INACTIVE trigger states
   - ACTIVE interrupts use override logic
   - INACTIVE interrupts use simple restoration logic

3. **Effect-Based Grouping**:
   - Group interrupts by effect (panel loading, theme setting, etc.)
   - Only one interrupt per effect group can be active
   - Simplifies override logic

## Migration Path

This architecture represents a significant departure from both the original and the recently implemented simplified system. Key changes required:

### Phase 1: Sensor Refactoring
- Standardize all sensors to use `SensorState` enum
- Implement consistent `HasStateChanged()` logic
- Remove sensor-specific trigger logic

### Phase 2: Interrupt Registration Redesign
- Create separate interrupts for each sensor state
- Implement `canBeOverridden` property
- Add sensor-interrupt association

### Phase 3: Override System Implementation
- Implement override resolution algorithm
- Add active interrupt tracking
- Create conflict detection logic

### Phase 4: State Management
- Resolve inactive state management issue
- Implement proper active flag semantics
- Test all override scenarios

## Benefits

1. **Extreme Simplicity**: Each component has exactly one responsibility
2. **Systematic Design**: Consistent patterns across all sensors/interrupts
3. **Flexible Override Logic**: Sophisticated restoration behavior
4. **Self-Documenting**: Clear naming conventions and state mappings
5. **Testable**: Each component can be tested in isolation
6. **Extensible**: Adding new sensors/interrupts follows the same pattern

## Risks and Considerations

1. **Complexity of Override Logic**: The resolution algorithm is non-trivial
2. **State Management**: The inactive state issue needs resolution
3. **Performance**: More interrupts means more processing overhead  
4. **Memory Usage**: Double the number of interrupts
5. **Breaking Changes**: Complete architectural redesign required

## Recommendation

This architecture proposal shows promise but requires resolution of the inactive state management issue before implementation. Consider prototyping the override resolution algorithm to validate the approach before committing to a full migration.

The design philosophy is sound: maximum simplification of individual components with sophisticated coordination logic. This follows the Unix philosophy of "do one thing well" while providing the complex behaviors needed for the automotive gauge system.

## Next Steps

1. **Resolve Inactive State Issue**: Define clear semantics for deactivation interrupts
2. **Prototype Override Algorithm**: Validate the resolution logic with test scenarios
3. **Define Migration Strategy**: Plan incremental transition from current system
4. **Performance Analysis**: Ensure the overhead is acceptable for ESP32 constraints
5. **Create Comprehensive Test Suite**: Cover all override scenarios and edge cases