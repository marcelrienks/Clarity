# Trigger System Architecture

## Overview

The Clarity trigger system implements a **dual-core event-driven architecture** with complete separation of responsibilities between ESP32 cores. The system uses a **producer-consumer pattern** with **dependency injection** to ensure reliable, race-condition-free trigger processing.

## Dual-Core Architecture

### Core 1 (Producer) - Event Monitoring

#### Hardware Layer
**GPIO Interrupt Handlers** (`trigger_manager.cpp:320+`)
- `gpio_key_present_isr()`, `gpio_lock_state_isr()`, `gpio_theme_switch_isr()`
- **Responsibility**: Detect hardware pin state changes immediately
- **Output**: Queue ISR events to `isrEventQueue`

#### Task Layer
**TriggerMonitoringTask** (`trigger_manager.cpp:223-309`)
- **Responsibility**: 
  - Receive ISR events from queue
  - Map hardware events to trigger IDs
  - Log trigger events for debugging (no state modification)
- **Priority**: Reduced priority (2) to avoid blocking Core 0
- **Output**: Events remain queued for Core 0 processing

### Core 0 (Consumer) - State Management & Execution

#### Main Loop Orchestration (`main.cpp:34-35`)
```cpp
// 1. Process trigger events
TriggerManager::GetInstance().ProcessPendingTriggerEvents();  
// 2. Evaluate triggers
auto triggerRequests = TriggerManager::GetInstance().EvaluateAndGetTriggerRequests();  
```

#### Trigger Event Processing
**ProcessPendingTriggerEvents()** (`trigger_manager.cpp:71-120`)
- **Responsibility**: 
  - Dequeue all pending trigger events (non-blocking)
  - Map events to trigger objects
  - Update trigger states (`INIT` → `ACTIVE`/`INACTIVE`)
- **Key Feature**: No mutex needed - Core 0 exclusive ownership

#### Trigger Evaluation
**EvaluateAndGetTriggerRequests()** (`trigger_manager.cpp:122-189`)
- **Responsibility**:
  - Read all trigger states
  - Sort by priority (CRITICAL → IMPORTANT → NORMAL)
  - Collect action requests based on states
- **Output**: Vector of `TriggerActionRequest` objects

## Core Design Principles

### 1. Single Responsibility
- **Triggers**: Know WHAT they need (business logic)
- **Managers**: Know HOW to provide it (implementation)
- **Main**: Orchestrates WHO calls WHOM (dependency injection)

### 2. Core Separation
- **Core 1**: Hardware monitoring only (no UI/logic)
- **Core 0**: All state management and UI operations

### 3. No Direct Dependencies
- Triggers don't include manager headers
- Managers don't know about triggers
- Communication via request/response pattern

### 4. Mutex-Free Operation
- Core 1 only writes to queue
- Core 0 only reads from queue and owns all trigger state
- No cross-core synchronization needed

## Trigger Implementations

Each trigger class implements specific business logic with **dependency injection**:

### KeyTrigger (`src/triggers/key_trigger.cpp`)
- **Action**: Request key panel load when key present
- **Restore**: Request previous panel restoration when key removed
- **Priority**: CRITICAL (0) - safety-related
- **Dependencies**: None (returns action requests)

### LockTrigger (`src/triggers/lock_trigger.cpp`)
- **Action**: Request lock panel load when locked
- **Restore**: Request previous panel restoration when unlocked  
- **Priority**: IMPORTANT (1) - security-related
- **Dependencies**: None (returns action requests)

### LightsTrigger (`src/triggers/lights_trigger.cpp`)
- **Action**: Request night theme when lights on
- **Restore**: Request day theme when lights off
- **Priority**: NORMAL (2) - aesthetic changes
- **Dependencies**: None (returns action requests)

## Action Request Processing

### Request Structure (`utilities/types.h`)
```cpp
enum class TriggerActionType {
    None,           // No action requested
    LoadPanel,      // Request to load a specific panel
    RestorePanel,   // Request to restore previous panel
    ToggleTheme     // Request to toggle theme
};

struct TriggerActionRequest {
    TriggerActionType type = TriggerActionType::None;
    const char* panelName = nullptr;  // Panel name for LoadPanel actions
    const char* triggerId = nullptr;  // Trigger ID for callbacks
    bool isTriggerDriven = false;     // Whether this is a trigger-driven panel change
};
```

