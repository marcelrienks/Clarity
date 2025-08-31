# Interrupt System Simplification Plan

## Overview
Refactor the interrupt system from a complex evaluation/execution model to a simple single-purpose interrupt design with explicit activate/deactivate interrupts and global restoration tracking.

## Current Problems
1. **Complex dual-function design**: Each interrupt has separate evaluation and execution functions
2. **State management complexity**: Interrupts track their own state changes
3. **Restoration logic scattered**: Complex restoration tracking throughout the system
4. **Unclear intent**: Single interrupt handles both activation and deactivation
5. **Unnecessary evaluation phase**: Constantly checking states that could be event-driven

## Proposed Solution

### Core Principles
1. **Single Responsibility**: Each interrupt does exactly ONE thing when triggered
2. **Explicit Interrupts**: Separate interrupts for activate (GPIO HIGH) and deactivate (GPIO LOW)
3. **Global Restoration**: Single global variable tracks the last user-driven panel
4. **Simple Blocking**: Interrupts have a `blocking` flag to prevent restoration

### New Interrupt Structure
```cpp
struct Interrupt {
    const char* id;                    // Interrupt identifier
    Priority priority;                  // Processing priority
    InterruptSource source;             // POLLED or QUEUED
    void (*execute)(void* context);     // Single execution function
    void* context;                      // Sensor or service context
    bool blocking;                      // Does this block restoration?
    InterruptFlags flags;               // State flags
};
```

### Interrupt Types

#### 1. Activation Interrupts (GPIO HIGH)
- **Purpose**: Load a panel and save current for restoration
- **Example**: `lock_engaged`, `key_present`
- **Behavior**: 
  - Save current panel to global restoration variable
  - Load the interrupt's panel
  - Mark as blocking if it should prevent other restorations

#### 2. Deactivation Interrupts (GPIO LOW)
- **Purpose**: Trigger restoration to previous panel
- **Example**: `lock_disengaged`, `key_not_present`
- **Behavior**:
  - Check if any blocking interrupts are active
  - If none, restore to saved panel
  - If blocked, do nothing (blocking interrupt maintains control)

#### 3. Action Interrupts (Button/User)
- **Purpose**: Perform specific actions without panel changes
- **Example**: `short_press`, `long_press`
- **Behavior**: Execute panel-specific functions

## State Change Detection Model

### How It Works
1. **Sensors continuously monitor GPIO state** (in polling loop or ISR)
2. **On state CHANGE detection**:
   - HIGH transition → Trigger "active" interrupt (e.g., `lock_engaged`)
   - LOW transition → Trigger "inactive" interrupt (e.g., `lock_disengaged`)
3. **Each interrupt is self-contained**:
   - Active interrupts load panels and save restoration state
   - Inactive interrupts trigger restoration (if not blocked)

### State Change Flow Example
```
Initial: Lock GPIO = LOW, Panel = OilPanel
User locks steering → GPIO changes LOW→HIGH
  → Sensor detects change
  → Triggers "lock_engaged" interrupt
  → Interrupt saves OilPanel for restoration
  → Interrupt loads LockPanel
  
User unlocks → GPIO changes HIGH→LOW
  → Sensor detects change  
  → Triggers "lock_disengaged" interrupt
  → Interrupt checks for blocking interrupts
  → If none, restores OilPanel
```

### Important Distinctions
- **State Change Detection**: Still happens in sensors (required!)
- **State Evaluation**: REMOVED - we don't ask "what state are you in?"
- **Interrupt Trigger**: Only on transitions, not continuous state
- **Interrupt Execution**: Direct action, no evaluation phase

## Implementation Plan

### Phase 1: Core Structure Changes
1. **Simplify Interrupt struct**
   - Remove `evaluationFunc` and `executionFunc`
   - Add single `execute` function
   - Add `blocking` boolean flag
   - Remove complex effect data union

2. **Global Restoration Tracking**
   ```cpp
   class InterruptManager {
       static const char* savedUserPanel;  // Last user-driven panel
       static bool hasBlockingInterrupt(); // Check for active blockers
   };
   ```

### Phase 2: Interrupt Registration Updates
Convert existing interrupts to new model:

#### Before:
```cpp
Interrupt lockStateInterrupt = {
    .id = "lock_state",
    .evaluationFunc = LockStateEvaluate,
    .executionFunc = LockStateExecute,
    .data = {.panel = {.panelName = LOCK, .trackForRestore = true}}
};
```

#### After:
```cpp
// Two separate interrupts for lock
Interrupt lockEngagedInterrupt = {
    .id = "lock_engaged",
    .execute = LockEngagedExecute,  // Loads lock panel
    .blocking = true
};

Interrupt lockDisengagedInterrupt = {
    .id = "lock_disengaged", 
    .execute = LockDisengagedExecute,  // Triggers restoration
    .blocking = false
};
```

### Phase 3: Sensor Updates with State Change Detection

**Critical**: Sensors must still track state changes to trigger the appropriate interrupt!

