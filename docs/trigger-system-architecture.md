# Trigger System Architecture

## Overview

The Clarity project implements a dual-core trigger management system for ESP32-based automotive engine monitoring. This system handles GPIO pin state changes and executes corresponding actions/restores based on priority-driven evaluation.

## Dual-Core Architecture

### Core 1 (GPIO Monitoring)
**Responsibilities:**
- Monitor GPIO pin state changes via interrupts
- Update shared trigger state (active/inactive) only
- Queue ISR events for safe task processing
- **No action execution** - purely state management

**Key Components:**
- `TriggerMonitoringTask()` - Main task running on Core 1
- GPIO interrupt handlers (ISR-safe)
- Event queue for ISR-to-Task communication
- Mutex-protected trigger state updates

### Core 0 (Trigger Evaluation & Execution)
**Responsibilities:**
- Evaluate all trigger states based on current GPIO pin state
- Execute actions for active triggers
- Execute restores for inactive triggers
- Process triggers from lowest to highest priority
- Handle all UI/LVGL operations safely

**Key Components:**
- `EvaluateAndExecuteTriggers()` - Main evaluation function
- Priority-based trigger sorting and execution
- Panel management and UI operations
- Theme switching and display updates

## Key Design Principles

### 1. No Complex State Tracking
- GPIO pin state directly controls trigger active status
- No transition tracking or complex state machines
- Pin state changes immediately reflect in trigger evaluation

### 2. Priority-Based Evaluation
```cpp
enum class TriggerPriority {
    CRITICAL = 0,  // Key presence, safety triggers
    IMPORTANT = 1, // Lock state, system modes  
    NORMAL = 2     // Theme changes, settings
}
```

- Triggers processed from lowest to highest priority (NORMAL → IMPORTANT → CRITICAL)
- Highest priority action executed last, ensuring it "wins"
- Multiple concurrent triggers handled gracefully

### 3. Action/Restore Pattern
- **Active triggers**: Execute action function
- **Inactive triggers**: Execute restore function
- Generic implementation using `std::function<void()>`

### 4. Cross-Core Safety
- All UI operations confined to Core 0
- LVGL single-threaded requirement maintained
- Mutex-protected shared state access

## Implementation Details

### Trigger Registration
```cpp
// Example: Lock trigger registration
auto lockTrigger = std::make_unique<LockTrigger>();
lockTrigger->init();
triggerManager.RegisterTrigger(std::move(lockTrigger));
```

### GPIO Event Flow
1. **GPIO Pin Change** → ISR Handler
2. **ISR Handler** → Queue Event (Core 1)
3. **Monitoring Task** → Update Trigger State (Core 1)
4. **Panel Manager** → Evaluate Triggers (Core 0)
5. **Trigger Manager** → Execute Actions/Restores (Core 0)

### Priority Execution Order
```cpp
// Execution sequence for concurrent triggers:
// 1. NORMAL priority triggers (lights/theme)
// 2. IMPORTANT priority triggers (lock state)
// 3. CRITICAL priority triggers (key presence)
// Result: CRITICAL trigger action is final and active
```

## Concrete Trigger Implementations

### Key Trigger (CRITICAL Priority)
- **Action**: Load key panel when key detected
- **Restore**: Restore previous panel when key removed
- **GPIO**: Monitors key presence pins (25, 26)

### Lock Trigger (IMPORTANT Priority)
- **Action**: Load lock panel when lock engaged
- **Restore**: Restore previous panel when lock disengaged
- **GPIO**: Monitors lock state pin (27)

### Lights Trigger (NORMAL Priority)
- **Action**: Switch to night theme when lights on
- **Restore**: Switch to day theme when lights off
- **GPIO**: Monitors lights state pin (32)

## Benefits

### Simplified Architecture
- Eliminated complex state machines
- Reduced mutex usage and contention
- Clear separation of concerns between cores

### Robust Priority Handling
- Concurrent trigger support
- Predictable behavior with multiple active triggers
- Highest priority always wins

### Cross-Core Safety
- No LVGL violations
- Consistent UI thread execution
- Eliminated cross-core crashes

### Maintainable Code
- Generic trigger interface
- Function-based action/restore pattern
- Easy to add new trigger types

## Usage Examples

### Adding a New Trigger
```cpp
// 1. Create trigger class inheriting from AlertTrigger
class CustomTrigger : public AlertTrigger {
public:
    CustomTrigger() : AlertTrigger(
        "custom_trigger_id",
        TriggerPriority::NORMAL,
        CustomAction,    // Action function
        CustomRestore    // Restore function
    ) {}
    
    static void CustomAction() {
        // Implement action logic
    }
    
    static void CustomRestore() {
        // Implement restore logic
    }
};

// 2. Register with trigger manager
auto customTrigger = std::make_unique<CustomTrigger>();
triggerManager.RegisterTrigger(std::move(customTrigger));
```

### GPIO Pin Configuration
```cpp
// Add to gpio_pins.h
constexpr int CUSTOM_PIN = 33;

// Add to ISR event types
enum class ISREventType {
    // ... existing types
    CUSTOM_EVENT
};

// Implement ISR handler and register interrupt
```

## Thread Safety

### Mutex Protection
- Single mutex protects trigger collection
- Timeout-based acquisition (100ms)
- Consistent lock ordering prevents deadlocks

### ISR Safety
- ISR handlers use queue-based communication
- No direct trigger execution from ISR context
- `IRAM_ATTR` for interrupt handlers

### Memory Management
- RAII-based trigger lifecycle
- `std::unique_ptr` for automatic cleanup
- No manual memory management required

## Performance Characteristics

### Latency
- GPIO interrupt to action execution: ~10-50ms
- Priority evaluation: O(n log n) for sorting
- Typical trigger count: 3-5 triggers

### Memory Usage
- Minimal per-trigger overhead
- Function objects more efficient than virtual calls
- Queue-based ISR communication limits memory usage

### CPU Usage
- Core 1: Minimal - only GPIO monitoring
- Core 0: Trigger evaluation integrated with main loop
- No dedicated high-frequency polling

## Future Enhancements

### Possible Improvements
1. **Configurable Priorities**: Runtime priority adjustment
2. **Trigger Groups**: Related trigger coordination
3. **Conditional Triggers**: Context-dependent activation
4. **Trigger Metrics**: Performance monitoring and debugging
5. **Hot-swappable Triggers**: Runtime trigger registration/removal

### Compatibility
- Designed for ESP32 dual-core architecture
- FreeRTOS task management
- LVGL UI framework integration
- Arduino framework compatibility