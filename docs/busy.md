# Unified Interrupt Handling Architecture

This document outlines the plan to unify the interrupt handling architecture for both triggers and actions in the Clarity system, applying consistent patterns across both subsystems.

## Overview

Both TriggerManager and ActionManager handle hardware interrupts but with different approaches. This plan unifies them under a common architecture while preserving their unique characteristics.

### Current State
- **TriggerManager**: Handles GPIO sensors (key, lock, lights) with priority-based execution
- **ActionManager**: Handles button presses with timing logic for short/long press detection
- Both systems have state management, service dependencies, and deferred execution

### Goal
Create a unified interrupt handling system that:
- Maintains consistency between trigger and action handling
- Reduces code duplication
- Simplifies adding new interrupt sources
- Preserves the unique characteristics of each interrupt type

## Architecture Design

### Directory Structure
```
include/
├── handlers/
│   ├── interrupt_handler.h          # Base class
│   ├── trigger_interrupt_handler.h  # Trigger-specific handler
│   └── action_interrupt_handler.h   # Action-specific handler
├── managers/
│   └── interrupt_manager.h          # Unified interrupt manager
└── utilities/
    └── interrupt_types.h            # Common types and enums
```

### Core Components

#### 1. Common Types (interrupt_types.h)
```cpp
enum class InterruptSource {
    GPIO_TRIGGER,    // Key, Lock, Lights sensors
    BUTTON_ACTION,   // Action button
    ERROR_EVENT,     // Error manager
    TIMER_EVENT      // Future: timed events
};

enum class InterruptState {
    ACTIVE,
    INACTIVE,
    PENDING
};

struct InterruptData {
    InterruptSource source;
    int priority;
    unsigned long timestamp;
    InterruptState state;
    std::variant<Trigger*, Action> payload;
};
```

#### 2. Base Interrupt Handler (interrupt_handler.h)
```cpp
/**
 * @class InterruptHandler
 * @brief Base class for all interrupt handlers
 * 
 * @details Provides common functionality for processing interrupts including
 * panel loading, theme switching, and execution state checking. All specific
 * interrupt handlers should inherit from this class.
 */
class InterruptHandler {
protected:
    IPanelService* panelService_;
    IStyleService* styleService_;
    
    // Common helper methods
    void LoadPanel(const char* panelName);
    void SetTheme(const char* themeName);
    bool CanExecute() const;
    
public:
    InterruptHandler(IPanelService* panel, IStyleService* style)
        : panelService_(panel), styleService_(style) {}
    
    virtual ~InterruptHandler() = default;
    
    // Pure virtual - must be implemented by derived classes
    virtual void ProcessInterrupt(const InterruptData& data) = 0;
    virtual void ProcessDeferred() = 0;
    virtual InterruptSource GetHandledSource() const = 0;
};
```

#### 3. Trigger Interrupt Handler (trigger_interrupt_handler.h)
```cpp
/**
 * @class TriggerInterruptHandler
 * @brief Handles GPIO trigger interrupts (key, lock, lights)
 * 
 * @details Processes trigger state changes and executes corresponding actions
 * like panel loads and theme changes. Supports priority-based execution and
 * deferred processing for non-critical triggers.
 */
class TriggerInterruptHandler : public InterruptHandler {
private:
    std::vector<InterruptData> deferredTriggers_;
    Trigger* triggers_;  // Static trigger array
    
    void ExecuteTrigger(Trigger* trigger, InterruptState state);
    void ExecuteActivation(Trigger* trigger);
    void ExecuteDeactivation(Trigger* trigger);
    void HandlePanelDeactivation(Trigger* trigger);
    Trigger* FindActivePanel();
    
public:
    TriggerInterruptHandler(IPanelService* panel, IStyleService* style, 
                           Trigger* triggerArray)
        : InterruptHandler(panel, style), triggers_(triggerArray) {}
    
    void ProcessInterrupt(const InterruptData& data) override;
    void ProcessDeferred() override;
    InterruptSource GetHandledSource() const override { 
        return InterruptSource::GPIO_TRIGGER; 
    }
};
```

