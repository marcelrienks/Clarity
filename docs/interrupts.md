# Interrupt Architecture - Trigger/Action Separation

**Related Documentation:**
- **[Architecture](architecture.md)** - Complete system architecture overview
- **[Requirements](requirements.md)** - Functional requirements and trigger scenarios  
- **[Application Flow](diagrams/application-flow.md)** - Runtime processing flow
- **[Interrupt Flow](diagrams/interrupt-handling-flow.md)** - Detailed processing diagrams

## Executive Summary

This document describes the interrupt architecture using a clear separation between **Triggers** (state-based interrupts from GPIO sensors) and **Actions** (event-based interrupts from buttons). This separation acknowledges the fundamental difference between persistent state changes and momentary events.

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
    CRITICAL = 2,    // Highest priority (e.g., key, errors) - numerically highest
    IMPORTANT = 1,   // Medium priority (e.g., lock)
    NORMAL = 0       // Lowest priority (e.g., lights) - numerically lowest
};

enum class TriggerType : uint8_t {
    PANEL = 0,       // Panel switching triggers
    STYLE = 1,       // Style/theme changing triggers
    FUNCTION = 2     // Function execution triggers
};

struct Trigger {
    const char* id;                          // Unique identifier
    Priority priority;                       // Execution priority (CRITICAL > IMPORTANT > NORMAL)
    TriggerType type;                        // Type classification (Panel, Style, Function)
    
    // Dual-state functions - no context needed
    void (*activateFunc)();                  // Execute when sensor goes ACTIVE
    void (*deactivateFunc)();                // Execute when sensor goes INACTIVE
    
    // State association
    BaseSensor* sensor;                      // Associated sensor (1:1)
    bool isActive;                           // Currently active (true after activate, false after deactivate)
    
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

### TriggerHandler
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
        // Check for higher priority active triggers (higher numeric value = higher priority)
        Trigger* higherPriorityTrigger = FindHigherPriorityActiveTrigger(trigger);
        
        if (!higherPriorityTrigger) {
            log_d("Executing activation for '%s'", trigger.id);
            trigger.ExecuteActivate();  // Execute and set active
        } else {
            log_d("Activation of '%s' suppressed by higher priority '%s'", 
                  trigger.id, higherPriorityTrigger->id);
            trigger.isActive = true;     // Only mark active, don't execute
        }
    }
    
    void HandleDeactivation(Trigger& trigger) {
        log_d("Executing deactivation for '%s'", trigger.id);
        trigger.ExecuteDeactivate();  // Execute and set inactive
        
        // Find highest priority active trigger of same type
        Trigger* sameTypeTrigger = FindHighestPrioritySameTypeTrigger(trigger.type);
        if (sameTypeTrigger) {
            log_d("Re-activating same-type trigger '%s'", sameTypeTrigger->id);
            sameTypeTrigger->ExecuteActivate();
        }
    }
    
    Trigger* FindHigherPriorityActiveTrigger(const Trigger& candidate) {
        // Look for any active trigger with higher priority
        for (auto& activeTrigger : triggers_) {
            if (activeTrigger.isActive && 
                activeTrigger.priority > candidate.priority &&
                &activeTrigger != &candidate) {
                return &activeTrigger;
            }
        }
        return nullptr;
    }
    
    Trigger* FindHighestPrioritySameTypeTrigger(TriggerType type) {
        Trigger* highestPriority = nullptr;
        
        for (auto& trigger : triggers_) {
            if (trigger.isActive && trigger.type == type) {
                if (!highestPriority || trigger.priority > highestPriority->priority) {
                    highestPriority = &trigger;
                }
            }
        }
        
        return highestPriority;
    }
};
```

### ActionHandler
```cpp
class ActionHandler : public IHandler {
private:
    std::vector<Action> actions_;
    std::unique_ptr<ButtonSensor> actionButtonSensor_;  // Handler owns button sensor
    
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
// In TriggerHandler initialization
std::vector<Trigger> systemTriggers = {
    // Key triggers
    {
        .id = "key_present",
        .priority = Priority::CRITICAL,
        .type = TriggerType::PANEL,
        .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::KEY); },
        .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
        .sensor = keyPresentSensor.get(),
        .isActive = false
    },
    {
        .id = "key_not_present", 
        .priority = Priority::CRITICAL,
        .type = TriggerType::PANEL,
        .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::KEY); },
        .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
        .sensor = keyNotPresentSensor.get(),
        .isActive = false
    },
    
    // Lock trigger
    {
        .id = "lock",
        .priority = Priority::IMPORTANT,
        .type = TriggerType::PANEL,
        .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::LOCK); },
        .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
        .sensor = lockSensor.get(),
        .isActive = false
    },
    
    // Lights trigger
    {
        .id = "lights",
        .priority = Priority::NORMAL,
        .type = TriggerType::STYLE,
        .activateFunc = []() { StyleManager::Instance().SetTheme(Theme::NIGHT); },
        .deactivateFunc = []() { StyleManager::Instance().SetTheme(Theme::DAY); },
        .sensor = lightsSensor.get(),
        .isActive = false
    },
    
    // Error trigger
    {
        .id = "error",
        .priority = Priority::CRITICAL,
        .type = TriggerType::PANEL,
        .activateFunc = []() { PanelManager::Instance().LoadPanel(PanelType::ERROR); },
        .deactivateFunc = []() { PanelManager::Instance().CheckRestoration(); },
        .sensor = errorSensor.get(),
        .isActive = false
    }
};
```

### Action Definitions
```cpp
// In ActionHandler initialization
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

