---
config:
  layout: dagre
---
sequenceDiagram
    participant Main as Main (Core 0)
    participant PM as PanelManager (Core 0)
    participant TM as TriggerManager (Core 1)
    participant GPIO as GPIO Hardware
    participant Queues as Priority Queues
    participant LVGL as LVGL Renderer

    Note over Main, LVGL: System Startup Phase
    Main->>PM: init() - Initialize dual-core system
    PM->>TM: init_dual_core_system()
    TM->>TM: Create priority queues (High/Medium/Low)
    TM->>TM: Create state mutex
    TM->>TM: Setup GPIO interrupts
    TM->>TM: Start Core 1 monitoring task
    TM->>PM: Provide queue handles
    
    Note over Main, LVGL: Initial Panel Loading
    Main->>PM: create_and_load_panel_with_splash("OemOilPanel")
    PM->>PM: Set UI state to LOADING
    PM->>LVGL: Load SplashPanel
    PM->>PM: Set current panel state
    PM->>TM: notify_core1_state_change("SplashPanel", "Day")
    
    Note over Main, LVGL: Splash → Oil Panel Transition
    LVGL-->>PM: Splash load complete callback
    PM->>PM: create_and_load_panel("OemOilPanel")
    PM->>PM: Set UI state to LOADING
    PM->>LVGL: Load OemOilPanel
    PM->>TM: notify_core1_state_change("OemOilPanel", "Day")
    LVGL-->>PM: Oil panel load complete
    PM->>PM: Set UI state to IDLE
    
    Note over Main, LVGL: Normal Operation Loop
    loop Main Loop
        Main->>PM: update_panel()
        PM->>PM: Set UI state to UPDATING
        PM->>PM: process_trigger_messages()
        
        alt UI State: UPDATING
            PM->>Queues: Process high priority queue
            PM->>Queues: Process medium priority queue
            Note over PM: Skip low priority queue during updates
        end
        
        PM->>LVGL: panel->update()
        PM->>PM: Set UI state to IDLE
        LVGL-->>PM: Update complete callback
    end
    
    Note over Main, LVGL: Hardware Interrupt Event
    GPIO->>TM: Key present interrupt (GPIO 25)
    TM->>TM: handle_key_present_interrupt(true)
    
    alt Key Present Logic
        TM->>TM: Check current panel != "KeyPanel"
        TM->>TM: Check application state via mutex
        TM->>Queues: post_message(ACTION_LOAD_PANEL, "KeyPanel", HIGH_PRIORITY)
        TM->>TM: Set pending_messages["key_present"] = true
    end
    
    Note over Main, LVGL: Core 0 Processes Key Panel Request
    PM->>Queues: Receive high priority message
    PM->>PM: execute_message_action("LoadPanel", "KeyPanel")
    PM->>PM: Set UI state to LOADING
    PM->>PM: create_and_load_panel("KeyPanel", trigger_callback, true)
    PM->>LVGL: Load KeyPanel
    PM->>TM: notify_core1_state_change("KeyPanel", "Day")
    LVGL-->>PM: Key panel load complete
    PM->>PM: Set UI state to IDLE
    
    Note over Main, LVGL: Key Removal Event
    GPIO->>TM: Key not present interrupt (GPIO 26)
    TM->>TM: handle_key_present_interrupt(false)
    
    alt Key Removal Logic
        TM->>TM: Check current panel == "KeyPanel"
        TM->>TM: Check application state via mutex
        TM->>Queues: post_message(ACTION_RESTORE_PREVIOUS_PANEL, "", HIGH_PRIORITY)
        TM->>TM: Set pending_messages["key_present"] = false
    end
    
    Note over Main, LVGL: Core 0 Processes Panel Restoration
    PM->>Queues: Receive high priority message
    PM->>PM: execute_message_action("RestorePreviousPanel", "")
    PM->>PM: Get restoration panel ("OemOilPanel")
    PM->>PM: Set UI state to LOADING
    PM->>PM: create_and_load_panel("OemOilPanel", trigger_callback, false)
    PM->>LVGL: Load OemOilPanel
    PM->>TM: notify_core1_state_change("OemOilPanel", "Day")
    LVGL-->>PM: Oil panel load complete
    PM->>PM: Set UI state to IDLE
    
    Note over Main, LVGL: Return to Normal Operation
    loop Resume Normal Loop
        Main->>PM: update_panel()
        PM->>PM: process_trigger_messages()
        PM->>LVGL: panel->update()
    end