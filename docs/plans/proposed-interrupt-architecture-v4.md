# Proposed Interrupt Architecture v4.0 - Trigger/Action Separation

## Executive Summary

This document refines the v3.0 proposal by introducing a clear separation between **Triggers** (state-based interrupts from GPIO sensors) and **Actions** (event-based interrupts from buttons). This separation acknowledges the fundamental difference between persistent state changes and momentary events.

## Core Design Principle

**Two distinct interrupt types based on behavior:**
- **Triggers**: State-based interrupts with activate/deactivate functions (GPIO sensors)
- **Actions**: Event-based interrupts with single execution (button presses)

## Interface Definitions

### Trigger Interface (State-Based)
Located in `include/types.h`:

```cpp
// Forward declarations
class BaseSensor;

enum class Priority : uint8_t {
    NORMAL = 0,      // Lowest priority (e.g., lights)
    IMPORTANT = 1,   // Medium priority (e.g., lock)
    CRITICAL = 2     // Highest priority (e.g., key, errors)
};

struct Trigger {
    const char* id;                          // Unique identifier
    Priority priority;                       // Execution priority (CRITICAL > IMPORTANT > NORMAL)
    
    // Dual-state functions - no context needed
    void (*activateFunc)();                  // Execute when sensor goes ACTIVE
    void (*deactivateFunc)();                // Execute when sensor goes INACTIVE
    
    // State association
    BaseSensor* sensor;                      // Associated sensor (1:1)
    
    // Override behavior - only applies to activation
    bool canBeOverriddenOnActivate;          // Can other triggers override activation?
    bool isActive;                           // Currently active (set by activation only)
    
    // Execution methods
    void ExecuteActivate() {
        if (activateFunc) {
            activateFunc();
            isActive = true;  // Only activation sets active flag
        }
    }
    
    void ExecuteDeactivate() {
        if (deactivateFunc) {
            deactivateFunc();
            isActive = false;  // Clear active flag on deactivation
        }
    }
};
```

### Action Interface (Event-Based)
Located in `include/types.h`:

```cpp
enum class ActionPress : uint8_t {
    SHORT = 0,
    LONG = 1
};

struct Action {
    const char* id;                          // Unique identifier
    
    // Event function - no context needed
    void (*executeFunc)();                   // Execute when action triggered
    
    // Event state
    bool hasTriggered;                       // Action pending execution
    ActionPress pressType;                   // SHORT or LONG press
    
    // No priority - Actions are processed in order
    // No sensor association - managed by ActionHandler
    // No override logic - Actions cannot block each other
    
    // Execution method
    void Execute() {
        if (executeFunc && hasTriggered) {
            executeFunc();
            hasTriggered = false;  // Clear trigger flag after execution
        }
    }
};
```

## Handler Architecture Integration

### TriggerHandler (formerly PolledHandler)
```cpp
class TriggerHandler : public IHandler {
private:
    std::vector<Trigger> triggers_;
    // Handler owns GPIO sensors
    std::unique_ptr<KeyPresentSensor> keyPresentSensor_;
    std::unique_ptr<KeyNotPresentSensor> keyNotPresentSensor_;
    std::unique_ptr<LockSensor> lockSensor_;
    std::unique_ptr<LightsSensor> lightsSensor_;
    
public:
    void Evaluate() override {
        // Only evaluate during UI idle
        if (!IsUIIdle()) return;
        
        for (auto& trigger : triggers_) {
            if (trigger.sensor && trigger.sensor->HasStateChanged()) {
                ProcessTriggerStateChange(trigger);
            }
        }
    }
    
private:
    void ProcessTriggerStateChange(Trigger& trigger) {
        SensorState currentState = trigger.sensor->GetCurrentState();
        
        if (currentState == SensorState::ACTIVE) {
            HandleActivation(trigger);
        } else {
            HandleDeactivation(trigger);
        }
    }
    
    void HandleActivation(Trigger& trigger) {
        // Check if this activation can be overridden
        Trigger* blockingTrigger = FindBlockingTrigger(trigger);
        
        if (blockingTrigger) {
            // Re-execute the blocking trigger instead
            log_d("Activation of '%s' blocked by '%s'", trigger.id, blockingTrigger->id);
            blockingTrigger->ExecuteActivate();
        } else {
            log_d("Executing activation for '%s'", trigger.id);
            trigger.ExecuteActivate();
        }
    }
    
    void HandleDeactivation(Trigger& trigger) {
        log_d("Executing deactivation for '%s'", trigger.id);
        trigger.ExecuteDeactivate();
    }
    
    Trigger* FindBlockingTrigger(const Trigger& candidate) {
        if (candidate.canBeOverriddenOnActivate) {
            return nullptr; // This trigger allows overrides
        }
        
        // Look for higher priority active triggers that cannot be overridden
        for (auto& activeTrigger : triggers_) {
            if (activeTrigger.isActive && 
                !activeTrigger.canBeOverriddenOnActivate &&
                activeTrigger.priority > candidate.priority &&
                &activeTrigger != &candidate) {
                return &activeTrigger;
            }
        }
        
        return nullptr;
    }
};
```

