# Interrupt System Consolidation Plan

## Overview

This document outlines the consolidation of Clarity's fragmented interrupt system from separate TriggerHandler and ActionHandler components into a unified interrupt architecture that simplifies multi-trigger management and restoration logic while maintaining ESP32 memory safety.

## Current System Analysis

### Problems with Current Architecture

#### **Fragmented Processing Models**
- **TriggerHandler**: Manages polled GPIO state changes with complex 4-phase processing
- **ActionHandler**: Manages queued button events, only processes when no triggers active
- **Different Processing Logic**: Two separate state machines with different evaluation patterns
- **Complex Interdependence**: ActionHandler blocked by any active trigger

#### **Complex Restoration Logic**
- **Priority-Based Blocking**: CRITICAL/IMPORTANT triggers block restoration, NORMAL don't
- **Edge Case Management**: Multiple triggers active/inactive combinations create complex scenarios
- **State Tracking**: Separate restoration logic in TriggerHandler with fallback mechanisms
- **Maintenance Burden**: Complex logic spread across multiple handlers

#### **Multi-Trigger Management Issues**
- **Simultaneous Triggers**: Multiple triggers can be active with unclear precedence
- **Restoration Conflicts**: When one trigger deactivates but others remain active
- **Priority Resolution**: Complex blocking logic determines which panels to show
- **State Synchronization**: Keeping multiple handlers in sync during state changes

#### **Extensibility Limitations**
- **New Interrupt Types**: Require creating new handler classes
- **Code Duplication**: Similar processing patterns across different handlers
- **Interface Consistency**: Different registration and processing patterns

### Current Memory Usage
- **TriggerHandler**: ~2KB for trigger array and processing state
- **ActionHandler**: ~1KB for action array and button state
- **Combined Overhead**: ~3KB plus registration complexity

## Unified Interrupt System Design

### Core Architecture

#### **Dual Specialized Handler Model**
```cpp
class InterruptManager {
private:
    std::unique_ptr<PolledHandler> polledHandler_;     // GPIO state monitoring
    std::unique_ptr<QueuedHandler> queuedHandler_;     // Button event processing
    PanelRestoreTracker restoreTracker_;               // Simplified restoration
    std::vector<Interrupt> registeredInterrupts_;      // All registered interrupts
    
public:
    void Process();                                     // Coordinates both handlers
    void RegisterInterrupt(const Interrupt& interrupt); // Routes to appropriate handler and stores
    void UnregisterInterrupt(const char* id);          // Removes interrupt by ID
    std::vector<Interrupt> GetRegisteredInterrupts() const; // Returns all registered interrupts
    void HandleRestoration();                           // Simplified restoration logic
};

class PolledHandler : public IHandler {
private:
    std::vector<Interrupt> polledInterrupts_;          // POLLED source interrupts only
    std::unordered_map<std::string, size_t> interruptIndex_; // ID to vector index mapping
    
public:
    void Process() override;                           // POLLED processing
    void RegisterInterrupt(const Interrupt& interrupt); // Adds to polled interrupts
    void UnregisterInterrupt(const char* id);          // Removes by ID
    std::vector<Interrupt> GetRegisteredInterrupts() const; // Returns all polled interrupts
    Interrupt* FindInterrupt(const char* id);          // Finds interrupt by ID
};

class QueuedHandler : public IHandler {
private:
    std::vector<Interrupt> queuedInterrupts_;          // QUEUED source interrupts only
    std::optional<ButtonEvent> latestButtonEvent_;    // Only maintains latest button event (not a queue)
    std::unordered_map<std::string, size_t> interruptIndex_; // ID to vector index mapping
    
public:
    void Process() override;                           // QUEUED processing
    void RegisterInterrupt(const Interrupt& interrupt); // Adds to queued interrupts
    void UnregisterInterrupt(const char* id);          // Removes by ID
    std::vector<Interrupt> GetRegisteredInterrupts() const; // Returns all queued interrupts
    Interrupt* FindInterrupt(const char* id);          // Finds interrupt by ID
    void QueueButtonEvent(ButtonEvent event);          // Replaces any existing event with latest
    bool HasPendingButtonEvent() const;                // Checks if latest event is unprocessed
    void ClearButtonEvent();                           // Clears the latest event after processing
};
```