#### 4. Action Interrupt Handler (action_interrupt_handler.h)
```cpp
/**
 * @class ActionInterruptHandler
 * @brief Handles button action interrupts with timing logic
 * 
 * @details Processes button presses, manages timing for short/long press
 * detection, and queues actions when UI is busy. Maintains compatibility
 * with existing Action-based panel callbacks.
 */
class ActionInterruptHandler : public InterruptHandler {
private:
    std::queue<std::pair<Action, unsigned long>> actionQueue_;
    static constexpr unsigned long QUEUE_TIMEOUT_MS = 3000;
    
    ButtonState buttonState_;
    unsigned long pressStartTime_;
    unsigned long debounceStartTime_;
    
    void ExecuteAction(const Action& action);
    void QueueAction(const Action& action, unsigned long timestamp);
    bool IsActionExpired(unsigned long timestamp) const;
    
public:
    ActionInterruptHandler(IPanelService* panel, IStyleService* style)
        : InterruptHandler(panel, style), 
          buttonState_(ButtonState::IDLE),
          pressStartTime_(0),
          debounceStartTime_(0) {}
    
    void ProcessInterrupt(const InterruptData& data) override;
    void ProcessDeferred() override;
    InterruptSource GetHandledSource() const override { 
        return InterruptSource::BUTTON_ACTION; 
    }
};
```

#### 5. Unified Interrupt Manager (interrupt_manager.h)
```cpp
/**
 * @class InterruptManager
 * @brief Centralized interrupt processing and routing
 * 
 * @details Manages all interrupt sources in the system, routes interrupts
 * to appropriate handlers, and coordinates deferred execution. Replaces
 * the separate TriggerManager and ActionManager with a unified approach.
 */
class InterruptManager : public IInterruptService {
private:
    // Handlers for different interrupt sources
    std::unique_ptr<TriggerInterruptHandler> triggerHandler_;
    std::unique_ptr<ActionInterruptHandler> actionHandler_;
    
    // Priority queue for pending interrupts
    std::priority_queue<InterruptData> interruptQueue_;
    
    // Sensors
    std::shared_ptr<KeySensor> keySensor_;
    std::shared_ptr<LockSensor> lockSensor_;
    std::shared_ptr<LightsSensor> lightSensor_;
    std::shared_ptr<ActionButtonSensor> buttonSensor_;
    
    bool initialized_;
    
    // Helper methods
    void PollSensors();
    void ProcessQueuedInterrupts();
    bool CanProcessDeferred() const;
    
public:
    InterruptManager(/* sensor dependencies */);
    
    // IInterruptService interface
    void Init() override;
    void ProcessInterruptEvents() override;
    const char* GetStartupPanelOverride() const override;
    int GetPriority() const override { return 100; }
    
    // Queue interrupt for processing
    void QueueInterrupt(InterruptData data);
    
    // Get specific handler (for testing)
    InterruptHandler* GetHandler(InterruptSource source);
};
```

## Implementation Details

### Handler Implementation Pattern

Each handler follows this pattern:

```cpp
void TriggerInterruptHandler::ProcessInterrupt(const InterruptData& data) {
    if (data.source != InterruptSource::GPIO_TRIGGER) return;
    
    auto* trigger = std::get<Trigger*>(data.payload);
    if (!trigger) return;
    
    // High priority triggers execute immediately
    if (trigger->priority == TriggerPriority::CRITICAL) {
        ExecuteTrigger(trigger, data.state);
        return;
    }
    
    // Lower priority triggers check UI state
    if (CanExecute()) {
        ExecuteTrigger(trigger, data.state);
    } else {
        deferredTriggers_.push_back(data);
    }
}

void TriggerInterruptHandler::ProcessDeferred() {
    if (!CanExecute() || deferredTriggers_.empty()) return;
    
    // Sort by priority
    std::sort(deferredTriggers_.begin(), deferredTriggers_.end(),
        [](const auto& a, const auto& b) { 
            auto* triggerA = std::get<Trigger*>(a.payload);
            auto* triggerB = std::get<Trigger*>(b.payload);
            return triggerA->priority < triggerB->priority;
        });
    
    // Process all deferred triggers
    for (const auto& data : deferredTriggers_) {
        ExecuteTrigger(std::get<Trigger*>(data.payload), data.state);
    }
    
    deferredTriggers_.clear();
}
```

