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
- **Core 1 (Producer)**: GPIO trigger states monitored
- **Core 0 (Consumer)**: UI and panel management
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
GPIO Pin Change → ISR Handler → Queue Event
```

**Responsibilities:**
- GPIO interrupt service routines (ISRs)
- Queue ISR events to `isrEventQueue`

### Core 0 (Consumer) - All Logic
```
Main Loop → ProcessPendingTriggerEvents() → Action Requests → Execute Actions
```

**Responsibilities:**
- Process queued ISR events
- Generate action requests on pin changes
- Execute actions via PanelManager/StyleManager
- All UI operations and state management

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
- No complex state machines to debug
- Clear logging at each step

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

### ✅ Clean Core Separation
```cpp
// DO: Core 1 only produces, Core 0 only consumes, and handle UI
// ISR: xQueueSendFromISR(queue, &event, &woken);
// Main: xQueueReceive(queue, &event, 0);
```

The architecture is designed to remain simple and extensible while maintaining the core principle: **pin changes drive actions, nothing else**.