#### **Interrupt Structure**
```cpp
enum class InterruptSource {
    POLLED,     // GPIO state monitoring (like current triggers)
    QUEUED      // Button event processing (like current actions)
};

struct Interrupt {
    const char* id;                                    // Static string ID
    Priority priority;                                 // Processing priority
    InterruptSource source;                           // POLLED or QUEUED evaluation
    bool (*evaluationFunc)(void* context);           // Checks current state (POLLED) or events (QUEUED)
    void (*activateFunc)(void* context);             // Custom function executed on interrupt activation
    void (*deactivateFunc)(void* context);           // Custom function executed on interrupt deactivation (optional)
    void* sensorContext;                              // Sensor context for evaluation
    void* serviceContext;                             // Service context for execution (PanelManager, StyleManager, etc.)
    bool maintainWhenInactive;                        // Whether interrupt should maintain when inactive (themes=true, panels=false)
    
    // Runtime state
    bool active;                                      // Current activation state
    bool previouslyActive;                           // Previous state for change detection
    unsigned long lastEvaluation;                    // Performance tracking
};
```

### Processing Model

#### **Coordinated Processing Flow**
```cpp
void InterruptManager::Process() {
    // Collect highest priority interrupt from each handler
    Interrupt* highestPolled = polledHandler_->GetHighestPriorityActiveInterrupt();
    Interrupt* highestQueued = queuedHandler_->GetHighestPriorityActiveInterrupt();
    
    // Determine which interrupt to execute based on priority
    Interrupt* toExecute = nullptr;
    if (highestPolled && highestQueued) {
        toExecute = (highestPolled->priority < highestQueued->priority) ? highestPolled : highestQueued;
    } else if (highestPolled) {
        toExecute = highestPolled;
    } else if (highestQueued) {
        toExecute = highestQueued;
    }
    
    // Execute highest priority interrupt
    if (toExecute) {
        ExecuteInterrupt(*toExecute);
    }
    
    // Handle panel restoration
    HandleRestoration();
}

void PolledHandler::Process() {
    // Process POLLED interrupts - GPIO state monitoring
    for (auto& interrupt : polledInterrupts_) {
        if (interrupt.evaluationFunc(interrupt.context)) {
            // Mark as ready for execution (InterruptManager will coordinate)
            interrupt.active = true;
            break; // Only process first active interrupt per handler
        }
    }
}

void QueuedHandler::Process() {
    // Process QUEUED interrupts - single latest button event only
    if (!latestButtonEvent_.has_value()) {
        return; // No button event to process
    }
    
    // Check each queued interrupt against the latest button event
    for (auto& interrupt : queuedInterrupts_) {
        if (interrupt.evaluationFunc(interrupt.sensorContext)) {
            // Mark as ready for execution (InterruptManager will coordinate)
            interrupt.active = true;
            // Clear the button event since it's being processed
            latestButtonEvent_.reset();
            break; // Only process first matching interrupt
        }
    }
}

// QueuedHandler implementation for single event management
void QueuedHandler::QueueButtonEvent(ButtonEvent event) {
    // Always replace any existing event with the latest one
    latestButtonEvent_ = event;
}

bool QueuedHandler::HasPendingButtonEvent() const {
    return latestButtonEvent_.has_value();
}

void QueuedHandler::ClearButtonEvent() {
    latestButtonEvent_.reset();
}
```

