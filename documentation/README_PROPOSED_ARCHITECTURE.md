# FreeRTOS Dual-Core Trigger Architecture

This document defines the architectural design for the Clarity trigger system using FreeRTOS dual-core capabilities for improved performance and responsiveness. implement the architecture described in the document, and above. two cores being used, one for UI tasks driven by Panel Manager, and one for interrupts driven by Trigger Manager. The role of the trigger manager is to track GPIO pin states, by monitoring state change hardware interrupts, checking current ui state, and evaluating if a change is required, if so posting, updating or deleting messages on priority queues. Ensuring that no redundant messages are read that would result in a net no change result

## 📋 Architecture Overview

### **Dual-Core Separation**
- **Core 0 (APP_CPU)**: LVGL/UI Management Core
- **Core 1 (PRO_CPU)**: Hardware Trigger Detection Core

### **Core Responsibilities**

#### **Core 0 (LVGL/UI Core)**
- **Primary**: All LVGL operations, panel management, UI rendering
- **Secondary**: Message queue consumption, trigger action execution
- **State Management**: Tracks application state (Idle, Updating, Loading, LVGL_Busy)
- **Queue Processing**: Reads trigger messages when safe to do so

#### **Core 1 (Hardware Core)**
- **Primary**: Hardware interrupt handling, trigger evaluation, decision making
- **Secondary**: Message queue production and management
- **Event-Driven**: Only posts messages when state changes require action
- **State-Aware**: Maintains current application state to make intelligent decisions

## 🔄 Message Flow Architecture

### **Message Queue Design**
```cpp
struct TriggerMessage {
    char trigger_id[32];        // Unique trigger identifier
    char action[32];            // Action to perform ("LoadPanel", "RestorePreviousPanel", "ChangeTheme")
    char target[32];            // Target panel/theme name
    int priority;               // Message priority level
    uint32_t timestamp;         // Message creation time
};

enum class UIState {
    IDLE,        // Safe to process all messages immediately
    UPDATING,    // Throttled processing (high/medium priority only)
    LOADING,     // No message processing
    LVGL_BUSY    // No message processing
};
```

### **Multiple Priority Queue System**
- **High Priority Queue**: Critical triggers (key presence, safety)
- **Medium Priority Queue**: Important triggers (lock state, system modes)
- **Low Priority Queue**: Non-critical triggers (theme changes, settings)

## 🔧 Implementation Design

### **Core 1 (Hardware) - Stateful TriggerManager**
```cpp
class TriggerManager {
private:
    // Priority queue handles
    QueueHandle_t high_priority_queue;
    QueueHandle_t medium_priority_queue;
    QueueHandle_t low_priority_queue;
    
    // Shared application state (thread-safe)
    std::string current_panel;
    std::string current_theme;
    std::map<std::string, bool> pending_messages; // Track queued messages
    
public:
    void handle_key_present_interrupt(bool key_present) {
        if (key_present == true) {
            // Key inserted - check if KeyPanel is already showing
            if (current_panel != "KeyPanel") {
                // Need to show KeyPanel - post message
                post_message("LoadPanel", "KeyPanel", HIGH_PRIORITY);
                pending_messages["key_present"] = true;
            }
            // else: KeyPanel already showing, do nothing
            
        } else {
            // Key removed - check current state
            if (current_panel == "KeyPanel") {
                // KeyPanel is showing - post restore message
                post_message("RestorePreviousPanel", "", HIGH_PRIORITY);
                pending_messages["key_present"] = false;
            } else if (pending_messages["key_present"]) {
                // KeyPanel not showing but message is pending - remove message
                remove_message_from_queue("key_present");
                pending_messages["key_present"] = false;
            }
            // else: KeyPanel not showing and no pending message, do nothing
        }
    }
    
    void handle_theme_switch_interrupt(bool night_mode) {
        std::string target_theme = night_mode ? "Night" : "Day";
        
        if (current_theme != target_theme) {
            if (pending_messages["theme_switch"]) {
                // Update existing message
                update_message_in_queue("theme_switch", "ChangeTheme", target_theme);
            } else {
                // Post new message
                post_message("ChangeTheme", target_theme, LOW_PRIORITY);
                pending_messages["theme_switch"] = true;
            }
        } else {
            // Target theme already active
            if (pending_messages["theme_switch"]) {
                // Remove pending message - no change needed
                remove_message_from_queue("theme_switch");
                pending_messages["theme_switch"] = false;
            }
        }
    }
};
```

