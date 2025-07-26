# Trigger System Architecture

## Overview

The Clarity trigger system implements a **pure pin-change-driven architecture** with complete separation between ESP32 cores. The system is designed for **maximum simplicity** with **no state tracking**, **no delays**, and **no artificial complexity**.

## Core Design Principles

### 1. Pin-Change-Driven Only
- **Only GPIO pin CHANGES trigger actions** - not pin states
- **No continuous state evaluation** - events only generated on actual hardware changes
- **No state tracking complexity** - triggers respond immediately to pin changes
- **INIT state prevents startup noise** - no restoration events until first pin change

### 2. Dual-Core Separation
- **Core 1 (Producer)**: Hardware interrupt handlers ONLY
- **Core 0 (Consumer)**: All logic, UI, and state management
- **No cross-core synchronization** - pure producer-consumer via event queue
- **No mutexes** - Core 0 has exclusive ownership of all application state

### 3. Maximum Simplicity
- **No artificial delays** - system responds immediately to changes
- **No task suspension/resume** - clean separation eliminates need for synchronization
- **No complex state machines** - triggers are simple pin-change → action mappings
- **No continuous processing** - events only created when hardware actually changes

### 4. Dependency Injection
- **Triggers return action requests** - they don't execute actions directly
- **Main loop owns all managers** - orchestrates dependencies via request/response
- **Clean interfaces** - triggers don't know about panels, panels don't know about triggers

## Dual-Core Architecture

### Core 1 (Producer) - Hardware Only
```
GPIO Pin Change → ISR Handler → Queue Event → Done
```

**Responsibilities:**
- GPIO interrupt service routines (ISRs)
- Queue ISR events to `isrEventQueue`
- **Nothing else** - no logic, no delays, no state management

**ISR Handlers:**
- `keyPresentIsrHandler()` - Queues `KEY_PRESENT` events
- `keyNotPresentIsrHandler()` - Queues `KEY_NOT_PRESENT` events  
- `lockStateIsrHandler()` - Queues `LOCK_STATE_CHANGE` events
- `themeSwitchIsrHandler()` - Queues `THEME_SWITCH` events

### Core 0 (Consumer) - All Logic
```
Main Loop → ProcessPendingTriggerEvents() → Action Requests → Execute Actions
```

**Responsibilities:**
- Process queued ISR events
- Generate action requests on pin changes
- Execute actions via PanelManager/StyleManager
- All UI operations and state management

## Pin-Change-Driven Flow

### 1. Hardware Event
```
Pin State Changes → ISR Fires → Event Queued
```

### 2. Event Processing
```cpp
auto triggerRequests = TriggerManager::GetInstance().ProcessPendingTriggerEvents();
```
- Dequeue all pending ISR events
- Map events to trigger objects
- **Immediately generate action requests** based on new pin state
- Return action requests to main loop

### 3. Action Execution
```cpp
for (const auto& request : triggerRequests) {
    switch (request.type) {
        case TriggerActionType::LoadPanel:
            PanelManager::GetInstance().CreateAndLoadPanel(...);
            break;
        case TriggerActionType::RestorePanel:
            // Load previous panel
            break;
        case TriggerActionType::ToggleTheme:
            StyleManager::GetInstance().set_theme(...);
            break;
    }
}
```

## Trigger Implementations

### Pin-Change Logic
All triggers follow the same pattern:
- **Pin HIGH → ACTIVE state → GetActionRequest()** (load specific panel/theme)
- **Pin LOW → INACTIVE state → GetRestoreRequest()** (restore previous state)

### Key Triggers
- **KeyTrigger** (`TRIGGER_KEY_PRESENT`): Key detected → Load KeyPanel with present=true styling
- **KeyNotPresentTrigger** (`TRIGGER_KEY_NOT_PRESENT`): Key absent → Load KeyPanel with present=false styling
- Both restore to previous panel when pin goes LOW

### Lock Trigger
- **LockTrigger** (`TRIGGER_LOCK_STATE`): Lock engaged → Load LockPanel, unlocked → Restore previous panel