#### **Activate/Deactivate Based Execution**
```cpp
void InterruptManager::ProcessInterrupt(Interrupt& interrupt) {
    bool currentState = interrupt.evaluationFunc(interrupt.sensorContext);
    
    // Check for state changes
    if (currentState != interrupt.previouslyActive) {
        if (currentState) {
            // Interrupt activated
            interrupt.activateFunc(interrupt.serviceContext);
            interrupt.active = true;
            
            // Track restoration if needed
            if (interrupt.effect == InterruptEffect::LOAD_PANEL && 
                interrupt.data.panel.trackForRestore) {
                restoreTracker_.TrackPanelChange(interrupt.id, interrupt.data.panel.panelName);
            }
        } else {
            // Interrupt deactivated
            if (interrupt.deactivateFunc) {
                interrupt.deactivateFunc(interrupt.serviceContext);
            }
            interrupt.active = false;
        }
        
        interrupt.previouslyActive = currentState;
        interrupt.lastEvaluation = millis();
    }
}

// Priority-based processing with maintenance
void InterruptManager::Process() {
    // Process all interrupts for state changes
    for (auto& interrupt : allInterrupts_) {
        ProcessInterrupt(interrupt);
    }
    
    // Find highest priority active interrupt that doesn't maintain
    Interrupt* highestNonMaintaining = nullptr;
    for (auto& interrupt : allInterrupts_) {
        if (interrupt.active) {
            bool maintains = (interrupt.effect == InterruptEffect::SET_THEME && 
                             interrupt.data.theme.maintainWhenInactive) ||
                            (interrupt.effect == InterruptEffect::LOAD_PANEL && 
                             interrupt.data.panel.maintainWhenInactive);
            
            if (!maintains && (!highestNonMaintaining || 
                              interrupt.priority < highestNonMaintaining->priority)) {
                highestNonMaintaining = &interrupt;
            }
        }
    }
    
    // Handle panel restoration
    HandleRestoration(highestNonMaintaining);
}
```

### Priority-Based Maintenance System

#### **Maintenance vs Restoration Logic**
```cpp
void InterruptManager::HandleRestoration() {
    // Find highest priority active interrupt that doesn't maintain
    Interrupt* highestNonMaintaining = nullptr;
    
    for (auto& interrupt : allActiveInterrupts_) {
        bool maintains = interrupt.maintainWhenInactive;
        
        // Only non-maintaining interrupts participate in restoration blocking
        if (!maintains && (!highestNonMaintaining || 
                          interrupt.priority < highestNonMaintaining->priority)) {
            highestNonMaintaining = &interrupt;
        }
    }
    
    // Restore user panel only when no non-maintaining interrupts are active
    if (!highestNonMaintaining) {
        restoreTracker_.RestoreUserPanel();
    }
}

bool InterruptManager::HasNonMaintainingInterrupts() const {
    for (const auto& interrupt : allActiveInterrupts_) {
        bool maintains = false;
        
        bool maintains = interrupt.maintainWhenInactive;
        
        if (!maintains && interrupt.active) {
            return true;
        }
    }
    return false;
}
```

**Key Maintenance Principles:**
- **Theme Maintenance**: Theme interrupts always maintain (maintainWhenInactive = true)
- **Panel Restoration**: Panel loading interrupts usually don't maintain (maintainWhenInactive = false)
- **Button Actions**: Button interrupts never maintain (maintainWhenInactive = false)
- **Priority Override**: Higher priority non-maintaining interrupts override lower priority ones
- **Maintenance Transparency**: Maintaining interrupts never block restoration or other interrupts

### Registration Patterns

#### **Polled Panel Loading (replaces current triggers)**
```cpp
// Key present detection with panel loading - routed to PolledHandler
interruptManager.RegisterInterrupt({
    .id = "key_present",
    .priority = Priority::CRITICAL,
    .source = InterruptSource::POLLED,
    .evaluationFunc = IsKeyPresent,              // Checks current state
    .activateFunc = LoadKeyPanel,                // Custom function: loads key panel
    .deactivateFunc = nullptr,                   // No deactivation function needed
    .sensorContext = keyPresentSensor,
    .serviceContext = panelManager,
    .maintainWhenInactive = false                // Don't maintain when inactive
});

// Lock status detection - routed to PolledHandler
interruptManager.RegisterInterrupt({
    .id = "lock_active", 
    .priority = Priority::IMPORTANT,
    .source = InterruptSource::POLLED,
    .evaluationFunc = IsLockEngaged,             // Checks current state
    .activateFunc = LoadLockPanel,               // Custom function: loads lock panel
    .deactivateFunc = nullptr,                   // No deactivation function needed
    .sensorContext = lockSensor,
    .serviceContext = panelManager,
    .maintainWhenInactive = false                // Don't maintain when inactive
});
```