### **Core 0 (UI) - Simplified Action Executor**
```cpp
// Core 0 PanelManager - No decision logic, just execution
void process_trigger_messages() {
    TriggerMessage msg;
    
    switch (ui_state) {
        case UIState::IDLE:
            // No throttling - process all priority queues
            process_all_priority_queues();
            break;
            
        case UIState::UPDATING:
            // State-based throttling - only high/medium priority queues
            process_high_priority_queue();
            process_medium_priority_queue();
            // Skip low priority queue
            break;
            
        case UIState::LOADING:
        case UIState::LVGL_BUSY:
            // No action - don't process any queues
            // Messages remain queued for later processing
            break;
    }
}

void execute_message_action(TriggerMessage msg) {
    if (strcmp(msg.action, "LoadPanel") == 0) {
        load_panel(msg.target);
    } else if (strcmp(msg.action, "RestorePreviousPanel") == 0) {
        restore_previous_panel();
    } else if (strcmp(msg.action, "ChangeTheme") == 0) {
        change_theme(msg.target);
    }
    
    // Notify Core 1 of state change
    notify_core1_state_change(current_panel, current_theme);
}
```

## ⚙️ Configuration

```cpp
// Multiple Priority Queues
#define HIGH_PRIORITY_QUEUE_SIZE    15   // High priority triggers (key, safety)
#define MEDIUM_PRIORITY_QUEUE_SIZE  15   // Medium priority triggers (lock, mode)  
#define LOW_PRIORITY_QUEUE_SIZE     15   // Low priority triggers (theme, settings)

// Priority levels for trigger classification
#define HIGH_PRIORITY_LEVEL         0    // Critical triggers
#define MEDIUM_PRIORITY_LEVEL       1    // Important triggers  
#define LOW_PRIORITY_LEVEL          2    // Non-critical triggers

// State-based throttling during UI updates
#define UPDATING_STATE_MAX_PRIORITY 1    // Process high/medium only during updates

// Shared state synchronization
#define PANEL_STATE_MUTEX_TIMEOUT   100  // Mutex timeout in milliseconds
#define THEME_STATE_MUTEX_TIMEOUT   100  // Mutex timeout in milliseconds
```

## 📊 State-Based Processing

| UI State | Processing Behavior | Queues Processed |
|----------|-------------------|------------------|
| **IDLE** | No throttling | High → Medium → Low |
| **UPDATING** | Selective throttling | High → Medium only |
| **LOADING** | No action | None (all queued) |
| **LVGL_BUSY** | No action | None (all queued) |

## 🎯 Key Architectural Principles

### **Stateful TriggerManager with Application State Awareness**
- **Core 1 Decision Making**: TriggerManager on Core 1 makes all decisions based on current application state (panel/theme) and hardware interrupts
- **Core 0 Action Execution**: Core 0 simply executes actions without decision logic
- **Intelligent Processing**: Only sends messages when actual state change is needed

### **State Synchronization Requirements**
- **Core 0 → Core 1**: Notify state changes after panel loads/theme changes
- **Thread Safety**: Shared state access must be mutex-protected
- **Initial State**: Core 1 must know initial panel/theme on startup

## 🚀 Benefits

This dual-core architecture provides:
- **Sub-millisecond hardware response** through dedicated Core 1 interrupt handling
- **Zero unnecessary UI operations** through intelligent state-aware decision making  
- **Clean separation of concerns** with Core 1 making decisions and Core 0 executing actions
- **Perfect efficiency** by only processing actual state changes that require action
- **Scalable design** for adding new triggers and system settings

The system eliminates current polling bottlenecks while maintaining safe LVGL operations and providing a robust foundation for future enhancements.

## 📝 Design Decision Rationale

### **Why Dual-Core Separation?**
- **Hardware Responsiveness**: Dedicated Core 1 ensures sub-millisecond interrupt response
- **UI Protection**: Core 0 LVGL operations protected from interrupt interference
- **Performance**: Parallel processing eliminates polling overhead

### **Why Stateful TriggerManager?**
- **Eliminates Redundancy**: Only processes changes that actually need action
- **Prevents UI Flashing**: No unnecessary panel switching from rapid state changes
- **Simplifies Core 0**: Pure action execution without complex decision logic

### **Why Multiple Priority Queues?**
- **Perfect Priority**: Always processes highest priority first without sorting
- **ISR Safe**: Standard FreeRTOS queue operations in interrupt context
- **State-based Throttling**: Different queues can be processed based on UI state

### **Why Application State Awareness?**
- **Intelligent Decisions**: Core 1 knows current panel/theme state for smart decisions
- **Prevents Conflicts**: Avoids sending messages when target state is already active
- **Optimal Efficiency**: Only sends messages when state change is actually needed