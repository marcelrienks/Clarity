# Proposed Interrupt Architecture v3.0 - Dual-State Interrupts

## Executive Summary

This document refines the v2.0 proposal by consolidating activation/deactivation into single dual-state interrupts, eliminating the complexity of separate interrupt types while maintaining sophisticated override behavior.

## Core Insight

**Instead of separate interrupts for each sensor state, have one interrupt per sensor with dual functions:**

```cpp
struct Interrupt {
    const char* id;                          // Unique identifier
    Priority priority;                       // Execution priority
    
    // Dual-state functions
    void (*activateFunc)(void* context);     // Execute when sensor goes ACTIVE
    void (*deactivateFunc)(void* context);   // Execute when sensor goes INACTIVE
    void* context;                           // Execution context
    
    // State association
    BaseSensor* sensor;                      // Associated sensor (1:1)
    
    // Override behavior - only applies to activation
    bool canBeOverriddenOnActivate;          // Can other interrupts override activation?
    bool isActive;                           // Currently active (set by activation only)
    
    // Execution methods
    void ExecuteActivate() {
        if (activateFunc) {
            activateFunc(context);
            isActive = true;  // Only activation sets active flag
        }
    }
    
    void ExecuteDeactivate() {
        if (deactivateFunc) {
            deactivateFunc(context);
            // Deactivation never sets isActive = true
            // But may set isActive = false for this interrupt
            isActive = false;
        }
    }
};
```

## Elegant Mappings

### Lock Interrupt (Single interrupt, dual functions)
```cpp
Interrupt lockInterrupt = {
    .id = "lock",
    .priority = Priority::IMPORTANT,
    .activateFunc = &LoadLockPanel,          // GPIO HIGH: Load LOCK panel
    .deactivateFunc = &CheckRestoration,     // GPIO LOW: Check restoration
    .context = &panelManager,
    .sensor = &lockSensor,
    .canBeOverriddenOnActivate = false,      // Lock cannot be overridden
    .isActive = false
};
```

### Lights Interrupt (Single interrupt, dual functions)
```cpp
Interrupt lightsInterrupt = {
    .id = "lights",
    .priority = Priority::NORMAL,
    .activateFunc = &SetNightTheme,          // GPIO HIGH: Night theme
    .deactivateFunc = &SetDayTheme,          // GPIO LOW: Day theme  
    .context = &styleManager,
    .sensor = &lightsSensor,
    .canBeOverriddenOnActivate = true,       // Lights can be overridden
    .isActive = false
};
```

### Key Present Interrupt
```cpp
Interrupt keyPresentInterrupt = {
    .id = "key_present",
    .priority = Priority::CRITICAL,
    .activateFunc = &LoadKeyPanel,           // Key detected: Load KEY panel
    .deactivateFunc = &CheckRestoration,     // Key removed: Check restoration
    .context = &panelManager,
    .sensor = &keyPresentSensor,
    .canBeOverriddenOnActivate = false,      // Key cannot be overridden
    .isActive = false
};
```

## Processing Flow