#### **Polled Theme Setting (replaces theme triggers)**
```cpp
// Light sensor theme switching - routed to PolledHandler
interruptManager.RegisterInterrupt({
    .id = "lights_day_theme",
    .priority = Priority::NORMAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::SET_THEME,
    .evaluationFunc = AreLightsOff,              // Checks current state (lights off = day)
    .activateFunc = SetDayTheme,                 // Executes when lights turn off
    .deactivateFunc = nullptr,                   // No deactivation needed (other interrupt handles night)
    .sensorContext = lightsSensor,
    .serviceContext = styleManager,
    .data = { .theme = { Theme::DAY, true } }    // Always maintain theme changes
});

// Night theme interrupt
interruptManager.RegisterInterrupt({
    .id = "lights_night_theme",
    .priority = Priority::NORMAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::SET_THEME,
    .evaluationFunc = AreLightsOn,               // Checks current state (lights on = night)
    .activateFunc = SetNightTheme,               // Executes when lights turn on
    .deactivateFunc = nullptr,                   // No deactivation needed (other interrupt handles day)
    .sensorContext = lightsSensor,
    .serviceContext = styleManager,
    .data = { .theme = { Theme::NIGHT, true } }  // Always maintain theme changes
});
```

#### **Queued Button Actions - Universal Panel Functions**
```cpp
// Every panel must have both short and long press functions
// Short press actions for current panel - routed to QueuedHandler
interruptManager.RegisterInterrupt({
    .id = "panel_short_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::BUTTON_ACTION,
    .evaluationFunc = HasShortPressEvent,
    .activateFunc = ExecutePanelShortPress,      // Calls current panel's short press function
    .deactivateFunc = nullptr,
    .sensorContext = actionButtonSensor,
    .serviceContext = panelManager,              // Used to get current panel
    .data = { .buttonActions = { nullptr, nullptr, nullptr } }  // Functions injected at runtime
});

// Long press actions for current panel - routed to QueuedHandler
interruptManager.RegisterInterrupt({
    .id = "panel_long_press",
    .priority = Priority::NORMAL,
    .source = InterruptSource::QUEUED,
    .effect = InterruptEffect::BUTTON_ACTION,
    .evaluationFunc = HasLongPressEvent,
    .activateFunc = ExecutePanelLongPress,       // Calls current panel's long press function
    .deactivateFunc = nullptr,
    .sensorContext = actionButtonSensor,
    .serviceContext = panelManager,              // Used to get current panel
    .data = { .buttonActions = { nullptr, nullptr, nullptr } }  // Functions injected at runtime
});
```

#### **Error System Integration**
```cpp
// Error trigger with highest priority - routed to PolledHandler
interruptManager.RegisterInterrupt({
    .id = "error_occurred",
    .priority = Priority::CRITICAL,
    .source = InterruptSource::POLLED,
    .effect = InterruptEffect::LOAD_PANEL,
    .evaluationFunc = HasPendingErrors,
    .executionFunc = LoadErrorPanel,
    .context = errorManager,
    .data = { .panel = { "ERROR", true } }  // Track for restoration
});
```

### Static Callback Implementation

#### **Context-Based Callbacks**
```cpp
// Static callback functions with context parameters
class InterruptCallbacks {
public:
    // POLLED evaluation callbacks - check current state
    static bool IsKeyPresent(void* context) {
        auto* sensor = static_cast<KeyPresentSensor*>(context);
        return sensor && sensor->GetCurrentState();  // Current state, not change
    }
    
    static bool AreLightsOn(void* context) {
        auto* sensor = static_cast<LightsSensor*>(context);
        return sensor && sensor->GetCurrentState();  // Current state, not change
    }
    
    static bool AreLightsOff(void* context) {
        auto* sensor = static_cast<LightsSensor*>(context);
        return sensor && !sensor->GetCurrentState(); // Current state inverted
    }
    
    // QUEUED evaluation callbacks - check button events
    static bool HasShortPressEvent(void* context) {
        auto* sensor = static_cast<ActionButtonSensor*>(context);
        return sensor && sensor->HasButtonEvent(ButtonEvent::SHORT_PRESS);
    }
    
    static bool HasLongPressEvent(void* context) {
        auto* sensor = static_cast<ActionButtonSensor*>(context);
        return sensor && sensor->HasButtonEvent(ButtonEvent::LONG_PRESS);
    }
    
    // POLLED activation callbacks - custom functions
    static void LoadKeyPanel(void* context) {
        auto* panelManager = static_cast<PanelManager*>(context);
        if (panelManager) {
            panelManager->LoadPanel("KEY");
        }
    }
    
    static void LoadLockPanel(void* context) {
        auto* panelManager = static_cast<PanelManager*>(context);
        if (panelManager) {
            panelManager->LoadPanel("LOCK");
        }
    }
    
    static void SetDayTheme(void* context) {
        auto* styleManager = static_cast<StyleManager*>(context);
        if (styleManager) {
            styleManager->SetTheme(Theme::DAY);
        }
    }
    
    static void SetNightTheme(void* context) {
        auto* styleManager = static_cast<StyleManager*>(context);
        if (styleManager) {
            styleManager->SetTheme(Theme::NIGHT);
        }
    }
    
    // QUEUED activation callbacks - panel button functions
    static void ExecutePanelShortPress(void* context) {
        auto* panelManager = static_cast<PanelManager*>(context);
        if (panelManager) {
            auto* currentPanel = panelManager->GetCurrentPanel();
            if (currentPanel && currentPanel->GetShortPressFunction()) {
                currentPanel->GetShortPressFunction()(currentPanel);
            }
        }
    }
    
    static void ExecutePanelLongPress(void* context) {
        auto* panelManager = static_cast<PanelManager*>(context);
        if (panelManager) {
            auto* currentPanel = panelManager->GetCurrentPanel();
            if (currentPanel && currentPanel->GetLongPressFunction()) {
                currentPanel->GetLongPressFunction()(currentPanel);
            }
        }
    }
};
```