## Recent Architecture Improvements

### Dynamic Error Generation During Error Sessions

Recent improvements enable dynamic error addition during active error panel sessions:

```cpp
// In src/handlers/trigger_handler.cpp - HandleTriggerActivation()
void TriggerHandler::HandleTriggerActivation(Trigger& trigger) {
    // ... existing priority logic ...

    // Early return if error panel is active - suppress trigger execution but keep state
    // Exception: Allow error trigger to execute during error panel to support dynamic error addition
    if (ErrorManager::Instance().IsErrorPanelActive() && strcmp(trigger.id, "error") != 0) {
        return;
    }

    // Execute activation function if available
    if (trigger.activateFunc) {
        trigger.activateFunc();
    }
}
```

**Key Benefits:**
- **Dynamic Error Support**: New errors can be generated while error panel is active
- **Defensive Architecture**: All other triggers remain blocked during error handling
- **Debug Error Testing**: Enables real-time error generation for testing dynamic addition
- **Session Independence**: ErrorPanel can maintain its own error state separately

### Error Trigger Enhancement

The error trigger now supports unique timestamped error generation:

```cpp
// In include/definitions/interrupts.h - Error trigger definition
{
    .id = TriggerIds::ERROR,
    .priority = Priority::CRITICAL,
    .type = TriggerType::PANEL,
    .activateFunc = []() {
        log_t("ErrorActivate() - Debug error button pressed, generating test errors");
#ifdef CLARITY_DEBUG
        // Generate three test errors with unique timestamps
        uint32_t timestamp = millis();
        ErrorManager::Instance().ReportWarning("DebugTest",
                                               "Test warning from debug error trigger @" + std::to_string(timestamp));
        ErrorManager::Instance().ReportError(ErrorLevel::ERROR, "DebugTest",
                                             "Test error from debug error trigger @" + std::to_string(timestamp));
        ErrorManager::Instance().ReportCriticalError("DebugTest",
                                                     "Test critical error from debug error trigger @" + std::to_string(timestamp));
#endif
    },
    .deactivateFunc = nullptr,  // One-shot trigger
    .sensor = errorSensor,
    .isActive = false
}
```

## Key Architecture Benefits

1. **Clear Separation**: Triggers (state-based) vs Actions (event-based)
2. **No Context Needed**: Direct function calls to singleton managers
3. **Priority System**: Only applies to Triggers where it matters
4. **Simple Override Logic**: Only Triggers can block, only on activation
5. **Clean Error Handling**: Standard try-catch with error panel fallback
6. **Smart Restoration**: Automatic return to last user panel
7. **Dynamic Error Support**: Real-time error addition during error handling sessions

## Implementation Benefits

### Performance Optimizations
1. **Continuous Responsiveness**: Actions always evaluated for button events
2. **Idle Protection**: Both execution types respect UI idle state
3. **Processing Priority**: Triggers process before Actions when both pending
4. **Memory Efficiency**: Static structures with direct singleton calls

### Reliability Features
1. **State Consistency**: BaseSensor change detection prevents false triggers
2. **Override Protection**: Priority-based blocking logic
3. **Queue Integrity**: Action events preserved until execution
4. **Error Isolation**: Failed interrupts don't affect others

## Conclusion

This architecture provides:
- **Cleaner conceptual model** with Trigger/Action separation
- **Simpler implementation** without context parameters
- **Robust error handling** with standard patterns
- **Smart restoration** to last user-driven state
- **Clear priority system** only where needed (Triggers)

The design acknowledges the fundamental difference between state changes and events, leading to a more intuitive and maintainable system.