### Interrupt Manager Flow

```cpp
void InterruptManager::ProcessInterruptEvents() {
    // 1. Poll all sensors
    PollSensors();
    
    // 2. Process queued interrupts by priority
    ProcessQueuedInterrupts();
    
    // 3. Process deferred interrupts if UI is idle
    if (CanProcessDeferred()) {
        triggerHandler_->ProcessDeferred();
        actionHandler_->ProcessDeferred();
    }
}

void InterruptManager::ProcessQueuedInterrupts() {
    while (!interruptQueue_.empty()) {
        auto data = interruptQueue_.top();
        interruptQueue_.pop();
        
        // Route to appropriate handler
        switch (data.source) {
            case InterruptSource::GPIO_TRIGGER:
                triggerHandler_->ProcessInterrupt(data);
                break;
                
            case InterruptSource::BUTTON_ACTION:
                actionHandler_->ProcessInterrupt(data);
                break;
                
            case InterruptSource::ERROR_EVENT:
                // Future: error handler
                break;
        }
    }
}
```

## Benefits

1. **Consistency**: Both triggers and actions follow the same interrupt handling pattern
2. **Code Reuse**: Common functionality (LoadPanel, SetTheme, CanExecute) in base class
3. **Extensibility**: Easy to add new interrupt sources (timers, network events, etc.)
4. **Priority Management**: Unified priority queue ensures proper execution order
5. **Testing**: Common base class and interfaces simplify unit testing
6. **Maintainability**: Clear separation of concerns with handler-specific logic isolated

## Migration Plan

### Phase 1: Create Base Infrastructure
1. Create `interrupt_types.h` with common enums and structures
2. Implement `InterruptHandler` base class with common helpers
3. Create handler directory structure

### Phase 2: Implement Handlers
1. Create `TriggerInterruptHandler` by extracting logic from TriggerManager
2. Create `ActionInterruptHandler` by extracting logic from ActionManager
3. Ensure both handlers maintain existing behavior

### Phase 3: Create Unified Manager
1. Implement `InterruptManager` with sensor polling
2. Add interrupt routing logic
3. Implement deferred processing coordination

### Phase 4: Integration
1. Replace TriggerManager and ActionManager usage with InterruptManager
2. Update factory methods to create InterruptManager
3. Update tests to use new architecture

### Phase 5: Cleanup
1. Remove old TriggerManager and ActionManager classes
2. Update documentation
3. Add integration tests for unified system

## Future Extensions

The unified architecture makes it easy to add:

1. **Timer-based interrupts**: For periodic updates or timeouts
2. **Network interrupts**: For remote control or OTA updates
3. **Sensor fusion**: Combining multiple sensors for complex triggers
4. **Event recording**: Logging all interrupts for debugging
5. **Interrupt masking**: Temporarily disabling certain interrupt sources

## Considerations

### Memory Usage
- Static allocation preferred for ESP32
- Minimal dynamic memory for queues with size limits
- Reuse existing sensor and service pointers

### Performance
- Priority queue ensures critical interrupts execute first
- Deferred processing prevents UI blocking
- Direct function calls avoid virtual function overhead where possible

### Backwards Compatibility
- Existing Action and Trigger structures preserved
- Panel interfaces unchanged
- Sensor interfaces maintained

This unified architecture provides a clean, extensible foundation for all interrupt handling in the Clarity system while maintaining the unique characteristics that make triggers and actions effective for their specific use cases.