### ActionHandler (formerly QueuedHandler)
```cpp
class ActionHandler : public IHandler {
private:
    std::vector<Action> actions_;
    std::unique_ptr<ActionButtonSensor> actionButtonSensor_;  // Handler owns action sensor
    
    // Button timing constants
    static constexpr uint32_t SHORT_PRESS_MIN_MS = 50;
    static constexpr uint32_t SHORT_PRESS_MAX_MS = 2000;
    static constexpr uint32_t LONG_PRESS_MIN_MS = 2000;
    static constexpr uint32_t LONG_PRESS_MAX_MS = 5000;
    
public:
    void Evaluate() override {
        // Always evaluate button state (not just during idle)
        if (actionButtonSensor_->HasStateChanged()) {
            uint32_t pressDuration = actionButtonSensor_->GetPressDuration();
            
            if (pressDuration >= SHORT_PRESS_MIN_MS && pressDuration < SHORT_PRESS_MAX_MS) {
                QueueAction(ActionPress::SHORT);
            } else if (pressDuration >= LONG_PRESS_MIN_MS && pressDuration <= LONG_PRESS_MAX_MS) {
                QueueAction(ActionPress::LONG);
            }
        }
    }
    
    void Execute() override {
        // Only execute during UI idle
        if (!IsUIIdle()) return;
        
        // Process all pending actions in order
        for (auto& action : actions_) {
            action.Execute();
        }
    }
    
private:
    void QueueAction(ActionPress type) {
        // Find the appropriate action and mark it as triggered
        for (auto& action : actions_) {
            if ((type == ActionPress::SHORT && strcmp(action.id, "short_press") == 0) ||
                (type == ActionPress::LONG && strcmp(action.id, "long_press") == 0)) {
                action.hasTriggered = true;
                action.pressType = type;
                break;
            }
        }
    }
};
```

## Complete System Definition

### Trigger Definitions
```cpp
// In InterruptManager or TriggerHandler initialization
std::vector<Trigger> systemTriggers = {
    // Key triggers
    {
        .id = "key_present",
        .priority = Priority::CRITICAL,
        .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::KEY); },
        .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
        .sensor = keyPresentSensor.get(),
        .canBeOverriddenOnActivate = false,
        .isActive = false
    },
    {
        .id = "key_not_present", 
        .priority = Priority::CRITICAL,
        .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::KEY); },
        .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
        .sensor = keyNotPresentSensor.get(),
        .canBeOverriddenOnActivate = false,
        .isActive = false
    },
    
    // Lock trigger
    {
        .id = "lock",
        .priority = Priority::IMPORTANT,
        .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::LOCK); },
        .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
        .sensor = lockSensor.get(),
        .canBeOverriddenOnActivate = false,
        .isActive = false
    },
    
    // Lights trigger
    {
        .id = "lights",
        .priority = Priority::NORMAL,
        .activateFunc = []() { StyleManager::Instance().SetTheme(Theme::NIGHT); },
        .deactivateFunc = []() { StyleManager::Instance().SetTheme(Theme::DAY); },
        .sensor = lightsSensor.get(),
        .canBeOverriddenOnActivate = true,  // Can be overridden by higher priority
        .isActive = false
    },
    
    // Error trigger
    {
        .id = "error",
        .priority = Priority::CRITICAL,
        .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::ERROR); },
        .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
        .sensor = errorSensor.get(),
        .canBeOverriddenOnActivate = false,
        .isActive = false
    }
};
```

### Action Definitions
```cpp
// In InterruptManager or ActionHandler initialization
std::vector<Action> systemActions = {
    // Button actions
    {
        .id = "short_press",
        .executeFunc = []() { PanelManager::Instance().HandleShortPress(); },
        .hasTriggered = false,
        .pressType = ActionPress::SHORT
    },
    {
        .id = "long_press",
        .executeFunc = []() { PanelManager::Instance().HandleLongPress(); },
        .hasTriggered = false,
        .pressType = ActionPress::LONG
    }
};
```