### Lights Trigger  
- **LightsTrigger** (`TRIGGER_LIGHTS_STATE`): Lights on → Night theme, lights off → Day theme

## Key Architecture Benefits

### 1. No Race Conditions
- Core 1 only writes to queue
- Core 0 only reads from queue  
- No shared state between cores

### 2. Immediate Response
- No artificial delays
- No waiting for state evaluation cycles
- Actions triggered immediately on pin changes

### 3. Predictable Behavior
- Events only generated on actual hardware changes
- No duplicate events or continuous processing
- Clear cause-and-effect relationship

### 4. Easy Debugging
- Simple event flow: Pin Change → ISR → Queue → Process → Action
- No complex state machines to debug
- Clear logging at each step

### 5. Easy Extension
- Add new triggers by implementing `GetActionRequest()` and `GetRestoreRequest()`
- Register trigger in `RegisterAllTriggers()`
- Add ISR event type and mapping
- No changes to core architecture

## Implementation Details

### Main Loop (Core 0)
```cpp
void loop() {
    // Process pin changes and get immediate action requests
    auto triggerRequests = TriggerManager::GetInstance().ProcessPendingTriggerEvents();
    
    // Execute action requests via dependency injection
    for (const auto& request : triggerRequests) {
        // Handle LoadPanel, RestorePanel, ToggleTheme requests
    }
    
    // Update current panel and handle LVGL
    PanelManager::GetInstance().UpdatePanel();
    Ticker::handle_lv_tasks();
}
```

### ISR Event Structure
```cpp
struct ISREvent {
    ISREventType eventType;  // KEY_PRESENT, LOCK_STATE_CHANGE, etc.
    bool pinState;          // HIGH or LOW
};
```

### Trigger Action Requests
```cpp
struct TriggerActionRequest {
    TriggerActionType type;    // LoadPanel, RestorePanel, ToggleTheme
    const char* panelName;     // Panel/theme name
    const char* triggerId;     // Source trigger ID
    bool isTriggerDriven;      // Whether this is trigger-driven
};
```

## Anti-Patterns to Avoid

### ❌ State Tracking
```cpp
// DON'T: Track request history, processed states, etc.
bool hasProcessedActiveState = false;
```

### ❌ Continuous Processing
```cpp
// DON'T: Generate requests based on current state
if (trigger->GetState() == ACTIVE) {
    return GetActionRequest(); // Creates infinite loops!
}
```

### ❌ Artificial Delays
```cpp
// DON'T: Add delays to "fix" timing issues
vTaskDelay(pdMS_TO_TICKS(100));
```

### ❌ Cross-Core Dependencies
```cpp
// DON'T: Core 1 calling Core 0 functions
PanelManager::GetInstance().LoadPanel(); // From ISR - NO!
```

## Correct Patterns

### ✅ Pure Pin-Change Processing
```cpp
// DO: Only generate requests on actual pin changes
if (newState == TriggerExecutionState::ACTIVE) {
    return trigger->GetActionRequest();
}
```

### ✅ Immediate Action
```cpp
// DO: Process events immediately when they occur
auto requests = ProcessPendingTriggerEvents();
for (const auto& request : requests) {
    ExecuteRequest(request);
}
```

### ✅ Clean Core Separation
```cpp
// DO: Core 1 only queues, Core 0 only processes
// ISR: xQueueSendFromISR(queue, &event, &woken);
// Main: xQueueReceive(queue, &event, 0);
```

## Future Extensions

### Adding New Trigger Types
1. Create trigger class inheriting from `AlertTrigger`
2. Implement `GetActionRequest()` and `GetRestoreRequest()`
3. Add ISR event type to `ISREventType` enum
4. Add event mapping in `ProcessPendingTriggerEvents()`
5. Register trigger in `RegisterAllTriggers()`

### Adding New Action Types
1. Add action type to `TriggerActionType` enum
2. Add handler case in main loop switch statement
3. Implement action in appropriate manager (PanelManager, StyleManager, etc.)

The architecture is designed to remain simple and extensible while maintaining the core principle: **pin changes drive actions, nothing else**.