### Memory Optimization

#### **Memory Usage Analysis**
- **Interrupt Struct**: ~48 bytes per interrupt (union keeps data small)
- **Handler Overhead**: ~1.5KB for both handlers plus InterruptManager coordination
- **Total Usage**: ~2.5KB for 20 interrupts (vs ~3KB current)
- **Memory Savings**: ~500 bytes reduction plus eliminated redundant layer

#### **ESP32-Specific Optimizations**
- **Static Callbacks**: No heap allocation for function pointers
- **Union Data**: Minimizes memory per interrupt
- **Single Handler**: Reduces object overhead
- **Context Pointers**: Eliminate lambda captures

### Multi-Trigger Scenarios

#### **Scenario 1: Multiple Panel-Loading Triggers**
```
1. Key inserted → IsKeyPresent() = true → LoadKeyPanel() activates (CRITICAL) → Key panel loaded
2. Lock engaged → IsLockEngaged() = true → LoadLockPanel() activates (IMPORTANT) → Lock panel loaded (overrides key panel due to current highest priority)
3. Key removed → IsKeyPresent() = false → key_present deactivates, lock still active → Lock panel remains (lock has priority)
4. Lock disengaged → IsLockEngaged() = false → lock_active deactivates, no panel triggers active → User panel restored
```

#### **Scenario 2: Theme Changes with Panel Triggers (Theme Always Maintains)**
```
1. Key inserted → IsKeyPresent() = true → Key panel loaded (priority CRITICAL, doesn't maintain)
2. Lights turned on → AreLightsOn() = true → SetNightTheme() activates (priority NORMAL, always maintains)
3. Key removed → IsKeyPresent() = false → key_present deactivates
   - Theme still active but maintains (doesn't block restoration)
   - No non-maintaining interrupts active → User panel restored with NIGHT theme
4. Lights turned off → AreLightsOff() = true → SetDayTheme() activates (theme change maintains)
   - User panel continues with DAY theme applied
```

#### **Scenario 3: Priority Override with Maintenance**
```
1. Lights on → Night theme active (NORMAL priority, maintains)
2. Key inserted → Key panel loads (CRITICAL priority, doesn't maintain, overrides theme display but theme maintains)
3. Lock engaged → Lock panel loads (IMPORTANT priority, doesn't maintain, overrides key panel)
4. Key removed → key_present deactivates, lock still active (IMPORTANT priority, doesn't maintain)
   - Lock panel continues (higher priority than theme)
5. Lock disengaged → lock_active deactivates, only theme active (maintains)
   - No non-maintaining interrupts → User panel restored with NIGHT theme
```

## Migration Strategy

### Phase 1: Infrastructure Setup
1. **Create UnifiedInterruptHandler class**
2. **Implement UnifiedInterrupt struct**
3. **Create static callback functions**
4. **Set up registration infrastructure**