```cpp
class LockSensor {
private:
    bool lastState = false;  // Track previous state
    
public:
    void Update() {
        bool currentState = gpioProvider->DigitalRead(LOCK_PIN);
        
        // Only trigger interrupt on STATE CHANGE
        if (currentState != lastState) {
            if (currentState == HIGH) {
                // State changed to HIGH - trigger engaged interrupt
                InterruptManager::TriggerInterrupt("lock_engaged");
            } else {
                // State changed to LOW - trigger disengaged interrupt  
                InterruptManager::TriggerInterrupt("lock_disengaged");
            }
            lastState = currentState;
        }
    }
};
```

**Key Points**:
- Sensors MUST track previous state
- Interrupts only trigger on state TRANSITIONS
- The interrupt ID matches the NEW state after transition
- No interrupt triggered if state hasn't changed

### Phase 4: Execution Functions

#### Activation Function Example:
```cpp
void LockEngagedExecute(void* context) {
    // Save current panel globally
    InterruptManager::SaveCurrentPanelForRestoration();
    
    // Load lock panel
    PanelManager::LoadPanel(PanelNames::LOCK);
}
```

#### Deactivation Function Example:
```cpp
void LockDisengagedExecute(void* context) {
    // Only restore if no blocking interrupts
    if (!InterruptManager::HasBlockingInterrupts()) {
        const char* savedPanel = InterruptManager::GetSavedPanel();
        if (savedPanel) {
            PanelManager::LoadPanel(savedPanel);
        }
    }
}
```

### Phase 5: Simplified Processing Loop
```cpp
void InterruptManager::Process() {
    // No evaluation phase needed!
    
    // Process triggered interrupts directly
    for (auto& interrupt : triggeredInterrupts) {
        interrupt.execute(interrupt.context);
    }
    
    // Clear processed interrupts
    triggeredInterrupts.clear();
}
```

## Benefits

### 1. **Simplicity**
- One interrupt = one action
- No complex state tracking
- Clear, linear execution flow

### 2. **Clarity**
- `lock_engaged` vs `lock_disengaged` is self-documenting
- No ambiguity about what each interrupt does
- Easy to debug and trace

### 3. **Performance**
- No constant evaluation loops
- Event-driven execution only
- Reduced CPU usage

### 4. **Maintainability**
- Adding new interrupts is straightforward
- Each interrupt is independent
- Restoration logic is centralized

### 5. **Testability**
- Each interrupt can be tested in isolation
- No complex state dependencies
- Clear input → output relationships

## Migration Strategy

1. **Create new interrupt structure** alongside existing
2. **Implement global restoration tracking**
3. **Convert one sensor at a time** (start with Lock)
4. **Test each conversion thoroughly**
5. **Remove old evaluation/execution system** once all converted
6. **Update documentation** to reflect new architecture

## Example: Complete Lock System

```cpp
// Registration
interruptManager->RegisterInterrupt(new Interrupt{
    .id = "lock_engaged",
    .priority = Priority::IMPORTANT,
    .source = InterruptSource::POLLED,
    .execute = [](void* ctx) {
        InterruptManager::SavePanelForRestoration();
        PanelManager::LoadPanel(PanelNames::LOCK);
    },
    .blocking = true
});

interruptManager->RegisterInterrupt(new Interrupt{
    .id = "lock_disengaged",
    .priority = Priority::IMPORTANT,
    .source = InterruptSource::POLLED,
    .execute = [](void* ctx) {
        if (!InterruptManager::HasBlockingInterrupts()) {
            PanelManager::RestorePanel();
        }
    },
    .blocking = false
});

// Sensor triggers
void LockSensor::Update() {
    bool currentState = ReadGPIO();
    if (currentState != lastState) {
        if (currentState) {
            InterruptManager::Trigger("lock_engaged");
        } else {
            InterruptManager::Trigger("lock_disengaged");
        }
        lastState = currentState;
    }
}
```

## Success Criteria

1. **All interrupts converted** to single-purpose design
2. **Restoration works correctly** for all panel transitions
3. **Blocking interrupts** properly prevent unwanted restorations
4. **Code complexity reduced** by at least 40%
5. **Performance improved** (lower CPU usage in main loop)
6. **Tests pass** for all interrupt scenarios

## Timeline Estimate

- Phase 1: 2 hours (core structure)
- Phase 2: 3 hours (interrupt registration)
- Phase 3: 2 hours (sensor updates)
- Phase 4: 2 hours (execution functions)
- Phase 5: 1 hour (simplified loop)
- Testing: 2 hours
- **Total: ~12 hours**

## Risks and Mitigation

1. **Risk**: Breaking existing functionality
   - **Mitigation**: Implement alongside existing system, switch over gradually

2. **Risk**: Missing edge cases in restoration
   - **Mitigation**: Comprehensive testing of all interrupt combinations

3. **Risk**: Performance regression
   - **Mitigation**: Profile before and after changes

## Notes

- This simplification aligns with the principle of "make it as simple as possible, but no simpler"
- The current evaluation/execution separation adds complexity without clear benefits
- Event-driven interrupts are more intuitive than state-polling
- Global restoration is easier to reason about than distributed restoration logic