### Sensor State Change Detection
```cpp
class InterruptManager {
public:
    void ProcessSensorStateChanges() {
        for (auto& interrupt : interrupts_) {
            if (interrupt.sensor && interrupt.sensor->HasStateChanged()) {
                ProcessInterruptStateChange(interrupt);
            }
        }
    }
    
private:
    void ProcessInterruptStateChange(Interrupt& interrupt) {
        SensorState currentState = interrupt.sensor->GetCurrentState();
        
        if (currentState == SensorState::ACTIVE) {
            HandleActivation(interrupt);
        } else {
            HandleDeactivation(interrupt);
        }
    }
    
    void HandleActivation(Interrupt& interrupt) {
        // Check if this activation can be overridden
        Interrupt* blockingInterrupt = FindBlockingInterrupt(interrupt);
        
        if (blockingInterrupt) {
            // Re-execute the blocking interrupt instead
            log_d("Activation blocked by '%s', re-executing blocker", blockingInterrupt->id);
            blockingInterrupt->ExecuteActivate();
        } else {
            // Execute activation
            log_d("Executing activation for '%s'", interrupt.id);
            interrupt.ExecuteActivate();
        }
    }
    
    void HandleDeactivation(Interrupt& interrupt) {
        // Deactivation is simpler - no override logic needed
        log_d("Executing deactivation for '%s'", interrupt.id);
        interrupt.ExecuteDeactivate();
    }
    
    Interrupt* FindBlockingInterrupt(const Interrupt& candidateInterrupt) {
        // Only activation can be blocked, and only by non-overridable active interrupts
        if (candidateInterrupt.canBeOverriddenOnActivate) {
            return nullptr; // This interrupt can be overridden
        }
        
        // Look for active interrupts that cannot be overridden
        for (auto& activeInterrupt : interrupts_) {
            if (activeInterrupt.isActive && 
                !activeInterrupt.canBeOverriddenOnActivate &&
                &activeInterrupt != &candidateInterrupt) {
                return &activeInterrupt;
            }
        }
        
        return nullptr; // No blocking interrupts found
    }
};
```

## Benefits of Dual-State Approach

### 1. **Eliminates Type Complexity**
- No need for separate activation/deactivation interrupt types
- Single interrupt structure handles both states
- Simpler registration and management

### 2. **Clear State Semantics**
- `isActive` flag only set by activation functions
- Deactivation functions never set `isActive = true`
- Override logic only applies to activation (where it makes sense)

### 3. **Natural Sensor Mapping**
- One interrupt per sensor (1:1 relationship)
- Both sensor states handled in single object
- Cleaner conceptual model

### 4. **Simplified Override Logic**
- Only activation needs override checking
- Deactivation is always straightforward
- No ambiguity about when overrides apply

## Example Scenarios

### Scenario 1: Key Present → Lock Engaged → Lock Disengaged
1. **Key Present Activates**: 
   - `keyPresentInterrupt.ExecuteActivate()` → Load KEY panel
   - `keyPresentInterrupt.isActive = true`

2. **Lock Sensor Goes HIGH**:
   - Check: Can `lockInterrupt` activation be overridden? **No**
   - Check: Are there blocking active interrupts? **Yes** (keyPresentInterrupt)
   - Result: Re-execute `keyPresentInterrupt.ExecuteActivate()` → Stay on KEY panel

3. **Lock Sensor Goes LOW**:
   - `lockInterrupt.ExecuteDeactivate()` → Check restoration
   - Key still present, so restoration blocked → Stay on KEY panel

### Scenario 2: Lights On → Lock Engaged → Lock Disengaged
1. **Lights Activates**:
   - `lightsInterrupt.ExecuteActivate()` → Set NIGHT theme
   - `lightsInterrupt.isActive = true`

2. **Lock Sensor Goes HIGH**:
   - Check: Can `lockInterrupt` activation be overridden? **No**
   - Check: Are there blocking active interrupts? **No** (lights can be overridden)
   - Result: `lockInterrupt.ExecuteActivate()` → Load LOCK panel
   - `lockInterrupt.isActive = true`, `lightsInterrupt.isActive = false`

3. **Lock Sensor Goes LOW**:
   - `lockInterrupt.ExecuteDeactivate()` → Check restoration
   - Lights still on, so `lightsInterrupt.ExecuteActivate()` → Restore NIGHT theme

## Implementation Structure