## Error Handling Strategy

### Standard Error Flow
1. **Function Execution Failure**: 
   - Caught by try-catch in Execute methods
   - Error logged via ErrorManager
   - Error trigger activated if critical

2. **Sensor Read Failure**:
   - Sensor returns UNKNOWN state
   - State change ignored for that cycle
   - Error logged for diagnostics

3. **Error Panel Display**:
   ```cpp
   void HandleExecutionError(const char* interruptId, const std::exception& e) {
       ErrorManager::Instance().ReportError(ErrorLevel::ERROR, 
                                        "InterruptSystem", 
                                        "Interrupt " + std::string(interruptId) + " failed: " + e.what());
       
       // Find and activate error trigger
       for (auto& trigger : triggers_) {
           if (strcmp(trigger.id, "error") == 0) {
               trigger.ExecuteActivate();
               break;
           }
       }
   }
   ```

## Panel Restoration Strategy

### Default Restoration Logic
```cpp
class PanelManager {
private:
    PanelType lastUserPanel_;  // Last panel before any trigger activation
    PanelType currentPanel_;
    
public:
    void CheckRestoration() {
        // Check if any non-overridable triggers are still active
        bool hasActiveTrigger = false;
        
        for (const auto& trigger : GetSystemTriggers()) {
            if (trigger.isActive && !trigger.canBeOverriddenOnActivate) {
                // Re-execute the active trigger's panel
                trigger.ExecuteActivate();
                hasActiveTrigger = true;
                break;  // Highest priority wins
            }
        }
        
        // If no blocking triggers, restore last user panel
        if (!hasActiveTrigger && lastUserPanel_ != currentPanel_) {
            LoadPanel(lastUserPanel_);
        }
    }
    
    void LoadPanel(PanelType type) {
        // Save current panel as last user panel if it's a user-initiated panel
        if (IsUserPanel(currentPanel_)) {
            lastUserPanel_ = currentPanel_;
        }
        
        currentPanel_ = type;
        // ... actual panel loading logic
    }
};
```

## Implementation File Structure

### Core Types
- `include/types.h` - Trigger, Action, Priority enums

### Handler Classes  
- `include/handlers/trigger_handler.h` - TriggerHandler interface
- `src/handlers/trigger_handler.cpp` - TriggerHandler implementation
- `include/handlers/action_handler.h` - ActionHandler interface  
- `src/handlers/action_handler.cpp` - ActionHandler implementation

### Manager Updates
- `include/managers/interrupt_manager.h` - Updated to coordinate both handlers
- `src/managers/interrupt_manager.cpp` - Implementation with new architecture

### Supporting Files
- `include/handlers/i_handler.h` - Base handler interface (unchanged)
- `include/sensors/base_sensor.h` - Base sensor class (unchanged)

## Processing Flow Summary

### Main Loop Integration
```cpp
void loop() {
    // 1. LVGL tasks
    lv_task_handler();
    
    // 2. Process interrupts (coordinated by InterruptManager)
    interruptManager->Process();   // Handles action evaluation and idle-based execution
    
    // 3. Process errors
    errorManager->Process();
    
    // 4. Update panels
    panelManager->Update();
}
```

## Key Architecture Benefits

1. **Clear Separation**: Triggers (state-based) vs Actions (event-based)
2. **No Context Needed**: Direct function calls to singleton managers
3. **Priority System**: Only applies to Triggers where it matters
4. **Simple Override Logic**: Only Triggers can block, only on activation
5. **Clean Error Handling**: Standard try-catch with error panel fallback
6. **Smart Restoration**: Automatic return to last user panel

## Migration Path

### Phase 1: Type System
1. Add new types to `types.h`
2. Create Trigger and Action structures
3. Define Priority enum

### Phase 2: Handler Refactoring
1. Rename PolledHandler → TriggerHandler
2. Rename QueuedHandler → ActionHandler
3. Update handler interfaces

### Phase 3: Interrupt Migration
1. Convert existing interrupts to Triggers
2. Convert action handling to Actions
3. Update InterruptManager coordination

### Phase 4: Testing
1. Test all trigger override scenarios
2. Verify action execution order
3. Validate error handling and restoration

## Conclusion

This v4 architecture provides:
- **Cleaner conceptual model** with Trigger/Action separation
- **Simpler implementation** without context parameters
- **Robust error handling** with standard patterns
- **Smart restoration** to last user-driven state
- **Clear priority system** only where needed (Triggers)

The design acknowledges the fundamental difference between state changes and events, leading to a more intuitive and maintainable system.