### Phase 2: Parallel Implementation  
1. **Register existing triggers as unified interrupts**
2. **Register existing actions as unified interrupts**
3. **Test parallel operation with current system**
4. **Validate behavior matches current system**

### Phase 3: Transition
1. **Switch InterruptManager to use UnifiedInterruptHandler**
2. **Disable old TriggerHandler and ActionHandler**
3. **Verify all functionality maintained**
4. **Performance and memory testing**

### Phase 4: Cleanup
1. **Remove TriggerHandler and ActionHandler classes**
2. **Clean up old interrupt data structures**
3. **Update all registration calls**
4. **Update documentation**

### Phase 5: Optimization
1. **Memory usage optimization**
2. **Performance tuning**
3. **Extended testing**
4. **Final validation**

## Benefits Analysis

### **Architectural Benefits**
- **Specialized Handler Model**: Dedicated handlers for POLLED and QUEUED sources with central coordination
- **Eliminated Redundancy**: Removes redundant UnifiedInterruptHandler layer, InterruptManager coordinates directly
- **Universal Panel Functions**: Every panel has short/long press functions injected via function pointers
- **Priority-Based Maintenance**: Clear maintenance system where themes maintain, panels don't, buttons never maintain
- **Activate/Deactivate Functions**: All interrupts have custom activate functions, optional deactivate functions

### **Maintenance Benefits**
- **Specialized Codebases**: Each handler optimized for its source type (POLLED vs QUEUED)
- **Consistent Processing**: Same Interrupt struct and effect patterns across both handlers
- **Easier Debugging**: Clear separation between GPIO state monitoring and button event processing
- **Cleaner Testing**: Two focused handlers plus coordination logic

### **Extensibility Benefits**
- **New Interrupt Types**: Just add new effects, no new handler classes needed
- **Source-Specific Optimization**: Each handler can optimize for its evaluation type
- **Configuration Changes**: Built-in preference setting support via SET_PREFERENCE effect
- **Future Features**: Theme system, error handling, etc. all use same Interrupt struct pattern

### **Performance Benefits**  
- **Memory Efficiency**: ~500 bytes savings plus eliminated redundant handler layer
- **CPU Efficiency**: Specialized evaluation loops (POLLED vs QUEUED) with central coordination
- **Cache Friendly**: Smaller, focused handlers mean better instruction cache usage
- **Predictable Timing**: Coordinated processing eliminates complex synchronization

## Risk Analysis

### **Implementation Risks**
- **Migration Complexity**: Moving from TriggerHandler/ActionHandler to PolledHandler/QueuedHandler coordination
- **Coordination Logic**: Ensuring InterruptManager properly coordinates between handlers
- **Union Management**: Careful handling of union data fields in Interrupt struct
- **Context Safety**: Ensuring void* contexts remain valid across handler boundaries

### **Mitigation Strategies**
- **Parallel Implementation**: Run both systems during transition
- **Handler-by-Handler Migration**: Migrate PolledHandler first, then QueuedHandler
- **Comprehensive Testing**: Validate coordination logic and priority resolution
- **Rollback Plan**: Keep old system until both handlers fully validated

### **Memory Safety**
- **Static Callbacks**: Maintain current memory safety
- **Context Validation**: Null checks on all context usage
- **Union Safety**: Proper field access based on effect type
- **Resource Cleanup**: Same GPIO cleanup patterns

## Success Criteria

### **Functional Requirements**
- ✅ All current trigger behaviors maintained
- ✅ All current action behaviors maintained  
- ✅ Panel restoration works correctly
- ✅ Multi-trigger scenarios handle properly
- ✅ Theme changes work without affecting restoration
- ✅ Error system integration maintained

### **Performance Requirements**
- ✅ Memory usage reduced by at least 500 bytes
- ✅ Processing latency maintained or improved
- ✅ No increase in interrupt processing time
- ✅ Stable operation for 24+ hours

### **Maintainability Requirements**
- ✅ Single interrupt registration pattern
- ✅ Consistent callback implementation
- ✅ Clear documentation of new system
- ✅ Easy addition of new interrupt types

This consolidation plan provides a clear path from the current fragmented system to a coordinated interrupt architecture with specialized handlers that simplifies multi-interrupt management while eliminating redundant layers and maintaining all current functionality and ESP32 memory safety requirements.