### Complete Interrupt Definitions
```cpp
std::vector<Interrupt> systemInterrupts = {
    // Key interrupts
    {
        .id = "key_present",
        .priority = Priority::CRITICAL,
        .activateFunc = &LoadKeyPanel,
        .deactivateFunc = &CheckRestoration,
        .context = &panelManager,
        .sensor = &keyPresentSensor,
        .canBeOverriddenOnActivate = false,
        .isActive = false
    },
    {
        .id = "key_not_present", 
        .priority = Priority::CRITICAL,
        .activateFunc = &LoadKeyPanel,
        .deactivateFunc = &CheckRestoration,
        .context = &panelManager,
        .sensor = &keyNotPresentSensor,
        .canBeOverriddenOnActivate = false,
        .isActive = false
    },
    
    // Lock interrupt
    {
        .id = "lock",
        .priority = Priority::IMPORTANT,
        .activateFunc = &LoadLockPanel,
        .deactivateFunc = &CheckRestoration,
        .context = &panelManager,
        .sensor = &lockSensor,
        .canBeOverriddenOnActivate = false,
        .isActive = false
    },
    
    // Lights interrupt
    {
        .id = "lights",
        .priority = Priority::NORMAL,
        .activateFunc = &SetNightTheme,
        .deactivateFunc = &SetDayTheme,
        .context = &styleManager,
        .sensor = &lightsSensor,
        .canBeOverriddenOnActivate = true,
        .isActive = false
    },
    
    // Error interrupt
    {
        .id = "error",
        .priority = Priority::CRITICAL,
        .activateFunc = &LoadErrorPanel,
        .deactivateFunc = &CheckRestoration,
        .context = &panelManager,
        .sensor = &errorSensor,
        .canBeOverriddenOnActivate = false,
        .isActive = false
    }
};
```

### Button Handling (Special Case)
Buttons are different because they don't have persistent states - they generate events:

```cpp
// Button interrupts are event-based, not state-based
Interrupt shortPressInterrupt = {
    .id = "short_press",
    .priority = Priority::IMPORTANT,
    .activateFunc = &HandleShortPress,       // Execute action
    .deactivateFunc = nullptr,               // No deactivation
    .context = &panelManager,
    .sensor = &buttonSensor,                 // Special event sensor
    .canBeOverriddenOnActivate = true,       // Can be blocked
    .isActive = false                        // Never stays active
};
```

## Memory and Performance

### Memory Usage
- **Reduction**: Half the number of interrupt objects vs v2.0
- **Overhead**: Two function pointers per interrupt instead of one
- **Net Result**: Similar memory usage with better organization

### Processing Overhead
- **Simpler Logic**: Single processing path per sensor
- **Reduced Complexity**: No need to match states to separate interrupts
- **Better Cache**: Related functions stored together

## Migration from Current System

### Phase 1: Restructure Interrupts
- Convert current single-function interrupts to dual-function
- Pair activation/deactivation functions
- Add `canBeOverriddenOnActivate` property

### Phase 2: Implement Override Logic
- Add blocking interrupt detection
- Implement activation override system
- Test all override scenarios

### Phase 3: Sensor Integration
- Ensure sensors use consistent state detection
- Link each interrupt to its sensor
- Validate state change processing

## Comparison with Previous Approaches

| Aspect | Original | v1.0 Simplified | v2.0 Dual Interrupts | **v3.0 Dual Functions** |
|--------|----------|-----------------|----------------------|------------------------|
| Interrupts per Sensor | 1 (complex) | 2 (separate) | 2 (separate) | **1 (dual-function)** |
| Override Logic | Complex effects | Global restoration | Per-interrupt flags | **Activation-only** |
| State Semantics | Unclear | Direct execution | Confusing inactive | **Clear active-only** |
| Code Complexity | High | Medium | Medium | **Low** |
| Conceptual Model | Tangled | Simple | Systematic | **Elegant** |

## Recommendation

**This v3.0 dual-function approach is superior** because it:

1. **Eliminates unnecessary duplication** (separate activate/deactivate interrupts)
2. **Provides clear semantics** (only activation sets isActive)  
3. **Simplifies override logic** (only activation can be overridden)
4. **Maintains 1:1 sensor mapping** (one interrupt per sensor)
5. **Reduces cognitive load** (related functions grouped together)

The architecture is both simpler to understand and more powerful in capability than any previous iteration.

## Next Steps

1. **Prototype the override algorithm** with test scenarios
2. **Define all function pairs** for existing interrupts  
3. **Plan migration strategy** from current simplified system
4. **Performance test** on ESP32 with actual GPIO timing
5. **Create comprehensive test suite** covering all override cases

This represents the most elegant solution so far - the right balance of simplicity and sophistication.