### Main Loop Processing (`main.cpp:37-71`)
```cpp
for (const auto& request : triggerRequests) {
    switch (request.type) {
        case TriggerActionType::LoadPanel:
            PanelManager::GetInstance().CreateAndLoadPanel(...);
            break;
        case TriggerActionType::RestorePanel:
            // Determine restoration panel and load
            break;
        case TriggerActionType::ToggleTheme:
            StyleManager::GetInstance().set_theme(...);
            break;
    }
}
```

### Manager Integration
- **PanelManager**: Executes panel loading/switching requests
- **StyleManager**: Executes theme change requests
- **Main**: Owns the relationship between triggers and managers

## Event Flow Example

### Lights Trigger Flow (Theme Toggle)

1. **Hardware**: Lights pin goes HIGH → GPIO interrupt fires
2. **Core 1**: ISR queues `THEME_SWITCH` event with `pinState=HIGH`
3. **Core 0**: Main loop calls `ProcessPendingTriggerEvents()`
4. **State Update**: Lights trigger state: `INIT` → `ACTIVE`
5. **Evaluation**: `EvaluateAndGetTriggerRequests()` calls `GetActionRequest()`
6. **Request**: Returns `{ToggleTheme, "Night", ...}`
7. **Execution**: Main calls `StyleManager::set_theme("Night")`
8. **Result**: Theme changes to night mode

When pin goes LOW, same flow but trigger goes `ACTIVE` → `INACTIVE` and calls `GetRestoreRequest()` returning day theme.

## State Management

### Trigger States
- **INIT**: Initial state, no GPIO changes detected yet
- **ACTIVE**: Trigger condition is active (pin HIGH)
- **INACTIVE**: Trigger condition is inactive (pin LOW)

### State Transitions
```
INIT → ACTIVE    (when pin goes HIGH)
INIT → INACTIVE  (when pin goes LOW)
ACTIVE → INACTIVE (when pin goes LOW)
INACTIVE → ACTIVE (when pin goes HIGH)
```

### Priority System
Triggers are processed in priority order (lowest number = highest priority):
- **CRITICAL (0)**: Safety-related (KeyTrigger)
- **IMPORTANT (1)**: Security-related (LockTrigger)  
- **NORMAL (2)**: Aesthetic changes (LightsTrigger)

Higher priority actions override lower priority actions when multiple triggers are active simultaneously.

## Extensibility

The architecture supports easy addition of new trigger sources:

### GPIO-Based Triggers
Current implementation supports GPIO interrupts via the existing ISR system.

### Future Trigger Sources
The generic `ProcessPendingTriggerEvents()` can be extended to handle:
- **Timer-based triggers**: Scheduled events
- **Network event triggers**: Remote commands
- **Sensor threshold triggers**: Analog sensor limits
- **User input triggers**: Button presses, touch events

### Adding New Triggers
1. Create trigger class inheriting from `AlertTrigger`
2. Implement `GetActionRequest()` and `GetRestoreRequest()`
3. Register trigger in `TriggerManager::RegisterAllTriggers()`
4. Add event source to queue trigger events

## Benefits

### Performance
- **No mutex contention** between cores
- **Reduced context switching** - GPIO monitoring isolated to Core 1
- **Predictable timing** - main loop controls execution order

### Reliability
- **No race conditions** - single-threaded state management
- **No missed events** - queued event processing
- **Clean error handling** - isolated failure domains

### Maintainability
- **Clean separation of concerns** - hardware vs. logic vs. UI
- **Easy testing** - triggers can be unit tested without hardware
- **Flexible architecture** - easy to add new trigger types and actions

### Extensibility
- **Generic event processing** - supports non-GPIO trigger sources
- **Dependency injection** - clean interfaces between components
- **Modular design** - triggers, managers, and main are independently testable

This architecture ensures reliable, scalable trigger processing while maintaining clean code organization and excellent performance characteristics.